#line 1 "./src/info.c"
/*
** Copyright (c) 2007 D. Richard Hipp
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
** This file contains code to implement the "info" command.  The
** "info" command gives command-line access to information about
** the current tree, or a particular artifact or check-in.
*/
#include "config.h"
#include "info.h"
#include <assert.h>

/*
** Return a string (in memory obtained from malloc) holding a
** comma-separated list of tags that apply to check-in with
** record-id rid.  If the "propagatingOnly" flag is true, then only
** show branch tags (tags that propagate to children).
**
** Return NULL if there are no such tags.
*/
char *info_tags_of_checkin(int rid, int propagatingOnly){
  char *zTags;
  zTags = db_text(0, "SELECT group_concat(substr(tagname, 5), ', ')"
                     "  FROM tagxref, tag"
                     " WHERE tagxref.rid=%d AND tagxref.tagtype>%d"
                     "   AND tag.tagid=tagxref.tagid"
                     "   AND tag.tagname GLOB 'sym-*'",
                     rid, propagatingOnly!=0);
  return zTags;
}


/*
** Print common information about a particular record.
**
**     *  The UUID
**     *  The record ID
**     *  mtime and ctime
**     *  who signed it
*/
void show_common_info(
  int rid,                   /* The rid for the check-in to display info for */
  const char *zUuidName,     /* Name of the UUID */
  int showComment,           /* True to show the check-in comment */
  int showFamily             /* True to show parents and children */
){
  Stmt q;
  char *zComment = 0;
  char *zTags;
  char *zDate;
  char *zUuid;
  zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
  if( zUuid ){
    zDate = db_text(0,
      "SELECT datetime(mtime) || ' UTC' FROM event WHERE objid=%d",
      rid
    );
         /* 01234567890123 */
    fossil_print("%-13s %s %s\n", zUuidName, zUuid, zDate ? zDate : "");
    free(zUuid);
    free(zDate);
  }
  if( zUuid && showComment ){
    zComment = db_text(0,
      "SELECT coalesce(ecomment,comment) || "
      "       ' (user: ' || coalesce(euser,user,'?') || ')' "
      "  FROM event WHERE objid=%d",
      rid
    );
  }
  if( showFamily ){
    db_prepare(&q, "SELECT uuid, pid, isprim FROM plink JOIN blob ON pid=rid "
                   " WHERE cid=%d"
                   " ORDER BY isprim DESC, mtime DESC /*sort*/", rid);
    while( db_step(&q)==SQLITE_ROW ){
      const char *zUuid = db_column_text(&q, 0);
      const char *zType = db_column_int(&q, 2) ? "parent:" : "merged-from:";
      zDate = db_text("",
        "SELECT datetime(mtime) || ' UTC' FROM event WHERE objid=%d",
        db_column_int(&q, 1)
      );
      fossil_print("%-13s %s %s\n", zType, zUuid, zDate);
      free(zDate);
    }
    db_finalize(&q);
    db_prepare(&q, "SELECT uuid, cid, isprim FROM plink JOIN blob ON cid=rid "
                   " WHERE pid=%d"
                   " ORDER BY isprim DESC, mtime DESC /*sort*/", rid);
    while( db_step(&q)==SQLITE_ROW ){
      const char *zUuid = db_column_text(&q, 0);
      const char *zType = db_column_int(&q, 2) ? "child:" : "merged-into:";
      zDate = db_text("",
        "SELECT datetime(mtime) || ' UTC' FROM event WHERE objid=%d",
        db_column_int(&q, 1)
      );
      fossil_print("%-13s %s %s\n", zType, zUuid, zDate);
      free(zDate);
    }
    db_finalize(&q);
  }
  zTags = info_tags_of_checkin(rid, 0);
  if( zTags && zTags[0] ){
    fossil_print("tags:         %s\n", zTags);
  }
  free(zTags);
  if( zComment ){
    fossil_print("comment:      ");
    comment_print(zComment, 14, 79);
    free(zComment);
  }
}

/*
** Print information about the URLs used to access a repository and
** checkouts in a repository.
*/
static void extraRepoInfo(void){
  Stmt s;
  db_prepare(&s, "SELECT substr(name,7), date(mtime,'unixepoch')"
                 "  FROM config"
                 " WHERE name GLOB 'ckout:*' ORDER BY name");
  while( db_step(&s)==SQLITE_ROW ){
    const char *zName;
    const char *zCkout = db_column_text(&s, 0);
    if( g.localOpen ){
      if( fossil_strcmp(zCkout, g.zLocalRoot)==0 ) continue;
      zName = "alt-root:";
    }else{
      zName = "check-out:";
    }
    fossil_print("%-11s   %-54s %s\n", zName, zCkout,
                 db_column_text(&s, 1));
  }
  db_finalize(&s);
  db_prepare(&s, "SELECT substr(name,9), date(mtime,'unixepoch')"
                 "  FROM config"
                 " WHERE name GLOB 'baseurl:*' ORDER BY name");
  while( db_step(&s)==SQLITE_ROW ){
    fossil_print("access-url:   %-54s %s\n", db_column_text(&s, 0),
                 db_column_text(&s, 1));
  }
  db_finalize(&s);
}


/*
** COMMAND: info
**
** Usage: %fossil info ?VERSION | REPOSITORY_FILENAME? ?OPTIONS?
**
** With no arguments, provide information about the current tree.
** If an argument is specified, provide information about the object
** in the repository of the current tree that the argument refers
** to.  Or if the argument is the name of a repository, show
** information about that repository.
**
** Use the "finfo" command to get information about a specific
** file in a checkout.
**
** Options:
**
**    -R|--repository FILE       Extract info from repository FILE
**    -v|--verbose               Show extra information
**
** See also: annotate, artifact, finfo, timeline
*/
void info_cmd(void){
  i64 fsize;
  int verboseFlag = find_option("verbose","v",0)!=0;
  if( !verboseFlag ){
    verboseFlag = find_option("detail","l",0)!=0; /* deprecated */
  }
  if( g.argc==3 && (fsize = file_size(g.argv[2]))>0 && (fsize&0x1ff)==0 ){
    db_open_config(0);
    db_record_repository_filename(g.argv[2]);
    db_open_repository(g.argv[2]);
    fossil_print("project-name: %s\n", db_get("project-name", "<unnamed>"));
    fossil_print("project-code: %s\n", db_get("project-code", "<none>"));
    extraRepoInfo();
    return;
  }
  db_find_and_open_repository(0,0);
  if( g.argc==2 ){
    int vid;
         /* 012345678901234 */
    db_record_repository_filename(0);
    fossil_print("project-name: %s\n", db_get("project-name", "<unnamed>"));
    if( g.localOpen ){
      fossil_print("repository:   %s\n", db_repository_filename());
      fossil_print("local-root:   %s\n", g.zLocalRoot);
    }
    if( verboseFlag ) extraRepoInfo();
    if( g.zConfigDbName ){
      fossil_print("config-db:    %s\n", g.zConfigDbName);
    }
    fossil_print("project-code: %s\n", db_get("project-code", ""));
    vid = g.localOpen ? db_lget_int("checkout", 0) : 0;
    if( vid ){
      show_common_info(vid, "checkout:", 1, 1);
    }
    fossil_print("checkins:     %d\n",
                 db_int(-1, "SELECT count(*) FROM event WHERE type='ci' /*scan*/"));
  }else{
    int rid;
    rid = name_to_rid(g.argv[2]);
    if( rid==0 ){
      fossil_fatal("no such object: %s\n", g.argv[2]);
    }
    show_common_info(rid, "uuid:", 1, 1);
  }
}

/*
** Show information about all tags on a given node.
*/
static void showTags(int rid, const char *zNotGlob){
  Stmt q;
  int cnt = 0;
  db_prepare(&q,
    "SELECT tag.tagid, tagname, "
    "       (SELECT uuid FROM blob WHERE rid=tagxref.srcid AND rid!=%d),"
    "       value, datetime(tagxref.mtime%s), tagtype,"
    "       (SELECT uuid FROM blob WHERE rid=tagxref.origid AND rid!=%d)"
    "  FROM tagxref JOIN tag ON tagxref.tagid=tag.tagid"
    " WHERE tagxref.rid=%d AND tagname NOT GLOB '%q'"
    " ORDER BY tagname /*sort*/", rid, timeline_utc(), rid, rid, zNotGlob
  );
  while( db_step(&q)==SQLITE_ROW ){
    const char *zTagname = db_column_text(&q, 1);
    const char *zSrcUuid = db_column_text(&q, 2);
    const char *zValue = db_column_text(&q, 3);
    const char *zDate = db_column_text(&q, 4);
    int tagtype = db_column_int(&q, 5);
    const char *zOrigUuid = db_column_text(&q, 6);
    cnt++;
    if( cnt==1 ){
      cgi_printf("<div class=\"section\">Tags And Properties</div>\n"
             "<ul>\n");
    }
    cgi_printf("<li>\n");
    if( tagtype==0 ){
      cgi_printf("<span class=\"infoTagCancelled\">%h</span> cancelled\n",(zTagname));
    }else if( zValue ){
      cgi_printf("<span class=\"infoTag\">%h=%h</span>\n",(zTagname),(zValue));
    }else {
      cgi_printf("<span class=\"infoTag\">%h</span>\n",(zTagname));
    }
    if( tagtype==2 ){
      if( zOrigUuid && zOrigUuid[0] ){
        cgi_printf("inherited from\n");
        hyperlink_to_uuid(zOrigUuid);
      }else{
        cgi_printf("propagates to descendants\n");
      }
#if 0
      if( zValue && fossil_strcmp(zTagname,"branch")==0 ){
        cgi_printf("&nbsp;&nbsp;\n"
               "%zbranch timeline</a>\n",(href("%R/timeline?r=%T",zValue)));
      }
#endif
    }
    if( zSrcUuid && zSrcUuid[0] ){
      if( tagtype==0 ){
        cgi_printf("by\n");
      }else{
        cgi_printf("added by\n");
      }
      hyperlink_to_uuid(zSrcUuid);
      cgi_printf("on\n");
      hyperlink_to_date(zDate,0);
    }
    cgi_printf("</li>\n");
  }
  db_finalize(&q);
  if( cnt ){
    cgi_printf("</ul>\n");
  }
}


/*
** Append the difference between artifacts to the output
*/
static void append_diff(
  const char *zFrom,    /* Diff from this artifact */
  const char *zTo,      /*  ... to this artifact */
  u64 diffFlags,        /* Diff formatting flags */
  ReCompiled *pRe       /* Only show change matching this regex */
){
  int fromid;
  int toid;
  Blob from, to, out;
  if( zFrom ){
    fromid = uuid_to_rid(zFrom, 0);
    content_get(fromid, &from);
  }else{
    blob_zero(&from);
  }
  if( zTo ){
    toid = uuid_to_rid(zTo, 0);
    content_get(toid, &to);
  }else{
    blob_zero(&to);
  }
  blob_zero(&out);
  if( diffFlags & DIFF_SIDEBYSIDE ){
    text_diff(&from, &to, &out, pRe, diffFlags | DIFF_HTML | DIFF_NOTTOOBIG);
    cgi_printf("%s\n",(blob_str(&out)));
  }else{
    text_diff(&from, &to, &out, pRe,
           diffFlags | DIFF_LINENO | DIFF_HTML | DIFF_NOTTOOBIG);
    cgi_printf("<pre class=\"udiff\">\n"
           "%s\n"
           "</pre>\n",(blob_str(&out)));
  }
  blob_reset(&from);
  blob_reset(&to);
  blob_reset(&out);
}

