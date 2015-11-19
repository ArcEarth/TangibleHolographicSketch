#include "pch_bcl.h"
#include "PenModeler.h"
#include "LeapMotion.h"
#include "EigenExtension.h"
#include <PrimitiveVisualizer.h>

using namespace Causality;
using namespace Devices;
using namespace Math;
using namespace DirectX::Visualizers;

REGISTER_SCENE_OBJECT_IN_PARSER(pen_modeler, PenModeler);

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

				XMVECTOR pos = indexFinger.tipPosition().toVector3<Vector3>();
				//XMVECTOR pos = hand.palmPosition().toVector3<Vector3>();

				pos = XMVector3TransformCoord(pos, world);

				XMVECTOR xdir = hand.palmNormal().toVector3<Vector3>();
				XMVECTOR ydir = indexFinger.bone(Leap::Bone::Type::TYPE_DISTAL).direction().toVector3<Vector3>();
				//XMVECTOR ydir = hand.direction().toVector3<Vector3>();
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
	: m_pTracker(new TrackedPen(objectIdx)), m_state(None)
{
}

PenModeler::~PenModeler()
{
}

void PenModeler::SurfaceSketchBegin()
{
	m_curves.emplace_back();
	m_state = Inking;

}

void PenModeler::SrufaceSketchUpdate(FXMVECTOR pos)
{
	auto& curve = m_curves.back();
	curve.push_back(pos);
}

void PenModeler::SurfaceSketchEnd()
{
	auto& curve = m_curves.back();
	curve.push_back(curve[0]);
	auto curveMap = Eigen::Matrix<float, -1, 4, Eigen::RowMajor>::MapAligned((float*)curve.data(), curve.size(), 4);
	Eigen::laplacianSmooth(curveMap, 0.8);
	curve.Update();
	curve.FixIntervalSampling(100);
	m_state = None;
}

void PenModeler::OnAirDragBegin()
{
	if (m_curves.empty())
		return;
	m_state = Dragging;
	m_extrusions.emplace_back();
	auto& extruder = m_extrusions.back();

	XMVECTOR lclPos = m_Transform.LclTranslation;
	float minDis = std::numeric_limits<float>::max();
	int idx;
	for (int i = 0; i < m_curves.size(); i++)
	{
		auto& curv = m_curves[i];
		float distance = LineSegmentTest::Distance(lclPos, (XMFLOAT3*)curv.data(), curv.size(), sizeof(XMFLOAT4A));
		if (distance < minDis)
			minDis = distance, idx = i;
	}
	extruder.curveFrom = m_curves[idx];
}

void PenModeler::OnAirDragUpdate(FXMVECTOR pos)
{
	auto& extruder = m_extrusions.back();
	extruder.extrusionPath.push_back(pos);
}

void PenModeler::OnAirDragEnd()
{
	m_state = None;
}

void PenModeler::Update(time_seconds const & time_delta)
{
	SceneObject::Update(time_delta);
	bool visible = m_pTracker->SetObjectCoordinateFromLeap(this, time_delta.count());
	m_isVisable = visible;
	if (m_isVisable)
	{
		XMVECTOR pos = m_Transform.LclTranslation;//GetPosition();
		
		if (m_state == None && m_pTracker->IsInking())
		{
			SurfaceSketchBegin();
		}
		else if (m_state == Inking && !m_pTracker->IsInking())
		{
			SurfaceSketchEnd();
		}
		else if (m_state == None && m_pTracker->IsDraging())
		{
			OnAirDragBegin();
		}
		else if (m_state == Dragging && !m_pTracker->IsDraging())
		{
			OnAirDragEnd();
		}

		switch (m_state)
		{
		case PenModeler::Inking:
			SrufaceSketchUpdate(pos);
			break;
		case PenModeler::Dragging:
			OnAirDragUpdate(pos);
			break;
		case PenModeler::None:
		case PenModeler::Erasing:
		default:
			break;
		}

	}

}

RenderFlags PenModeler::GetRenderFlags() const
{
	return RenderFlags::SpecialEffects;
}

bool PenModeler::IsVisible(const BoundingGeometry & viewFrustum) const
{
	return true;
}

void DrawCurve(const SpaceCurve& curve, FXMVECTOR color)
{
	for (int i = 1; i < curve.size(); i++)
	{
		XMVECTOR p0 = curve[i - 1];
		XMVECTOR p1 = curve[i];
		p0 = XMVectorSetW(p0, 1.0f);
		g_PrimitiveDrawer.DrawLine(p0, p1, color);
	}
}


void PenModeler::Render(IRenderContext * context, IEffect * pEffect)
{
	if (m_isVisable)
	{
		g_PrimitiveDrawer.SetWorld(XMMatrixIdentity());
		XMVECTOR rot = GetOrientation();
		XMVECTOR yDir = XMVector3Rotate(g_XMIdentityR1.v, rot);
		XMVECTOR pos = GetPosition();
		XMVECTOR color = Colors::White.v;

		if (m_pTracker->IsInking())
			color = Colors::LimeGreen.v;
		else if (m_pTracker->IsDraging())
			color = Colors::Red.v;

		float length = 0.05f;
		float radius = 0.01f;
		g_PrimitiveDrawer.DrawCone(pos - (yDir * length), yDir, length, radius, color);
	}

	if (m_curves.empty())
		return;
	g_PrimitiveDrawer.SetWorld(parent()->GetGlobalTransform().TransformMatrix());
	g_PrimitiveDrawer.Begin();
	for (auto& trajectory : m_curves)
	{
		DrawCurve(trajectory, Colors::LimeGreen.v);
	}
	for (auto& extruder : m_extrusions)
	{
		DrawCurve(extruder.extrusionPath, Colors::Red.v);
		DrawCurve(extruder.curveFrom, Colors::Yellow.v);
	}

	g_PrimitiveDrawer.End();
	//g_PrimitiveDrawer.DrawSphere(pos, radius, color);
}

void XM_CALLCONV PenModeler::UpdateViewMatrix(FXMMATRIX view, CXMMATRIX projection)
{
	g_PrimitiveDrawer.SetProjection(projection);
	g_PrimitiveDrawer.SetView(view);
}
