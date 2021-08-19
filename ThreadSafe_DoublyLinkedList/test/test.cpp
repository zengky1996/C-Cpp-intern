#include <iostream>
#include <string>
#include <unistd.h>
#define DEBUG_LIST 0
#include "list.hpp"
#include "test.hpp"

void testaddhead()
{
    std::cout<<"---[List addhead Test Start]---"<<std::endl;
    std::cout<<std::endl;
    List<ControlRequest> dList(4);
    ControlRequest CR1(10,3,8362,2233,111);
    ControlRequest CR2(20,2,2341,2912,321);
    ControlRequest CR3(30,9,1213,2082,391);
    ControlRequest CR4(40,9,1293,2122,986);
    ControlRequest CR5(50,9,1293,2122,986);
    ControlRequest CR6(60,9,1293,2122,986);

    //test function addhead 
    while(1) 
    {
      dList.addhead(CR1);
      dList.addhead(CR2);
      dList.addhead(CR3);
      dList.addhead(CR4);
      dList.addhead(CR5);
      dList.addhead(CR6);
      if(   dList.find(CR6.id)->pos != 1 
         || dList.find(CR5.id)->pos != 2 
         || dList.find(CR4.id)->pos != 3 
         || dList.find(CR3.id)->pos != 4 
         || dList.find(CR2.id)->pos != 5
         || dList.find(CR1.id)->pos != 6 )
      {
          std::cout<<"List addhead test1-------[FAIL]"<<std::endl;
          std::cout<<std::endl;
          break;
      }else
      {
         std::cout<<"List addhead test1-------[OK]"<<std::endl;
         std::cout<<std::endl;
         break;
      }  
  } 
}

void testaddtail()
{
    std::cout<<"---[List addtail Test Start]---"<<std::endl;
    std::cout<<std::endl;
    List<ControlRequest> dList(4);
    ControlRequest CR1(10,3,8362,2233,111);
    ControlRequest CR2(20,2,2341,2912,321);
    ControlRequest CR3(30,9,1213,2082,391);
    ControlRequest CR4(40,9,1293,2122,986);
    ControlRequest CR5(50,9,1293,2122,986);
    ControlRequest CR6(60,9,1293,2122,986);
  //test function addtail
    while(1) 
    {
      dList.addtail(CR1);
      dList.addtail(CR2);
      dList.addtail(CR3);
      dList.addtail(CR4);
      dList.addtail(CR5);
      dList.addtail(CR6);
      if(   dList.find(CR1.id)->pos != 1 
         || dList.find(CR2.id)->pos != 2 
         || dList.find(CR3.id)->pos != 3 
         || dList.find(CR4.id)->pos != 4 
         || dList.find(CR5.id)->pos != 5
         || dList.find(CR6.id)->pos != 6 )
      {
          std::cout<<"List addtail test1-------[FAIL]"<<std::endl;
          std::cout<<std::endl;
          break;
      }else
      {
         std::cout<<"List addtail test1-------[OK]"<<std::endl;
         std::cout<<std::endl;
         break;
      }  
  } 
}

void testremove()
{
    std::cout<<"---[List remove_node Test Start]---"<<std::endl;
    std::cout<<std::endl;
    List<ControlRequest> dList(8);
    ControlRequest CR1(10,3,8362,2233,111);
    ControlRequest CR2(20,2,2341,2912,321);
    ControlRequest CR3(30,9,1213,2082,391);
    ControlRequest CR4(40,9,1293,2122,986);
    ControlRequest CR5(50,9,1293,2122,986);
    ControlRequest CR6(60,9,1293,2122,986);
  //test function remove
    while(1) 
    {
      dList.addtail(CR1);
      dList.addtail(CR2);
      dList.addtail(CR3);
      dList.addhead(CR4);
      dList.addhead(CR5);
      dList.addtail(CR6);
      dList.pop(CR3).dump();
      dList.pop(CR1).dump();
      //dList.pop(CR3);
      //dList.pop(CR1);
     // dList.pop(CR2);
    //  dList.pop(CR3);
     // dList.pop(CR4);
      break;
      #if 0
      if(   dList.find(CR1.id)->pos != 1 
         || dList.find(CR2.id)->pos != 2 
         || dList.find(CR4.id)->pos != 3
         || dList.find(CR5.id)->pos != 4
         || dList.find(CR6.id)->pos != 5 )
      {
          cout<<"List remove test1-------[FAIL]"<<endl;
          cout<<endl;
          break;
      }else
      {
         cout<<"List remove test1-------[OK]"<<endl;
         cout<<endl;
         break;
      }  
      #endif
  } 
}

