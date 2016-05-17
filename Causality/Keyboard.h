#pragma once
#include <minwindef.h>
#include "Events.h"

namespace Causality
{
	enum KeyModifiers
	{
		Mod_Shift = 0x001,
		Mod_Control = 0x002,
		Mod_Meta = 0x004,
		Mod_Alt = 0x008,
	};

	struct KeyboardEventArgs
	{
		unsigned Modifier;
		unsigned Key;
	};

	struct KeyboardHandler
	{
	public:
		using KeyStateType = int_fast8_t;

		KeyboardHandler()
		{
			memset(Keys, 0, std::size(Keys) * sizeof(KeyStateType));
		}

		unsigned GetCurrentModifiers() const
		{
			return (Keys[VK_CONTROL] & Mod_Control) | (Keys[VK_SHIFT] & Mod_Shift) | (Keys[VK_MENU] & Mod_Alt) | ((Keys[VK_LWIN] | Keys[VK_RWIN])& Mod_Meta);
		}

		void ProcessMessage(UINT umessage, WPARAM wparam, LPARAM lparam);

		bool IsKeyDown(unsigned short key) const { return (bool)Keys[key]; }
		bool IsKeyChanged(unsigned short key) const { return (bool)(Keys[key] >> 1); }

		Event<const KeyboardEventArgs&> KeyDown;
		Event<const KeyboardEventArgs&> KeyUp;

		KeyStateType Keys[255];
	};

	namespace CoreInputs
	{
		KeyboardHandler*	PrimaryKeyboard();
	};
}