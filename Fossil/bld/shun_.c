#line 1 "./src/shun.c"
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
** This file contains code used to manage SHUN table of the repository
*/
#include "config.h"
#include "shun.h"
#include <assert.h>

/*
** Return true if the given artifact ID should be shunned.
*/
int uuid_is_shunned(const char *zUuid){
  static Stmt q;
  int rc;
  if( zUuid==0 || zUuid[0]==0 ) return 0;
  db_static_prepare(&q, "SELECT 1 FROM shun WHERE uuid=:uuid");
  db_bind_text(&q, ":uuid", zUuid);
  rc = db_step(&q);
  db_reset(&q);
  return rc==SQLITE_ROW;
}

/*
** WEBPAGE: shun
*/
void shun_page(void){
  Stmt q;
  int cnt = 0;
  const char *zUuid = P("uuid");
  int nUuid;
  char zCanonical[UUID_SIZE+1];

  login_check_credentials();
  if( !g.perm.Admin ){
    login_needed();
  }
  if( P("rebuild") ){
    db_close(1);
    db_open_repository(g.zRepositoryName);
    db_begin_transaction();
    rebuild_db(0, 0, 0);
    db_end_transaction(0);
  }
  if( zUuid ){
    nUuid = strlen(zUuid);
    if( nUuid!=40 || !validate16(zUuid, nUuid) ){
      zUuid = 0;
    }else{
      memcpy(zCanonical, zUuid, UUID_SIZE+1);
      canonical16(zCanonical, UUID_SIZE);
      zUuid = zCanonical;
    }
  }
  style_header("Shunned Artifacts");
  if( zUuid && P("sub") ){
    login_verify_csrf_secret();
    db_multi_exec("DELETE FROM shun WHERE uuid='%s'", zUuid);
    if( db_exists("SELECT 1 FROM blob WHERE uuid='%s'", zUuid) ){
      cgi_printf("<p class=\"noMoreShun\">Artifact \n"
             "<a href=\"%s/artifact/%s\">%s</a> is no\n"
             "longer being shunned.</p>\n",(g.zTop),(zUuid),(zUuid));
    }else{
      cgi_printf("<p class=\"noMoreShun\">Artifact %s will no longer\n"
             "be shunned.  But it does not exist in the repository.  It\n"
             "may be necessary to rebuild the repository using the\n"
             "<b>fossil rebuild</b> command-line before the artifact content\n"
             "can pulled in from other repositories.</p>\n",(zUuid));
    }
  }
  if( zUuid && P("add") ){
    int rid, tagid;
    login_verify_csrf_secret();
    db_multi_exec(
      "INSERT OR IGNORE INTO shun(uuid,mtime)"
      " VALUES('%s', now())", zUuid);
    cgi_printf("<p class=\"shunned\">Artifact\n"
           "<a href=\"%s/artifact/%s\">%s</a> has been\n"
           "shunned.  It will no longer be pushed.\n"
           "It will be removed from the repository the next time the repository\n"
           "is rebuilt using the <b>fossil rebuild</b> command-line</p>\n",(g.zTop),(zUuid),(zUuid));
    db_multi_exec("DELETE FROM attachment WHERE src=%Q", zUuid);
    rid = db_int(0, "SELECT rid FROM blob WHERE uuid=%Q", zUuid);
    if( rid ){
      db_multi_exec("DELETE FROM event WHERE objid=%d", rid);
    }
    tagid = db_int(0, "SELECT tagid FROM tag WHERE tagname='tkt-%q'", zUuid);
    if( tagid ){
      db_multi_exec("DELETE FROM ticket WHERE tkt_uuid=%Q", zUuid);
      db_multi_exec("DELETE FROM tag WHERE tagid=%d", tagid);
      db_multi_exec("DELETE FROM tagxref WHERE tagid=%d", tagid);
    }
  }
  cgi_printf("<p>A shunned artifact will not be pushed nor accepted in a pull and the\n"
         "artifact content will be purged from the repository the next time the\n"
         "repository is rebuilt.  A list of shunned artifacts can be seen at the\n"
         "bottom of this page.</p>\n"
         "\n"
         "<a name=\"addshun\"></a>\n"
         "<p>To shun an artifact, enter its artifact ID (the 40-character SHA1\n"
         "hash of the artifact) in the\n"
         "following box and press the \"Shun\" button.  This will cause the artifact\n"
         "to be removed from the repository and will prevent the artifact from being\n"
         "readded to the repository by subsequent sync operation.</p>\n"
         "\n"
         "<p>Note that you must enter the full 40-character artifact ID, not\n"
         "an abbreviation or a symbolic tag.</p>\n"
         "\n"
         "<p>Warning:  Shunning should only be used to remove inappropriate content\n"
         "from the repository.  Inappropriate content includes such things as\n"
         "spam added to Wiki, files that violate copyright or patent agreements,\n"
         "or artifacts that by design or accident interfere with the processing\n"
         "of the repository.  Do not shun artifacts merely to remove them from\n"
         "sight - set the \"hidden\" tag on such artifacts instead.</p>\n"
         "\n"
         "<blockquote>\n"
         "<form method=\"post\" action=\"%s/%s\"><div>\n",(g.zTop),(g.zPath));
  login_insert_csrf_secret();
  cgi_printf("<input type=\"text\" name=\"uuid\" value=\"%h\" size=\"50\" />\n"
         "<input type=\"submit\" name=\"add\" value=\"Shun\" />\n"
         "</div></form>\n"
         "</blockquote>\n"
         "\n"
         "<a name=\"delshun\"></a>\n"
         "<p>Enter the UUID of a previous shunned artifact to cause it to be\n"
         "accepted again in the repository.  The artifact content is not\n"
         "restored because the content is unknown.  The only change is that\n"
         "the formerly shunned artifact will be accepted on subsequent sync\n"
         "operations.</p>\n"
         "\n"
         "<blockquote>\n"
         "<form method=\"post\" action=\"%s/%s\"><div>\n",(PD("shun","")),(g.zTop),(g.zPath));
  login_insert_csrf_secret();
  cgi_printf("<input type=\"text\" name=\"uuid\" value=\"%h\" size=\"50\" />\n"
         "<input type=\"submit\" name=\"sub\" value=\"Accept\" />\n"
         "</div></form>\n"
         "</blockquote>\n"
         "\n"
         "<p>Press the Rebuild button below to rebuild the repository.  The\n"
         "content of newly shunned artifacts is not purged until the repository\n"
         "is rebuilt.  On larger repositories, the rebuild may take minute or\n"
         "two, so be patient after pressing the button.</p>\n"
         "\n"
         "<blockquote>\n"
         "<form method=\"post\" action=\"%s/%s\"><div>\n",(PD("accept", "")),(g.zTop),(g.zPath));
  login_insert_csrf_secret();
  cgi_printf("<input type=\"submit\" name=\"rebuild\" value=\"Rebuild\" />\n"
         "</div></form>\n"
         "</blockquote>\n"
         "\n"
         "<hr /><p>Shunned Artifacts:</p>\n"
         "<blockquote><p>\n");
  db_prepare(&q, 
     "SELECT uuid, EXISTS(SELECT 1 FROM blob WHERE blob.uuid=shun.uuid)"
     "  FROM shun ORDER BY uuid");
  while( db_step(&q)==SQLITE_ROW ){
    const char *zUuid = db_column_text(&q, 0);
    int stillExists = db_column_int(&q, 1);
    cnt++;
    if( stillExists ){
      cgi_printf("<b><a href=\"%s/artifact/%s\">%s</a></b><br />\n",(g.zTop),(zUuid),(zUuid));
    }else{
      cgi_printf("<b>%s</b><br />\n",(zUuid));
    }
  }
  if( cnt==0 ){
    cgi_printf("<i>no artifacts are shunned on this server</i>\n");
  }
  db_finalize(&q);
  cgi_printf("</p></blockquote>\n");
  style_footer();
}

