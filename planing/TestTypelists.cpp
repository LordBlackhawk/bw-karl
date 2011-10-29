#include "typelists.h"

#include <iostream>

struct TT1 { };
struct TT2 { };
struct TT3 { };

template <class, class>
struct FUNC { };

typedef type_list<TT1, TT2, TT3> LIST;

typedef tensorlist< FUNC, LIST >::type TENSOR;

int main()
{
	std::cout << "Tensor.size: " << size<TENSOR>::value << "\n";
	std::cout << "finished.\n";
	return 0;
}