#pragma once
#include "NativeWindow.h"
#include "Renderable.h"
#include "Scene.h"
#include <boost\filesystem\path.hpp>
#include "Events.h"
#include "SmartPointers.h"
#include <StepTimer.h>
#include <DeviceResources.h>

namespace DirectX
{

}

//extern std::unique_ptr<Causality::DXAppMain> m_main;

namespace Causality
{
	namespace Devices
	{
		class OculusRift;
		class LeapMotion;
		class KinectSensor;
	}

	class App : public Application, public DirectX::IDeviceNotify
	{
	public:
		static App* Current() {
			return static_cast<App*>(Application::Current.get());
		}

		App();
		~App();

		// Inherited via Application
		virtual void OnStartup(const std::vector<std::string>& args) override;
		virtual void OnExit() override;
		virtual void OnIdle() override;

		// Inherited via IDeviceNotify
		virtual void OnDeviceLost() override;
		virtual void OnDeviceRestored() override;

		boost::filesystem::path	GetResourcesDirectory() const;
		void SetResourcesDirectory(const std::wstring& dir);

		void RegisterComponent(ICursorInteractive *pComponent);
		void RegisterComponent(IKeybordInteractive *pComponent);
		void RegisterComponent(IUserHandsInteractive *pComponent);
		void RegisterComponent(IAppComponent *pComponent);
		void UnregisterComponent(IAppComponent *pComponent);
		void XM_CALLCONV RenderToView(DirectX::FXMMATRIX view, DirectX::CXMMATRIX projection);
		Event<const DirectX::StepTimer&> TimeElapsed;
		//void NotifyChildrenCursorButtonDown(const CursorButtonEvent&e);
	protected:
		// Devices & Resources
		boost::filesystem::path							ResourceDirectory;

		// System resources
		sptr<DebugConsole>								pConsole;
		sptr<NativeWindow>								pWindow;
		sptr<DirectX::DeviceResources>					pDeviceResources;

		cptr<IRenderDevice>								pDevice;
		cptr<IRenderContext>							pContext;

		// Extern Devices
		sptr<Devices::OculusRift>						pRift;
		sptr<Devices::KinectSensor>						pKinect;
		sptr<Devices::LeapMotion>						pLeap;

		// Application Logic object
		std::vector<std::unique_ptr<IAppComponent>>		Components;
		std::map<IAppComponent*, std::vector<EventConnection>> ComponentsEventRegisterations;


		// Rendering loop timer.
		DirectX::StepTimer								m_timer;

		// Should be the first thing to destroy
		std::vector<uptr<Scene>>						Scenes;

	};

}
