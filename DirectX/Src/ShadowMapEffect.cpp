#include "pch_directX.h"
#include "ShadowMapEffect.h"
#include "EffectCommon.h"
#include <CommonStates.h>

using namespace DirectX;

namespace
{
	using namespace HLSLVectors;
#define REGISTER(b)
#define cbuffer XM_ALIGNATTR struct

#include "Shaders\ShadowMapEffectCBuffer.hlsli"

#undef cbuffer
#undef REGISTER

	struct BonesCBuffer
	{
		float4x3 Bones[MAX_BONES];
	};
}

using namespace DirectX;

struct ShadowMapEffectTraits
{
	typedef ShadowMapEffectCBuffer ConstantBufferType;

	static const int VertexShaderCount = 20;  // 4 + 4 + 12
	static const int PixelShaderCount = 7;	// 2 + 2 + 3
	static const int ShaderPermutationCount = 24; // 6 + 6 + 12

	static const int TexturePermCount = 3;
	static const int LightPermCount = 1;
	static const int BonesPermCount = 2;

	static const int TexturePermStride = 1;
	static const int LightPermStride = TexturePermCount * TexturePermStride;
	static const int BonesPermStride = LightPermCount * LightPermStride;

};

namespace
{
#if defined(_XBOX_ONE) && defined(_TITLE)
#include "Shaders/Windows/ShadowMapEffectVS_VS_OneLightNoBoneNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_OneLightNoBoneTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_OneLightFourBoneNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_OneLightFourBoneTex.inc"

#include "Shaders/Windows/ShadowMapEffectPS_PS_OneLightNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectPS_PS_OneLightTex.inc"
#else
#include "Shaders/Windows/ShadowMapEffectVS_VS_OneLightNoBoneNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_OneLightNoBoneTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_OneLightFourBoneNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_OneLightFourBoneTex.inc"

#include "Shaders/Windows/ShadowMapEffectPS_PS_OneLightNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectPS_PS_OneLightTex.inc"

#include "Shaders/Windows/ShadowMapEffectVS_VS_BinaryOneLightNoBoneNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_BinaryOneLightNoBoneTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_BinaryOneLightFourBoneNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_BinaryOneLightFourBoneTex.inc"

#include "Shaders/Windows/ShadowMapEffectPS_PS_BinaryOneLightNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectPS_PS_BinaryOneLightTex.inc"

// Screen Space No Texture VS 
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceNoBoneNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceOneBoneNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceTwoBoneNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceFourBoneNoTex.inc"
// Screen Space Texture VS 
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceNoBoneTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceOneBoneTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceTwoBoneTex.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceFourBoneTex.inc"
// Screen Space Texture with Bump VS 
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceNoBoneTexBump.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceOneBoneTexBump.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceTwoBoneTexBump.inc"
#include "Shaders/Windows/ShadowMapEffectVS_VS_ScreenSpaceFourBoneTexBump.inc"

// Screen Space PS
#include "Shaders/Windows/ShadowMapEffectPS_PS_ScreenSpaceNoTex.inc"
#include "Shaders/Windows/ShadowMapEffectPS_PS_ScreenSpaceTex.inc"
#include "Shaders/Windows/ShadowMapEffectPS_PS_ScreenSpaceTexBump.inc"

#endif
}


typedef ShadowMapEffectTraits			EffectTraitsType;
typedef EffectBase<EffectTraitsType>	EffectBaseType;

SharedResourcePool<ID3D11Device*, EffectBaseType::DeviceResources> EffectBaseType::deviceResourcesPool;

template <size_t Size>
inline ShaderBytecode MakeShaderByteCode(const BYTE(&bytecode)[Size])
{
	return ShaderBytecode{ bytecode ,sizeof(bytecode) };
}


