/*
** Copyright (c) 2010 D. Richard Hipp
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
** This file contains code for dealing with attachments.
*/
#include "config.h"
#include "attach.h"
#include <assert.h>

/*
** WEBPAGE: attachlist
**
**    tkt=TICKETUUID
**    page=WIKIPAGE
**
** List attachments.
** Either one of tkt= or page= are supplied or neither.  If neither
** are given, all attachments are listed.  If one is given, only
** attachments for the designated ticket or wiki page are shown.
** TICKETUUID must be complete 
*/
void attachlist_page(void){
  const char *zPage = P("page");
  const char *zTkt = P("tkt");
  Blob sql;
  Stmt q;

  if( zPage && zTkt ) zTkt = 0;
  login_check_credentials();
  blob_zero(&sql);
  blob_appendf(&sql,
     "SELECT datetime(mtime%s), src, target, filename,"
     "       comment, user,"
     "       (SELECT uuid FROM blob WHERE rid=attachid), attachid"
     "  FROM attachment",
     timeline_utc()
  );
  if( zPage ){
    if( g.perm.RdWiki==0 ) login_needed();
    style_header("Attachments To %h", zPage);
    blob_appendf(&sql, " WHERE target=%Q", zPage);
  }else if( zTkt ){
    if( g.perm.RdTkt==0 ) login_needed();
    style_header("Attachments To Ticket %.10s", zTkt);
    blob_appendf(&sql, " WHERE target GLOB '%q*'", zTkt);
  }else{
    if( g.perm.RdTkt==0 && g.perm.RdWiki==0 ) login_needed();
    style_header("All Attachments");
  }
  blob_appendf(&sql, " ORDER BY mtime DESC");
  db_prepare(&q, "%s", blob_str(&sql));
  @ <ol>
  while( db_step(&q)==SQLITE_ROW ){
    const char *zDate = db_column_text(&q, 0);
    const char *zSrc = db_column_text(&q, 1);
    const char *zTarget = db_column_text(&q, 2);
    const char *zFilename = db_column_text(&q, 3);
    const char *zComment = db_column_text(&q, 4);
    const char *zUser = db_column_text(&q, 5);
    const char *zUuid = db_column_text(&q, 6);
    int attachid = db_column_int(&q, 7);
    const char *zDispUser = zUser && zUser[0] ? zUser : "anonymous";
    int i;
    char *zUrlTail;
    for(i=0; zFilename[i]; i++){
      if( zFilename[i]=='/' && zFilename[i+1]!=0 ){ 
        zFilename = &zFilename[i+1];
        i = -1;
      }
    }
    if( strlen(zTarget)==UUID_SIZE && validate16(zTarget,UUID_SIZE) ){
      zUrlTail = mprintf("tkt=%s&file=%t", zTarget, zFilename);
    }else{
      zUrlTail = mprintf("page=%t&file=%t", zTarget, zFilename);
    }
    @ <li><p>
    @ Attachment %z(href("%R/ainfo/%s",zUuid))%S(zUuid)</a>
    if( moderation_pending(attachid) ){
      @ <span class="modpending">*** Awaiting Moderator Approval ***</span>
    }
    @ <br><a href="/attachview?%s(zUrlTail)">%h(zFilename)</a>
    @ [<a href="/attachdownload/%t(zFilename)?%s(zUrlTail)">download</a>]<br />
    if( zComment ) while( fossil_isspace(zComment[0]) ) zComment++;
    if( zComment && zComment[0] ){
      @ %!w(zComment)<br />
    }
    if( zPage==0 && zTkt==0 ){
      if( zSrc==0 || zSrc[0]==0 ){
        zSrc = "Deleted from";
      }else {
        zSrc = "Added to";
      }
      if( strlen(zTarget)==UUID_SIZE && validate16(zTarget, UUID_SIZE) ){
        @ %s(zSrc) ticket <a href="%s(g.zTop)/tktview?name=%s(zTarget)">
        @ %S(zTarget)</a>
      }else{
        @ %s(zSrc) wiki page <a href="%s(g.zTop)/wiki?name=%t(zTarget)">
        @ %h(zTarget)</a>
      }
    }else{
      if( zSrc==0 || zSrc[0]==0 ){
        @ Deleted
      }else {
        @ Added
      }
    }
    @ by %h(zDispUser) on
    hyperlink_to_date(zDate, ".");
    free(zUrlTail);
  }
  db_finalize(&q);
  @ </ol>
  style_footer();
  return;
}

