#ifndef __List_HPP__
#define __List_HPP__

#include <iostream>
#include <string>
#include <mutex>
#include <memory>
#include <condition_variable>


template<typename T>
struct Node
{
    T data;//the data of node 
    Node *next, *prev;
 #if DEBUG_LIST   
    int pos;
    Node(T& n):data(n),next(0),prev(0),pos(0){}
#endif
    Node(T& n):data(n),next(0),prev(0){}
};

template<typename T>
class List
{
    private:
        Node<T> *first,*last;
        std::mutex mlock;
        std::condition_variable notEmpty;
        std::condition_variable notFull;
        int maxCapacity ;
        int capacity = 0;
    public:
        List(int maxCapacity);
        void addhead(T& n);
        void addtail(T& n);
        T pop();
        Node<T>* find(uint64_t id);     
        void modify(T& n,T& n1);//modify the node of certain data n by data n1
        const std::string printfromhead();
        const std::string printfromtail();
        ~List();
};

template<typename T>
List<T>::List(int maxCapacity):first(0),last(0),maxCapacity(maxCapacity){}

template<typename T>
void List<T>::addhead(T& n)
{
    std::unique_lock<std::mutex> lock(mlock,std::try_to_lock);
    notFull.wait(lock,[this]{
        return ((capacity+1) <= maxCapacity);
    });
    Node<T>* p = new Node<T>(n);
    if(first == NULL)
    {
        p->prev = NULL;
        first = p;
#if DEBUG_LIST
        first->pos = 1;
#endif
        last = p; 
        notEmpty.notify_all();
        capacity += 1;
        return;
    }
    p->next = first;
    first = p;
    (last ? p->next->prev : last) = p;  
    capacity += 1;    
#if DEBUG_LIST    
    for(p = first; p ; p = p->next)
    {
        p->pos += 1;
    }
#endif   
}

template<typename T>
void List<T>::addtail(T& n)
{
    Node<T>* p = new Node<T>(n);
    {
        std::unique_lock<std::mutex> lock(mlock);
        notFull.wait(lock,[this]{
            return ((capacity+1) <= maxCapacity);
        });
        
        p->next = NULL;
        if(first == NULL)
        {
            
            p->prev = NULL;
            first = p;
    #if DEBUG_LIST
            first->pos = 1;
    #endif
            last = p; 
            notEmpty.notify_one();
            capacity += 1;
            return;
        }
        //while (last->next != NULL)
        //{
        //    last = last->next;  
        //}
        last->next = p;
        p->prev = last;
        //last = p;
    #if DEBUG_LIST
        p->pos = last->pos+1;
    #endif
        last = last->next; 
        capacity += 1;
    }
}

//find the node according to node's id
template<typename T>
Node<T>* List<T>::find(uint64_t id)
{
    std::unique_lock<std::mutex> lock(mlock);
    Node<T>* p = first;
    for(;p;p = p->next)
    {
        if(p->data.id == id)  
            break;
    }
    return p;
}


//pop the node from head and return the data of node
template<typename T>
T List<T>::pop()
{   
    Node<T>* p = nullptr;
    {
        std::unique_lock<std::mutex> lock(mlock);
        notEmpty.wait(lock,[this]{
            return (first != NULL);
        });
        p = first;
        T t = p->data;
        (p->next ? p->next->prev : last) = p->prev;
        (p->prev ? p->prev->next : first) = p->next;
    #if DEBUG_LIST
        for(;p;p = p->next)
        {
            p->pos -= 1;
        } 
    #endif
        if(capacity == maxCapacity)
            notFull.notify_all();
        capacity -= 1;
        return t;
    }

    if(p != nullptr) {
        delete p;
    }
}

//modify the node of certain data n by data n1
template<typename T>
void List<T>::modify(T& n,T& n1)
{
    std::unique_lock<std::mutex> lock(mlock);
    Node<T> *p = find(n);
    p->data = n1;
}


template<typename T>
const std::string List<T>::printfromhead()
{   
    std::unique_lock<std::mutex> lock(mlock);
    std::string sequ;
    for(Node<T>* p = first;p;p = p->next)
    {   
    #if DEBUG_LIST
        sequ += std::to_string(p->pos);
        std::cout<<"sequ = "<<sequ<<std::endl;
    #endif
        p->data.print();   
    }
    return sequ;
}

template<typename T>
const std::string List<T>::printfromtail()
{
    std::unique_lock<std::mutex> lock(mlock);
    std::string sequ;
    for(Node<T>* p = last;p;p = p->prev)
    {      
    #if DEBUG_LIST
        sequ += std::to_string(p->pos);
        std::cout<<"sequ = "<<sequ<<std::endl;
    #endif
        p->data.print();
    }
    return sequ;
}

template<typename T>
List<T>::~List()
{
    for(Node<T>* p;p = first;delete p)
    {
        first = first ->next;
    }
}

#endif
