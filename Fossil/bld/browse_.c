#line 1 "./src/browse.c"
/*
** Copyright (c) 2008 D. Richard Hipp
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the Simplified BSD License (also
** known as the "2-Clause License" or "FreeBSD License".)

** This program is distributed in the hope that it will be useful,
** but without any warranty; without even the implied warranty of
** merchantability or fitness for a particular purpose.
**
** Author contact information:
**   drh@hwaci.com
**   http://www.hwaci.com/drh/
**
*******************************************************************************
**
** This file contains code to implement the file browser web interface.
*/
#include "config.h"
#include "browse.h"
#include <assert.h>

/*
** This is the implementation of the "pathelement(X,N)" SQL function.
**
** If X is a unix-like pathname (with "/" separators) and N is an
** integer, then skip the initial N characters of X and return the
** name of the path component that begins on the N+1th character
** (numbered from 0).  If the path component is a directory (if
** it is followed by other path components) then prepend "/".
**
** Examples:
**
**      pathelement('abc/pqr/xyz', 4)  ->  '/pqr'
**      pathelement('abc/pqr', 4)      ->  'pqr'
**      pathelement('abc/pqr/xyz', 0)  ->  '/abc'
*/
void pathelementFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const unsigned char *z;
  int len, n, i;
  char *zOut;

  assert( argc==2 );
  z = sqlite3_value_text(argv[0]);
  if( z==0 ) return;
  len = sqlite3_value_bytes(argv[0]);
  n = sqlite3_value_int(argv[1]);
  if( len<=n ) return;
  if( n>0 && z[n-1]!='/' ) return;
  for(i=n; i<len && z[i]!='/'; i++){}
  if( i==len ){
    sqlite3_result_text(context, (char*)&z[n], len-n, SQLITE_TRANSIENT);
  }else{
    zOut = sqlite3_mprintf("/%.*s", i-n, &z[n]);
    sqlite3_result_text(context, zOut, i-n+1, sqlite3_free);
  }
}

/*
** Given a pathname which is a relative path from the root of
** the repository to a file or directory, compute a string which
** is an HTML rendering of that path with hyperlinks on each
** directory component of the path where the hyperlink redirects
** to the "dir" page for the directory.
**
** There is no hyperlink on the file element of the path.
**
** The computed string is appended to the pOut blob.  pOut should
** have already been initialized.
*/
void hyperlinked_path(
  const char *zPath,    /* Path to render */
  Blob *pOut,           /* Write into this blob */
  const char *zCI,      /* check-in name, or NULL */
  const char *zURI,     /* "dir" or "tree" */
  const char *zREx      /* Extra query parameters */
){
  int i, j;
  char *zSep = "";

  for(i=0; zPath[i]; i=j){
    for(j=i; zPath[j] && zPath[j]!='/'; j++){}
    if( zPath[j] && g.perm.Hyperlink ){
      if( zCI ){
        char *zLink = href("%R/%s?ci=%S&name=%#T%s", zURI, zCI, j, zPath,zREx);
        blob_appendf(pOut, "%s%z%#h</a>",
                     zSep, zLink, j-i, &zPath[i]);
      }else{
        char *zLink = href("%R/%s?name=%#T%s", zURI, j, zPath, zREx);
        blob_appendf(pOut, "%s%z%#h</a>",
                     zSep, zLink, j-i, &zPath[i]);
      }
    }else{
      blob_appendf(pOut, "%s%#h", zSep, j-i, &zPath[i]);
    }
    zSep = "/";
    while( zPath[j]=='/' ){ j++; }
  }
}


