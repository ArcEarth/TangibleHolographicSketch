#include "pch_bcl.h"
#include "PenModeler.h"
#include "LeapMotion.h"
#include <PrimitiveVisualizer.h>

using namespace Causality;
using namespace Devices;
using namespace Math;
using namespace DirectX::Visualizers;

class PenModeler::TrackedPen
{
public:
	TrackedPen(int idx)
	{
		m_pLeap = LeapMotion::GetForCurrentView();
		XMMATRIX world = XMMatrixTranslation(0, 0.50f, 0.0f);
		m_pLeap->SetDeviceWorldCoord(world);
		m_inkingStr = 0;
		m_dragingStr = 0;
		m_erasingStr = 0;
	}

	bool SetObjectCoordinateFromLeap(SceneObject* object, double dt)
	{
		if (!object) return false;
		auto frame = m_pLeap->Controller().frame();
		XMMATRIX world = m_pLeap->ToWorldTransform();

		auto& hands = frame.hands();
		for (auto& hand : hands)
		{
			if (hand.isRight())
			{
				m_inkingStr = hand.pinchStrength();
				m_dragingStr = hand.grabStrength();

				auto indexFingerList = hand.fingers().fingerType(Leap::Finger::Type::TYPE_INDEX);
				auto indexFinger = *indexFingerList.begin(); //since there is only one per hand


				//XMVECTOR pos = indexFinger.tipPosition().toVector3<Vector3>();
				XMVECTOR pos = hand.palmPosition().toVector3<Vector3>();

				pos = XMVector3TransformCoord(pos, world);

				XMVECTOR xdir = hand.palmNormal().toVector3<Vector3>();
				XMVECTOR ydir = hand.direction().toVector3<Vector3>();
				XMVECTOR zdir = XMVector3Cross(xdir,ydir);

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

	bool IsInking() const
	{
		return m_inkingStr > 0.5f && m_inkingStr > m_dragingStr && m_inkingStr > m_erasingStr;
	}
	bool IsErasing() const
	{
		return m_erasingStr > 0.5f && m_erasingStr > m_dragingStr && m_erasingStr > m_inkingStr;
	}
	bool IsDraging() const
	{
		return m_dragingStr > 0.5f && m_dragingStr > m_inkingStr && m_dragingStr > m_erasingStr;
	}

private:
	float m_inkingStr;
	float m_erasingStr;
	float m_dragingStr;

	sptr<LeapMotion> m_pLeap;
};

PenModeler::PenModeler(int objectIdx)
	: m_pTracker(new TrackedPen(objectIdx))
{
}

PenModeler::~PenModeler()
{
}

void PenModeler::Update(time_seconds const & time_delta)
{
	SceneObject::Update(time_delta);
	bool visible = m_pTracker->SetObjectCoordinateFromLeap(this, time_delta.count());
	m_isVisable = visible;
}

RenderFlags PenModeler::GetRenderFlags() const
{
	return RenderFlags::SpecialEffects;
}

bool PenModeler::IsVisible(const BoundingGeometry & viewFrustum) const
{
	return m_isVisable;
}

void PenModeler::Render(IRenderContext * context, IEffect * pEffect)
{
	g_PrimitiveDrawer.SetWorld(XMMatrixIdentity());
	XMVECTOR rot = GetOrientation();
	XMVECTOR yDir = XMVector3Rotate(g_XMIdentityR1.v, rot);
	XMVECTOR pos = GetPosition();
	XMVECTOR color = Colors::White.v;

	if (m_pTracker->IsInking())
		color = Colors::LimeGreen.v;
	else if(m_pTracker->IsDraging())
		color = Colors::Red.v;

	float length = 0.05f;
	float radius = 0.01f;
	//	g_PrimitiveDrawer.DrawCone(pos - (yDir * length), yDir, length, radius, color);
	g_PrimitiveDrawer.DrawSphere(pos, radius, color);
}

void XM_CALLCONV PenModeler::UpdateViewMatrix(FXMMATRIX view, CXMMATRIX projection)
{
	g_PrimitiveDrawer.SetProjection(projection);
	g_PrimitiveDrawer.SetView(view);
}
