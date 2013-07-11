#include "TPCW_Simulation.h"
#include "Client.h"
#include "Event.h"
#include "Server.h"

Request::Request ( int type, int serial, long double time, Client * client ) {
    request_type = type;
    request_serial = serial;
    request_time = time;
    request_client = client;
    next = NULL;
}


Client::Client ( int i ) {
    clientID = i;
    clientGeneration = -1;
    think_time_generator = NULL;
    next_state_picker = NULL;

    firstSession = true;
}

Client::~Client ( void ) {
    gsl_rng_free ( think_time_generator );
    gsl_rng_free ( next_state_picker );
}

int Client::init ( void ) {
    ++clientGeneration;
    status = CS_READY;
    request_number = 0;

    current_requestID = CRT_NOT_REQUEST_ID;
    current_request_type = CRT_NOT_REQUEST;
    current_request_serial = CRT_NOT_REQUEST_SERIAL;
    current_timeout_event = NULL;

    last_cycle_end_time = sim_time;

    if ( think_time_generator == NULL ) {
        think_time_generator = gsl_rng_alloc ( T );
    }
    gsl_rng_set ( think_time_generator, ( unsigned long ) ( gsl_rng_uniform ( seed_generator ) * SEED_SCALE ) );

    if ( next_state_picker == NULL ) {
        next_state_picker = gsl_rng_alloc ( T );
    }
    gsl_rng_set ( next_state_picker, ( unsigned long ) ( gsl_rng_uniform ( seed_generator ) * SEED_SCALE ) );

    return 0;
}

int Client::schedule_a_request ( int client_status ) {
    long double request_time;
    int request_type;
    int request_serial;
    int eventIsNewPage = 0;

    if ( do_schedule ( client_status, &request_time, &request_type, &request_serial, &eventIsNewPage ) ) {
        cerr<<"Error in Client "<<clientID<<": cannot schedule a request!"<<endl;
        return 1;
    }


    if ( status != CS_LEFT ) {
        Event * e = new Event;
        e->eventID = event_number++;
        e->eventType = ET_SaR;
        e->eventTime = request_time;
        e->client = this;
        e->server = NULL;
        e->requestType = request_type;
        e->requestSerial = request_serial; // Request Still Not Send
        e->isNewPage = eventIsNewPage;
        if ( e->add_to_list () ) {
            cerr<<"Error in Client "<<clientID<<": cannot add event for a request!"<<endl;
            return 1;
        }
        current_request_type = request_type;
        current_request_serial = request_serial;
    }

    return 0;
}

