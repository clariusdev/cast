#ifndef LISTEN_EXPORT_H
#define LISTEN_EXPORT_H

#ifdef LISTEN_STATIC_DEFINE
#  define LISTEN_EXPORT
#  define LISTEN_NO_EXPORT
#else
#  ifndef LISTEN_EXPORT
#    ifdef listen_EXPORTS
      /* We are building this library */
#     ifdef _MSC_VER
#      define LISTEN_EXPORT __declspec(dllexport)
#     else
#       define LISTEN_EXPORT __attribute__((visibility("default")))
#     endif
#    else
      /* We are using this library */
#     ifdef _MSC_VER
#       define LISTEN_EXPORT __declspec(dllimport)
#     else
#       define LISTEN_EXPORT __attribute__((visibility("default")))
#     endif
#    endif
#  endif

#  ifndef LISTEN_NO_EXPORT
#    define LISTEN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef LISTEN_DEPRECATED
#  define LISTEN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef LISTEN_DEPRECATED_EXPORT
#  define LISTEN_DEPRECATED_EXPORT LISTEN_EXPORT LISTEN_DEPRECATED
#endif

#ifndef LISTEN_DEPRECATED_NO_EXPORT
#  define LISTEN_DEPRECATED_NO_EXPORT LISTEN_NO_EXPORT LISTEN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LISTEN_NO_DEPRECATED
#    define LISTEN_NO_DEPRECATED
#  endif
#endif

#endif /* LISTEN_EXPORT_H */
