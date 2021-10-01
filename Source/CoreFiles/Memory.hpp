#pragma once

#include "Definitions.hpp"
#include <malloc.h>
#ifdef __unix__
#include <sys/mman.h>

#endif

enum class EMemProtect : int32
{
    None = PROT_NONE,
    Read = PROT_READ,
    Write = PROT_WRITE,
    Exec = PROT_EXEC,
    GrowUp = PROT_GROWSUP,
    GrowDown = PROT_GROWSDOWN,
    ReadWrite = PROT_READ | PROT_WRITE
};

inline constexpr EMemProtect operator|(const EMemProtect Lhs, const EMemProtect Rhs)
{
    return static_cast<EMemProtect>(static_cast<int32>(Lhs) | static_cast<int32>(Rhs));
}

enum class EMapFlags : int32
{
    Anonymous = MAP_ANON,
    Private = MAP_PRIVATE,
    Shared = MAP_SHARED,
    SharedValidate = MAP_SHARED_VALIDATE,
    DenyWrite = MAP_DENYWRITE,
    Executable = MAP_EXECUTABLE,
    File = MAP_FILE,
    Fixed = MAP_FIXED,
    FixedNoReplace = MAP_FIXED_NOREPLACE,
    GrowsDown = MAP_GROWSDOWN,
    HugeLB = MAP_HUGETLB,
    HugeMask = MAP_HUGE_MASK,
    HugeShift = MAP_HUGE_SHIFT,
    Locked = MAP_LOCKED,
    NonBlock = MAP_NONBLOCK,
    Populate = MAP_POPULATE,
    Stack = MAP_STACK,
    Sync = MAP_SYNC,
    NoReserve = MAP_NORESERVE,
    Type = MAP_TYPE
};

inline constexpr EMapFlags operator|(const EMapFlags Lhs, const EMapFlags Rhs)
{
    return static_cast<EMapFlags>(static_cast<int32>(Lhs) | static_cast<int32>(Rhs));
}

enum class EReMapFlags : int32
{
    //DontUnmap = MREMAP_DONTUNMAP,
    Fixed = MREMAP_FIXED,
    MayMove = MREMAP_MAYMOVE
};

enum class EFetchPreparation : uint8
{
    Read = 0,
    Write = 1,
};

enum class ETemporalLocality : uint8
{
    None = 0,
    Low = 1,
    Moderate = 2,
    High = 3,
};

namespace Memory
{

    namespace Cache
    {
#ifdef __unix__
        extern const int64 L1LineSize;
        extern const int64 L2LineSize;
        extern const int64 L3LineSize;
        extern const int64 L4LineSize;

        extern const int64 L1Size;
        extern const int64 L2Size;
        extern const int64 L3Size;
        extern const int64 L4Size;
#elif defined(_WIN64)
        //todo
#endif
    }

    /**
     * \param StoredType type of the element to allocate
     * \param Num number of elements to allocate
     * \returns pointer to new memory allocation
     */
    template<typename StoredType = void>
    NODISCARD INLINE StoredType* Allocate(const uint64 Size)
    {
        return static_cast<StoredType*>(::malloc(Size));
    }

    /**
     * \param StoredType type of the element to allocate
     * \param Num number of elements to allocate
     * \returns pointer to new memory allocation
     */
    template<typename StoredType>
    NODISCARD INLINE StoredType* ZeroAllocate(const uint64 Num)
    {
        return static_cast<StoredType*>(::calloc(Num, sizeof(StoredType)));
    }

    /**
     * \param Source pointer to existing block of memory or nullptr
     * \param Size new size of memory block in bytes
     * \returns pointer to new location of memory block
     */
    template<typename StoredType = void>
    NODISCARD INLINE StoredType* ReAllocate(StoredType* Source, const uint64 Size)
    {
        return reinterpret_cast<StoredType*>(::realloc(Source, Size));
    }

   /**
    * \param Source pointer to memory allocated by Memory::Allocate or Memory::ZeroAllocate
    * \returns the size of the allocation in bytes (may be larger than the requested size)
    */
    NODISCARD INLINE uint64 AllocatedSize(void* Source)
    {
        return ::malloc_usable_size(Source);
    }

    /// \param Location pointer to existing block of memory
    INLINE void Free(void* const Location)
    {
        ::free(Location);
    }

#ifdef __unix__

