#include "pch_bcl.h"
#include "TrackedObjectControl.h"
#include "LeapMotion.h"

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
		XMMATRIX world = XMMatrixTranslation(0, 0.50f, 0.0f);
		m_pLeap->SetDeviceWorldCoord(world);
	}

	void SetObjectCoordinateFromLeap(SceneObject* object, double dt)
	{
		if (!object) return;
		auto frame = m_pLeap->Controller().frame();
		XMMATRIX world = m_pLeap->ToWorldTransform();

		auto& hands = frame.hands();
		for (auto& hand : hands)
		{
			if (hand.isLeft() && m_idx == 0)
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
				return;
			}
		}
	}

	sptr<LeapMotion> m_pLeap;

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
	m_pImpl->SetObjectCoordinateFromLeap(parent(),time_delta.count());
}