/*
** Write a line of web-page output that shows changes that have occurred
** to a file between two check-ins.
*/
static void append_file_change_line(
  const char *zName,    /* Name of the file that has changed */
  const char *zOld,     /* blob.uuid before change.  NULL for added files */
  const char *zNew,     /* blob.uuid after change.  NULL for deletes */
  const char *zOldName, /* Prior name.  NULL if no name change. */
  u64 diffFlags,        /* Flags for text_diff().  Zero to omit diffs */
  ReCompiled *pRe,      /* Only show diffs that match this regex, if not NULL */
  int mperm             /* executable or symlink permission for zNew */
){
  if( !g.perm.Hyperlink ){
    if( zNew==0 ){
      cgi_printf("<p>Deleted %h</p>\n",(zName));
    }else if( zOld==0 ){
      cgi_printf("<p>Added %h</p>\n",(zName));
    }else if( zOldName!=0 && fossil_strcmp(zName,zOldName)!=0 ){
      cgi_printf("<p>Name change from %h to %h\n",(zOldName),(zName));
    }else if( fossil_strcmp(zNew, zOld)==0 ){
      cgi_printf("<p>Execute permission %s\n"
             " for %h</p>\n",(( mperm==PERM_EXE )?"set":"cleared"),(zName));
    }else{
      cgi_printf("<p>Changes to %h</p>\n",(zName));
    }
    if( diffFlags ){
      append_diff(zOld, zNew, diffFlags, pRe);
    }
  }else{
    if( zOld && zNew ){
      if( fossil_strcmp(zOld, zNew)!=0 ){
        cgi_printf("<p>Modified %z%h</a>\n"
               "from %z[%S]</a>\n"
               "to %z[%S].</a>\n",(href("%R/finfo?name=%T",zName)),(zName),(href("%R/artifact/%s",zOld)),(zOld),(href("%R/artifact/%s",zNew)),(zNew));
      }else if( zOldName!=0 && fossil_strcmp(zName,zOldName)!=0 ){
        cgi_printf("<p>Name change\n"
               "from %z%h</a>\n"
               "to %z%h</a>.\n",(href("%R/finfo?name=%T",zOldName)),(zOldName),(href("%R/finfo?name=%T",zName)),(zName));
      }else{
        cgi_printf("<p>Execute permission %s for\n"
               "%z%h</a>\n",(( mperm==PERM_EXE )?"set":"cleared"),(href("%R/finfo?name=%T",zName)),(zName));
      }
    }else if( zOld ){
      cgi_printf("<p>Deleted %z%h</a>\n"
             "version %z[%S]</a>\n",(href("%s/finfo?name=%T",g.zTop,zName)),(zName),(href("%R/artifact/%s",zOld)),(zOld));
    }else{
      cgi_printf("<p>Added %z%h</a>\n"
             "version %z[%S]</a>\n",(href("%R/finfo?name=%T",zName)),(zName),(href("%R/artifact/%s",zNew)),(zNew));
    }
    if( diffFlags ){
      append_diff(zOld, zNew, diffFlags, pRe);
    }else if( zOld && zNew && fossil_strcmp(zOld,zNew)!=0 ){
      cgi_printf("&nbsp;&nbsp;\n"
             "%z[diff]</a>\n",(href("%R/fdiff?v1=%S&v2=%S&sbs=1",zOld,zNew)));
    }
  }
}

/*
** Generate javascript to enhance HTML diffs.
*/
void append_diff_javascript(int sideBySide){
  if( !sideBySide ) return;
  cgi_printf("<script>(function(){\n"
         "var SCROLL_LEN = 25;\n"
         "function initSbsDiff(diff){\n"
         "  var txtCols = diff.querySelectorAll('.difftxtcol');\n"
         "  var txtPres = diff.querySelectorAll('.difftxtcol pre');\n"
         "  var width = Math.max(txtPres[0].scrollWidth, txtPres[1].scrollWidth);\n"
         "  for(var i=0; i<2; i++){\n"
         "    txtPres[i].style.width = width + 'px';\n"
         "    txtCols[i].onscroll = function(e){\n"
         "      txtCols[0].scrollLeft = txtCols[1].scrollLeft = this.scrollLeft;\n"
         "    };\n"
         "  }\n"
         "  diff.tabIndex = 0;\n"
         "  diff.onkeydown = function(e){\n"
         "    e = e || event;\n"
         "    var len = {37: -SCROLL_LEN, 39: SCROLL_LEN}[e.keyCode];\n"
         "    if( !len ) return;\n"
         "    txtCols[0].scrollLeft += len;\n"
         "    return false;\n"
         "  };\n"
         "}\n"
         "\n"
         "var diffs = document.querySelectorAll('.sbsdiffcols');\n"
         "for(var i=0; i<diffs.length; i++){\n"
         "  initSbsDiff(diffs[i]);\n"
         "}\n"
         "}())</script>\n");
}

/*
** Construct an appropriate diffFlag for text_diff() based on query
** parameters and the to boolean arguments.
*/
u64 construct_diff_flags(int verboseFlag, int sideBySide){
  u64 diffFlags;
  if( verboseFlag==0 ){
    diffFlags = 0;  /* Zero means do not show any diff */
  }else{
    int x;
    if( sideBySide ){
      diffFlags = DIFF_SIDEBYSIDE | DIFF_IGNORE_EOLWS;

      /* "dw" query parameter determines width of each column */
      x = atoi(PD("dw","80"))*(DIFF_CONTEXT_MASK+1);
      if( x<0 || x>DIFF_WIDTH_MASK ) x = DIFF_WIDTH_MASK;
      diffFlags += x;
    }else{
      diffFlags = DIFF_INLINE | DIFF_IGNORE_EOLWS;
    }

    /* "dc" query parameter determines lines of context */
    x = atoi(PD("dc","7"));
    if( x<0 || x>DIFF_CONTEXT_MASK ) x = DIFF_CONTEXT_MASK;
    diffFlags += x;

    /* The "noopt" parameter disables diff optimization */
    if( PD("noopt",0)!=0 ) diffFlags |= DIFF_NOOPT;
  }
  return diffFlags;
}


/*
** WEBPAGE: vinfo
** WEBPAGE: ci
** URL:  /ci?name=RID|ARTIFACTID
**
** Display information about a particular check-in.
**
** We also jump here from /info if the name is a version.
**
** If the /ci page is used (instead of /vinfo or /info) then the
** default behavior is to show unified diffs of all file changes.
** With /vinfo and /info, only a list of the changed files are
** shown, without diffs.  This behavior is inverted if the
** "show-version-diffs" setting is turned on.
*/
void ci_page(void){
  Stmt q1, q2, q3;
  int rid;
  int isLeaf;
  int verboseFlag;     /* True to show diffs */
  int sideBySide;      /* True for side-by-side diffs */
  u64 diffFlags;       /* Flag parameter for text_diff() */
  const char *zName;   /* Name of the checkin to be displayed */
  const char *zUuid;   /* UUID of zName */
  const char *zParent; /* UUID of the parent checkin (if any) */
  const char *zRe;     /* regex parameter */
  ReCompiled *pRe = 0; /* regex */

  login_check_credentials();
  if( !g.perm.Read ){ login_needed(); return; }
  zName = P("name");
  rid = name_to_rid_www("name");
  if( rid==0 ){
    style_header("Check-in Information Error");
    cgi_printf("No such object: %h\n",(g.argv[2]));
    style_footer();
    return;
  }
  zRe = P("regex");
  if( zRe ) re_compile(&pRe, zRe, 0);
  zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
  zParent = db_text(0,
    "SELECT uuid FROM plink, blob"
    " WHERE plink.cid=%d AND blob.rid=plink.pid AND plink.isprim",
    rid
  );
  isLeaf = is_a_leaf(rid);
  db_prepare(&q1,
     "SELECT uuid, datetime(mtime%s), user, comment,"
     "       datetime(omtime%s), mtime"
     "  FROM blob, event"
     " WHERE blob.rid=%d"
     "   AND event.objid=%d",
     timeline_utc(), timeline_utc(), rid, rid
  );
  sideBySide = !is_false(PD("sbs","1"));
  if( db_step(&q1)==SQLITE_ROW ){
    const char *zUuid = db_column_text(&q1, 0);
    char *zTitle = mprintf("Check-in [%.10s]", zUuid);
    char *zEUser, *zEComment;
    const char *zUser;
    const char *zComment;
    const char *zDate;
    const char *zOrigDate;

    style_header(zTitle);
    login_anonymous_available();
    free(zTitle);
    zEUser = db_text(0,
                   "SELECT value FROM tagxref"
                   " WHERE tagid=%d AND rid=%d AND tagtype>0",
                    TAG_USER, rid);
    zEComment = db_text(0,
                   "SELECT value FROM tagxref WHERE tagid=%d AND rid=%d",
                   TAG_COMMENT, rid);
    zUser = db_column_text(&q1, 2);
    zComment = db_column_text(&q1, 3);
    zDate = db_column_text(&q1,1);
    zOrigDate = db_column_text(&q1, 4);
    cgi_printf("<div class=\"section\">Overview</div>\n"
           "<table class=\"label-value\">\n"
           "<tr><th>SHA1&nbsp;Hash:</th><td>%s\n",(zUuid));
    if( g.perm.Setup ){
      cgi_printf("(Record ID: %d)\n",(rid));
    }
    cgi_printf("</td></tr>\n"
           "<tr><th>Date:</th><td>\n");
    hyperlink_to_date(zDate, "</td></tr>");
    if( zOrigDate && fossil_strcmp(zDate, zOrigDate)!=0 ){
      cgi_printf("<tr><th>Original&nbsp;Date:</th><td>\n");
      hyperlink_to_date(zOrigDate, "</td></tr>");
    }
    if( zEUser ){
      cgi_printf("<tr><th>Edited&nbsp;User:</th><td>\n");
      hyperlink_to_user(zEUser,zDate,"</td></tr>");
      cgi_printf("<tr><th>Original&nbsp;User:</th><td>\n");
      hyperlink_to_user(zUser,zDate,"</td></tr>");
    }else{
      cgi_printf("<tr><th>User:</th><td>\n");
      hyperlink_to_user(zUser,zDate,"</td></tr>");
    }
    if( zEComment ){
      cgi_printf("<tr><th>Edited&nbsp;Comment:</th><td class=\"infoComment\">%!w</td></tr>\n"
             "<tr><th>Original&nbsp;Comment:</th><td class=\"infoComment\">%!w</td></tr>\n",(zEComment),(zComment));
    }else{
      cgi_printf("<tr><th>Comment:</th><td class=\"infoComment\">%!w</td></tr>\n",(zComment));
    }
    if( g.perm.Admin ){
      db_prepare(&q2,
         "SELECT rcvfrom.ipaddr, user.login, datetime(rcvfrom.mtime)"
         "  FROM blob JOIN rcvfrom USING(rcvid) LEFT JOIN user USING(uid)"
         " WHERE blob.rid=%d",
         rid
      );
      if( db_step(&q2)==SQLITE_ROW ){
        const char *zIpAddr = db_column_text(&q2, 0);
        const char *zUser = db_column_text(&q2, 1);
        const char *zDate = db_column_text(&q2, 2);
        if( zUser==0 || zUser[0]==0 ) zUser = "unknown";
        cgi_printf("<tr><th>Received&nbsp;From:</th>\n"
               "<td>%h @ %h on %s</td></tr>\n",(zUser),(zIpAddr),(zDate));
      }
      db_finalize(&q2);
    }
    if( g.perm.Hyperlink ){
      char *zPJ = db_get("short-project-name", 0);
      Blob projName;
      int jj;
      if( zPJ==0 ) zPJ = db_get("project-name", "unnamed");
      blob_zero(&projName);
      blob_append(&projName, zPJ, -1);
      blob_trim(&projName);
      zPJ = blob_str(&projName);
      for(jj=0; zPJ[jj]; jj++){
        if( (zPJ[jj]>0 && zPJ[jj]<' ') || strchr("\"*/:<>?\\|", zPJ[jj]) ){
          zPJ[jj] = '_';
        }
      }
      cgi_printf("<tr><th>Timelines:</th><td>\n"
             "  %zfamily</a>\n",(href("%R/timeline?f=%S&unhide",zUuid)));
      if( zParent ){
        cgi_printf("| %zancestors</a>\n",(href("%R/timeline?p=%S&unhide",zUuid)));
      }
      if( !isLeaf ){
        cgi_printf("| %zdescendants</a>\n",(href("%R/timeline?d=%S&unhide",zUuid)));
      }
      if( zParent && !isLeaf ){
        cgi_printf("| %zboth</a>\n",(href("%R/timeline?dp=%S&unhide",zUuid)));
      }
      db_prepare(&q2,"SELECT substr(tag.tagname,5) FROM tagxref, tag "
                     " WHERE rid=%d AND tagtype>0 "
                     "   AND tag.tagid=tagxref.tagid "
                     "   AND +tag.tagname GLOB 'sym-*'", rid);
      while( db_step(&q2)==SQLITE_ROW ){
        const char *zTagName = db_column_text(&q2, 0);
        cgi_printf(" | %z%h</a>\n",(href("%R/timeline?r=%T&unhide",zTagName)),(zTagName));
      }
      db_finalize(&q2);


      /* The Download: line */
      if( g.perm.Zip ){
        char *zUrl = mprintf("%R/tarball/%t-%S.tar.gz?uuid=%s",
                             zPJ, zUuid, zUuid);
        cgi_printf("</td></tr>\n"
               "<tr><th>Downloads:</th><td>\n"
               "%zTarball</a>\n"
               "| %z\n"
               "        ZIP archive</a>\n",(href("%s",zUrl)),(href("%R/zip/%t-%S.zip?uuid=%s",zPJ,zUuid,zUuid)));
        fossil_free(zUrl);
      }
      cgi_printf("</td></tr>\n"
             "<tr><th>Other&nbsp;Links:</th>\n"
             "  <td>\n"
             "    %zfiles</a>\n"
             "  | %zfile ages</a>\n"
             "  | %zfolders</a>\n"
             "  | %zmanifest</a>\n",(href("%R/tree?ci=%S",zUuid)),(href("%R/fileage?name=%S",zUuid)),(href("%R/tree?ci=%S&nofiles",zUuid)),(href("%R/artifact/%S",zUuid)));
      if( g.perm.Write ){
        cgi_printf("  | %zedit</a>\n",(href("%R/ci_edit?r=%S",zUuid)));
      }
      cgi_printf("  </td>\n"
             "</tr>\n");
      blob_reset(&projName);
    }
    cgi_printf("</table>\n");
  }else{
    style_header("Check-in Information");
    login_anonymous_available();
  }
  db_finalize(&q1);
  showTags(rid, "");
  if( zParent ){
    cgi_printf("<div class=\"section\">Changes</div>\n"
           "<div class=\"sectionmenu\">\n");
    verboseFlag = g.zPath[0]!='c';
    if( db_get_boolean("show-version-diffs", 0)==0 ){
      verboseFlag = !verboseFlag;
      if( verboseFlag ){
        cgi_printf("%z\n"
               "hide&nbsp;diffs</a>\n",(xhref("class='button'","%R/vinfo/%T",zName)));
        if( sideBySide ){
          cgi_printf("%z\n"
                 "unified&nbsp;diffs</a>\n",(xhref("class='button'","%R/ci/%T?sbs=0",zName)));
        }else{
          cgi_printf("%z\n"
                 "side-by-side&nbsp;diffs</a>\n",(xhref("class='button'","%R/ci/%T?sbs=1",zName)));
        }
      }else{
        cgi_printf("%z\n"
               "show&nbsp;unified&nbsp;diffs</a>\n"
               "%z\n"
               "show&nbsp;side-by-side&nbsp;diffs</a>\n",(xhref("class='button'","%R/ci/%T?sbs=0",zName)),(xhref("class='button'","%R/ci/%T?sbs=1",zName)));
      }
    }else{
      if( verboseFlag ){
        cgi_printf("%zhide&nbsp;diffs</a>\n",(xhref("class='button'","%R/ci/%T",zName)));
        if( sideBySide ){
          cgi_printf("%z\n"
                 "unified&nbsp;diffs</a>\n",(xhref("class='button'","%R/info/%T?sbs=0",zName)));
        }else{
          cgi_printf("%z\n"
                 "side-by-side&nbsp;diffs</a>\n",(xhref("class='button'","%R/info/%T?sbs=1",zName)));
        }
      }else{
        cgi_printf("%z\n"
               "show&nbsp;unified&nbsp;diffs</a>\n"
               "%z\n"
               "show&nbsp;side-by-side&nbsp;diffs</a>\n",(xhref("class='button'","%R/vinfo/%T?sbs=0",zName)),(xhref("class='button'","%R/vinfo/%T?sbs=1",zName)));
      }
    }
    cgi_printf("%z\n"
           "patch</a></div>\n",(xhref("class='button'","%R/vpatch?from=%S&to=%S",zParent,zUuid)));
    if( pRe ){
      cgi_printf("<p><b>Only differences that match regular expression \"%h\"\n"
             "are shown.</b></p>\n",(zRe));
    }
    db_prepare(&q3,
       "SELECT name,"
       "       mperm,"
       "       (SELECT uuid FROM blob WHERE rid=mlink.pid),"
       "       (SELECT uuid FROM blob WHERE rid=mlink.fid),"
       "       (SELECT name FROM filename WHERE filename.fnid=mlink.pfnid)"
       "  FROM mlink JOIN filename ON filename.fnid=mlink.fnid"
       " WHERE mlink.mid=%d"
       "   AND (mlink.fid>0"
              " OR mlink.fnid NOT IN (SELECT pfnid FROM mlink WHERE mid=%d))"
       " ORDER BY name /*sort*/",
       rid, rid
    );
    diffFlags = construct_diff_flags(verboseFlag, sideBySide);
    while( db_step(&q3)==SQLITE_ROW ){
      const char *zName = db_column_text(&q3,0);
      int mperm = db_column_int(&q3, 1);
      const char *zOld = db_column_text(&q3,2);
      const char *zNew = db_column_text(&q3,3);
      const char *zOldName = db_column_text(&q3, 4);
      append_file_change_line(zName, zOld, zNew, zOldName, diffFlags,pRe,mperm);
    }
    db_finalize(&q3);
  }
  append_diff_javascript(sideBySide);
  style_footer();
}

