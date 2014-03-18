/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
void test_th_eval(void);
typedef struct Blob Blob;
int blob_read_from_file(Blob *pBlob,const char *zFilename);
#if defined(FOSSIL_ENABLE_TCL)
#include "tcl.h"
#endif
#if defined(FOSSIL_ENABLE_JSON)
#include "cson_amalgamation.h"
#include "json_detail.h"
#endif
void usage(const char *zFormat);
void test_th_render(void);
int Th_Render(const char *z);
int fossil_isalnum(char c);
int fossil_isalpha(char c);
typedef struct Th_Interp Th_Interp;
int Th_UnsetVar(Th_Interp *,const char *,int);
void Th_Unstore(const char *zName);
void Th_StoreInt(const char *zName,int iValue);
void Th_Store(const char *zName,const char *zValue);
typedef int(*Th_CommandProc)(Th_Interp *,void *,int,const char **,int *);
int Th_CreateCommand(Th_Interp *interp,const char *zName,Th_CommandProc xProc,void *pContext,void(*xDel)(Th_Interp *,void *));
#if defined(FOSSIL_ENABLE_TCL)
int th_register_tcl(Th_Interp *interp,void *pContext);
#endif
int db_get_boolean(const char *zName,int dflt);
#include <dirent.h>
char *fossil_getenv(const char *zName);
int th_register_language(Th_Interp *interp);
typedef struct Th_Vtab Th_Vtab;
struct Th_Vtab {
  void *(*xMalloc)(unsigned int);
  void (*xFree)(void *);
};
Th_Interp *Th_CreateInterp(Th_Vtab *pVtab);
void db_open_config(int useAttach);
#define OPEN_ANY_SCHEMA      0x002      /* Do not error if schema is wrong */
#define WIKI_LINKSONLY      0x020  /* No markup.  Only decorate links */
typedef unsigned int u32;
void Th_FossilInit(u32 flags);
typedef struct UrlData UrlData;
void transport_close(UrlData *pUrlData);
void transport_send(UrlData *pUrlData,Blob *toSend);
const char *transport_errmsg(UrlData *pUrlData);
int transport_open(UrlData *pUrlData);
const char *get_user_agent();
void fossil_free(void *p);
char *encode64(const char *zData,int nData);
void blob_appendf(Blob *pBlob,const char *zFormat,...);
void blob_append(Blob *pBlob,const char *aData,int nData);
void url_parse_local(const char *zUrl,unsigned int urlFlags,UrlData *pUrlData);
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
typedef struct ReCompiled ReCompiled;
void re_free(ReCompiled *pRe);
int re_match(ReCompiled *pRe,const unsigned char *zIn,int nIn);
const char *re_compile(ReCompiled **ppRe,const char *zIn,int noCase);
typedef struct ReInput ReInput;
struct ReInput {
  const unsigned char *z;  /* All text */
  int i;                   /* Next byte to read */
  int mx;                  /* EOF when i>=mx */
};
struct ReCompiled {
  ReInput sIn;                /* Regular expression text */
  const char *zErr;           /* Error message to return */
  char *aOp;                  /* Operators for the virtual machine */
  int *aArg;                  /* Arguments to each operator */
  unsigned (*xNextChar)(ReInput*);  /* Next character function */
  unsigned char zInit[12];    /* Initial text to match */
  int nInit;                  /* Number of characters in zInit */
  unsigned nState;            /* Number of entries in aOp[] and aArg[] */
  unsigned nAlloc;            /* Slots allocated for aOp[] and aArg[] */
};
char *db_get(const char *zName,char *zDefault);
int fossil_strcmp(const char *zA,const char *zB);
int Th_Eval(Th_Interp *interp,int iFrame,const char *zProg,int nProg);
int Th_SetVar(Th_Interp *,const char *,int,const char *,int);
const char *Th_GetResult(Th_Interp *,int *);
int Th_GetVar(Th_Interp *,const char *,int);
int th_strlen(const char *);
int report_query_authorizer(void *pError,int code,const char *zArg1,const char *zArg2,const char *zArg3,const char *zArg4);
int Th_ErrorMessage(Th_Interp *,const char *,const char *,int);
int encode16(const unsigned char *pIn,unsigned char *zOut,int N);
#define OPEN_OK_NOT_FOUND    0x001      /* Do not error out if not found */
void db_find_and_open_repository(int bFlags,int nArgUsed);
void Th_Free(Th_Interp *,void *);
char *mprintf(const char *zFormat,...);
#define blob_size(X)  ((X)->nUsed)
#define blob_buffer(X)  ((X)->aData)
char *Th_Fetch(const char *zName,int *pSize);
int Th_SplitList(Th_Interp *,const char *,int,char ***,int **,int *);
int fossil_strnicmp(const char *zA,const char *zB,int nByte);
int Th_SetResultInt(Th_Interp *,int);
int login_has_capability(const char *zCap,int nCap);
const char *timeline_utc();
char *db_text(char const *zDefault,const char *zSql,...);
int Th_SetResult(Th_Interp *,const char *,int);
void blob_reset(Blob *pBlob);
void wiki_convert(Blob *pIn,Blob *pOut,int flags);
void blob_init(Blob *pBlob,const char *zData,int size);
struct Blob {
  unsigned int nUsed;            /* Number of bytes used in aData[] */
  unsigned int nAlloc;           /* Number of bytes allocated for aData[] */
  unsigned int iCursor;          /* Next character of input to parse */
  char *aData;                   /* Where the information is stored */
  void (*xRealloc)(Blob*, unsigned int); /* Function to reallocate the buffer */
};
#define WIKI_NOBADLINKS     0x010  /* Ignore broken hyperlinks */
#define WIKI_INLINE         0x002  /* Do not surround with <p>..</p> */
void cgi_append_content(const char *zData,int nAmt);
char *htmlize(const char *zIn,int n);
#define TH_CONTINUE 4
#define TH_RETURN   3
#define TH_BREAK    2
#define TH_ERROR    1
#define TH_OK       0
const char *Th_ReturnCodeName(int rc,int nullIfOk);
int Th_ToInt(Th_Interp *,const char *,int,int *);
int Th_WrongNumArgs(Th_Interp *interp,const char *zMsg);
char *blob_str(Blob *p);
void fossil_print(const char *zFormat,...);
void Th_PrintTraceLog();
void blob_zero(Blob *pBlob);
const char *find_option(const char *zLong,const char *zShort,int hasArg);
void Th_InitTraceLog();
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
void blob_vappendf(Blob *pBlob,const char *zFormat,va_list ap);
void Th_Trace(const char *zFormat,...);
int Th_GetOutstandingMalloc();
void *fossil_malloc(size_t n);
#define TH_INIT_NONE        ((u32)0x00000000) /* No flags. */
#define TH_INIT_DEFAULT     (TH_INIT_NONE)    /* Default flags. */
#define TH_INIT_FORCE_SETUP ((u32)0x00000008) /* Force eval of setup script? */
#define TH_INIT_FORCE_RESET ((u32)0x00000004) /* Force TH commands re-added? */
#define TH_INIT_FORCE_TCL   ((u32)0x00000002) /* Force Tcl to be enabled? */
#define TH_INIT_NEED_CONFIG ((u32)0x00000001) /* Open configuration first? */
#define INTERFACE 0
