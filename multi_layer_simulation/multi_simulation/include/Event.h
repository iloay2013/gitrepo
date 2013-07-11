#include "TPCW_Simulation.h"

class Client;
class Server;
class OS;
// Event Type
#define ET_SaR 0 // Send a Request
#define ET_RTO 1 // Request Timeout
#define ET_RD  2 // Request Processed (by Server)
#define ET_SRj 3 // Start a Rejuvenation
#define ET_FRj 4 // Finish a Rejuvenation
#define ET_SRy 5 // Server Recovery

#define EVENT_IS_NEW_PAGE 1

#define NOT_EVENT_ID -1
#define NOT_EVENT 512

class Event {
    public:
        int eventID;
        int eventType;
        long double eventTime;

        Client * client;
        Server * server;

        int requestType;
        int requestSerial;
        int requestID;
        int isNewPage;

        Event * next;
        Event * period;

        bool invalid;

    public:
        Event ( void );
        int doEvent ( void );
        int add_to_list ( void );

    private:
        int create_a_request ( void );
        int request_timeout ( void );
        int finish_a_request ( int ID );
        int os_memory_consumed( void );
        int start_rejuvenation ( void );
        int finish_rejuvenation ( void );
        int server_recovery ( void );
};