/*
** WEBPAGE: winfo
** URL:  /winfo?name=UUID
**
** Return information about a wiki page.
*/
void winfo_page(void){
  int rid;
  Manifest *pWiki;
  char *zUuid;
  char *zDate;
  Blob wiki;
  int modPending;
  const char *zModAction;

  login_check_credentials();
  if( !g.perm.RdWiki ){ login_needed(); return; }
  rid = name_to_rid_www("name");
  if( rid==0 || (pWiki = manifest_get(rid, CFTYPE_WIKI, 0))==0 ){
    style_header("Wiki Page Information Error");
    cgi_printf("No such object: %h\n",(P("name")));
    style_footer();
    return;
  }
  if( g.perm.ModWiki && (zModAction = P("modaction"))!=0 ){
    if( strcmp(zModAction,"delete")==0 ){
      moderation_disapprove(rid);
      cgi_redirectf("%R/wiki?name=%T", pWiki->zWikiTitle);
      /*NOTREACHED*/
    }
    if( strcmp(zModAction,"approve")==0 ){
      moderation_approve(rid);
    }
  }
  style_header("Update of \"%h\"", pWiki->zWikiTitle);
  zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
  zDate = db_text(0, "SELECT datetime(%.17g)", pWiki->rDate);
  style_submenu_element("Raw", "Raw", "artifact/%S", zUuid);
  style_submenu_element("History", "History", "whistory?name=%t",
                        pWiki->zWikiTitle);
  style_submenu_element("Page", "Page", "wiki?name=%t",
                        pWiki->zWikiTitle);
  login_anonymous_available();
  cgi_printf("<div class=\"section\">Overview</div>\n"
         "<p><table class=\"label-value\">\n"
         "<tr><th>Artifact&nbsp;ID:</th>\n"
         "<td>%z%s</a>\n",(href("%R/artifact/%s",zUuid)),(zUuid));
  if( g.perm.Setup ){
    cgi_printf("(%d)\n",(rid));
  }
  modPending = moderation_pending(rid);
  if( modPending ){
    cgi_printf("<span class=\"modpending\">*** Awaiting Moderator Approval ***</span>\n");
  }
  cgi_printf("</td></tr>\n"
         "<tr><th>Page&nbsp;Name:</th><td>%h</td></tr>\n"
         "<tr><th>Date:</th><td>\n",(pWiki->zWikiTitle));
  hyperlink_to_date(zDate, "</td></tr>");
  cgi_printf("<tr><th>Original&nbsp;User:</th><td>\n");
  hyperlink_to_user(pWiki->zUser, zDate, "</td></tr>");
  if( pWiki->nParent>0 ){
    int i;
    cgi_printf("<tr><th>Parent%s:</th><td>\n",(pWiki->nParent==1?"":"s"));
    for(i=0; i<pWiki->nParent; i++){
      char *zParent = pWiki->azParent[i];
      cgi_printf("%z%s</a>\n",(href("info/%S",zParent)),(zParent));
    }
    cgi_printf("</td></tr>\n");
  }
  cgi_printf("</table>\n");

  if( g.perm.ModWiki && modPending ){
    cgi_printf("<div class=\"section\">Moderation</div>\n"
           "<blockquote>\n"
           "<form method=\"POST\" action=\"%R/winfo/%s\">\n"
           "<label><input type=\"radio\" name=\"modaction\" value=\"delete\">\n"
           "Delete this change</label><br />\n"
           "<label><input type=\"radio\" name=\"modaction\" value=\"approve\">\n"
           "Approve this change</label><br />\n"
           "<input type=\"submit\" value=\"Submit\">\n"
           "</form>\n"
           "</blockquote>\n",(zUuid));
  }


  cgi_printf("<div class=\"section\">Content</div>\n");
  blob_init(&wiki, pWiki->zWiki, -1);
  wiki_convert(&wiki, 0, 0);
  blob_reset(&wiki);
  manifest_destroy(pWiki);
  style_footer();
}

/*
** Show a webpage error message
*/
void webpage_error(const char *zFormat, ...){
  va_list ap;
  const char *z;
  va_start(ap, zFormat);
  z = vmprintf(zFormat, ap);
  va_end(ap);
  style_header("URL Error");
  cgi_printf("<h1>Error</h1>\n"
         "<p>%h</p>\n",(z));
  style_footer();
}

/*
** Find an checkin based on query parameter zParam and parse its
** manifest.  Return the number of errors.
*/
static Manifest *vdiff_parse_manifest(const char *zParam, int *pRid){
  int rid;

  *pRid = rid = name_to_rid_www(zParam);
  if( rid==0 ){
    const char *z = P(zParam);
    if( z==0 || z[0]==0 ){
      webpage_error("Missing \"%s\" query parameter.", zParam);
    }else{
      webpage_error("No such artifact: \"%s\"", z);
    }
    return 0;
  }
  if( !is_a_version(rid) ){
    webpage_error("Artifact %s is not a checkin.", P(zParam));
    return 0;
  }
  return manifest_get(rid, CFTYPE_MANIFEST, 0);
}

/*
** Output a description of a check-in
*/
static void checkin_description(int rid){
  Stmt q;
  db_prepare(&q,
    "SELECT datetime(mtime), coalesce(euser,user),"
    "       coalesce(ecomment,comment), uuid,"
    "      (SELECT group_concat(substr(tagname,5), ', ') FROM tag, tagxref"
    "        WHERE tagname GLOB 'sym-*' AND tag.tagid=tagxref.tagid"
    "          AND tagxref.rid=blob.rid AND tagxref.tagtype>0)"
    "  FROM event, blob"
    " WHERE event.objid=%d AND type='ci'"
    "   AND blob.rid=%d",
    rid, rid
  );
  while( db_step(&q)==SQLITE_ROW ){
    const char *zDate = db_column_text(&q, 0);
    const char *zUser = db_column_text(&q, 1);
    const char *zUuid = db_column_text(&q, 3);
    const char *zTagList = db_column_text(&q, 4);
    Blob comment;
    int wikiFlags = WIKI_INLINE|WIKI_NOBADLINKS;
    if( db_get_boolean("timeline-block-markup", 0)==0 ){
      wikiFlags |= WIKI_NOBLOCK;
    }
    hyperlink_to_uuid(zUuid);
    blob_zero(&comment);
    db_column_blob(&q, 2, &comment);
    wiki_convert(&comment, 0, wikiFlags);
    blob_reset(&comment);
    cgi_printf("(user:\n");
    hyperlink_to_user(zUser,zDate,",");
    if( zTagList && zTagList[0] && g.perm.Hyperlink ){
      int i;
      const char *z = zTagList;
      Blob links;
      blob_zero(&links);
      while( z && z[0] ){
        for(i=0; z[i] && (z[i]!=',' || z[i+1]!=' '); i++){}
        blob_appendf(&links,
              "%z%#h</a>%.2s",
              href("%R/timeline?r=%#t&nd&c=%t",i,z,zDate), i,z, &z[i]
        );
        if( z[i]==0 ) break;
        z += i+2;
      }
      cgi_printf("tags: %s,\n",(blob_str(&links)));
      blob_reset(&links);
    }else{
      cgi_printf("tags: %h,\n",(zTagList));
    }
    cgi_printf("date:\n");
    hyperlink_to_date(zDate, ")");
  }
  db_finalize(&q);
}