/*
** WEBPAGE: attachdownload
** WEBPAGE: attachimage
** WEBPAGE: attachview
**
**    tkt=TICKETUUID
**    page=WIKIPAGE
**    file=FILENAME
**    attachid=ID
**
** List attachments.
*/
void attachview_page(void){
  const char *zPage = P("page");
  const char *zTkt = P("tkt");
  const char *zFile = P("file");
  const char *zTarget = 0;
  int attachid = atoi(PD("attachid","0"));
  char *zUUID;

  if( zPage && zTkt ) zTkt = 0;
  if( zFile==0 ) fossil_redirect_home();
  login_check_credentials();
  if( zPage ){
    if( g.perm.RdWiki==0 ) login_needed();
    zTarget = zPage;
  }else if( zTkt ){
    if( g.perm.RdTkt==0 ) login_needed();
    zTarget = zTkt;
  }else{
    fossil_redirect_home();
  }
  if( attachid>0 ){
    zUUID = db_text(0,
       "SELECT coalesce(src,'x') FROM attachment"
       " WHERE target=%Q AND attachid=%d",
       zTarget, attachid
    );
  }else{
    zUUID = db_text(0,
       "SELECT coalesce(src,'x') FROM attachment"
       " WHERE target=%Q AND filename=%Q"
       " ORDER BY mtime DESC LIMIT 1",
       zTarget, zFile
    );
  }
  if( zUUID==0 || zUUID[0]==0 ){
    style_header("No Such Attachment");
    @ No such attachment....
    style_footer();
    return;
  }else if( zUUID[0]=='x' ){
    style_header("Missing");
    @ Attachment has been deleted
    style_footer();
    return;
  }
  g.perm.Read = 1;
  cgi_replace_parameter("name",zUUID);
  if( fossil_strcmp(g.zPath,"attachview")==0 ){
    artifact_page();
  }else{
    cgi_replace_parameter("m", mimetype_from_name(zFile));
    rawartifact_page();
  }
}

/*
** Save an attachment control artifact into the repository
*/
static void attach_put(
  Blob *pAttach,     /* Text of the Attachment record */
  int attachRid,     /* RID for the file that is being attached */
  int needMod        /* True if the attachment is subject to moderation */
){
  int rid;
  if( needMod ){
    rid = content_put_ex(pAttach, 0, 0, 0, 1);
    moderation_table_create();
    db_multi_exec(
      "INSERT INTO modreq(objid,attachRid) VALUES(%d,%d);",
      rid, attachRid
    );
  }else{
    rid = content_put(pAttach);
    db_multi_exec("INSERT OR IGNORE INTO unsent VALUES(%d);", rid);
    db_multi_exec("INSERT OR IGNORE INTO unclustered VALUES(%d);", rid);
  }
  manifest_crosslink(rid, pAttach, MC_NONE);
}