//const ShaderBytecode EffectBaseType::VertexShaderBytecode[] =
//{
//	MakeShaderByteCode(ShadowMapEffectVS_VS_OneLightNoBoneNoTex),
//	MakeShaderByteCode(ShadowMapEffectVS_VS_OneLightNoBoneTex),
//	MakeShaderByteCode(ShadowMapEffectVS_VS_OneLightFourBoneNoTex),
//	MakeShaderByteCode(ShadowMapEffectVS_VS_OneLightFourBoneTex),
//};
const ShaderBytecode EffectBaseType::VertexShaderBytecode[] =
{
	MakeShaderByteCode(ShadowMapEffectVS_VS_OneLightNoBoneNoTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_OneLightNoBoneTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_OneLightFourBoneNoTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_OneLightFourBoneTex),

	MakeShaderByteCode(ShadowMapEffectVS_VS_BinaryOneLightNoBoneNoTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_BinaryOneLightNoBoneTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_BinaryOneLightFourBoneNoTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_BinaryOneLightFourBoneTex),

	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceNoBoneNoTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceNoBoneTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceNoBoneTexBump),
	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceOneBoneNoTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceOneBoneTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceOneBoneTexBump),
	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceTwoBoneNoTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceTwoBoneTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceTwoBoneTexBump),
	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceFourBoneNoTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceFourBoneTex),
	MakeShaderByteCode(ShadowMapEffectVS_VS_ScreenSpaceFourBoneTexBump),
};


const int EffectBaseType::VertexShaderIndices[] =
{
	0,	// NoBone x OneLight x NoTex
	1,  // NoBone x OneLight x Tex
	1,  // NoBone x OneLight x Tex
	2,	// FourBone x OneLight x NoTex
	3,	// FourBone x OneLight x Tex
	3,	// FourBone x OneLight x Tex

	4,	// NoBone x OneLight x NoTex
	5,  // NoBone x OneLight x Tex
	5,  // NoBone x OneLight x Tex
	6,	// FourBone x OneLight x NoTex
	7,	// FourBone x OneLight x Tex
	7,	// FourBone x OneLight x Tex

	8,	// NoBone x NoTex
	9,  // NoBone x Tex
	10, // NoBone x Tex Bump
	11,	// OneBone x NoTex
	12,	// OneBone x Tex
	13,	// OneBone x Tex Bump
	14,	// TwoBone x NoTex
	15,  // TwoBone x Tex
	16,  // TwoBone x Tex Bump
	17,	// FourBone x NoTex
	18,	// FourBone x Tex
	19,	// FourBone x Tex Bump
};


//const ShaderBytecode EffectBaseType::PixelShaderBytecode[] =
//{
//	MakeShaderByteCode(ShadowMapEffectPS_PS_OneLightNoTex),
//	MakeShaderByteCode(ShadowMapEffectPS_PS_OneLightTex),
//};
const ShaderBytecode EffectBaseType::PixelShaderBytecode[] =
{
	MakeShaderByteCode(ShadowMapEffectPS_PS_OneLightNoTex),
	MakeShaderByteCode(ShadowMapEffectPS_PS_OneLightTex),

	MakeShaderByteCode(ShadowMapEffectPS_PS_BinaryOneLightNoTex),
	MakeShaderByteCode(ShadowMapEffectPS_PS_BinaryOneLightTex),

	MakeShaderByteCode(ShadowMapEffectPS_PS_ScreenSpaceNoTex),
	MakeShaderByteCode(ShadowMapEffectPS_PS_ScreenSpaceTex),
	MakeShaderByteCode(ShadowMapEffectPS_PS_ScreenSpaceTexBump),
};


const int EffectBaseType::PixelShaderIndices[] =
{
	0,      // OneLight x NoTex
	1,      // OneLight x Tex
	1,      // OneLight x Tex
	0,      // OneLight x NoTex
	1,      // OneLight x Tex
	1,      // OneLight x Tex

	2,      // OneLight x NoTex
	3,      // OneLight x Tex
	3,      // OneLight x Tex
	2,      // OneLight x NoTex
	3,      // OneLight x Tex
	3,      // OneLight x Tex

	4,      // Screen x NoTex
	5,      // Screen x Tex
	6,		// Screen x Tex Bump
	4,      // Screen x NoTex
	5,      // Screen x Tex
	6,		// Screen x Tex Bump
	4,      // Screen x NoTex
	5,      // Screen x Tex
	6,		// Screen x Tex Bump
	4,      // Screen x NoTex
	5,      // Screen x Tex
	6,		// Screen x Tex Bump
};