/*
** WEBPAGE: vdiff
** URL: /vdiff
**
** Query parameters:
**
**   from=TAG
**   to=TAG
**   branch=TAG
**   v=BOOLEAN
**   sbs=BOOLEAN
**
**
** Show all differences between two checkins.
*/
void vdiff_page(void){
  int ridFrom, ridTo;
  int verboseFlag = 0;
  int sideBySide = 0;
  u64 diffFlags = 0;
  Manifest *pFrom, *pTo;
  ManifestFile *pFileFrom, *pFileTo;
  const char *zBranch;
  const char *zFrom;
  const char *zTo;
  const char *zRe;
  const char *zVerbose;
  ReCompiled *pRe = 0;

  login_check_credentials();
  if( !g.perm.Read ){ login_needed(); return; }
  login_anonymous_available();

  zRe = P("regex");
  if( zRe ) re_compile(&pRe, zRe, 0);
  zBranch = P("branch");
  if( zBranch && zBranch[0] ){
    cgi_replace_parameter("from", mprintf("root:%s", zBranch));
    cgi_replace_parameter("to", zBranch);
  }
  pTo = vdiff_parse_manifest("to", &ridTo);
  if( pTo==0 ) return;
  pFrom = vdiff_parse_manifest("from", &ridFrom);
  if( pFrom==0 ) return;
  sideBySide = !is_false(PD("sbs","1"));
  zVerbose = P("v");
  if( !zVerbose ){
    zVerbose = P("verbose");
  }
  if( !zVerbose ){
    zVerbose = P("detail"); /* deprecated */
  }
  verboseFlag = (zVerbose!=0) && !is_false(zVerbose);
  if( !verboseFlag && sideBySide ) verboseFlag = 1;
  zFrom = P("from");
  zTo = P("to");
  if( sideBySide || verboseFlag ){
    style_submenu_element("Hide Diff", "hidediff",
                          "%R/vdiff?from=%T&to=%T&sbs=0",
                          zFrom, zTo);
  }
  if( !sideBySide ){
    style_submenu_element("Side-by-side Diff", "sbsdiff",
                          "%R/vdiff?from=%T&to=%T&sbs=1",
                          zFrom, zTo);
  }
  if( sideBySide || !verboseFlag ) {
    style_submenu_element("Unified Diff", "udiff",
                          "%R/vdiff?from=%T&to=%T&sbs=0&v",
                          zFrom, zTo);
  }
  style_submenu_element("Invert", "invert",
                        "%R/vdiff?from=%T&to=%T&sbs=%d%s", zTo, zFrom,
                        sideBySide, (verboseFlag && !sideBySide)?"&v":"");
  style_header("Check-in Differences");
  cgi_printf("<h2>Difference From:</h2><blockquote>\n");
  checkin_description(ridFrom);
  cgi_printf("</blockquote><h2>To:</h2><blockquote>\n");
  checkin_description(ridTo);
  cgi_printf("</blockquote>\n");
  if( pRe ){
    cgi_printf("<p><b>Only differences that match regular expression \"%h\"\n"
           "are shown.</b></p>\n",(zRe));
  }
 cgi_printf("<hr /><p>\n");

  manifest_file_rewind(pFrom);
  pFileFrom = manifest_file_next(pFrom, 0);
  manifest_file_rewind(pTo);
  pFileTo = manifest_file_next(pTo, 0);
  diffFlags = construct_diff_flags(verboseFlag, sideBySide);
  while( pFileFrom || pFileTo ){
    int cmp;
    if( pFileFrom==0 ){
      cmp = +1;
    }else if( pFileTo==0 ){
      cmp = -1;
    }else{
      cmp = fossil_strcmp(pFileFrom->zName, pFileTo->zName);
    }
    if( cmp<0 ){
      append_file_change_line(pFileFrom->zName,
                              pFileFrom->zUuid, 0, 0, diffFlags, pRe, 0);
      pFileFrom = manifest_file_next(pFrom, 0);
    }else if( cmp>0 ){
      append_file_change_line(pFileTo->zName,
                              0, pFileTo->zUuid, 0, diffFlags, pRe,
                              manifest_file_mperm(pFileTo));
      pFileTo = manifest_file_next(pTo, 0);
    }else if( fossil_strcmp(pFileFrom->zUuid, pFileTo->zUuid)==0 ){
      /* No changes */
      pFileFrom = manifest_file_next(pFrom, 0);
      pFileTo = manifest_file_next(pTo, 0);
    }else{
      append_file_change_line(pFileFrom->zName,
                              pFileFrom->zUuid,
                              pFileTo->zUuid, 0, diffFlags, pRe,
                              manifest_file_mperm(pFileTo));
      pFileFrom = manifest_file_next(pFrom, 0);
      pFileTo = manifest_file_next(pTo, 0);
    }
  }
  manifest_destroy(pFrom);
  manifest_destroy(pTo);
  append_diff_javascript(sideBySide);
  style_footer();
}

#if INTERFACE
/*
** Possible return values from object_description()
*/
#define OBJTYPE_CHECKIN    0x0001
#define OBJTYPE_CONTENT    0x0002
#define OBJTYPE_WIKI       0x0004
#define OBJTYPE_TICKET     0x0008
#define OBJTYPE_ATTACHMENT 0x0010
#define OBJTYPE_EVENT      0x0020
#define OBJTYPE_TAG        0x0040
#define OBJTYPE_SYMLINK    0x0080
#define OBJTYPE_EXE        0x0100
#endif

/*
** Write a description of an object to the www reply.
**
** If the object is a file then mention:
**
**     * It's artifact ID
**     * All its filenames
**     * The check-in it was part of, with times and users
**
** If the object is a manifest, then mention:
**
**     * It's artifact ID
**     * date of check-in
**     * Comment & user
*/
int object_description(
  int rid,                 /* The artifact ID */
  int linkToView,          /* Add viewer link if true */
  Blob *pDownloadName      /* Fill with an appropriate download name */
){
  Stmt q;
  int cnt = 0;
  int nWiki = 0;
  int objType = 0;
  char *zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);

  char *prevName = 0;

  db_prepare(&q,
    "SELECT filename.name, datetime(event.mtime),"
    "       coalesce(event.ecomment,event.comment),"
    "       coalesce(event.euser,event.user),"
    "       b.uuid, mlink.mperm,"
    "       coalesce((SELECT value FROM tagxref"
                    "  WHERE tagid=%d AND tagtype>0 AND rid=mlink.mid),'trunk')"
    "  FROM mlink, filename, event, blob a, blob b"
    " WHERE filename.fnid=mlink.fnid"
    "   AND event.objid=mlink.mid"
    "   AND a.rid=mlink.fid"
    "   AND b.rid=mlink.mid"
    "   AND mlink.fid=%d"
    "   ORDER BY filename.name, event.mtime /*sort*/",
    TAG_BRANCH, rid
  );
  cgi_printf("<ul>\n");
  while( db_step(&q)==SQLITE_ROW ){
    const char *zName = db_column_text(&q, 0);
    const char *zDate = db_column_text(&q, 1);
    const char *zCom = db_column_text(&q, 2);
    const char *zUser = db_column_text(&q, 3);
    const char *zVers = db_column_text(&q, 4);
    int mPerm = db_column_int(&q, 5);
    const char *zBr = db_column_text(&q, 6);
    if( !prevName || fossil_strcmp(zName, prevName) ) {
      if( prevName ) {
        cgi_printf("</ul>\n");
      }
      if( mPerm==PERM_LNK ){
        cgi_printf("<li>Symbolic link\n");
        objType |= OBJTYPE_SYMLINK;
      }else if( mPerm==PERM_EXE ){
        cgi_printf("<li>Executable file\n");
        objType |= OBJTYPE_EXE;
      }else{
        cgi_printf("<li>File\n");
      }
      objType |= OBJTYPE_CONTENT;
      cgi_printf("%z%h</a>\n"
             "<ul>\n",(href("%R/finfo?name=%T",zName)),(zName));
      prevName = fossil_strdup(zName);
    }
    cgi_printf("<li>\n");
    hyperlink_to_date(zDate,"");
    cgi_printf("- part of checkin\n");
    hyperlink_to_uuid(zVers);
    if( zBr && zBr[0] ){
      cgi_printf("on branch %z%h</a>\n",(href("%R/timeline?r=%T",zBr)),(zBr));
    }
    cgi_printf("- %!w (user:\n",(zCom));
    hyperlink_to_user(zUser,zDate,")");
    if( g.perm.Hyperlink ){
      cgi_printf("%z[ancestry]</a>\n"
             "%z\n"
             "[annotate]</a>\n"
             "%z\n"
             "[blame]</a>\n",(href("%R/finfo?name=%T&ci=%S",zName,zVers)),(href("%R/annotate?checkin=%S&filename=%T",zVers,zName)),(href("%R/blame?checkin=%S&filename=%T",zVers,zName)));
    }
    cnt++;
    if( pDownloadName && blob_size(pDownloadName)==0 ){
      blob_append(pDownloadName, zName, -1);
    }
  }
  if( prevName ){
    cgi_printf("</ul>\n");
  }
  cgi_printf("</ul>\n");
  free(prevName);
  db_finalize(&q);
  db_prepare(&q,
    "SELECT substr(tagname, 6, 10000), datetime(event.mtime),"
    "       coalesce(event.euser, event.user)"
    "  FROM tagxref, tag, event"
    " WHERE tagxref.rid=%d"
    "   AND tag.tagid=tagxref.tagid"
    "   AND tag.tagname LIKE 'wiki-%%'"
    "   AND event.objid=tagxref.rid",
    rid
  );
  while( db_step(&q)==SQLITE_ROW ){
    const char *zPagename = db_column_text(&q, 0);
    const char *zDate = db_column_text(&q, 1);
    const char *zUser = db_column_text(&q, 2);
    if( cnt>0 ){
      cgi_printf("Also wiki page\n");
    }else{
      cgi_printf("Wiki page\n");
    }
    objType |= OBJTYPE_WIKI;
    cgi_printf("[%z%h</a>] by\n",(href("%R/wiki?name=%t",zPagename)),(zPagename));
    hyperlink_to_user(zUser,zDate," on");
    hyperlink_to_date(zDate,".");
    nWiki++;
    cnt++;
    if( pDownloadName && blob_size(pDownloadName)==0 ){
      blob_appendf(pDownloadName, "%s.txt", zPagename);
    }
  }
  db_finalize(&q);
  if( nWiki==0 ){
    db_prepare(&q,
      "SELECT datetime(mtime), user, comment, type, uuid, tagid"
      "  FROM event, blob"
      " WHERE event.objid=%d"
      "   AND blob.rid=%d",
      rid, rid
    );
    while( db_step(&q)==SQLITE_ROW ){
      const char *zDate = db_column_text(&q, 0);
      const char *zUser = db_column_text(&q, 1);
      const char *zCom = db_column_text(&q, 2);
      const char *zType = db_column_text(&q, 3);
      const char *zUuid = db_column_text(&q, 4);
      if( cnt>0 ){
        cgi_printf("Also\n");
      }
      if( zType[0]=='w' ){
        cgi_printf("Wiki edit\n");
        objType |= OBJTYPE_WIKI;
      }else if( zType[0]=='t' ){
        cgi_printf("Ticket change\n");
        objType |= OBJTYPE_TICKET;
      }else if( zType[0]=='c' ){
        cgi_printf("Manifest of check-in\n");
        objType |= OBJTYPE_CHECKIN;
      }else if( zType[0]=='e' ){
        cgi_printf("Instance of event\n");
        objType |= OBJTYPE_EVENT;
        hyperlink_to_event_tagid(db_column_int(&q, 5));
      }else{
        cgi_printf("Control file referencing\n");
      }
      if( zType[0]!='e' ){
        hyperlink_to_uuid(zUuid);
      }
      cgi_printf("- %!w by\n",(zCom));
      hyperlink_to_user(zUser,zDate," on");
      hyperlink_to_date(zDate, ".");
      if( pDownloadName && blob_size(pDownloadName)==0 ){
        blob_appendf(pDownloadName, "%.10s.txt", zUuid);
      }
      cnt++;
    }
    db_finalize(&q);
  }
  db_prepare(&q,
    "SELECT target, filename, datetime(mtime), user, src"
    "  FROM attachment"
    " WHERE src=(SELECT uuid FROM blob WHERE rid=%d)"
    " ORDER BY mtime DESC /*sort*/",
    rid
  );
  while( db_step(&q)==SQLITE_ROW ){
    const char *zTarget = db_column_text(&q, 0);
    const char *zFilename = db_column_text(&q, 1);
    const char *zDate = db_column_text(&q, 2);
    const char *zUser = db_column_text(&q, 3);
    /* const char *zSrc = db_column_text(&q, 4); */
    if( cnt>0 ){
      cgi_printf("Also attachment \"%h\" to\n",(zFilename));
    }else{
      cgi_printf("Attachment \"%h\" to\n",(zFilename));
    }
    objType |= OBJTYPE_ATTACHMENT;
    if( strlen(zTarget)==UUID_SIZE && validate16(zTarget,UUID_SIZE) ){
      if( g.perm.Hyperlink && g.perm.RdTkt ){
        cgi_printf("ticket [%z%S</a>]\n",(href("%R/tktview?name=%S",zTarget)),(zTarget));
      }else{
        cgi_printf("ticket [%S]\n",(zTarget));
      }
    }else{
      if( g.perm.Hyperlink && g.perm.RdWiki ){
        cgi_printf("wiki page [%z%h</a>]\n",(href("%R/wiki?name=%t",zTarget)),(zTarget));
      }else{
        cgi_printf("wiki page [%h]\n",(zTarget));
      }
    }
    cgi_printf("added by\n");
    hyperlink_to_user(zUser,zDate," on");
    hyperlink_to_date(zDate,".");
    cnt++;
    if( pDownloadName && blob_size(pDownloadName)==0 ){
      blob_append(pDownloadName, zFilename, -1);
    }
  }
  db_finalize(&q);
  if( cnt==0 ){
    cgi_printf("Control artifact.\n");
    if( pDownloadName && blob_size(pDownloadName)==0 ){
      blob_appendf(pDownloadName, "%.10s.txt", zUuid);
    }
  }else if( linkToView && g.perm.Hyperlink ){
    cgi_printf("%z[view]</a>\n",(href("%R/artifact/%S",zUuid)));
  }
  return objType;
}


