#include "pch_bcl.h"
#include "TrackedObjectControl.h"
#include "LeapMotion.h"

#include "Vicon.h"


using namespace Causality;
using namespace Causality::Devices;
using namespace Math;

REGISTER_SCENE_OBJECT_IN_PARSER(tracked_object, TrackedObjectControl);

bool TrackedObjectControl::UpdateFromVicon(double dt)
{
	if (m_pVicon)
	{
		auto object = m_pRigid;
		// name of the rigid body
		string viconName = object->Name + ':' + object->Name;
		// name of some marker in there will be objName:Mark1
		auto id = m_pVicon->GetRigidID(viconName);
		if (id == -1) return false;
		auto rigid = m_pVicon->GetRigid(id);
		object->SetPosition(rigid.Translation);
		object->SetOrientation(rigid.Rotation);
		return true;
	}
	return false;
}

bool TrackedObjectControl::UpdateFromLeapHand(double dt)
{
	if (!m_pRigid) return false;
	auto frame = m_pLeap->Controller().frame();
	XMMATRIX world = m_pLeap->ToWorldTransform();

	auto& hands = frame.hands();
	for (auto& hand : hands)
	{
		if (m_idx && hand.isRight() || !m_idx && hand.isLeft())
		{
			XMVECTOR pos = hand.palmPosition().toVector3<Vector3>();
			pos = XMVector3TransformCoord(pos, world);

			XMVECTOR xdir = hand.palmNormal().toVector3<Vector3>();
			XMVECTOR zdir = -hand.direction().toVector3<Vector3>();
			XMVECTOR ydir = XMVector3Cross(zdir, xdir);

			XMMATRIX rot = XMMatrixIdentity();
			rot.r[0] = xdir;
			rot.r[1] = ydir;
			rot.r[2] = zdir;

			//! HACK, since rot(world) == I
			XMVECTOR rotQ = XMQuaternionRotationMatrix(rot);

			m_pRigid->SetPosition(pos);
			m_pRigid->SetOrientation(rotQ);
			return true;
		}
	}
	return false;
}

TrackedObjectControl::TrackedObjectControl()
{
	m_pLeap = LeapMotion::GetForCurrentView();
	m_pVicon = IViconClient::GetFroCurrentView();

	if (m_pVicon && !m_pVicon->IsStreaming())
		m_pVicon.reset();

	XMMATRIX world = XMMatrixTranslation(0, 0.50f, 0.0f);
	m_pLeap->SetDeviceWorldCoord(world);
}

TrackedObjectControl::~TrackedObjectControl()
{
}

void TrackedObjectControl::Parse(const ParamArchive * archive)
{
	GetParam(archive, "index", m_idx);
}

void TrackedObjectControl::OnParentChanged(SceneObject * oldParent)
{
	m_pRigid = parent();
}

void TrackedObjectControl::Update(time_seconds const & time_delta)
{
	SceneObject::Update(time_delta);
	if (m_pVicon)
		UpdateFromVicon(time_delta.count());
	else
		UpdateFromVicon(time_delta.count());
}