void test_printfromhead_addtail()
{
    std::cout<<"---[List print_head_addtail Test Start]---"<<std::endl;
    std::cout<<std::endl;
    List<ControlRequest> dList(5);
    ControlRequest CR1(10,3,8362,2233,111);
    ControlRequest CR2(20,2,2341,2912,321);
    ControlRequest CR3(30,9,1213,2082,391);
    ControlRequest CR4(40,9,1293,2122,986);
    ControlRequest CR5(50,9,1293,2122,986);
    ControlRequest CR6(60,9,1293,2122,986);
    std::string sequ;
    while(1)
    {
      dList.addtail(CR1);
      dList.addtail(CR2);
      dList.addtail(CR3);
      dList.addtail(CR4);
      dList.addtail(CR5);
      dList.addtail(CR6);
      sequ = dList.printfromhead();
      std::cout<<"test_printfromhead_head_addtail()'s sequ = "<<sequ<<std::endl;
      if(sequ == "123456")
      {
          std::cout<<"List printfromhead_addtail test1-------[OK]"<<std::endl;
          std::cout<<std::endl;
          break;
      }
      else
      {
         std::cout<<"List printfromhead_addtail test1-------[FAIL]"<<std::endl;
         std::cout<<std::endl;
         break;
      } 
    }
}

void test_printfromhead_addhead()
{
    std::cout<<"---[List printfromhead_addtail Test Start]---"<<std::endl;
    std::cout<<std::endl;
    List<ControlRequest> dList(5);
    ControlRequest CR1(10,3,8362,2233,111);
    ControlRequest CR2(20,2,2341,2912,321);
    ControlRequest CR3(30,9,1213,2082,391);
    ControlRequest CR4(40,9,1293,2122,986);
    ControlRequest CR5(50,9,1293,2122,986);
    ControlRequest CR6(60,9,1293,2122,986);
    std::string sequ;
    while(1)
    {
      dList.addhead(CR1);
      dList.addhead(CR2);
      dList.addhead(CR3);
      dList.addhead(CR4);
      dList.addhead(CR5);
      dList.addhead(CR6);
      sequ = dList.printfromhead();
      std::cout<<"test_printfromhead_head_addhead's sequ = "<<sequ<<std::endl;
      if(sequ == "123456")
      {
          std::cout<<"List printfromhead_addhead test1-------[OK]"<<std::endl;
          std::cout<<std::endl;
          break;
      }
      else
      {
         std::cout<<"List printfromhead_addhead test1-------[FAIL]"<<std::endl;
         std::cout<<std::endl;
         break;
      } 
    }
}


void test_printfromtail_addtail()
{
    std::cout<<"---[List printfromtail_addtail Test Start]---"<<std::endl;
    std::cout<<std::endl;
    List<ControlRequest> dList(5);
    ControlRequest CR1(10,3,8362,2233,111);
    ControlRequest CR2(20,2,2341,2912,321);
    ControlRequest CR3(30,9,1213,2082,391);
    ControlRequest CR4(40,9,1293,2122,986);
    ControlRequest CR5(50,9,1293,2122,986);
    ControlRequest CR6(60,9,1293,2122,986);
    std::string sequ;
    while(1)
    {
      dList.addtail(CR1);
      dList.addtail(CR2);  
      dList.addtail(CR3);  
      dList.addtail(CR4);   
      dList.addtail(CR5);   
      dList.addtail(CR6);     
      sequ = dList.printfromtail();
      std::cout<<"test_printfromtail_addtail()'s sequ = "<<sequ<<std::endl;
      if(sequ == "654321")
      {
          std::cout<<"List  printfromtail_addtail test1-------[OK]"<<std::endl;
          std::cout<<std::endl;
          break;
      }
      else
      {
         std::cout<<"List printfromtail_addtail test1-------[FAIL]"<<std::endl;
         std::cout<<std::endl;
         break;
      } 
    }
}

void test_printfromtail_addhead()
{
    std::cout<<"---[List printfromtail_addhead Test Start]---"<<std::endl;
    std::cout<<std::endl;
    List<ControlRequest> dList(5);
    ControlRequest CR1(10,3,8362,2233,111);
    ControlRequest CR2(20,2,2341,2912,321);
    ControlRequest CR3(30,9,1213,2082,391);
    ControlRequest CR4(40,9,1293,2122,986);
    ControlRequest CR5(50,9,1293,2122,986);
    ControlRequest CR6(60,9,1293,2122,986);
    std::string sequ;
    while(1)
    {
      dList.addhead(CR1);
      dList.addhead(CR2);  
      dList.addhead(CR3);  
      dList.addhead(CR4);   
      dList.addhead(CR5);   
      dList.addhead(CR6);     
      sequ = dList.printfromtail();
      std::cout<<"test_printfromtail_addhead()'s sequ = "<<sequ<<std::endl;
      if(sequ == "654321")
      {
          std::cout<<"List printfromtail_addhead test1-------[OK]"<<std::endl;
          std::cout<<std::endl;
          break;
      }
      else
      {
         std::cout<<"List printfromtail_addhead test1-------[FAIL]"<<std::endl;
         std::cout<<std::endl;
         break;
      } 
    }
}

int main()
{
   // testaddhead();
   // testaddtail();
    testremove();
   // test_printfromhead_addtail();
   // test_printfromhead_addhead();   
   // test_printfromtail_addtail();
   // test_printfromtail_addhead();
   
    return 0;
}