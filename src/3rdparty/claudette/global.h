#pragma once

#ifndef FOUG_DECL_EXPORT
 #if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) \
     || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
 # define FOUG_DECL_EXPORT __declspec(dllexport)
 # define FOUG_DECL_IMPORT __declspec(dllimport)
 #else
 # define FOUG_DECL_EXPORT
 # define FOUG_DECL_IMPORT
 #endif /* WIN */
#endif /* !FOUG_DECL_EXPORT */

#ifdef CLAUDETTE_LIB_DLL
# ifdef CLAUDETTE_LIB_MAKE_DLL
#  define CLAUDETTE_LIB_EXPORT FOUG_DECL_EXPORT
# else
#  define CLAUDETTE_LIB_EXPORT FOUG_DECL_IMPORT
# endif /* CLAUDETTE_LIB_MAKE_DLL */
#else
# define CLAUDETTE_LIB_EXPORT
#endif /* CLAUDETTE_LIB_DLL */
