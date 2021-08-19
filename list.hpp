#ifndef __List_1_4_HPP__
#define __List_1_4_HPP__

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
    int pos;
    Node(T& n):data(n),next(0),prev(0),pos(0){}
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
        T pop(T& n);
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
    std::unique_lock<std::mutex> lock(mlock);
    notFull.wait(lock,[this]{
        return ((capacity+1) <= maxCapacity);
    });
    Node<T>* p = new Node<T>(n);
    p->next = first;
    first = p;
    (last ? p->next->prev : last) = p;  
    capacity += 1;
    std::cout<<capacity<<std::endl;
    notEmpty.notify_all();
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
    std::unique_lock<std::mutex> lock(mlock);
    notFull.wait(lock,[this]{
        return ((capacity+1) <= maxCapacity);
    });
    Node<T>* p = new Node<T>(n);
    p->next = NULL;
    if(first == NULL)
    {
        p->prev = NULL;
        first = p;
#if DEBUG_LIST
        first->pos = 1;
#endif
        last = p; 
        capacity += 1;
        std::cout<<capacity<<std::endl;
        notEmpty.notify_all();
        return;
    }
    while (last->next != NULL)
    {
        last = last->next;  
    }
    last->next = p;
    p->prev = last;
#if DEBUG_LIST
    p->pos = last->pos+1;
#endif
    last = last->next; 
    capacity += 1;
    std::cout<<capacity<<std::endl;
    notEmpty.notify_all();
}

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
   // cout<<"find node in pos "<<p->pos<<endl;
   // p->data.dump();
    return p;
}

//从头部弹出节点,并返回节点数据
template<typename T>
T List<T>::pop(T& n)
{
    std::unique_lock<std::mutex> lock(mlock);
    notEmpty.wait(lock,[this]{
        return (first != NULL);
    });
    uint64_t id;
    id = n.id;
    Node<T>* p = first;
    T t = p->data;
    (p->next ? p->next->prev : last) = p->prev;
    (p->prev ? p->prev->next : first) = p->next;
#if DEBUG_LIST
    for(;p;p = p->next)
    {
        p->pos -= 1;
    } 
#endif
    delete p;
    capacity -= 1;
    notFull.notify_all();
    return t;
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
        p->data.dump();   
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
        p->data.dump();
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