/*
** WEBPAGE: fdiff
** URL: fdiff?v1=UUID&v2=UUID&patch&sbs=BOOLEAN&regex=REGEX
**
** Two arguments, v1 and v2, identify the files to be diffed.  Show the
** difference between the two artifacts.  Show diff side by side unless sbs
** is 0.  Generate plaintext if "patch" is present.
*/
void diff_page(void){
  int v1, v2;
  int isPatch;
  int sideBySide;
  char *zV1;
  char *zV2;
  const char *zRe;
  ReCompiled *pRe = 0;
  u64 diffFlags;

  login_check_credentials();
  if( !g.perm.Read ){ login_needed(); return; }
  v1 = name_to_rid_www("v1");
  v2 = name_to_rid_www("v2");
  if( v1==0 || v2==0 ) fossil_redirect_home();
  zRe = P("regex");
  if( zRe ) re_compile(&pRe, zRe, 0);
  isPatch = P("patch")!=0;
  if( isPatch ){
    Blob c1, c2, *pOut;
    pOut = cgi_output_blob();
    cgi_set_content_type("text/plain");
    diffFlags = 4;
    content_get(v1, &c1);
    content_get(v2, &c2);
    text_diff(&c1, &c2, pOut, pRe, diffFlags);
    blob_reset(&c1);
    blob_reset(&c2);
    return;
  }

  sideBySide = !is_false(PD("sbs","1"));
  zV1 = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", v1);
  zV2 = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", v2);
  diffFlags = construct_diff_flags(1, sideBySide) | DIFF_HTML;

  style_header("Diff");
  style_submenu_element("Patch", "Patch", "%s/fdiff?v1=%T&v2=%T&patch",
                        g.zTop, P("v1"), P("v2"));
  if( !sideBySide ){
    style_submenu_element("Side-by-side Diff", "sbsdiff",
                          "%s/fdiff?v1=%T&v2=%T&sbs=1",
                          g.zTop, P("v1"), P("v2"));
  }else{
    style_submenu_element("Unified Diff", "udiff",
                          "%s/fdiff?v1=%T&v2=%T&sbs=0",
                          g.zTop, P("v1"), P("v2"));
  }

  if( P("smhdr")!=0 ){
    cgi_printf("<h2>Differences From Artifact\n"
           "%z[%S]</a> To\n"
           "%z[%S]</a>.</h2>\n",(href("%R/artifact/%S",zV1)),(zV1),(href("%R/artifact/%S",zV2)),(zV2));
  }else{
    cgi_printf("<h2>Differences From\n"
           "Artifact %z[%S]</a>:</h2>\n",(href("%R/artifact/%S",zV1)),(zV1));
    object_description(v1, 0, 0);
    cgi_printf("<h2>To Artifact %z[%S]</a>:</h2>\n",(href("%R/artifact/%S",zV2)),(zV2));
    object_description(v2, 0, 0);
  }
  if( pRe ){
    cgi_printf("<b>Only differences that match regular expression \"%h\"\n"
           "are shown.</b>\n",(zRe));
  }
  cgi_printf("<hr />\n");
  append_diff(zV1, zV2, diffFlags, pRe);
  append_diff_javascript(sideBySide);
  style_footer();
}

/*
** WEBPAGE: raw
** URL: /raw?name=ARTIFACTID&m=TYPE
**
** Return the uninterpreted content of an artifact.  Used primarily
** to view artifacts that are images.
*/
void rawartifact_page(void){
  int rid;
  char *zUuid;
  const char *zMime;
  Blob content;

  rid = name_to_rid_www("name");
  login_check_credentials();
  if( !g.perm.Read ){ login_needed(); return; }
  if( rid==0 ) fossil_redirect_home();
  zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
  if( fossil_strcmp(P("name"), zUuid)==0 ){
    g.isConst = 1;
  }
  free(zUuid);
  zMime = P("m");
  if( zMime==0 ){
    char *zFName = db_text(0, "SELECT filename.name FROM mlink, filename"
                              " WHERE mlink.fid=%d"
                              "   AND filename.fnid=mlink.fnid", rid);
    if( !zFName ){
      /* Look also at the attachment table */
      zFName = db_text(0, "SELECT attachment.filename FROM attachment, blob"
                          " WHERE blob.rid=%d"
                          "   AND attachment.src=blob.uuid", rid);
    }
    if( zFName ) zMime = mimetype_from_name(zFName);
    if( zMime==0 ) zMime = "application/x-fossil-artifact";
  }
  content_get(rid, &content);
  cgi_set_content_type(zMime);
  cgi_set_content(&content);
}

/*
** Render a hex dump of a file.
*/
static void hexdump(Blob *pBlob){
  const unsigned char *x;
  int n, i, j, k;
  char zLine[100];
  static const char zHex[] = "0123456789abcdef";

  x = (const unsigned char*)blob_buffer(pBlob);
  n = blob_size(pBlob);
  for(i=0; i<n; i+=16){
    j = 0;
    zLine[0] = zHex[(i>>24)&0xf];
    zLine[1] = zHex[(i>>16)&0xf];
    zLine[2] = zHex[(i>>8)&0xf];
    zLine[3] = zHex[i&0xf];
    zLine[4] = ':';
    sqlite3_snprintf(sizeof(zLine), zLine, "%04x: ", i);
    for(j=0; j<16; j++){
      k = 5+j*3;
      zLine[k] = ' ';
      if( i+j<n ){
        unsigned char c = x[i+j];
        zLine[k+1] = zHex[c>>4];
        zLine[k+2] = zHex[c&0xf];
      }else{
        zLine[k+1] = ' ';
        zLine[k+2] = ' ';
      }
    }
    zLine[53] = ' ';
    zLine[54] = ' ';
    for(j=0; j<16; j++){
      k = j+55;
      if( i+j<n ){
        unsigned char c = x[i+j];
        if( c>=0x20 && c<=0x7e ){
          zLine[k] = c;
        }else{
          zLine[k] = '.';
        }
      }else{
        zLine[k] = 0;
      }
    }
    zLine[71] = 0;
    cgi_printf("%h\n",(zLine));
  }
}

/*
** WEBPAGE: hexdump
** URL: /hexdump?name=ARTIFACTID
**
** Show the complete content of a file identified by ARTIFACTID
** as preformatted text.
*/
void hexdump_page(void){
  int rid;
  Blob content;
  Blob downloadName;
  char *zUuid;

  rid = name_to_rid_www("name");
  login_check_credentials();
  if( !g.perm.Read ){ login_needed(); return; }
  if( rid==0 ) fossil_redirect_home();
  if( g.perm.Admin ){
    const char *zUuid = db_text("", "SELECT uuid FROM blob WHERE rid=%d", rid);
    if( db_exists("SELECT 1 FROM shun WHERE uuid='%s'", zUuid) ){
      style_submenu_element("Unshun","Unshun", "%s/shun?accept=%s&sub=1#delshun",
            g.zTop, zUuid);
    }else{
      style_submenu_element("Shun","Shun", "%s/shun?shun=%s#addshun",
            g.zTop, zUuid);
    }
  }
  style_header("Hex Artifact Content");
  zUuid = db_text("?","SELECT uuid FROM blob WHERE rid=%d", rid);
  if( g.perm.Setup ){
    cgi_printf("<h2>Artifact %s (%d):</h2>\n",(zUuid),(rid));
  }else{
    cgi_printf("<h2>Artifact %s:</h2>\n",(zUuid));
  }
  blob_zero(&downloadName);
  object_description(rid, 0, &downloadName);
  style_submenu_element("Download", "Download",
        "%s/raw/%T?name=%s", g.zTop, blob_str(&downloadName), zUuid);
  cgi_printf("<hr />\n");
  content_get(rid, &content);
  cgi_printf("<blockquote><pre>\n");
  hexdump(&content);
  cgi_printf("</pre></blockquote>\n");
  style_footer();
}

/*
** Look for "ci" and "filename" query parameters.  If found, try to
** use them to extract the record ID of an artifact for the file.
*/
int artifact_from_ci_and_filename(void){
  const char *zFilename;
  const char *zCI;
  int cirid;
  Manifest *pManifest;
  ManifestFile *pFile;

  zCI = P("ci");
  if( zCI==0 ) return 0;
  zFilename = P("filename");
  if( zFilename==0 ) return 0;
  cirid = name_to_rid_www("ci");
  pManifest = manifest_get(cirid, CFTYPE_MANIFEST, 0);
  if( pManifest==0 ) return 0;
  manifest_file_rewind(pManifest);
  while( (pFile = manifest_file_next(pManifest,0))!=0 ){
    if( fossil_strcmp(zFilename, pFile->zName)==0 ){
      int rid = db_int(0, "SELECT rid FROM blob WHERE uuid=%Q", pFile->zUuid);
      manifest_destroy(pManifest);
      return rid;
    }
  }
  return 0;
}

