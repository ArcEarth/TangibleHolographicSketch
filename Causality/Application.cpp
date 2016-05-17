#include "pch_bcl.h"
#include "Application.h"
#include "NativeWindow.h"
#include <Keyboard.h>

namespace Causality
{
	std::map<HWND, std::weak_ptr<IWindow>> Application::WindowsLookup;
	std::unique_ptr<Application> Application::Current;

	void Application::RegisterComponent(IAppComponent *pComponent)
	{
		Components.emplace_back(pComponent);
	}

	void Application::UnregisterComponent(IAppComponent * pComponent)
	{
	}
	Application::Application()
	{
		hInstance = GetModuleHandle(NULL);
	}

	Application::~Application()
	{
	}

	int Application::Run(const std::vector<std::string>& args)
	{
		if (OnStartup(args))
		{
			while (!exitProposal)
			{
				MSG msg;
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					bool succ = OnIdle();
					if (!succ)
						exitProposal = true;
				}
			}
		}

		OnExit();
		return S_OK;
	}

	void Application::Exit()
	{
		for (auto& p : WindowsLookup)
		{
			auto pWindow = p.second.lock();
			if (pWindow)
				pWindow->Close();
		}
		PostQuitMessage(0);
	}


	LRESULT CALLBACK Application::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
	{
		//return Application::CoreWindow->MessageHandler(umessage, wparam, lparam);
		std::shared_ptr<IWindow> window = nullptr;
		auto itr = WindowsLookup.find(hwnd);
		if (itr != WindowsLookup.end() && !itr->second.expired())
			window = itr->second.lock();


		switch (umessage)
		{
		case WM_ACTIVATEAPP:
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			DirectX::Keyboard::ProcessMessage(umessage, wparam, lparam);
		}

		//DirectX::Mouse::ProcessMessage(umessage, wparam, lparam);

		switch (umessage)
		{
			// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			Current->exitProposal = true;
			return S_OK;
		}
		break;

		case WM_SIZE:
		{
			window->OnResize(LOWORD(lparam), HIWORD(lparam));
		}
		break;
		// Check if the window is being closed.
		case WM_CLOSE:
		{
			if (window)
			{
				window->Close();
				PostQuitMessage(0);
			}
			return S_OK;
		}
		break;
		//Any other messages send to the default message handler as our application won't make use of them.
		// All other messages pass to the message handler in the system class.
		default:
		{
			window->ProcessMessage(umessage, wparam, lparam);
			return DefWindowProc(hwnd, umessage, wparam, lparam);
		}

		}
		return S_OK;
	}

	void IAppComponent::Register()
	{
		Application::Current->RegisterComponent(this);
	}

	void IAppComponent::Unregister()
	{
		Application::Current->UnregisterComponent(this);
	}

}