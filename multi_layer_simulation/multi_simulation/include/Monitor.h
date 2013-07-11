#include "TPCW_Simulation.h"
#include <gsl/gsl_rng.h>
class OSleak;
class Server;
#define ON 0
#define OFF 1

#define Robust 0
#define RO 1
#define RA 2

#define REPORT_OUTPUT_RATE 0.5 

#define RANK_OF_DIAGACCUR 3
const double diagAccur[3][3]=
{
 //       Robust   RO    RA   
/*Robust*/{ 0.98 , 0.01, 0.01 },
 /*RO*/   { 0.01 , 0.98, 0.01 },
 /*RA*/   { 0.01 , 0.01, 0.98 }
};
class Monitor
{
    private:
        double diagAccur[3][3];
        int disposeResult(int);
        int dealMethod(int,Server *server=NULL);
    public:
        Monitor();
        ~Monitor();
        int init(void);
        int detectStatus();
        int mState;
        gsl_rng * next_picker;
};
