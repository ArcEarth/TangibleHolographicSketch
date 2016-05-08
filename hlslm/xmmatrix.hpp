#pragma once
#include "traits.hpp"
#include "xmvector.hpp"

#ifndef _HLSLM_MATRIX_DEFAULT_COLUMN_MAJOR
#define _HLSLM_MATRIX_DEFAULT_COLUMN_MAJOR 0
#endif

namespace DirectX
{
    namespace hlsl
    {
        template <typename _T, size_t _Rows, size_t _Cols, MatrixMajorEnum _Major>
        struct xmmatrix
        {
            static constexpr size_t Size = _Rows * _Cols;
            static constexpr size_t Rows = _Rows;
            static constexpr size_t Cols = _Cols;
            using Scalar = _T;
            using RowVectorType = xmvector<Scalar, Cols>;
			using ColVectorType = xmvector<Scalar, Rows>;

            RowVectorType r[Rows];  

			// Row accessers
			template <size_t _row>
			RowVectorType row() const { return r[_row]; }
			template <size_t _row>
			RowVectorType& row() { return r[_row]; }

			// Column accessers
			template <size_t _col>
			ColVectorType get_col() const { return r[_col]; }
			
			template <size_t _col, class rhs_t>
			std::enable_if_t<traits::is_assignable<ColVectorType, rhs_t>::value> 
			set_col(const rhs_t& value) { return r[_col]; }

		};
        
        using xmmatrix4f   = xmmatrix<float,4,4>;
        using xmmatrix3f   = xmmatrix<float,3,3>;
        using xmmatrix4x4f = xmmatrix<float,4,4>;
        using xmmatrix3x3f = xmmatrix<float,3,3>;
        using xmmatrix2x4f = xmmatrix<float,2,4>;
        using xmmatrix3x4f = xmmatrix<float,2,4>;
    }
}