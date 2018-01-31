//
//  SafeList.h
//  SafeContainer
//
//  Created by Haitao on 4/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

//这对链表实现一个线程安全的版本，链表中每个元素都持有一个mutex，从而对链表的每一个操作至多持有当前节点和下一节点的mutex，这样锁的粒度更细了提高了并发的性能:

#ifndef SafeList_h
#define SafeList_h

#include <memory>
#include <mutex>

template<typename T>
class threadsafe_list
{
    struct node//每个节点持有一个mutex
    {
        std::mutex m;
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
        
        node():
        next()
        {}
        
        node(T const& value):
        data(std::make_shared<T>(value))
        {}
    };
    
    node head;
    
public:
    threadsafe_list()
    {}
    
    ~threadsafe_list()
    {
        remove_if([](T const&){return true;});
    }
    
    threadsafe_list(threadsafe_list const& other)=delete;
    threadsafe_list& operator=(threadsafe_list const& other)=delete;
    
    void push_front(T const& value)//从头部插入一个节点只需要锁住head
    {
        std::unique_ptr<node> new_node(new node(value));//在临界区外new，这样既可以减小临界区又可以避免临界区中抛出异常
        std::lock_guard<std::mutex> lk(head.m);
        new_node->next=std::move(head.next);//unique_ptr不能直接赋值，但可以通过reset或move
        head.next=std::move(new_node);
    }
    
    template<typename Function>
    void for_each(Function f)//针对链表中每个元素执行f
    {
        node* current=&head;
        std::unique_lock<std::mutex> lk(head.m);
        while(node* const next=current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->m);//锁住当前节点后，立即释放上一个节点
            lk.unlock();//
            f(*next->data);
            current=next;
            lk=std::move(next_lk);//向后移动，unique_lock is moveable not copyable，而lock_guard不具备移动语义，可见unique_lock比lock_guard灵活
        }
    }
    
    template<typename Predicate>
    std::shared_ptr<T> find_first_if(Predicate p)//找到链表中事谓词P返回true的第一个元素
    {
        node* current=&head;
        std::unique_lock<std::mutex> lk(head.m);
        while(node* const next=current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->m);
            lk.unlock();//拿到当前元素的锁后立即释放上一个锁
            if(p(*next->data))//谓词P返回true，那么返回该元素
            {
                return next->data;
            }
            current=next;
            lk=std::move(next_lk);
        }
        return std::shared_ptr<T>();
    }
    
    template<typename Predicate>
    void remove_if(Predicate p)//删除哪些使得谓词P返回true的元素
    {
        node* current=&head;
        std::unique_lock<std::mutex> lk(head.m);
        while(node* const next=current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->m);
            if(p(*next->data))
            {
                std::unique_ptr<node> old_next=std::move(current->next);
                current->next=std::move(next->next);//重置连接
                next_lk.unlock();//注意这里并没有对lk解锁或者重置
            }
            else
            {
                lk.unlock();
                current=next;
                lk=std::move(next_lk);
            }
        }
    }
};

#endif /* SafeList_h */
