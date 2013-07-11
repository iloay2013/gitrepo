#include "TPCW_Simulation.h"
#include "TPCW.h"
#include "Event.h"
#include "Client.h"
#include "Server.h"
#include "OSleak.h"

int TPCW::init ( void ) { // Initialize Class TPCW
    int i = 0;

    status = TPCW_STOPPED;
    sim_time = 0.0;
    event_number = 0;
    event_length = 0;
    queue_length = 0;

    global_request_number = 0;
    page_request_number = 0;
    timeout_number = 0;
    error_number = 0;
    finished_number = 0;
    rejuvenation_counter = 0;
    failure_counter = 0;

    if ( empty_event_list ( event_head, event_tail ) ) { // Empty event list
        cerr<<"Error in TPCW initialization: cannot empty event list!"<<endl;
        return 1;
    }
    cout<<"empty event list"<<endl;

    if ( empty_request_list ( request_head, request_tail ) ) { // Empty request list
        cerr<<"Error in TPCW initialization: cannot empty request list!"<<endl;
        return 1;
    }
    cout<<"empty request list"<<endl;

    for ( i = 0; i < CLIENT_NUM; ++i ) { // Create and Initialize Clients
        if ( NULL == ( pClient_list[i] = new Client ( i ) ) ) {
            cerr<<"Error in TPCW initialization: cannot create Client "<<i<<"!"<<endl;
            return 1;
        }
        else {
          if ( pClient_list[i]->init () ) {
              cerr<<"Error in TPCW initialization: cannot initialize Client "<<i<<"!"<<endl;
              return 1;
          }
          ++alive_client;
        }
    }
    for ( i = 0; i < SERVER_NUM; ++i ) { // Create and Initialize Servers
        if ( NULL == ( pServer_list[i] = new Server ( i ) ) ) {
            cerr<<"Error in TPCW initialization: cannot create Server "<<i<<"!"<<endl;
            return 1;
        }
        else {
            if ( pServer_list[i]->init ( ) ) {
                cerr<<"Error in TPCW initialization: cannot initialize Server "<<i<<"!"<<endl;
                return 1;
            }
        }
    }
    return 0;
}

int TPCW::start ( void ) {
    Event * p;
    int i;
    int number_of_failed_server = 0;
    int number_of_left_client = 0;

    unsigned long long max_limit;
    max_limit = numeric_limits<unsigned long long>::max ();
    // max_limit = MAX_EVENT_LIMIT;

    if(event_head==NULL&&request_head==NULL)
    {
        cout<<"Event and request list is empty."<<endl;
    }
    else
    {
        event_head=event_tail=NULL;
        request_head=request_tail=NULL;
    }
    if ( active_clients () ) { // Active all Clients
        cerr<<"Error in TPCW simulation: cannot active clients!"<<endl;
        return 1;
    }

    // while ( event_head != NULL && sim_time < MAX_SIM_TIME ) {
//    while ( event_head != NULL && event_number <= max_limit ) {
        // If all Servers Failed, then quit
     while( event_head !=NULL && event_number <= max_limit ){
        for ( i = 0; i < SERVER_NUM; i++ ) { // Find Number of Failed Server
            if ( pServer_list[i]->status == SS_FAILED )
                ++number_of_failed_server;
        }
        if ( number_of_failed_server == SERVER_NUM ) {
            break;
        }
        number_of_failed_server = 0;

        // If all Client left, then quit
        for ( i = 0; i < CLIENT_NUM; i++ ) { // Find Number of Left Client
            if ( pClient_list[i]->status == CS_LEFT )
                ++number_of_left_client;
        }
        if ( number_of_left_client == CLIENT_NUM ) {
            break;
        }
        number_of_left_client = 0;

        // Fetch an Event
        p = event_head;
        event_head = event_head->next;
        if ( event_head != NULL ) {
            event_head->period = NULL;
        }
        else {
            event_tail = NULL;
        }

        --event_length;

        // Fire the current Event
        sim_time = p->eventTime;
        if ( p->doEvent () ) {
            cerr<<"Error in TPCW simulation: Event "<<p->eventID;
            if ( p->client != NULL ) {
                cerr<<" of Client "<<p->client->clientID;
            }
            if ( p->server != NULL ) {
                cerr<<" on Server "<<p->server->serverID;
            }
            cerr<<" encountered an ERROR! "<<endl;

            delete p;
            return 1;
        }

        // Delete it
        delete p;
    }

    // Do some clean
    for ( i = 0; i < CLIENT_NUM; i++ ) {
        if ( pClient_list[i] != NULL ) {
            delete pClient_list[i];
        }
    }
    for ( int i = 0; i < SERVER_NUM; i++ ) {
        if ( pServer_list[i] != NULL ) {
            delete pServer_list[i];
        }
    }

    return 0;
}

int TPCW::empty_event_list ( Event * head, Event * tail ) {
    while ( head != NULL) { // Empty list
        if ( tail->period == NULL ) {
            delete tail;
            head = NULL;
            tail = NULL;
        }
        else {
            tail = tail->period;
            delete tail->next;
            tail->next = NULL;
        }
    }
    if ( head != NULL || tail != NULL )
        return 1;
    else
        return 0;
}

int TPCW::empty_request_list ( Request * head, Request * tail ) {
    tail = head;
    while ( head != NULL ) {
        head = head->next;
        delete tail;
        tail = head;
    }
    if ( head != NULL || tail != NULL )
        return 1;
    else
        return 0;
}

int TPCW::active_clients ( void ) {
    int i = 0;

    for ( i = 0; i < CLIENT_NUM; i++ ) { // Active all Clients
        if ( pClient_list[i]->status != CS_READY ) {
            cerr<<"Error: Client "<<i<<" not initialized when active it."<<endl;
            exit (-1);
        }
        if ( pClient_list[i]->schedule_a_request ( CS_READY ) ) {
            cerr<<"Error in TPCW simulation: cannot active Client "<<i<<"!"<<endl;
            return 1;
        }
    }
    return 0;
}

int TPCW::log ( void ) {
    // TODO
//    cout<<"Accelerate Level is     : "<<global_accelerate_level<<"."<<endl;
    cout<<"Simulation Time goes to : "<<sim_time<<"."<<endl;
    cerr<<"Total Page Request      : "<<page_request_number<<"."<<endl;
    cerr<<"Responsed Request       : "<<finished_number<<"."<<endl;
    cerr<<"Timeouted Request       : "<<timeout_number<<"."<<endl;
    cerr<<"Total Error Number      : "<<error_number<<"."<<endl;
    cerr<<"Rejuvenation Times      : "<<rejuvenation_counter<<"."<<endl;
    cerr<<"Failed Times            : "<<failure_counter<<"."<<endl;
    return 0;
}
