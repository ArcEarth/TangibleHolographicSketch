#include "pch_bcl.h"

#include <Causality\Vicon.h>
#include <Causality\LeapMotion.h>
#if defined(__HAS_LEAP__)
#include <Leap.h>
#endif
#include "TrackedObjectControl.h"
#include "Scene.h"
#include "CameraObject.h"
#include "Pointer.h"

using namespace Causality;
using namespace Causality::Devices;
using namespace Math;

REGISTER_SCENE_OBJECT_IN_PARSER(tracked_object, TrackedObjectControl);

bool TrackedObjectControl::UpdateFromVicon(double dt)
{
	m_freq = 1.0 / dt;

	if (m_pVicon)
	{
		auto object = m_pRigid;
		// name of the rigid body
		string viconName = object->Name + ':' + object->Name;
		// name of some marker in there will be objName:Mark1
		auto id = m_pVicon->GetRigidID(viconName);
		if (id == -1) return false;

		auto rigid = m_pVicon->GetRigid(id);

		RigidTransform calibrate;
		RigidTransform temp2;
		//calibrate.Translation.x = -0.0145;
		//calibrate.Translation.y = 0.0075f;
		calibrate = rigid.Inversed();
		//temp2 *= rigid;

		Vector4 cq = calibrate.Rotation;
		//std::cout << object->Name << " : translation=\"" << calibrate.Translation << "\" rotation=\"{" << cq << "}\"" << std::endl;

		RigidTransform fin = m_intrinsic;
		fin *= rigid;

		if (Vector3::Distance(rigid.Translation,Vector3::Zero) < 0.001f)
			return true;

		fin.Translation = m_posFilter.Apply(fin.Translation);
		fin.Rotation = m_rotFilter.Apply(fin.Rotation);

		object->SetPosition(fin.Translation);
		object->SetOrientation(fin.Rotation);
		return true;
	}
	return false;
}

bool TrackedObjectControl::UpdateFromLeapHand(double dt)
{
	if (!m_pRigid) return false;

#if defined(__HAS_LEAP__)
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
#endif
	return false;
}

bool TrackedObjectControl::UpdateFromCursor(double time_delta)
{
	auto viewport = this->Scene->SceneViewport();
	auto pView = this->Scene->PrimaryCamera()->GetCollisionView();
	auto view = pView->GetViewMatrix();
	auto proj = pView->GetProjectionMatrix();

	if (!m_cursor) return false;

	XMVECTOR pos = m_cursor->Position(); // X-Y-Z is mapped to X-Y-Wheel
	pos = XMVectorSetZ(pos,0.4f);
	pos = viewport.Unproject(pos,proj,view, XMMatrixIdentity());
	XMVECTOR dir = pos + view.r[3];
	std::cout << "tracker_dir = " << Vector3(dir) << std::endl;

	//dir = _DXMEXT XMVector3Normalize(dir);

	XMVECTOR rotQ = XMQuaternionRotationVectorToVector(g_XMIdentityR2.v, dir);
	m_pRigid->SetPosition(pos);
	m_pRigid->SetOrientation(rotQ);
	std::cout << "tracker_orientation = " << rotQ << std::endl;

	//pos = XMVector3Rotate(g_XMIdentityR2.v, rotQ);
	//std::cout << Vector3(dir) << Vector3(pos) << std::endl;
	//assert(XMVector3NearEqual(pos, dir, g_XMEpsilon.v * 128));
	return true;
}

TrackedObjectControl::TrackedObjectControl()
	:m_intrinsic(m_Transform.LocalTransform()), m_pLeap(nullptr)
{
	m_pRigid = nullptr;
	m_cursor = nullptr;
	m_pRigid = this;

	//m_parentChangedConnection = this->OnParentChanged.connect([this](SceneObject* _this, SceneObject* oldParent) {
	//	m_pRigid = this->Parent();
	//});

	XMMATRIX world = XMMatrixTranslation(0, 0.50f, 0.0f);

	m_pVicon = IViconClient::GetFroCurrentView();

	if (m_pVicon && !m_pVicon->IsStreaming())
		m_pVicon.reset();

#if defined(__HAS_LEAP__)
	m_pLeap = LeapSensor::GetForCurrentView();
	if (m_pLeap && m_pLeap->IsStreaming())
	{
		m_pLeap->SetDeviceWorldCoord(world);
	}
	else
		m_pLeap.reset();
#endif

	m_posFilter.SetUpdateFrequency(&m_freq);
	m_rotFilter.SetUpdateFrequency(&m_freq);
}

TrackedObjectControl::~TrackedObjectControl()
{
}

void TrackedObjectControl::Parse(const ParamArchive * archive)
{
	SceneObject::Parse(archive);
	GetParam(archive, "index", m_idx);
	m_cursor = CoreInputs::PrimaryPointersHandler()->GetPointer(m_idx);

	float tvcap = 2.0f, rvcap = 5.0f;
	GetParam(archive,"tvcap", tvcap);
	m_posFilter.SetCutoffFrequency(tvcap);
	GetParam(archive, "rvcap", rvcap);
	m_rotFilter.SetCutoffFrequency(rvcap);
}

void TrackedObjectControl::Update(time_seconds const & time_delta)
{
	SceneObject::Update(time_delta);
	if (m_pVicon)
		m_visible = UpdateFromVicon(time_delta.count());
	else if (m_pLeap) 
		m_visible = UpdateFromLeapHand(time_delta.count());
	else {
		m_visible = m_cursor ? UpdateFromCursor(time_delta.count()) : false;
	}
	
}
