//
// Created by max on 1/1/18.
//

#ifndef HOTSPOT_TYPEINFO_H
#define HOTSPOT_TYPEINFO_H

#include <exception>
#if __cplusplus >= 201103L
#include "hash_bytes.h"
#endif

#pragma GCC visibility push(default)

extern "C++" {

namespace __cxxabiv1
{
    class __class_type_info;
} // namespace __cxxabiv1

// Determine whether typeinfo names for the same type are merged (in which
// case comparison can just compare pointers) or not (in which case strings
// must be compared), and whether comparison is to be implemented inline or
// not.  We used to do inline pointer comparison by default if weak symbols
// are available, but even with weak symbols sometimes names are not merged
// when objects are loaded with RTLD_LOCAL, so now we always use strcmp by
// default.  For ABI compatibility, we do the strcmp inline if weak symbols
// are available, and out-of-line if not.  Out-of-line pointer comparison
// is used where the object files are to be portable to multiple systems,
// some of which may not be able to use pointer comparison, but the
// particular system for which libstdc++ is being built can use pointer
// comparison; in particular for most ARM EABI systems, where the ABI
// specifies out-of-line comparison.  The compiler's target configuration
// can override the defaults by defining __GXX_TYPEINFO_EQUALITY_INLINE to
// 1 or 0 to indicate whether or not comparison is inline, and
// __GXX_MERGED_TYPEINFO_NAMES to 1 or 0 to indicate whether or not pointer
// comparison can be used.

#ifndef __GXX_MERGED_TYPEINFO_NAMES
// By default, typeinfo names are not merged.
#define __GXX_MERGED_TYPEINFO_NAMES 0
#endif

// By default follow the old inline rules to avoid ABI changes.
#ifndef __GXX_TYPEINFO_EQUALITY_INLINE
#if !__GXX_WEAK__
#define __GXX_TYPEINFO_EQUALITY_INLINE 0
#else
#define __GXX_TYPEINFO_EQUALITY_INLINE 1
#endif
#endif

namespace std
{
    /**
     *  @brief  Part of RTTI.
     *
     *  The @c type_info class describes type information generated by
     *  an implementation.
    */
    class type_info
    {
    public:
        /** Destructor first. Being the first non-inline virtual function, this
         *  controls in which translation unit the vtable is emitted. The
         *  compiler makes use of that information to know where to emit
         *  the runtime-mandated type_info structures in the new-abi.  */
        virtual ~type_info();

        /** Returns an @e implementation-defined byte string; this is not
         *  portable between compilers!  */
        const char* name() const _GLIBCXX_NOEXCEPT
        { return __name[0] == '*' ? __name + 1 : __name; }

#if !__GXX_TYPEINFO_EQUALITY_INLINE
        // In old abi, or when weak symbols are not supported, there can
    // be multiple instances of a type_info object for one
    // type. Uniqueness must use the _name value, not object address.
    bool before(const type_info& __arg) const _GLIBCXX_NOEXCEPT;
    bool operator==(const type_info& __arg) const _GLIBCXX_NOEXCEPT;
#else
#if !__GXX_MERGED_TYPEINFO_NAMES
        /** Returns true if @c *this precedes @c __arg in the implementation's
         *  collation order.  */
        // Even with the new abi, on systems that support dlopen
        // we can run into cases where type_info names aren't merged,
        // so we still need to do string comparison.
        bool before(const type_info& __arg) const _GLIBCXX_NOEXCEPT
        { return (__name[0] == '*' && __arg.__name[0] == '*')
                 ? __name < __arg.__name
                 : __builtin_strcmp (__name, __arg.__name) < 0; }

        bool operator==(const type_info& __arg) const _GLIBCXX_NOEXCEPT
        {
            return ((__name == __arg.__name)
                    || (__name[0] != '*' &&
                        __builtin_strcmp (__name, __arg.__name) == 0));
        }
#else
        // On some targets we can rely on type_info's NTBS being unique,
    // and therefore address comparisons are sufficient.
    bool before(const type_info& __arg) const _GLIBCXX_NOEXCEPT
    { return __name < __arg.__name; }

    bool operator==(const type_info& __arg) const _GLIBCXX_NOEXCEPT
    { return __name == __arg.__name; }
#endif
#endif
        bool operator!=(const type_info& __arg) const _GLIBCXX_NOEXCEPT
        { return !operator==(__arg); }

#if __cplusplus >= 201103L
        size_t hash_code() const noexcept
        {
#  if !__GXX_MERGED_TYPEINFO_NAMES
            return _Hash_bytes(name(), __builtin_strlen(name()),
                               static_cast<size_t>(0xc70f6907UL));
#  else
            return reinterpret_cast<size_t>(__name);
#  endif
        }
#endif // C++11

        // Return true if this is a pointer type of some kind
        virtual bool __is_pointer_p() const;

        // Return true if this is a function type
        virtual bool __is_function_p() const;

        // Try and catch a thrown type. Store an adjusted pointer to the
        // caught type in THR_OBJ. If THR_TYPE is not a pointer type, then
        // THR_OBJ points to the thrown object. If THR_TYPE is a pointer
        // type, then THR_OBJ is the pointer itself. OUTER indicates the
        // number of outer pointers, and whether they were const
        // qualified.
        virtual bool __do_catch(const type_info *__thr_type, void **__thr_obj,
                                unsigned __outer) const;

        // Internally used during catch matching
        virtual bool __do_upcast(const __cxxabiv1::__class_type_info *__target,
                                 void **__obj_ptr) const;

    protected:
        const char *__name;

        explicit type_info(const char *__n): __name(__n) { }

    private:
        /// Assigning type_info is not supported.
        type_info& operator=(const type_info&);
        type_info(const type_info&);
    };

    /**
     *  @brief  Thrown during incorrect typecasting.
     *  @ingroup exceptions
     *
     *  If you attempt an invalid @c dynamic_cast expression, an instance of
     *  this class (or something derived from this class) is thrown.  */
    class bad_cast : public exception
    {
    public:
        bad_cast() _GLIBCXX_USE_NOEXCEPT { }

        // This declaration is not useless:
        // http://gcc.gnu.org/onlinedocs/gcc-3.0.2/gcc_6.html#SEC118
        virtual ~bad_cast() _GLIBCXX_USE_NOEXCEPT;

        // See comment in eh_exception.cc.
        virtual const char* what() const _GLIBCXX_USE_NOEXCEPT;
    };

    /**
     *  @brief Thrown when a NULL pointer in a @c typeid expression is used.
     *  @ingroup exceptions
     */
    class bad_typeid : public exception
    {
    public:
        bad_typeid () _GLIBCXX_USE_NOEXCEPT { }

        // This declaration is not useless:
        // http://gcc.gnu.org/onlinedocs/gcc-3.0.2/gcc_6.html#SEC118
        virtual ~bad_typeid() _GLIBCXX_USE_NOEXCEPT;

        // See comment in eh_exception.cc.
        virtual const char* what() const _GLIBCXX_USE_NOEXCEPT;
    };
} // namespace std

} // extern "C++"

#endif //HOTSPOT_TYPEINFO_H