/*
** WEBPAGE: dir
**
** Query parameters:
**
**    name=PATH        Directory to display.  Optional.  Top-level if missing
**    ci=LABEL         Show only files in this check-in.  Optional.
*/
void page_dir(void){
  char *zD = fossil_strdup(P("name"));
  int nD = zD ? strlen(zD)+1 : 0;
  int mxLen;
  int nCol, nRow;
  int cnt, i;
  char *zPrefix;
  Stmt q;
  const char *zCI = P("ci");
  int rid = 0;
  char *zUuid = 0;
  Blob dirname;
  Manifest *pM = 0;
  const char *zSubdirLink;
  int linkTrunk = 1;
  int linkTip = 1;
  HQuery sURI;

  if( strcmp(PD("type",""),"tree")==0 ){ page_tree(); return; }
  login_check_credentials();
  if( !g.perm.Read ){ login_needed(); return; }
  while( nD>1 && zD[nD-2]=='/' ){ zD[(--nD)-1] = 0; }
  style_header("File List");
  sqlite3_create_function(g.db, "pathelement", 2, SQLITE_UTF8, 0,
                          pathelementFunc, 0, 0);
  url_initialize(&sURI, "dir");

  /* If the name= parameter is an empty string, make it a NULL pointer */
  if( zD && strlen(zD)==0 ){ zD = 0; }

  /* If a specific check-in is requested, fetch and parse it.  If the
  ** specific check-in does not exist, clear zCI.  zCI==0 will cause all
  ** files from all check-ins to be displayed.
  */
  if( zCI ){
    pM = manifest_get_by_name(zCI, &rid);
    if( pM ){
      int trunkRid = symbolic_name_to_rid("tag:trunk", "ci");
      linkTrunk = trunkRid && rid != trunkRid;
      linkTip = rid != symbolic_name_to_rid("tip", "ci");
      zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
      url_add_parameter(&sURI, "ci", zCI);
    }else{
      zCI = 0;
    }
  }

  /* Compute the title of the page */
  blob_zero(&dirname);
  if( zD ){
    url_add_parameter(&sURI, "name", zD);
    blob_append(&dirname, "in directory ", -1);
    hyperlinked_path(zD, &dirname, zCI, "dir", "");
    zPrefix = mprintf("%s/", zD);
    style_submenu_element("Top-Level", "Top-Level", "%s",
                          url_render(&sURI, "name", 0, 0, 0));
  }else{
    blob_append(&dirname, "in the top-level directory", -1);
    zPrefix = "";
  }
  if( linkTrunk ){
    style_submenu_element("Trunk", "Trunk", "%s",
                          url_render(&sURI, "ci", "trunk", 0, 0));
  }
  if( linkTip ){
    style_submenu_element("Tip", "Tip", "%s",
                          url_render(&sURI, "ci", "tip", 0, 0));
  }
  if( zCI ){
    char zShort[20];
    memcpy(zShort, zUuid, 10);
    zShort[10] = 0;
    cgi_printf("<h2>Files of check-in [%z%s</a>]\n"
           "%s</h2>\n",(href("vinfo?name=%T",zUuid)),(zShort),(blob_str(&dirname)));
    zSubdirLink = mprintf("%R/dir?ci=%S&name=%T", zUuid, zPrefix);
    if( nD==0 ){
      style_submenu_element("File Ages", "File Ages", "%R/fileage?name=%S",
                            zUuid);
    }
  }else{
    cgi_printf("<h2>The union of all files from all check-ins\n"
           "%s</h2>\n",(blob_str(&dirname)));
    zSubdirLink = mprintf("%R/dir?name=%T", zPrefix);
  }
  style_submenu_element("All", "All", "%s",
                        url_render(&sURI, "ci", 0, 0, 0));
  style_submenu_element("Tree-View", "Tree-View", "%s",
                        url_render(&sURI, "type", "tree", 0, 0));

  /* Compute the temporary table "localfiles" containing the names
  ** of all files and subdirectories in the zD[] directory.
  **
  ** Subdirectory names begin with "/".  This causes them to sort
  ** first and it also gives us an easy way to distinguish files
  ** from directories in the loop that follows.
  */
  db_multi_exec(
     "CREATE TEMP TABLE localfiles(x UNIQUE NOT NULL, u);"
  );
  if( zCI ){
    Stmt ins;
    ManifestFile *pFile;
    ManifestFile *pPrev = 0;
    int nPrev = 0;
    int c;

    db_prepare(&ins,
       "INSERT OR IGNORE INTO localfiles VALUES(pathelement(:x,0), :u)"
    );
    manifest_file_rewind(pM);
    while( (pFile = manifest_file_next(pM,0))!=0 ){
      if( nD>0
       && (fossil_strncmp(pFile->zName, zD, nD-1)!=0
           || pFile->zName[nD-1]!='/')
      ){
        continue;
      }
      if( pPrev
       && fossil_strncmp(&pFile->zName[nD],&pPrev->zName[nD],nPrev)==0
       && (pFile->zName[nD+nPrev]==0 || pFile->zName[nD+nPrev]=='/')
      ){
        continue;
      }
      db_bind_text(&ins, ":x", &pFile->zName[nD]);
      db_bind_text(&ins, ":u", pFile->zUuid);
      db_step(&ins);
      db_reset(&ins);
      pPrev = pFile;
      for(nPrev=0; (c=pPrev->zName[nD+nPrev]) && c!='/'; nPrev++){}
      if( c=='/' ) nPrev++;
    }
    db_finalize(&ins);
  }else if( zD ){
    db_multi_exec(
      "INSERT OR IGNORE INTO localfiles"
      " SELECT pathelement(name,%d), NULL FROM filename"
      "  WHERE name GLOB '%q/*'",
      nD, zD
    );
  }else{
    db_multi_exec(
      "INSERT OR IGNORE INTO localfiles"
      " SELECT pathelement(name,0), NULL FROM filename"
    );
  }

  /* Generate a multi-column table listing the contents of zD[]
  ** directory.
  */
  mxLen = db_int(12, "SELECT max(length(x)) FROM localfiles /*scan*/");
  cnt = db_int(0, "SELECT count(*) FROM localfiles /*scan*/");
  if( mxLen<12 ) mxLen = 12;
  nCol = 100/mxLen;
  if( nCol<1 ) nCol = 1;
  if( nCol>5 ) nCol = 5;
  nRow = (cnt+nCol-1)/nCol;
  db_prepare(&q, "SELECT x, u FROM localfiles ORDER BY x /*scan*/");
  cgi_printf("<table class=\"browser\"><tr><td class=\"browser\"><ul class=\"browser\">\n");
  i = 0;
  while( db_step(&q)==SQLITE_ROW ){
    const char *zFN;
    if( i==nRow ){
      cgi_printf("</ul></td><td class=\"browser\"><ul class=\"browser\">\n");
      i = 0;
    }
    i++;
    zFN = db_column_text(&q, 0);
    if( zFN[0]=='/' ){
      zFN++;
      cgi_printf("<li class=\"dir\">%z%h</a></li>\n",(href("%s%T",zSubdirLink,zFN)),(zFN));
    }else{
      const char *zLink;
      if( zCI ){
        const char *zUuid = db_column_text(&q, 1);
        zLink = href("%R/artifact/%s",zUuid);
      }else{
        zLink = href("%R/finfo?name=%T%T",zPrefix,zFN);
      }
      cgi_printf("<li class=\"%z\">%z%h</a></li>\n",(fileext_class(zFN)),(zLink),(zFN));
    }
  }
  db_finalize(&q);
  manifest_destroy(pM);
  cgi_printf("</ul></td></tr></table>\n");
  style_footer();
}

