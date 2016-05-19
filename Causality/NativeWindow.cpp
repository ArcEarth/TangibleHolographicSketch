#include "pch_bcl.h"
#include "Application.h"
#include "NativeWindow.h"
#include "resource.h"

using namespace Causality;
//Window::~Window()
//{
//}

NativeWindow::NativeWindow()
{
}

void NativeWindow::Hide()
{}

void NativeWindow::Minimize() {}
void NativeWindow::Maximize() {}

bool NativeWindow::IsFullScreen() const {
	return m_FullScreen;
}

void NativeWindow::EnterFullScreen() {}
void NativeWindow::ExitFullScreen() {}

bool NativeWindow::Move(int x, int y)
{
	return MoveWindow(m_hWnd, x, y, m_Boundary.Width(), m_Boundary.Height(), TRUE);
}

//inline void NativeWindow::OnKeyDown(unsigned char key)
//{
//	switch (key)
//	{
//	case VK_LBUTTON:
//		if (ButtonStates[LButton]) return;
//		ButtonStates[LButton] = true;
//		PointerDown(PointerButtonEvent(LButton));
//		break;
//	case VK_RBUTTON:
//		if (ButtonStates[RButton]) return;
//		ButtonStates[RButton] = true;
//		PointerDown(PointerButtonEvent(RButton));
//		break;
//	case VK_MBUTTON:
//		if (ButtonStates[MButton]) return;
//		ButtonStates[MButton] = true;
//		PointerDown(PointerButtonEvent(MButton));
//		break;
//	default:
//		if (Keys[key]) return;
//		Keys[key] = true;
//		KeyDown(KeyboardEventArgs{ GetCurrentModifiers(),key });
//		break;
//	}
//}

//inline void NativeWindow::OnKeyUp(unsigned char key) {
//	switch (key)
//	{
//	case VK_LBUTTON:
//		if (!ButtonStates[LButton]) return;
//		ButtonStates[LButton] = false;
//		PointerUp(PointerButtonEvent(LButton));
//		break;
//	case VK_RBUTTON:
//		if (!ButtonStates[RButton]) return;
//		ButtonStates[RButton] = false;
//		PointerUp(PointerButtonEvent(RButton));
//		break;
//	case VK_MBUTTON:
//		if (!ButtonStates[MButton]) return;
//		ButtonStates[MButton] = false;
//		PointerUp(PointerButtonEvent(MButton));
//		break;
//	default:
//		if (!Keys[key]) return;
//		Keys[key] = false;
//		KeyUp(KeyboardEventArgs{ GetCurrentModifiers(),key });
//		break;
//	}
//}

void NativeWindow::OnResize(size_t width, size_t height)
{
	if (!SizeChanged.empty())
	{
		SizeChanged(Vector2(width, height));
	}
}

void NativeWindow::ProcessMessage(UINT umessage, WPARAM wparam, LPARAM lparam)
{
	m_cursor.ProcessMessage(umessage, wparam, lparam);
	m_keyboard.ProcessMessage(umessage, wparam, lparam);

	switch (umessage)
	{
	default:
		break;
	}
}

NativeWindow::~NativeWindow()
{
	Close();
}

shared_ptr<NativeWindow> Causality::NativeWindow::GetForCurrentView()
{
	for (auto& pr : Application::WindowsLookup)
	{
		if (!pr.second.expired() && dynamic_cast<NativeWindow*>(pr.second._Get()))
			return std::static_pointer_cast<NativeWindow>(pr.second.lock());
	}
	return nullptr;
}

void NativeWindow::Initialize(const std::string& title, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	// Get the instance of this application.
	m_hInstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_Title.assign(title.begin(), title.end());

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = Application::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_Title.c_str();
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (fullScreen)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		// Determine the resolution of the clients desktop screen.
		screenHeight = GetSystemMetrics(SM_CYSCREEN);
		screenWidth = GetSystemMetrics(SM_CXSCREEN);

		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		posX = (GetSystemMetrics(SM_CXSCREEN) - (int)screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - (int)screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_hWnd = CreateWindowEx(WS_EX_APPWINDOW, wc.lpszClassName, wc.lpszClassName,
		(fullScreen ? (WS_OVERLAPPED | WS_POPUP) : (WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME)),
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hInstance, NULL);

	Application::WindowsLookup[m_hWnd] = this->shared_from_this();

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hWnd, SW_SHOW);
	//ShowCursor(FALSE);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	auto titlebarHeight = GetSystemMetrics(SM_CYSIZE);
	//DirectX::Mouse::Get().SetWindow(m_hWnd);

	RECT bound;
	GetWindowRect(m_hWnd, &bound);
	m_Boundary.Position.x = (float)bound.left;
	m_Boundary.Position.y = (float)bound.top;
	m_Boundary.Size.x = (float)(bound.right - bound.left);
	m_Boundary.Size.y = (float)(bound.bottom - bound.top);

	return;
}

void NativeWindow::Show()
{
	ShowWindow(m_hWnd, SW_SHOW);
}

void NativeWindow::Focus()
{
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);
}

void NativeWindow::Close()
{
	//// Show the mouse cursor.
	//ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (m_FullScreen)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hWnd);

	m_hWnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_Title.c_str(), m_hInstance);
	m_hInstance = NULL;

	return;
}

void DebugConsole::Initialize(const std::string& title, unsigned int width, unsigned int height, bool fullScreen)
{
	if (!AllocConsole())
		return;
	HWND hWnd = GetConsoleWindow();
	MoveWindow(hWnd, 0, 0, width, height, TRUE);

#pragma warning( push )
#pragma warning( disable: 4996 )
	// Reopen file handles for stdin,stdout and sterr to point to
	// the newly created console
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

#pragma warning( pop )

	std::cout << "DEBUG console has been created" << std::endl;

	Application::WindowsLookup[hWnd] = shared_from_this();
	return;
}

void DebugConsole::Show()
{
}

void DebugConsole::Hide()
{
}

void DebugConsole::Focus()
{
}

void DebugConsole::Close()
{
}

void DebugConsole::Minimize()
{
}

void DebugConsole::Maximize()
{
}

bool DebugConsole::IsFullScreen() const
{
	return false;
}

void DebugConsole::EnterFullScreen()
{
}

void DebugConsole::ExitFullScreen()
{
}

void DebugConsole::OnResize(size_t width, size_t height)
{
}

bool DebugConsole::Move(int x, int y)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	return MoveWindow(hWnd, x, y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}
