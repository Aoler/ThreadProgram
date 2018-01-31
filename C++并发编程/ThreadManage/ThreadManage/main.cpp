//
//  main.cpp
//  ThreadManage
//
//  Created by Haitao on 19/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#include <iostream>
#include <thread>
#include <algorithm>
#include <numeric>
#include <vector>
#include <future>

/*
 std::thread::hardware_concurrency() 在新版C++标准库中是一个很有用的函数。这个函数将
 返回能同时并发在一个程序中的线程数量。例如，多核系统中，返回值可以是CPU核芯的数
 量。返回值也仅仅是一个提示，当系统信息无法获取时，函数也会返回0。但是，这也无法掩
 盖这个函数对启动线程数量的帮助。
 
 
 清单2.8实现了一个并行版的 std::accumulate 。代码中将整体工作拆分成小任务交给每个线
 程去做，其中设置最小任务数，是为了避免产生太多的线程。程序可能会在操作数量为0的时
 候抛出异常
*/


//
//template<typename Iterator,typename T>
//struct accumulate_block
//{
//    void operator()(Iterator first,Iterator last,T& result)
//    {
//        result=std::accumulate(first,last,result);
//    }
//};
//template<typename Iterator,typename T>
//T parallel_accumulate(Iterator first,Iterator last,T init)
//{
//    unsigned long const length=std::distance(first,last);
//    if(!length) // 1
//        return init;
//    unsigned long const min_per_thread=25;
//    unsigned long const max_threads= (length+min_per_thread-1)/min_per_thread; // 2
//    unsigned long const hardware_threads = std::thread::hardware_concurrency(); //3
//    unsigned long const num_threads= std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
//    unsigned long const block_size=length/num_threads; // 4
//    std::vector<T> results(num_threads);
//    std::vector<std::thread> threads(num_threads-1); // 5
//    Iterator block_start=first;
//    for(unsigned long i=0; i < (num_threads-1); ++i)
//    {
//        Iterator block_end=block_start;
//        std::advance(block_end,block_size); // 6
//        threads[i]=std::thread(accumulate_block<Iterator,T>(),block_start,block_end,std::ref(results[i])); // 7
//        block_start=block_end; // 8
//    }
//    accumulate_block<Iterator,T>()(block_start,last,results[num_threads-1]); // 9
//    std::for_each(threads.begin(),threads.end(),
//                  std::mem_fn(&std::thread::join)); // 10
//    return std::accumulate(results.begin(),results.end(),init); // 11
//}


/*
 
 第一个修改就是调用accumulate_block的操作现在就是直接将结果返回，而非使用引用将结
 果存储在某个地方①。使用 std::packaged_task 和 std::future 是线程安全的，所以你可以使
 用它们来对结果进行转移。当调用 std::accumulate ②时，需要你显示传入T的默认构造函
 数，而非复用result的值，不过这只是一个小改动。
 
 下一个改动就是，不用向量来存储结果，而使用futures向量为每个新生线程存
 储 std::future<T> ③。在新线程生成循环中，首先要为accumulate_block创建一个任务
 ④。 std::packaged_task<T(Iterator,Iterator)> 声明，需要操作的两个Iterators和一个想要获
 取的T。然后，从任务中获取future⑤，再将需要处理的数据块的开始和结束信息传入⑥，让
 新线程去执行这个任务。当任务执行时，future将会获取对应的结果，以及任何抛出的异常。
 
 使用future，就不能获得到一组结果数组，所以需要将最终数据块的结果赋给一个变量进行保
 存⑦，而非对一个数组进行填槽。同样，因为需要从future中获取结果，使用简单的for循环，
 就要比使用 std::accumulate 好的多；循环从提供的初始值开始⑧，并且将每个future上的值
 进行累加⑨。如果相关任务抛出一个异常，那么异常就会被future捕捉到，并且使用get()的时
 候获取数据时，这个异常会再次抛出。最后，在返回结果给调用者之前，将最后一个数据块
 上的结果添加入结果中⑩。
 
*/

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


template<typename Iterator,typename T>
struct accumulate_block
{
    T operator()(Iterator first,Iterator last) // 1
    {
        return std::accumulate(first,last,T()); // 2
    }
};
template<typename Iterator,typename T>
T parallel_accumulate(Iterator first,Iterator last,T init)
{
    unsigned long const length=std::distance(first,last);
    if(!length)
        return init;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length+min_per_thread-1)/min_per_thread;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(hardware_threads!=0?hardware_threads:2,max_threads);
    unsigned long const block_size = length/num_threads;
    std::vector<std::future<T> > futures(num_threads-1); // 3
    std::vector<std::thread> threads(num_threads-1);
    join_threads joiner(threads);  //异常安全，保证线程退出
    
    
    Iterator block_start=first;
    for(unsigned long i=0;i<(num_threads-1);++i)
    {
        Iterator block_end=block_start;
        std::advance(block_end,block_size);
        std::packaged_task<T(Iterator,Iterator)> task((accumulate_block<Iterator,T>()));// 4
        futures[i]=std::future<T>(task.get_future()); // 5
        threads[i]=std::thread(std::move(task),block_start,block_end); // 6
        block_start=block_end;
    }
    T last_result=accumulate_block<Iterator,T>()(block_start,last); // 7
    std::for_each(threads.begin(),threads.end(),
                  std::mem_fn(&std::thread::join));
    T result=init; // 8
    for(unsigned long i=0;i<(num_threads-1);++i)
    {
        result+=futures[i].get(); // 9
    }
    result += last_result; // 10
    return result;
}

int main(int argc, const char * argv[])
{
    // insert code here...
    std::cout << "Hello, World!\n";
    std::cout << int() << std::endl;
    std::vector<int> v(90,30);
    typedef std::vector<int>::iterator m_iterator;
    int value = parallel_accumulate<m_iterator,int>(v.begin(),v.end(),0);
    std::cout << "value:" << value <<std::endl;
    
    return 0;
}