/*
** Objects used by the "tree" webpage.
*/
typedef struct FileTreeNode FileTreeNode;
typedef struct FileTree FileTree;

/*
** A single line of the file hierarchy
*/
struct FileTreeNode {
  FileTreeNode *pNext;      /* Next line in sequence */
  FileTreeNode *pPrev;      /* Previous line */
  FileTreeNode *pParent;    /* Directory containing this line */
  char *zName;              /* Name of this entry.  The "tail" */
  char *zFullName;          /* Full pathname of this entry */
  char *zUuid;              /* SHA1 hash of this file.  May be NULL. */
  unsigned nFullName;       /* Length of zFullName */
  unsigned iLevel;          /* Levels of parent directories */
  u8 isDir;                 /* True if there are children */
  u8 isLast;                /* True if this is the last child of its parent */
};

/*
** A complete file hierarchy
*/
struct FileTree {
  FileTreeNode *pFirst;     /* First line of the list */
  FileTreeNode *pLast;      /* Last line of the list */
};

/*
** Add one or more new FileTreeNodes to the FileTree object so that the
** leaf object zPathname is at the end of the node list
*/
static void tree_add_node(
  FileTree *pTree,         /* Tree into which nodes are added */
  const char *zPath,       /* The full pathname of file to add */
  const char *zUuid        /* UUID of the file.  Might be NULL. */
){
  int i;
  FileTreeNode *pParent;
  FileTreeNode *pChild;

  pChild = pTree->pLast;
  pParent = pChild ? pChild->pParent : 0;
  while( pParent!=0 &&
      ( strncmp(pParent->zFullName, zPath, pParent->nFullName)!=0
        || zPath[pParent->nFullName]!='/' )
  ){
    pChild = pParent;
    pParent = pChild->pParent;
  }
  i = pParent ? pParent->nFullName+1 : 0;
  if( pChild ) pChild->isLast = 0;
  while( zPath[i] ){
    FileTreeNode *pNew;
    int iStart = i;
    int nByte;
    while( zPath[i] && zPath[i]!='/' ){ i++; }
    nByte = sizeof(*pNew) + i + 1;
    if( zUuid!=0 && zPath[i]==0 ) nByte += UUID_SIZE+1;
    pNew = fossil_malloc( nByte );
    pNew->zFullName = (char*)&pNew[1];
    memcpy(pNew->zFullName, zPath, i);
    pNew->zFullName[i] = 0;
    pNew->nFullName = i;
    if( zUuid!=0 && zPath[i]==0 ){
      pNew->zUuid = pNew->zFullName + i + 1;
      memcpy(pNew->zUuid, zUuid, UUID_SIZE+1);
    }else{
      pNew->zUuid = 0;
    }
    pNew->zName = pNew->zFullName + iStart;
    if( pTree->pLast ){
      pTree->pLast->pNext = pNew;
    }else{
      pTree->pFirst = pNew;
    }
    pNew->pPrev = pTree->pLast;
    pNew->pNext = 0;
    pNew->pParent = pParent;
    pTree->pLast = pNew;
    pNew->iLevel = pParent ? pParent->iLevel+1 : 0;
    pNew->isDir = zPath[i]=='/';
    pNew->isLast = 1;
    while( zPath[i]=='/' ){ i++; }
    pParent = pNew;
  }
}