/*
** WEBPAGE: attachadd
**
**    tkt=TICKETUUID
**    page=WIKIPAGE
**    from=URL
**
** Add a new attachment.
*/
void attachadd_page(void){
  const char *zPage = P("page");
  const char *zTkt = P("tkt");
  const char *zFrom = P("from");
  const char *aContent = P("f");
  const char *zName = PD("f:filename","unknown");
  const char *zTarget;
  const char *zTargetType;
  int szContent = atoi(PD("f:bytes","0"));
  int goodCaptcha = 1;

  if( P("cancel") ) cgi_redirect(zFrom);
  if( zPage && zTkt ) fossil_redirect_home();
  if( zPage==0 && zTkt==0 ) fossil_redirect_home();
  login_check_credentials();
  if( zPage ){
    if( g.perm.ApndWiki==0 || g.perm.Attach==0 ) login_needed();
    if( !db_exists("SELECT 1 FROM tag WHERE tagname='wiki-%q'", zPage) ){
      fossil_redirect_home();
    }
    zTarget = zPage;
    zTargetType = mprintf("Wiki Page <a href=\"%s/wiki?name=%h\">%h</a>",
                           g.zTop, zPage, zPage);
  }else{
    if( g.perm.ApndTkt==0 || g.perm.Attach==0 ) login_needed();
    if( !db_exists("SELECT 1 FROM tag WHERE tagname='tkt-%q'", zTkt) ){
      zTkt = db_text(0, "SELECT substr(tagname,5) FROM tag" 
                        " WHERE tagname GLOB 'tkt-%q*'", zTkt);
      if( zTkt==0 ) fossil_redirect_home();
    }
    zTarget = zTkt;
    zTargetType = mprintf("Ticket <a href=\"%s/tktview/%S\">%S</a>",
                          g.zTop, zTkt, zTkt);
  }
  if( zFrom==0 ) zFrom = mprintf("%s/home", g.zTop);
  if( P("cancel") ){
    cgi_redirect(zFrom);
  }
  if( P("ok") && szContent>0 && (goodCaptcha = captcha_is_correct()) ){
    Blob content;
    Blob manifest;
    Blob cksum;
    char *zUUID;
    const char *zComment;
    char *zDate;
    int rid;
    int i, n;
    int addCompress = 0;
    Manifest *pManifest;
    int needModerator;

    db_begin_transaction();
    blob_init(&content, aContent, szContent);
    pManifest = manifest_parse(&content, 0, 0);
    manifest_destroy(pManifest);
    blob_init(&content, aContent, szContent);
    if( pManifest ){
      blob_compress(&content, &content);
      addCompress = 1;
    }
    needModerator =
         (zTkt!=0 && g.perm.ModTkt==0 && db_get_boolean("modreq-tkt",0)==1) ||
         (zPage!=0 && g.perm.ModWiki==0 && db_get_boolean("modreq-wiki",0)==1);
    rid = content_put_ex(&content, 0, 0, 0, needModerator);
    zUUID = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
    blob_zero(&manifest);
    for(i=n=0; zName[i]; i++){
      if( zName[i]=='/' || zName[i]=='\\' ) n = i;
    }
    zName += n;
    if( zName[0]==0 ) zName = "unknown";
    blob_appendf(&manifest, "A %F%s %F %s\n",
                 zName, addCompress ? ".gz" : "", zTarget, zUUID);
    zComment = PD("comment", "");
    while( fossil_isspace(zComment[0]) ) zComment++;
    n = strlen(zComment);
    while( n>0 && fossil_isspace(zComment[n-1]) ){ n--; }
    if( n>0 ){
      blob_appendf(&manifest, "C %#F\n", n, zComment);
    }
    zDate = date_in_standard_format("now");
    blob_appendf(&manifest, "D %s\n", zDate);
    blob_appendf(&manifest, "U %F\n", g.zLogin ? g.zLogin : "nobody");
    md5sum_blob(&manifest, &cksum);
    blob_appendf(&manifest, "Z %b\n", &cksum);
    attach_put(&manifest, rid, needModerator);
    assert( blob_is_reset(&manifest) );
    db_end_transaction(0);
    cgi_redirect(zFrom);
  }
  style_header("Add Attachment");
  if( !goodCaptcha ){
    @ <p class="generalError">Error: Incorrect security code.</p>
  }
  @ <h2>Add Attachment To %s(zTargetType)</h2>
  form_begin("enctype='multipart/form-data'", "%R/attachadd");
  @ <div>
  @ File to Attach:
  @ <input type="file" name="f" size="60" /><br />
  @ Description:<br />
  @ <textarea name="comment" cols="80" rows="5" wrap="virtual"></textarea><br />
  if( zTkt ){
    @ <input type="hidden" name="tkt" value="%h(zTkt)" />
  }else{
    @ <input type="hidden" name="page" value="%h(zPage)" />
  }
  @ <input type="hidden" name="from" value="%h(zFrom)" />
  @ <input type="submit" name="ok" value="Add Attachment" />
  @ <input type="submit" name="cancel" value="Cancel" />
  @ </div>
  captcha_generate(0);
  @ </form>
  style_footer();
}