int Client::do_schedule ( int do_schedule_status, long double * time, int * type, int * serial, int * eventIsNewPage ) {
    long double think_time_result = 0.0;

    if ( do_schedule_status == CS_READY ) { // First schedule, Ramp-up
        long double ramp_time = 0.0;
        if ( firstSession ) {
            ramp_time = RAMP_UP_TIME;
            firstSession = false;
        } else {
            ramp_time = RAMP_DOWN_TIME + RAMP_UP_TIME;
        }
        *time = sim_time + ramp_time;
        *type = CRT_INIT;
        *serial = FIRST_SERIAL;
    } else if ( do_schedule_status == CS_SRET ) { // Error
        if ( ( think_time_result = think_time ( do_schedule_status ) ) > 0 ) {
            *time = sim_time + ERROR_WAIT_TIME + SLEEP_TIME + think_time_result;
            *type = current_request_type;
            *serial = current_request_serial;
        } else {
            cout<<"Wrong think time!\nExiting..."<<endl;
            exit (-1);
        }
    } else if ( do_schedule_status == CS_TOT ) { // Timeout
        if ( ( think_time_result = think_time ( do_schedule_status ) ) > 0 ) {
            *time = sim_time + TIMEOUT_WAIT_TIME + SLEEP_TIME + think_time_result;
            *type = current_request_type;
            *serial = current_request_serial;
        } else {
            cout<<"Wrong think time!\nExiting..."<<endl;
            exit (-1);
        }
    } else if ( do_schedule_status == CS_WAITING ) { // Normal Situation
        if ( should_goto_next_page ( current_request_type, current_request_serial ) ) {
            if ( ( think_time_result = think_time ( do_schedule_status ) ) > 0 ) {
//                *time = sim_time + think_time_result;
#ifdef DEBUG
                cout<<"Think time is"<<think_time_result<<"!"<<endl;
#endif
                *type = get_next_request ( current_request_type );
#ifdef SIMPLE_MODEL
                *time = sim_time + think_time_result + SLEEP_TIME * (get_page_image_num(*type)-1);
#else                
                *time = sim_time + think_time_result;
#endif                
                *serial = FIRST_SERIAL;
                *eventIsNewPage = EVENT_IS_NEW_PAGE;
            } else {
                cout<<"Wrong think time!\nExiting..."<<endl;
                exit (-1);
            }
#ifdef DEBUG
        cout<<"Next Request type is"<<*type<<endl;
#endif
        } else {
            if ( current_request_serial != FIRST_SERIAL ) {
                *time = sim_time + SLEEP_TIME;
            } else {
                *time = sim_time;
            }
            *type = current_request_type;
            *serial = current_request_serial + 1; // Next serial
        }
    } else {
        cerr<<"Error when do_schedule: Client status should not be "<<do_schedule_status<<"!"<<endl;
        return 1;
    }

    return 0;
}

long double Client::think_time ( int current_status ) {
    double random = gsl_rng_uniform_pos ( think_time_generator ); // random: (0, 1)

    if ( current_status == CS_READY ) { // Ramp-up thinktime // TODO Can be different
        return negExp ( random, THINK_TIME_MIN, THINK_TIME_lMIN, THINK_TIME_MAX, THINK_TIME_lMAX, THINK_TIME_MU );
    } else if ( current_status == CS_SRET ) { // Error thinktime // TODO Can be different
        return negExp ( random, THINK_TIME_MIN, THINK_TIME_lMIN, THINK_TIME_MAX, THINK_TIME_lMAX, THINK_TIME_MU );
    } else if ( current_status == CS_TOT ) { // Timeout thinktime // TODO Can be different
        return negExp ( random, THINK_TIME_MIN, THINK_TIME_lMIN, THINK_TIME_MAX, THINK_TIME_lMAX, THINK_TIME_MU );
    } else if ( current_status == CS_WAITING ) { // Normal
        return negExp ( random, THINK_TIME_MIN, THINK_TIME_lMIN, THINK_TIME_MAX, THINK_TIME_lMAX, THINK_TIME_MU );
    } else {
        cerr<<"Error when generating think time: Client status should not be "<<current_status<<"!"<<endl;
        return -1;
    }
}

long double Client::negExp ( double random, double min, double lMin, double max, double lMax, double mu ) {
    if ( random < lMax ) {
        return ( (long double) max );
    }
    return ( (long double) ( -mu * log ( random ) ) );
}                                                           

int Client::get_next_request ( int current_request ) {
    int i;
    int picker;

    do {
        picker = gsl_rng_uniform_pos ( next_state_picker ) * 10000;
    } while ( 0 >= picker );

    if ( picker <= 0 || picker >= 10000 ) {
        cerr<<"Error when Client "<<clientID<<" choose a picker!"<<endl;
        exit (-1);
    }

    for ( i = 0; transProb[current_request][i] < picker; i++ )
        ;

    if ( i < NUMBER_OF_REQUEST ) {
        return i;
    }
    else {
        cerr<<"Error when Client "<<clientID<<" picking next request!"<<endl;
        exit (-1);
    }
}

