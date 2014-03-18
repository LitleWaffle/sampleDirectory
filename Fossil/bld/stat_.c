#line 1 "./src/stat.c"
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
** This file contains code to implement the stat web page
**
*/
#include "config.h"
#include <string.h>
#include "stat.h"

/*
** For a sufficiently large integer, provide an alternative
** representation as MB or GB or TB.
*/
static void bigSizeName(int nOut, char *zOut, sqlite3_int64 v){
  if( v<100000 ){
    sqlite3_snprintf(nOut, zOut, "%lld bytes", v);
  }else if( v<1000000000 ){
    sqlite3_snprintf(nOut, zOut, "%lld bytes (%.1fMB)",
                    v, (double)v/1000000.0);
  }else{
    sqlite3_snprintf(nOut, zOut, "%lld bytes (%.1fGB)",
                    v, (double)v/1000000000.0);
  }
}

/*
** WEBPAGE: stat
**
** Show statistics and global information about the repository.
*/
void stat_page(void){
  i64 t, fsize;
  int n, m;
  int szMax, szAvg;
  const char *zDb;
  int brief;
  char zBuf[100];

  login_check_credentials();
  if( !g.perm.Read ){ login_needed(); return; }
  brief = P("brief")!=0;
  style_header("Repository Statistics");
  if( g.perm.Admin ){
    style_submenu_element("URLs", "URLs and Checkouts", "urllist");
    style_submenu_element("Schema", "Repository Schema", "repo_schema");
  }
  cgi_printf("<table class=\"label-value\">\n"
         "<tr><th>Repository&nbsp;Size:</th><td>\n");
  fsize = file_size(g.zRepositoryName);
  bigSizeName(sizeof(zBuf), zBuf, fsize);
  cgi_printf("%s\n"
         "</td></tr>\n",(zBuf));
  if( !brief ){
    cgi_printf("<tr><th>Number&nbsp;Of&nbsp;Artifacts:</th><td>\n");
    n = db_int(0, "SELECT count(*) FROM blob");
    m = db_int(0, "SELECT count(*) FROM delta");
    cgi_printf("%d (%d fulltext and %d deltas)\n"
           "</td></tr>\n",(n),(n-m),(m));
    if( n>0 ){
      int a, b;
      Stmt q;
      cgi_printf("<tr><th>Uncompressed&nbsp;Artifact&nbsp;Size:</th><td>\n");
      db_prepare(&q, "SELECT total(size), avg(size), max(size)"
                     " FROM blob WHERE size>0");
      db_step(&q);
      t = db_column_int64(&q, 0);
      szAvg = db_column_int(&q, 1);
      szMax = db_column_int(&q, 2);
      db_finalize(&q);
      bigSizeName(sizeof(zBuf), zBuf, t);
      cgi_printf("%d bytes average, %d bytes max, %s total\n"
             "</td></tr>\n"
             "<tr><th>Compression&nbsp;Ratio:</th><td>\n",(szAvg),(szMax),(zBuf));
      if( t/fsize < 5 ){
        b = 10;
        fsize /= 10;
      }else{
        b = 1;
      }
      a = t/fsize;
      cgi_printf("%d:%d\n"
             "</td></tr>\n",(a),(b));
    }
    cgi_printf("<tr><th>Number&nbsp;Of&nbsp;Check-ins:</th><td>\n");
    n = db_int(0, "SELECT count(*) FROM event WHERE type='ci' /*scan*/");
    cgi_printf("%d\n"
           "</td></tr>\n"
           "<tr><th>Number&nbsp;Of&nbsp;Files:</th><td>\n",(n));
    n = db_int(0, "SELECT count(*) FROM filename /*scan*/");
    cgi_printf("%d\n"
           "</td></tr>\n"
           "<tr><th>Number&nbsp;Of&nbsp;Wiki&nbsp;Pages:</th><td>\n",(n));
    n = db_int(0, "SELECT count(*) FROM tag  /*scan*/"
                  " WHERE +tagname GLOB 'wiki-*'");
    cgi_printf("%d\n"
           "</td></tr>\n"
           "<tr><th>Number&nbsp;Of&nbsp;Tickets:</th><td>\n",(n));
    n = db_int(0, "SELECT count(*) FROM tag  /*scan*/"
                  " WHERE +tagname GLOB 'tkt-*'");
    cgi_printf("%d\n"
           "</td></tr>\n",(n));
  }
  cgi_printf("<tr><th>Duration&nbsp;Of&nbsp;Project:</th><td>\n");
  n = db_int(0, "SELECT julianday('now') - (SELECT min(mtime) FROM event)"
                " + 0.99");
  cgi_printf("%d days or approximately %.2f years.\n"
         "</td></tr>\n"
         "<tr><th>Project&nbsp;ID:</th><td>%h</td></tr>\n"
         "<tr><th>Fossil&nbsp;Version:</th><td>\n"
         "%h %h\n"
         "(%h) [compiled using %h]\n"
         "</td></tr>\n"
         "<tr><th>SQLite&nbsp;Version:</th><td>%.19s\n"
         "[%.10s] (%s)</td></tr>\n"
         "<tr><th>Repository Rebuilt:</th><td>\n"
         "%h\n"
         "By Fossil %h</td></tr>\n"
         "<tr><th>Database&nbsp;Stats:</th><td>\n",(n),(n/365.2425),(db_get("project-code","")),(MANIFEST_DATE),(MANIFEST_VERSION),(RELEASE_VERSION),(COMPILER_NAME),(sqlite3_sourceid()),(&sqlite3_sourceid()[20]),(sqlite3_libversion()),(db_get_mtime("rebuilt","%Y-%m-%d %H:%M:%S","Never")),(db_get("rebuilt","Unknown")));
  zDb = db_name("repository");
  cgi_printf("%d pages,\n"
         "%d bytes/page,\n"
         "%d free pages,\n"
         "%s,\n"
         "%s mode\n"
         "</td></tr>\n",(db_int(0, "PRAGMA %s.page_count", zDb)),(db_int(0, "PRAGMA %s.page_size", zDb)),(db_int(0, "PRAGMA %s.freelist_count", zDb)),(db_text(0, "PRAGMA %s.encoding", zDb)),(db_text(0, "PRAGMA %s.journal_mode", zDb)));

  cgi_printf("</table>\n");
  style_footer();
}