namespace DirectX
{
	namespace EffectDirtyFlags
	{
		const int BoneTransforms = 0x100;
		const int LightsViewProjection = 0x200;
	}
}

using Microsoft::WRL::ComPtr;

XM_ALIGNATTR
struct PerspectiveLightData
{
	Matrix4x4 View;
	Matrix4x4 Projection;
	Vector4	  SourcePosition;
	Vector4	  FocusDirection;
};

class ShadowMapEffect::Impl : public EffectBaseType
{
public:
	typedef EffectTraitsType	Traits;
	typedef EffectBaseType		Base;

	ShadowMapEffectMode				mode;
	bool							alpha_discard;
	CommonStates					commonStates;
	BonesCBuffer					boneConstant;
	ConstantBuffer<BonesCBuffer>	BoneTransforms;
	int								weightsPerVertex;
	int								lightsEnabled;
	PerspectiveLightData			lights[MAX_LIGHTS];

	ID3D11ShaderResourceView*		pNormalTexture;
	ID3D11ShaderResourceView*		pSpecularTexture;

	ID3D11ShaderResourceView*		pScreenSpaceShadowMap;
	ID3D11ShaderResourceView*		pScreenSpaceShadowMapSharp;
	ID3D11ShaderResourceView*		pShadowMaps[MAX_LIGHTS];

	// Linear shadow map sampler
	ComPtr<ID3D11SamplerState>		pShadowMapSampler;

	Impl(ID3D11Device* device)
		: EffectBase(device),
		BoneTransforms(device),
		commonStates(device),
		weightsPerVertex(0), 
		lightsEnabled(1),
		mode(LightSpaceShadowRender),
		alpha_discard(false),
		pNormalTexture(NULL), pSpecularTexture(NULL), pScreenSpaceShadowMap(NULL), pScreenSpaceShadowMapSharp(NULL)
	{
		static_assert(_countof(Base::VertexShaderIndices) == Traits::ShaderPermutationCount, "array/max mismatch");
		static_assert(_countof(Base::VertexShaderBytecode) == Traits::VertexShaderCount, "array/max mismatch");
		static_assert(_countof(Base::PixelShaderBytecode) == Traits::PixelShaderCount, "array/max mismatch");
		static_assert(_countof(Base::PixelShaderIndices) == Traits::ShaderPermutationCount, "array/max mismatch");

		XMMATRIX id = XMMatrixIdentity();

		ZeroMemory(pShadowMaps, sizeof(pShadowMaps));

		for (size_t i = 0; i < MaxBones; ++i)
		{
			XMStoreFloat3x4(&boneConstant.Bones[i], id);
		}

		CD3D11_DEFAULT def;
		CD3D11_SAMPLER_DESC samplerDesc (def);
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;

		ThrowIfFailed(
			device->CreateSamplerState(&samplerDesc, &pShadowMapSampler)
			);
	}

	int GetCurrentShaderPermutation() const
	{
		static const int bonesConv[] = { 0, -1 ,-1 ,-1, 1 };
		static const int bonesConv2[] = { 0, 1 , 2 ,-1, 3 };

		int perm = 6 * mode;
		switch (mode)
		{
		case DirectX::ShadowMapEffect::LightSpaceShadowRender:
		case DirectX::ShadowMapEffect::ScreenSpaceShadowGeneration:

			perm += bonesConv[weightsPerVertex] * Traits::BonesPermStride;

			perm += (lightsEnabled - 1) * Traits::LightPermStride;

			if (texture != nullptr)
				perm += Traits::TexturePermStride;
			return perm;

		case DirectX::ShadowMapEffect::ScreenSpaceShadowRender:

			perm += bonesConv2[weightsPerVertex] * Traits::BonesPermStride; // Tex / NoTex

			if (texture != nullptr) // material texture
				if (pNormalTexture != nullptr)
					perm += 2 * Traits::TexturePermStride;
				else
					perm += Traits::TexturePermStride;

			return perm;
		}

	}

