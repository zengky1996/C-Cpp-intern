#ifndef __CAN_HPP__
#define __CAN_HPP__

#include<iostream>  

using namespace std;

class CANRequest {
public:
    uint32_t  event;  
    uint32_t  data;
    char buff[64 - sizeof(uint32_t) - sizeof(uint32_t)];

#if 0
    CANRequest(const CANRequest &param)
    {
        this->event = param.can_id;      
        this->data = param.data;
    }
#endif

    CANRequest() {}

    CANRequest(uint32_t  event,  /* 32 bit CAN_ID + FORMAT/CANFD/TYPE flags */
               uint32_t  data) 
    {
        this->event = event;
        this->data = data;
    }

    void print() 
    {
        std::cout<<"event is "<<this->event<<"   CAN data is "<<this->data<<std::endl;
    }

};
#endif