/*
** COMMAND: dbstat*
**
** Usage: %fossil dbstat ?-brief | -b?
**
** Shows statistics and global information about the repository.
**
** The (-brief|-b) option removes any "long-running" statistics, namely
** those whose calculations are known to slow down as the repository
** grows.
**
*/
void dbstat_cmd(void){
  i64 t, fsize;
  int n, m;
  int szMax, szAvg;
  const char *zDb;
  int brief;
  char zBuf[100];
  const int colWidth = -19 /* printf alignment/width for left column */;
  brief = find_option("brief", "b",0)!=0;
  db_find_and_open_repository(0,0);
  fsize = file_size(g.zRepositoryName);
  bigSizeName(sizeof(zBuf), zBuf, fsize);
  fossil_print( "%*s%s\n", colWidth, "repository-size:", zBuf );
  if( !brief ){
    n = db_int(0, "SELECT count(*) FROM blob");
    m = db_int(0, "SELECT count(*) FROM delta");
    fossil_print("%*s%d (stored as %d full text and %d delta blobs)\n",
                 colWidth, "artifact-count:",
                 n, n-m, m);
    if( n>0 ){
      int a, b;
      Stmt q;
      db_prepare(&q, "SELECT total(size), avg(size), max(size)"
                     " FROM blob WHERE size>0");
      db_step(&q);
      t = db_column_int64(&q, 0);
      szAvg = db_column_int(&q, 1);
      szMax = db_column_int(&q, 2);
      db_finalize(&q);
      bigSizeName(sizeof(zBuf), zBuf, t);
      fossil_print( "%*s%d average, "
                    "%d max, %s total\n",
                    colWidth, "artifact-sizes:",
                    szAvg, szMax, zBuf);
      if( t/fsize < 5 ){
        b = 10;
        fsize /= 10;
      }else{
        b = 1;
      }
      a = t/fsize;
      fossil_print("%*s%d:%d\n", colWidth, "compression-ratio:", a, b);
    }
    n = db_int(0, "SELECT COUNT(*) FROM event e WHERE e.type='ci'");
    fossil_print("%*s%d\n", colWidth, "checkins:", n);
    n = db_int(0, "SELECT count(*) FROM filename /*scan*/");
    fossil_print("%*s%d across all branches\n", colWidth, "files:", n);
    n = db_int(0, "SELECT count(*) FROM tag  /*scan*/"
                  " WHERE tagname GLOB 'wiki-*'");
    m = db_int(0, "SELECT COUNT(*) FROM event WHERE type='w'");
    fossil_print("%*s%d (%d changes)\n", colWidth, "wikipages:", n, m);
    n = db_int(0, "SELECT count(*) FROM tag  /*scan*/"
                  " WHERE tagname GLOB 'tkt-*'");
    m = db_int(0, "SELECT COUNT(*) FROM event WHERE type='t'");
    fossil_print("%*s%d (%d changes)\n", colWidth, "tickets:", n, m);
    n = db_int(0, "SELECT COUNT(*) FROM event WHERE type='e'");
    fossil_print("%*s%d\n", colWidth, "events:", n);
    n = db_int(0, "SELECT COUNT(*) FROM event WHERE type='g'");
    fossil_print("%*s%d\n", colWidth, "tagchanges:", n);
  }
  n = db_int(0, "SELECT julianday('now') - (SELECT min(mtime) FROM event)"
                " + 0.99");
  fossil_print("%*s%d days or approximately %.2f years.\n",
               colWidth, "project-age:", n, n/365.2425);
  fossil_print("%*s%s\n", colWidth, "project-id:", db_get("project-code",""));
  fossil_print("%*s%s %s [%s] (%s)\n",
               colWidth, "fossil-version:",
               MANIFEST_DATE, MANIFEST_VERSION, RELEASE_VERSION,
               COMPILER_NAME);
  fossil_print("%*s%.19s [%.10s] (%s)\n",
               colWidth, "sqlite-version:",
               sqlite3_sourceid(), &sqlite3_sourceid()[20],
               sqlite3_libversion());
  zDb = db_name("repository");
  fossil_print("%*s%d pages, %d bytes/pg, %d free pages, "
               "%s, %s mode\n",
               colWidth, "database-stats:",
               db_int(0, "PRAGMA %s.page_count", zDb),
               db_int(0, "PRAGMA %s.page_size", zDb),
               db_int(0, "PRAGMA %s.freelist_count", zDb),
               db_text(0, "PRAGMA %s.encoding", zDb),
               db_text(0, "PRAGMA %s.journal_mode", zDb));

}

