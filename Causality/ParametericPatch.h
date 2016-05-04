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
		Geometrics::TriangleMesh<DirectX::VertexPositionNormalColorTexture>	m_mesh;
		uptr<DirectX::Scene::IModelNode>
			m_pModel;
	};
}