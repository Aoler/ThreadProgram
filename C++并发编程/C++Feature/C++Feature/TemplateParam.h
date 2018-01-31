//
//  TemplateParam.h
//  C++Feature
//
//  Created by Haitao on 18/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef TemplateParam_h
#define TemplateParam_h

变参模板：就是可以使用不定数量的参数进行特化的模板。就像你接触到的变参函数一样，
printf就接受可变参数

和变参函数一样，变参部分可以在参数列表章使用省略号 ... 代表，变参模板需要在参数列
表中使用省略号：
template<typename ... ParameterPack>
class my_template
{};


即使主模板不是变参模板，模板进行部分特化的类中，也可以使用可变参数模板。

这个简单的模板只有一个参数：
template<typename FunctionType>
class packaged_task;
不过，并不是所有地方都这样定义；对于部分特化模板来说，其就像是一个“占位符”：
template<typename ReturnType,typename ... Args>
class packaged_task<ReturnType(Args...)>;


声明展示了两个变参模板的附加特性。第一个比较简单：普通模板参数(例如ReturnType)和可
变模板参数(Args)可以同时声明。第二个特性，展示了 Args... 特化类的模板参数列表中如何
使用，为了展示实例化模板中的Args的组成类型。实际上，因为这是部分特化，所以其作为
一种模式进行匹配；在列表中出现的类型(被Args捕获)都会进行实例化。参数包(parameter
                                       pack)调用可变参数Args，并且使用 Args... 作为包的扩展。
和可变参函数一样，变参部分可能什么都没有，也可能有很多类型项。例
如， std::packaged_task<my_class()> 中ReturnType参数就是my_class，并且Args参数包是空
的，不过 std::packaged_task<void(int,double,my_class&,std::string*)> 中，ReturnType为
void，并且Args列表中的类型就有：int, double, my_class&和std::string*。



A.6.1 扩展参数包
变参模板主要依靠包括扩展功能，因为不能限制有更多的类型添加到模板参数中。首先，列
表中的参数类型使用到的时候，可以使用包扩展，比如：需要给其他模板提供类型参数。
template<typename ... Params>
struct dummy
{
    std::tuple<Params...> data;
};


可以将包扩展和普通类型相结合：
template<typename ... Params>
struct dummy2
{
    std::tuple<std::string,Params...> data;
};

例如，创建使用参数包来创建元组中所有的元素，不如在元组中创建指针，或使
用 std::unique_ptr<> 指针，指向对应元素：
template<typename ... Params>
struct dummy3
{
    std::tuple<Params* ...> pointers;
    std::tuple<std::unique_ptr<Params> ...> unique_pointers;
};


可以使用包扩展的方式，对函数的参数进行声明：
template<typename ... Args>
void foo(Args ... args);

这将会创建一个新参数包args，其是一组函数参数，而非一组类型，并且这里 ... 也能像之
前一样进行扩展。例如，可以在 std::thread 的构造函数中使用，使用右值引用的方式获取函
数所有的参数(见A.1节)：
template<typename CallableType,typename ... Args>
thread::thread(CallableType&& func,Args&& ... args);


函数参数包也可以用来调用其他函数，将制定包扩展成参数列表，匹配调用的函数。如同类
型扩展一样，也可以使用某种模式对参数列表进行扩展。
例如，使用 std::forward() 以右值引用的方式来保存提供给函数的参数：

template<typename ... ArgTypes>
void bar(ArgTypes&& ... args)
{
    foo(std::forward<ArgTypes>(args)...);
}

注意一下这个例子，包扩展包括对类型包ArgTypes和函数参数包args的扩展，并且省略了其
余的表达式。
当这样调用bar函数：

int i;
bar(i,3.141,std::string("hello "));

template<>
void bar<int&,double,std::string>(
                                  int& args_1,
                                  double&& args_2,
                                  std::string&& args_3)
{
    foo(std::forward<int&>(args_1),
        std::forward<double>(args_2),
        std::forward<std::string>(args_3));
}

sizeof...(p) 就
是p参数包中所包含元素的个数

下面的函数会返回参数的数量：
template<typename ... Args>
unsigned count_args(Args ... args)
{
    return sizeof... (Args);
}


#endif /* TemplateParam_h */
