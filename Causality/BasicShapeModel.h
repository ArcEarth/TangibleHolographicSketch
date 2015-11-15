#pragma once
#include "Models.h"
#include <DirectXColors.h>
#include <btBulletDynamicsCommon.h>

namespace Causality
{
	class IShaped
	{
	public:
		virtual std::shared_ptr<btCollisionShape> CreateCollisionShape() = 0;
	};

	class CubeModel : public DirectX::Scene::IModelNode, virtual public IShaped
	{
	public:
		CubeModel(const std::string& name = "Cube", DirectX::FXMVECTOR extend = DirectX::g_XMOne, DirectX::FXMVECTOR color = DirectX::Colors::White);
		virtual std::shared_ptr<btCollisionShape> CreateCollisionShape() override;

		DirectX::XMVECTOR GetColor();
		void XM_CALLCONV SetColor(DirectX::FXMVECTOR color);
		virtual void Render(ID3D11DeviceContext *pContext, DirectX::IEffect* pEffect);

	private:
		DirectX::Color m_Color;
	};
}