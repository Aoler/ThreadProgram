//
//  main.cpp
//  ThreadPool
//
//  Created by Haitao on 25/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#include <iostream>
//#include "ThreadPoolV1.h"
#include "Accumulate.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    std::vector<int> v(90,30);
    typedef std::vector<int>::iterator m_iterator;
    int value = parallel_accumulate<m_iterator,int>(v.begin(),v.end(),0);
    std::cout << "value:" << value <<std::endl;
    
    return 0;
}
