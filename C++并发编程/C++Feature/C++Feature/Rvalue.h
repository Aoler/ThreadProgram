//
//  Rvalue.h
//  C++Feature
//
//  Created by Haitao on 18/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef Rvalue_h
#define Rvalue_h

#include <vector>

/*
 A.1.1 移动语义
 右值通常都是临时的，所以可以随意修改；如果知道函数的某个参数是一个右值，就可以将
 其看作为一个临时存储或“窃取”内容，也不影响程序的正确性。这就意味着，比起拷贝右值参
 数的内容，不如移动其内容
 
 */

/*
试想，一个函数以 std::vector<int> 作为一个参数，就需要将其拷贝进来，而
不对原始的数据做任何操作。C++03/98的办法是，将这个参数作为一个左值的const引用传
入，然后做内部拷贝：
*/

void process_copy(std::vector<int> const& vec_)
{
    std::vector<int> vec(vec_);
    vec.push_back(42);
}

/*
这就允许函数能以左值或右值的形式进行传递，不过任何情况下都是通过拷贝来完成的。如
果使用右值引用版本的函数来重载这个函数，就能避免在传入右值的时候，函数会进行内部
拷贝的过程，因为可以任意的对原始值进行修改：
*/
void process_copy(std::vector<int> && vec)
{
    vec.push_back(42);
}


class X
{
private:
    int* data;
public:
    X():
    data(new int[1000000])
    {}
    ~X()
    {
        delete [] data;
    }
    X(const X& other): // 1 copy构造函数
    data(new int[1000000])
    {
        std::copy(other.data,other.data+1000000,data);
    }
    X(X&& other): // 2  移动构造函数
    data(other.data)
    {
        other.data=nullptr;
    }
};


/*
移动构造函
数允许对指针的所有权，在实例之间进行传递，并且允许 std::unique_ptr<> 像一个带有返回
值的函数一样使用——指针的转移是通过移动，而非拷贝。
如果你已经知道，某个变量在之后就不会在用到了，这时候可以选择显式的移动，你可以使
用 static_cast<X&&> 将对应变量转换为右值，或者通过调用 std::move() 函数来做这件事：
 */


X x1;
X x2=std::move(x1);
X x3=static_cast<X&&>(x2);

/*
 想要将参数值不通过拷贝，转化为本地变量或成员变量时，就可以使用这个办法；虽然右值
 引用参数绑定了右值，不过在函数内部，会当做左值来进行处理：
 
*/

void do_stuff(X&& x_)
{
    X a(x_); // 拷贝
    X b(std::move(x_)); // 移动
}

//do_stuff(X()); // ok，右值绑定到右值引用上
//X x;
//do_stuff(x); // 错误，左值不能绑定到右值引用上

/*
移动语义在线程库中用的比较广泛，无拷贝操作对数据进行转移可以作为一种优化方式，避
免对将要被销毁的变量进行额外的拷贝。在2.2节中看到，在线程中使用 std::move() 转
移 std::unique_ptr<> 得到一个新实例；在2.3节中，了解了在 std:thread 的实例间使用移动
语义，用来转移线程的所有权。
*/


/*
A.1.2 右值引用和函数模板
在使用右值引用作为函数模板的参数时，与之前的用法有些不同：如果函数模板参数以右值
引用作为一个模板参数，当对应位置提供左值的时候，模板会自动将其类型认定为左值引
用；当提供右值的时候，会当做普通数据使用。可能有些口语化，来看几个例子吧。
考虑一下下面的函数模板：
*/

//随后传入一个右值，T的类型将被推导为：
//foo(42); // foo<int>(42)
//foo(3.14159); // foo<double><3.14159>
//foo(std::string()); // foo<std::string>(std::string())
//不过，向foo传入左值的时候，T会比推导为一个左值引用：
//int i = 42;
//foo(i); // foo<int&>(i)
//因为函数参数声明为 T&& ，所以就是引用的引用，可以视为是原始的引用类型。那么foo()就
//相当于：
//foo<int&>(); // void foo<int&>(int& t);
//这就允许一个函数模板可以即接受左值，又可以接受右值参数；这种方式已经
//被 std::thread 的构造函数所使用(2.1节和2.2节)，所以能够将可调用对象移动到内部存储，
//而非当参数是右值的时候进行拷贝。



#endif /* Rvalue_h */
