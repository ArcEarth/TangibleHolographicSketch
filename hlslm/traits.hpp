#pragma once
#include <cstdint>
#include <type_traits>

namespace DirectX
{
	struct XMFLOAT3;
	struct XMFLOAT2;
	struct XMFLOAT4;
	struct XMFLOAT2A;
	struct XMFLOAT3A;
	struct XMFLOAT4A;

	namespace hlsl
    {
		//using std::size_t;
		using index_t = size_t;
		using uint = uint32_t;
		using std::enable_if_t;
		
		template <typename _T>
		struct xmscalar;

		template <typename _T, size_t _Size>
		struct xmvector;

		enum MatrixMajorEnum
		{
			RowMajor = 0,
			ColumnMajor = 1,
		};

#if(_HLSLM_MATRIX_DEFAULT_COLUMN_MAJOR)
		static constexpr MatrixMajorEnum DefaultMajor = ColumnMajor;
#else
		static constexpr MatrixMajorEnum DefaultMajor = RowMajor;
#endif

		template <typename _T, size_t _Rows, size_t _Cols, MatrixMajorEnum _Major = DefaultMajor>
		struct xmmatrix;

		// All wrappers must contains two methods : eval() and assign(Ty)
		// wrapper for matrix transepoe
		template <typename _T, size_t _SrcRows, size_t _SrcCols>
		struct xmtranposer;

		// wrapper for matrix block operation
		template <typename _T, size_t _StRows, size_t StCols, size_t _Rows, size_t _Cols>
		struct xmmatblock;

		// wrapper for vector selection and swizzle
		template <typename _T, index_t... _SwzArgs>
		struct xmswizzler;

		namespace traits
		{
			template <typename T>
			struct vector_traits
			{
				//static_assert(false,"type not support, please add cutomized vector_traits specialization.");
				static constexpr size_t rows = 0;
				static constexpr int cols = 0;
				using scalar = void;
			};

			// is the lhs_t and rhs_t are *identical* matrix expression
			template <typename lhs_t, typename rhs_t>
			struct is_assignable
			{
				using lhs_traits = vector_traits<lhs_t>;
				using rhs_traits = vector_traits<rhs_t>;
				static _CONST_DATA bool value = std::is_same<typename lhs_traits::scalar, typename rhs_traits::scalar>::value
												&& lhs_traits::rows == rhs_traits::rows && lhs_traits::cols == lhs_traits::cols;
			};

			template <typename _Ty>
			struct enable_hlsl_operator
			{
				// exlusive operators, such as float[3] + float[3], suggest to disable
				static constexpr bool exclusive = false;
				// inclusive operators, such as vector3f + float[3], suggest to enable
				static constexpr bool inclusive = true;
			};

			template <typename _Ty>
			struct scalar_traits : public std::false_type {};

			template <typename _Ty>
			struct is_expression : public std::false_type {};

			template <typename _Ty, size_t _Size>
			struct enable_hlsl_operator< xmvector<_Ty, _Size >>
			{
				static constexpr bool exclusive = true;
				static constexpr bool inclusive = true;
			};

			template <typename _Ty>
			struct enable_hlsl_operator<xmscalar<_Ty>>
			{
				static constexpr bool exclusive = true;
				static constexpr bool inclusive = true;
			};

			template <typename _Ty, index_t... _SwzIdx>
			struct enable_hlsl_operator<xmswizzler<_Ty, _SwzIdx...>>
			{
				static constexpr bool exclusive = true;
				static constexpr bool inclusive = true;
			};

			template <typename _Ty, size_t _Size>
			struct vector_traits< xmvector<_Ty, _Size >>
			{
				static constexpr size_t rows = 1;
				static constexpr int cols = _Size;
				using scalar = _Ty;
			};

			template <typename _Ty>
			struct vector_traits<xmscalar<_Ty>>
			{
				static constexpr size_t rows = 1;
				static constexpr int cols = 1;
				using scalar = _Ty;
			};

			template <typename _Ty, index_t... _SwzIdx>
			struct vector_traits<xmswizzler<_Ty, _SwzIdx...>>
			{
				static constexpr size_t rows = 1;
				static constexpr int cols = sizeof...(_SwzIdx);
				using scalar = _Ty;
			};

			template <typename _Ty>
			struct is_xmvector : std::false_type {};

			template <typename _Ty>
			struct is_xmvector<xmscalar<_Ty>> : std::true_type {};

			template <typename _Ty, index_t... _SwzIdx>
			struct is_expression<xmswizzler<_Ty, _SwzIdx...>> : public std::true_type {};

			template <typename _Ty, size_t _SrcRows, size_t _SrcCols>
			struct is_expression<xmtranposer<_Ty, _SrcRows, _SrcCols>> : public std::true_type {};

			template <typename _Ty, size_t _StRows, size_t _StCols, size_t _Rows, size_t _Cols>
			struct is_expression<xmmatblock<_Ty, _StRows, _StCols, _Rows, _Cols>> : public std::true_type {};