/*
** WEBPAGE: urllist
**
** Show ways in which this repository has been accessed
*/
void urllist_page(void){
  Stmt q;
  int cnt;
  login_check_credentials();
  if( !g.perm.Admin ){ login_needed(); return; }

  style_header("URLs and Checkouts");
  style_submenu_element("Stat", "Repository Stats", "stat");
  style_submenu_element("Schema", "Repository Schema", "repo_schema");
  cgi_printf("<div class=\"section\">URLs</div>\n"
         "<table border=\"0\" width='100%%'>\n");
  db_prepare(&q, "SELECT substr(name,9), datetime(mtime,'unixepoch')"
                 "  FROM config WHERE name GLOB 'baseurl:*' ORDER BY 2 DESC");
  cnt = 0;
  while( db_step(&q)==SQLITE_ROW ){
    cgi_printf("<tr><td width='100%%'>%h</td>\n"
           "<td><nobr>%h</nobr></td></tr>\n",(db_column_text(&q,0)),(db_column_text(&q,1)));
    cnt++;
  }
  db_finalize(&q);
  if( cnt==0 ){
    cgi_printf("<tr><td>(none)</td>\n");
  }
  cgi_printf("</table>\n"
         "<div class=\"section\">Checkouts</div>\n"
         "<table border=\"0\" width='100%%'>\n");
  db_prepare(&q, "SELECT substr(name,7), datetime(mtime,'unixepoch')"
                 "  FROM config WHERE name GLOB 'ckout:*' ORDER BY 2 DESC");
  cnt = 0;
  while( db_step(&q)==SQLITE_ROW ){
    cgi_printf("<tr><td width='100%%'>%h</td>\n"
           "<td><nobr>%h</nobr></td></tr>\n",(db_column_text(&q,0)),(db_column_text(&q,1)));
    cnt++;
  }
  db_finalize(&q);
  if( cnt==0 ){
    cgi_printf("<tr><td>(none)</td>\n");
  }
  cgi_printf("</table>\n");
  style_footer();
}

/*
** WEBPAGE: repo_schema
**
** Show the repository schema
*/
void repo_schema_page(void){
  Stmt q;
  login_check_credentials();
  if( !g.perm.Admin ){ login_needed(); return; }

  style_header("Repository Schema");
  style_submenu_element("Stat", "Repository Stats", "stat");
  style_submenu_element("URLs", "URLs and Checkouts", "urllist");
  db_prepare(&q, "SELECT sql FROM %s.sqlite_master WHERE sql IS NOT NULL",
             db_name("repository"));
  cgi_printf("<pre>\n");
  while( db_step(&q)==SQLITE_ROW ){
    cgi_printf("%h;\n",(db_column_text(&q, 0)));
  }
  cgi_printf("</pre>\n");
  db_finalize(&q);
  style_footer();
}
