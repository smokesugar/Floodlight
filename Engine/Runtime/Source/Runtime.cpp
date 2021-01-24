#include "Runtime.h"

#include <iostream>

namespace Floodlight {

	Runtime::Runtime()
	{
		std::cout << "Ctor" << std::endl;
	}

	Runtime::~Runtime()
	{
		std::cout << "Dtor" << std::endl;
	}

	void Runtime::Tick()
	{
		std::cout << "Tick" << std::endl;
	}

}