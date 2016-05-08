#pragma once
#include <utility>
#include <type_traits>

namespace mpl
{
	using std::integer_sequence;
	using std::index_sequence;
	using std::integral_constant;
	using std::make_index_sequence;
	using std::make_integer_sequence;
	using std::conditional;
	using std::conditional_t;

	template<class...> struct conjunction : std::true_type { };
	template<class B1> struct conjunction<B1> : B1 { };
	template<class B1, class... Bn>
	struct conjunction<B1, Bn...> : std::conditional_t<B1::value != false, conjunction<Bn...>, B1> {};

	template<class...> struct disjunction : std::false_type { };
	template<class B1> struct disjunction<B1> : B1 { };
	template<class B1, class... Bn>
	struct disjunction<B1, Bn...> : std::conditional_t<B1::value != false, B1, disjunction<Bn...>> {};

	template< size_t I, class T >
	struct get_element;

	// recursive case
	template< typename _Ty, size_t I, _Ty Head, _Ty... Tail >
	struct get_element<I, integer_sequence<_Ty, Head, Tail...>>
		: public get_element<I - 1, integer_sequence<_Ty, Tail...>> {
		static_assert(I <= sizeof...(Tail), "index out of range in static integer sequence");
	};

	// base case
	template< typename _Ty, _Ty Head, _Ty... Tail >
	struct get_element<0, integer_sequence<_Ty, Head, Tail...>>
		: public integral_constant<_Ty, Head> {};

	template<typename _Ty, size_t I, _Ty Val, class Sequence, class PrevSeq = integer_sequence<_Ty>>
	struct set_element_impl;

	template<typename _Ty, _Ty Val, size_t I, _Ty Current, _Ty... Tail, _Ty... Prev>
	struct set_element_impl<_Ty, I, Val, integer_sequence<_Ty, Current, Tail...>, integer_sequence<_Ty, Prev...>>
		: public set_element_impl<_Ty, I - 1, Val, integer_sequence<_Ty, Tail...>, integer_sequence<_Ty, Prev..., Current>>
	{};

	template<typename _Ty, _Ty Val, _Ty Current, _Ty... Tail, _Ty... Prev>
	struct set_element_impl<_Ty, 0, Val, integer_sequence<_Ty, Current, Tail...>, integer_sequence<_Ty, Prev...>>
	{
		using type = integer_sequence<_Ty, Prev..., Val, Tail...>;
	};

	template<typename _Ty, size_t I, _Ty Val, class Sequence, class PrevSeq = integer_sequence<_Ty>>
	using set_element_impl_t = typename set_element_impl<_Ty, I, Val, Sequence, PrevSeq>::type;

	template<size_t I, int Val, class Sequence>
	struct set_element;

	template<size_t I, int Val, typename _Ty, _Ty... IS>
	struct set_element<I, Val, integer_sequence<_Ty, IS...>>
		: public set_element_impl<_Ty, I, _Ty(Val), integer_sequence<_Ty, IS...>>
	{
	};

	template<size_t I, int Val, class Sequence>
	using set_element_t = typename set_element<I,Val, Sequence>::type;

	template <typename Seq1, typename Seq2, int Offset = 0>
	struct concat_sequence;

	template <typename _Ty, int Offset, _Ty ... Is1, _Ty ... Is2>
	struct concat_sequence<integer_sequence<_Ty, Is1...>, integer_sequence<_Ty, Is2...>, Offset>
	{
		using type = integer_sequence<_Ty, Is1..., ((_Ty)(Offset + Is2))...>;
	};

	template <typename Seq1, typename Seq2, int Offset = 0>
	using concat_sequence_t = typename concat_sequence<Seq1, Seq2, Offset>::type;

	template <typename T>
	struct type_wrapper
	{
		using type = T;
	};

	template <size_t Start, typename IS>
	struct tail_sequence;

	template <typename _Ty>
	struct tail_sequence<0U, integer_sequence<_Ty>>
	{
		using type = integer_sequence<_Ty>;
	};

	template <typename _Ty, _Ty Head, _Ty... Tail>
	struct tail_sequence<0U, integer_sequence<_Ty, Head, Tail...>>
	{
		using type = integer_sequence<_Ty, Head, Tail...>;
	};

	template <size_t Start, typename _Ty, _Ty Head, _Ty... Tail>
	struct tail_sequence<Start, integer_sequence<_Ty, Head, Tail...>>
		: public tail_sequence <Start - 1, integer_sequence<_Ty, Tail...>>
	{};


	template <size_t Start, typename _Ty>
	using tail_sequence_t = typename tail_sequence<Start, _Ty>::type;

	template <size_t Size, typename IS>
	struct head_sequence;

	template <size_t Size, typename _Ty>
	struct head_sequence<Size, integer_sequence<_Ty>>
	{
		using type = integer_sequence<_Ty>;
	};

	template <size_t Size, typename _Ty, _Ty... Head, _Ty Tail>
	struct head_sequence<Size, integer_sequence<_Ty, Head..., Tail>>
		: public conditional_t < (Size == (sizeof...(Head)+1)),
		type_wrapper<integer_sequence<_Ty, Head..., Tail>>,
		head_sequence<Size, integer_sequence<_Ty, Head...>>>
	{};

	template <size_t Size, typename _Ty>
	using head_sequence_t = typename head_sequence<Size, _Ty>::type;

	template<size_t I, class Sequence, class PrevSeq>
	struct split_sequence;

