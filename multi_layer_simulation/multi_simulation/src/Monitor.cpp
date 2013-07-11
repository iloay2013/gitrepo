#include "TPCW_Simulation.h"
#include "Monitor.h"
#include "OSleak.h"
#include "Server.h"
Monitor::Monitor()
{
    mState=OFF;
    next_picker=NULL;
}
Monitor::~Monitor()
{
    gsl_rng_free(next_picker);
}
int Monitor::init(void)
{
    mState=ON;
    if(next_picker==NULL)
        next_picker=gsl_rng_alloc(T);
    gsl_rng_set(next_picker,(unsigned long)(gsl_rng_uniform(seed_generator)*SEED_SCALE));
    return 0;
}
int Monitor::detectStatus()
{
    cout<<"Monitor begin to detect APP and OS status"<<endl;
    int j;
    int nextStatus=-1;

    time_t generateReportTime=1/REPORT_OUTPUT_RATE;

    if(mState==ON)
    {
        cout<<"Monitor is on"<<endl;
      //先判断APP的状态，再判断操作系统层的状态
        sleep(generateReportTime);
        sleep(2);
        for(j=0;j<SERVER_NUM;j++)
        {
            cout<<"begin to detect server"<<endl;
            if(pServer_list[j]->status==SS_DEGRADED)
                nextStatus=disposeResult(RA);
            else if(pServer_list[j]->status==SS_IDLE || pServer_list[j]->status==SS_WORKING)
                nextStatus=disposeResult(Robust);
            else
            {
                ;//do nothing
            }
            cout<<"begin to solve APP problems"<<endl;
            if(dealMethod(nextStatus,pServer_list[j]))
            {
                cerr<<"Error when Monitor solve APP problems"<<endl;
                exit(-1);
            }
        }
        cout<<"finish detecting server"<<endl;

        cout<<"begin to detect OS"<<endl;
        if(os->osStatus==OS_Normal)
        {
            cout<<"OS Normal"<<endl;
            nextStatus=disposeResult(Robust);
        }
        else if(os->osStatus==OS_Degraded)
        {
            cout<<"OS degrade"<<endl;
            nextStatus=disposeResult(RO);
        }
        cout<<"begin to solve OS problems"<<endl;
        if(dealMethod(nextStatus))
        {
            cerr<<"Error when Monitor solve OS problems"<<endl;
            exit(-1);
        }
        cout<<"finish detecting OS"<<endl;
    }
    mState=OFF;
    cout<<"turn off Monitor"<<endl;
    return 0;
}
int Monitor::disposeResult(int Status)
{
    int i;
    int picker;
    do
    {
        picker=gsl_rng_uniform_pos(next_picker);
    }while( picker<0 );


    double sumPro=0.0;
    for( i=0; sumPro<picker && i<RANK_OF_DIAGACCUR; i++)
    {
        sumPro+=diagAccur[Status][i];
    }

    if( i >= RANK_OF_DIAGACCUR)
    {
        cerr<<"Error when Monitor judging next OS status"<<endl;
        exit(-1);
    }
    return i;
}
int Monitor::dealMethod(int nextStatus,Server *server)
{
   if(nextStatus==RA)
   {
       cout<<"oops,APP degrade,start to restart APP"<<endl;
       server->rejuvenate(true);
   }
   else if(nextStatus==RO)
   {
       cout<<"oops,OS degrade,start to restart OS"<<endl;
       os->rejuvenate();
       beginRun=true;
   }
   else
   {
       ;//do nothing
   }
   return 0;
}
