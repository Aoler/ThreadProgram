//
//  AsyncAccum.h
//  ThreadManage
//
//  Created by Haitao on 25/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef AsyncAccum_h
#define AsyncAccum_h


template<typename Iterator,typename T>
T parallel_accumulate(Iterator first,Iterator last,T init)
{
    unsigned long const length=std::distance(first,last); // 1
    unsigned long const max_chunk_size=25;
    if(length<=max_chunk_size)
    {
        return std::accumulate(first,last,init); // 2
    }
    else
    {
        Iterator mid_point=first;
        std::advance(mid_point,length/2); // 3
        std::future<T> first_half_result=
        std::async(parallel_accumulate<Iterator,T>, // 4
                   first,mid_point,init);
        T second_half_result=parallel_accumulate(mid_point,last,T()); // 5
        return first_half_result.get()+second_half_result; // 6
    }
}

/*
 这个版本对数据进行递归划分，而非在预计算后对数据进行分块；因此，这个版本要比之前
 的版本简单很多，并且这个版本也是异常安全的。和之前一样，一开始要确定序列的长度①，
 如果其长度小于数据块包含数据的最大数量，那么可以直接调用 std::accumulate ②。如果元
 素的数量超出了数据块包含数据的最大数量，那么就需要找到数量中点③，将这个数据块分成
 两部分，然后再生成一个异步任务对另一半数据进行处理④。第二半的数据是通过直接的递归
 调用来处理的⑤，之后将两个块的结果加和到一起⑥。标准库能保证 std::async 的调用能够
 充分的利用硬件线程，并且不会产生线程的超额认购，一些“异步”调用是在调用get()⑥后同步
 执行的。 
*/

#endif /* AsyncAccum_h */
