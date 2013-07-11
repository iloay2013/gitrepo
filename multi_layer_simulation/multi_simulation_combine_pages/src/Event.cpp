#include "TPCW_Simulation.h"
#include "Event.h"
#include "Client.h"
#include "Server.h"
#include "OSleak.h"

Event::Event ( void ) {
    eventID = NOT_EVENT_ID;
    eventType = NOT_EVENT;
    eventTime = 0.0;

    requestType = CRT_NOT_REQUEST;
    requestSerial = CRT_NOT_REQUEST_SERIAL;
    requestID = CRT_NOT_REQUEST_ID;
    isNewPage = 0;

    next = period = NULL;

    invalid = false;
}

int Event::doEvent ( void ) {
    if ( eventType == ET_SaR ) {
        if ( this->invalid ) { // Dead Client
            return 0;
        } else if ( create_a_request () ) {
            cerr<<"Error when sending a request from Client!"<<endl;
            return 1;
        }
    }
    else if ( eventType == ET_RTO ) {
        if ( this->invalid ) { // Dead Client
            return 0;
        } else if ( request_timeout ( ) ) {
            cerr<<"Error when Client dealing with a timeout error!"<<endl;
            return 1;
        }
    }
    else if ( eventType == ET_RD ) {
        if ( this->invalid ) {
            return 0;
        } else if ( finish_a_request ( requestID ) ) {
            cerr<<"Error when Server processing a request!"<<endl;
            return 1;
        }
    }
    else if ( eventType == ET_SRj ) {
        if ( this->invalid ) {
            return 0;
        } else if ( start_rejuvenation ( ) ) {
            cerr<<"Error when start rejuvenation!"<<endl;
            return 1;
        }
    }
    else if ( eventType == ET_FRj ) {
        if ( this->invalid ) {
            return 0;
        } else if ( finish_rejuvenation ( ) ) {
            cerr<<"Error when finish rejuvenation!"<<endl;
            return 1;
        }
    }
    else if ( eventType == ET_SRy) {
        if ( this->invalid ) {
            return 0;
        } else if ( server_recovery ( ) ) {
            cerr<<"Error when recovery server!"<<endl;
            return 1;
        }
    }
    else {
        cerr<<"Error! Bad Event Type!"<<endl;
        return 1;
    }

    if(os_memory_consumed())
    {
        cerr<<"Error when OS consume memory!"<<endl;
        return 1;
    }
    return 0;
}

int Event::add_to_list ( void ) {
    Event *e = event_head;

    if ( e == NULL ) {
        if ( event_tail != NULL ) {
            cerr<<"Error in Event::add_to_list: event_head is NULL while event_tail is not!"<<endl;
            exit (-1);
        }
        event_head = event_tail = this;
        this->period = NULL;
        this->next = NULL;
    } else {
        while ( e->eventTime < this->eventTime && e->next != NULL ) {
            e = e->next;
        }
        if ( e->eventTime >= this->eventTime ) {
            if ( e->period == NULL ) {
                event_head = this;
                this->period = NULL;
            } else {
                e->period->next = this;
                this->period = e->period;
            }
            e->period = this;
            this->next = e;
        } else {
            e->next = this;
            this->period = e;
            this->next = NULL;
            event_tail = this;
        }
    }
    ++event_length;
    return 0;
}

int Event::create_a_request ( void ) {
    if ( client->send_a_request ( requestType, requestSerial, eventTime ) ) {
        return 1;
    }
    ++global_request_number;
    if ( isNewPage == EVENT_IS_NEW_PAGE ) {
        ++page_request_number;
    }
    return 0;
}

int Event::request_timeout ( void ) {
    if ( client->timeout () ) {
        return 1;
    }
    ++timeout_number;
    return 0;
}

int Event::finish_a_request ( int ID ) {
    if ( server->process_a_request ( requestType, requestSerial ) ) {
        return 1;
    }
    if ( client->response_received ( ID ) ) {
        return 1;
    }
    if ( isNewPage == EVENT_IS_NEW_PAGE ) {
        ++finished_number;
    }
    return 0;
}

int Event::start_rejuvenation ( void ) {
    if ( server->rejuvenate ( true ) ) {
        return 1;
    }
    ++rejuvenation_counter;
    return 0;
}

int Event::finish_rejuvenation ( void ) {
    if ( server->rejuvenate ( false ) ) {
        return 1;
    }
    return 0;
}

int Event::server_recovery ( void ) {
    if ( server->recovery ( ) ) {
        return 1;
    }
    return 0;
}
int Event::os_memory_consumed( void ){
    if( os->memory_consumed()){
        return 1;
    }
    return 0;
}
