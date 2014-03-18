/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#if defined(FOSSIL_ENABLE_TCL)
#include "tcl.h"
#endif
#if defined(FOSSIL_ENABLE_JSON)
#include "cson_amalgamation.h"
#include "json_detail.h"
#endif
void test_echo_cmd(void);
#if defined(_WIN32)
void win32_http_server(int mnPort,int mxPort,const char *zBrowser,const char *zStopper,const char *zNotFound,const char *zFileGlob,const char *zIpAddr,int flags);
int win32_http_service(int nPort,const char *zNotFound,const char *zFileGlob,int flags);
#endif
int cgi_http_server(int mnPort,int mxPort,const char *zBrowser,const char *zIpAddr,int flags);
int db_get_int(const char *zName,int dflt);
#define HTTP_SERVER_LOCALHOST      0x0001     /* Bind to 127.0.0.1 only */
#define HTTP_SERVER_SCGI           0x0002     /* SCGI instead of HTTP */
void cmd_webserver(void);
#include <dirent.h>
int file_access(const char *zFilename,int flags);
void cgi_set_parameter(const char *zName,const char *zValue);
void login_set_capabilities(const char *zCap,unsigned flags);
void Th_InitTraceLog();
void cmd_test_http(void);
#define CGI_SSH_COMPAT           0x0002     /* Compat for old SSH transport */
#define CGI_SSH_FOSSIL           0x0004     /* Use new Fossil SSH transport */
void cgi_handle_ssh_http_request(const char *zIpAddr);
void cgi_handle_http_request(const char *zIpAddr);
typedef struct Glob Glob;
void ssh_request_loop(const char *zIpAddr,Glob *FileGlob);
void cgi_handle_scgi_request(void);
#define CGI_SSH_CLIENT           0x0001     /* Client is SSH */
const char *cgi_ssh_remote_addr(const char *zDefault);
void cmd_http(void);
void db_must_be_within_tree(void);
void cgi_init(void);
NORETURN void cgi_panic(const char *zFormat,...);
Glob *glob_create(const char *zPatternList);
void *fossil_realloc(void *p,size_t n);
typedef struct Blob Blob;
int blob_trim(Blob *p);
int blob_tail(Blob *pFrom,Blob *pTo);
void cgi_setenv(const char *zName,const char *zValue);
void blob_reset(Blob *pBlob);
# define blob_eq(B,S) \
     ((B)->nUsed==sizeof(S)-1 && memcmp((B)->aData,S,sizeof(S)-1)==0)