	void UpdateEffectMatrices()
	{
		if (dirtyFlags & EffectDirtyFlags::WorldViewProj)
		{
			XMMATRIX ViewProj = XMMatrixMultiply(matrices.view, matrices.projection);
			constants.World = XMMatrixTranspose(matrices.world);
			constants.ViewProjection = XMMatrixTranspose(ViewProj);
			//worldViewProjConstant = XMMatrixTranspose(XMMatrixMultiply(worldView, projection));

			dirtyFlags &= ~EffectDirtyFlags::WorldViewProj;
			dirtyFlags |= EffectDirtyFlags::ConstantBuffer;
		}
	}

	void UpdateEffectLights()
	{
		if (dirtyFlags & EffectDirtyFlags::LightsViewProjection)
		{
			for (size_t i = 0; i < lightsEnabled; i++) //? Use MAX_LIGHT instead of 1
			{
				XMMATRIX lvp = lights[i].View.LoadA();
				constants.LightPosition[i] = lights[i].SourcePosition;
				constants.LightDirection[i] = lights[i].FocusDirection;
				lvp *= lights[i].Projection.LoadA();
				constants.LightViewProjection[i] = XMMatrixTranspose(lvp);
			}

			dirtyFlags &= ~EffectDirtyFlags::LightsViewProjection;
			dirtyFlags |= EffectDirtyFlags::ConstantBuffer;
		}
	}

	void Apply(ID3D11DeviceContext* pContext)
	{
		//matrices.SetConstants(dirtyFlags, constants.worldViewProj);
		//fog.SetConstants(dirtyFlags, matrices.worldView, constants.fogVector);
		UpdateEffectMatrices();

		UpdateEffectLights();

		int permutation = GetCurrentShaderPermutation();

		ApplyShaders(pContext, permutation);
		// Make sure the constant buffer is up to date.
		if (weightsPerVertex > 0 && dirtyFlags & EffectDirtyFlags::BoneTransforms)
		{
			BoneTransforms.SetData(pContext, boneConstant);
			dirtyFlags &= ~EffectDirtyFlags::BoneTransforms;
			ID3D11Buffer* buffers[] = { /*mConstantBuffer.GetBuffer(),*/BoneTransforms.GetBuffer() };
			pContext->VSSetConstantBuffers(1, 1, buffers);
		}

		ID3D11ShaderResourceView* pSrvs[6] = { texture.Get(),pNormalTexture,pShadowMaps[0],pShadowMaps[1],pShadowMaps[2],pShadowMaps[3] };

		if (mode == ScreenSpaceShadowRender) // Use screen space shadow map
		{
			pSrvs[2] = pScreenSpaceShadowMap;
			pSrvs[3] = pScreenSpaceShadowMapSharp;
			if (constants.MaterialDiffuse.w < 0.85 || alpha_discard)
				pContext->OMSetBlendState(commonStates.NonPremultiplied(), Colors::Black.f, -1);
		}
		else
		{
			pContext->OMSetBlendState(commonStates.Opaque(), Colors::Black.f, -1);
		}

		ID3D11SamplerState* pSamplers[] = { commonStates.AnisotropicWrap(), commonStates.AnisotropicWrap(), commonStates.AnisotropicClamp(), pShadowMapSampler.Get()};
		pContext->PSSetSamplers(0, std::size(pSamplers), pSamplers);

		if (constants.MaterialDiffuse.w < 0.85)
			pContext->OMSetDepthStencilState(commonStates.DepthRead(), -1);
		else
			pContext->OMSetDepthStencilState(commonStates.DepthDefault(),-1);

		pContext->PSSetShaderResources(0, 2 + lightsEnabled, pSrvs);
	}
};

ShadowMapEffect::ShadowMapEffect(ID3D11Device* device)
	: pImpl(new Impl(device))
{
}

ShadowMapEffect::~ShadowMapEffect()
{
}


ShadowMapEffect::ShadowMapEffectMode ShadowMapEffect::GetEffectMode() const
{
	return pImpl->mode;
}

void DirectX::ShadowMapEffect::SetEffectMode(ShadowMapEffectMode mode)
{
	pImpl->mode = mode;
}

void ShadowMapEffect::SetScreenSpaceLightsShadowMap(ID3D11ShaderResourceView* pSharpShadow, ID3D11ShaderResourceView* pSoftShadow)
{
	pImpl->pScreenSpaceShadowMap = pSoftShadow;
	pImpl->pScreenSpaceShadowMapSharp = pSharpShadow;
	//pImpl->pShadowMaps[0] = pSoftShadow;
	//pImpl->pShadowMaps[1] = pSharpShadow;
}

