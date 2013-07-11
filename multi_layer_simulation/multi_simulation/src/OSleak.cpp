#include "OSleak.h"
#include "TPCW_Simulation.h"
#include "Server.h"
#include "Event.h"
#include "Client.h"
#include "Monitor.h"
OS::OS()
{
    lastInspectionTime=0;
    lastConsumedTime=0;
    logfile.open ( "OS.log" );
	logfile.setf(ios::showpoint);
	logfile.precision(std::numeric_limits<long double>::digits10);
}
int OS::init()
{
    begin=time(NULL);
    lastConsumedTime=begin;
    lastInspectionTime=begin;
    end=begin;
    osStatus=OS_Normal;
    osmemoryConsumed=0.0;
    osmemoryUsage=OS_INIT_MEMORY_USAGE;
    isTheFirstTimeToReachRJ=true;
    count=0;
    logfile<<"XXX: OS start at :"<<getCurrentSystemTime()-getStartSystemTime()<<endl;
    return 0;
}

int OS::rejuvenate()
{
    osStatus=OS_RJ;
    clear();
    logfile<<"XXX: OS RJ at :"<<getCurrentSystemTime()-getStartSystemTime()<<endl;
    sleep(OS_RJ_TIME);//单位是秒
    return 0;
}

int OS::memory_consumed()
{
    time_t os_sim_time=getCurrentSystemTime()-getLastConsumedTime();
    setLastConsumedTime();
    if( osmemoryUsage < OS_MEMORY_CAPACITY * 0.85)
    {
        osmemoryConsumed=os_sim_time * OS_NORMAL_MEM_CONSUME_RATE;
        osmemoryUsage += osmemoryConsumed;
    }
    else if(osmemoryUsage >= OS_MEMORY_CAPACITY * 0.85)
    {
        if(osmemoryUsage >= OS_MEMORY_CAPACITY)
        {
            cout<<"OS failed"<<endl;
            osStatus=OS_Failed;
            cout<<osmemoryUsage<<"       "<<OS_MEMORY_CAPACITY<<endl;
            logfile<<"XXX:OS stopped at"<<getCurrentSystemTime()-getStartSystemTime()<<endl;
            clear();
//            sleep(OS_FAILED_TO_ROBUST_TIME);
        }
        else
        {
//            cout<<"OS degraded"<<endl;
            osmemoryConsumed=os_sim_time * OS_DEGRADED_MEM_CONSUME_RATE;
            osmemoryUsage += osmemoryConsumed;
            if(isTheFirstTimeToReachRJ)
            {
                osStatus=OS_Degraded;
                logfile<<"XXX:OS degraded at"<<getCurrentSystemTime()-getStartSystemTime()<<endl;
                isTheFirstTimeToReachRJ=false;
            }
        }
    }


    //定时开启监测器
    time_t lastTime=getCurrentSystemTime()-lastInspectionTime;
    if(lastTime>=INSPECTION_INTERVAL)
    {
        //开启监测器,并进行监测
        startMonitor();
        lastInspectionTime=getCurrentSystemTime();
    }
    return 0;
}

int OS::inject_memory()
{
//    osmemoryUsage += OS_INJECTED_MEMORY;
    osmemoryUsage +=lambda_array[count++]; 
    logfile<<getCurrentSystemTime()-getStartSystemTime()<<" "<<osmemoryUsage<<endl;
    return 0;
}

OS::~OS()
{
    logfile.close();
}

time_t OS::getStartSystemTime()
{
    return begin;
}

time_t OS::getCurrentSystemTime()
{
    end=time(NULL);
    return end;
}

time_t OS::getLastConsumedTime()
{
    return lastConsumedTime;
}

void OS::setLastConsumedTime()
{
    lastConsumedTime=end;
}

void OS::clear()
{
    Event * p = event_head; // mark remain events invalid
    if ( p != NULL ) {
        while ( p != NULL ) {
            p->invalid = true;
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
}
int OS::startMonitor()
{
    cout<<"OS start Monitor"<<endl;
    if(monitor->init())
    {
        cerr<<"Error when init monitor"<<endl;
        return 1;
    }
    cout<<"finish initingg monitor"<<endl;
    if(monitor->detectStatus())
    {
        cerr<<"Error when monitor detect APP and OS status"<<endl;
        return 1;
    }
    cout<<"finish detecting status"<<endl;
    return 0;
}
