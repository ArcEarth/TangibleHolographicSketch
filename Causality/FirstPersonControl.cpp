#include "pch_bcl.h"
#include "FirstPersonControl.h"
#include "Scene.h"
#include "NativeWindow.h"

using namespace Causality;

REGISTER_SCENE_OBJECT_IN_PARSER(first_person_keyboard_mouse_control, KeyboardMouseFirstPersonControl);

KeyboardMouseFirstPersonControl::KeyboardMouseFirstPersonControl()
{
	Speed = 2.0f;
	TuringSpeed = 1.0f;
	Flip_X = true;
	Flip_Y = true;
	m_directionalKeys = {'W','A','S','D'};
	m_turingBtn = MButton;
	
	ResetTarget(nullptr);

	m_conParChg = this->OnParentChanged.connect([this](SceneObject* _this, SceneObject* _oldParent)
	{ this->ResetTarget(this->Parent()); });

	m_keyboard = CoreInputs::PrimaryKeyboard();
	m_cursor = CoreInputs::PrimaryPointer();

	//if (m_keyboard)
	//	m_conkeyUp = m_keyboard->KeyUp.connect([this](const auto&args) {
	//		this->OnKeyUp(args);
	//	});

	if (m_cursor)
		m_cursorMove = m_cursor->Move += [this](const auto&args) {
			this->OnPointerMove(args);
		};
}

void KeyboardMouseFirstPersonControl::Parse(const ParamArchive * store)
{
	GetParam(store, "speed", Speed);
	GetParam(store, "turing_speed", TuringSpeed);

	GetParam(store, "flip_x", Flip_X);
	GetParam(store, "flip_y", Flip_Y);

	GetParam(store, "turn_button", m_turingBtn);

	const char* movement_keys = nullptr;
	GetParam(store, "movement_keys", movement_keys);
	if (movement_keys && strlen(movement_keys) >= 4)
		std::copy_n(movement_keys,4,m_directionalKeys.begin());
}

void KeyboardMouseFirstPersonControl::ResetTarget(IRigid * pTarget)
{
	m_pTarget = pTarget;
	if (pTarget)
	{
		using namespace DirectX;
		m_pTarget = pTarget;
		InitialOrientation = XMQuaternionIdentity();
		if (m_pTarget = pTarget)
		{
			Vector3 euler = XMQuaternionEulerAngleYawPitchRoll(pTarget->GetOrientation());;
			AddationalYaw = euler.y;
			AddationalPitch = euler.x;
			AddationalRoll = euler.z;
		} else
		{
			AddationalYaw = .0f;
			AddationalPitch = .0f;
			AddationalRoll = .0f;
		}
	}

}

void KeyboardMouseFirstPersonControl::Update(time_seconds const& time_delta)
{
	using namespace DirectX;

	XMVECTOR vel = XMVectorZero();
	if (m_keyboard->IsKeyDown(m_directionalKeys[0])) // 'W'
		vel += XMVectorSet( .0f,.0f,-1.0f,.0f);
	if (m_keyboard->IsKeyDown(m_directionalKeys[2])) // 'S'
		vel += XMVectorSet(.0f, .0f, 1.0f, .0f);
	if (m_keyboard->IsKeyDown(m_directionalKeys[1])) // 'A'
		vel += XMVectorSet(-1.0f, .0f, .0f, .0f);
	if (m_keyboard->IsKeyDown(m_directionalKeys[3])) // 'D'
		vel += XMVectorSet(1.0f, .0f, .0f, .0f);
	vel = XMVector3Normalize(vel);
	CameraVeclocity = vel;

	if (m_pTarget && CameraVeclocity.LengthSquared() > 0.01f)
	{
		XMVECTOR disp = (Speed * (float)time_delta.count()) * vel;

		disp = XMVector3Rotate(disp, m_pTarget->GetOrientation());

		m_pTarget->Move(disp);
	}
}

void KeyboardMouseFirstPersonControl::OnPointerMove(const PointerMoveEventArgs & e)
{
	using namespace DirectX;
	auto btnStates = e.Pointer->ButtonStates();

	auto state = btnStates.GetState(m_turingBtn);

	if (state == PointerButton_Pressing)
		ResetTarget(m_pTarget); // Reset Rotational states

	IsTrackingCursor = state & 0x1;

	if (!IsTrackingCursor) 
		return;

	float yawFlip = Flip_X ? -1.0f : 1.0f;
	float pitchFlip = Flip_Y ? -1.0f : 1.0f;

	auto yaw = yawFlip * e.PositionDelta.x / 1000.0f * XM_PI * TuringSpeed;
	auto pitch = pitchFlip * e.PositionDelta.y / 1000.0f * XM_PI * TuringSpeed;

	AddationalYaw += yaw;
	AddationalPitch += pitch;

	// Explicit prohibit pitch to exceed [-90,90] degrees
	AddationalPitch = std::min(AddationalPitch, XM_PIDIV2 - 0.01f);
	AddationalPitch = std::max(AddationalPitch, -XM_PIDIV2 + 0.01f);

	if (m_pTarget)
	{
		XMVECTOR extrinsic = XMQuaternionRotationRollPitchYaw(AddationalPitch, AddationalYaw, 0);
		XMVECTOR intial = InitialOrientation;
		intial = XMQuaternionMultiply(intial,extrinsic);
		m_pTarget->SetOrientation(intial);
	}

}