void ShadowMapEffect::Apply(ID3D11DeviceContext * deviceContext)
{
	pImpl->Apply(deviceContext);
}

void ShadowMapEffect::GetVertexShaderBytecode(void const ** pShaderByteCode, size_t * pByteCodeLength)
{
	auto perm = pImpl->GetCurrentShaderPermutation();
	pImpl->GetVertexShaderBytecode(perm, pShaderByteCode, pByteCodeLength);
}

void XM_CALLCONV ShadowMapEffect::SetWorld(FXMMATRIX value)
{
	pImpl->matrices.world = value;

	pImpl->dirtyFlags |= EffectDirtyFlags::WorldViewProj | EffectDirtyFlags::WorldInverseTranspose | EffectDirtyFlags::FogVector;
}


void XM_CALLCONV ShadowMapEffect::SetView(FXMMATRIX value)
{
	pImpl->matrices.view = value;

	pImpl->dirtyFlags |= EffectDirtyFlags::WorldViewProj | EffectDirtyFlags::EyePosition | EffectDirtyFlags::FogVector;
}


void XM_CALLCONV ShadowMapEffect::SetProjection(FXMMATRIX value)
{
	pImpl->matrices.projection = value;

	pImpl->dirtyFlags |= EffectDirtyFlags::WorldViewProj;
}

void ShadowMapEffect::EnableDefaultLighting()
{
	EffectLights::EnableDefaultLighting(this);
}


void ShadowMapEffect::SetFogEnabled(bool value)
{
	pImpl->fog.enabled = value;

	pImpl->dirtyFlags |= EffectDirtyFlags::FogEnable;
}


void ShadowMapEffect::SetFogStart(float value)
{
	pImpl->fog.start = value;

	pImpl->dirtyFlags |= EffectDirtyFlags::FogVector;
}


void ShadowMapEffect::SetFogEnd(float value)
{
	pImpl->fog.end = value;

	pImpl->dirtyFlags |= EffectDirtyFlags::FogVector;
}


void XM_CALLCONV ShadowMapEffect::SetFogColor(FXMVECTOR value)
{
	//pImpl->constants.fogColor = value;

	//pImpl->dirtyFlags |= EffectDirtyFlags::ConstantBuffer;
}

void XM_CALLCONV ShadowMapEffect::SetDiffuseColor(FXMVECTOR value)
{
	pImpl->constants.MaterialDiffuse = value;
	pImpl->constants.MaterialAmbient = value;
	pImpl->dirtyFlags |= EffectDirtyFlags::MaterialColor | EffectDirtyFlags::ConstantBuffer;
}

void XM_CALLCONV ShadowMapEffect::SetEmissiveColor(FXMVECTOR value)
{
	pImpl->constants.MaterialEmissive = value;
	pImpl->dirtyFlags |= EffectDirtyFlags::MaterialColor | EffectDirtyFlags::ConstantBuffer;

}

void XM_CALLCONV ShadowMapEffect::SetSpecularColor(FXMVECTOR value)
{
	pImpl->constants.MaterialSpecular = value;
	pImpl->dirtyFlags |= EffectDirtyFlags::MaterialColor | EffectDirtyFlags::ConstantBuffer;
}

void ShadowMapEffect::SetSpecularPower(float value)
{
	pImpl->constants.MaterialSpecularPower = value;
	pImpl->dirtyFlags |= EffectDirtyFlags::MaterialColor | EffectDirtyFlags::ConstantBuffer;
}

void ShadowMapEffect::DisableSpecular()
{
	pImpl->constants.MaterialSpecular = float3(0.f);
	pImpl->constants.MaterialSpecularPower = 1.0f;
	pImpl->dirtyFlags |= EffectDirtyFlags::MaterialColor | EffectDirtyFlags::ConstantBuffer;
}

void ShadowMapEffect::SetAlpha(float value)
{
	pImpl->constants.MaterialDiffuse.w = value;
	pImpl->dirtyFlags |= EffectDirtyFlags::MaterialColor | EffectDirtyFlags::ConstantBuffer;
}

