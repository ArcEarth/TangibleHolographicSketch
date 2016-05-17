#pragma once
#ifndef NOMINMAX 
#define NOMINMAX
#endif
#include <minwindef.h>

namespace Causality
{
	class Application;

	class IAppComponent {
	public:
		virtual ~IAppComponent();
		virtual LRESULT ProcessMessage(UINT, WPARAM, LPARAM);

		void IAppComponent::Register();

		void IAppComponent::Unregister();
	};
}