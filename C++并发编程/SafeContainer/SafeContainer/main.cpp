//
//  main.cpp
//  SafeContainer
//
//  Created by Haitao on 4/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#include <iostream>
#include "SafeStack.hpp"
#include "SafeQueue.hpp"
#include "NodeQueue.hpp"
#include "SafeList.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    threadsafe_stack<std::string> stack;
    threadsafe_queue<std::vector<char>> queue;
    threadsafe_list<std::string> list;
    std::cout << "Hello, World!\n";
    return 0;
}
