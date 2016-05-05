#pragma once
#include <Causality\VisualObject.h>
#include <Geometrics\BezierMesh.h>
#include <VertexTypes.h>

namespace Causality
{
	class BezierPatchObject : public VisualObject
	{
	public:
		void Parse(const ParamArchive* archive) override;
		~BezierPatchObject();

		virtual void Render(IRenderContext * pContext, IEffect* pEffect = nullptr) override;
	private:
		Geometrics::CubicBezierPatch m_patch;

		using TriangleMeshType = Geometrics::TriangleMesh<DirectX::VertexPositionNormalTangentColorTexture>;
		TriangleMeshType	m_mesh;
		uptr<DirectX::Scene::IModelNode>
			m_pModel;
	};
}