#pragma once

#include "Floodlight/Application/Application.h"

namespace Floodlight {

	class Runtime : public Application {
	public:
		Runtime();
		~Runtime();

		virtual void Tick() override;
	};

}