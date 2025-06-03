
#ifndef CAST_EXPORT_H
#define CAST_EXPORT_H

#ifdef _MSC_VER
#  define CAST_HELPER_DLL_IMPORT __declspec(dllimport)
#  define CAST_HELPER_DLL_EXPORT __declspec(dllexport)
#  define CAST_HELPER_DLL_LOCAL
#  define CAST_DEPRECATED __declspec(deprecated)
#else
#  define CAST_HELPER_DLL_IMPORT __attribute__((visibility("default")))
#  define CAST_HELPER_DLL_EXPORT __attribute__((visibility("default")))
#  define CAST_HELPER_DLL_LOCAL  __attribute__((visibility("hidden")))
#  define CAST_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifdef CAST_STATIC_DEFINE
#  define CAST_EXPORT
#  define CAST_NO_EXPORT
#else
#  ifndef CAST_EXPORT
#    ifdef cast_EXPORTS
        /* We are building this library */
#      define CAST_EXPORT CAST_HELPER_DLL_EXPORT
#    else
        /* We are using this library */
#      define CAST_EXPORT CAST_HELPER_DLL_IMPORT
#    endif
#  endif

#  ifndef CAST_NO_EXPORT
#    define CAST_NO_EXPORT CAST_HELPER_DLL_LOCAL
#  endif
#endif

#ifndef CAST_DEPRECATED_EXPORT
#  define CAST_DEPRECATED_EXPORT CAST_EXPORT CAST_DEPRECATED
#endif

#ifndef CAST_DEPRECATED_NO_EXPORT
#  define CAST_DEPRECATED_NO_EXPORT CAST_NO_EXPORT CAST_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CAST_NO_DEPRECATED
#    define CAST_NO_DEPRECATED
#  endif
#endif

#endif /* CAST_EXPORT_H */
