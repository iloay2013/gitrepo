#include <TPCW_Simulation.h>

class Event;
class Server;
class Client;
class Request;
class OS;

#define MAX_EVENT_LIMIT 5000000
#define MAX_SIM_TIME       100000000.0

// TPCW Status
#define TPCW_RUNNING 1
#define TPCW_STOPPED 0

class TPCW {
    public:
        int init ( void );
        int start ( void );
        int log ( void );

    private:
        int empty_event_list ( Event * head, Event * tail );
        int empty_request_list ( Request * head, Request * tail );
        int active_clients ();
};
