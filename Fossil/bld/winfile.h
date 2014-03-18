/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
void fossil_free(void *p);
char *fossil_filename_to_utf8(const void *zFilename);
NORETURN void fossil_fatal(const char *zFormat,...);
void *fossil_malloc(size_t n);
#if defined(_WIN32)
void win32_getcwd(char *zBuf,int nBuf);
int win32_chdir(const char *zChDir,int bChroot);
int win32_access(const char *zFilename,int flags);
#endif
void fossil_filename_free(void *pOld);
void *fossil_utf8_to_filename(const char *zUtf8);
#if defined(_WIN32) && (defined(__MSVCRT__) || defined(_MSC_VER))
typedef struct fossilStat fossilStat;
#endif
#include <dirent.h>
#if defined(_WIN32) && (defined(__MSVCRT__) || defined(_MSC_VER))
struct fossilStat {
    i64 st_size;
    i64 st_mtime;
    int st_mode;
};
#endif
#if defined(_WIN32)
int win32_stat(const char *zFilename,struct fossilStat *buf,int isWd);
#endif
