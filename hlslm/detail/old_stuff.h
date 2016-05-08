#pragma once

//template <size_t _Size>
//inline xmvector<float, _Size> XM_CALLCONV operator+(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
//{
//	xmvector<float, _Size> ret;
//	ret.v = XMVectorAdd(lhs.v, rhs.v);
//	return ret;
//}

//template <size_t _Size>
//inline xmvector<float, _Size> XM_CALLCONV operator-(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
//{
//	xmvector<float, _Size> ret;
//	ret.v = XMVectorSubtract(lhs.v, rhs.v);
//	return ret;
//}

//template <size_t _Size>
//inline xmvector<float, _Size> XM_CALLCONV operator*(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
//{
//	xmvector<float, _Size> ret;
//	ret.v = XMVectorMultiply(lhs.v, rhs.v);
//	return ret;
//}

//template <size_t _Size>
//inline xmvector<float, _Size> XM_CALLCONV operator/(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
//{
//	xmvector<float, _Size> ret;
//	ret.v = XMVectorDivide(lhs.v, rhs.v);
//	return ret;
//}

//template <typename lhs_t, typename rhs_t>
//struct if_both_float_vector : public std::enable_if<
//	std::is_same < float, typename binary_operator_traits<lhs_t, rhs_t>::scalar_type>::value &&
//	binary_operator_traits<lhs_t, rhs_t>::left_v &&
//	binary_operator_traits<lhs_t, rhs_t>::right_v,
//	typename binary_operator_traits<lhs_t, rhs_t>::return_type >
//{};
//
//template <typename lhs_t, typename rhs_t>
//struct if_left_float_vector : public std::enable_if<
//	std::is_same < float, typename binary_operator_traits<lhs_t, rhs_t>::scalar_type>::value &&
//	binary_operator_traits<lhs_t, rhs_t>::left_v &&
//	!binary_operator_traits<lhs_t, rhs_t>::right_v,
//	typename binary_operator_traits<lhs_t, rhs_t>::return_type >
//{};
//
//template <typename lhs_t, typename rhs_t>
//struct if_none_float_vector : public std::enable_if<
//	std::is_same < float, typename binary_operator_traits<lhs_t, rhs_t>::scalar_type>::value &&
//	!binary_operator_traits<lhs_t, rhs_t>::left_v &&
//	!binary_operator_traits<lhs_t, rhs_t>::right_v,
//	typename binary_operator_traits<lhs_t, rhs_t>::return_type >
//{};