int Client::should_goto_next_page ( int request_type, int request_serial ) { // FIXME store MAX_SERIAL_* to an array
    if ( request_type == CRT_INIT )
        return 1;

    if ( request_type == CRT_ADMC && request_serial >= MAX_SERIAL_ADMC )
        return 1;

    if ( request_type == CRT_ADMR && request_serial >= MAX_SERIAL_ADMR )
        return 1;

    if ( request_type == CRT_BESS && request_serial >= MAX_SERIAL_BESS )
        return 1;

    if ( request_type == CRT_BUYC && request_serial >= MAX_SERIAL_BUYC )
        return 1;

    if ( request_type == CRT_BUYR && request_serial >= MAX_SERIAL_BUYR )
        return 1;

    if ( request_type == CRT_CREG && request_serial >= MAX_SERIAL_CREG )
        return 1;

    if ( request_type == CRT_HOME && request_serial >= MAX_SERIAL_HOME )
        return 1;

    if ( request_type == CRT_NEWP && request_serial >= MAX_SERIAL_NEWP )
        return 1;

    if ( request_type == CRT_ORDD && request_serial >= MAX_SERIAL_ORDD )
        return 1;

    if ( request_type == CRT_ORDI && request_serial >= MAX_SERIAL_ORDI )
        return 1;

    if ( request_type == CRT_PROD && request_serial >= MAX_SERIAL_PROD )
        return 1;

    if ( request_type == CRT_SREQ && request_serial >= MAX_SERIAL_SREQ )
        return 1;

    if ( request_type == CRT_SRES && request_serial >= MAX_SERIAL_SRES )
        return 1;

    if ( request_type == CRT_SHOP && request_serial >= MAX_SERIAL_SHOP )
        return 1;

    return 0;
}
int Client::get_page_image_num(int request_type){
    // FIXME: store MAX_SERIAL_* to an array
    if ( request_type == CRT_INIT )
        return 0;

    if (request_type == CRT_ADMC)
        return 6;

    if (request_type == CRT_ADMR)
        return 6;

    if (request_type == CRT_BESS)
        return 9;

    if (request_type == CRT_BUYC)
        return 2;

    if (request_type == CRT_BUYR)
        return 3;

    if (request_type == CRT_CREG)
        return 4;

    if (request_type == CRT_HOME)
        return 11;

    if (request_type == CRT_NEWP)
        return 9;

    if (request_type == CRT_ORDD)
        return 0;

    if (request_type == CRT_ORDI)
        return 3;

    if (request_type == CRT_PROD)
        return 5;

    if (request_type == CRT_SREQ)
        return 9;

    if (request_type == CRT_SRES)
        return 9;

    if (request_type == CRT_SHOP)
        return 9;

    return 0;
}

int Client::wakeup_a_server ( void ) {
    int i = 0;
    for ( i = 0; i < SERVER_NUM; i++) {
        if ( pServer_list[i]->status == SS_IDLE ) {
            return i;
        } else if ( pServer_list[i]->status == SS_FAILED || pServer_list[i]->status == SS_RJ ) {
            return i;
        }
    }
    return i;
}

