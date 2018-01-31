//
//  Delete.h
//  C++Feature
//
//  Created by Haitao on 18/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef Delete_h
#define Delete_h

/*
通常为了避免进行拷贝操作，会将拷贝构造函数和拷贝赋值操作符声明为私有成员，并且不
进行实现。如果对实例进行拷贝，将会引起编译错误；如果有其他成员函数或友元函数想要
拷贝一个实例，那将会引起链接错误(因为缺少实现)：
 */

/*
委员会提供了
更多的通用机制：可以通过添加 = delete 将一个函数声明为删除函数。
 */


class no_copies
{
public:
    no_copies(){}
    no_copies(no_copies const&) = delete;
    no_copies& operator=(no_copies const&) = delete;
};




class move_only
{
    std::unique_ptr<my_class> data;
public:
    move_only(const move_only&) = delete;
    move_only(move_only&& other):
    data(std::move(other.data))
    {}
    move_only& operator=(const move_only&) = delete;
    move_only& operator=(move_only&& other)
    {
        data=std::move(other.data);
        return *this;
    }
};
move_only m1;
move_only m2(m1); // 错误，拷贝构造声明为“已删除”
move_only m3(std::move(m1)); // OK，找到移动构造函数


#endif /* Delete_h */
