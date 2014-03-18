/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
int fossil_strnicmp(const char *zA,const char *zB,int nByte);
void test_without_rowid(void);
void test_timespan_cmd(void);
char *db_timespan_name(double rSpan);
void manifest_to_disk(int vid);
void setting_cmd(void);
void info_cmd(void);
void checkout_cmd(void);
extern const char zLocalSchema[];
void url_proxy_options(void);
void cmd_open(void);
void db_record_repository_filename(const char *zName);
int db_table_has_column(char const *zTableName,char const *zColName);
void db_lset_int(const char *zName,int value);
int db_lget_int(const char *zName,int dflt);
int db_get_int(const char *zName,int dflt);
void db_unset(const char *zName,int globalFlag);
char *db_get_mtime(const char *zName,char *zFormat,char *zDefault);
typedef struct stControlSettings stControlSettings;
struct stControlSettings {
  char const *name;     /* Name of the setting */
  char const *var;      /* Internal variable name used by db_set() */
  int width;            /* Width of display.  0 for boolean values */
  int versionable;      /* Is this setting versionable? */
  char const *def;      /* Default value */
};
void *fossil_malloc(size_t n);
typedef struct Blob Blob;
int blob_trim(Blob *p);
int blob_read_from_file(Blob *pBlob,const char *zFilename);
char *db_get_do_versionable(const char *zName,char *zNonVersionedSetting);
void db_swap_connections(void);
int is_false(const char *zVal);
int fossil_stricmp(const char *zA,const char *zB);
int is_truth(const char *zVal);
char *db_reveal(const char *zKey);
char *sha1sum_finish(Blob *pOut);
void sha1sum_step_text(const char *zText,int nBytes);
int validate16(const char *zIn,int nIn);
char *db_conceal(const char *zContent,int n);
const char *cgi_parameter(const char *zName,const char *zDefault);
#define P(x)        cgi_parameter((x),0)
void fossil_print(const char *zFormat,...);
void create_repository_cmd(void);
#define MC_NONE           0  /*  default handling           */
int manifest_crosslink(int rid,Blob *pContent,int flags);
int content_put(Blob *pBlob);
int md5sum_blob(const Blob *pIn,Blob *pCksum);
char *md5sum_finish(Blob *pOut);
void md5sum_init(void);
char *date_in_standard_format(const char *zInputDate);
#define CONFIGSET_ALL       0x0000ff     /* Everything */
const char *configure_inop_rhs(int iMask);
void user_select(void);
void db_set_int(const char *zName,int value,int globalFlag);
int db_is_global(const char *zName);
#if defined(FOSSIL_ENABLE_TCL)
#include "tcl.h"
#endif
#if defined(FOSSIL_ENABLE_JSON)
#include "cson_amalgamation.h"
#include "json_detail.h"
#endif
const char *get_version();
#define CONTENT_SCHEMA  "2"
void db_set(const char *zName,const char *zValue,int globalFlag);
void db_initial_setup(const char *zTemplate,const char *zInitialDate,const char *zDefaultUser,int makeServerCodes);
void blob_appendf(Blob *pBlob,const char *zFormat,...);
extern struct stControlSettings const ctrlSettings[];
const char *db_setting_inop_rhs();
void db_create_default_users(int setupUserOnly,const char *zDefaultUser);
extern const char zRepositorySchema2[];
extern const char zRepositorySchemaDefaultReports[];
extern const char zRepositorySchema1[];
void db_create_repository(const char *zFilename);
void db_must_be_within_tree(void);
void db_lset(const char *zName,const char *zValue);
#include <dirent.h>
void file_canonical_name(const char *zOrigName,Blob *pOut,int slash);
void usage(const char *zFormat);
void move_repo_cmd(void);
char *db_get(const char *zName,char *zDefault);
#define AUX_SCHEMA      "2011-04-25 19:50"
int db_schema_is_outofdate(void);
int fossil_strcmp(const char *zA,const char *zB);
void db_verify_schema(void);
const char *find_option(const char *zLong,const char *zShort,int hasArg);
void db_find_and_open_repository(int bFlags,int nArgUsed);
#define OPEN_ANY_SCHEMA      0x002      /* Do not error if schema is wrong */
#define OPEN_OK_NOT_FOUND    0x001      /* Do not error out if not found */
int db_get_boolean(const char *zName,int dflt);
int file_is_absolute_path(const char *zPath);
char *db_lget(const char *zName,char *zDefault);
const char *db_repository_filename(void);
void db_open_repository(const char *zDbName);
void file_getcwd(char *zBuf,int nBuf);
int db_open_local(const char *zDbName);
void fossil_free(void *p);
int strglob(const char *zGlob,const char *z);
const char *db_name(const char *zDb);
extern const char zConfigSchema[];
i64 file_size(const char *zFilename);
int file_access(const char *zFilename,int flags);
int file_isdir(const char *zFilename);
char *fossil_getenv(const char *zName);
void db_open_config(int useAttach);
void db_open_or_attach(const char *zDbName,const char *zLabel,int *pWasAttached);
void db_attach(const char *zDbName,const char *zLabel);
void db_detach(const char *zLabel);
int re_add_sql_func(sqlite3 *db);
#define LOCAL static
LOCAL void db_sql_trace(void *notUsed,const char *zSql);
LOCAL void file_is_selected(sqlite3_context *context,int argc,sqlite3_value **argv);
LOCAL void db_sql_print(sqlite3_context *context,int argc,sqlite3_value **argv);
LOCAL void db_sql_cgi(sqlite3_context *context,int argc,sqlite3_value **argv);
LOCAL void db_sql_user(sqlite3_context *context,int argc,sqlite3_value **argv);
void fossil_trace(const char *zFormat,...);
void *fossil_utf8_to_filename(const char *zUtf8);
int mtime_of_manifest_file(int vid,int fid,i64 *pMTime);
void db_checkin_mtime_function(sqlite3_context *context,int argc,sqlite3_value **argv);
void db_now_function(sqlite3_context *context,int argc,sqlite3_value **argv);
LOCAL sqlite3 *db_open(const char *zDbName);
void db_init_database(const char *zFileName,const char *zSchema,...);
char *db_text(char const *zDefault,const char *zSql,...);
void db_blob(Blob *pResult,const char *zSql,...);
double db_double(double rDflt,const char *zSql,...);
int db_exists(const char *zSql,...);
int db_int(int iDflt,const char *zSql,...);
i64 db_int64(i64 iDflt,const char *zSql,...);
int db_is_writeable(const char *zName);
void db_optional_sql(const char *zDb,const char *zSql,...);
typedef struct Stmt Stmt;
int db_exec(Stmt *pStmt);
void blob_init(Blob *pBlob,const char *zData,int size);
void db_ephemeral_blob(Stmt *pStmt,int N,Blob *pBlob);
void blob_append(Blob *pBlob,const char *aData,int nData);
void db_column_blob(Stmt *pStmt,int N,Blob *pBlob);
char *mprintf(const char *zFormat,...);
char *db_column_malloc(Stmt *pStmt,int N);
int db_column_count(Stmt *pStmt);
const char *db_column_name(Stmt *pStmt,int N);
const char *db_column_raw(Stmt *pStmt,int N);
const char *db_column_text(Stmt *pStmt,int N);
double db_column_double(Stmt *pStmt,int N);
i64 db_column_int64(Stmt *pStmt,int N);
int db_column_int(Stmt *pStmt,int N);
int db_column_bytes(Stmt *pStmt,int N);
int db_changes(void);
NORETURN void fossil_fatal(const char *zFormat,...);
int db_last_insert_rowid(void);
void blob_reset(Blob *pBlob);
void db_check_result(int rc);
int db_reset(Stmt *pStmt);
void fossil_warning(const char *zFormat,...);
int db_step(Stmt *pStmt);
int db_bind_str(Stmt *pStmt,const char *zParamName,Blob *pBlob);
#define blob_buffer(X)  ((X)->aData)
int db_bind_blob(Stmt *pStmt,const char *zParamName,Blob *pBlob);
int db_bind_null(Stmt *pStmt,const char *zParamName);
int db_bind_text(Stmt *pStmt,const char *zParamName,const char *zValue);
int db_bind_double(Stmt *pStmt,const char *zParamName,double rValue);
int db_bind_int64(Stmt *pStmt,const char *zParamName,i64 iValue);
int db_bind_int(Stmt *pStmt,const char *zParamName,int iValue);
#define blob_size(X)  ((X)->nUsed)
int db_static_prepare(Stmt *pStmt,const char *zFormat,...);
int db_prepare_ignore_error(Stmt *pStmt,const char *zFormat,...);
int db_prepare(Stmt *pStmt,const char *zFormat,...);
char *blob_str(Blob *p);
void blob_vappendf(Blob *pBlob,const char *zFormat,va_list ap);
void blob_zero(Blob *pBlob);
int db_vprepare(Stmt *pStmt,int errOk,const char *zFormat,va_list ap);
void db_commit_hook(int(*x)(void),int sequence);
int file_delete(const char *zFilename);
void db_close(int reportErrors);
void undo_rollback(void);
int db_finalize(Stmt *pStmt);
void leaf_do_pending_checks(void);
void db_end_transaction(int rollbackFlag);
int db_multi_exec(const char *zSql,...);
void db_begin_transaction(void);
NORETURN void fossil_panic(const char *zFormat,...);
char *fossil_strdup(const char *zOrig);
#define count(X)  (sizeof(X)/sizeof(X[0]))
void db_delete_on_failure(const char *zFilename);
NORETURN void fossil_exit(int rc);
void db_force_rollback(void);
void cgi_reply(void);
void cgi_printf(const char *zFormat,...);
void cgi_reset_content(void);
#if defined(FOSSIL_ENABLE_JSON)
void json_err(int code,char const *msg,char alsoOutput);
#endif
typedef struct Global Global;
struct Blob {
  unsigned int nUsed;            /* Number of bytes used in aData[] */
  unsigned int nAlloc;           /* Number of bytes allocated for aData[] */
  unsigned int iCursor;          /* Next character of input to parse */
  char *aData;                   /* Where the information is stored */
  void (*xRealloc)(Blob*, unsigned int); /* Function to reallocate the buffer */
};
typedef struct Th_Interp Th_Interp;
typedef struct FossilUserPerms FossilUserPerms;
struct FossilUserPerms {
  char Setup;            /* s: use Setup screens on web interface */
  char Admin;            /* a: administrative permission */
  char Delete;           /* d: delete wiki or tickets */
  char Password;         /* p: change password */
  char Query;            /* q: create new reports */
  char Write;            /* i: xfer inbound. checkin */
  char Read;             /* o: xfer outbound. checkout */
  char Hyperlink;        /* h: enable the display of hyperlinks */
  char Clone;            /* g: clone */
  char RdWiki;           /* j: view wiki via web */
  char NewWiki;          /* f: create new wiki via web */
  char ApndWiki;         /* m: append to wiki via web */
  char WrWiki;           /* k: edit wiki via web */
  char ModWiki;          /* l: approve and publish wiki content (Moderator) */
  char RdTkt;            /* r: view tickets via web */
  char NewTkt;           /* n: create new tickets */
  char ApndTkt;          /* c: append to tickets via the web */
  char WrTkt;            /* w: make changes to tickets via web */
  char ModTkt;           /* q: approve and publish ticket changes (Moderator) */
  char Attach;           /* b: add attachments */
  char TktFmt;           /* t: create new ticket report formats */
  char RdAddr;           /* e: read email addresses or other private data */
  char Zip;              /* z: download zipped artifact via /zip URL */
  char Private;          /* x: can send and receive private content */
};
#if defined(FOSSIL_ENABLE_TCL)
typedef struct TclContext TclContext;
struct TclContext {
  int argc;              /* Number of original (expanded) arguments. */
  char **argv;           /* Full copy of the original (expanded) arguments. */
  void *library;         /* The Tcl library module handle. */
  void *xFindExecutable; /* See tcl_FindExecutableProc in th_tcl.c. */
  void *xCreateInterp;   /* See tcl_CreateInterpProc in th_tcl.c. */
  void *xDeleteInterp;   /* See tcl_DeleteInterpProc in th_tcl.c. */
  void *xFinalize;       /* See tcl_FinalizeProc in th_tcl.c. */
  Tcl_Interp *interp;    /* The on-demand created Tcl interpreter. */
  int useObjProc;        /* Non-zero if an objProc can be called directly. */
  char *setup;           /* The optional Tcl setup script. */
  void *xPreEval;        /* Optional, called before Tcl_Eval*(). */
  void *pPreContext;     /* Optional, provided to xPreEval(). */
  void *xPostEval;       /* Optional, called after Tcl_Eval*(). */
  void *pPostContext;    /* Optional, provided to xPostEval(). */
};
#endif
#define MX_AUX  5
struct Global {
  int argc; char **argv;  /* Command-line arguments to the program */
  char *nameOfExe;        /* Full path of executable. */
  const char *zErrlog;    /* Log errors to this file, if not NULL */
  int isConst;            /* True if the output is unchanging */
  const char *zVfsName;   /* The VFS to use for database connections */
  sqlite3 *db;            /* The connection to the databases */
  sqlite3 *dbConfig;      /* Separate connection for global_config table */
  int useAttach;          /* True if global_config is attached to repository */
  const char *zConfigDbName;/* Path of the config database. NULL if not open */
  sqlite3_int64 now;      /* Seconds since 1970 */
  int repositoryOpen;     /* True if the main repository database is open */
  char *zRepositoryName;  /* Name of the repository database */
  const char *zMainDbType;/* "configdb", "localdb", or "repository" */
  const char *zConfigDbType;  /* "configdb", "localdb", or "repository" */
  int localOpen;          /* True if the local database is open */
  char *zLocalRoot;       /* The directory holding the  local database */
  int minPrefix;          /* Number of digits needed for a distinct UUID */
  int fSqlTrace;          /* True if --sqltrace flag is present */
  int fSqlStats;          /* True if --sqltrace or --sqlstats are present */
  int fSqlPrint;          /* True if -sqlprint flag is present */
  int fQuiet;             /* True if -quiet flag is present */
  int fHttpTrace;         /* Trace outbound HTTP requests */
  int fSystemTrace;       /* Trace calls to fossil_system(), --systemtrace */
  int fSshTrace;          /* Trace the SSH setup traffic */
  int fSshClient;         /* HTTP client flags for SSH client */
  char *zSshCmd;          /* SSH command string */
  int fNoSync;            /* Do not do an autosync ever.  --nosync */
  char *zPath;            /* Name of webpage being served */
  char *zExtra;           /* Extra path information past the webpage name */
  char *zBaseURL;         /* Full text of the URL being served */
  char *zTop;             /* Parent directory of zPath */
  const char *zContentType;  /* The content type of the input HTTP request */
  int iErrPriority;       /* Priority of current error message */
  char *zErrMsg;          /* Text of an error message */
  int sslNotAvailable;    /* SSL is not available.  Do not redirect to https: */
  Blob cgiIn;             /* Input to an xfer www method */
  int cgiOutput;          /* Write error and status messages to CGI */
  int xferPanic;          /* Write error messages in XFER protocol */
  int fullHttpReply;      /* True for full HTTP reply.  False for CGI reply */
  Th_Interp *interp;      /* The TH1 interpreter */
  char *th1Setup;         /* The TH1 post-creation setup script, if any */
  FILE *httpIn;           /* Accept HTTP input from here */
  FILE *httpOut;          /* Send HTTP output here */
  int xlinkClusterOnly;   /* Set when cloning.  Only process clusters */
  int fTimeFormat;        /* 1 for UTC.  2 for localtime.  0 not yet selected */
  int *aCommitFile;       /* Array of files to be committed */
  int markPrivate;        /* All new artifacts are private if true */
  int clockSkewSeen;      /* True if clocks on client and server out of sync */
  int wikiFlags;          /* Wiki conversion flags applied to %w and %W */
  char isHTTP;            /* True if server/CGI modes, else assume CLI. */
  char javascriptHyperlink; /* If true, set href= using script, not HTML */
  Blob httpHeader;        /* Complete text of the HTTP request header */

