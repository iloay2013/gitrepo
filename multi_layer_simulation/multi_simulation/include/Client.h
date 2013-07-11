#include <TPCW_Simulation.h>
#include <gsl/gsl_rng.h>

class Event;
class Server;

// Client Status
#define CS_READY    0
#define CS_WAITING  1
#define CS_SRET     2 // Server Rejected Error Thinking
#define CS_TOT      3 // Timeout Thinking
#define CS_THINKING 4
#define CS_LEFT     5

// Request Type
#define CRT_NOT_REQUEST -1
#define CRT_NOT_REQUEST_ID -1
#define CRT_NOT_REQUEST_SERIAL -1
#define CRT_INIT 0
#define CRT_ADMC 1
#define CRT_ADMR 2
#define CRT_BESS 3
#define CRT_BUYC 4
#define CRT_BUYR 5
#define CRT_CREG 6
#define CRT_HOME 7
#define CRT_NEWP 8
#define CRT_ORDD 9
#define CRT_ORDI 10
#define CRT_PROD 11
#define CRT_SREQ 12
#define CRT_SRES 13
#define CRT_SHOP 14
#define NUMBER_OF_REQUEST 15

#define FIRST_SERIAL 0
// Max Request Serial ( # of Images on Page )
#define MAX_SERIAL_ADMC  6
#define MAX_SERIAL_ADMR  6
#define MAX_SERIAL_BESS  9
#define MAX_SERIAL_BUYC  2
#define MAX_SERIAL_BUYR  3
#define MAX_SERIAL_CREG  4
#define MAX_SERIAL_HOME 11
#define MAX_SERIAL_NEWP  9
#define MAX_SERIAL_ORDD  0
#define MAX_SERIAL_ORDI  3
#define MAX_SERIAL_PROD  5
#define MAX_SERIAL_SREQ  9
#define MAX_SERIAL_SRES  9
#define MAX_SERIAL_SHOP  9

#define RAMP_UP_TIME 1.0 // -RU(Ramp-up time)=100
#define RAMP_DOWN_TIME 1.0 // -RD(Ramp-down time)=100

#define CYCLE_TIME 1198.0 + RAMP_UP_TIME // Ramp-up time plus 1000 Seconds

#define ERROR_WAIT_TIME 60.0 // XXX
#define TIMEOUT_WAIT_TIME 30.0 // XXX
#define SLEEP_TIME 5.0 // This is an expection in ImageReader
#define TIMEOUT 20.0 // XXX

#define THINK_TIME_MIN 7.0
#define THINK_TIME_lMIN 0.36788
#define THINK_TIME_MAX 70.0
#define THINK_TIME_lMAX 4.54E-5
#define THINK_TIME_MU 7.0

class Client;

class Request {
    public:
        int request_type;
        int request_serial;
        int requestID;
        long double request_time;
        Client * request_client;
        Request * next;

        Request ( int type, int serial, long double time, Client * client );
};

class Client {
    public:
        int clientID;
        int clientGeneration;
        int status;

        gsl_rng * think_time_generator;
        gsl_rng * next_state_picker;

    private:
        bool firstSession;
        int request_number;
        int current_requestID;
        int current_request_type;
        int current_request_serial;
        Event * current_timeout_event;

        long double last_cycle_end_time;

    public:
        Client ( int i );
        int init ( void );
        int schedule_a_request ( int client_status );
        int send_a_request ( int type, int serial, long double time );
        int timeout ( void );
        int response_received ( int requestID );
        ~Client ( void );

    private:
        int do_schedule ( int do_schedule_status, long double * time, int * type, int *serial, int *eventIsNewPage );
        int should_goto_next_page ( int request_type, int request_serial );
        long double think_time ( int current_status );
        long double negExp ( double random, double min, double lMin, double max, double lMax, double mu );
        int get_next_request ( int type );
        int wakeup_a_server ( void ); // Return a sleeped server
        int add_timeout_event ( void );
        int remove_timeout_event ( void );
        int leave ( void );
};
