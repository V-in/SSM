#include "./Helpers.h"
#include <iostream>

int main(){
	Helpers::Random a(0,1);

	std::cout << a() << std::endl;
}
