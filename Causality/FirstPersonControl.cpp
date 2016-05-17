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
}

void KeyboardMouseFirstPersonControl::ResetTarget(IRigid * pTarget)
{
	m_pTarget = pTarget;
	if (pTarget)
	{
		m_pTarget = pTarget;
		InitialOrientation = pTarget->GetOrientation();
		AddationalYaw = 0;
		AddationalPitch = 0;
		AddationalRoll = 0;
	}

}

void KeyboardMouseFirstPersonControl::Update(time_seconds const& time_delta)
{
	using namespace DirectX;

	XMVECTOR vel = XMVectorZero();
	if (m_keyboard->IsKeyDown('W'))
		vel += XMVectorSet( .0f,.0f,-1.0f,.0f);
	if (m_keyboard->IsKeyDown('S'))
		vel += XMVectorSet(.0f, .0f, 1.0f, .0f);
	if (m_keyboard->IsKeyDown('A'))
		vel += XMVectorSet(-1.0f, .0f, .0f, .0f);
	if (m_keyboard->IsKeyDown('D'))
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

	auto state = btnStates.GetState(MButton);

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

	if (m_pTarget)
	{
		XMVECTOR extrinsic = XMQuaternionRotationRollPitchYaw(AddationalPitch, AddationalYaw, 0);
		XMVECTOR intial = InitialOrientation;
		intial = XMQuaternionMultiply(intial, extrinsic);
		m_pTarget->SetOrientation(intial);
	}

}