void DirectX::ShadowMapEffect::SetAlphaDiscard(bool enable)
{
	pImpl->alpha_discard = enable;
}

void ShadowMapEffect::SetDiffuseMap(ID3D11ShaderResourceView * pTexture)
{
	pImpl->texture = pTexture;
}

void ShadowMapEffect::SetNormalMap(ID3D11ShaderResourceView * pTexture)
{
	pImpl->pNormalTexture = pTexture;
}

void ShadowMapEffect::SetSpecularMap(ID3D11ShaderResourceView * pTexture)
{
	pImpl->pSpecularTexture = pTexture;
}

void ShadowMapEffect::SetWeightsPerVertex(int value)
{
	if ((value != 0) &&
		//(value != 1) &&
		//(value != 2) &&
		(value != 4))
	{
		throw std::out_of_range("WeightsPerVertex must be 0, 1, 2, or 4");
	}

	pImpl->weightsPerVertex = value;
}


void ShadowMapEffect::SetBoneTransforms(_In_reads_(count) XMMATRIX const* value, size_t count)
{
	if (count > MaxBones)
		throw std::out_of_range("count parameter out of range");

	auto& boneConstant = pImpl->boneConstant.Bones;

	for (size_t i = 0; i < count; i++)
	{
		XMMATRIX boneMatrix = XMMatrixTranspose(value[i]);
		XMStoreFloat3x4(&boneConstant[i], boneMatrix);
	}

	pImpl->dirtyFlags |= EffectDirtyFlags::BoneTransforms;
}


void ShadowMapEffect::ResetBoneTransforms()
{
	auto boneConstant = pImpl->boneConstant.Bones;

	XMMATRIX id = XMMatrixIdentity();

	for (size_t i = 0; i < MaxBones; ++i)
	{
		XMStoreFloat3x4(&boneConstant[i], id);
	}

	pImpl->dirtyFlags |= EffectDirtyFlags::BoneTransforms;
}

void XM_CALLCONV ShadowMapEffect::SetAmbientLightColor(FXMVECTOR value)
{
	pImpl->constants.AmbientLight = value;
	pImpl->dirtyFlags |= EffectDirtyFlags::ConstantBuffer;
}

void ShadowMapEffect::SetLightEnabled(int whichLight, bool value)
{
	//? Not supported yet, always light 0
}

void XM_CALLCONV ShadowMapEffect::SetLightDirection(int whichLight, FXMVECTOR value)
{
	//! Not support
}

void XM_CALLCONV ShadowMapEffect::SetLightDiffuseColor(int whichLight, FXMVECTOR value)
{
	pImpl->constants.LightColor[whichLight] = value;
	pImpl->dirtyFlags |= EffectDirtyFlags::ConstantBuffer;
}

void XM_CALLCONV ShadowMapEffect::SetLightSpecularColor(int whichLight, FXMVECTOR value)
{
	//! Not support
}

void DirectX::ShadowMapEffect::SetLightShadowMapBias(int whichLight, float bias)
{
}

void ShadowMapEffect::SetLightShadowMap(int whichLight, ID3D11ShaderResourceView * pTexture)
{
	pImpl->pShadowMaps[whichLight] = pTexture;
}

void XM_CALLCONV ShadowMapEffect::SetLightView(int whichLight, FXMMATRIX value)
{
	pImpl->lights[whichLight].View = value;

	// Extract Light Position and orientation
	pImpl->lights[whichLight].SourcePosition = -value.r[3];
	pImpl->lights[whichLight].SourcePosition.w = 1.0f;

	XMMATRIX viewTrans = XMMatrixTranspose(value);
	pImpl->lights[whichLight].FocusDirection = XMVectorSelect(g_XMSelect1110.v, viewTrans.r[2], g_XMSelect1110.v);

	pImpl->dirtyFlags |= EffectDirtyFlags::LightsViewProjection;
}

void XM_CALLCONV ShadowMapEffect::SetLightProjection(int whichLight, FXMMATRIX value)
{
	pImpl->lights[whichLight].Projection = value;
	pImpl->dirtyFlags |= EffectDirtyFlags::LightsViewProjection;
}
