#pragma once
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
}