int Client::send_a_request ( int type, int serial, long double time ) {
    if ( status == CS_LEFT )
        return 0;

    Request *r;

    if ( queue_length < MAX_QUEUE_LEN ) {
        r = new Request ( type, serial, time, this );
        if ( queue_length == 0 ) {
            request_head = request_tail = r;
        } else {
            request_tail->next = r;
            request_tail = r;
        }
        request_tail->next = NULL;
        ++queue_length;
        ++request_number;
        r->requestID = global_request_number;
        current_requestID = r->requestID;

        if ( add_timeout_event () ) {
            cerr<<"Error when adding Timeout event by Client!"<<clientID<<endl;
            return 1;
        }

        status = CS_WAITING;
    } else {
        ++request_number;
        if ( schedule_a_request ( CS_SRET ) ) {
            cerr<<"Error when thinking after a rejected request: Client "<<clientID<<" cannot schedule a request!"<<endl;
            return 1;
        }
        status = CS_SRET;
        ++error_number;
    }

    int picked_server = SERVER_NUM;
    if ( ( picked_server = wakeup_a_server () ) < SERVER_NUM ) {
        if ( pServer_list[picked_server]->fetch_a_request ( ) ) {
            cerr<<"Error when wake up sleeped server!"<<endl;
        }
    }

    if ( sim_time >= last_cycle_end_time + CYCLE_TIME ) {
        if ( init () ) {
            cerr<<"Error when Client "<<clientID<<" reinit!"<<endl;
            exit (-1);
        } else { // Client Nirvana!
            Event * e = event_head;
            while ( e != NULL ) { // Mark old Event invalid
                if ( e->client == this && ( e->eventType == ET_SaR || e->eventType == ET_RTO ) ) {
                    e->invalid = true;
                }
                e = e->next;
            }
            if ( this->schedule_a_request ( CS_READY ) ) {
                cerr<<"Error when reinit client "<<this->clientID<<": cannot active Client!"<<endl;
                return 1;
            }
        }
    }

    return 0;
}

int Client::timeout ( void ) {
    current_timeout_event = NULL;
    current_requestID = CRT_NOT_REQUEST_ID;
    if ( status == CS_LEFT ) {
        return 0;
    }
    if ( schedule_a_request ( CS_TOT ) ) {
        cerr<<"Error when thinking after Timeout: Client "<<clientID<<" cannot schedule a request!"<<endl;
        return 1;
    }
    status = CS_TOT;
    return 0;
}

int Client::response_received ( int requestID ) {
    if ( status == CS_LEFT ) {
        return 0;
    }
    if ( requestID == current_requestID ) {
        if ( remove_timeout_event () ) {
            cerr<<"Error when Client "<<clientID<<" remove a Timeout Event!"<<endl;
            return 1;
        }
        if ( schedule_a_request ( CS_WAITING ) ) {
            cerr<<"Error when thinking: Client "<<clientID<<" cannot schedule a request!"<<endl;
            return 1;
        }
        status = CS_THINKING;
        current_requestID = CRT_NOT_REQUEST_ID;
        current_timeout_event = NULL;
    } else {
    }
    return 0;
}

int Client::add_timeout_event ( void ) {
    Event *e = new Event;
    e->eventID = event_number++;
    e->eventType = ET_RTO;
    e->eventTime = sim_time + TIMEOUT;
    e->client = this;
    e->server = NULL;
    e->requestType = CRT_NOT_REQUEST;
    if ( e->add_to_list () ) {
        cerr<<"Error in Client "<<clientID<<": cannot set a Timeout event!"<<endl;
        return 1;
    }
    current_timeout_event = e;
    return 0;
}

int Client::remove_timeout_event ( void ) {
    Event * e;

    if ( current_timeout_event != NULL ) {
        e = current_timeout_event;
    } else {
        return 1;
    }

    if ( e == event_head ) { // when e is the first event
        event_head = event_head->next; // e will never be empty here
        if ( event_head == NULL ) { // we removed the very last event
            event_tail = NULL;
        } else {
            event_head->period = NULL;
        }
    } else if ( e == event_tail ) { // when e is the last event
        event_tail = event_tail->period; // same as above
        if ( event_tail == NULL ) {
            event_head = NULL;
        } else {
            event_tail->next = NULL;
        }
    } else {
        if(e->period==NULL)
            cout<<"Event period is NULL"<<endl;
        if(e->next==NULL)
            cout<<"Event next is NULL"<<endl;
        if(e->period->next==NULL)
            cout<<"Event period next is NULL"<<endl;
        if(e->next->period==NULL)
            cout<<"Event next period is NULL"<<endl;
        e->period->next = e->next;
        e->next->period = e->period;
    }
    --event_length;
    delete e;
    current_timeout_event = NULL;
    return 0;
}

int Client::leave ( void ) {
    status = CS_LEFT;
    --alive_client;
    return 0;
}
