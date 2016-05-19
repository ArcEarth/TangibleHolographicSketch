#include "pch_bcl.h"
#include "SurfaceInspectionPlanner.h"
#include "Scene.h"
#include <Models.h>
#include <VertexTypes.h>
#include <PrimitiveVisualizer.h>
#include <Eigen\Core>
#include "Cca.h"
#include <Geometrics\csg.h>
#include <GeometricPrimitive.h>
#include "Pointer.h"
#include "TrackerdPen.h"

using namespace Causality;
using namespace Causality::SurfaceInspection;
using namespace DirectX::Scene;
using namespace Causality::Math;

REGISTER_SCENE_OBJECT_IN_PARSER(surface_inspector, SurfaceInspectionPlanner);

float g_ControlPointsRaius = 0.005f;
float g_ControlPointsConnectionRadius = 0.002;
bool  g_VisualizeNormal = false;
static constexpr size_t g_DecalResolution = 1024;

namespace Causality
{
	extern bool		g_DebugView;
}

struct TeapotPatch
{
	bool mirrorZ;
	int indices[16];
};

// Static data array defines the bezier patches that make up the teapot.
extern const TeapotPatch TeapotPatches[10];

// Static array defines the control point positions that make up the teapot.
extern const DirectX::XMVECTORF32 TeapotControlPoints[127];

template <typename _VertexType, typename _IndexType>
inline std::shared_ptr<MeshBuffer> CreateMeshBuffer(IRenderDevice* pDevice,
	const Geometrics::TriangleMesh<_VertexType, _IndexType>& mesh)
{
	auto pMesh = make_shared<MeshBuffer>();
	pMesh->CreateDeviceResources(pDevice, mesh.vertices.data(), mesh.vertices.size(), mesh.indices.data(), mesh.indices.size());
	return pMesh;
}

