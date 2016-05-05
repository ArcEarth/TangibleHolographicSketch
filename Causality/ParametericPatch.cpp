#include "pch_bcl.h"
#include "ParametericPatch.h"
#include "Scene.h"
#include <Models.h>
#include <VertexTypes.h>
#include <PrimitiveVisualizer.h>
using namespace Causality;
using namespace DirectX::Scene;

REGISTER_SCENE_OBJECT_IN_PARSER(bezier_patch, BezierPatchObject);

float g_ControlPointsRaius = 0.005f;
float g_ControlPointsConnectionRadius = 0.002;

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
const TeapotPatch TeapotPatches[] =
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
const DirectX::XMVECTORF32 TeapotControlPoints[] =
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


void BezierPatchObject::Parse(const ParamArchive * archive)
{
	SceneObject::Parse(archive);
	auto pDevice = this->Scene->GetRenderDevice();
	const char* cpstr = nullptr;
	std::vector<float> cps;

	int tessellation = 9;
	GetParam(archive,"tessellation", tessellation);
	Color color = DirectX::Colors::White;
	GetParam(archive, "color", color);
	//GetParamArray(archive, "control_points", cps);

	//if (cps.size() < 16 * 3)
	//	return;

	//std::copy_n(cps.data(), 3 * 16, (float*)m_patch.data());

	auto& patch = TeapotPatches[9];
	for (int i = 0; i < std::size(patch.indices); i++)
	{
		m_patch[i] = TeapotControlPoints[patch.indices[i]].v;
	}

	bool succ = Geometrics::triangluate(m_patch, m_mesh, tessellation);

	for (auto& v : m_mesh.vertices)
	{
		DirectX::VertexTraits::set_color(v, color);
	}

	m_mesh.flip();

	auto pModel = new MonolithModel();

	pModel->pMesh = make_shared<MeshBuffer>();

	pModel->SetName("bezier patch");

	DirectX::CreateBoundingBoxesFromPoints(
		pModel->BoundBox,
		pModel->BoundOrientedBox,
		16, m_patch.data(), sizeof(decltype(m_patch)::ValueType));

	pModel->pMesh->CreateDeviceResources(pDevice,
		m_mesh.vertices.data(),
		m_mesh.vertices.size(),
		m_mesh.indices.data(),
		m_mesh.indices.size());

	m_pModel.reset(pModel);
	VisualObject::m_pRenderModel = pModel;
}

BezierPatchObject::~BezierPatchObject()
{
	//auto pModel = static_cast<MonolithModel*>(m_pModel.release());
	//if (pModel) delete pModel;
}

void BezierPatchObject::Render(IRenderContext * pContext, IEffect * pEffect)
{
	VisualObject::Render(pContext, pEffect);

	if (g_DebugView && pEffect)
	{
		auto& drawer = DirectX::Visualizers::g_PrimitiveDrawer;
		auto world = this->GlobalTransformMatrix();
		//geo.Transform(geo, GlobalTransformMatrix());
		Color color = DirectX::Colors::LimeGreen.v;

		drawer.SetWorld(world);

		if (m_pModel)
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