/*
** The "z" argument is a string that contains the text of a source code
** file.  This routine appends that text to the HTTP reply with line numbering.
**
** zLn is the ?ln= parameter for the HTTP query.  If there is an argument,
** then highlight that line number and scroll to it once the page loads.
** If there are two line numbers, highlight the range of lines.
*/
void output_text_with_line_numbers(
  const char *z,
  const char *zLn
){
  int iStart, iEnd;    /* Start and end of region to highlight */
  int n = 0;           /* Current line number */
  int i;               /* Loop index */
  int iTop = 0;        /* Scroll so that this line is on top of screen. */

  iStart = iEnd = atoi(zLn);
  if( iStart>0 ){
    for(i=0; fossil_isdigit(zLn[i]); i++){}
    if( zLn[i]==',' || zLn[i]=='-' || zLn[i]=='.' ){
      i++;
      while( zLn[i]=='.' ){ i++; }
      iEnd = atoi(&zLn[i]);
    }
    if( iEnd<iStart ) iEnd = iStart;
    iTop = iStart - 15 + (iEnd-iStart)/4;
    if( iTop>iStart - 2 ) iTop = iStart-2;
  }
  cgi_printf("<pre>\n");
  while( z[0] ){
    n++;
    for(i=0; z[i] && z[i]!='\n'; i++){}
    if( n==iTop ) cgi_append_content("<span id=\"topln\">", -1);
    if( n==iStart ){
      cgi_append_content("<div class=\"selectedText\">",-1);
    }
    cgi_printf("%6d  ", n);
    if( i>0 ){
      char *zHtml = htmlize(z, i);
      cgi_append_content(zHtml, -1);
      fossil_free(zHtml);
    }
    if( n==iStart-15 ) cgi_append_content("</span>", -1);
    if( n==iEnd ) cgi_append_content("</div>", -1);
    else cgi_append_content("\n", 1);
    z += i;
    if( z[0]=='\n' ) z++;
  }
  if( n<iEnd ) cgi_printf("</div>");
  cgi_printf("</pre>\n");
  if( iStart ){
    cgi_printf("<script>gebi('topln').scrollIntoView(true);</script>\n");
  }
}


/*
** WEBPAGE: artifact
** URL: /artifact/ARTIFACTID
** URL: /artifact?ci=CHECKIN&filename=PATH
**
** Additional query parameters:
**
**   ln              - show line numbers
**   ln=N            - highlight line number N
**   ln=M-N          - highlight lines M through N inclusive
**
** Show the complete content of a file identified by ARTIFACTID
** as preformatted text.
*/
void artifact_page(void){
  int rid = 0;
  Blob content;
  const char *zMime;
  Blob downloadName;
  int renderAsWiki = 0;
  int renderAsHtml = 0;
  int objType;
  int asText;
  const char *zUuid;

  if( P("ci") && P("filename") ){
    rid = artifact_from_ci_and_filename();
  }
  if( rid==0 ){
    rid = name_to_rid_www("name");
  }

  login_check_credentials();
  if( !g.perm.Read ){ login_needed(); return; }
  if( rid==0 ) fossil_redirect_home();
  if( g.perm.Admin ){
    const char *zUuid = db_text("", "SELECT uuid FROM blob WHERE rid=%d", rid);
    if( db_exists("SELECT 1 FROM shun WHERE uuid='%s'", zUuid) ){
      style_submenu_element("Unshun","Unshun", "%s/shun?accept=%s&sub=1#accshun",
            g.zTop, zUuid);
    }else{
      style_submenu_element("Shun","Shun", "%s/shun?shun=%s#addshun",
            g.zTop, zUuid);
    }
  }
  style_header("Artifact Content");
  zUuid = db_text("?", "SELECT uuid FROM blob WHERE rid=%d", rid);
  if( g.perm.Setup ){
    cgi_printf("<h2>Artifact %s (%d):</h2>\n",(zUuid),(rid));
  }else{
    cgi_printf("<h2>Artifact %s:</h2>\n",(zUuid));
  }
  blob_zero(&downloadName);
  objType = object_description(rid, 0, &downloadName);
  style_submenu_element("Download", "Download",
          "%R/raw/%T?name=%s", blob_str(&downloadName), zUuid);
  if( db_exists("SELECT 1 FROM mlink WHERE fid=%d", rid) ){
    style_submenu_element("Checkins Using", "Checkins Using",
          "%R/timeline?n=200&uf=%s",zUuid);
  }
  asText = P("txt")!=0;
  zMime = mimetype_from_name(blob_str(&downloadName));
  if( zMime ){
    if( fossil_strcmp(zMime, "text/html")==0 ){
      if( asText ){
        style_submenu_element("Html", "Html",
                              "%s/artifact/%s", g.zTop, zUuid);
      }else{
        renderAsHtml = 1;
        style_submenu_element("Text", "Text",
                              "%s/artifact/%s?txt=1", g.zTop, zUuid);
      }
    }else if( fossil_strcmp(zMime, "text/x-fossil-wiki")==0 ){
      if( asText ){
        style_submenu_element("Wiki", "Wiki",
                              "%s/artifact/%s", g.zTop, zUuid);
      }else{
        renderAsWiki = 1;
        style_submenu_element("Text", "Text",
                              "%s/artifact/%s?txt=1", g.zTop, zUuid);
      }
    }
  }
  if( (objType & (OBJTYPE_WIKI|OBJTYPE_TICKET))!=0 ){
    style_submenu_element("Parsed", "Parsed", "%R/info/%s", zUuid);
  }
  cgi_printf("<hr />\n");
  content_get(rid, &content);
  if( renderAsWiki ){
    wiki_convert(&content, 0, 0);
  }else if( renderAsHtml ){
    cgi_printf("<iframe src=\"%R/raw/%T?name=%s\"\n"
           "  width=\"100%%\" frameborder=\"0\" marginwidth=\"0\" marginheight=\"0\"\n"
           "  sandbox=\"allow-same-origin\"\n"
           "  onload=\"this.height = this.contentDocument.documentElement.scrollHeight;\">\n"
           "</iframe>\n",(blob_str(&downloadName)),(zUuid));
  }else{
    style_submenu_element("Hex","Hex", "%s/hexdump?name=%s", g.zTop, zUuid);
    zMime = mimetype_from_content(&content);
    cgi_printf("<blockquote>\n");
    if( zMime==0 ){
      const char *zLn = P("ln");
      const char *z;
      blob_to_utf8_no_bom(&content, 0);
      z = blob_str(&content);
      if( zLn ){
        output_text_with_line_numbers(z, zLn);
      }else{
        cgi_printf("<pre>\n"
               "%h\n"
               "</pre>\n",(z));
      }
    }else if( strncmp(zMime, "image/", 6)==0 ){
      cgi_printf("<img src=\"%R/raw/%S?m=%s\" />\n",(zUuid),(zMime));
      style_submenu_element("Image", "Image",
                            "%R/raw/%S?m=%s", zUuid, zMime);
    }else{
      cgi_printf("<i>(file is %d bytes of binary data)</i>\n",(blob_size(&content)));
    }
    cgi_printf("</blockquote>\n");
  }
  style_footer();
}

/*
** WEBPAGE: tinfo
** URL: /tinfo?name=ARTIFACTID
**
** Show the details of a ticket change control artifact.
*/
void tinfo_page(void){
  int rid;
  char *zDate;
  const char *zUuid;
  char zTktName[UUID_SIZE+1];
  Manifest *pTktChng;
  int modPending;
  const char *zModAction;
  char *zTktTitle;
  login_check_credentials();
  if( !g.perm.RdTkt ){ login_needed(); return; }
  rid = name_to_rid_www("name");
  if( rid==0 ){ fossil_redirect_home(); }
  zUuid = db_text("", "SELECT uuid FROM blob WHERE rid=%d", rid);
  if( g.perm.Admin ){
    if( db_exists("SELECT 1 FROM shun WHERE uuid='%s'", zUuid) ){
      style_submenu_element("Unshun","Unshun", "%s/shun?accept=%s&sub=1#accshun",
            g.zTop, zUuid);
    }else{
      style_submenu_element("Shun","Shun", "%s/shun?shun=%s#addshun",
            g.zTop, zUuid);
    }
  }
  pTktChng = manifest_get(rid, CFTYPE_TICKET, 0);
  if( pTktChng==0 ) fossil_redirect_home();
  zDate = db_text(0, "SELECT datetime(%.12f)", pTktChng->rDate);
  memcpy(zTktName, pTktChng->zTicketUuid, UUID_SIZE+1);
  if( g.perm.ModTkt && (zModAction = P("modaction"))!=0 ){
    if( strcmp(zModAction,"delete")==0 ){
      moderation_disapprove(rid);
      cgi_redirectf("%R/tktview/%s", zTktName);
      /*NOTREACHED*/
    }
    if( strcmp(zModAction,"approve")==0 ){
      moderation_approve(rid);
    }
  }
  zTktTitle = db_table_has_column( "ticket", "title" )
      ? db_text("(No title)", "SELECT title FROM ticket WHERE tkt_uuid=%Q", zTktName)
      : 0;
  style_header("Ticket Change Details");
  style_submenu_element("Raw", "Raw", "%R/artifact/%S", zUuid);
  style_submenu_element("History", "History", "%R/tkthistory/%s", zTktName);
  style_submenu_element("Page", "Page", "%R/tktview/%t", zTktName);
  style_submenu_element("Timeline", "Timeline", "%R/tkttimeline/%t", zTktName);
  if( P("plaintext") ){
    style_submenu_element("Formatted", "Formatted", "%R/info/%S", zUuid);
  }else{
    style_submenu_element("Plaintext", "Plaintext",
                          "%R/info/%S?plaintext", zUuid);
  }

  cgi_printf("<div class=\"section\">Overview</div>\n"
         "<p><table class=\"label-value\">\n"
         "<tr><th>Artifact&nbsp;ID:</th>\n"
         "<td>%z%s</a>\n",(href("%R/artifact/%s",zUuid)),(zUuid));
  if( g.perm.Setup ){
    cgi_printf("(%d)\n",(rid));
  }
  modPending = moderation_pending(rid);
  if( modPending ){
    cgi_printf("<span class=\"modpending\">*** Awaiting Moderator Approval ***</span>\n");
  }
  cgi_printf("<tr><th>Ticket:</th>\n"
         "<td>%z%s</a>\n",(href("%R/tktview/%s",zTktName)),(zTktName));
  if( zTktTitle ){
       cgi_printf("<br>%h\n",(zTktTitle));
  }
 cgi_printf("</td></tr>\n"
         "<tr><th>Date:</th><td>\n");
  hyperlink_to_date(zDate, "</td></tr>");
  cgi_printf("<tr><th>User:</th><td>\n");
  hyperlink_to_user(pTktChng->zUser, zDate, "</td></tr>");
  cgi_printf("</table>\n");
  free(zDate);
  free(zTktTitle);

  if( g.perm.ModTkt && modPending ){
    cgi_printf("<div class=\"section\">Moderation</div>\n"
           "<blockquote>\n"
           "<form method=\"POST\" action=\"%R/tinfo/%s\">\n"
           "<label><input type=\"radio\" name=\"modaction\" value=\"delete\">\n"
           "Delete this change</label><br />\n"
           "<label><input type=\"radio\" name=\"modaction\" value=\"approve\">\n"
           "Approve this change</label><br />\n"
           "<input type=\"submit\" value=\"Submit\">\n"
           "</form>\n"
           "</blockquote>\n",(zUuid));
  }

  cgi_printf("<div class=\"section\">Changes</div>\n"
         "<p>\n");
  ticket_output_change_artifact(pTktChng, 0);
  manifest_destroy(pTktChng);
  style_footer();
}


/*
** WEBPAGE: info
** URL: info/ARTIFACTID
**
** The argument is a artifact ID which might be a baseline or a file or
** a ticket changes or a wiki edit or something else.
**
** Figure out what the artifact ID is and jump to it.
*/
void info_page(void){
  const char *zName;
  Blob uuid;
  int rid;
  int rc;

  zName = P("name");
  if( zName==0 ) fossil_redirect_home();
  if( validate16(zName, strlen(zName)) ){
    if( db_exists("SELECT 1 FROM ticket WHERE tkt_uuid GLOB '%q*'", zName) ){
      tktview_page();
      return;
    }
    if( db_exists("SELECT 1 FROM tag WHERE tagname GLOB 'event-%q*'", zName) ){
      event_page();
      return;
    }
  }
  blob_set(&uuid, zName);
  rc = name_to_uuid(&uuid, -1, "*");
  if( rc==1 ){
    style_header("No Such Object");
    cgi_printf("<p>No such object: %h</p>\n",(zName));
    style_footer();
    return;
  }else if( rc==2 ){
    cgi_set_parameter("src","info");
    ambiguous_page();
    return;
  }
  zName = blob_str(&uuid);
  rid = db_int(0, "SELECT rid FROM blob WHERE uuid='%s'", zName);
  if( rid==0 ){
    style_header("Broken Link");
    cgi_printf("<p>No such object: %h</p>\n",(zName));
    style_footer();
    return;
  }
  if( db_exists("SELECT 1 FROM mlink WHERE mid=%d", rid) ){
    ci_page();
  }else
  if( db_exists("SELECT 1 FROM tagxref JOIN tag USING(tagid)"
                " WHERE rid=%d AND tagname LIKE 'wiki-%%'", rid) ){
    winfo_page();
  }else
  if( db_exists("SELECT 1 FROM tagxref JOIN tag USING(tagid)"
                " WHERE rid=%d AND tagname LIKE 'tkt-%%'", rid) ){
    tinfo_page();
  }else
  if( db_exists("SELECT 1 FROM plink WHERE cid=%d", rid) ){
    ci_page();
  }else
  if( db_exists("SELECT 1 FROM plink WHERE pid=%d", rid) ){
    ci_page();
  }else
  if( db_exists("SELECT 1 FROM attachment WHERE attachid=%d", rid) ){
    ainfo_page();
  }else
  {
    artifact_page();
  }
}