void SurfaceInspectionPlanner::Parse(const ParamArchive * archive)
{
	using namespace DirectX::VertexTraits;
	VisualObject::Parse(archive);

	const char* cpstr = nullptr;
	std::vector<float> cps;
	const char* mesh_name = nullptr;

	m_cursor = CoreInputs::PrimaryPointer();
	m_decalBackground = Colors::Transparent.v;
	m_decalFill = Colors::LimeGreen.v;
	m_decalStroke = Colors::Black.v;
	//m_decalStroke.A(0.8f);
	//m_decalFill.A(0.8f);

	Color color = DirectX::Colors::White;

	m_cursorMaterial = std::make_shared<PhongMaterial>();
	m_cursorMaterial->Alpha = 0.5f;
	
	m_isReady = false;
	m_declDirtyFalg = 0;
	int tessellation = 9;
	GetParam(archive, "tessellation", tessellation);
	GetParam(archive, "color", color);
	float patchSize = 0.1f, margin = 0.001f, z_tolerence = 0.01;

	GetParam(archive, "patch_size", patchSize);
	GetParam(archive, "margin", margin);
	GetParam(archive, "z_tolerence", z_tolerence);
	GetParam(archive, "active_fill_color", m_decalFill);
	GetParam(archive, "stroke_color", m_decalStroke);

	//GetParamArray(archive, "control_points", cps);

	//if (cps.size() < 16 * 3)
	//	return;

	//std::copy_n(cps.data(), 3 * 16, (float*)m_patch.data());

	if (m_pRenderModel)
	{
		ExtractMeshFromModel(m_mesh, m_pRenderModel);
	}
	else
		BuildTriangleMesh(tessellation, color);

	BoundingBox bb;
	BoundingOrientedBox obb;

	using TVertex = decltype(m_mesh.vertices[0]);
	using TIndex = decltype(m_mesh.indices[0]);
	// the BoundingOrientedBox is computed from an 3x3 svd, (pca of the vertices)
	CreateBoundingBoxesFromPoints(bb, obb,
		m_mesh.vertices.size(), &m_mesh.vertices[0].position, sizeof(TVertex));

	m_mesh.build();

	auto pDevice = this->Scene->GetRenderDevice();
	auto pD2dContext = this->Scene->Get2DContext();

	{
		auto pModel = new MonolithModel();
		pModel->pMesh = CreateMeshBuffer(pDevice, m_mesh);
		pModel->SetName("whole_patch");
		pModel->BoundBox = bb;
		pModel->BoundOrientedBox = obb;
		m_decalModel.reset(pModel);
		//VisualObject::m_pRenderModel = pModel;

		m_decal = RenderableTexture2D(pDevice, g_DecalResolution, g_DecalResolution, DXGI_FORMAT_B8G8R8A8_UNORM, 1, 0, true);
		m_decal.CreateD2DBitmapView(pD2dContext);

		auto declMat = make_shared<PhongMaterial>();
		declMat->DiffuseMap = m_decal;
		declMat->UseAlphaDiscard = true;
		pModel->pMaterial = std::move(declMat);

		UpdateDecalGeometry(Scene->Get2DFactory());
	}

	m_requestCancelLoading = 0;
	//m_loadingTask = concurrency::create_task([=]() {
	//	m_mesh.build();

	//	int xsize = (int)ceilf(obb.Extents.x * 2.0f / patchSize);
	//	int ysize = (int)ceilf(obb.Extents.z * 2.0f / patchSize);
	//	xsize = 0;
	//	ysize = 0;

	//	//m_mesh.transform(XMMatrixTranslationFromVector(-XMLoad(bb.Center)));

	//	using Geometrics::csg::BSPNode;

	//	std::cout << "Building mesh BSP tree..." << std::endl;

	//	uptr<BSPNode> meshNode;
	//	try
	//	{
	//		meshNode = BSPNode::create(m_mesh);
	//	}
	//	catch (const std::exception&)
	//	{
	//		return;
	//	}

	//	if (this->m_requestCancelLoading) return;

	//	Geometrics::TriangleMesh<VertexPositionNormalTexture> cubeMesh;
	//	DirectX::GeometricPrimitive::CreateCube(cubeMesh.vertices, cubeMesh.indices, 1.0f);

	//	if (this->m_requestCancelLoading) return;

	//	float zoffset = bb.Center.y;
	//	XMVECTOR sclFactor = XMVectorSet(0.9f * patchSize, 5.0 * z_tolerence, 0.9f *  patchSize, 1.0f);
	//	for (int i = 0; i < xsize; i++)
	//	{
	//		for (int j = 0; j < ysize; j++)
	//		{
	//			float xoffset = bb.Center.x - bb.Extents.x + (i + 0.5f) * patchSize;
	//			float yoffset = bb.Center.z - bb.Extents.z + (j + 0.5f) * patchSize;

	//			XMMATRIX M = XMMatrixAffineTransformation(
	//				sclFactor, XMVectorZero(), XMQuaternionIdentity(),
	//				XMVectorSet(xoffset, zoffset, yoffset, 0.0f));

	//			cubeMesh.transform(M);

	//			if (this->m_requestCancelLoading) return;

	//			// the area of interest for this index
	//			auto cubeNode = BSPNode::create(cubeMesh);

	//			if (this->m_requestCancelLoading) return;

	//			XMVECTOR det;
	//			M = XMMatrixInverse(&det, M);
	//			cubeMesh.transform(M); // transform back

	//			std::cout << "Computing mesh insection (" << i << ',' << j << ')' << std::endl;
	//			uptr<BSPNode> nodeRet;
	//			try
	//			{
	//				nodeRet.reset(Geometrics::csg::nodeSubtract(meshNode.get(), cubeNode.get()));
	//			}
	//			catch (const std::exception&)
	//			{
	//				return;
	//			}

	//			if (this->m_requestCancelLoading) return;

	//			// the segmented mesh
	//			m_fracorizedMeshes.emplace_back();
	//			try
	//			{
	//				nodeRet->convertToMesh(m_fracorizedMeshes.back());
	//			}
	//			catch (const std::exception&)
	//			{
	//				return;
	//			}

	//			if (this->m_requestCancelLoading) return;
	//		}
	//	}

	//	//m_projMesh = m_mesh; // copy the mesh
	//	//for (auto& v : m_projMesh.vertices)
	//	//{
	//	//	XMVECTOR p = get_position(v);
	//	//	p = XMVector3Rotate(p,quat);
	//	//	set_position(v, p);
	//	//}

	//	{
	//		auto pModel = new CompositionModel();
	//		this->m_factorizeModel.reset(pModel);
	//		if (this->m_requestCancelLoading) return;

	//		auto& parts = pModel->Parts;
	//		for (auto& mesh : m_fracorizedMeshes)
	//		{
	//			parts.emplace_back();
	//			auto& part = parts.back();

	//			if (this->m_requestCancelLoading) return;

	//			part.pMesh = CreateMeshBuffer(pDevice, mesh);
	//			part.Name = "factorized_patch";

	//			if (this->m_requestCancelLoading) return;

	//			CreateBoundingBoxesFromPoints(part.BoundBox, part.BoundOrientedBox,
	//				mesh.vertices.size(), &mesh.vertices[0].position, sizeof(TVertex));

	//			if (this->m_requestCancelLoading) return;
	//		}
	//		pModel->CreateBoundingGeometry();

	//		if (this->m_requestCancelLoading) return;

	//		auto pVisual = new VisualObject();
	//		pVisual->Scene = this->Scene;
	//		pVisual->SetRenderModel(pModel);

	//		if (this->m_requestCancelLoading) return;
	//		{
	//			this->AddChild(pVisual);
	//			std::lock_guard<std::mutex>(this->Scene->ContentMutex());
	//			this->Scene->SignalCameraCache();
	//		}
	//		//VisualObject::m_pRenderModel = pModel;
	//	}

	//	m_isReady = true;
	//});
}