  /*
  ** NOTE: These members MUST be kept in sync with those in the "UrlData"
  **       structure defined in "url.c".
  */
  int urlIsFile;          /* True if a "file:" url */
  int urlIsHttps;         /* True if a "https:" url */
  int urlIsSsh;           /* True if an "ssh:" url */
  char *urlName;          /* Hostname for http: or filename for file: */
  char *urlHostname;      /* The HOST: parameter on http headers */
  char *urlProtocol;      /* "http" or "https" */
  int urlPort;            /* TCP port number for http: or https: */
  int urlDfltPort;        /* The default port for the given protocol */
  char *urlPath;          /* Pathname for http: */
  char *urlUser;          /* User id for http: */
  char *urlPasswd;        /* Password for http: */
  char *urlCanonical;     /* Canonical representation of the URL */
  char *urlProxyAuth;     /* Proxy-Authorizer: string */
  char *urlFossil;        /* The fossil query parameter on ssh: */
  unsigned urlFlags;      /* Boolean flags controlling URL processing */

  const char *zLogin;     /* Login name.  "" if not logged in. */
  const char *zSSLIdentity;  /* Value of --ssl-identity option, filename of
                             ** SSL client identity */
  int useLocalauth;       /* No login required if from 127.0.0.1 */
  int noPswd;             /* Logged in without password (on 127.0.0.1) */
  int userUid;            /* Integer user id */
  int isHuman;            /* True if access by a human, not a spider or bot */

