#pragma once

#include "Definitions.hpp"
#include <utility>

namespace TuplePrivate
{
    template<uint64 Index, typename HeadItem, typename... TailItems>
    struct INTERNAL_LINKAGE MemberTypeHelper
    {
        using Type = typename MemberTypeHelper<Index - 1, TailItems...>::Type;
    };

    template<typename HeadItem, typename... TailItems>
    struct INTERNAL_LINKAGE MemberTypeHelper<0, HeadItem, TailItems...>
    {
        using Type = HeadItem;
    };

    template<uint64 Index, typename MemberType>
    struct INTERNAL_LINKAGE TTupleLeaf
    {
        template<typename InMemberType>
        inline explicit constexpr TTupleLeaf(InMemberType&& Initializer)
            : Member(MoveIfPossible(Initializer))
        {
        }

        inline constexpr TTupleLeaf(TTupleLeaf&& Other)
            : Member(Move(Other.Member))
        {
        }

        inline constexpr TTupleLeaf(const TTupleLeaf& Other)
            : Member(Move(Other.Member))
        {
        }

        virtual ~TTupleLeaf() = default;

        MemberType Member;
    };

    template<uint64 Index, typename... Items>
    class INTERNAL_LINKAGE TTupleImpl
    {
    };

    template<uint64 Index>
    class INTERNAL_LINKAGE TTupleImpl<Index>
    {
    };

    template<uint64 Index, typename HeadItem, typename... TailItems>
    class INTERNAL_LINKAGE TTupleImpl<Index, HeadItem, TailItems...> : public TTupleLeaf<Index, HeadItem>, public TTupleImpl<Index + 1, TailItems...>
    {
    public:

        template<typename InHeadItem, typename... InTailItems>
        inline constexpr explicit TTupleImpl(InHeadItem&& HeadArg, InTailItems&& ... TailArgs)
            : TTupleLeaf<Index, HeadItem>(MoveIfPossible(HeadArg))
            , TTupleImpl<Index + 1, TailItems...>(MoveIfPossible(TailArgs)...)
        {
        }

        inline constexpr TTupleImpl(TTupleImpl&& Other) = default;
        inline constexpr TTupleImpl(const TTupleImpl& Other) = default;

        virtual ~TTupleImpl() = default;

    };
}

template<typename HeadItem, typename... TailItems>
class TTuple final : public TuplePrivate::TTupleImpl<0, HeadItem, TailItems...>
{

    template<uint64 Offset>
    using MemberType = typename TuplePrivate::MemberTypeHelper<Offset, HeadItem, TailItems...>::Type;

    template<uint64 Offset>
    using LeafType = TuplePrivate::TTupleLeaf<Offset, MemberType<Offset>>;

public:

    template<typename HeadType, typename... VarArgs>
    inline constexpr TTuple(HeadType&& HeadArg, VarArgs&&... Args)
        : TuplePrivate::TTupleImpl<0, HeadItem, TailItems...>(MoveIfPossible(HeadArg), MoveIfPossible(Args)...)
    {
    }

    inline constexpr TTuple(TTuple&& Other) = default;
    inline constexpr TTuple(const TTuple& Other) = default;

    template<uint64 Offset>
    inline constexpr MemberType<Offset>& Get()
    {
        return static_cast<LeafType<Offset>&>(*this).Member;
    }

    template<uint64 Offset>
    inline constexpr const MemberType<Offset>& Get() const
    {
        return static_cast<const LeafType<Offset>&>(*this).Member;
    }

    template<uint64 Offset>
    inline constexpr MemberType<Offset>& get()
    {
        return static_cast<LeafType<Offset>&>(*this).Member;
    }

    template<uint64 Offset>
    inline constexpr const MemberType<Offset>& get() const
    {
        return static_cast<const LeafType<Offset>&>(*this).Member;
    }

};

namespace TuplePrivate
{
    template<uint64 Offset, typename... Items>
    inline constexpr bool TupleCompare(const TTuple<Items...>& Left, const TTuple<Items...>& Right)
    {
        if constexpr(Offset == 0)
        {
            return Left.template Get<0>() == Right.template Get<0>();
        }
        else
        {
            return Left.template Get<Offset>() == Right.template Get<Offset>() && TupleCompare<Offset - 1, Items...>(Left, Right);
        }
    }
}

template<typename... Items>
inline constexpr bool operator==(const TTuple<Items...>& Left, const TTuple<Items...>& Right)
{
    return TuplePrivate::TupleCompare<sizeof...(Items) - 1, Items...>(Left, Right);
}

template<typename... Items>
TTuple(Items...)->TTuple<Items...>;

namespace std
{
    template<typename... Items>
    struct tuple_size<TTuple<Items...>>
    {
        static const constinit uint64 value{sizeof...(Items)};
    };

    template<uint64 Offset, typename... Items>
    struct tuple_element<Offset, TTuple<Items...>>
    {
        using type = typename TuplePrivate::MemberTypeHelper<Offset, Items...>::Type;
    };
}