/*
** WEBPAGE: ainfo
** URL: /ainfo?name=ARTIFACTID
**
** Show the details of an attachment artifact.
*/
void ainfo_page(void){
  int rid;                       /* RID for the control artifact */
  int ridSrc;                    /* RID for the attached file */
  char *zDate;                   /* Date attached */
  const char *zUuid;             /* UUID of the control artifact */
  Manifest *pAttach;             /* Parse of the control artifact */
  const char *zTarget;           /* Wiki or ticket attached to */
  const char *zSrc;              /* UUID of the attached file */
  const char *zName;             /* Name of the attached file */
  const char *zDesc;             /* Description of the attached file */
  const char *zWikiName = 0;     /* Wiki page name when attached to Wiki */
  const char *zTktUuid = 0;      /* Ticket ID when attached to a ticket */
  int modPending;                /* True if awaiting moderation */
  const char *zModAction;        /* Moderation action or NULL */
  int isModerator;               /* TRUE if user is the moderator */
  const char *zMime;             /* MIME Type */
  Blob attach;                   /* Content of the attachment */

  login_check_credentials();
  if( !g.perm.RdTkt && !g.perm.RdWiki ){ login_needed(); return; }
  rid = name_to_rid_www("name");
  if( rid==0 ){ fossil_redirect_home(); }
  zUuid = db_text("", "SELECT uuid FROM blob WHERE rid=%d", rid);
#if 0
  /* Shunning here needs to get both the attachment control artifact and
  ** the object that is attached. */
  if( g.perm.Admin ){
    if( db_exists("SELECT 1 FROM shun WHERE uuid='%s'", zUuid) ){
      style_submenu_element("Unshun","Unshun", "%s/shun?uuid=%s&sub=1",
            g.zTop, zUuid);
    }else{
      style_submenu_element("Shun","Shun", "%s/shun?shun=%s#addshun",
            g.zTop, zUuid);
    }
  }
#endif
  pAttach = manifest_get(rid, CFTYPE_ATTACHMENT, 0);
  if( pAttach==0 ) fossil_redirect_home();
  zTarget = pAttach->zAttachTarget;
  zSrc = pAttach->zAttachSrc;
  ridSrc = db_int(0,"SELECT rid FROM blob WHERE uuid='%s'", zSrc);
  zName = pAttach->zAttachName;
  zDesc = pAttach->zComment;
  if( validate16(zTarget, strlen(zTarget))
   && db_exists("SELECT 1 FROM ticket WHERE tkt_uuid='%s'", zTarget)
  ){
    zTktUuid = zTarget;
    if( !g.perm.RdTkt ){ login_needed(); return; }
    if( g.perm.WrTkt ){
      style_submenu_element("Delete","Delete","%R/ainfo/%s?del", zUuid);
    }
  }else if( db_exists("SELECT 1 FROM tag WHERE tagname='wiki-%q'",zTarget) ){
    zWikiName = zTarget;
    if( !g.perm.RdWiki ){ login_needed(); return; }
    if( g.perm.WrWiki ){
      style_submenu_element("Delete","Delete","%R/ainfo/%s?del", zUuid);
    }
  }
  zDate = db_text(0, "SELECT datetime(%.12f)", pAttach->rDate);

  if( P("confirm")
   && ((zTktUuid && g.perm.WrTkt) || (zWikiName && g.perm.WrWiki))
  ){
    int i, n, rid;
    char *zDate;
    Blob manifest;
    Blob cksum;
    const char *zFile = zName;

    db_begin_transaction();
    blob_zero(&manifest);
    for(i=n=0; zFile[i]; i++){
      if( zFile[i]=='/' || zFile[i]=='\\' ) n = i;
    }
    zFile += n;
    if( zFile[0]==0 ) zFile = "unknown";
    blob_appendf(&manifest, "A %F %F\n", zFile, zTarget);
    zDate = date_in_standard_format("now");
    blob_appendf(&manifest, "D %s\n", zDate);
    blob_appendf(&manifest, "U %F\n", g.zLogin ? g.zLogin : "nobody");
    md5sum_blob(&manifest, &cksum);
    blob_appendf(&manifest, "Z %b\n", &cksum);
    rid = content_put(&manifest);
    manifest_crosslink(rid, &manifest, MC_NONE);
    db_end_transaction(0);
    @ <p>The attachment below has been deleted.</p>
  }

  if( P("del")
   && ((zTktUuid && g.perm.WrTkt) || (zWikiName && g.perm.WrWiki))
  ){
    form_begin(0, "%R/ainfo/%s", zUuid);
    @ <p>Confirm you want to delete the attachment shown below.
    @ <input type="submit" name="confirm" value="Confirm">
    @ </form>
  }

  isModerator = g.perm.Admin || 
                (zTktUuid && g.perm.ModTkt) ||
                (zWikiName && g.perm.ModWiki);
  if( isModerator && (zModAction = P("modaction"))!=0 ){
    if( strcmp(zModAction,"delete")==0 ){
      moderation_disapprove(rid);
      if( zTktUuid ){
        cgi_redirectf("%R/tktview/%s", zTktUuid);
      }else{
        cgi_redirectf("%R/wiki?name=%t", zWikiName);
      }
      return;
    }
    if( strcmp(zModAction,"approve")==0 ){
      moderation_approve(rid);
    }
  }
  style_header("Attachment Details");
  style_submenu_element("Raw", "Raw", "%R/artifact/%S", zUuid);

  @ <div class="section">Overview</div>
  @ <p><table class="label-value">
  @ <tr><th>Artifact&nbsp;ID:</th>
  @ <td>%z(href("%R/artifact/%s",zUuid))%s(zUuid)</a>
  if( g.perm.Setup ){
    @ (%d(rid))
  }
  modPending = moderation_pending(rid);
  if( modPending ){
    @ <span class="modpending">*** Awaiting Moderator Approval ***</span>
  }
  if( zTktUuid ){
    @ <tr><th>Ticket:</th>
    @ <td>%z(href("%R/tktview/%s",zTktUuid))%s(zTktUuid)</a></td></tr>
  }
  if( zWikiName ){
    @ <tr><th>Wiki&nbsp;Page:</th>
    @ <td>%z(href("%R/wiki?name=%t",zWikiName))%h(zWikiName)</a></td></tr>
  }
  @ <tr><th>Date:</th><td>
  hyperlink_to_date(zDate, "</td></tr>");
  @ <tr><th>User:</th><td>
  hyperlink_to_user(pAttach->zUser, zDate, "</td></tr>");
  @ <tr><th>Artifact&nbsp;Attached:</th>
  @ <td>%z(href("%R/artifact/%s",zSrc))%s(zSrc)</a>
  if( g.perm.Setup ){
    @ (%d(ridSrc))
  }
  @ <tr><th>Filename:</th><td>%h(zName)</td></tr>
  zMime = mimetype_from_name(zName);
  if( g.perm.Setup ){
    @ <tr><th>MIME-Type:</th><td>%h(zMime)</td></tr>
  }
  @ <tr><th valign="top">Description:</th><td valign="top">%h(zDesc)</td></tr>
  @ </table>
  
  if( isModerator && modPending ){
    @ <div class="section">Moderation</div>
    @ <blockquote>
    form_begin(0, "%R/ainfo/%s", zUuid);
    @ <label><input type="radio" name="modaction" value="delete">
    @ Delete this change</label><br />
    @ <label><input type="radio" name="modaction" value="approve">
    @ Approve this change</label><br />
    @ <input type="submit" value="Submit">
    @ </form>
    @ </blockquote>
  }

  @ <div class="section">Content Appended</div>
  @ <blockquote>
  blob_zero(&attach);
  if( zMime==0 || strncmp(zMime,"text/", 5)==0 ){
    const char *z;
    const char *zLn = P("ln");
    content_get(ridSrc, &attach);
    blob_to_utf8_no_bom(&attach, 0);
    z = blob_str(&attach);
    if( zLn ){
      output_text_with_line_numbers(z, zLn);
    }else{
      @ <pre>
      @ %h(z)
      @ </pre>
    }
  }else if( strncmp(zMime, "image/", 6)==0 ){
    @ <img src="%R/raw/%S(zSrc)?m=%s(zMime)"></img>
    style_submenu_element("Image", "Image", "%R/raw/%S?m=%s", zSrc, zMime);
  }else{
    int sz = db_int(0, "SELECT size FROM blob WHERE rid=%d", ridSrc);
    @ <i>(file is %d(sz) bytes of binary data)</i>
  }
  @ </blockquote>
  manifest_destroy(pAttach);
  blob_reset(&attach);
  style_footer();
}

