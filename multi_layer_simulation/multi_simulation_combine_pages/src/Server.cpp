#include "TPCW_Simulation.h"
#include "Server.h"
#include "Client.h"
#include "Event.h"
#include "OSleak.h"
#include "TPCW.h"

Server::Server ( int i ) {
    serverID = i;
//    randomNumber_generator = NULL;

    logfile.open ( "Server.log" );
    RJSlogfile.open ( "Rejuvenation.log" );
	logfile.setf(ios::showpoint);
	logfile.precision(std::numeric_limits<long double>::digits10);
	RJSlogfile.setf(ios::showpoint);
	RJSlogfile.precision(std::numeric_limits<long double>::digits10);
}

Server::~Server ( void ) {
//    gsl_rng_free ( randomNumber_generator );
    logfile.close ( );
    RJSlogfile.close ( );
}

int Server::init ( ) {
    status = SS_IDLE;
//    accelerate_level = global_accelerate_level;
    memoryUsage = INIT_MEMORY_USAGE;
    nextGCTime = sim_time + GC_INTERVAL;
    memoryCanBeGCed = 0.0;
    lastConsumeTime = sim_time;

    lambda_count_1=lambda_count_2=0;
    
//    if ( randomNumber_generator == NULL ) {
//        randomNumber_generator = gsl_rng_alloc ( T );
//    }
//    gsl_rng_set ( randomNumber_generator, ( unsigned long ) ( gsl_rng_uniform ( seed_generator ) * SEED_SCALE ) );
//    
//    
//    randomNumber = 0;

    logfile<<"XXX: Server started at: "<<sim_time<<endl; // Server start time
    return 0;
}

int Server::fetch_a_request ( void ) {
    // Fetch a Request and add event
    Request * r;
    if ( request_head == NULL ) {
        if ( status != SS_FAILED && status != SS_RJ ) {
            status = SS_IDLE;
        }
    } else {
        r = request_head;
        request_head = r->next;
        if ( request_head == NULL ) {
            request_tail = NULL;
        }
        --queue_length;

        if ( status == SS_FAILED ) {
        } else if ( status == SS_RJ ) {
        } else {
            long double service_time = 0.0;
            if ( generate_service_time ( r->request_type, r->request_serial, &service_time ) ) {
                cerr<<"Error in Server "<<serverID<<": cannot generate service time!"<<endl;
                return 1;
            }
            
            Event *e = new Event;
            e->eventID = event_number++;
            e->eventType = ET_RD;
            e->eventTime = sim_time + service_time;
            e->client = r->request_client;
            e->server = this;
            e->requestType = r->request_type;
            e->requestSerial = r->request_serial;
            e->requestID = r->requestID;
            if ( e->requestSerial == FIRST_SERIAL ) {
                e->isNewPage = EVENT_IS_NEW_PAGE;
            }
            if ( e->add_to_list () ) {
                cerr<<"Error in Server "<<serverID<<": cannot add a server event!"<<endl;
                return 1;
            }
            if ( status == SS_IDLE ) {
                status = SS_WORKING;
            }
        }

        delete r;
    }

    return 0;
}