/*
** WEBPAGE: tree
**
** Query parameters:
**
**    name=PATH        Directory to display.  Optional
**    ci=LABEL         Show only files in this check-in.  Optional.
**    re=REGEXP        Show only files matching REGEXP.  Optional.
**    expand           Begin with the tree fully expanded.
**    nofiles          Show directories (folders) only.  Omit files.
*/
void page_tree(void){
  char *zD = fossil_strdup(P("name"));
  int nD = zD ? strlen(zD)+1 : 0;
  const char *zCI = P("ci");
  int rid = 0;
  char *zUuid = 0;
  Blob dirname;
  Manifest *pM = 0;
  int nFile = 0;           /* Number of files (or folders with "nofiles") */
  int linkTrunk = 1;       /* include link to "trunk" */
  int linkTip = 1;         /* include link to "tip" */
  const char *zRE;         /* the value for the re=REGEXP query parameter */
  const char *zObjType;    /* "files" by default or "folders" for "nofiles" */
  char *zREx = "";         /* Extra parameters for path hyperlinks */
  ReCompiled *pRE = 0;     /* Compiled regular expression */
  FileTreeNode *p;         /* One line of the tree */
  FileTree sTree;          /* The complete tree of files */
  HQuery sURI;             /* Hyperlink */
  int startExpanded;       /* True to start out with the tree expanded */
  int showDirOnly;         /* Show directories only.  Omit files */
  char *zProjectName = db_get("project-name", 0);

  if( strcmp(PD("type",""),"flat")==0 ){ page_dir(); return; }
  memset(&sTree, 0, sizeof(sTree));
  login_check_credentials();
  if( !g.perm.Read ){ login_needed(); return; }
  while( nD>1 && zD[nD-2]=='/' ){ zD[(--nD)-1] = 0; }
  sqlite3_create_function(g.db, "pathelement", 2, SQLITE_UTF8, 0,
                          pathelementFunc, 0, 0);
  url_initialize(&sURI, "tree");
  if( P("nofiles")!=0 ){
    showDirOnly = 1;
    url_add_parameter(&sURI, "nofiles", "1");
    style_header("Folder Hierarchy");
  }else{
    showDirOnly = 0;
    style_header("File Tree");
  }
  if( P("expand")!=0 ){
    startExpanded = 1;
    url_add_parameter(&sURI, "expand", "1");
  }else{
    startExpanded = 0;
  }

  /* If a regular expression is specified, compile it */
  zRE = P("re");
  if( zRE ){
    re_compile(&pRE, zRE, 0);
    url_add_parameter(&sURI, "re", zRE);
    zREx = mprintf("&re=%T", zRE);
  }

  /* If the name= parameter is an empty string, make it a NULL pointer */
  if( zD && strlen(zD)==0 ){ zD = 0; }

  /* If a specific check-in is requested, fetch and parse it.  If the
  ** specific check-in does not exist, clear zCI.  zCI==0 will cause all
  ** files from all check-ins to be displayed.
  */
  if( zCI ){
    pM = manifest_get_by_name(zCI, &rid);
    if( pM ){
      int trunkRid = symbolic_name_to_rid("tag:trunk", "ci");
      linkTrunk = trunkRid && rid != trunkRid;
      linkTip = rid != symbolic_name_to_rid("tip", "ci");
      zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
      url_add_parameter(&sURI, "ci", zCI);
    }else{
      zCI = 0;
    }
  }

  /* Compute the title of the page */
  blob_zero(&dirname);
  if( zD ){
    url_add_parameter(&sURI, "name", zD);
    blob_append(&dirname, "within directory ", -1);
    hyperlinked_path(zD, &dirname, zCI, "tree", zREx);
    if( zRE ) blob_appendf(&dirname, " matching \"%s\"", zRE);
    style_submenu_element("Top-Level", "Top-Level", "%s",
                          url_render(&sURI, "name", 0, 0, 0));
  }else{
    if( zRE ){
      blob_appendf(&dirname, "matching \"%s\"", zRE);
    }
  }
  if( zCI ){
    style_submenu_element("All", "All", "%s",
                          url_render(&sURI, "ci", 0, 0, 0));
    if( nD==0 && !showDirOnly ){
      style_submenu_element("File Ages", "File Ages", "%R/fileage?name=%S",
                            zUuid);
    }
  }
  if( linkTrunk ){
    style_submenu_element("Trunk", "Trunk", "%s",
                          url_render(&sURI, "ci", "trunk", 0, 0));
  }
  if ( linkTip ){
    style_submenu_element("Tip", "Tip", "%s",
                          url_render(&sURI, "ci", "tip", 0, 0));
  }
  if( !showDirOnly ){
    style_submenu_element("Flat-View", "Flat-View", "%s",
                          url_render(&sURI, "type", "flat", 0, 0));
  }

  /* Compute the file hierarchy.
  */
  if( zCI ){
    Stmt ins, q;
    ManifestFile *pFile;

    db_multi_exec(
        "CREATE TEMP TABLE filelist("
        "   x TEXT PRIMARY KEY COLLATE nocase,"
        "   uuid TEXT"
        ")%s;",
        /* Can be removed as soon as SQLite 3.8.2 is sufficiently wide-spread */
        sqlite3_libversion_number()>=3008002 ? " WITHOUT ROWID" : ""
    );
    db_prepare(&ins, "INSERT OR IGNORE INTO filelist VALUES(:f,:u)");
    manifest_file_rewind(pM);
    while( (pFile = manifest_file_next(pM,0))!=0 ){
      if( nD>0
       && (fossil_strncmp(pFile->zName, zD, nD-1)!=0
           || pFile->zName[nD-1]!='/')
      ){
        continue;
      }
      if( pRE && re_match(pRE, (const u8*)pFile->zName, -1)==0 ) continue;
      db_bind_text(&ins, ":f", pFile->zName);
      db_bind_text(&ins, ":u", pFile->zUuid);
      db_step(&ins);
      db_reset(&ins);
    }
    db_finalize(&ins);
    db_prepare(&q, "SELECT x, uuid FROM filelist ORDER BY x");
    while( db_step(&q)==SQLITE_ROW ){
      tree_add_node(&sTree, db_column_text(&q,0), db_column_text(&q,1));
      nFile++;
    }
    db_finalize(&q);
  }else{
    Stmt q;
    db_prepare(&q, "SELECT name FROM filename ORDER BY name COLLATE nocase");
    while( db_step(&q)==SQLITE_ROW ){
      const char *z = db_column_text(&q, 0);
      if( nD>0 && (fossil_strncmp(z, zD, nD-1)!=0 || z[nD-1]!='/') ){
        continue;
      }
      if( pRE && re_match(pRE, (const u8*)z, -1)==0 ) continue;
      tree_add_node(&sTree, z, 0);
      nFile++;
    }
    db_finalize(&q);
  }

  if( showDirOnly ){
    for(nFile=0, p=sTree.pFirst; p; p=p->pNext){
      if( p->isDir && p->nFullName>nD ) nFile++;
    }
    zObjType = "folders";
    style_submenu_element("Files","Files","%s",
                          url_render(&sURI,"nofiles",0,0,0));
  }else{
    zObjType = "files";
    style_submenu_element("Folders","Folders","%s",
                          url_render(&sURI,"nofiles","1",0,0));
  }

  if( zCI ){
    cgi_printf("<h2>%d %s of check-in\n",(nFile),(zObjType));
    if( sqlite3_strnicmp(zCI, zUuid, (int)strlen(zCI))!=0 ){
      cgi_printf("\"%h\"\n",(zCI));
    }
    cgi_printf("[%z%S</a>] %s</h2>\n",(href("vinfo?name=%T",zUuid)),(zUuid),(blob_str(&dirname)));
  }else{
    int n = db_int(0, "SELECT count(*) FROM plink");
    cgi_printf("<h2>%d %s from all %d check-ins\n"
           "%s</h2>\n",(nFile),(zObjType),(n),(blob_str(&dirname)));
  }


  /* Generate tree of lists.
  **
  ** Each file and directory is a list element: <li>.  Files have class=file
  ** and if the filename as the suffix "xyz" the file also has class=file-xyz.
  ** Directories have class=dir.  The directory specfied by the name= query
  ** parameter (or the top-level directory if there is no name= query parameter)
  ** adds class=subdir.
  **
  ** The <li> element for directories also contains a sublist <ul>
  ** for the contents of that directory.
  */
  cgi_printf("<div class=\"filetree\"><ul>\n");
  if( nD ){
    cgi_printf("<li class=\"dir\">\n");
  }else{
    cgi_printf("<li class=\"dir subdir\">\n");
  }
  cgi_printf("%z%h</a>\n"
         "<ul>\n",(href("%s",url_render(&sURI,"name",0,0,0))),(zProjectName));
  for(p=sTree.pFirst; p; p=p->pNext){
    if( p->isDir ){
      if( p->nFullName==nD-1 ){
        cgi_printf("<li class=\"dir subdir\">\n");
      }else{
        cgi_printf("<li class=\"dir\">\n");
      }
      cgi_printf("%z%h</a>\n",(href("%s",url_render(&sURI,"name",p->zFullName,0,0))),(p->zName));
      if( startExpanded || p->nFullName<=nD ){
        cgi_printf("<ul>\n");
      }else{
        cgi_printf("<ul style='display:none;'>\n");
      }
    }else if( !showDirOnly ){
      char *zLink;
      if( zCI ){
        zLink = href("%R/artifact/%S",p->zUuid);
      }else{
        zLink = href("%R/finfo?name=%T",p->zFullName);
      }
      cgi_printf("<li class=\"%z\">%z%h</a>\n",(fileext_class(p->zName)),(zLink),(p->zName));
    }
    if( p->isLast ){
      int nClose = p->iLevel - (p->pNext ? p->pNext->iLevel : 0);
      while( nClose-- > 0 ){
        cgi_printf("</ul>\n");
      }
    }
  }
  cgi_printf("</ul>\n"
         "</ul></div>\n"
         "<script>(function(){\n"
         "function style(elem, prop){\n"
         "  return window.getComputedStyle(elem).getPropertyValue(prop);\n"
         "}\n"
         "\n"
         "function toggleAll(tree){\n"
         "  var lists = tree.querySelectorAll('.subdir > ul > li ul');\n"
         "  var display = 'block';  /* Default action: make all sublists visible */\n"
         "  for( var i=0; lists[i]; i++ ){\n"
         "    if( style(lists[i], 'display')!='none'){\n"
         "      display = 'none'; /* Any already visible - make them all hidden */\n"
         "      break;\n"
         "    }\n"
         "  }\n"
         "  for( var i=0; lists[i]; i++ ){\n"
         "    lists[i].style.display = display;\n"
         "  }\n"
         "}\n"
         "\n"
         "var outer_ul = document.querySelector('.filetree > ul');\n"
         "var subdir = outer_ul.querySelector('.subdir');\n"
         "outer_ul.onclick = function( e ){\n"
         "  var a = e.target;\n"
         "  if( a.nodeName!='A' ) return true;\n"
         "  if( a.parentNode==subdir ){\n"
         "    toggleAll(outer_ul);\n"
         "    return false;\n"
         "  }\n"
         "  if( !subdir.contains(a) ) return true;\n"
         "  var ul = a.nextSibling;\n"
         "  while( ul && ul.nodeName!='UL' ) ul = ul.nextSibling;\n"
         "  if( !ul ) return true; /* This is a file link, not a directory */\n"
         "  ul.style.display = style(ul, 'display')=='none' ? 'block' : 'none';\n"
         "  return false;\n"
         "}\n"
         "}())</script>\n");
  style_footer();

  /* We could free memory used by sTree here if we needed to.  But
  ** the process is about to exit, so doing so would not really accomplish
  ** anything useful. */
}

