#include "pch_bcl.h"
#include "PenModeler.h"
#include "EigenExtension.h"
#include <PrimitiveVisualizer.h>
#include <Models.h>
#include "Scene.h"
#include <iostream>

using namespace Causality;
using namespace Devices;
using namespace Math;
using namespace DirectX::Visualizers;
using namespace std;

REGISTER_SCENE_OBJECT_IN_PARSER(pen_modeler, PenModeler);

typedef uint32_t IndexType;
typedef VertexPositionNormal VertexType;
static const size_t	g_MeshBufferVertexCap = 2048;
static const size_t	g_MeshBufferIndexCap = 2048;

PenModeler::PenModeler(int objectIdx)
	: m_state(None), m_target(new MeshType)
{
	m_pMeshBuffer.reset(new DynamicMeshBuffer());
	m_pTracker = nullptr;
}

PenModeler::~PenModeler()
{
}

void PenModeler::AddChild(SceneObject * child)
{
	SceneObject::AddChild(child);
	auto pPen = dynamic_cast<TrackedPen*>(child);
	if (pPen)
	{
		m_pTracker = pPen;
	}
}

void PenModeler::Parse(const ParamArchive * store)
{
	SceneObject::Parse(store);
	auto device = this->Scene->GetRenderDevice();
	m_pMeshBuffer->CreateDeviceResources<VertexType,IndexType>(device,
		g_MeshBufferVertexCap,
		g_MeshBufferIndexCap);
}

void PenModeler::OnParentChanged(SceneObject * oldParent)
{
}

void PenModeler::SurfaceSketchBegin(MeshType* surface)
{
	m_patches.emplace_back();
	m_state = Inking;

}

void PenModeler::SrufaceSketchUpdate(FXMVECTOR pos)
{
	auto& curve = m_patches.back().boundry();
	curve.append(pos);
}

void PenModeler::SurfaceSketchEnd()
{
	auto& curve = m_patches.back().boundry();
	curve.closeLoop();

	//curve.append(curve[0]);
	//auto curveMap = Eigen::Matrix<float, -1, 4, Eigen::RowMajor>::MapAligned((float*)curve.data(), curve.size(), 4);
	//Eigen::laplacianSmooth(curveMap, 0.8);
	//curve.updateLength();

	if (curve.size() > 100)
		curve.resample(100);
	m_state = None;
}

void PenModeler::OnAirDragBegin()
{
	if (m_patches.empty())
		return;
	m_state = Dragging;
	m_extrusions.emplace_back();
	auto& extruder = m_extrusions.back();

	XMVECTOR lclPos = m_Transform.LclTranslation;
	float minDis = std::numeric_limits<float>::max();
	int idx;
	for (int i = 0; i < m_patches.size(); i++)
	{
		auto& curv = m_patches[i].boundry();
		float distance = LineSegmentTest::Distance(lclPos, (XMFLOAT3*)curv.data(), curv.size(), sizeof(XMFLOAT4A));
		if (distance < minDis)
			minDis = distance, idx = i;
	}
	extruder.setBottom(&m_patches[idx]);
	extruder.setAxis(new Curve());
}

void PenModeler::OnAirDragUpdate(FXMVECTOR pos)
{
	auto& extruder = m_extrusions.back();
	extruder.axis().append(pos);

	if (extruder.axis().size() > 16)
	{
		extruder.triangulate(16, 16);
		UpdateMeshBuffer(extruder);
	}
}

void PenModeler::UpdateMeshBuffer(Geometrics::Extrusion & extruder)
{
	auto context = this->Scene->GetRenderContext();
	auto& vertices = extruder.mesh().vertices;
	auto& indices = extruder.mesh().indices;

	m_pMeshBuffer->UpdateVertexBuffer(context,
		reinterpret_cast<VertexType*>(vertices.data()),
		vertices.size());

	m_pMeshBuffer->UpdateIndexBuffer(context, indices.data(), indices.size());

}

void PenModeler::OnAirDragEnd()
{
	m_state = None;
}

void PenModeler::Update(time_seconds const & time_delta)
{
	SceneObject::Update(time_delta);
	if (!m_pTracker)
		return;

	m_pTracker->Update(time_delta);
	m_isVisable = m_pTracker->IsVisible();
	m_isVisable = true;
	if (m_isVisable)
	{
		XMVECTOR pos = XMLoadA(m_Transform.LclTranslation);

		if (m_state == None && m_pTracker->IsInking())
		{
			SurfaceSketchBegin(m_target);
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
			break;
		case PenModeler::Erasing:
		default:
			break;
		}
	}
}

//XMVECTOR Causality::PenModeler::GetDirection()
//{
//
//}

RenderFlags PenModeler::GetRenderFlags() const
{
	return RenderFlags::SpecialEffects;
}

bool PenModeler::IsVisible(const BoundingGeometry & viewFrustum) const
{
	return true;
}

void DrawCurve(const Curve& curve, FXMVECTOR color)
{
	if (curve.size() < 2) return;

	for (int i = 1; i < curve.size(); i++)
	{
		XMVECTOR p0 = curve[i - 1];
		XMVECTOR p1 = curve[i];
		g_PrimitiveDrawer.DrawLine(p0, p1, color);
	}
}


void PenModeler::Render(IRenderContext * context, IEffect * pEffect)
{
	if (m_isVisable && m_pTracker)
	{
		g_PrimitiveDrawer.SetWorld(XMMatrixIdentity());
		XMVECTOR rot = GetOrientation();
		XMVECTOR yDir = XMVector3Rotate(-g_XMIdentityR0.v, rot);
		XMVECTOR pos = GetPosition();
		XMVECTOR color = Colors::Yellow.v;

		if (m_pTracker->IsInking())
			color = Colors::LimeGreen.v;
		else if (m_pTracker->IsDraging())
			color = Colors::Red.v;

		float length = 0.05f;
		float radius = 0.01f;
		g_PrimitiveDrawer.DrawCone(pos - (yDir * length), yDir, length, radius, color);
	}

	if (m_patches.empty())
		return;
	g_PrimitiveDrawer.SetWorld(parent()->GetGlobalTransform().TransformMatrix());
	g_PrimitiveDrawer.Begin();
	for (auto& patch : m_patches)
	{
		DrawCurve(patch.boundry(), Colors::LimeGreen.v);
	}
	for (auto& extruder : m_extrusions)
	{
		DrawCurve(extruder.axis(), Colors::Red.v);
		DrawCurve(extruder.bottom().boundry(), Colors::Yellow.v);
	}

	g_PrimitiveDrawer.End();

	context->RSSetState(g_PrimitiveDrawer.GetStates()->CullNone());
	m_pMeshBuffer->Draw(context, pEffect);

	//g_PrimitiveDrawer.DrawSphere(pos, radius, color);
}

void XM_CALLCONV PenModeler::UpdateViewMatrix(FXMMATRIX view, CXMMATRIX projection)
{
	g_PrimitiveDrawer.SetProjection(projection);
	g_PrimitiveDrawer.SetView(view);
}
