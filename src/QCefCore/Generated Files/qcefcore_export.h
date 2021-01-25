
#ifndef QCEFCORE_EXPORT_H
#define QCEFCORE_EXPORT_H

#ifdef QCEFCORE_STATIC_DEFINE
#  define QCEFCORE_EXPORT
#  define QCEFCORE_NO_EXPORT
#else
#  ifndef QCEFCORE_EXPORT
#    ifdef QCefCore_EXPORTS
        /* We are building this library */
#      define QCEFCORE_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define QCEFCORE_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef QCEFCORE_NO_EXPORT
#    define QCEFCORE_NO_EXPORT 
#  endif
#endif

#ifndef QCEFCORE_DEPRECATED
#  define QCEFCORE_DEPRECATED __declspec(deprecated)
#endif

#ifndef QCEFCORE_DEPRECATED_EXPORT
#  define QCEFCORE_DEPRECATED_EXPORT QCEFCORE_EXPORT QCEFCORE_DEPRECATED
#endif

#ifndef QCEFCORE_DEPRECATED_NO_EXPORT
#  define QCEFCORE_DEPRECATED_NO_EXPORT QCEFCORE_NO_EXPORT QCEFCORE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef QCEFCORE_NO_DEPRECATED
#    define QCEFCORE_NO_DEPRECATED
#  endif
#endif

#endif /* QCEFCORE_EXPORT_H */