/*
** Remove from the BLOB table all artifacts that are in the SHUN table.
*/
void shun_artifacts(void){
  Stmt q;
  db_multi_exec(
     "CREATE TEMP TABLE toshun(rid INTEGER PRIMARY KEY);"
     "INSERT INTO toshun SELECT rid FROM blob, shun WHERE blob.uuid=shun.uuid;"
  );
  db_prepare(&q,
     "SELECT rid FROM delta WHERE srcid IN toshun"
  );
  while( db_step(&q)==SQLITE_ROW ){
    int srcid = db_column_int(&q, 0);
    content_undelta(srcid);
  }
  db_finalize(&q);
  db_multi_exec(
     "DELETE FROM delta WHERE rid IN toshun;"
     "DELETE FROM blob WHERE rid IN toshun;"
     "DROP TABLE toshun;"
     "DELETE FROM private "
     " WHERE NOT EXISTS (SELECT 1 FROM blob WHERE rid=private.rid);"
  );
}

/*
** WEBPAGE: rcvfromlist
**
** Show a listing of RCVFROM table entries.
*/
void rcvfromlist_page(void){
  int ofst = atoi(PD("ofst","0"));
  int cnt;
  Stmt q;

  login_check_credentials();
  if( !g.perm.Admin ){
    login_needed();
  }
  style_header("Content Sources");
  if( ofst>0 ){
    style_submenu_element("Newer", "Newer", "rcvfromlist?ofst=%d",
                           ofst>30 ? ofst-30 : 0);
  }
  db_prepare(&q, 
    "SELECT rcvid, login, datetime(rcvfrom.mtime), rcvfrom.ipaddr"
    "  FROM rcvfrom LEFT JOIN user USING(uid)"
    " ORDER BY rcvid DESC LIMIT 31 OFFSET %d",
    ofst
  );
  cgi_printf("<p>Whenever new artifacts are added to the repository, either by\n"
         "push or using the web interface, an entry is made in the RCVFROM table\n"
         "to record the source of that artifact.  This log facilitates\n"
         "finding and fixing attempts to inject illicit content into the\n"
         "repository.</p>\n"
         "\n"
         "<p>Click on the \"rcvid\" to show a list of specific artifacts received\n"
         "by a transaction.  After identifying illicit artifacts, remove them\n"
         "using the \"Shun\" feature.</p>\n"
         "\n"
         "<table cellpadding=\"0\" cellspacing=\"0\" border=\"0\">\n"
         "<tr><th style=\"padding-right: 15px;text-align: right;\">rcvid</th>\n"
         "    <th style=\"padding-right: 15px;text-align: left;\">Date</th>\n"
         "    <th style=\"padding-right: 15px;text-align: left;\">User</th>\n"
         "    <th style=\"text-align: left;\">IP&nbsp;Address</th></tr>\n");
  cnt = 0;
  while( db_step(&q)==SQLITE_ROW ){
    int rcvid = db_column_int(&q, 0);
    const char *zUser = db_column_text(&q, 1);
    const char *zDate = db_column_text(&q, 2);
    const char *zIpAddr = db_column_text(&q, 3);
    if( cnt==30 ){
      style_submenu_element("Older", "Older",
         "rcvfromlist?ofst=%d", ofst+30);
    }else{
      cnt++;
      cgi_printf("<tr>\n"
             "<td style=\"padding-right: 15px;text-align: right;\"><a href=\"rcvfrom?rcvid=%d\">%d</a></td>\n"
             "<td style=\"padding-right: 15px;text-align: left;\">%s</td>\n"
             "<td style=\"padding-right: 15px;text-align: left;\">%h</td>\n"
             "<td style=\"text-align: left;\">%s</td>\n"
             "</tr>\n",(rcvid),(rcvid),(zDate),(zUser),(zIpAddr));
    }
  }
  db_finalize(&q);
  cgi_printf("</table>\n");
  style_footer();
}

