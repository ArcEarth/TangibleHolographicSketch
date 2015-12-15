#include "pch_bcl.h"
#include "PenModeler.h"
#include "EigenExtension.h"
#include <Geometrics\TriangleMesh.h>
#include <PrimitiveVisualizer.h>
#include <Models.h>
#include "Scene.h"
#include <iostream>
#include <ShaderEffect.h>
#include "AssetDictionary.h"
#include "Settings.h"

using namespace Causality;
using namespace Devices;
using namespace Math;
using namespace DirectX::Visualizers;
using namespace std;

REGISTER_SCENE_OBJECT_IN_PARSER(pen_modeler, PenModeler);

typedef uint32_t IndexType;
typedef VertexPositionNormalColor VertexType;
static const size_t	g_MeshBufferVertexCap = 4096;
static const size_t	g_MeshBufferIndexCap = 65536;
static float g_contactThred = 1.6f; //cm

PenModeler::PenModeler(int objectIdx)
	: m_state(None), m_target(nullptr)
{
	m_pTracker = nullptr;
	m_patches.reserve(100);
	m_extrusions.reserve(100);
	m_meshBuffers.reserve(100);
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
	m_pDevice = this->Scene->GetRenderDevice();
	m_extruMat = this->Scene->Assets().GetMaterial("default");
}

void PenModeler::OnParentChanged(SceneObject * oldParent)
{
	if (oldParent != parent())
	{
		auto pVisual = dynamic_cast<VisualObject*>(parent());
		if (pVisual)
		{
			ExtractMeshFromVisual(pVisual);
		}
	}

}

void PenModeler::ExtractMeshFromVisual(Causality::VisualObject * pVisual)
{
	using DirectX::Scene::DefaultStaticModel;
	using DirectX::Scene::DefaultSkinningModel;

	if (m_target)
	{
		delete m_target;
		m_target = nullptr;
	}

	{
		auto pModel = dynamic_cast<DefaultStaticModel*>(pVisual->RenderModel());
		if (pModel)
		{
			m_target = new MeshType;
			auto& vertics = m_target->vertices;
			auto& indices = m_target->indices;

			MeshType::VertexType vt;
			for (auto& v : pModel->Vertices)
			{
				vt.position = v.position;
				vt.normal = v.normal;
				vertics.push_back(vt);
			}
			for (auto& f : pModel->Facets)
			{
				for (int i = 0; i < 3; ++i)
					indices.push_back(f[i]);
			}
			return;
		}
	}
}

void PenModeler::SurfaceSketchBegin()
{
	m_patches.emplace_back();
	m_state = Inking;
}

void PenModeler::SrufaceSketchUpdate(XMVECTOR pos, XMVECTOR dir)
{
	bool touching = false;
	// Find closest point on mesh using pen direction
	vector<Geometrics::MeshRayIntersectionInfo> interInfos;
	m_target->intersect(pos, dir, &interInfos);

	if (interInfos.size() == 0) {
		cout << "Pen not touching; no intersections" << endl;
		return;
	}

	auto& info = interInfos[0];

	float shortestDist = info.distance;

	// this is cm
	if (shortestDist < g_contactThred) {
		// touching
		auto & patch = m_patches.back();
		//auto& curve = patch.boundry();
		patch.append(info.position, info.facet);
	}
	else {
		cout << "Pen not touching; too far, " << shortestDist << endl;
	}
}

void PenModeler::SurfaceSketchEnd()
{
	m_state = None;
	auto& patch = m_patches.back();
	auto& curve = patch.boundry();
	if (curve.empty())
	{
		m_patches.pop_back();
	}

	patch.closeLoop();

	//curve.append(curve[0]);
	//auto curveMap = Eigen::Matrix<float, -1, 4, Eigen::RowMajor>::MapAligned((float*)curve.data(), curve.size(), 4);
	//Eigen::laplacianSmooth(curveMap, 0.8);
	//curve.updateLength();

	//if (curve.size() > 100)
	//{
	//	curve.resample(100);
	//}
	//curve.smooth(0.8f, 4);
}

void PenModeler::OnAirDragBegin()
{
	if (m_patches.empty())
		return;
	m_state = Dragging;
	m_extrusions.emplace_back();
	m_meshBuffers.emplace_back(new DynamicMeshBuffer());
	auto& meshBuffer = *m_meshBuffers.back();
	meshBuffer.CreateDeviceResources<VertexType, IndexType>(m_pDevice,
		g_MeshBufferVertexCap,
		g_MeshBufferIndexCap);

	auto& extruder = m_extrusions.back();

	XMVECTOR lclPos = m_Transform.LclTranslation;
	float minDis = std::numeric_limits<float>::max();
	int idx = -1;
	for (int i = 0; i < m_patches.size(); i++)
	{
		auto& curv = m_patches[i].boundry();
		if (curv.empty()) continue;
		float distance = LineSegmentTest::Distance(lclPos, (XMFLOAT3*)curv.data(), curv.size(), sizeof(XMFLOAT4A));
		if (distance < minDis)
			minDis = distance, idx = i;
	}
	if (idx < 0)
	{
		m_state = None;
		return;
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
		extruder.triangulate(64, 32);
		UpdateMeshBuffer(extruder);
	}
}

void PenModeler::UpdateMeshBuffer(Geometrics::Extrusion & extruder)
{
	auto context = this->Scene->GetRenderContext();
	auto& vertices = extruder.mesh().vertices;
	auto& indices = extruder.mesh().indices;

	m_meshBuffers.back()->UpdateVertexBuffer(context,
		reinterpret_cast<VertexType*>(vertices.data()),
		vertices.size());

	m_meshBuffers.back()->UpdateIndexBuffer(context, indices.data(), indices.size());

}

void PenModeler::OnAirDragEnd()
{
	m_state = None;
	auto& extrusion = m_extrusions.back();
	auto& axis = extrusion.axis();
	if (axis.empty())
	{
		m_extrusions.pop_back();
	}
	else
	{
		axis.smooth(0.8f, 4);
		extrusion.triangulate(64, 32);
	}

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
		XMVECTOR dir = XMVector3Rotate(-g_XMIdentityR0.v, m_Transform.LclRotation);

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
			SrufaceSketchUpdate(pos, dir);
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
		g_PrimitiveDrawer.DrawSphere(pos - (yDir * TrackedPen::TipLength), 0.0075f, color);
		g_PrimitiveDrawer.DrawCone(pos - (yDir * length * 0.5f), yDir, length, radius, color);
		//g_PrimitiveDrawer.DrawCylinder(pos - (yDir * TrackedPen::TipLength), -yDir, length * 3, radius * 0.5, color);
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

	if (!g_DebugView)
		context->RSSetState(g_PrimitiveDrawer.GetStates()->CullNone());
	else
		context->RSSetState(g_PrimitiveDrawer.GetStates()->Wireframe());

	if (m_meshBuffers.empty())
		return;

	m_extruMat->SetupEffect(pEffect);
	auto pSkin = dynamic_cast<IEffectSkinning*>(pEffect);
	if (pSkin)
		pSkin->SetWeightsPerVertex(0);
	pEffect->Apply(context);
	for (int i = 0; i < m_meshBuffers.size(); i++) {
		m_meshBuffers[i]->Draw(context, pEffect);
	}

	//g_PrimitiveDrawer.DrawSphere(pos, radius, color);
}

void XM_CALLCONV PenModeler::UpdateViewMatrix(FXMMATRIX view, CXMMATRIX projection)
{
	g_PrimitiveDrawer.SetProjection(projection);
	g_PrimitiveDrawer.SetView(view);
}