int Server::process_a_request ( int type, int serial ) {
    // Memory Consumption and Garbage Collection
    long double memoryConsumed = 0.0;
    if ( status != SS_FAILED ) { // We measured memory consumption rate
        memoryConsumed = ( sim_time - lastConsumeTime ) * MEM_CONSUME_RATE;
        memoryUsage += memoryConsumed;
        memoryCanBeGCed += memoryConsumed;
    }
    lastConsumeTime = sim_time;

    if ( sim_time >= nextGCTime ) { // GC
        if ( garbage_collection () ) {
            exit (-1);
        }
    }
//当type==CRT_SHOP && serial == FIRST_SERIAL时在AS层注入内存
//当type==CRT_SREQ && serial == FIRST_SERIAL使同时在AS层和OS层注入内存
    // Inject Memory Leak
//    if ( (type == CRT_SREQ && serial == FIRST_SERIAL) || (type ==CRT_SHOP && serial ==FIRST_SERIAL)) {
//        if ( randomNumber == 0 ) {
//            do {
//                randomNumber = (int) ( gsl_rng_uniform_pos ( randomNumber_generator ) * accelerate_level );
//            } while ( 0 > randomNumber );
//
//            memoryUsage += INJECTED_MEMORY; // Inject 1MB Memory Usage
//            logfile<<sim_time<<" "<<memoryUsage<<endl; // LOG AT INJECT POINT
//
//            if(type == CRT_SHOP)
//            {
//                //调用操作系统层的内存注入方法
//                os->inject_memory();
//
//            }
//        } else {
//            --randomNumber;
//        }
//    }

//    if ( (type == CRT_SREQ && serial == FIRST_SERIAL) || (type ==CRT_SHOP && serial ==FIRST_SERIAL)) {
//        //往APP注入内存
//        memoryUsage += INJECTED_MEMORY; // Inject Memory Usage
//        logfile<<sim_time<<" "<<memoryUsage<<endl; // LOG AT INJECT POINT
//        if(type==CRT_SHOP)
//        {
//            //往OS注入内存
//            os->inject_memory();
//        }
//    }


    if(type == CRT_SREQ && serial == FIRST_SERIAL)
    {
        memoryUsage += lambda_array_1[lambda_count_1++];
        logfile<<sim_time<<" "<<memoryUsage<<endl;

        os->inject_memory();
    }
    if(type == CRT_SHOP && serial == FIRST_SERIAL)
    {
        memoryUsage += lambda_array_2[lambda_count_2++];
        logfile<<sim_time<<" "<<memoryUsage<<endl;
    }
    if( memoryUsage >= MEMORY_CAPACITY * 0.85)
    {
    // If Memory Full
        if ( memoryUsage >= MEMORY_CAPACITY ) {
            cout<<"APP failed"<<endl;
            if ( garbage_collection () ) {
                exit (-1);
            }
            if ( memoryUsage >= MEMORY_CAPACITY ) { // Still Full, then we die
                status = SS_FAILED;
                ++failure_counter;

                Event * p = event_head; // mark remain events invalid
                if ( p != NULL ) {
                    while ( p != NULL ) {
                        if ( p->server == this ) {
                            p->invalid = true;
                        }
                        p = p->next;
                    }
                }

                Request * r = request_head; // empty request list
                while ( r != NULL ) {
                    r = r->next;
                    delete request_head;
                    request_head = r;
                }
                request_head = request_tail = NULL;
                queue_length = 0;

                
                logfile<<"XXX SERVER FAILED at "<<sim_time<<endl;// " RECOVERY TIME IS  "<<e->eventTime-sim_time<<endl;
//                sleep(5);
            }
        }
        else
        {
//            cout<<"APP degraded"<<endl;
            status=SS_DEGRADED;
        }
    }

    if ( fetch_a_request () ) { // Fetch another request
        cout<<"Error! Cannot fetch a request!"<<endl;
    }

    return 0;
}

int Server::generate_service_time ( int req_type, int req_serial, long double * time ) {
    if ( req_type == CRT_INIT ) {
        *time = SRT_INIT;
    } else if ( req_serial == FIRST_SERIAL ) { // Requesting HTML Page
        if ( req_type == CRT_ADMC ) {
            *time = SRT_ADMC;
        } else if ( req_type == CRT_ADMR ) {
            *time = SRT_ADMR;
        } else if ( req_type == CRT_BESS ) {
            *time = SRT_BESS;
        } else if ( req_type == CRT_BUYC ) {
            *time = SRT_BUYC;
        } else if ( req_type == CRT_BUYR ) {
            *time = SRT_BUYR;
        } else if ( req_type == CRT_CREG ) {
            *time = SRT_CREG;
        } else if ( req_type == CRT_HOME ) {
            *time = SRT_HOME;
        } else if ( req_type == CRT_NEWP ) {
            *time = SRT_NEWP;
        } else if ( req_type == CRT_ORDD ) {
            *time = SRT_ORDD;
        } else if ( req_type == CRT_ORDI ) {
            *time = SRT_ORDI;
        } else if ( req_type == CRT_PROD ) {
            *time = SRT_PROD;
        } else if ( req_type == CRT_SREQ ) {
            *time = SRT_SREQ;
        } else if ( req_type == CRT_SRES ) {
            *time = SRT_SRES;
        } else if ( req_type == CRT_SHOP ) {
            *time = SRT_SHOP;
        }
    } else { // Requesting Images
        if ( req_type == CRT_ADMC ) {
            *time = SRT_ADMC_IMAGE;
        } else if ( req_type == CRT_ADMR ) {
            *time = SRT_ADMR_IMAGE;
        } else if ( req_type == CRT_BESS ) {
            *time = SRT_BESS_IMAGE;
        } else if ( req_type == CRT_BUYC ) {
            *time = SRT_BUYC_IMAGE;
        } else if ( req_type == CRT_BUYR ) {
            *time = SRT_BUYR_IMAGE;
        } else if ( req_type == CRT_CREG ) {
            *time = SRT_CREG_IMAGE;
        } else if ( req_type == CRT_HOME ) {
            *time = SRT_HOME_IMAGE;
        } else if ( req_type == CRT_NEWP ) {
            *time = SRT_NEWP_IMAGE;
        } else if ( req_type == CRT_ORDD ) {
            *time = SRT_ORDD_IMAGE;
        } else if ( req_type == CRT_ORDI ) {
            *time = SRT_ORDI_IMAGE;
        } else if ( req_type == CRT_PROD ) {
            *time = SRT_PROD_IMAGE;
        } else if ( req_type == CRT_SREQ ) {
            *time = SRT_SREQ_IMAGE;
        } else if ( req_type == CRT_SRES ) {
            *time = SRT_SRES_IMAGE;
        } else if ( req_type == CRT_SHOP ) {
            *time = SRT_SHOP_IMAGE;
        }
    }

    if ( *time < 0 ) {
        cerr<<"Error when generate service time: service_time < 0!"<<endl;
    }

    return 0;
}