	template<typename _Ty, size_t I, _Ty Current, _Ty... Tail, _Ty... Prev>
	struct split_sequence<I, integer_sequence<_Ty, Current, Tail...>, integer_sequence<_Ty, Prev...>>
		: public split_sequence<I - 1, integer_sequence<_Ty, Tail...>, integer_sequence<_Ty, Prev..., Current>>
	{};

	template<typename _Ty, _Ty Current, _Ty... Tail, _Ty... Prev>
	struct split_sequence<0, integer_sequence<_Ty, Current, Tail...>, integer_sequence<_Ty, Prev...>>
	{
		using head = integer_sequence<_Ty, Current, Prev...>;
		using tail = integer_sequence<_Ty, Tail...>;
	};

	template <typename IS1, typename IS2, typename Sorted>
	struct ordered_merge;

	template <typename _Ty , _Ty... Seq1, _Ty... Sorted>
	struct ordered_merge<integer_sequence<_Ty, Seq1...>, integer_sequence<_Ty>, integer_sequence<_Ty, Sorted...>>
	{
		using type = integer_sequence<_Ty, Sorted..., Seq1...>;
	};

	template <typename _Ty, _Ty... Seq2, _Ty... Sorted>
	struct ordered_merge<integer_sequence<_Ty>, integer_sequence<_Ty, Seq2...>, integer_sequence<_Ty, Sorted...>>
	{
		using type = integer_sequence<_Ty, Sorted..., Seq2...>;
	};

	template <typename _Ty, _Ty Head1, _Ty... Tail1, _Ty Head2, _Ty... Tail2, _Ty... Sorted>
	struct ordered_merge<integer_sequence<_Ty, Head1, Tail1...>, integer_sequence<_Ty, Head2, Tail2...>, integer_sequence<_Ty, Sorted...>>
		: std::conditional_t<Head1 < Head2,
			ordered_merge<integer_sequence<_Ty, Tail1...>, integer_sequence<_Ty, Head2, Tail2...>, integer_sequence<_Ty, Sorted..., Head1>>,
			ordered_merge<integer_sequence<_Ty, Head1, Tail1...>, integer_sequence<_Ty, Tail2...>, integer_sequence<_Ty, Sorted..., Head2>>
		>
	{};

	template <typename IS>
	struct sort_sequence;

	template <typename _Ty, _Ty a>
	struct sort_sequence<integer_sequence<_Ty, a>>
	{
		using type = integer_sequence<_Ty, a>;
	};

	template <typename _Ty, _Ty a, _Ty b>
	struct sort_sequence<integer_sequence<_Ty, a, b>>
		: public conditional < a < b,
		integer_sequence<_Ty, a, b >,
		integer_sequence<_Ty, b, a >> {
	};

	template <typename _Ty, _Ty... values>
	struct sort_sequence<integer_sequence<_Ty, values...>>
	{
		using splited_seq = split_sequence<sizeof...(values) / 2, integer_sequence<_Ty, values...>, integer_sequence<_Ty>>;
		using sorted_head_seq = typename sort_sequence<typename splited_seq::head>::type;
		using sorted_tail_seq = typename sort_sequence<typename splited_seq::tail>::type;
		using type = typename ordered_merge<sorted_head_seq, sorted_tail_seq, integer_sequence<_Ty>>::type;
	};

	// As[Idx] = Vals
	template <typename As, typename Idx, typename Vals>
	struct indirect_assign;

	template <typename _Ty, _Ty... Masks, size_t IdxHead, size_t... Indices, _Ty ValHead, _Ty... Values>
	struct indirect_assign<integer_sequence<_Ty, Masks...>, index_sequence<IdxHead, Indices...>, integer_sequence<_Ty, ValHead, Values...>>
		: public indirect_assign< set_element_t<IdxHead, ValHead, integer_sequence<_Ty, Masks...>>, index_sequence<Indices...>, integer_sequence<_Ty, Values...>>
	{};

	template <typename _Ty, _Ty... Masks>
	struct indirect_assign<integer_sequence<_Ty, Masks...>, index_sequence<>, integer_sequence<_Ty>>
	{
		using type = integer_sequence<_Ty, Masks...>;
	};


	//template <size_t Start, size_t Size, typename Seq>
	//struct sub_sequence
	//{
	//	using type = head_sequence<Size, tail_sequence<Start, Seq>::type>::type;
	//};


	//template <size_t Start, size_t Size, typename Seq>
	//using sub_sequence_t = typename sub_sequence<Start, Size, Seq>::type;

	// Preserve the length of Seq2, but overrite the content with Seq1 as long as poosible
	template <typename Seq1, typename Seq2>
	struct overwrite_sequence;

	template <typename _Ty, _Ty ... Is1, _Ty ... Is2>
	struct overwrite_sequence<integer_sequence<_Ty, Is1...>, integer_sequence<_Ty, Is2...>>
	{
		using type = concat_sequence_t <
			integer_sequence<_Ty, Is1...>,
			tail_sequence_t <sizeof...(Is1),
			integer_sequence<_Ty, Is2... >> >;
	};

	template <typename Seq1, typename Seq2>
	using overwrite_sequence_t = typename overwrite_sequence<Seq1, Seq2>::type;

	template <typename _Ty, typename IS>
	struct cast_sequence;

	template <typename _Ty, typename _Tother, _Tother... IS>
	struct cast_sequence<_Ty, integer_sequence<_Tother, IS...>>
	{
		using type = integer_sequence<_Ty, _Ty(IS)...>;
	};

	template <typename _Ty, typename IS>
	using cast_sequence_t = typename cast_sequence<_Ty, IS>::type;
}
