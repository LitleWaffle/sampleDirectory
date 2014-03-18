/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
int fossil_utf8_to_console(const char *zUtf8,int nByte,int toStdErr);
void fossil_filename_free(void *pOld);
#if defined(FOSSIL_ENABLE_TCL)
#include "tcl.h"
#endif
#if defined(FOSSIL_ENABLE_JSON)
#include "cson_amalgamation.h"
#include "json_detail.h"
#endif
#define count(X)  (sizeof(X)/sizeof(X[0]))
int fossil_isalpha(char c);
void *fossil_utf8_to_filename(const char *zUtf8);
void *fossil_malloc(size_t n);
char *fossil_filename_to_utf8(const void *zFilename);
void fossil_free(void *p);
void fossil_unicode_free(void *pOld);
void *fossil_utf8_to_unicode(const char *zUtf8);
char *fossil_strdup(const char *zOrig);
char *fossil_unicode_to_utf8(const void *zUnicode);
#if defined(_WIN32)
void fossil_mbcs_free(char *zOld);
char *fossil_mbcs_to_utf8(const char *zMbcs);
#endif