/*
** Generate HTML that will present the user with a selection of
** potential background colors for timeline entries.
*/
void render_color_chooser(
  int fPropagate,             /* Default value for propagation */
  const char *zDefaultColor,  /* The current default color */
  const char *zIdPropagate,   /* ID of form element checkbox.  NULL for none */
  const char *zId,            /* The ID of the form element */
  const char *zIdCustom       /* ID of text box for custom color */
){
  static const struct SampleColors {
     const char *zCName;
     const char *zColor;
  } aColor[] = {
     { "(none)",  "" },
     { "#f2dcdc", 0 },
     { "#bde5d6", 0 },
     { "#a0a0a0", 0 },
     { "#b0b0b0", 0 },
     { "#c0c0c0", 0 },
     { "#d0d0d0", 0 },
     { "#e0e0e0", 0 },

     { "#c0fff0", 0 },
     { "#c0f0ff", 0 },
     { "#d0c0ff", 0 },
     { "#ffc0ff", 0 },
     { "#ffc0d0", 0 },
     { "#fff0c0", 0 },
     { "#f0ffc0", 0 },
     { "#c0ffc0", 0 },

     { "#a8d3c0", 0 },
     { "#a8c7d3", 0 },
     { "#aaa8d3", 0 },
     { "#cba8d3", 0 },
     { "#d3a8bc", 0 },
     { "#d3b5a8", 0 },
     { "#d1d3a8", 0 },
     { "#b1d3a8", 0 },

     { "#8eb2a1", 0 },
     { "#8ea7b2", 0 },
     { "#8f8eb2", 0 },
     { "#ab8eb2", 0 },
     { "#b28e9e", 0 },
     { "#b2988e", 0 },
     { "#b0b28e", 0 },
     { "#95b28e", 0 },

     { "#80d6b0", 0 },
     { "#80bbd6", 0 },
     { "#8680d6", 0 },
     { "#c680d6", 0 },
     { "#d680a6", 0 },
     { "#d69b80", 0 },
     { "#d1d680", 0 },
     { "#91d680", 0 },


     { "custom",  "##" },
  };
  int nColor = sizeof(aColor)/sizeof(aColor[0])-1;
  int stdClrFound = 0;
  int i;

  if( zIdPropagate ){
    cgi_printf("<div><label>\n");
    if( fPropagate ){
      cgi_printf("<input type=\"checkbox\" name=\"%s\" checked=\"checked\" />\n",(zIdPropagate));
    }else{
      cgi_printf("<input type=\"checkbox\" name=\"%s\" />\n",(zIdPropagate));
    }
    cgi_printf("Propagate color to descendants</label></div>\n");
  }
  cgi_printf("<table border=\"0\" cellpadding=\"0\" cellspacing=\"1\" class=\"colorpicker\">\n"
         "<tr>\n");
  for(i=0; i<nColor; i++){
    const char *zClr = aColor[i].zColor;
    if( zClr==0 ) zClr = aColor[i].zCName;
    if( zClr[0] ){
      cgi_printf("<td style=\"background-color: %h;\">\n",(zClr));
    }else{
      cgi_printf("<td>\n");
    }
    cgi_printf("<label>\n");
    if( fossil_strcmp(zDefaultColor, zClr)==0 ){
      cgi_printf("<input type=\"radio\" name=\"%s\" value=\"%h\"\n"
             " checked=\"checked\" />\n",(zId),(zClr));
      stdClrFound=1;
    }else{
      cgi_printf("<input type=\"radio\" name=\"%s\" value=\"%h\" />\n",(zId),(zClr));
    }
    cgi_printf("%h</label></td>\n",(aColor[i].zCName));
    if( (i%8)==7 && i+1<nColor ){
      cgi_printf("</tr><tr>\n");
    }
  }
  cgi_printf("</tr><tr>\n");
  if( stdClrFound ){
    cgi_printf("<td colspan=\"6\"><label>\n"
           "<input type=\"radio\" name=\"%s\" value=\"%h\"\n"
           " onclick=\"gebi('%s').select();\" />\n",(zId),(aColor[nColor].zColor),(zIdCustom));
  }else{
    cgi_printf("<td style=\"background-color: %h;\" colspan=\"6\"><label>\n"
           "<input type=\"radio\" name=\"%s\" value=\"%h\"\n"
           " checked=\"checked\" onclick=\"gebi('%s').select();\" />\n",(zDefaultColor),(zId),(aColor[nColor].zColor),(zIdCustom));
  }
  cgi_printf("%h</label>&nbsp;\n"
         "<input type=\"text\" name=\"%s\"\n"
         " id=\"%s\" class=\"checkinUserColor\"\n"
         " value=\"%h\"\n"
         " onfocus=\"this.form.elements['%s'][%d].checked = true;\" />\n"
         "</td>\n"
         "</tr>\n"
         "</table>\n",(aColor[i].zCName),(zIdCustom),(zIdCustom),(stdClrFound?"":zDefaultColor),(zId),(nColor));
}

/*
** Do a comment comparison.
**
** +  Leading and trailing whitespace are ignored.
** +  \r\n characters compare equal to \n
**
** Return true if equal and false if not equal.
*/
static int comment_compare(const char *zA, const char *zB){
  if( zA==0 ) zA = "";
  if( zB==0 ) zB = "";
  while( fossil_isspace(zA[0]) ) zA++;
  while( fossil_isspace(zB[0]) ) zB++;
  while( zA[0] && zB[0] ){
    if( zA[0]==zB[0] ){ zA++; zB++; continue; }
    if( zA[0]=='\r' && zA[1]=='\n' && zB[0]=='\n' ){
      zA += 2;
      zB++;
      continue;
    }
    if( zB[0]=='\r' && zB[1]=='\n' && zA[0]=='\n' ){
      zB += 2;
      zA++;
      continue;
    }
    return 0;
  }
  while( fossil_isspace(zB[0]) ) zB++;
  while( fossil_isspace(zA[0]) ) zA++;
  return zA[0]==0 && zB[0]==0;
}

