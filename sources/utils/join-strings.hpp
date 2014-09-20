#pragma once

#define MY_JOIN( X, Y ) MY_DO_JOIN( X, Y )
#define MY_DO_JOIN( X, Y ) MY_DO_JOIN2(X,Y)
#define MY_DO_JOIN2( X, Y ) X##Y
