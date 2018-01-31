//
//  Constexpr.h
//  C++Feature
//
//  Created by Haitao on 18/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef Constexpr_h
#define Constexpr_h

/*
constexpr 会对功能进行修改，当参数和函数返回类型符合要求，并且实现很简单，那么这
样的函数就能够被声明为 constexpr ，这样函数可以当做常数表达式来使用：
 */

constexpr int square(int x)
{
    return x*x;
}
int array[square(5)];

#endif /* Constexpr_h */
