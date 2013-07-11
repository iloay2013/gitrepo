#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <gsl/gsl_rng.h>

#define CLIENT_NUM 100 // XXX # of EBs
#define SERVER_NUM 1 // XXX

#define MAX_QUEUE_LEN 300 * SERVER_NUM // ( 200 (# of threads) + 100 (length of buffer) ) * SERVER_NUM

#define SEED_SCALE 1000000000.0

using namespace std;

class TPCW;
class Client;
class Server;
class Event;
class Request;
class OS;
class Monitor;

extern int status;
extern long double sim_time;
extern int alive_client;
extern vector<Client*> pClient_list;
extern vector<Server*> pServer_list;
extern unsigned long long event_number;

extern Event * event_head;
extern Event * event_tail;
extern unsigned long event_length;

extern Request * request_head;
extern Request * request_tail;
extern int queue_length;

extern Monitor *monitor;
extern TPCW * tpcw;
extern OS *os;
extern int global_request_number;
extern int page_request_number;
extern int timeout_number;
extern int error_number;
extern int finished_number;
extern int rejuvenation_counter;
extern int failure_counter;

//extern int global_accelerate_level;

extern const int transProb[15][15];

extern bool beginRun;
extern const gsl_rng_type *T;
extern unsigned long seed_of_seed_generator;
extern gsl_rng *seed_generator;

//void Usage ( void );
