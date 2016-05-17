#include "pch_bcl.h"
#include "Pointer.h"
#include <WinUser.h>
#include "NativeWindow.h"

using namespace Causality;

IPointer::IPointer() : m_parent(nullptr), m_btnCount(1), m_btnState(0), m_type(PointerType_Unknown) {}

IPointer::~IPointer() = default;

MousePointer::MousePointer() {
	m_deltaEpsilon = 1.0f;
	m_parent = nullptr;
	m_type = PointerType_Mouse;
}

MousePointer::~MousePointer()
{

}

void MousePointer::Update(Vector4 pos, PointerButtonStates state) {
	m_delta = pos - m_pos;
	m_delta.z = pos.z;
	m_pos.x = pos.x; m_pos.y = pos.y; m_pos.z += m_pos.z;

	if (state != m_btnState)
		SignalButton();

	if (m_delta.LengthSquared() > m_deltaEpsilon)
		SignalMoved();
}

static bool is_rm_rdp_mouse(char cDeviceString[])
{
	int i;
	char cRDPString[] = "\\??\\Root#RDP_MOU#0000#";

	if (strlen(cDeviceString) < 22) {
		return 0;
	}

	for (i = 0; i < 22; i++) {
		if (cRDPString[i] != cDeviceString[i]) {
			return 0;
		}
	}

	return 1;
}


bool CursorHandler::SetWindow(IWindow *window) {
	unsigned int nInputDevices = 0, i;
	PRAWINPUTDEVICELIST pRawInputDeviceList = { 0 };

	unsigned int nSize = 1024;
	char psName[1024] = { 0 }; // A fix size buffer for storing name string

	if (m_initialized) {
		fprintf(stderr, "WARNING: rawmouse init called after initialization already completed.");
		m_initialized = true;
		return 0;
	}

	// 1st call to GetRawInputDeviceList: Pass nullptr to get the number of devices.
	if (GetRawInputDeviceList(nullptr, &nInputDevices, sizeof(RAWINPUTDEVICELIST)) != 0) {
		fprintf(stderr, "ERROR: Unable to count raw input devices.\n");
		return 0;
	}
	// Allocate the array to hold the DeviceList
	if ((pRawInputDeviceList = ((PRAWINPUTDEVICELIST)malloc(sizeof(RAWINPUTDEVICELIST) * nInputDevices))) == nullptr) {
		fprintf(stderr, "ERROR: Unable to allocate memory for raw input device list.\n");
		return 0;
	}
	// 2nd call to GetRawInputDeviceList: Pass the pointer to our DeviceList and GetRawInputDeviceList() will fill the array
	if (GetRawInputDeviceList(pRawInputDeviceList, &nInputDevices, sizeof(RAWINPUTDEVICELIST)) == -1) {
		fprintf(stderr, "ERROR: Unable to get raw input device list.\n");
		return 0;
	}

	// Loop through all devices and count the mice
	for (i = 0; i < nInputDevices; i++) {
		if (pRawInputDeviceList[i].dwType == RIM_TYPEMOUSE) {
			///* Get the device name and use it to determine if it's the RDP Terminal Services virtual device. */
			//// 2nd call to GetRawInputDeviceInfo: Pass our pointer to get the device name
			//if ((int)GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, nullptr, &nSize) < 0) {
			//	fprintf(stderr, "ERROR: Unable to get raw input device name.\n");
			//	return 0;
			//} 
			//if ((psName = (char *)malloc(sizeof(TCHAR) * nSize)) == NULL)  {
			//	fprintf(stderr, "ERROR: Unable to allocate memory for device name.\n");
			//	return 0;
			//}
			nSize = 1024;
			auto hr = (int)GetRawInputDeviceInfoA(pRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, psName, &nSize);
			if (hr < 0) {
				fprintf(stderr, "ERROR: Unable to get raw input device name.\n");
				return 0;
			}

			bool is_rdp = is_rm_rdp_mouse(psName);
			if ((!is_rdp) || m_useRdp) {
				auto mptr = new MousePointer();
				m_pointers.push_back(mptr);
				auto& device = *mptr;
				device.m_device_handle = pRawInputDeviceList[i].hDevice;
				device.m_device_name = psName;
				device.m_eventFlag = 0;
				device.m_btnState = 0;
			}
		}
	}

	// free the RAWINPUTDEVICELIST
	free(pRawInputDeviceList);

	
	auto hWnd = static_cast<Causality::NativeWindow*> (window)->Handle();

	// finally, register to recieve raw input WM_INPUT messages
	RAWINPUTDEVICE Rid = { 0x01, 0x02 , 0 , hWnd }; // Register only for mouse messages from wm_input.  
													// Register to receive the WM_INPUT message for any change in mouse (buttons, wheel, and movement will all generate the same message)
	if (!RegisterRawInputDevices(&Rid, 1, sizeof(Rid)))
	{
		fprintf(stderr, "ERROR: Unable to register raw input (2).\n");
		return false;
	}

	m_initialized = true;
	return true;
}