    template<typename StoredType = void>
    NODISCARD INLINE StoredType* Map(StoredType* Address, uint64 Length, EMemProtect Protect, EMapFlags MapFlags, int32 FileDescriptor = -1, int32 Offset = 0)
    {
        return reinterpret_cast<StoredType*>(::mmap(Address, Length, static_cast<int32>(Protect), static_cast<int32>(MapFlags), FileDescriptor, Offset));
    }
    /*
     * Remap pages mapped by the range [ADDR,ADDR+OLD_LEN) to new length
     * NEW_LEN.  If MREMAP_MAYMOVE is set in FLAGS the returned address
     * may differ from ADDR.  If MREMAP_FIXED is set in FLAGS the function
     * takes another parameter which is a fixed address at which the block
     * resides after a successful call.
    */
    template<typename StoredType = void>
    NODISCARD INLINE StoredType* ReMapFixed(StoredType* Address, uint64 OldLength, uint64 NewLength)
    {
        return reinterpret_cast<StoredType*>(::mremap(Address, OldLength, NewLength, static_cast<int32>(EReMapFlags::Fixed), Address));
    }
    /*
    * Remap pages mapped by the range [ADDR,ADDR+OLD_LEN) to new length
    * NEW_LEN.  If MREMAP_MAYMOVE is set in FLAGS the returned address
    * may differ from ADDR.  If MREMAP_FIXED is set in FLAGS the function
    * takes another parameter which is a fixed address at which the block
    * resides after a successful call.
    */
    template<typename StoredType = void>
    NODISCARD INLINE StoredType* ReMapMove(StoredType* Address, uint64 OldLength, uint64 NewLength)
    {
        return reinterpret_cast<StoredType*>(::mremap(Address, OldLength, NewLength, static_cast<int32>(EReMapFlags::MayMove)));
    }
    /*
    * Deallocate any mapping for the region starting at ADDR and extending LEN
    * bytes.  Returns 0 if successful, -1 for errors (and sets errno)
    */
    template<typename StoredType = void>
    INLINE int32 UnMap(StoredType* Address, uint64 Length)
    {
        return ::munmap(Address, Length);
    }
#endif //__unix__
    //Copies count bytes from the object pointed to by Source to the object pointed to by Destination. Both objects are reinterpreted as arrays of unsigned char.
    template<typename TargetType, typename SourceType>
    INLINE constexpr void Copy(TargetType* Destination, const SourceType* Source, const uint64 Num)
    {
        __builtin_memcpy(Destination, Source, Num);
    }

    //Copies count bytes from the object pointed to by Source to the object pointed to by Destination using an intermediate buffer. Both objects are reinterpreted as arrays of unsigned char.
    template<typename TargetType, typename SourceType>
    INLINE constexpr void Move(TargetType* Destination, SourceType* Source, const uint64 Num)
    {
        __builtin_memmove(Destination, Source, Num);
    }

    template<typename TargetType>
    INLINE constexpr void Set(TargetType* Destination, const uint8 Value, const uint64 Num)
    {
        __builtin_memset(Destination, Value, Num);
    }

    template<EFetchPreparation FetchPreparation, ETemporalLocality TemporalLocality>
    INLINE constexpr void Prefetch(const void* const NONNULL Address)
    {
        __builtin_prefetch(Address, FetchPreparation, TemporalLocality);
    }

    template<EFetchPreparation FetchPreparation, ETemporalLocality TemporalLocality>
    INLINE constexpr void PrefetchRange(const char8* NONNULL Start, const uint64 ByteCount)
    {
        const char8* End{Start + ByteCount};

        #pragma unroll
        while(Start < End)
        {
            __builtin_prefetch(Start, FetchPreparation, TemporalLocality);
            Start += Cache::L1LineSize;
        }
    }

    template<EFetchPreparation FetchPreparation, ETemporalLocality TemporalLocality>
    INLINE constexpr void PrefetchRange(const char8* NONNULL Start, const char8* const NONNULL End)
    {
        #pragma unroll
        while(Start < End)
        {
            __builtin_prefetch(Start, FetchPreparation, TemporalLocality);
            Start += Cache::L1LineSize;
        }
    }

    template<typename TargetType, typename SourceType>
    INLINE constexpr int32 Compare(TargetType* Right, SourceType* Left, const uint64 Num)
    {
        return __builtin_memcmp(Right, Left, Num);
    }

    INLINE bool StringCompare(const char8* Right, const char8* Left)
    {
        return __builtin_strcmp(Right, Left) == 0;
    }

}
