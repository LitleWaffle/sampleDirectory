/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#if defined(FOSSIL_ENABLE_TCL)
#include "tcl.h"
#endif
#if defined(FOSSIL_ENABLE_JSON)
#include "cson_amalgamation.h"
#include "json_detail.h"
#endif
void usage(const char *zFormat);
#if defined(FOSSIL_ENABLE_JSON) && defined(FOSSIL_ENABLE_JSON)
void json_cmd_top(void);
void json_page_top(void);
#endif
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_page_wiki();
cson_value *json_page_whoami();
cson_value *json_page_timeline();
cson_value *json_page_status();
cson_value *json_page_status();
cson_value *json_page_finfo();
cson_value *json_page_finfo();
cson_value *json_page_config();
cson_value *json_page_config();
cson_value *json_page_user();
cson_value *json_page_user();
cson_value *json_page_tag();
cson_value *json_page_tag();
cson_value *json_page_report();
cson_value *json_page_report();
cson_value *json_page_query();
cson_value *json_page_query();
cson_value *json_page_logout();
cson_value *json_page_logout();
cson_value *json_page_login();
cson_value *json_page_login();
cson_value *json_page_dir();
cson_value *json_page_dir();
cson_value *json_page_diff();
cson_value *json_page_diff();
cson_value *json_page_branch();
cson_value *json_page_branch();
cson_value *json_page_artifact();
cson_value *json_page_artifact();
cson_value *json_page_anon_password();
cson_value *json_page_anon_password();
#endif
void db_end_transaction(int rollbackFlag);
int rebuild_db(int randomize,int doOut,int doClustering);
void db_begin_transaction(void);
void db_open_repository(const char *zDbName);
void db_close(int reportErrors);
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_page_dispatch_helper(JsonPageDef const *pages);
#endif
typedef struct Blob Blob;
void blob_init(Blob *pBlob,const char *zData,int size);
#if defined(FOSSIL_ENABLE_JSON)
void json_dispatch_missing_args_err(JsonPageDef const *pCommands,char const *zErrPrefix);
#endif
char *db_text(char const *zDefault,const char *zSql,...);
const char *db_name(const char *zDb);
typedef struct Stmt Stmt;
int db_column_int(Stmt *pStmt,int N);
i64 db_column_int64(Stmt *pStmt,int N);
int db_int(int iDflt,const char *zSql,...);
#include <dirent.h>
i64 file_size(const char *zFilename);
char *db_get(const char *zName,char *zDefault);
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_page_stat();
cson_value *json_page_cap();
#endif
#define RELEASE_VERSION_NUMBER 128
#define RELEASE_VERSION "1.28"
#define MANIFEST_YEAR "2014"
#define MANIFEST_DATE "2014-01-27 17:33:44"
#define MANIFEST_VERSION "[3d49f04587]"
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_page_version();
cson_value *json_page_resultCodes();
cson_value *json_value_to_bool(cson_value const *zVal);
#endif
char *info_tags_of_checkin(int rid,int propagatingOnly);
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_tags_for_checkin_rid(int rid,char propagatingOnly);
#endif
int db_finalize(Stmt *pStmt);
char *blob_str(Blob *p);
int db_prepare(Stmt *pStmt,const char *zFormat,...);
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
extern const struct Stmt empty_Stmt;
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_sql_to_array_of_obj(Blob *pSql,cson_array *pTgt,char resetBlob);
cson_value *json_stmt_to_array_of_values(Stmt *pStmt,int resultColumn,cson_array *pTgt);
cson_value *json_stmt_to_array_of_array(Stmt *pStmt,cson_array *pTgt);
#endif
int db_step(Stmt *pStmt);
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_stmt_to_array_of_obj(Stmt *pStmt,cson_array *pTgt);
#endif
char *mprintf(const char *zFormat,...);
#if defined(FOSSIL_ENABLE_JSON)
int json_set_err(int code,char const *fmt,...);
void json_err(int code,char const *msg,char alsoOutput);
#endif
sqlite3_uint64 fossil_timer_stop(int timerId);
int fossil_timer_is_active(int timerId);
#define MANIFEST_UUID "3d49f045879456e9b9ce96be213a3e3411f448d0"
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_cap_value();
cson_value *json_g_to_json();
#endif
void blob_reset(Blob *pBlob);
#define blob_buffer(X)  ((X)->aData)
#define blob_size(X)  ((X)->nUsed)
void blob_appendf(Blob *pBlob,const char *zFormat,...);
void fossil_warning(const char *zFormat,...);
extern const Blob empty_blob;
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_new_timestamp(cson_int_t timeVal);
cson_int_t json_timestamp();
#endif
i64 db_int64(i64 iDflt,const char *zSql,...);
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_julian_to_timestamp(double j);
JsonPageDef const *json_handler_for_name(char const *name,JsonPageDef const *head);
char const *json_auth_token_cstr();
#endif
#define OPEN_ANY_SCHEMA      0x002      /* Do not error if schema is wrong */
void db_find_and_open_repository(int bFlags,int nArgUsed);
void login_check_credentials(void);
#if defined(FOSSIL_ENABLE_JSON)
void cgi_parse_POST_JSON(FILE *zIn,unsigned int contentLen);
#endif
NORETURN void fossil_fatal(const char *zFormat,...);
FILE *fossil_fopen(const char *zName,const char *zMode);
void cgi_set_content_type(const char *zType);
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_string_split2(char const *zStr,char separator,char doDeHttp);
#endif
void fossil_free(void *p);
int dehttpize(char *z);
void *fossil_malloc(size_t n);
int fossil_isspace(char c);
#if defined(FOSSIL_ENABLE_JSON)
int json_string_split(char const *zStr,char separator,char doDeHttp,cson_array *target);
void json_warn(int code,char const *fmt,...);
#endif
int fossil_timer_start();
#if defined(FOSSIL_ENABLE_JSON)
void json_main_bootstrap();
cson_value *json_req_payload_get(char const *pKey);
#endif
const char *cgi_parameter(const char *zName,const char *zDefault);
#define P(x)        cgi_parameter((x),0)
void cgi_replace_parameter(const char *zName,const char *zValue);
char *login_cookie_name(void);
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_auth_token();
#endif
void cgi_printf(const char *zFormat,...);
void cgi_reset_content(void);
#if defined(FOSSIL_ENABLE_JSON)
void json_send_response(cson_value const *pResponse);
char const *json_guess_content_type();
int json_find_option_int(char const *zKey,char const *zCLILong,char const *zCLIShort,int dflt);
char json_find_option_bool(char const *zKey,char const *zCLILong,char const *zCLIShort,char dflt);
char const *json_find_option_cstr(char const *zKey,char const *zCLILong,char const *zCLIShort);
char const *json_command_arg(unsigned char ndx);
#endif
const char *find_option(const char *zLong,const char *zShort,int hasArg);
#if defined(FOSSIL_ENABLE_JSON)
char const *json_find_option_cstr2(char const *zKey,char const *zCLILong,char const *zCLIShort,int argPos);
char const *json_getenv_cstr(char const *zKey);
char json_getenv_bool(char const *pKey,char dflt);
int json_getenv_int(char const *pKey,int dflt);
int json_setenv(char const *zKey,cson_value *v);
#endif
char *fossil_getenv(const char *zName);
#define PD(x,y)     cgi_parameter((x),(y))
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_getenv(char const *zKey);
cson_value *json_new_int(i64 v);
#endif
char *vmprintf(const char *zFormat,va_list ap);
#if defined(FOSSIL_ENABLE_JSON)
cson_value *json_new_string_f(char const *fmt,...);
cson_value *json_new_string(char const *str);
cson_value *json_rc_string(int code);
#endif
NORETURN void fossil_exit(int rc);
#if defined(FOSSIL_ENABLE_JSON)
void json_gc_add(char const *key,cson_value *v);
char const *json_rc_cstr(int code);
#endif
void cgi_append_content(const char *zData,int nAmt);
#if defined(FOSSIL_ENABLE_JSON)
int cson_data_dest_cgi(void *pState,void const *src,unsigned int n);
#endif
void blob_rewind(Blob *p);
#if defined(FOSSIL_ENABLE_JSON)
cson_value *cson_parse_Blob(Blob *pSrc,cson_parse_info *pInfo);
int cson_output_Blob(cson_value const *pVal,Blob *pDest,cson_output_opt const *pOpt);
#endif
unsigned int blob_read(Blob *pIn,void *pDest,unsigned int nLen);
#if defined(FOSSIL_ENABLE_JSON)
int cson_data_src_Blob(void *pState,void *dest,unsigned int *n);
#endif
void blob_append(Blob *pBlob,const char *aData,int nData);
#if defined(FOSSIL_ENABLE_JSON)
int cson_data_dest_Blob(void *pState,void const *src,unsigned int n);
cson_value *json_page_nyi();
#endif
typedef struct Global Global;
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
#if defined(FOSSIL_ENABLE_JSON)
char fossil_has_json();
extern const FossilJsonKeys_ FossilJsonKeys;
#endif
#define INTERFACE 0