/*
** Return a CSS class name based on the given filename's extension.
** Result must be freed by the caller.
**/
const char *fileext_class(const char *zFilename){
  char *zClass;
  const char *zExt = strrchr(zFilename, '.');
  int isExt = zExt && zExt!=zFilename && zExt[1];
  int i;
  for( i=1; isExt && zExt[i]; i++ ) isExt &= fossil_isalnum(zExt[i]);
  if( isExt ){
    zClass = mprintf("file file-%s", zExt+1);
    for ( i=5; zClass[i]; i++ ) zClass[i] = fossil_tolower(zClass[i]);
  }else{
    zClass = mprintf("file");
  }
  return zClass;
}

/*
** Look at all file containing in the version "vid".  Construct a
** temporary table named "fileage" that contains the file-id for each
** files, the pathname, the check-in where the file was added, and the
** mtime on that checkin.
*/
int compute_fileage(int vid){
  Manifest *pManifest;
  ManifestFile *pFile;
  int nFile = 0;
  double vmtime;
  Stmt ins;
  Stmt q1, q2, q3;
  Stmt upd;
  db_multi_exec(
    /*"DROP TABLE IF EXISTS temp.fileage;"*/
    "CREATE TEMP TABLE fileage("
    "  fid INTEGER,"
    "  mid INTEGER,"
    "  mtime DATETIME,"
    "  pathname TEXT"
    ");"
    "CREATE INDEX fileage_fid ON fileage(fid);"
  );
  pManifest = manifest_get(vid, CFTYPE_MANIFEST, 0);
  if( pManifest==0 ) return 1;
  manifest_file_rewind(pManifest);
  db_prepare(&ins,
     "INSERT INTO temp.fileage(fid, pathname)"
     "  SELECT rid, :path FROM blob WHERE uuid=:uuid"
  );
  while( (pFile = manifest_file_next(pManifest, 0))!=0 ){
    db_bind_text(&ins, ":uuid", pFile->zUuid);
    db_bind_text(&ins, ":path", pFile->zName);
    db_step(&ins);
    db_reset(&ins);
    nFile++;
  }
  db_finalize(&ins);
  manifest_destroy(pManifest);
  db_prepare(&q1,"SELECT fid FROM mlink WHERE mid=:mid");
  db_prepare(&upd, "UPDATE fileage SET mid=:mid, mtime=:vmtime"
                      " WHERE fid=:fid AND mid IS NULL");
  db_prepare(&q2,"SELECT pid FROM plink WHERE cid=:vid AND isprim");
  db_prepare(&q3,"SELECT mtime FROM event WHERE objid=:vid");
  while( nFile>0 && vid>0 ){
    db_bind_int(&q3, ":vid", vid);
    if( db_step(&q3)==SQLITE_ROW ){
      vmtime = db_column_double(&q3, 0);
    }else{
      break;
    }
    db_reset(&q3);
    db_bind_int(&q1, ":mid", vid);
    db_bind_int(&upd, ":mid", vid);
    db_bind_double(&upd, ":vmtime", vmtime);
    while( db_step(&q1)==SQLITE_ROW ){
      db_bind_int(&upd, ":fid", db_column_int(&q1, 0));
      db_step(&upd);
      nFile -= db_changes();
      db_reset(&upd);
    }
    db_reset(&q1);
    db_bind_int(&q2, ":vid", vid);
    if( db_step(&q2)!=SQLITE_ROW ) break;
    vid = db_column_int(&q2, 0);
    db_reset(&q2);
  }
  db_finalize(&q1);
  db_finalize(&upd);
  db_finalize(&q2);
  db_finalize(&q3);
  return 0;
}

