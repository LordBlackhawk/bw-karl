#pragma once

void stackoverflow(int k);
int exfun(int* x);

class Base
{
    public:
        virtual int execute(int* x) = 0;
};

Base* getExfun();