			//template <typename _Ty>
			//struct is_xmvector<xmswizzler<_Ty, 0>> : std::true_type {};

			//template <typename _Ty>
			//struct is_xmvector<xmswizzler<_Ty, 0, 1>> : std::true_type {};

			//template <typename _Ty>
			//struct is_xmvector<xmswizzler<_Ty, 0, 1, 2>> : std::true_type {};

			//template <typename _Ty>
			//struct is_xmvector<xmswizzler<_Ty, 0, 1, 2, 3>> : std::true_type {};

			template <typename _Ty, size_t _Size>
			struct is_xmvector<xmvector<_Ty, _Size>> : std::true_type {};

			template <>
			struct scalar_traits<uint> : public std::true_type {
				using type = uint;
			};

			template <>
			struct scalar_traits<int32_t> : public std::true_type {
				using type = int32_t;
			};

			template <>
			struct scalar_traits<float> : public std::true_type {
				using type = float;
			};

			template <typename _Ty>
			struct is_aligned
			{
				static constexpr bool value = (alignof(_Ty) >= alignof(XMVECTOR) && (alignof(_Ty) % alignof(XMVECTOR) == 0));
			};

			using std::conditional_t;

			template <typename scalar_type, size_t rows, size_t cols>
			using get_xm_type =
				conditional_t < rows*cols == 0, void,
				conditional_t < rows*cols == 1, xmscalar<scalar_type>,
				conditional_t < rows == 1,		xmvector<scalar_type, cols>,
												xmmatrix < scalar_type, rows, cols >>>>;

			template <typename lhs_t, typename rhs_t>
			struct binary_operator_traits
			{
				static constexpr size_t get_dimension(size_t a, size_t b)
				{
					return
						a == b ? a :
						a == 1 ? b :
						b == 1 ? a :
						0;
				};

				using lop = enable_hlsl_operator<lhs_t>;
				using rop = enable_hlsl_operator<rhs_t>;

				using left_type = lhs_t;
				using right_type = rhs_t;

				using lhs_traits = vector_traits<lhs_t>;
				using rhs_traits = vector_traits<rhs_t>;

				static constexpr bool left_v = is_xmvector<lhs_t>::value;
				static constexpr bool right_v = is_xmvector<rhs_t>::value;

				using scalar_type = std::conditional_t<
					std::is_same<typename lhs_traits::scalar, typename rhs_traits::scalar>::value,
					typename rhs_traits::scalar,
					void>;

				static constexpr size_t rows = get_dimension(lhs_traits::rows, rhs_traits::rows);
				static constexpr size_t cols = get_dimension(lhs_traits::cols, rhs_traits::cols);
				static constexpr size_t size = rows*cols;

				using return_type = get_xm_type<scalar_type, rows, cols>;
				using type = return_type;

				static constexpr bool is_valiad_type = !std::is_same<type, void>::value && scalar_traits<scalar_type>::value;

				static constexpr bool overload = is_valiad_type && (lop::exclusive || rop::exclusive) && lop::inclusive && rop::inclusive;
				static constexpr bool overload_assign = is_valiad_type && lop::exclusive && rop::inclusive;

			};

			template <typename lhs_t, typename rhs_t>
			using binary_operator_return_type = typename binary_operator_traits<lhs_t, rhs_t>::return_type;

			template <typename _Ty>
			struct is_memory_type : public std::true_type {};

			// Floats and int are load into registers
			template <>
			struct is_memory_type<float> : std::false_type {};

			template <>
			struct is_memory_type<uint> : std::false_type {};

			template <typename _Ty>
			struct is_memory_type<xmscalar<_Ty>> : std::false_type {};

			template <typename _Ty, size_t _Size>
			struct is_memory_type<xmvector<_Ty, _Size>> : std::false_type {};

			template <typename _Ty, index_t... _SwzArgs>
			struct is_memory_type<xmswizzler<_Ty, _SwzArgs...>> : std::false_type {};

			template <typename _Ty>
			struct memery_vector_traits : public vector_traits<_Ty>
			{
				using void_type = void;
				using memery_type = _Ty;
				using traits = vector_traits<_Ty>;
				static constexpr size_t size = traits::cols * traits::rows;

				using type = std::conditional_t< traits::rows == 1,
					xmvector<typename traits::scalar, traits::cols>,
					xmmatrix<typename traits::scalar, traits::rows, traits::cols>
				>;
			};

			template <typename _Ty>
			struct unary_operator_traits : public vector_traits<_Ty>
			{
				using void_type = void;
				using memery_type = _Ty;
				using traits = vector_traits<_Ty>;

				using type = get_xm_type<typename traits::scalar, traits::rows, traits::cols>;
			};

			template <typename _Ty>
			struct enable_memery_traits : public std::enable_if<is_memory_type<_Ty>::value, typename memery_vector_traits<_Ty>>
			{
			};

			template <typename _Ty>
			using enable_memery_traits_t = typename enable_memery_traits<_Ty>::type;
		}
}
}