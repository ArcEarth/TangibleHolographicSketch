#pragma once
#include <Causality\VisualObject.h>
#include <Geometrics\BezierMesh.h>
#include <VertexTypes.h>
#include <atomic>
#include <Textures.h>

namespace Causality
{
	typedef Geometrics::Bezier::BezierPatch<Vector3, 3U> CubicBezierPatch;
	typedef CubicBezierPatch::ClippingType CubicBezierCurve;

	class BezierPatchObject : public VisualObject
	{
	public:
		void Parse(const ParamArchive* archive) override;

		~BezierPatchObject();

		virtual void Render(IRenderContext * pContext, IEffect* pEffect = nullptr) override;
	private:
		void DrawDecal(I2DContext* pContext);
		void UpdateDecalGeometry(I2DFactory* pFactory);

		CubicBezierPatch	m_patch;
		using TriangleMeshType = Geometrics::TriangleMesh<DirectX::VertexPositionNormalTangentColorTexture>;
		TriangleMeshType	m_mesh;

		void ExtractMeshFromModel(TriangleMeshType& mesh, const IModelNode* pNode);
		void BuildTriangleMesh(int tessellation, DirectX::SimpleMath::Color &color);

		// Decal texture for rendering highlights in target model
		int								m_declDirtyFalg;
		int								m_requestCancelLoading;
		RenderableTexture2D				m_decal;
		cptr<ID2D1PathGeometry>			m_patchGeos;
		cptr<ID2D1SolidColorBrush>		m_brush;

		Color							m_decalBackground;
		Color							m_decalStroke;
		Color							m_decalFill;

		std::vector<TriangleMeshType>		m_fracorizedMeshes;
		uptr<DirectX::Scene::IModelNode>	m_pModel;

		uptr<DirectX::Scene::IModelNode>	m_factorizeModel;

		std::atomic_bool				m_isReady;
		concurrency::task<void>			m_loadingTask;
	};
}