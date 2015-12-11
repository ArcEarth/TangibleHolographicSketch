#include "pch_bcl.h"
#include "TrackedObjectControl.h"
#include "LeapMotion.h"

#include "Vicon.h"


using namespace Causality;
using namespace Causality::Devices;
using namespace Math;

REGISTER_SCENE_OBJECT_IN_PARSER(tracked_object, TrackedObjectControl);

class TrackedObjectControl::Impl
{
public:
	Impl(int idx)
		: m_idx(idx)
	{
		m_pLeap = LeapMotion::GetForCurrentView();
		m_pVicon = IViconClient::GetFroCurrentView();
		if (m_pVicon && !m_pVicon->IsStreaming())
			m_pVicon.reset();

		XMMATRIX world = XMMatrixTranslation(0, 0.50f, 0.0f);
		m_pLeap->SetDeviceWorldCoord(world);
	}

	bool SetObjectCoordinateFromVicon(SceneObject* object, double dt)
	{
		if (m_pVicon)
		{
			auto id = m_pVicon->GetRigidID(object->Name);
			if (id == -1) return false;
			auto rigid = m_pVicon->GetRigid(id);
			object->SetPosition(rigid.Translation);
			object->SetOrientation(rigid.Rotation);
			return true;
		}
	}

	bool SetObjectCoordinateFromLeap(SceneObject* object, double dt)
	{
		if (!object) return false;
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

				object->SetPosition(pos);
				object->SetOrientation(rotQ);
				return true;
			}
		}
		return false;
	}

	sptr<LeapMotion>	m_pLeap;
	sptr<IViconClient>	m_pVicon;
	int m_idx;
};

TrackedObjectControl::TrackedObjectControl(int objectIdx)
	: m_pImpl(new Impl(objectIdx))
{
}

TrackedObjectControl::~TrackedObjectControl()
{
}

void TrackedObjectControl::Update(time_seconds const & time_delta)
{
	SceneObject::Update(time_delta);
	if (m_pImpl->m_pVicon)
		m_pImpl->SetObjectCoordinateFromVicon(parent(), time_delta.count());
	else
		m_pImpl->SetObjectCoordinateFromLeap(parent(), time_delta.count());
}
