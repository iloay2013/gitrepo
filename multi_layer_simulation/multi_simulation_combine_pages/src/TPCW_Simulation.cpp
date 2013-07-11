#include "TPCW_Simulation.h"
#include "TPCW.h"
#include "Event.h"
#include "Client.h"
#include "Server.h"
#include "OSleak.h"
#include "Monitor.h"

int status;
long double sim_time;
int alive_client;
vector<Client*> pClient_list(CLIENT_NUM);
vector<Server*> pServer_list(SERVER_NUM);
unsigned long long event_number;

Event * event_head;
Event * event_tail;
unsigned long event_length;

Request * request_head;
Request * request_tail;
int queue_length;

Monitor *monitor;
OS *os;
TPCW *tpcw;
int global_request_number;
int page_request_number;
int timeout_number;
int error_number;
int finished_number;
int rejuvenation_counter;
int failure_counter;

//int global_accelerate_level;

const gsl_rng_type *T;
unsigned long seed_of_seed_generator;
gsl_rng *seed_generator;

bool beginRun=true;
const int transProb[15][15] =
{
  //         INIT  ADMC  ADMR  BESS  BUYC  BUYR  CREG  HOME  NEWP  ORDD  ORDI  PROD  SREQ  SRES  SHOP
  /*INIT*/ {    0,    0,    0,    0,    0,    0,    0, 9999,    0,    0,    0,    0,    0,    0,    0},
                                                                                                       
  /*ADMC*/ {    0,    0,    0,    0,    0,    0,    0, 9952,    0,    0,    0,    0, 9999,    0,    0},
  /*ADMR*/ {    0, 8999,    0,    0,    0,    0,    0, 9999,    0,    0,    0,    0,    0,    0,    0},
  /*BESS*/ {    0,    0,    0,    0,    0,    0,    0,  167,    0,    0,    0,  472, 9927,    0, 9999},
  /*BUYC*/ {    0,    0,    0,    0,    0,    0,    0,   84,    0,    0,    0,    0, 9999,    0,    0},
  /*BUYR*/ {    0,    0,    0,    0, 4614,    0,    0, 6546,    0,    0,    0,    0,    0,    0, 9999},
  /*CREG*/ {    0,    0,    0,    0,    0, 8666,    0, 8760,    0,    0,    0,    0, 9999,    0,    0},
  /*HOME*/ {    0,    0,    0, 3124,    0,    0,    0,    0, 6249,    0, 6718,    0, 7026,    0, 9999},
  /*NEWP*/ {    0,    0,    0,    0,    0,    0,    0,  156,    0,    0,    0, 9735, 9784,    0, 9999},
  /*ORDD*/ {    0,    0,    0,    0,    0,    0,    0,   69,    0,    0,    0,    0, 9999,    0,    0},
  /*ORDI*/ {    0,    0,    0,    0,    0,    0,    0,   72,    0, 8872,    0,    0, 9999,    0,    0},
  /*PROD*/ {    0,    0,   58,    0,    0,    0,    0,  832,    0,    0,    0, 1288, 8603,    0, 9999},
  /*SREQ*/ {    0,    0,    0,    0,    0,    0,    0,  635,    0,    0,    0,    0,    0, 9135, 9999},
  /*SRES*/ {    0,    0,    0,    0,    0,    0,    0, 2657,    0,    0,    0, 9294, 9304,    0, 9999},
  /*SHOP*/ {    0,    0,    0,    0,    0,    0, 2585, 9992,    0,    0,    0,    0,    0,    0, 9999}

};

//void Usage ( char * msg ) {
//    cout<<"Usage:"<<endl;
//    cout<<"\t"<<msg<<" N"<<endl;
//    cout<<"\tWhere N > 0  is the accelerate level."<<endl;
//}

int main (int argc, char * argv[]) {
//    if ( argc != 2 ) {
//        Usage ( argv[0] );
//        exit (-1);
//    }
    while( beginRun )
    {
        beginRun=false;
        vector<Client*> pClient_list(CLIENT_NUM);
        vector<Server*> pServer_list(SERVER_NUM);
        os = new OS();
        monitor = new Monitor();
//        global_accelerate_level = atoi ( argv[1] );
//        if ( global_accelerate_level < 0 ) {
//            Usage ( argv[0] );
//            exit (-1);
//        }
        if ( os->init() )
        {
            cerr<<"Error in TPCW initialization: cannot initialize OS"<<endl;
            return 1;
        }

        while( os->osStatus == OS_Normal || os->osStatus == OS_Degraded )
        {
            // Initialization
            tpcw = new TPCW();
            T = gsl_rng_ranlxs0;
            seed_of_seed_generator = ((unsigned long)(time(NULL)));

            seed_generator = gsl_rng_alloc (T);
            gsl_rng_set ( seed_generator, seed_of_seed_generator );

            if ( tpcw->init () ) {
                cerr<<"Exiting..."<<endl;
                exit (-1);
            }
            // Kickstart the simulation
            //
            if ( tpcw->start () ) {
                cerr<<"Exiting..."<<endl;
                exit (-1);
            }
            // Log
            //
            if ( tpcw->log () ) {
                cerr<<"Exiting..."<<endl;
                exit (-1);
            }
            // End and Clean
            gsl_rng_free ( seed_generator );
            while ( request_head != NULL ) {
                Request * r = request_head;
                request_head = request_head->next;
                delete r;
            }
            request_tail = NULL;

            while ( event_head != NULL ) {
                Event * e = event_head;
                event_head = event_head->next;
                delete e;
            }
            event_tail = NULL;
            delete tpcw;
            sleep(2);
        }
    }
    return 0;
}
