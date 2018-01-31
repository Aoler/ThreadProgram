//
//  main.cpp
//  mutex
//
//  Created by Haitao on 2/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//


//
//创建一个互斥量std::mutex; mutex::lock()，mutex::unlock()互斥量的加锁解锁，try_lock未加锁时加锁否则不加锁返回false。
//但是不建议直接使用lock和unlock，因为这样需要在各种情形下保证lock和unlock的匹配，即使是抛出异常。标准库中采用RAII手法封装了
//mutex的类std::lock_guard在构造时lock，析构时unlock。lock_guard是个模板类。


//通常的做法是：将mutex和需要保护的数据封装在一个类里面，以避免使用全局变量,但是若这个类的某个成员函数返回了保护数据的引用或
//指针会是不安全的;同理成员函数在没有使用mutex前提下保存了数据的引用或指针，而后使用数据时将会导致安全问题。