void SurfaceInspectionPlanner::ExtractMeshFromModel(TriangleMeshType& mesh, const IModelNode* pNode)
{
	using DirectX::Scene::DefaultStaticModel;
	using DirectX::Scene::DefaultSkinningModel;
	using namespace DirectX::VertexTraits;

	auto pModel = dynamic_cast<const DefaultStaticModel*>(pNode);
	if (pModel)
	{
		auto& vertics = mesh.vertices;
		auto& indices = mesh.indices;

		TriangleMeshType::VertexType vt;
		for (auto& v : pModel->Vertices)
		{
			convert_vertex(v, vt);
			vertics.push_back(vt);
		}
		for (auto& f : pModel->Facets)
		{
			for (int i = 0; i < 3; ++i)
				indices.push_back(f[i]);
		}
	}
}

void SurfaceInspectionPlanner::BuildTriangleMesh(int tessellation, DirectX::SimpleMath::Color &color)
{
	auto& patch = TeapotPatches[9];
	for (int i = 0; i < std::size(patch.indices); i++)
	{
		m_patch[i] = TeapotControlPoints[patch.indices[i]].v;
	}

	bool succ = Geometrics::triangluate(m_patch, m_mesh, tessellation);

	for (auto& cp : m_patch) { cp.x = -cp.x; cp.z = -cp.z; }
	succ = Geometrics::triangluate(m_patch, m_mesh, tessellation);

	for (auto& cp : m_patch) cp.x = -cp.x;
	succ = Geometrics::triangluate(m_patch, m_mesh, tessellation, true);

	for (auto& cp : m_patch) { cp.x = -cp.x; cp.z = -cp.z; }
	succ = Geometrics::triangluate(m_patch, m_mesh, tessellation, true);

	for (auto& v : m_mesh.vertices)
	{
		DirectX::VertexTraits::set_color(v, color);
	}

	m_mesh.flip();

}

SurfaceInspectionPlanner::SurfaceInspectionPlanner()
{
	m_pen = nullptr;
}