/*
** Output HTML to show a list of attachments.
*/
void attachment_list(
  const char *zTarget,   /* Object that things are attached to */
  const char *zHeader    /* Header to display with attachments */
){
  int cnt = 0;
  Stmt q;
  db_prepare(&q,
     "SELECT datetime(mtime%s), filename, user,"
     "       (SELECT uuid FROM blob WHERE rid=attachid), src"
     "  FROM attachment"
     " WHERE isLatest AND src!='' AND target=%Q"
     " ORDER BY mtime DESC", 
     timeline_utc(), zTarget
  );
  while( db_step(&q)==SQLITE_ROW ){
    const char *zDate = db_column_text(&q, 0);
    const char *zFile = db_column_text(&q, 1);
    const char *zUser = db_column_text(&q, 2);
    const char *zUuid = db_column_text(&q, 3);
    const char *zSrc = db_column_text(&q, 4);
    const char *zDispUser = zUser && zUser[0] ? zUser : "anonymous";
    if( cnt==0 ){
      @ %s(zHeader)
    }
    cnt++;
    @ <li>
    @ %z(href("%R/artifact/%s",zSrc))%h(zFile)</a>
    @ added by %h(zDispUser) on
    hyperlink_to_date(zDate, ".");
    @ [%z(href("%R/ainfo/%s",zUuid))details</a>]
    @ </li>
  }
  if( cnt ){
    @ </ul>
  }
  db_finalize(&q);
  
}
