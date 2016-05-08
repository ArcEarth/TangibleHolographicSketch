#pragma once

// Options
#define __AVX2__ 1
#define __AVX__  1
#define __SSE4__ 1
#define __SSE3__ 1

#define _HLSLM_QUATERNION_COLUMN_MAJOR 0
#define _HLSLM_MATRIX_DEFAULT_COLUMN_MAJOR 0

#if _HLSLM_QUATERNION_COLUMN_MAJOR != _HLSLM_MATRIX_DEFAULT_COLUMN_MAJOR
#warning "Quaternion and Matrix Default Major are different, this may generate undesired behavier"
#endif

#include <DirectXMath.h>
#define _NO_SIMPLE_VECTORS
#include <DirectXMathExtend.h>
#include <DirectXMathIntrinsics.h>
#ifndef _DXMEXT
#define _DXMEXT
#endif
#define _XM_VECTOR_USE_LOAD_STORE_HELPER_

#include "traits.hpp"
#pragma warning(push)
#pragma warning(disable:4996)
#include "xmvector.hpp"
#include "xmswizzler.hpp"
#include "xmoperators.hpp"
#include "xmquaternion.hpp"
#include "xmconstants.hpp"
#include "xmmatrix.hpp"
#include "load_store.hpp"

// explicit instantiation
namespace DirectX
{
	namespace hlsl
	{
		template struct xmscalar<float>;
		template struct xmvector<float, 2>;
		template struct xmvector<float, 3>;
		template struct xmvector<float, 4>;

		template struct xmscalar<uint>;
		template struct xmvector<uint, 2>;
		template struct xmvector<uint, 3>;
		template struct xmvector<uint, 4>;
	}
}

// namespace for hlsl-math libary
namespace hlsl = DirectX::hlsl;

#pragma warning(pop)