SurfaceInspectionPlanner::~SurfaceInspectionPlanner()
{
	m_requestCancelLoading = 1;

	//if (!m_loadingTask.is_done())
	//	m_loadingTask.wait();

	//m_decal.BitmapView()->Release();
	m_decal.Reset();
	//std::cout << "Decal destroyed" << std::endl;
	//auto pModel = static_cast<MonolithModel*>(m_pModel.release());
	//if (pModel) delete pModel;
}

void SurfaceInspectionPlanner::AddChild(SceneObject * child)
{
	VisualObject::AddChild(child);
	if (!m_pen)
		m_pen = child->As<TrackedPen>();
}

void SurfaceInspectionPlanner::Update(time_seconds const & time_delta)
{
}

void SurfaceInspectionPlanner::Render(IRenderContext * pContext, IEffect * pEffect)
{
	if (m_declDirtyFalg)
		DrawDecal(Scene->Get2DContext());

	VisualObject::Render(pContext, pEffect);

	m_decalModel->Render(pContext, GlobalTransformMatrix(), pEffect);

	if (m_pen)
	{
		RenderPen(pContext,pEffect);
	}

	if (g_DebugView && pEffect)
	{
		auto& drawer = DirectX::Visualizers::g_PrimitiveDrawer;
		auto world = this->GlobalTransformMatrix();
		//geo.Transform(geo, GlobalTransformMatrix());
		Color color = DirectX::Colors::LimeGreen.v;

		drawer.SetWorld(world);

		if (m_decalModel && g_VisualizeNormal)
		{
			drawer.Begin();
			using namespace DirectX::VertexTraits;
			for (auto& v : m_mesh.vertices)
			{
				XMVECTOR pv = get_position(v);
				XMVECTOR pnv = get_normal(v);
				pnv = pv + 0.01 * pnv;
				drawer.DrawLine(pv, pnv, DirectX::Colors::Red.v);

				pnv = get_tangent(v);
				pnv = pv + 0.01 * pnv;
				drawer.DrawLine(pv, pnv, color);
			}

			drawer.End();
		}

		for (int i = 0; i < 4; i++)
		{
			drawer.DrawSphere(m_patch.control_point(i, 0), g_ControlPointsRaius, color);
			drawer.DrawSphere(m_patch.control_point(i, 3), g_ControlPointsRaius, color);
		}

		for (int i = 1; i < 3; i++)
		{
			drawer.DrawSphere(m_patch.control_point(0, i), g_ControlPointsRaius, color);
			drawer.DrawSphere(m_patch.control_point(3, i), g_ControlPointsRaius, color);
		}

		for (int i = 0; i < 3; i++)
		{
			drawer.DrawCylinder(m_patch.control_point(i, 0), m_patch.control_point((i + 1) % 4, 0), g_ControlPointsConnectionRadius, color);
			drawer.DrawCylinder(m_patch.control_point(i, 3), m_patch.control_point((i + 1) % 4, 3), g_ControlPointsConnectionRadius, color);
			drawer.DrawCylinder(m_patch.control_point(0, i), m_patch.control_point(0, (i + 1) % 4), g_ControlPointsConnectionRadius, color);
			drawer.DrawCylinder(m_patch.control_point(3, i), m_patch.control_point(3, (i + 1) % 4), g_ControlPointsConnectionRadius, color);
		}

		color *= 1.2;
		color.A(1.0f);
		for (int i = 1; i < 3; i++)
		{
			drawer.DrawCylinder(m_patch.control_point(i, 0), m_patch.control_point(i, 1), g_ControlPointsConnectionRadius, color);
			drawer.DrawCylinder(m_patch.control_point(i, 1), m_patch.control_point(i, 2), g_ControlPointsConnectionRadius, color);
			drawer.DrawCylinder(m_patch.control_point(i, 2), m_patch.control_point(i, 3), g_ControlPointsConnectionRadius, color);
			drawer.DrawCylinder(m_patch.control_point(0, i), m_patch.control_point(1, i), g_ControlPointsConnectionRadius, color);
			drawer.DrawCylinder(m_patch.control_point(1, i), m_patch.control_point(2, i), g_ControlPointsConnectionRadius, color);
			drawer.DrawCylinder(m_patch.control_point(2, i), m_patch.control_point(3, i), g_ControlPointsConnectionRadius, color);
		}
	}
}

