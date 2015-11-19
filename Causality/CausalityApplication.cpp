#include "pch_bcl.h"
#include "CausalityApplication.h"
//#include "Content\CubeScene.h"
//#include "Content\SampleFpsTextRenderer.h"
#include <CommonStates.h>
#include <PrimitiveVisualizer.h>
#include <boost\filesystem.hpp>
#include <ppltasks.h>

#include "Kinect.h"
#include "OculusRift.h"
#include "LeapMotion.h"

using namespace Causality;
using namespace std;
using namespace DirectX;
using namespace DirectX::Scene;
using namespace boost;

//std::wstring sceneFile = L"ArSketch.xml";
std::wstring sceneFile = L"SelectorScene.xml";


IAppComponent::~IAppComponent()
{
	Unregister();
}

void IAppComponent::Register()
{
	App::Current()->RegisterComponent(this);
}

void IAppComponent::Unregister()
{
	App::Current()->UnregisterComponent(this);
}


wstring ResourcesDirectory(L"C:\\Users\\Yupeng\\Documents\\GitHub\\VR\\Causality\\Resources\\");

App::App()
{
}

App::~App()
{
	for (auto& pCom : Components)
	{
		UnregisterComponent(pCom.get());
	}
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
	OnStartup(args);
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
			OnIdle();
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

void App::OnStartup(const std::vector<std::string>& args)
{
	ResourceDirectory = filesystem::current_path() / "Resources";
	
	// Initialize Windows
	pConsole = make_shared<DebugConsole>();
	pConsole->Initialize(std::string("Ghost Trick Consle"), 1200, 800, false);
	MoveWindow(pConsole->Handle(), 2000, 100, 1200, 800, false);

	//pRift = Devices::OculusRift::GetForCurrentView();

	pWindow = make_shared<NativeWindow>();
	if (!pRift)
		pWindow->Initialize(std::string("Ghost Trick"), 1920, 1080, false);
	else
	{
		//auto res = pRift->Resoulution();
		Vector2 res = { 1920, 1080 };
		pWindow->Initialize(std::string("Ghost Trick"),(unsigned) res.x, (unsigned) res.y, false);
	}
	//bool useOvr = Devices::OculusRift::Initialize();

	// Initialize DirectX
	pDeviceResources = make_shared<DirectX::DeviceResources>();
	pDeviceResources->SetNativeWindow(pWindow->Handle());
	// Register to be notified if the Device is lost or recreated
	pDeviceResources->RegisterDeviceNotify(this);

	//return;

	pDeviceResources->GetD3DDevice()->AddRef();
	pDeviceResources->GetD3DDeviceContext()->AddRef();
	pDevice.Attach(pDeviceResources->GetD3DDevice());
	pContext.Attach(pDeviceResources->GetD3DDeviceContext());

	Visualizers::g_PrimitiveDrawer.Initialize(pContext.Get());

	// Oculus Rift
	//if (pRift)
	//{
	//	if (!pRift->InitializeGraphics(pWindow->Handle(), pDeviceResources.get()))
	//		pRift = nullptr;
	//}

	pLeap = Devices::LeapMotion::GetForCurrentView();
	pKinect = Devices::KinectSensor::GetForCurrentView();
	//auto loadingScene = new Scene;
	//Scenes.emplace_back(loadingScene);
	//loadingScene->SetRenderDeviceAndContext(pDevice, pContext);
	//loadingScene->SetCanvas(pDeviceResources->GetBackBufferRenderTarget());

	
	Scenes.emplace_back(new Scene);
	auto& selector = Scenes.back();
	selector->SetRenderDeviceAndContext(pDevice.Get(), pContext.Get());
	selector->SetCanvas(pDeviceResources->GetBackBufferRenderTarget());
	concurrency::task<void> loadScene([&]() {
		cout << "[Scene] Loading ...";
		CoInitializeEx(NULL, COINIT::COINIT_APARTMENTTHREADED);
		
		selector->LoadFromFile((ResourceDirectory / sceneFile).string());

		XMMATRIX kinectCoord = XMMatrixRigidTransform(
			XMQuaternionRotationRollPitchYaw(-XM_PI / 12.0f, XM_PI, 0), // Orientation
			XMVectorSet(0, 0.0, 1.0f, 1.0f)); // Position
		pKinect->SetDeviceCoordinate(kinectCoord);
		pKinect->Start();

		CoUninitialize();
		cout << "[Scene] Loading Finished!";
	});
}

void App::RegisterComponent(IAppComponent *pComponent)
{
	auto pCursorInteractive = pComponent->As<ICursorInteractive>();
	auto& Regs = ComponentsEventRegisterations[pComponent];
	if (pCursorInteractive)
	{
		Regs.push_back(pWindow->CursorButtonDown += MakeEventHandler(&ICursorInteractive::OnMouseButtonDown, pCursorInteractive));
		Regs.push_back(pWindow->CursorButtonUp += MakeEventHandler(&ICursorInteractive::OnMouseButtonUp, pCursorInteractive));
		Regs.push_back(pWindow->CursorMove += MakeEventHandler(&ICursorInteractive::OnMouseMove, pCursorInteractive));
	}
	auto pKeyInteractive = pComponent->As<IKeybordInteractive>();
	if (pKeyInteractive)
	{
		Regs.push_back(pWindow->KeyDown += MakeEventHandler(&IKeybordInteractive::OnKeyDown, pKeyInteractive));
		Regs.push_back(pWindow->KeyUp += MakeEventHandler(&IKeybordInteractive::OnKeyUp, pKeyInteractive));
	}
	//auto pAnimatable = pComponent->As<ITimeAnimatable>();
	//if (pAnimatable)
	//	Regs.push_back(TimeElapsed += MakeEventHandler(&ITimeAnimatable::UpdateAnimation, pAnimatable));
	auto pHands = pComponent->As<IUserHandsInteractive>();
	if (pHands && pLeap)
	{
		Regs.push_back(pLeap->HandsTracked += MakeEventHandler(&IUserHandsInteractive::OnHandsTracked, pHands));
		Regs.push_back(pLeap->HandsLost += MakeEventHandler(&IUserHandsInteractive::OnHandsTrackLost, pHands));
		Regs.push_back(pLeap->HandsMove += MakeEventHandler(&IUserHandsInteractive::OnHandsMove, pHands));
	}
	//Components.push_back(std::move(pComponent));
}

void App::UnregisterComponent(IAppComponent * pComponent)
{
	auto itr = ComponentsEventRegisterations.find(pComponent);
	if (itr != ComponentsEventRegisterations.end())
		for (auto& connection : itr->second)
			connection.disconnect();
}


void App::OnExit()
{
}

void App::OnIdle()
{
	//ComPtr<IBodyFrame> pBodyFrame;
	//HRESULT hr = pKinect->BodyFrameReader()->AcquireLatestFrame(&pBodyFrame);
	//if (SUCCEEDED(hr))
	//{
	//	IBody *pBodys[6];
	//	hr = pBodyFrame->GetAndRefreshBodyData(6, pBodys);
	//	INT64 nTime = 0;
	//	hr = pBodyFrame->get_RelativeTime(&nTime);

	//	int count = 0;
	//	while (pBodys[count] != nullptr && count < 6)
	//		count++;

	//	if (hr)
	//		for (const auto& pComponent : Components)
	//		{
	//			auto pBodyInteractive = pComponent->As<IUserBodyInteractive>();
	//			if (pBodyInteractive)
	//			{
	//				pBodyInteractive->OnBodyFrameUpdated(nTime, count, pBodys);
	//			}
	//		}
	//}

	// Processing & Distribute Extra Input
	if (pLeap)
		pLeap->PullFrame();
	//if (pKinect)
	//	pKinect->ProcessFrame();

	for (auto& pScene : Scenes)
	{
		pScene->Update();
	}

	pDeviceResources->GetBackBufferRenderTarget().Clear(pContext.Get());
	for (auto& pScene : Scenes)
	{
		pScene->Render(pContext.Get());
	}

	pDeviceResources->Present();
}

void App::OnDeviceLost()
{
}

void App::OnDeviceRestored()
{
}

boost::filesystem::path App::GetResourcesDirectory() const
{
	return ResourceDirectory.wstring();
}

void App::SetResourcesDirectory(const std::wstring & dir)
{
	ResourceDirectory = dir;
}

void XM_CALLCONV App::RenderToView(DirectX::FXMMATRIX view, DirectX::CXMMATRIX projection)
{
	//auto pContext = pDeviceResources->GetD3DDeviceContext();
	//for (auto& pScene : Components)
	//{
	//	auto pViewable = dynamic_cast<IViewable*>(pScene.get());
	//	if (pViewable)
	//	{
	//		pViewable->UpdateViewMatrix(view,projection);
	//		//pViewable->UpdateProjectionMatrix(projection);
	//	}
	//	//auto pRenderable = pScene->As<IRenderable>();
	//	//if (pRenderable)
	//	//	pRenderable->Render(pContext);
	//}
}

//inline void SampleListener::onConnect(const Leap::Controller & controller) {
//	std::cout << "Connected" << std::endl;
//}
//
//inline void SampleListener::onFrame(const Leap::Controller & controller) {
//	auto frame = controller.frame();
//	if (frame.hands().count() > PrevHandsCount)
//	{
//		PrevHandsCount = frame.hands().count();
//	}
//	else (frame.hands().count() < PrevHandsCount)
//	{
//		PrevHandsCount = frame.hands().count();
//	}
//	//std::cout << "Frame available" << std::endl;
//}

