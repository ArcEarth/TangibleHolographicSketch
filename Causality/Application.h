#pragma once

#include <vector>
#include <map>
#include <memory>

#ifndef NOMINMAX 
#define NOMINMAX
#endif

#include "AppComponents.h"
#include <winapifamily.h>
#include <windef.h>

namespace Causality
{
	class IWindow;

	class Application
	{
	public:
		template <class TDerived>
		static int Invoke(const std::vector<std::string>& args)
		{
			Current = std::make_unique<TDerived>();
			return Current->Run(args);
		}

	public:
		Application();

		virtual ~Application();

		int Run(const std::vector<std::string>& args);

		void Exit();

		virtual bool OnStartup(const std::vector<std::string>& args) = 0;
		virtual void OnExit() = 0;
		virtual bool OnIdle() = 0;

		HINSTANCE Instance()
		{
			return hInstance;
		}

		void ProcessCompents();

		void RegisterComponent(IAppComponent *pComponent);
		void UnregisterComponent(IAppComponent *pComponent);

	public:
		static std::unique_ptr<Application> Current;
	public:
		static std::map<HWND, std::weak_ptr<IWindow>> WindowsLookup;
		static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	protected:
		HINSTANCE	hInstance;
		bool		exitProposal = false;

		// Application Logic object
		std::vector<std::unique_ptr<IAppComponent>>		Components;
	};
}