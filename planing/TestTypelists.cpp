#include "typelists.h"

#include <iostream>

struct TT1 { };
struct TT2 { };
struct TT3 { };

template <class, class>
struct FUNC { };

typedef TL::type_list<TT1, TT2, TT3> LIST;

typedef TL::tensorlist< FUNC, LIST >::type TENSOR;

int main()
{
	std::cout << "Tensor.size: " << TL::size<TENSOR>::value << "\n";
	std::cout << "finished.\n";
	return 0;
}