  /* Information used to populate the RCVFROM table */
  int rcvid;              /* The rcvid.  0 if not yet defined. */
  char *zIpAddr;          /* The remote IP address */
  char *zNonce;           /* The nonce used for login */

  /* permissions used by the server */
  struct FossilUserPerms perm;

#ifdef FOSSIL_ENABLE_TCL
  /* all Tcl related context necessary for integration */
  struct TclContext tcl;
#endif

  /* For defense against Cross-site Request Forgery attacks */
  char zCsrfToken[12];    /* Value of the anti-CSRF token */
  int okCsrf;             /* Anti-CSRF token is present and valid */

  int parseCnt[10];       /* Counts of artifacts parsed */
  FILE *fDebug;           /* Write debug information here, if the file exists */
  int thTrace;            /* True to enable TH1 debugging output */
  Blob thLog;             /* Text of the TH1 debugging output */

  int isHome;             /* True if rendering the "home" page */

  /* Storage for the aux() and/or option() SQL function arguments */
  int nAux;                    /* Number of distinct aux() or option() values */
  const char *azAuxName[MX_AUX]; /* Name of each aux() or option() value */
  char *azAuxParam[MX_AUX];      /* Param of each aux() or option() value */
  const char *azAuxVal[MX_AUX];  /* Value of each aux() or option() value */
  const char **azAuxOpt[MX_AUX]; /* Options of each option() value */
  int anAuxCols[MX_AUX];         /* Number of columns for option() values */