void SurfaceInspectionPlanner::RenderPen(IRenderContext * pContext, IEffect * pEffect)
{
	auto& drawer = DirectX::Visualizers::g_PrimitiveDrawer;

	XMVECTOR pos = m_pen->GetTipPosition();
	XMVECTOR dir = m_pen->GetTipDirection();

	std::cout << "dir = " << Vector3(dir) << std::endl;
	std::cout << "orientation = " << m_pen->GetOrientation() << std::endl;

	XMVECTOR color = Colors::Yellow.v;

	if (m_pen->IsInking())
		color = Colors::LimeGreen.v;
	else if (m_pen->IsDraging())
		color = Colors::Red.v;

	color = XMVectorSetW(color,0.5f);

	XMMATRIX world = this->GetGlobalTransform().TransformMatrix();
	XMMATRIX invworld = XMMatrixInverse(nullptr,world);
	float scl = this->GetGlobalTransform().Scale.x;

	pos = XMVector3Transform(pos, invworld);
	dir = XMVector3TransformNormal(dir, invworld);

	
	std::vector<Geometrics::MeshRayIntersectionInfo> intersecs;
	m_mesh.intersect(pos, dir, &intersecs);
	if (!intersecs.empty())
	{
		color = XMVectorSetW(color, 1.0f);
		pos = intersecs[0].position;
	}

	float length = 0.01f / scl;
	float radius = 0.0035f / scl;

	drawer.SetWorld(world);
	drawer.DrawSphere(pos, radius, color);
	drawer.DrawCylinder(pos, pos + dir * length, radius * 0.5, color);
}

inline D2D1_COLOR_F XM_CALLCONV GetD2DColor(const Color& color)
{
	D2D1_COLOR_F cf = reinterpret_cast<const D2D1_COLOR_F&>(color);
	return cf;
}

inline D2D1_POINT_2F XM_CALLCONV GetD2DPoint(const Vector2& v)
{
	D2D1_POINT_2F dv = reinterpret_cast<const D2D1_POINT_2F&>(v);
	return dv;
}



void SurfaceInspectionPlanner::UpdateDecalGeometry(I2DFactory* pFactory)
{
	ThrowIfFailed(pFactory->CreatePathGeometry(&m_patchGeos));
	cptr<ID2D1GeometrySink> pSink;
	ThrowIfFailed(m_patchGeos->Open(&pSink));
	pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
	pSink->BeginFigure(
		D2D1::Point2F(346, 255),
		D2D1_FIGURE_BEGIN_FILLED
	);
	D2D1_POINT_2F points[5] = {
		D2D1::Point2F(267, 177),
		D2D1::Point2F(236, 192),
		D2D1::Point2F(212, 160),
		D2D1::Point2F(156, 255),
		D2D1::Point2F(346, 255),
	};
	pSink->AddLines(points, ARRAYSIZE(points));
	pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	pSink->Close();
	pSink.Reset();

	m_declDirtyFalg = 1;
}

void SurfaceInspectionPlanner::DrawDecal(I2DContext *pContext)
{
	D2D1_COLOR_F color;
	color = { .2f,0.7f,.2f,1.0f };

	if (!m_brush)
		ThrowIfFailed(pContext->CreateSolidColorBrush(color, &m_brush));

	//color = { .0f,.0f,.0f,.0f };
	pContext->SetTarget(m_decal);
	pContext->BeginDraw();
	pContext->Clear(GetD2DColor(m_decalBackground));

	if (m_patchGeos && m_brush)
	{
		m_brush->SetColor(GetD2DColor(m_decalFill));
		pContext->FillGeometry(m_patchGeos.Get(), m_brush.Get());

		m_brush->SetColor(GetD2DColor(m_decalStroke));
		pContext->DrawGeometry(m_patchGeos.Get(), m_brush.Get(), 2.0f);
	}

	ThrowIfFailed(pContext->EndDraw());
	pContext->SetTarget(nullptr);

	m_declDirtyFalg = 0;
}