int CursorHandler::MouseInputDispicher(void* _input)
{
	HRAWINPUT input = static_cast<HRAWINPUT>(_input);

	unsigned int dwSize = 0;

	if (GetRawInputData(input, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER)) == -1) {
		fprintf(stderr, "ERROR: Unable to add to get size of raw input header.\n");
		return -2;
	}
	if (m_lbp.size() < dwSize) {
		m_lbp.resize(dwSize);
		fprintf(stderr, "ERROR: Unable to allocate memory for raw input header.\n");
		return -2;
	}
	if (GetRawInputData(input, RID_INPUT, m_lbp.data(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
		fprintf(stderr, "ERROR: Unable to add to get raw input header.\n");
		return -2;
	}

	if (!((RAWINPUT*)m_lbp.data())->data.mouse.usButtonFlags)
		return -1;

	auto TranslateWINPINT = [this](PRAWINPUT raw) -> int
	{
		HANDLE handle = raw->header.hDevice;
		auto flag = raw->data.mouse.usButtonFlags;

		int x = raw->data.mouse.lLastX;
		int y = raw->data.mouse.lLastY;

		auto ditr = std::find_if(m_pointers.begin(), m_pointers.end(), [handle](IPointer* ptr) {
			auto mptr = dynamic_cast<MousePointer*>(ptr);
			return mptr && mptr->DeviceHandle() == handle;
		});

		auto& mouse = *static_cast<MousePointer*>(*ditr);
		auto index = ditr - m_pointers.begin();

		mouse.m_eventFlag = RI_MOUSE_WHEEL;

		int w = mouse.WheelDelta();

		if (flag & RI_MOUSE_WHEEL)
		{
			w += static_cast<short>(raw->data.mouse.usButtonData) / WHEEL_DELTA;
		}

		auto state = mouse.ButtonStates();

		if (flag & RI_MOUSE_BUTTON_1_DOWN)	state.SetState(0, PointerButton_Down);
		if (flag & RI_MOUSE_BUTTON_1_UP)	state.SetState(0, PointerButton_Up);
		if (flag & RI_MOUSE_BUTTON_2_DOWN)	state.SetState(1, PointerButton_Down);
		if (flag & RI_MOUSE_BUTTON_2_UP)	state.SetState(1, PointerButton_Up);
		if (flag & RI_MOUSE_BUTTON_3_DOWN)	state.SetState(2, PointerButton_Down);
		if (flag & RI_MOUSE_BUTTON_3_UP)	state.SetState(2, PointerButton_Up);

		mouse.Update(Vector4(x, y, 0, w), state);

		return index << (sizeof(short)*8) | flag & (sizeof(short)*8 - 1);
	};


	auto flag = TranslateWINPINT((RAWINPUT*)m_lbp.data());

	return flag;
}

void CursorHandler::ProcessMessage(UINT umessage, WPARAM wparam, LPARAM lparam)
{
	auto state = m_primaryPtr.ButtonStates();
	Vector4 pos = m_primaryPtr.XY();
	switch (umessage)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	{
		auto key = (umessage - WM_LBUTTONDOWN) / (WM_RBUTTONDOWN - WM_LBUTTONDOWN);
		state.SetState(key, PointerButton_Down);
		break;
	}
	break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	{
		auto key = (umessage - WM_LBUTTONUP) / (WM_RBUTTONDOWN - WM_LBUTTONDOWN);
		state.SetState(key, PointerButton_Up);
		break;
	}
	break;
	//Handel the mouse(hand gesture glove) input
	case WM_MOUSEMOVE:
	{
		pos.x = LOWORD(lparam);
		pos.y = HIWORD(lparam);
		break;
	}
	case WM_MOUSEWHEEL:
	{
		pos.w = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
	}
	break;
	case WM_INPUT:
		MouseInputDispicher(reinterpret_cast<void*>(lparam));
	default:
		break;
	}

	m_primaryPtr.Update(pos, state);

}

IPointer * CoreInputs::PrimaryPointer()
{
	return nullptr;
}
