#include "pch_bcl.h"
#include "TrackerdPen.h"
#include "Scene.h"
#include "CameraObject.h"
#include "Pointer.h"

#include "LeapMotion.h"
#if defined(__HAS_LEAP__)
#include <Leap.h>
#endif
#include "Vicon.h"

using namespace Causality;
using namespace Math;

REGISTER_SCENE_OBJECT_IN_PARSER(tracked_pen, TrackedPen);

const float TrackedPen::TipLength = 0.0145f;

TrackedPen::TrackedPen()
{
	m_visible = false;
	m_inkingStr = 0;
	m_dragingStr = 0;
	m_erasingStr = 0;
	m_tipDirbase = -g_XMIdentityR0.v;
	//m_con_pc = this->OnParentChanged += [this](SceneObject* _this, SceneObject *oldParent)
	//{
	//	auto pCamera = dynamic_cast<SceneObject*>(this->Scene->PrimaryCamera());
	//};
}

TrackedPen::~TrackedPen()
{

}

void TrackedPen::Parse(const ParamArchive * archive)
{
	TrackedObjectControl::Parse(archive);
	GetParam(archive, "tip_dir", m_tipDirbase);
}

void TrackedPen::Update(const time_seconds & time_delta)
{
	TrackedObjectControl::Update(time_delta);
}

bool TrackedPen::UpdateFromLeapFinger(double dt)
{
	auto object = m_pRigid;
	if (!object) return false;
#if defined(__HAS_LEAP__)
	auto frame = m_pLeap->Controller().frame();
	XMMATRIX world = m_pLeap->ToWorldTransform();

	auto& hands = frame.hands();
	for (auto& hand : hands)
	{
		if (m_idx && hand.isRight() || !m_idx && hand.isLeft())
		{
			m_inkingStr = hand.pinchStrength();
			m_dragingStr = hand.grabStrength();

			auto indexFingerList = hand.fingers().fingerType(Leap::Finger::Type::TYPE_INDEX);
			auto indexFinger = *indexFingerList.begin(); //since there is only one per hand

			XMVECTOR pos = indexFinger.tipPosition().toVector3<Vector3>();
			//XMVECTOR pos = hand.palmPosition().toVector3<Vector3>();

			pos = XMVector3TransformCoord(pos, world);

			XMVECTOR xdir = hand.palmNormal().toVector3<Vector3>();
			XMVECTOR ydir = indexFinger.bone(Leap::Bone::Type::TYPE_DISTAL).direction().toVector3<Vector3>();
			//XMVECTOR ydir = hand.direction().toVector3<Vector3>();
			XMVECTOR zdir = XMVector3Cross(xdir, ydir);

			XMMATRIX rot = XMMatrixIdentity();
			rot.r[0] = xdir;
			rot.r[1] = ydir;
			rot.r[2] = zdir;

			//! HACK, since rot(world) == I
			XMVECTOR rotQ = XMQuaternionRotationMatrix(rot);

			object->SetPosition(pos);
			object->SetOrientation(rotQ);
			return true;
		}
	}
#endif
	return false;
}

bool TrackedPen::IsInking() const
{
	return (m_pVicon != nullptr && m_cursor->IsButtonDown(LButton))
		|| (m_cursor != nullptr && m_cursor->IsButtonDown(LButton))
		|| (m_pLeap != nullptr && (m_inkingStr > 0.5f && m_inkingStr > m_dragingStr && m_inkingStr > m_erasingStr));
	// Was for leap:
	//return ;
}

bool TrackedPen::IsErasing() const
{
	return m_pLeap != nullptr && m_erasingStr > 0.5f && m_erasingStr > m_dragingStr && m_erasingStr > m_inkingStr;
}

bool TrackedPen::IsDraging() const
{
	return (m_pVicon != nullptr && m_cursor->IsButtonDown(RButton))
		|| (m_cursor != nullptr && m_cursor->IsButtonDown(RButton))
		|| (m_pLeap != nullptr && (m_dragingStr > 0.5f && m_dragingStr > m_inkingStr && m_dragingStr > m_erasingStr));
}

bool TrackedPen::IsVisible() const
{
	return m_visible;
}

XMVECTOR XM_CALLCONV TrackedPen::GetTipPosition() const
{
	XMVECTOR pos = GetPosition();
	return pos;
}

XMVECTOR XM_CALLCONV TrackedPen::GetTipDirection() const
{
	XMVECTOR rot = GetOrientation();
	XMVECTOR dir = XMVector3Rotate(m_tipDirbase, rot);

	//std::cout << "dir = " << Vector3(dir) << std::endl;
	//std::cout << "orientation = " << Quaternion(rot) << std::endl;

	return dir;
}

XMDUALVECTOR XM_CALLCONV TrackedPen::GetTipRay() const
{
	XMDUALVECTOR ray;
	ray.r[0] = GetPosition();
	ray.r[1] = GetTipDirection();
	return ray;
}

