#include "KeyboardInput.h"

#include <unordered_set>

#include "Floodlight/Utilities/WinIncLean.h"
#include "Floodlight/Utilities/Keywords.h"
#include "Floodlight/Utilities/IntTypes.h"

namespace Floodlight {

	confined std::unordered_set<Key> KeyPressedSet;
	confined std::unordered_set<Key> KeyReleasedSet;

	void
	ResetKeyboardEvents()
	{
		KeyPressedSet.clear();
		KeyReleasedSet.clear();
	}

    bool
	IsKeyPressed(Key K)
    {
		return GetKeyState((uint16)K) & 0x8000;
    }

    bool
	WasAnyKeyPressed()
	{
		return !KeyPressedSet.empty();
	}

	bool
	WasAnyKeyReleased()
	{
		return !KeyReleasedSet.empty();
	}

	bool
	WasKeyPressed(Key K)
	{
		return KeyPressedSet.find(K) != KeyPressedSet.end();
	}

	void
	SubmitKeyPressed(Key K)
	{
		KeyPressedSet.insert(K);
	}

	bool
	WasKeyReleased(Key K)
	{
		return KeyReleasedSet.find(K) != KeyReleasedSet.end();
	}

	void
	SubmitKeyReleased(Key K)
	{
		KeyReleasedSet.insert(K);
	}

}
