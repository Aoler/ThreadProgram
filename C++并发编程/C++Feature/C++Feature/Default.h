//
//  Default.h
//  C++Feature
//
//  Created by Haitao on 18/12/17.
//  Copyright © 2017年 mess. All rights reserved.
//

#ifndef Default_h
#define Default_h


class Y
{
private:
    Y() = default; // 改变访问级别
public:
    Y(Y&) = default; // 以非const引用作为参数
    T& operator=(const Y&) = default; // 作为文档的形式，声明为默认函数
protected:
    virtual ~Y() = default; // 改变访问级别，以及添加虚函数标签
};



#endif /* Default_h */