/*
** WEBPAGE: rcvfrom
**
** Show a single RCVFROM table entry.
*/
void rcvfrom_page(void){
  int rcvid = atoi(PD("rcvid","0"));
  Stmt q;

  login_check_credentials();
  if( !g.perm.Admin ){
    login_needed();
  }
  style_header("Content Source %d", rcvid);
  db_prepare(&q, 
    "SELECT login, datetime(rcvfrom.mtime), rcvfrom.ipaddr"
    "  FROM rcvfrom LEFT JOIN user USING(uid)"
    " WHERE rcvid=%d",
    rcvid
  );
  cgi_printf("<table cellspacing=\"15\" cellpadding=\"0\" border=\"0\">\n"
         "<tr><th valign=\"top\" align=\"right\">rcvid:</th>\n"
         "<td valign=\"top\">%d</td></tr>\n",(rcvid));
  if( db_step(&q)==SQLITE_ROW ){
    const char *zUser = db_column_text(&q, 0);
    const char *zDate = db_column_text(&q, 1);
    const char *zIpAddr = db_column_text(&q, 2);
    cgi_printf("<tr><th valign=\"top\" align=\"right\">User:</th>\n"
           "<td valign=\"top\">%s</td></tr>\n"
           "<tr><th valign=\"top\" align=\"right\">Date:</th>\n"
           "<td valign=\"top\">%s</td></tr>\n"
           "<tr><th valign=\"top\" align=\"right\">IP&nbsp;Address:</th>\n"
           "<td valign=\"top\">%s</td></tr>\n",(zUser),(zDate),(zIpAddr));
  }
  db_finalize(&q);
  db_prepare(&q,
    "SELECT rid, uuid, size FROM blob WHERE rcvid=%d", rcvid
  );
  cgi_printf("<tr><th valign=\"top\" align=\"right\">Artifacts:</th>\n"
         "<td valign=\"top\">\n");
  while( db_step(&q)==SQLITE_ROW ){
    int rid = db_column_int(&q, 0);
    const char *zUuid = db_column_text(&q, 1);
    int size = db_column_int(&q, 2);
    cgi_printf("<a href=\"%s/info/%s\">%s</a>\n"
           "(rid: %d, size: %d)<br />\n",(g.zTop),(zUuid),(zUuid),(rid),(size));
  }
  cgi_printf("</td></tr>\n"
         "</table>\n");
  db_finalize(&q);
  style_footer();
}