int blob_token(Blob *pFrom,Blob *pTo);
void fossil_binary_mode(FILE *p);
void cmd_cgi(void);
int validate16(const char *zIn,int nIn);
int db_schema_is_outofdate(void);
void page_xfer(void);
void cgi_set_parameter_nocopy(const char *zName,const char *zValue,int isQP);
int dehttpize(char *z);
void login_as_user(const char *zUser);
int file_simplify_name(char *z,int n,int slash);
void login_check_credentials(void);
char *db_text(char const *zDefault,const char *zSql,...);
void db_open_repository(const char *zDbName);
void cgi_replace_parameter(const char *zName,const char *zValue);
void cgi_set_status(int iStat,const char *zStat);
#if defined(FOSSIL_ENABLE_JSON)
void json_err(int code,char const *msg,char alsoOutput);
#endif
NORETURN void cgi_redirect(const char *zURL);
void cgi_reply(void);
void cgi_set_content(Blob *pNewContent);
void cgi_set_content_type(const char *zType);
int blob_read_from_file(Blob *pBlob,const char *zFilename);
const char *mimetype_from_name(const char *zName);
int strglob(const char *zGlob,const char *z);
int glob_match(Glob *pGlob,const char *zString);
int file_isfile(const char *zFilename);
i64 file_size(const char *zFilename);
int fossil_isalnum(char c);
struct Glob {
  int nPattern;        /* Number of patterns */
  char **azPattern;    /* Array of pointers to patterns */
};
int file_isdir(const char *zFilename);
void file_canonical_name(const char *zOrigName,Blob *pOut,int slash);
char *db_get(const char *zName,char *zDefault);
NORETURN void cgi_redirectf(const char *zFormat,...);
NORETURN void fossil_redirect_home(void);
void db_optional_sql(const char *zDb,const char *zSql,...);
int db_multi_exec(const char *zSql,...);
int db_exists(const char *zSql,...);
int db_is_writeable(const char *zName);
int fossil_stricmp(const char *zA,const char *zB);
const char *cgi_parameter(const char *zName,const char *zDefault);
#define PD(x,y)     cgi_parameter((x),(y))
void test_all_help_page(void);
void style_footer(void);
void fossil_free(void *p);
char *mprintf(const char *zFormat,...);
void cgi_printf(const char *zFormat,...);
void style_submenu_element(const char *zLabel,const char *zTitle,const char *zLink,...);
void style_header(const char *zTitleFormat,...);
#define P(x)        cgi_parameter((x),0)
void help_page(void);
void help_cmd(void);
typedef struct Th_Interp Th_Interp;
const char *Th_GetResult(Th_Interp *,int *);
const char *Th_ReturnCodeName(int rc,int nullIfOk);
int Th_Eval(Th_Interp *interp,int iFrame,const char *zProg,int nProg);
typedef unsigned int u32;
#define TH_INIT_FORCE_TCL   ((u32)0x00000002) /* Force Tcl to be enabled? */
#define TH_INIT_NONE        ((u32)0x00000000) /* No flags. */
#define TH_INIT_DEFAULT     (TH_INIT_NONE)    /* Default flags. */
void Th_FossilInit(u32 flags);
#define AUX_SCHEMA      "2011-04-25 19:50"
void version_cmd(void);
const char *get_user_agent();
#define MANIFEST_DATE "2014-01-27 17:33:44"
#define MANIFEST_VERSION "[3d49f04587]"
#define RELEASE_VERSION "1.28"
const char *get_version();
void cmd_test_webpage_list(void);
void verify_all_options(void);
void usage(const char *zFormat);
void blob_appendf(Blob *pBlob,const char *zFormat,...);
void blob_zero(Blob *pBlob);
NORETURN void fossil_panic(const char *zFormat,...);
int is_valid_fd(int fd);
int file_chdir(const char *zChDir,int bChroot);
NORETURN void fossil_exit(int rc);
void fossil_print(const char *zFormat,...);
const char *find_option(const char *zLong,const char *zShort,int hasArg);
int fossil_timer_start();
#if defined(_WIN32) && !(defined(_WIN32) && !defined(BROKEN_MINGW_CMDLINE))
extern int _CRT_glob;
#endif
#if defined(_WIN32) && !defined(BROKEN_MINGW_CMDLINE)
extern int _dowildcard;
#endif
void fossil_warning(const char *zFormat,...);
char *fossil_strdup(const char *zOrig);
int fossil_isspace(char c);
#define blob_buffer(X)  ((X)->aData)
int blob_line(Blob *pFrom,Blob *pTo);
void blob_rewind(Blob *p);
void *fossil_malloc(size_t n);
char *blob_str(Blob *p);
void blob_to_utf8_no_bom(Blob *pBlob,int useMbcs);
int blob_read_from_channel(Blob *pBlob,FILE *in,int nToRead);
NORETURN void fossil_fatal(const char *zFormat,...);
FILE *fossil_fopen(const char *zName,const char *zMode);
char *fossil_filename_to_utf8(const void *zFilename);
#if defined(_WIN32)
char *fossil_mbcs_to_utf8(const char *zMbcs);
#endif
struct Blob {
  unsigned int nUsed;            /* Number of bytes used in aData[] */
  unsigned int nAlloc;           /* Number of bytes allocated for aData[] */
  unsigned int iCursor;          /* Next character of input to parse */
  char *aData;                   /* Where the information is stored */
  void (*xRealloc)(Blob*, unsigned int); /* Function to reallocate the buffer */
};
extern const Blob empty_blob;
int Th_GetOutstandingMalloc();
void Th_DeleteInterp(Th_Interp *);
char *fossil_getenv(const char *zName);
void db_close(int reportErrors);
#if defined(FOSSIL_ENABLE_TCL)
int unloadTcl(Th_Interp *interp,void *pContext);
#endif
int fossil_strcmp(const char *zA,const char *zB);
void cgi_debug(const char *zFormat,...);
typedef struct Global Global;
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
#define CGIDEBUG(X)  if( g.fDebug ) cgi_debug X
typedef struct UrlData UrlData;
struct UrlData {
  /*
  ** NOTE: These members MUST be kept in sync with the related ones in the
  **       "Global" structure defined in "main.c".
  */
  int isFile;      /* True if a "file:" url */
  int isHttps;     /* True if a "https:" url */
  int isSsh;       /* True if an "ssh:" url */
  char *name;      /* Hostname for http: or filename for file: */
  char *hostname;  /* The HOST: parameter on http headers */
  char *protocol;  /* "http" or "https" */
  int port;        /* TCP port number for http: or https: */
  int dfltPort;    /* The default port for the given protocol */
  char *path;      /* Pathname for http: */
  char *user;      /* User id for http: */
  char *passwd;    /* Password for http: */
  char *canonical; /* Canonical representation of the URL */
  char *proxyAuth; /* Proxy-Authorizer: string */
  char *fossil;    /* The fossil query parameter on ssh: */
  unsigned flags;  /* Boolean flags controlling URL processing */
};
#define GLOBAL_URL()      ((UrlData *)(&g.urlIsFile))
#define UUID_SIZE 40
#define count(X)  (sizeof(X)/sizeof(X[0]))
#define INTERFACE 0