const TeapotPatch TeapotPatches[10] =
{
	// Rim.
	{ true,{ 102, 103, 104, 105, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 } },

	// Body.
	{ true,{ 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 } },
	{ true,{ 24, 25, 26, 27, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40 } },

	// Lid.
	{ true,{ 96, 96, 96, 96, 97, 98, 99, 100, 101, 101, 101, 101, 0, 1, 2, 3 } },
	{ true,{ 0, 1, 2, 3, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117 } },

	// Handle.
	{ false,{ 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56 } },
	{ false,{ 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 28, 65, 66, 67 } },

	// Spout.
	{ false,{ 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83 } },
	{ false,{ 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 } },

	// Bottom.
	{ true,{ 118, 118, 118, 118, 124, 122, 119, 121, 123, 126, 125, 120, 40, 39, 38, 37 } },
};


// Static array defines the control point positions that make up the teapot.
const DirectX::XMVECTORF32 TeapotControlPoints[127] =
{
	{ 0, 0.345f, -0.05f },
	{ -0.028f, 0.345f, -0.05f },
	{ -0.05f, 0.345f, -0.028f },
	{ -0.05f, 0.345f, -0 },
	{ 0, 0.3028125f, -0.334375f },
	{ -0.18725f, 0.3028125f, -0.334375f },
	{ -0.334375f, 0.3028125f, -0.18725f },
	{ -0.334375f, 0.3028125f, -0 },
	{ 0, 0.3028125f, -0.359375f },
	{ -0.20125f, 0.3028125f, -0.359375f },
	{ -0.359375f, 0.3028125f, -0.20125f },
	{ -0.359375f, 0.3028125f, -0 },
	{ 0, 0.27f, -0.375f },
	{ -0.21f, 0.27f, -0.375f },
	{ -0.375f, 0.27f, -0.21f },
	{ -0.375f, 0.27f, -0 },
	{ 0, 0.13875f, -0.4375f },
	{ -0.245f, 0.13875f, -0.4375f },
	{ -0.4375f, 0.13875f, -0.245f },
	{ -0.4375f, 0.13875f, -0 },
	{ 0, 0.007499993f, -0.5f },
	{ -0.28f, 0.007499993f, -0.5f },
	{ -0.5f, 0.007499993f, -0.28f },
	{ -0.5f, 0.007499993f, -0 },
	{ 0, -0.105f, -0.5f },
	{ -0.28f, -0.105f, -0.5f },
	{ -0.5f, -0.105f, -0.28f },
	{ -0.5f, -0.105f, -0 },
	{ 0, -0.105f, 0.5f },
	{ 0, -0.2175f, -0.5f },
	{ -0.28f, -0.2175f, -0.5f },
	{ -0.5f, -0.2175f, -0.28f },
	{ -0.5f, -0.2175f, -0 },
	{ 0, -0.27375f, -0.375f },
	{ -0.21f, -0.27375f, -0.375f },
	{ -0.375f, -0.27375f, -0.21f },
	{ -0.375f, -0.27375f, -0 },
	{ 0, -0.2925f, -0.375f },
	{ -0.21f, -0.2925f, -0.375f },
	{ -0.375f, -0.2925f, -0.21f },
	{ -0.375f, -0.2925f, -0 },
	{ 0, 0.17625f, 0.4f },
	{ -0.075f, 0.17625f, 0.4f },
	{ -0.075f, 0.2325f, 0.375f },
	{ 0, 0.2325f, 0.375f },
	{ 0, 0.17625f, 0.575f },
	{ -0.075f, 0.17625f, 0.575f },
	{ -0.075f, 0.2325f, 0.625f },
	{ 0, 0.2325f, 0.625f },
	{ 0, 0.17625f, 0.675f },
	{ -0.075f, 0.17625f, 0.675f },
	{ -0.075f, 0.2325f, 0.75f },
	{ 0, 0.2325f, 0.75f },
	{ 0, 0.12f, 0.675f },
	{ -0.075f, 0.12f, 0.675f },
	{ -0.075f, 0.12f, 0.75f },
	{ 0, 0.12f, 0.75f },
	{ 0, 0.06375f, 0.675f },
	{ -0.075f, 0.06375f, 0.675f },
	{ -0.075f, 0.007499993f, 0.75f },
	{ 0, 0.007499993f, 0.75f },
	{ 0, -0.04875001f, 0.625f },
	{ -0.075f, -0.04875001f, 0.625f },
	{ -0.075f, -0.09562501f, 0.6625f },
	{ 0, -0.09562501f, 0.6625f },
	{ -0.075f, -0.105f, 0.5f },
	{ -0.075f, -0.18f, 0.475f },
	{ 0, -0.18f, 0.475f },
	{ 0, 0.02624997f, -0.425f },
	{ -0.165f, 0.02624997f, -0.425f },
	{ -0.165f, -0.18f, -0.425f },
	{ 0, -0.18f, -0.425f },
	{ 0, 0.02624997f, -0.65f },
	{ -0.165f, 0.02624997f, -0.65f },
	{ -0.165f, -0.12375f, -0.775f },
	{ 0, -0.12375f, -0.775f },
	{ 0, 0.195f, -0.575f },
	{ -0.0625f, 0.195f, -0.575f },
	{ -0.0625f, 0.17625f, -0.6f },
	{ 0, 0.17625f, -0.6f },
	{ 0, 0.27f, -0.675f },
	{ -0.0625f, 0.27f, -0.675f },
	{ -0.0625f, 0.27f, -0.825f },
	{ 0, 0.27f, -0.825f },
	{ 0, 0.28875f, -0.7f },
	{ -0.0625f, 0.28875f, -0.7f },
	{ -0.0625f, 0.2934375f, -0.88125f },
	{ 0, 0.2934375f, -0.88125f },
	{ 0, 0.28875f, -0.725f },
	{ -0.0375f, 0.28875f, -0.725f },
	{ -0.0375f, 0.298125f, -0.8625f },
	{ 0, 0.298125f, -0.8625f },
	{ 0, 0.27f, -0.7f },
	{ -0.0375f, 0.27f, -0.7f },
	{ -0.0375f, 0.27f, -0.8f },
	{ 0, 0.27f, -0.8f },
	{ 0, 0.4575f, -0 },
	{ 0, 0.4575f, -0.2f },
	{ -0.1125f, 0.4575f, -0.2f },
	{ -0.2f, 0.4575f, -0.1125f },
	{ -0.2f, 0.4575f, -0 },
	{ 0, 0.3825f, -0 },
	{ 0, 0.27f, -0.35f },
	{ -0.196f, 0.27f, -0.35f },
	{ -0.35f, 0.27f, -0.196f },
	{ -0.35f, 0.27f, -0 },
	{ 0, 0.3075f, -0.1f },
	{ -0.056f, 0.3075f, -0.1f },
	{ -0.1f, 0.3075f, -0.056f },
	{ -0.1f, 0.3075f, -0 },
	{ 0, 0.3075f, -0.325f },
	{ -0.182f, 0.3075f, -0.325f },
	{ -0.325f, 0.3075f, -0.182f },
	{ -0.325f, 0.3075f, -0 },
	{ 0, 0.27f, -0.325f },
	{ -0.182f, 0.27f, -0.325f },
	{ -0.325f, 0.27f, -0.182f },
	{ -0.325f, 0.27f, -0 },
	{ 0, -0.33f, -0 },
	{ -0.1995f, -0.33f, -0.35625f },
	{ 0, -0.31125f, -0.375f },
	{ 0, -0.33f, -0.35625f },
	{ -0.35625f, -0.33f, -0.1995f },
	{ -0.375f, -0.31125f, -0 },
	{ -0.35625f, -0.33f, -0 },
	{ -0.21f, -0.31125f, -0.375f },
	{ -0.375f, -0.31125f, -0.21f },
};