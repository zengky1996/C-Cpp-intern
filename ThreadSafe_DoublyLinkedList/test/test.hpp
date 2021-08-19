#ifndef __Test_add_head_HPP__
#define __Test_add_head_HPP__

#include<iostream>  
using namespace std;

void testaddhead();
void testaddtail();
void testremove();
void test_printfromhead_addtail();
void test_printfromhead_addhead();
void test_printfromtail_addtail();
void test_printfromtail_addhead();

class ControlRequest {
public:
    //uint32_t pid;
    uint64_t id;
    uint32_t type;
    uint32_t level;
    uint32_t interval;
    uint64_t nextTime;

    #if 0
    ControlRequest(const ControlRequest &param)
    {
        this->id = param.id;
        this->type = param.type;
        this->level = param.level;
        this->interval = param.interval;
        this->nextTime = param.nextTime;
    }
    #endif

    ControlRequest(uint64_t id,
                   uint32_t type,
                   uint32_t level,
                   uint32_t interval,
                   uint64_t nextTime) 
    {
        this->id = id;
        this->type = type;
        this->level = level;
        this->interval = interval;
        this->nextTime = nextTime;
    }

    void dump() 
    {
        std::cout<<"id is "<<id<<std::endl;
        std::cout<<"type is "<<type<<std::endl;
        std::cout<<"level is "<<level<<std::endl;
        std::cout<<"interval is "<<interval<<std::endl;
        std::cout<<"nextTime is "<<nextTime<<std::endl;
        std::cout << "      " << std::endl;
    }

};
#endif