  int allowSymlinks;             /* Cached "allow-symlinks" option */

  int mainTimerId;               /* Set to fossil_timer_start() */
#ifdef FOSSIL_ENABLE_JSON
  struct FossilJsonBits {
    int isJsonMode;            /* True if running in JSON mode, else
                                  false. This changes how errors are
                                  reported. In JSON mode we try to
                                  always output JSON-form error
                                  responses and always exit() with
                                  code 0 to avoid an HTTP 500 error.
                               */
    int resultCode;            /* used for passing back specific codes
                               ** from /json callbacks. */
    int errorDetailParanoia;   /* 0=full error codes, 1=%10, 2=%100, 3=%1000 */
    cson_output_opt outOpt;    /* formatting options for JSON mode. */
    cson_value * authToken;    /* authentication token */
    char const * jsonp;        /* Name of JSONP function wrapper. */
    unsigned char dispatchDepth /* Tells JSON command dispatching
                                   which argument we are currently
                                   working on. For this purpose, arg#0
                                   is the "json" path/CLI arg.
                                */;
    struct {                   /* "garbage collector" */
      cson_value * v;
      cson_array * a;
    } gc;
    struct {                   /* JSON POST data. */
      cson_value * v;
      cson_array * a;
      int offset;              /* Tells us which PATH_INFO/CLI args
                                  part holds the "json" command, so
                                  that we can account for sub-repos
                                  and path prefixes.  This is handled
                                  differently for CLI and CGI modes.
                               */
      char const * commandStr  /*"command" request param.*/;
    } cmd;
    struct {                   /* JSON POST data. */
      cson_value * v;
      cson_object * o;
    } post;
    struct {                   /* GET/COOKIE params in JSON mode. */
      cson_value * v;
      cson_object * o;
    } param;
    struct {
      cson_value * v;
      cson_object * o;
    } reqPayload;              /* request payload object (if any) */
    cson_array * warnings;     /* response warnings */
    int timerId;               /* fetched from fossil_timer_start() */
  } json;
#endif /* FOSSIL_ENABLE_JSON */
};
extern Global g;
char *vmprintf(const char *zFormat,va_list ap);
struct Stmt {
  Blob sql;               /* The SQL for this statement */
  sqlite3_stmt *pStmt;    /* The results of sqlite3_prepare() */
  Stmt *pNext, *pPrev;    /* List of all unfinalized statements */
  int nStep;              /* Number of sqlite3_step() calls */
};
extern const struct Stmt empty_Stmt;
void blobReallocMalloc(Blob *pBlob,unsigned int newSize);
#define BLOB_INITIALIZER  {0,0,0,0,blobReallocMalloc}
#define empty_Stmt_m {BLOB_INITIALIZER,NULL, NULL, NULL, 0}
#define INTERFACE 0
