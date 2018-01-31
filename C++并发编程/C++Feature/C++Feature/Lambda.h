//
//  Lambda.h
//  C++Feature
//
//  Created by Haitao on 18/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef Lambda_h
#define Lambda_h




std::vector<int> data=make_data();
std::for_each(data.begin(),data.end(),[](int i){std::cout<<i<<"\n";});


当lambda函数体中有多个return语句，就需要显式的指定返回类型。只有一个返回语句的时
候，也可以这样做，不过这样可能会让你的lambda函数体看起来更复杂。返回类型可以使用
跟在参数列表后面的箭头(->)进行设置。如果lambda函数没有任何参数，还需要包含(空)的参
数列表，这样做是为了能显式的对返回类型进行指定。对条件变量的预测可以写成下面这种
方式：
cond.wait(lk,[]()->bool{return data_ready;});

cond.wait(lk,[]()->bool{
    if(data_ready)
    {
        std::cout<<”Data ready”<<std::endl;
        return true;
    }
    else
    {
        std::cout<<”Data not ready, resuming wait”<<std::endl;
        return false;
    }
});s


A.5.1 引用本地变量的Lambda函数
lambda函数使用空的 [] (lambda introducer)就不能引用当前范围内的本地变量；其只能使用
全局变量，或将其他值以参数的形式进行传递。当想要访问一个本地变量，需要对其进行捕
获。最简单的方式就是将范围内的所有本地变量都进行捕获，使用 [=] 就可以完成这样的功
能。函数被创建的时候，就能对本地变量的副本进行访问了。
实践一下，看一下下面的例子：
std::function<int(int)> make_offseter(int offset)
{
    return [=](int j){return offset+j;};
}
当调用make_offseter时，就会通过 std::function<> 函数包装返回一个新的lambda函数体。
这个带有返回的函数添加了对参数的偏移功能。例如：


你可以选择以引用或拷
贝的方式对变量进行捕获，并且你还可以通过调整中括号中的表达式，来对特定的变量进行
显式捕获。如果想要拷贝所有变量，而非一两个，可以使用 [=] ，通过参考中括号中的符
号，对变量进行捕获。
int main()
{
    int i=1234,j=5678,k=9;
    std::function<int()> f=[=,&j,&k]{return i+j+k;};
    i=1;
    j=2;
    k=3;
    std::cout<<f()<<std::endl;
}

也可以通过默认引用方式对一些变量做引用，而对一些特别的变量进行拷贝。这种情
况下，就要使用 [&] 与拷贝符号相结合的方式对列表中的变量进行拷贝捕获

#endif /* Lambda_h */