int Server::garbage_collection ( void ) {
    memoryUsage -= memoryCanBeGCed;
    memoryCanBeGCed = 0;
    nextGCTime += GC_INTERVAL;

    if ( sim_time >= nextGCTime ) {
        nextGCTime = sim_time + GC_INTERVAL / 2;
    }

    // When GC is working, other threads must wait
    double gc_time = GC_TIME;

    Event * e = event_head;
    while ( e != NULL ) {
        if ( e->eventType == ET_RD ) {
            e->eventTime += gc_time;
        }
        e = e->next;
    }

    return 0;
}

int Server::rejuvenate ( bool start ) {
    Request * r = request_head; // empty request list TWICE!
    while ( r != NULL ) {
        r = r->next;
        delete request_head;
        request_head = r;
    }
    request_head = request_tail = NULL;
    queue_length = 0;

    if ( start ) { // Start a RJ
        status = SS_RJ;

        Event * p = event_head;
        if ( p != NULL ) {
            while ( p != NULL ) {
                if ( p->server == this && p->eventType == ET_RD ) {
                    p->invalid = true;
                }
                p = p->next;
            }
        }

        schedule_a_rejuvenation ( false ); // to finish this RJ
        logfile<<"XXX: Server start REJ at: "<<sim_time<<endl; // Server start REJ time
    } else { // Finish a RJ
        this->init ( ); // reinit server
        logfile<<"XXX: Server finished REJ at: "<<sim_time<<endl; // Server finish REJ time
    }
    return 0;
}

int Server::schedule_a_rejuvenation ( bool start ) {
    long double event_time = 0.0;
    Event * e = new Event;

    e->eventID = event_number++;
    e->client = NULL;
    e->server = this;

    if ( start ) { // schedule a RJ
        e->eventType = ET_SRj;
        // event_time = gsl_ran_erlang ( rejuvenation_scheduler, RJ_SCHE_LAMBDA, RJ_SCHE_K ); // erlang
        event_time = REJUVENATION_INTERVAL;
        RJSlogfile<<"S: "<<sim_time + event_time<<endl; // LOG RJ Start Time
    } else { // schedule finish time for an ongoing RJ
        e->eventType = ET_FRj;
        // event_time = gsl_ran_exponential ( rejuvenation_time_generator, RJ_FIN_TIME ); // exponential
        event_time = RJ_FIN_TIME;
        RJSlogfile<<"T: "<<sim_time + event_time<<endl; // LOG RJ Finish Time
    }

    e->eventTime = sim_time + event_time;

    if ( e->add_to_list ( ) ) {
        cerr<<"Cannot add RJ Event to List!"<<endl;
        return 1;
    }

    return 0;
}

int Server::recovery ( void ) {
    Request * r = request_head; // empty request list!
    while ( r != NULL ) {
        r = r->next;
        delete request_head;
        request_head = r;
    }
    request_head = request_tail = NULL;
    queue_length = 0;

    this->init ( ); // reinit server
    logfile<<"XXX: Server recoveried at: "<<sim_time<<endl; // Server recovery time

    return 0;
}
