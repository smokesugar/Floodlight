#pragma once

namespace Floodlight {

	/// <summary>
	/// Base class for client applications to inherent from.
	/// </summary>
	class Application {
	public:
		// Empty ctor/dtor; behaviour is controlled externally.
		Application() {}
		virtual ~Application() {}
		// Called externally every frame
		virtual void Tick() {}
	};

	extern Application* CreateFloodlightInstance();

}