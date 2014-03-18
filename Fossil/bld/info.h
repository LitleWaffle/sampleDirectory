/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
# define TAG_HIDDEN     5     /* Do not display in timeline */
# define TAG_CLOSED     9     /* Do not display this check-in as a leaf */
int fossil_strncmp(const char *zA,const char *zB,int nByte);
void login_insert_csrf_secret(void);
void form_begin(const char *zOtherArgs,const char *zAction,...);
void db_end_transaction(int rollbackFlag);
typedef struct Blob Blob;
int blob_is_reset(Blob *pBlob);
#define MC_PERMIT_HOOKS   1  /*  permit hooks to execute    */
int manifest_crosslink(int rid,Blob *pContent,int flags);
int content_put(Blob *pBlob);
int content_is_private(int rid);
void db_begin_transaction(void);
int md5sum_blob(const Blob *pIn,Blob *pCksum);
int db_multi_exec(const char *zSql,...);
char *date_in_standard_format(const char *zInputDate);
void login_verify_csrf_secret(void);
# define TAG_BGCOLOR    1     /* Set the background color for display */
char *cgi_parameter_trimmed(const char *zName,const char *zDefault);
#define PT(x)       cgi_parameter_trimmed((x),0)
#define PDT(x,y)    cgi_parameter_trimmed((x),(y))
int name_to_typed_rid(const char *zName,const char *zType);
void ci_edit_page(void);
int fossil_isspace(char c);
void render_color_chooser(int fPropagate,const char *zDefaultColor,const char *zIdPropagate,const char *zId,const char *zIdCustom);
void ainfo_page(void);
void ambiguous_page(void);
void cgi_set_parameter(const char *zName,const char *zValue);
int name_to_uuid(Blob *pName,int iErrPriority,const char *zType);
void blob_set(Blob *pBlob,const char *zStr);
void event_page(void);
void tktview_page(void);
void info_page(void);
typedef struct Manifest Manifest;
void ticket_output_change_artifact(Manifest *pTkt,const char *zListType);
int db_table_has_column(char const *zTableName,char const *zColName);
#define CFTYPE_TICKET     5
void tinfo_page(void);
void blob_to_utf8_no_bom(Blob *pBlob,int useMbcs);
const char *mimetype_from_content(Blob *pBlob);
void artifact_page(void);
char *htmlize(const char *zIn,int n);
void cgi_append_content(const char *zData,int nAmt);
int fossil_isdigit(char c);
void output_text_with_line_numbers(const char *z,const char *zLn);
int artifact_from_ci_and_filename(void);
int db_exists(const char *zSql,...);
void hexdump_page(void);
#define blob_buffer(X)  ((X)->aData)
void cgi_set_content(Blob *pNewContent);
const char *mimetype_from_name(const char *zName);
void rawartifact_page(void);
void cgi_set_content_type(const char *zType);
Blob *cgi_output_blob(void);
#if defined(FOSSIL_ENABLE_TCL)
#include "tcl.h"
#endif
#if defined(FOSSIL_ENABLE_JSON)
#include "cson_amalgamation.h"
#include "json_detail.h"
#endif
NORETURN void fossil_redirect_home(void);
void diff_page(void);
int validate16(const char *zIn,int nIn);
#define UUID_SIZE 40
void hyperlink_to_event_tagid(int tagid);
#define blob_size(X)  ((X)->nUsed)
char *fossil_strdup(const char *zOrig);
#define PERM_LNK          2     /*  symlink       */
# define TAG_BRANCH     8     /* Value is name of the current branch */
int object_description(int rid,int linkToView,Blob *pDownloadName);
#define OBJTYPE_EXE        0x0100
#define OBJTYPE_SYMLINK    0x0080
#define OBJTYPE_TAG        0x0040
#define OBJTYPE_EVENT      0x0020
#define OBJTYPE_ATTACHMENT 0x0010
#define OBJTYPE_TICKET     0x0008
#define OBJTYPE_WIKI       0x0004
#define OBJTYPE_CONTENT    0x0002
#define OBJTYPE_CHECKIN    0x0001
#define INTERFACE 0
typedef struct ManifestFile ManifestFile;
int manifest_file_mperm(ManifestFile *pFile);
ManifestFile *manifest_file_next(Manifest *p,int *pErr);
void manifest_file_rewind(Manifest *p);
void cgi_replace_parameter(const char *zName,const char *zValue);
struct ManifestFile { 
  char *zName;           /* Name of a file */
  char *zUuid;           /* UUID of the file */
  char *zPerm;           /* File permissions */
  char *zPrior;          /* Prior name if the name was changed */
};
void vdiff_page(void);
void blob_appendf(Blob *pBlob,const char *zFormat,...);
typedef struct Stmt Stmt;
void db_column_blob(Stmt *pStmt,int N,Blob *pBlob);
#define WIKI_NOBLOCK        0x004  /* No block markup of any kind */
#define WIKI_NOBADLINKS     0x010  /* Ignore broken hyperlinks */
#define WIKI_INLINE         0x002  /* Do not surround with <p>..</p> */
#define CFTYPE_MANIFEST   1
int is_a_version(int rid);
char *vmprintf(const char *zFormat,va_list ap);
void webpage_error(const char *zFormat,...);
void manifest_destroy(Manifest *p);
void wiki_convert(Blob *pIn,Blob *pOut,int flags);
void blob_init(Blob *pBlob,const char *zData,int size);
int moderation_pending(int rid);
void style_submenu_element(const char *zLabel,const char *zTitle,const char *zLink,...);
void moderation_approve(int rid);
NORETURN void cgi_redirectf(const char *zFormat,...);
void moderation_disapprove(int objid);
#define CFTYPE_WIKI       4
Manifest *manifest_get(int rid,int cfType,Blob *pErr);
struct Blob {
  unsigned int nUsed;            /* Number of bytes used in aData[] */
  unsigned int nAlloc;           /* Number of bytes allocated for aData[] */
  unsigned int iCursor;          /* Next character of input to parse */
  char *aData;                   /* Where the information is stored */
  void (*xRealloc)(Blob*, unsigned int); /* Function to reallocate the buffer */
};
struct Manifest {
  Blob content;         /* The original content blob */
  int type;             /* Type of artifact.  One of CFTYPE_xxxxx */
  int rid;              /* The blob-id for this manifest */
  char *zBaseline;      /* Baseline manifest.  The B card. */
  Manifest *pBaseline;  /* The actual baseline manifest */
  char *zComment;       /* Decoded comment.  The C card. */
  double rDate;         /* Date and time from D card.  0.0 if no D card. */
  char *zUser;          /* Name of the user from the U card. */
  char *zRepoCksum;     /* MD5 checksum of the baseline content.  R card. */
  char *zWiki;          /* Text of the wiki page.  W card. */
  char *zWikiTitle;     /* Name of the wiki page. L card. */
  char *zMimetype;      /* Mime type of wiki or comment text.  N card.  */
  double rEventDate;    /* Date of an event.  E card. */
  char *zEventId;       /* UUID for an event.  E card. */
  char *zTicketUuid;    /* UUID for a ticket. K card. */
  char *zAttachName;    /* Filename of an attachment. A card. */
  char *zAttachSrc;     /* UUID of document being attached. A card. */
  char *zAttachTarget;  /* Ticket or wiki that attachment applies to.  A card */
  int nFile;            /* Number of F cards */
  int nFileAlloc;       /* Slots allocated in aFile[] */
  int iFile;            /* Index of current file in iterator */
  ManifestFile *aFile;  /* One entry for each F-card */
  int nParent;          /* Number of parents. */
  int nParentAlloc;     /* Slots allocated in azParent[] */
  char **azParent;      /* UUIDs of parents.  One for each P card argument */
  int nCherrypick;      /* Number of entries in aCherrypick[] */
  struct {            
    char *zCPTarget;    /* UUID of cherry-picked version w/ +|- prefix */
    char *zCPBase;      /* UUID of cherry-pick baseline. NULL for singletons */
  } *aCherrypick;
  int nCChild;          /* Number of cluster children */
  int nCChildAlloc;     /* Number of closts allocated in azCChild[] */
  char **azCChild;      /* UUIDs of referenced objects in a cluster. M cards */
  int nTag;             /* Number of T Cards */
  int nTagAlloc;        /* Slots allocated in aTag[] */
  struct TagType {
    char *zName;           /* Name of the tag */
    char *zUuid;           /* UUID that the tag is applied to */
    char *zValue;          /* Value if the tag is really a property */
  } *aTag;              /* One for each T card */
  int nField;           /* Number of J cards */
  int nFieldAlloc;      /* Slots allocated in aField[] */
  struct { 
    char *zName;           /* Key or field name */
    char *zValue;          /* Value of the field */
  } *aField;            /* One for each J card */
};
void winfo_page(void);
char *xhref(const char *zExtra,const char *zFormat,...);
int db_get_boolean(const char *zName,int dflt);
void fossil_free(void *p);
int blob_trim(Blob *p);
void blob_append(Blob *pBlob,const char *aData,int nData);
void hyperlink_to_user(const char *zU,const char *zD,const char *zSuf);
# define TAG_COMMENT    2     /* The check-in comment */
# define TAG_USER       3     /* User who made a checking */
void login_anonymous_available(void);
char *mprintf(const char *zFormat,...);
int is_false(const char *zVal);
int is_a_leaf(int rid);
typedef struct ReCompiled ReCompiled;
const char *re_compile(ReCompiled **ppRe,const char *zIn,int noCase);
void style_footer(void);
void style_header(const char *zTitleFormat,...);
int name_to_rid_www(const char *zParamName);
const char *cgi_parameter(const char *zName,const char *zDefault);
#define P(x)        cgi_parameter((x),0)
void login_needed(void);
void login_check_credentials(void);
void ci_page(void);
#define DIFF_NOOPT        (((u64)0x01)<<32) /* Suppress optimizations (debug) */
#define DIFF_INLINE       ((u64)0x00000000) /* Inline (not side-by-side) diff */
#define DIFF_WIDTH_MASK   ((u64)0x00ff0000) /* side-by-side column width */
#define DIFF_CONTEXT_MASK ((u64)0x0000ffff) /* Lines of context. Default if 0 */
#define PD(x,y)     cgi_parameter((x),(y))
#define DIFF_IGNORE_EOLWS ((u64)0x01000000) /* Ignore end-of-line whitespace */
u64 construct_diff_flags(int verboseFlag,int sideBySide);
void append_diff_javascript(int sideBySide);
char *href(const char *zFormat,...);
#define PERM_EXE          1     /*  executable    */
void blob_reset(Blob *pBlob);
#define DIFF_LINENO       ((u64)0x20000000) /* Show line numbers */
char *blob_str(Blob *p);
#define DIFF_NOTTOOBIG    (((u64)0x08)<<32) /* Only display if not too big */
#define DIFF_HTML         ((u64)0x10000000) /* Render for HTML */
int *text_diff(Blob *pA_Blob,Blob *pB_Blob,Blob *pOut,ReCompiled *pRe,u64 diffFlags);
#define DIFF_SIDEBYSIDE   ((u64)0x02000000) /* Generate a side-by-side diff */
void blob_zero(Blob *pBlob);
int content_get(int rid,Blob *pBlob);
int uuid_to_rid(const char *zUuid,int phantomize);
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
void hyperlink_to_date(const char *zDate,const char *zSuffix);
void hyperlink_to_uuid(const char *zUuid);
void cgi_printf(const char *zFormat,...);
const char *timeline_utc();
NORETURN void fossil_fatal(const char *zFormat,...);
int name_to_rid(const char *zName);
int db_int(int iDflt,const char *zSql,...);
int db_lget_int(const char *zName,int dflt);
const char *db_repository_filename(void);
void db_find_and_open_repository(int bFlags,int nArgUsed);
char *db_get(const char *zName,char *zDefault);
void db_open_repository(const char *zDbName);
void db_record_repository_filename(const char *zName);
void db_open_config(int useAttach);
#include <dirent.h>
i64 file_size(const char *zFilename);
const char *find_option(const char *zLong,const char *zShort,int hasArg);
void info_cmd(void);
int fossil_strcmp(const char *zA,const char *zB);
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
int comment_print(const char *zText,int indent,int lineLength);
int db_finalize(Stmt *pStmt);
int db_column_int(Stmt *pStmt,int N);
const char *db_column_text(Stmt *pStmt,int N);
int db_step(Stmt *pStmt);
int db_prepare(Stmt *pStmt,const char *zFormat,...);
void fossil_print(const char *zFormat,...);
struct Stmt {
  Blob sql;               /* The SQL for this statement */
  sqlite3_stmt *pStmt;    /* The results of sqlite3_prepare() */
  Stmt *pNext, *pPrev;    /* List of all unfinalized statements */
  int nStep;              /* Number of sqlite3_step() calls */
};
void show_common_info(int rid,const char *zUuidName,int showComment,int showFamily);
char *db_text(char const *zDefault,const char *zSql,...);
char *info_tags_of_checkin(int rid,int propagatingOnly);
