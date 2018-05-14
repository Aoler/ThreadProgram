//
//  ThreadJoiner.h
//  ThreadManage
//
//  Created by Haitao on 14/5/18.
//  Copyright © 2018年 mess. All rights reserved.
//


#ifndef ThreadJoiner_h
#define ThreadJoiner_h

#include <vector>
#include <thread>

class join_threads
{
    std::vector<std::thread>& threads;
public:
    explicit join_threads(std::vector<std::thread>& threads_):
    threads(threads_)
    {}
    ~join_threads()
    {
        for(unsigned long i=0;i<threads.size();++i)
        {
            if(threads[i].joinable())
                threads[i].join();
        }
    }
};

#endif /* ThreadJoiner_h */