/*
** WEBPAGE: ci_edit
** URL:  ci_edit?r=RID&c=NEWCOMMENT&u=NEWUSER
**
** Present a dialog for updating properties of a baseline:
**
**     *  The check-in user
**     *  The check-in comment
**     *  The background color.
*/
void ci_edit_page(void){
  int rid;
  const char *zComment;         /* Current comment on the check-in */
  const char *zNewComment;      /* Revised check-in comment */
  const char *zUser;            /* Current user for the check-in */
  const char *zNewUser;         /* Revised user */
  const char *zDate;            /* Current date of the check-in */
  const char *zNewDate;         /* Revised check-in date */
  const char *zColor;
  const char *zNewColor;
  const char *zNewTagFlag;
  const char *zNewTag;
  const char *zNewBrFlag;
  const char *zNewBranch;
  const char *zCloseFlag;
  const char *zHideFlag;
  int fPropagateColor;          /* True if color propagates before edit */
  int fNewPropagateColor;       /* True if color propagates after edit */
  int fHasHidden = 0;           /* True if hidden tag already set */
  int fHasClosed = 0;           /* True if closed tag already set */
  const char *zChngTime = 0;     /* Value of chngtime= query param, if any */
  char *zUuid;
  Blob comment;
  char *zBranchName = 0;
  Stmt q;

  login_check_credentials();
  if( !g.perm.Write ){ login_needed(); return; }
  rid = name_to_typed_rid(P("r"), "ci");
  zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
  zComment = db_text(0, "SELECT coalesce(ecomment,comment)"
                        "  FROM event WHERE objid=%d", rid);
  if( zComment==0 ) fossil_redirect_home();
  if( P("cancel") ){
    cgi_redirectf("ci?name=%s", zUuid);
  }
  if( g.perm.Setup ) zChngTime = P("chngtime");
  zNewComment = PD("c",zComment);
  zUser = db_text(0, "SELECT coalesce(euser,user)"
                     "  FROM event WHERE objid=%d", rid);
  if( zUser==0 ) fossil_redirect_home();
  zNewUser = PDT("u",zUser);
  zDate = db_text(0, "SELECT datetime(mtime)"
                     "  FROM event WHERE objid=%d", rid);
  if( zDate==0 ) fossil_redirect_home();
  zNewDate = PDT("dt",zDate);
  zColor = db_text("", "SELECT bgcolor"
                        "  FROM event WHERE objid=%d", rid);
  zNewColor = PDT("clr",zColor);
  if( fossil_strcmp(zNewColor,"##")==0 ){
    zNewColor = PT("clrcust");
  }
  fPropagateColor = db_int(0, "SELECT tagtype FROM tagxref"
                              " WHERE rid=%d AND tagid=%d",
                              rid, TAG_BGCOLOR)==2;
  fNewPropagateColor = P("clr")!=0 ? P("pclr")!=0 : fPropagateColor;
  zNewTagFlag = P("newtag") ? " checked" : "";
  zNewTag = PDT("tagname","");
  zNewBrFlag = P("newbr") ? " checked" : "";
  zNewBranch = PDT("brname","");
  zCloseFlag = P("close") ? " checked" : "";
  zHideFlag = P("hide") ? " checked" : "";
  if( P("apply") ){
    Blob ctrl;
    char *zNow;
    int nChng = 0;

    login_verify_csrf_secret();
    blob_zero(&ctrl);
    zNow = date_in_standard_format(zChngTime ? zChngTime : "now");
    blob_appendf(&ctrl, "D %s\n", zNow);
    db_multi_exec("CREATE TEMP TABLE newtags(tag UNIQUE, prefix, value)");
    if( zNewColor[0]
     && (fPropagateColor!=fNewPropagateColor
             || fossil_strcmp(zColor,zNewColor)!=0)
    ){
      char *zPrefix = "+";
      if( fNewPropagateColor ){
        zPrefix = "*";
      }
      db_multi_exec("REPLACE INTO newtags VALUES('bgcolor',%Q,%Q)",
                    zPrefix, zNewColor);
    }
    if( zNewColor[0]==0 && zColor[0]!=0 ){
      db_multi_exec("REPLACE INTO newtags VALUES('bgcolor','-',NULL)");
    }
    if( comment_compare(zComment,zNewComment)==0 ){
      db_multi_exec("REPLACE INTO newtags VALUES('comment','+',%Q)",
                    zNewComment);
    }
    if( fossil_strcmp(zDate,zNewDate)!=0 ){
      db_multi_exec("REPLACE INTO newtags VALUES('date','+',%Q)",
                    zNewDate);
    }
    if( fossil_strcmp(zUser,zNewUser)!=0 ){
      db_multi_exec("REPLACE INTO newtags VALUES('user','+',%Q)", zNewUser);
    }
    db_prepare(&q,
       "SELECT tag.tagid, tagname FROM tagxref, tag"
       " WHERE tagxref.rid=%d AND tagtype>0 AND tagxref.tagid=tag.tagid",
       rid
    );
    while( db_step(&q)==SQLITE_ROW ){
      int tagid = db_column_int(&q, 0);
      const char *zTag = db_column_text(&q, 1);
      char zLabel[30];
      sqlite3_snprintf(sizeof(zLabel), zLabel, "c%d", tagid);
      if( P(zLabel) ){
        db_multi_exec("REPLACE INTO newtags VALUES(%Q,'-',NULL)", zTag);
      }
    }
    db_finalize(&q);
    if( zHideFlag[0] ){
      db_multi_exec("REPLACE INTO newtags VALUES('hidden','*',NULL)");
    }
    if( zCloseFlag[0] ){
      db_multi_exec("REPLACE INTO newtags VALUES('closed','%s',NULL)",
          is_a_leaf(rid)?"+":"*");
    }
    if( zNewTagFlag[0] && zNewTag[0] ){
      db_multi_exec("REPLACE INTO newtags VALUES('sym-%q','+',NULL)", zNewTag);
    }
    if( zNewBrFlag[0] && zNewBranch[0] ){
      db_multi_exec(
        "REPLACE INTO newtags "
        " SELECT tagname, '-', NULL FROM tagxref, tag"
        "  WHERE tagxref.rid=%d AND tagtype==2"
        "    AND tagname GLOB 'sym-*'"
        "    AND tag.tagid=tagxref.tagid",
        rid
      );
      db_multi_exec("REPLACE INTO newtags VALUES('branch','*',%Q)", zNewBranch);
      db_multi_exec("REPLACE INTO newtags VALUES('sym-%q','*',NULL)",
                    zNewBranch);
    }
    db_prepare(&q, "SELECT tag, prefix, value FROM newtags"
                   " ORDER BY prefix || tag");
    while( db_step(&q)==SQLITE_ROW ){
      const char *zTag = db_column_text(&q, 0);
      const char *zPrefix = db_column_text(&q, 1);
      const char *zValue = db_column_text(&q, 2);
      nChng++;
      if( zValue ){
        blob_appendf(&ctrl, "T %s%F %s %F\n", zPrefix, zTag, zUuid, zValue);
      }else{
        blob_appendf(&ctrl, "T %s%F %s\n", zPrefix, zTag, zUuid);
      }
    }
    db_finalize(&q);
    if( nChng>0 ){
      int nrid;
      Blob cksum;
      blob_appendf(&ctrl, "U %F\n", g.zLogin);
      md5sum_blob(&ctrl, &cksum);
      blob_appendf(&ctrl, "Z %b\n", &cksum);
      db_begin_transaction();
      g.markPrivate = content_is_private(rid);
      nrid = content_put(&ctrl);
      manifest_crosslink(nrid, &ctrl, MC_PERMIT_HOOKS);
      assert( blob_is_reset(&ctrl) );
      db_end_transaction(0);
    }
    cgi_redirectf("ci?name=%s", zUuid);
  }
  blob_zero(&comment);
  blob_append(&comment, zNewComment, -1);
  zUuid[10] = 0;
  style_header("Edit Check-in [%s]", zUuid);
  /*
  ** chgcbn/chgbn: Handle change of (checkbox for) branch name in
  ** remaining of form.
  */
  cgi_printf("<script>\n"
         "function chgcbn(checked, branch){\n"
         "  val = gebi('brname').value.trim();\n"
         "  if( !val || !checked ) val = branch;\n"
         "  if( checked ) gebi('brname').select();\n"
         "  gebi('hbranch').textContent = val;\n"
         "  cidbrid = document.getElementById('cbranch');\n"
         "  if( cidbrid ) cidbrid.textContent = val;\n"
         "}\n"
         "function chgbn(val, branch){\n"
         "  if( !val ) val = branch;\n"
         "  gebi('newbr').checked = (val!=branch);\n"
         "  gebi('hbranch').textContent = val;\n"
         "  cidbrid = document.getElementById('cbranch');\n"
         "  if( cidbrid ) cidbrid.textContent = val;\n"
         "}\n"
         "</script>\n");
  if( P("preview") ){
    Blob suffix;
    int nTag = 0;
    cgi_printf("<b>Preview:</b>\n"
           "<blockquote>\n"
           "<table border=0>\n");
    if( zNewColor && zNewColor[0] ){
      cgi_printf("<tr><td style=\"background-color: %h;\">\n",(zNewColor));
    }else{
      cgi_printf("<tr><td>\n");
    }
    cgi_printf("%!w\n",(blob_str(&comment)));
    blob_zero(&suffix);
    blob_appendf(&suffix, "(user: %h", zNewUser);
    db_prepare(&q, "SELECT substr(tagname,5) FROM tagxref, tag"
                   " WHERE tagname GLOB 'sym-*' AND tagxref.rid=%d"
                   "   AND tagtype>1 AND tag.tagid=tagxref.tagid",
                   rid);
    while( db_step(&q)==SQLITE_ROW ){
      const char *zTag = db_column_text(&q, 0);
      if( nTag==0 ){
        blob_appendf(&suffix, ", tags: %h", zTag);
      }else{
        blob_appendf(&suffix, ", %h", zTag);
      }
      nTag++;
    }
    db_finalize(&q);
    blob_appendf(&suffix, ")");
    cgi_printf("%s\n"
           "</td></tr></table>\n",(blob_str(&suffix)));
    if( zChngTime ){
      cgi_printf("<p>The timestamp on the tag used to make the changes above\n"
             "will be overridden as: %s</p>\n",(date_in_standard_format(zChngTime)));
    }
    cgi_printf("</blockquote>\n"
           "<hr />\n");
    blob_reset(&suffix);
  }
  cgi_printf("<p>Make changes to attributes of check-in\n"
         "[%z%s</a>]:</p>\n",(href("%R/ci/%s",zUuid)),(zUuid));
  form_begin(0, "%R/ci_edit");
  login_insert_csrf_secret();
  cgi_printf("<div><input type=\"hidden\" name=\"r\" value=\"%S\" />\n"
         "<table border=\"0\" cellspacing=\"10\">\n",(zUuid));

  cgi_printf("<tr><th align=\"right\" valign=\"top\">User:</th>\n"
         "<td valign=\"top\">\n"
         "  <input type=\"text\" name=\"u\" size=\"20\" value=\"%h\" />\n"
         "</td></tr>\n",(zNewUser));

  cgi_printf("<tr><th align=\"right\" valign=\"top\">Comment:</th>\n"
         "<td valign=\"top\">\n"
         "<textarea name=\"c\" rows=\"10\" cols=\"80\">%h</textarea>\n"
         "</td></tr>\n",(zNewComment));

  cgi_printf("<tr><th align=\"right\" valign=\"top\">Check-in Time:</th>\n"
         "<td valign=\"top\">\n"
         "  <input type=\"text\" name=\"dt\" size=\"20\" value=\"%h\" />\n"
         "</td></tr>\n",(zNewDate));

  if( zChngTime ){
    cgi_printf("<tr><th align=\"right\" valign=\"top\">Timestamp of this change:</th>\n"
           "<td valign=\"top\">\n"
           "  <input type=\"text\" name=\"chngtime\" size=\"20\" value=\"%h\" />\n"
           "</td></tr>\n",(zChngTime));
  }

  cgi_printf("<tr><th align=\"right\" valign=\"top\">Background Color:</th>\n"
         "<td valign=\"top\">\n");
  render_color_chooser(fNewPropagateColor, zNewColor, "pclr", "clr", "clrcust");
  cgi_printf("</td></tr>\n");

  cgi_printf("<tr><th align=\"right\" valign=\"top\">Tags:</th>\n"
         "<td valign=\"top\">\n"
         "<label><input type=\"checkbox\" id=\"newtag\" name=\"newtag\"%s />\n"
         "Add the following new tag name to this check-in:</label>\n"
         "<input type=\"text\" style=\"width:15;\" name=\"tagname\" value=\"%h\"\n"
         "onkeyup=\"gebi('newtag').checked=!!this.value\" />\n",(zNewTagFlag),(zNewTag));
  zBranchName = db_text(0, "SELECT value FROM tagxref, tag"
     " WHERE tagxref.rid=%d AND tagtype>0 AND tagxref.tagid=tag.tagid"
     " AND tagxref.tagid=%d", rid, TAG_BRANCH);
  db_prepare(&q,
     "SELECT tag.tagid, tagname, tagxref.value FROM tagxref, tag"
     " WHERE tagxref.rid=%d AND tagtype>0 AND tagxref.tagid=tag.tagid"
     " ORDER BY CASE WHEN tagname GLOB 'sym-*' THEN substr(tagname,5)"
     "               ELSE tagname END /*sort*/",
     rid
  );
  while( db_step(&q)==SQLITE_ROW ){
    int tagid = db_column_int(&q, 0);
    const char *zTagName = db_column_text(&q, 1);
    int isSpecialTag = fossil_strncmp(zTagName, "sym-", 4)!=0;
    char zLabel[30];

    if( tagid == TAG_CLOSED ){
      fHasClosed = 1;
    }else if( (tagid == TAG_COMMENT) || (tagid == TAG_BRANCH) ){
      continue;
    }else if( tagid==TAG_HIDDEN ){
      fHasHidden = 1;
    }else if( !isSpecialTag && zTagName &&
        fossil_strcmp(&zTagName[4], zBranchName)==0){
      continue;
    }
    sqlite3_snprintf(sizeof(zLabel), zLabel, "c%d", tagid);
    cgi_printf("<br /><label>\n");
    if( P(zLabel) ){
      cgi_printf("<input type=\"checkbox\" name=\"c%d\" checked=\"checked\" />\n",(tagid));
    }else{
      cgi_printf("<input type=\"checkbox\" name=\"c%d\" />\n",(tagid));
    }
    if( isSpecialTag ){
      cgi_printf("Cancel special tag <b>%h</b></label>\n",(zTagName));
    }else{
      cgi_printf("Cancel tag <b>%h</b></label>\n",(&zTagName[4]));
    }
  }
  db_finalize(&q);
  cgi_printf("</td></tr>\n");

  if( !zBranchName ){
    zBranchName = db_get("main-branch", "trunk");
  }
  if( !zNewBranch || !zNewBranch[0]){
    zNewBranch = zBranchName;
  }
  cgi_printf("<tr><th align=\"right\" valign=\"top\">Branching:</th>\n"
         "<td valign=\"top\">\n"
         "<label><input id=\"newbr\" type=\"checkbox\" name=\"newbr\"%s\n"
         "onchange=\"chgcbn(this.checked,'%h')\" />\n"
         "Make this check-in the start of a new branch named:</label>\n"
         "<input id=\"brname\" type=\"text\" style=\"width:15;\" name=\"brname\"\n"
         "value=\"%h\"\n"
         "onkeyup=\"chgbn(this.value.trim(),'%h')\" /></td></tr>\n",(zNewBrFlag),(zBranchName),(zNewBranch),(zBranchName));
  if( !fHasHidden ){
    cgi_printf("<tr><th align=\"right\" valign=\"top\">Branch Hiding:</th>\n"
           "<td valign=\"top\">\n"
           "<label><input type=\"checkbox\" id=\"hidebr\" name=\"hide\"%s />\n"
           "Hide branch \n"
           "<span style=\"font-weight:bold\" id=\"hbranch\">%h</span>\n"
           "from the timeline starting from this check-in</label>\n"
           "</td></tr>\n",(zHideFlag),(zBranchName));
  }
  if( !fHasClosed ){
    if( is_a_leaf(rid) ){
      cgi_printf("<tr><th align=\"right\" valign=\"top\">Leaf Closure:</th>\n"
             "<td valign=\"top\">\n"
             "<label><input type=\"checkbox\" name=\"close\"%s />\n"
             "Mark this leaf as \"closed\" so that it no longer appears on the\n"
             "\"leaves\" page and is no longer labeled as a \"<b>Leaf</b>\"</label>\n"
             "</td></tr>\n",(zCloseFlag));
    }else if( zBranchName ){
      cgi_printf("<tr><th align=\"right\" valign=\"top\">Branch Closure:</th>\n"
             "<td valign=\"top\">\n"
             "<label><input type=\"checkbox\" name=\"close\"%s />\n"
             "Mark branch\n"
             "<span style=\"font-weight:bold\" id=\"cbranch\">%h</span>\n"
             "as \"closed\" so that its leafs no longer appear on the \"leaves\" page\n"
             "and are no longer labeled as a leaf \"<b>Leaf</b>\"</label>\n"
             "</td></tr>\n",(zCloseFlag),(zBranchName));
    }
  }
  if( zBranchName ) fossil_free(zBranchName);


  cgi_printf("<tr><td colspan=\"2\">\n"
         "<input type=\"submit\" name=\"preview\" value=\"Preview\" />\n"
         "<input type=\"submit\" name=\"apply\" value=\"Apply Changes\" />\n"
         "<input type=\"submit\" name=\"cancel\" value=\"Cancel\" />\n"
         "</td></tr>\n"
         "</table>\n"
         "</div></form>\n");
  style_footer();
}
