#pragma once
#include "NativeWindow.h"
#include "Renderable.h"
#include "Scene.h"
#include "Events.h"
#include "SmartPointers.h"
#include <filesystem>
#include <StepTimer.h>
#include <DeviceResources.h>

namespace DirectX
{

}

//extern std::unique_ptr<Causality::DXAppMain> m_main;

namespace Causality
{
	using std::tr2::sys::path;

	namespace Devices
	{
		class OculusRift;
		class LeapSensor;
		class KinectSensor;
		class IViconClient;
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
		virtual bool OnStartup(const std::vector<std::string>& args) override;
		void SetupDevices(const ParamArchive* arch);
		virtual void OnExit() override;
		virtual bool OnIdle() override;

		// Inherited via IDeviceNotify
		virtual void OnDeviceLost() override;
		virtual void OnDeviceRestored() override;
		void OnResize(Vector2 size);

		path GetResourcesDirectory() const;
		void SetResourcesDirectory(const std::wstring& dir);

		Event<const DirectX::StepTimer&> TimeElapsed;
		//void NotifyChildrenCursorButtonDown(const PointerButtonEvent&e);
	protected:
		// Devices & Resources
		path											m_assetsDir;

		// System resources
		sptr<DebugConsole>								pConsole;
		sptr<NativeWindow>								pWindow;
		sptr<DirectX::DeviceResources>					pDeviceResources;

		cptr<IRenderDevice>								pDevice;
		cptr<IRenderContext>							pContext;

		// Extern Devices
		sptr<Devices::OculusRift>						pRift;
		sptr<Devices::KinectSensor>						pKinect;
		sptr<Devices::LeapSensor>						pLeap;
		sptr<Devices::IViconClient>						pVicon;

		// Application Logic object
		std::vector<std::unique_ptr<IAppComponent>>		Components;

		// Rendering loop timer.
		DirectX::StepTimer								m_timer;

		// Should be the first thing to destroy
		std::vector<uptr<Scene>>						Scenes;

	};

}