/*
** WEBPAGE:  fileage
**
** Parameters:
**   name=VERSION
*/
void fileage_page(void){
  int rid;
  const char *zName;
  char *zBaseTime;
  Stmt q;
  double baseTime;
  int lastMid = -1;

  login_check_credentials();
  if( !g.perm.Read ){ login_needed(); return; }
  zName = P("name");
  if( zName==0 ) zName = "tip";
  rid = symbolic_name_to_rid(zName, "ci");
  if( rid==0 ){
    fossil_fatal("not a valid check-in: %s", zName);
  }
  style_submenu_element("Tree-View", "Tree-View", "%R/tree?ci=%T", zName);
  style_header("File Ages", zName);
  compute_fileage(rid);
  baseTime = db_double(0.0, "SELECT mtime FROM event WHERE objid=%d", rid);
  zBaseTime = db_text("","SELECT datetime(%.20g%s)", baseTime, timeline_utc());
  cgi_printf("<h2>File Ages For Check-in\n"
         "%z%h</a></h2>\n"
         "\n"
         "<p>The times given are relative to\n"
         "%z%s</a>, which is the\n"
         "check-in time for\n"
         "%z%h</a></p>\n"
         "\n"
         "<table border=0 cellspacing=0 cellpadding=0>\n",(href("%R/info?name=%T",zName)),(zName),(href("%R/timeline?c=%T",zBaseTime)),(zBaseTime),(href("%R/info?name=%T",zName)),(zName));
  db_prepare(&q,
    "SELECT mtime, (SELECT uuid FROM blob WHERE rid=fid), mid, pathname"
    "  FROM fileage"
    " ORDER BY mtime DESC, mid, pathname"
  );
  while( db_step(&q)==SQLITE_ROW ){
    double age = baseTime - db_column_double(&q, 0);
    int mid = db_column_int(&q, 2);
    const char *zFUuid = db_column_text(&q, 1);
    char zAge[200];
    if( lastMid!=mid ){
      cgi_printf("<tr><td colspan=3><hr></tr>\n");
      lastMid = mid;
      if( age*86400.0<120 ){
        sqlite3_snprintf(sizeof(zAge), zAge, "%d seconds", (int)(age*86400.0));
      }else if( age*1440.0<90 ){
        sqlite3_snprintf(sizeof(zAge), zAge, "%.1f minutes", age*1440.0);
      }else if( age*24.0<36 ){
        sqlite3_snprintf(sizeof(zAge), zAge, "%.1f hours", age*24.0);
      }else if( age<365.0 ){
        sqlite3_snprintf(sizeof(zAge), zAge, "%.1f days", age);
      }else{
        sqlite3_snprintf(sizeof(zAge), zAge, "%.2f years", age/365.0);
      }
    }else{
      zAge[0] = 0;
    }
    cgi_printf("<tr>\n"
           "<td>%s\n"
           "<td width=\"25\">\n"
           "<td>%z%h</a>\n"
           "</tr>\n"
           "\n",(zAge),(href("%R/artifact/%S?ln", zFUuid)),(db_column_text(&q, 3)));
  }
  cgi_printf("<tr><td colspan=3><hr></tr>\n"
         "</table>\n");
  db_finalize(&q);
  style_footer();
}
