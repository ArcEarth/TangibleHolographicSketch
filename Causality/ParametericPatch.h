#pragma once
#include <Causality\VisualObject.h>
#include <Geometrics\BezierMesh.h>
#include <VertexTypes.h>
#include <atomic>

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
		CubicBezierPatch m_patch;
		using TriangleMeshType = Geometrics::TriangleMesh<DirectX::VertexPositionNormalTangentColorTexture>;
		TriangleMeshType	m_mesh;
		std::vector<TriangleMeshType> m_fracorizedMeshes;
		uptr<DirectX::Scene::IModelNode> m_pModel;
		uptr<DirectX::Scene::IModelNode> m_factorizeModel;

		std::atomic_bool	m_isReady;
	};
}