/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
typedef struct Bag Bag;
void bag_clear(Bag *p);
int bag_next(Bag *p,int e);
int bag_first(Bag *p);
void leaf_do_pending_checks(void);
int bag_insert(Bag *p,int e);
void leaf_eventually_check(int rid);
# define TAG_CLOSED     9     /* Do not display this check-in as a leaf */
char *mprintf(const char *zFormat,...);
char *leaf_is_closed_sql(const char *zVar);
void leaf_check(int rid);
struct Bag {
  int cnt;   /* Number of integers in the bag */
  int sz;    /* Number of slots in a[] */
  int used;  /* Number of used slots in a[] */
  int *a;    /* Hash table of integers that are in the bag */
};
int db_multi_exec(const char *zSql,...);
void leaf_rebuild(void);
typedef struct Stmt Stmt;
int db_reset(Stmt *pStmt);
int db_column_int(Stmt *pStmt,int N);
int db_step(Stmt *pStmt);
int db_bind_int(Stmt *pStmt,const char *zParamName,int iValue);
int db_static_prepare(Stmt *pStmt,const char *zFormat,...);
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
int count_nonbranch_children(int pid);
# define TAG_BRANCH     8     /* Value is name of the current branch */
int db_int(int iDflt,const char *zSql,...);
int is_a_leaf(int rid);
