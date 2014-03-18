/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
int count_nonbranch_children(int pid);
#define SYNC_PUSH      0x0001
const char *db_name(const char *zDb);
void undo_reset(void);
typedef struct Blob Blob;
void vfile_aggregate_checksum_manifest(int vid,Blob *pOut,Blob *pManOut);
void vfile_compare_repository_to_disk(int vid);
int blob_compare(Blob *pA,Blob *pB);
void vfile_aggregate_checksum_repository(int vid,Blob *pOut);
void db_lset_int(const char *zName,int value);
int blob_is_reset(Blob *pBlob);
#define MC_PERMIT_HOOKS   1  /*  permit hooks to execute    */
#define MC_NONE           0  /*  default handling           */
int manifest_crosslink(int rid,Blob *pContent,int flags);
int clearsign(Blob *pIn,Blob *pOut);
typedef struct Manifest Manifest;
Manifest *manifest_get_by_name(const char *zName,int *pRid);
#define CFTYPE_MANIFEST   1
Manifest *manifest_get(int rid,int cfType,Blob *pErr);
int content_deltify(int rid,int srcid,int force);
int content_put(Blob *pBlob);
int contains_merge_marker(Blob *p);
int blob_read_link(Blob *pBlob,const char *zFilename);
#include <dirent.h>
int file_wd_islink(const char *zFilename);
char *glob_expr(const char *zVal,const char *zGlobList);
void db_end_transaction(int rollbackFlag);
void vfile_aggregate_checksum_disk(int vid,Blob *pOut);
void db_begin_transaction(void);
int unsaved_changes(unsigned int cksigFlags);
void user_select(void);
NORETURN void fossil_exit(int rc);
#define SYNC_PULL      0x0002
int autosync(int flags);
void *fossil_realloc(void *p,size_t n);
void url_proxy_options(void);
void commit_cmd(void);
FILE *fossil_fopen(const char *zName,const char *zMode);
char *file_newname(const char *zBase,const char *zSuffix,int relFlag);
void fossil_free(void *p);
#define LOOK_LONG    ((int)0x00000040) /* An over length line was found. */
#define LOOK_CRLF    ((int)0x00000020) /* One or more CR/LF pairs were found. */
#define LOOK_LONE_CR ((int)0x00000004) /* An unpaired CR char was found. */
#define LOOK_LONE_LF ((int)0x00000010) /* An unpaired LF char was found. */
#define LOOK_EOL     (LOOK_LONE_CR | LOOK_LONE_LF | LOOK_CRLF) /* Line seps. */
#define LOOK_NUL     ((int)0x00000001) /* One or more NUL chars were found. */
#define LOOK_SHORT   ((int)0x00000100) /* Unable to perform full check. */
#define LOOK_BINARY  (LOOK_NUL | LOOK_LONG | LOOK_SHORT) /* May be binary. */
#define LOOK_CR      ((int)0x00000002) /* One or more CR chars were found. */
int looks_like_utf8(const Blob *pContent,int stopFlags);
int looks_like_utf16(const Blob *pContent,int bReverse,int stopFlags);
int could_be_utf16(const Blob *pContent,int *pbReverse);
int md5sum_blob(const Blob *pIn,Blob *pCksum);
# define TAG_CLOSED     9     /* Do not display this check-in as a leaf */
int is_a_leaf(int rid);
int content_is_private(int rid);
void content_make_public(int rid);
#define PERM_LNK          2     /*  symlink       */
#define PERM_EXE          1     /*  executable    */
int file_wd_perm(const char *zFilename);
typedef struct ManifestFile ManifestFile;
ManifestFile *manifest_file_next(Manifest *p,int *pErr);
void manifest_file_rewind(Manifest *p);
struct ManifestFile { 
  char *zName;           /* Name of a file */
  char *zUuid;           /* UUID of the file */
  char *zPerm;           /* File permissions */
  char *zPrior;          /* Prior name if the name was changed */
};
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
#define INTERFACE 0
#define OPEN_ANY_SCHEMA      0x002      /* Do not error if schema is wrong */
void db_find_and_open_repository(int bFlags,int nArgUsed);
void test_date_format(void);
const char *cgi_parameter(const char *zName,const char *zDefault);
#define PD(x,y)     cgi_parameter((x),(y))
char *date_in_standard_format(const char *zInputDate);
int db_exists(const char *zSql,...);
typedef struct Bag Bag;
int bag_next(Bag *p,int e);
int bag_first(Bag *p);
int bag_count(Bag *p);
void *fossil_malloc(size_t n);
int bag_insert(Bag *p,int e);
void bag_clear(Bag *p);
void bag_init(Bag *p);
struct Bag {
  int cnt;   /* Number of integers in the bag */
  int sz;    /* Number of slots in a[] */
  int used;  /* Number of used slots in a[] */
  int *a;    /* Hash table of integers that are in the bag */
};
int select_commit_files(void);
char *info_tags_of_checkin(int rid,int propagatingOnly);
const unsigned char *get_utf8_bom(int *pnByte);
typedef struct CheckinInfo CheckinInfo;
struct CheckinInfo {
  Blob *pComment;             /* Check-in comment text */
  const char *zMimetype;      /* Mimetype of check-in command.  May be NULL */
  int verifyDate;             /* Verify that child is younger */
  int closeFlag;              /* Close the branch being committed */
  int integrateFlag;          /* Close merged-in branches */
  Blob *pCksum;               /* Repository checksum.  May be 0 */
  const char *zDateOvrd;      /* Date override.  If 0 then use 'now' */
  const char *zUserOvrd;      /* User override.  If 0 then use g.zLogin */
  const char *zBranch;        /* Branch name.  May be 0 */
  const char *zColor;         /* One-time background color.  May be 0 */
  const char *zBrClr;         /* Persistent branch color.  May be 0 */
  const char **azTag;         /* Tags to apply to this check-in */
};
void blob_resize(Blob *pBlob,unsigned int newSize);
int fossil_isspace(char c);
#define blob_buffer(X)  ((X)->aData)
int blob_line(Blob *pFrom,Blob *pTo);
void blob_to_lf_only(Blob *p);
void blob_to_utf8_no_bom(Blob *pBlob,int useMbcs);
int blob_read_from_file(Blob *pBlob,const char *zFilename);
int fossil_system(const char *zOrigCmd);
char *db_text(char const *zDefault,const char *zSql,...);
#if defined(_WIN32) || defined(__CYGWIN__)
void blob_add_cr(Blob *p);
#endif
void *fossil_utf8_to_filename(const char *zUtf8);
char *fossil_getenv(const char *zName);
void prompt_for_user_comment(Blob *pComment,Blob *pPrompt);
int file_rmdir(const char *zName);
typedef struct Glob Glob;
int vfile_dir_scan(Blob *pPath,int nPrefix,unsigned scanFlags,Glob *pIgnore1,Glob *pIgnore2,Glob *pIgnore3);
int file_delete(const char *zFilename);
void prompt_user(const char *zPrompt,Blob *pIn);
int glob_match(Glob *pGlob,const char *zString);
#define SCAN_NESTED 0x004    /* Scan for empty dirs in nested checkouts */
void clean_cmd(void);
const char *fossil_all_reserved_names(int omitRepo);
void glob_free(Glob *pGlob);
Glob *glob_create(const char *zPatternList);
void capture_case_sensitive_option(void);
#define SCAN_TEMP   0x002    /* Only Fossil-generated files like *-baseline */
#define SCAN_ALL    0x001    /* Includes files that begin with "." */
void extra_cmd(void);
int file_wd_isdir(const char *zFilename);
void file_canonical_name(const char *zOrigName,Blob *pOut,int slash);
void vfile_scan(Blob *pPath,int nPrefix,unsigned scanFlags,Glob *pIgnore1,Glob *pIgnore2);
void blob_init(Blob *pBlob,const char *zData,int size);
int db_multi_exec(const char *zSql,...);
struct Glob {
  int nPattern;        /* Number of patterns */
  char **azPattern;    /* Array of pointers to patterns */
};
const char *timeline_utc();
#if defined(FOSSIL_ENABLE_TCL)
#include "tcl.h"
#endif
#if defined(FOSSIL_ENABLE_JSON)
#include "cson_amalgamation.h"
#include "json_detail.h"
#endif
void verify_all_options(void);
void ls_cmd(void);
void db_record_repository_filename(const char *zName);
void show_common_info(int rid,const char *zUuidName,int showComment,int showFamily);
const char *db_repository_filename(void);
void status_cmd(void);
int blob_write_to_file(Blob *pBlob,const char *zFilename);
char *db_get(const char *zName,char *zDefault);
void fossil_print(const char *zFormat,...);
#define CKSIG_SHA1      0x002   /* Verify file content using sha1sum */
void vfile_check_signature(int vid,unsigned int cksigFlags);
int db_lget_int(const char *zName,int dflt);
void db_must_be_within_tree(void);
void changes_cmd(void);
const char *find_option(const char *zLong,const char *zShort,int hasArg);
int db_get_boolean(const char *zName,int dflt);
NORETURN void fossil_fatal(const char *zFormat,...);
typedef struct Stmt Stmt;
int db_finalize(Stmt *pStmt);
int file_contains_merge_marker(const char *zFullpath);
void fossil_warning(const char *zFormat,...);
int file_access(const char *zFilename,int flags);
int file_wd_isfile_or_link(const char *zFilename);
void blob_append(Blob *pBlob,const char *aData,int nData);
void file_relative_name(const char *zOrigName,Blob *pOut,int slash);
char *mprintf(const char *zFormat,...);
int db_column_int(Stmt *pStmt,int N);
const char *db_column_text(Stmt *pStmt,int N);
int db_step(Stmt *pStmt);
int db_prepare(Stmt *pStmt,const char *zFormat,...);
const char *filename_collation(void);
#define blob_size(X)  ((X)->nUsed)
void blob_appendf(Blob *pBlob,const char *zFormat,...);
void blob_reset(Blob *pBlob);
int fossil_strcmp(const char *zA,const char *zB);
char *blob_str(Blob *p);
int file_tree_name(const char *zOrigName,Blob *pOut,int errFatal);
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
void blob_zero(Blob *pBlob);
struct Stmt {
  Blob sql;               /* The SQL for this statement */
  sqlite3_stmt *pStmt;    /* The results of sqlite3_prepare() */
  Stmt *pNext, *pPrev;    /* List of all unfinalized statements */
  int nStep;              /* Number of sqlite3_step() calls */
};
