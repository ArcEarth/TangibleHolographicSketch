#include "pch_bcl.h"
#include "Keyboard.h"
#include "NativeWindow.h"

using namespace Causality;

void KeyboardHandler::ProcessMessage(UINT umessage, WPARAM wparam, LPARAM lparam)
{
	unsigned char key = (unsigned char)(0xFF & wparam);
	switch (umessage)
	{
		//Check if a key has been pressed on the keyboard.
	case WM_KEYDOWN:
	{
		if (Keys[key]) return;
		Keys[key] = true;
		KeyDown(KeyboardEventArgs{ GetCurrentModifiers(), key });
		break;
	}

	// Check if a key has been released on the keyboard.
	case WM_KEYUP:
	{
		if (!Keys[key]) return;
		Keys[key] = false;
		KeyUp(KeyboardEventArgs{ GetCurrentModifiers(), key });
		break;
	}
	break;
	}
}

KeyboardHandler * CoreInputs::PrimaryKeyboard()
{
	auto window = NativeWindow::GetForCurrentView();
	return window ? &window->Keyboard() : nullptr;
}
