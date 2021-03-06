/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
typedef struct Stmt Stmt;
int db_finalize(Stmt *pStmt);
typedef struct Bag Bag;
int bag_count(Bag *p);
int db_column_int(Stmt *pStmt,int N);
int db_step(Stmt *pStmt);
int db_prepare(Stmt *pStmt,const char *zFormat,...);
void db_must_be_within_tree(void);
typedef struct Blob Blob;
struct Blob {
  unsigned int nUsed;            /* Number of bytes used in aData[] */
  unsigned int nAlloc;           /* Number of bytes allocated for aData[] */
  unsigned int iCursor;          /* Next character of input to parse */
  char *aData;                   /* Where the information is stored */
  void (*xRealloc)(Blob*, unsigned int); /* Function to reallocate the buffer */
};
struct Stmt {
  Blob sql;               /* The SQL for this statement */
  sqlite3_stmt *pStmt;    /* The results of sqlite3_prepare() */
  Stmt *pNext, *pPrev;    /* List of all unfinalized statements */
  int nStep;              /* Number of sqlite3_step() calls */
};
void verify_all_cmd(void);
void verify_cancel(void);
int bag_insert(Bag *p,int e);
void db_commit_hook(int(*x)(void),int sequence);
void verify_before_commit(int rid);
void bag_clear(Bag *p);
int bag_next(Bag *p,int e);
int bag_first(Bag *p);
void content_clear_cache(void);
struct Bag {
  int cnt;   /* Number of integers in the bag */
  int sz;    /* Number of slots in a[] */
  int used;  /* Number of used slots in a[] */
  int *a;    /* Hash table of integers that are in the bag */
};
int blob_compare(Blob *pA,Blob *pB);
void blob_reset(Blob *pBlob);
int sha1sum_blob(const Blob *pIn,Blob *pCksum);
int content_get(int rid,Blob *pBlob);
NORETURN void fossil_fatal(const char *zFormat,...);
#if defined(FOSSIL_ENABLE_TCL)
#include "tcl.h"
#endif
#if defined(FOSSIL_ENABLE_JSON)
#include "cson_amalgamation.h"
#include "json_detail.h"
#endif
#define UUID_SIZE 40
#define blob_size(X)  ((X)->nUsed)
void db_blob(Blob *pResult,const char *zSql,...);
void blob_zero(Blob *pBlob);
int content_size(int rid,int dflt);
