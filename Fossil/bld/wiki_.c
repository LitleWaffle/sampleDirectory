#line 1 "./src/wiki.c"
/*
** Copyright (c) 2007 D. Richard Hipp
** Copyright (c) 2008 Stephan Beal
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
** This file contains code to do formatting of wiki text.
*/
#include "config.h"
#include <assert.h>
#include <ctype.h>
#include "wiki.h"

/*
** Return true if the input string is a well-formed wiki page name.
**
** Well-formed wiki page names do not begin or end with whitespace,
** and do not contain tabs or other control characters and do not
** contain more than a single space character in a row.  Well-formed
** names must be between 1 and 100 characters in length, inclusive.
*/
int wiki_name_is_wellformed(const unsigned char *z){
  int i;
  if( z[0]<=0x20 ){
    return 0;
  }
  for(i=1; z[i]; i++){
    if( z[i]<0x20 ) return 0;
    if( z[i]==0x20 && z[i-1]==0x20 ) return 0;
  }
  if( z[i-1]==' ' ) return 0;
  if( i<1 || i>100 ) return 0;
  return 1;
}

/*
** Output rules for well-formed wiki pages
*/
static void well_formed_wiki_name_rules(void){
  cgi_printf("<ul>\n"
         "<li> Must not begin or end with a space.</li>\n"
         "<li> Must not contain any control characters, including tab or\n"
         "     newline.</li>\n"
         "<li> Must not have two or more spaces in a row internally.</li>\n"
         "<li> Must be between 1 and 100 characters in length.</li>\n"
         "</ul>\n");
}

/*
** Check a wiki name.  If it is not well-formed, then issue an error
** and return true.  If it is well-formed, return false.
*/
static int check_name(const char *z){
  if( !wiki_name_is_wellformed((const unsigned char *)z) ){
    style_header("Wiki Page Name Error");
    cgi_printf("The wiki name \"<span class=\"wikiError\">%h</span>\" is not well-formed.\n"
           "Rules for wiki page names:\n",(z));
    well_formed_wiki_name_rules();
    style_footer();
    return 1;
  }
  return 0;
}

/*
** WEBPAGE: home
** WEBPAGE: index
** WEBPAGE: not_found
*/
void home_page(void){
  char *zPageName = db_get("project-name",0);
  char *zIndexPage = db_get("index-page",0);
  login_check_credentials();
  if( zIndexPage ){
    const char *zPathInfo = P("PATH_INFO");
    while( zIndexPage[0]=='/' ) zIndexPage++;
    while( zPathInfo[0]=='/' ) zPathInfo++;
    if( fossil_strcmp(zIndexPage, zPathInfo)==0 ) zIndexPage = 0;
  }
  if( zIndexPage ){
    cgi_redirectf("%s/%s", g.zTop, zIndexPage);
  }
  if( !g.perm.RdWiki ){
    cgi_redirectf("%s/login?g=%s/home", g.zTop, g.zTop);
  }
  if( zPageName ){
    login_check_credentials();
    g.zExtra = zPageName;
    cgi_set_parameter_nocopy("name", g.zExtra, 1);
    g.isHome = 1;
    wiki_page();
    return;
  }
  style_header("Home");
  cgi_printf("<p>This is a stub home-page for the project.\n"
         "To fill in this page, first go to\n"
         "%zsetup/config</a>\n"
         "and establish a \"Project Name\".  Then create a\n"
         "wiki page with that name.  The content of that wiki page\n"
         "will be displayed in place of this message.</p>\n",(href("%R/setup_config")));
  style_footer();
}

/*
** Return true if the given pagename is the name of the sandbox
*/
static int is_sandbox(const char *zPagename){
  return fossil_stricmp(zPagename,"sandbox")==0 ||
         fossil_stricmp(zPagename,"sand box")==0;
}

/*
** Only allow certain mimetypes through.
** All others become "text/x-fossil-wiki"
*/
const char *wiki_filter_mimetypes(const char *zMimetype){
  if( zMimetype!=0 &&
      ( fossil_strcmp(zMimetype, "text/x-markdown")==0
        || fossil_strcmp(zMimetype, "text/plain")==0 )
  ){
    return zMimetype;
  }
  return "text/x-fossil-wiki";
}

/*
** Render wiki text according to its mimetype
*/
void wiki_render_by_mimetype(Blob *pWiki, const char *zMimetype){
  if( zMimetype==0 || fossil_strcmp(zMimetype, "text/x-fossil-wiki")==0 ){
    wiki_convert(pWiki, 0, 0);
  }else if( fossil_strcmp(zMimetype, "text/x-markdown")==0 ){
    Blob title = BLOB_INITIALIZER;
    Blob tail = BLOB_INITIALIZER;
    markdown_to_html(pWiki, &title, &tail);
    if( blob_size(&title)>0 ){
      cgi_printf("<h1>%s</h1>\n",(blob_str(&title)));
    }
    cgi_printf("%s\n",(blob_str(&tail)));
    blob_reset(&title);
    blob_reset(&tail);
  }else{
    cgi_printf("<pre>\n"
           "%h\n"
           "</pre>\n",(blob_str(pWiki)));
  }
}


/*
** WEBPAGE: wiki
** URL: /wiki?name=PAGENAME
*/
void wiki_page(void){
  char *zTag;
  int rid = 0;
  int isSandbox;
  char *zUuid;
  Blob wiki;
  Manifest *pWiki = 0;
  const char *zPageName;
  const char *zMimetype = 0;
  char *zBody = mprintf("%s","<i>Empty Page</i>");

  login_check_credentials();
  if( !g.perm.RdWiki ){ login_needed(); return; }
  zPageName = P("name");
  if( zPageName==0 ){
    style_header("Wiki");
    cgi_printf("<ul>\n");
    { char *zWikiHomePageName = db_get("index-page",0);
      if( zWikiHomePageName ){
        cgi_printf("<li> %z\n"
               "     %h</a> wiki home page.</li>\n",(href("%R%s",zWikiHomePageName)),(zWikiHomePageName));
      }
    }
    { char *zHomePageName = db_get("project-name",0);
      if( zHomePageName ){
        cgi_printf("<li> %z\n"
               "     %h</a> project home page.</li>\n",(href("%R/wiki?name=%t",zHomePageName)),(zHomePageName));
      }
    }
    cgi_printf("<li> %zRecent changes</a> to wiki pages.</li>\n"
           "<li> %zFormatting rules</a> for wiki.</li>\n"
           "<li> Use the %zSandbox</a>\n"
           "     to experiment.</li>\n",(href("%R/timeline?y=w")),(href("%R/wiki_rules")),(href("%R/wiki?name=Sandbox")));
    if( g.perm.NewWiki ){
      cgi_printf("<li>  Create a %znew wiki page</a>.</li>\n",(href("%R/wikinew")));
      if( g.perm.Write ){
        cgi_printf("<li>   Create a %znew event</a>.</li>\n",(href("%R/eventedit")));
      }
    }
    cgi_printf("<li> %zList of All Wiki Pages</a>\n"
           "     available on this server.</li>\n",(href("%R/wcontent")));
    if( g.perm.ModWiki ){
      cgi_printf("<li> %zTend to pending moderation requests</a></li>\n",(href("%R/modreq")));
    }
    cgi_printf("<li>\n");
    form_begin(0, "%R/wfind");
    cgi_printf(" <div>Search wiki titles: <input type=\"text\" name=\"title\"/>\n"
           " &nbsp; <input type=\"submit\" /></div></form>\n"
           "</li>\n"
           "</ul>\n");
    style_footer();
    return;
  }
  if( check_name(zPageName) ) return;
  isSandbox = is_sandbox(zPageName);
  if( isSandbox ){
    zBody = db_get("sandbox",zBody);
    zMimetype = db_get("sandbox-mimetype","text/x-fossil-wiki");
    rid = 0;
  }else{
    zTag = mprintf("wiki-%s", zPageName);
    rid = db_int(0, 
      "SELECT rid FROM tagxref"
      " WHERE tagid=(SELECT tagid FROM tag WHERE tagname=%Q)"
      " ORDER BY mtime DESC", zTag
    );
    free(zTag);
    pWiki = manifest_get(rid, CFTYPE_WIKI, 0);
    if( pWiki ){
      zBody = pWiki->zWiki;
      zMimetype = pWiki->zMimetype;
    }
  }
  zMimetype = wiki_filter_mimetypes(zMimetype);
  if( !g.isHome ){
    if( rid ){
      style_submenu_element("Diff", "Last change",
                 "%R/wdiff?name=%T&a=%d", zPageName, rid);
      zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
      style_submenu_element("Details", "Details",
                   "%R/info/%S", zUuid);
    }
    if( (rid && g.perm.WrWiki) || (!rid && g.perm.NewWiki) ){
      if( db_get_boolean("wysiwyg-wiki", 0) ){
        style_submenu_element("Edit", "Edit Wiki Page",
             "%s/wikiedit?name=%T&wysiwyg=1",
             g.zTop, zPageName);
      }else{
        style_submenu_element("Edit", "Edit Wiki Page",
             "%s/wikiedit?name=%T",
             g.zTop, zPageName);
      }
    }
    if( rid && g.perm.ApndWiki && g.perm.Attach ){
      style_submenu_element("Attach", "Add An Attachment",
           "%s/attachadd?page=%T&from=%s/wiki%%3fname=%T",
           g.zTop, zPageName, g.zTop, zPageName);
    }
    if( rid && g.perm.ApndWiki ){
      style_submenu_element("Append", "Add A Comment", 
           "%s/wikiappend?name=%T&mimetype=%s",
           g.zTop, zPageName, zMimetype);
    }
    if( g.perm.Hyperlink ){
      style_submenu_element("History", "History", "%s/whistory?name=%T",
           g.zTop, zPageName);
    }
  }
  style_set_current_page("%s?name=%T", g.zPath, zPageName);
  style_header(zPageName);
  blob_init(&wiki, zBody, -1);
  wiki_render_by_mimetype(&wiki, zMimetype);
  blob_reset(&wiki);
  attachment_list(zPageName, "<hr /><h2>Attachments:</h2><ul>");
  manifest_destroy(pWiki);
  style_footer();
}

/*
** Write a wiki artifact into the repository
*/
static void wiki_put(Blob *pWiki, int parent){
  int nrid;
  if( g.perm.ModWiki || db_get_boolean("modreq-wiki",0)==0 ){
    nrid = content_put_ex(pWiki, 0, 0, 0, 0);
    if( parent) content_deltify(parent, nrid, 0);
  }else{
    nrid = content_put_ex(pWiki, 0, 0, 0, 1);
    moderation_table_create();
    db_multi_exec("INSERT INTO modreq(objid) VALUES(%d)", nrid);
  }
  db_multi_exec("INSERT OR IGNORE INTO unsent VALUES(%d)", nrid);
  db_multi_exec("INSERT OR IGNORE INTO unclustered VALUES(%d);", nrid);
  manifest_crosslink(nrid, pWiki, MC_NONE);
}

/*
** Formal names and common names for the various wiki styles.
*/
static const char *azStyles[] = {
  "text/x-fossil-wiki", "Fossil Wiki",
  "text/x-markdown",    "Markdown",
  "text/plain",         "Plain Text"
};

/*
** Output a selection box from which the user can select the
** wiki mimetype.
*/
static void mimetype_option_menu(const char *zMimetype){
  unsigned i;
  cgi_printf("Markup style: <select name=\"mimetype\" size=\"1\">\n");
  for(i=0; i<sizeof(azStyles)/sizeof(azStyles[0]); i+=2){
    if( fossil_strcmp(zMimetype,azStyles[i])==0 ){
      cgi_printf("<option value=\"%s\" selected>%s</option>\n",(azStyles[i]),(azStyles[i+1]));
    }else{
      cgi_printf("<option value=\"%s\">%s</option>\n",(azStyles[i]),(azStyles[i+1]));
    }
  }
  cgi_printf("</select>\n");
}

/*
** Given a mimetype, return its common name.
*/
static const char *mimetype_common_name(const char *zMimetype){
  int i;
  for(i=4; i>=2; i-=2){
    if( zMimetype && fossil_strcmp(zMimetype, azStyles[i])==0 ){
      return azStyles[i+1];
    }
  }
  return azStyles[1];
}

/*
** WEBPAGE: wikiedit
** URL: /wikiedit?name=PAGENAME
*/
void wikiedit_page(void){
  char *zTag;
  int rid = 0;
  int isSandbox;
  Blob wiki;
  Manifest *pWiki = 0;
  const char *zPageName;
  int n;
  const char *z;
  char *zBody = (char*)P("w");
  const char *zMimetype = wiki_filter_mimetypes(P("mimetype"));
  int isWysiwyg = P("wysiwyg")!=0;
  int goodCaptcha = 1;

  if( P("edit-wysiwyg")!=0 ){ isWysiwyg = 1; zBody = 0; }
  if( P("edit-markup")!=0 ){ isWysiwyg = 0; zBody = 0; }
  if( zBody ){
    if( isWysiwyg ){
      Blob body;
      blob_zero(&body);
      htmlTidy(zBody, &body);
      zBody = blob_str(&body);
    }else{
      zBody = mprintf("%s", zBody);
    }
  }
  login_check_credentials();
  zPageName = PD("name","");
  if( check_name(zPageName) ) return;
  isSandbox = is_sandbox(zPageName);
  if( isSandbox ){
    if( !g.perm.WrWiki ){
      login_needed();
      return;
    }
    if( zBody==0 ){
      zBody = db_get("sandbox","");
      zMimetype = db_get("sandbox-mimetype","text/x-fossil-wiki");
    }
  }else{
    zTag = mprintf("wiki-%s", zPageName);
    rid = db_int(0, 
      "SELECT rid FROM tagxref"
      " WHERE tagid=(SELECT tagid FROM tag WHERE tagname=%Q)"
      " ORDER BY mtime DESC", zTag
    );
    free(zTag);
    if( (rid && !g.perm.WrWiki) || (!rid && !g.perm.NewWiki) ){
      login_needed();
      return;
    }
    if( zBody==0 && (pWiki = manifest_get(rid, CFTYPE_WIKI, 0))!=0 ){
      zBody = pWiki->zWiki;
      zMimetype = pWiki->zMimetype;
    }
  }
  if( P("submit")!=0 && zBody!=0
   && (goodCaptcha = captcha_is_correct())
  ){
    char *zDate;
    Blob cksum;
    blob_zero(&wiki);
    db_begin_transaction();
    if( isSandbox ){
      db_set("sandbox",zBody,0);
      db_set("sandbox-mimetype",zMimetype,0);
    }else{
      login_verify_csrf_secret();
      zDate = date_in_standard_format("now");
      blob_appendf(&wiki, "D %s\n", zDate);
      free(zDate);
      blob_appendf(&wiki, "L %F\n", zPageName);
      if( fossil_strcmp(zMimetype,"text/x-fossil-wiki")!=0 ){
        blob_appendf(&wiki, "N %s\n", zMimetype);
      }
      if( rid ){
        char *zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
        blob_appendf(&wiki, "P %s\n", zUuid);
        free(zUuid);
      }
      if( g.zLogin ){
        blob_appendf(&wiki, "U %F\n", g.zLogin);
      }
      blob_appendf(&wiki, "W %d\n%s\n", strlen(zBody), zBody);
      md5sum_blob(&wiki, &cksum);
      blob_appendf(&wiki, "Z %b\n", &cksum);
      blob_reset(&cksum);
      wiki_put(&wiki, 0);
    }
    db_end_transaction(0);
    cgi_redirectf("wiki?name=%T", zPageName);
  }
  if( P("cancel")!=0 ){
    cgi_redirectf("wiki?name=%T", zPageName);
    return;
  }
  if( zBody==0 ){
    zBody = mprintf("<i>Empty Page</i>");
  }
  style_set_current_page("%s?name=%T", g.zPath, zPageName);
  style_header("Edit: %s", zPageName);
  if( !goodCaptcha ){
    cgi_printf("<p class=\"generalError\">Error:  Incorrect security code.</p>\n");
  }
  blob_zero(&wiki);
  blob_append(&wiki, zBody, -1);
  if( P("preview")!=0 ){
    cgi_printf("Preview:<hr />\n");
    wiki_render_by_mimetype(&wiki, zMimetype);
    cgi_printf("<hr />\n");
    blob_reset(&wiki);
  }
  for(n=2, z=zBody; z[0]; z++){
    if( z[0]=='\n' ) n++;
  }
  if( n<20 ) n = 20;
  if( n>30 ) n = 30;
  if( !isWysiwyg ){
    /* Traditional markup-only editing */
    form_begin(0, "%R/wikiedit");
    cgi_printf("<div>\n");
    mimetype_option_menu(zMimetype);
    cgi_printf("<br /><textarea name=\"w\" class=\"wikiedit\" cols=\"80\" \n"
           " rows=\"%d\" wrap=\"virtual\">%h</textarea>\n"
           "<br />\n",(n),(zBody));
    if( db_get_boolean("wysiwyg-wiki", 0) ){
      cgi_printf("<input type=\"submit\" name=\"edit-wysiwyg\" value=\"Wysiwyg Editor\"\n"
             " onclick='return confirm(\"Switching to WYSIWYG-mode\\nwill erase your markup\\nedits. Continue?\")' />\n");
    }
    cgi_printf("<input type=\"submit\" name=\"preview\" value=\"Preview Your Changes\" />\n");
  }else{
    /* Wysiwyg editing */
    Blob html, temp;
    form_begin("onsubmit='wysiwygSubmit()'", "%R/wikiedit");
    cgi_printf("<div>\n"
           "<input type=\"hidden\" name=\"wysiwyg\" value=\"1\" />\n");
    blob_zero(&temp);
    wiki_convert(&wiki, &temp, 0);
    blob_zero(&html);
    htmlTidy(blob_str(&temp), &html);
    blob_reset(&temp);
    wysiwygEditor("w", blob_str(&html), 60, n);
    blob_reset(&html);
    cgi_printf("<br />\n"
           "<input type=\"submit\" name=\"edit-markup\" value=\"Markup Editor\"\n"
           " onclick='return confirm(\"Switching to markup-mode\\nwill erase your WYSIWYG\\nedits. Continue?\")' />\n");
  }
  login_insert_csrf_secret();
  cgi_printf("<input type=\"submit\" name=\"submit\" value=\"Apply These Changes\" />\n"
         "<input type=\"hidden\" name=\"name\" value=\"%h\" />\n"
         "<input type=\"submit\" name=\"cancel\" value=\"Cancel\"\n"
         " onclick='confirm(\"Abandon your changes?\")' />\n"
         "</div>\n",(zPageName));
  captcha_generate(0);
  cgi_printf("</form>\n");
  manifest_destroy(pWiki);
  blob_reset(&wiki);
  style_footer();
}

/*
** WEBPAGE: wikinew
** URL /wikinew
**
** Prompt the user to enter the name of a new wiki page.  Then redirect
** to the wikiedit screen for that new page.
*/
void wikinew_page(void){
  const char *zName;
  const char *zMimetype;
  login_check_credentials();
  if( !g.perm.NewWiki ){
    login_needed();
    return;
  }  
  zName = PD("name","");
  zMimetype = wiki_filter_mimetypes(P("mimetype"));
  if( zName[0] && wiki_name_is_wellformed((const unsigned char *)zName) ){
    if( fossil_strcmp(zMimetype,"text/x-fossil-wiki")==0
     && db_get_boolean("wysiwyg-wiki", 0)
    ){
      cgi_redirectf("wikiedit?name=%T&wysiwyg=1", zName);
    }else{
      cgi_redirectf("wikiedit?name=%T&mimetype=%s", zName, zMimetype);
    }
  }
  style_header("Create A New Wiki Page");
  cgi_printf("<p>Rules for wiki page names:</p>\n");
  well_formed_wiki_name_rules();
  form_begin(0, "%R/wikinew");
  cgi_printf("<p>Name of new wiki page:\n"
         "<input style=\"width: 35;\" type=\"text\" name=\"name\" value=\"%h\" /><br />\n",(zName));
  mimetype_option_menu("text/x-fossil-wiki");
  cgi_printf("<br /><input type=\"submit\" value=\"Create\" />\n"
         "</p></form>\n");
  if( zName[0] ){
    cgi_printf("<p><span class=\"wikiError\">\n"
           "\"%h\" is not a valid wiki page name!</span></p>\n",(zName));
  }
  style_footer();
}


/*
** Append the wiki text for an remark to the end of the given BLOB.
*/
static void appendRemark(Blob *p, const char *zMimetype){
  char *zDate;
  const char *zUser;
  const char *zRemark;
  char *zId;

  zDate = db_text(0, "SELECT datetime('now')");
  zRemark = PD("r","");
  zUser = PD("u",g.zLogin);
  if( fossil_strcmp(zMimetype, "text/x-fossil-wiki")==0 ){
    zId = db_text(0, "SELECT lower(hex(randomblob(8)))");
    blob_appendf(p, "\n\n<hr><div id=\"%s\"><i>On %s UTC %h", 
      zId, zDate, g.zLogin);
    if( zUser[0] && fossil_strcmp(zUser,g.zLogin) ){
      blob_appendf(p, " (claiming to be %h)", zUser);
    }
    blob_appendf(p, " added:</i><br />\n%s</div id=\"%s\">", zRemark, zId);
  }else if( fossil_strcmp(zMimetype, "text/x-markdown")==0 ){
    blob_appendf(p, "\n\n------\n*On %s UTC %h", zDate, g.zLogin);
    if( zUser[0] && fossil_strcmp(zUser,g.zLogin) ){
      blob_appendf(p, " (claiming to be %h)", zUser);
    }
    blob_appendf(p, " added:*\n\n%s\n", zRemark);
  }else{
    blob_appendf(p, "\n\n------------------------------------------------\n"
                    "On %s UTC %s", zDate, g.zLogin);
    if( zUser[0] && fossil_strcmp(zUser,g.zLogin) ){
      blob_appendf(p, " (claiming to be %s)", zUser);
    }
    blob_appendf(p, " added:\n\n%s\n", zRemark);
  }
  fossil_free(zDate);
}

/*
** WEBPAGE: wikiappend
** URL: /wikiappend?name=PAGENAME&mimetype=MIMETYPE
*/
void wikiappend_page(void){
  char *zTag;
  int rid = 0;
  int isSandbox;
  const char *zPageName;
  const char *zUser;
  const char *zMimetype;
  int goodCaptcha = 1;
  const char *zFormat;

  login_check_credentials();
  zPageName = PD("name","");
  zMimetype = wiki_filter_mimetypes(P("mimetype"));
  if( check_name(zPageName) ) return;
  isSandbox = is_sandbox(zPageName);
  if( !isSandbox ){
    zTag = mprintf("wiki-%s", zPageName);
    rid = db_int(0, 
      "SELECT rid FROM tagxref"
      " WHERE tagid=(SELECT tagid FROM tag WHERE tagname=%Q)"
      " ORDER BY mtime DESC", zTag
    );
    free(zTag);
    if( !rid ){
      fossil_redirect_home();
      return;
    }
  }
  if( !g.perm.ApndWiki ){
    login_needed();
    return;
  }
  if( P("submit")!=0 && P("r")!=0 && P("u")!=0
   && (goodCaptcha = captcha_is_correct())
  ){
    char *zDate;
    Blob cksum;
    Blob body;
    Blob wiki;
    Manifest *pWiki = 0;

    blob_zero(&body);
    if( isSandbox ){
      blob_appendf(&body, db_get("sandbox",""));
      appendRemark(&body, zMimetype);
      db_set("sandbox", blob_str(&body), 0);
    }else{
      login_verify_csrf_secret();
      pWiki = manifest_get(rid, CFTYPE_WIKI, 0);
      if( pWiki ){
        blob_append(&body, pWiki->zWiki, -1);
        manifest_destroy(pWiki);
      }
      blob_zero(&wiki);
      db_begin_transaction();
      zDate = date_in_standard_format("now");
      blob_appendf(&wiki, "D %s\n", zDate);
      blob_appendf(&wiki, "L %F\n", zPageName);
      if( fossil_strcmp(zMimetype, "text/x-fossil-wiki")!=0 ){
        blob_appendf(&wiki, "N %s\n", zMimetype);
      }
      if( rid ){
        char *zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
        blob_appendf(&wiki, "P %s\n", zUuid);
        free(zUuid);
      }
      if( g.zLogin ){
        blob_appendf(&wiki, "U %F\n", g.zLogin);
      }
      appendRemark(&body, zMimetype);
      blob_appendf(&wiki, "W %d\n%s\n", blob_size(&body), blob_str(&body));
      md5sum_blob(&wiki, &cksum);
      blob_appendf(&wiki, "Z %b\n", &cksum);
      blob_reset(&cksum);
      wiki_put(&wiki, rid);
      db_end_transaction(0);
    }
    cgi_redirectf("wiki?name=%T", zPageName);
  }
  if( P("cancel")!=0 ){
    cgi_redirectf("wiki?name=%T", zPageName);
    return;
  }
  style_set_current_page("%s?name=%T", g.zPath, zPageName);
  style_header("Append Comment To: %s", zPageName);
  if( !goodCaptcha ){
    cgi_printf("<p class=\"generalError\">Error: Incorrect security code.</p>\n");
  }
  if( P("preview")!=0 ){
    Blob preview;
    blob_zero(&preview);
    appendRemark(&preview, zMimetype);
    cgi_printf("Preview:<hr>\n");
    wiki_render_by_mimetype(&preview, zMimetype);
    cgi_printf("<hr>\n");
    blob_reset(&preview);
  }
  zUser = PD("u", g.zLogin);
  form_begin(0, "%R/wikiappend");
  login_insert_csrf_secret();
  cgi_printf("<input type=\"hidden\" name=\"name\" value=\"%h\" />\n"
         "<input type=\"hidden\" name=\"mimetype\" value=\"%h\" />\n"
         "Your Name:\n"
         "<input type=\"text\" name=\"u\" size=\"20\" value=\"%h\" /><br />\n",(zPageName),(zMimetype),(zUser));
  zFormat = mimetype_common_name(zMimetype);
  cgi_printf("Comment to append (formatted as %s):<br />\n"
         "<textarea name=\"r\" class=\"wikiedit\" cols=\"80\" \n"
         " rows=\"10\" wrap=\"virtual\">%h</textarea>\n"
         "<br />\n"
         "<input type=\"submit\" name=\"preview\" value=\"Preview Your Comment\" />\n"
         "<input type=\"submit\" name=\"submit\" value=\"Append Your Changes\" />\n"
         "<input type=\"submit\" name=\"cancel\" value=\"Cancel\" />\n",(zFormat),(PD("r","")));
  captcha_generate(0);
  cgi_printf("</form>\n");
  style_footer();
}

/*
** Name of the wiki history page being generated
*/
static const char *zWikiPageName;

/*
** Function called to output extra text at the end of each line in
** a wiki history listing.
*/
static void wiki_history_extra(int rid){
  if( db_exists("SELECT 1 FROM tagxref WHERE rid=%d", rid) ){
    cgi_printf("%z[diff]</a>\n",(href("%R/wdiff?name=%t&a=%d",zWikiPageName,rid)));
  }
}

/*
** WEBPAGE: whistory
** URL: /whistory?name=PAGENAME
**
** Show the complete change history for a single wiki page.
*/
void whistory_page(void){
  Stmt q;
  char *zTitle;
  char *zSQL;
  const char *zPageName;
  login_check_credentials();
  if( !g.perm.Hyperlink ){ login_needed(); return; }
  zPageName = PD("name","");
  zTitle = mprintf("History Of %s", zPageName);
  style_header(zTitle);
  free(zTitle);

  zSQL = mprintf("%s AND event.objid IN "
                 "  (SELECT rid FROM tagxref WHERE tagid="
                       "(SELECT tagid FROM tag WHERE tagname='wiki-%q')"
                 "   UNION SELECT attachid FROM attachment"
                          " WHERE target=%Q)"
                 "ORDER BY mtime DESC",
                 timeline_query_for_www(), zPageName, zPageName);
  db_prepare(&q, zSQL);
  free(zSQL);
  zWikiPageName = zPageName;
  www_print_timeline(&q, TIMELINE_ARTID, 0, 0, wiki_history_extra);
  db_finalize(&q);
  style_footer();
}

/*
** WEBPAGE: wdiff
** URL: /whistory?name=PAGENAME&a=RID1&b=RID2
**
** Show the difference between two wiki pages.
*/
void wdiff_page(void){
  char *zTitle;
  int rid1, rid2;
  const char *zPageName;
  Manifest *pW1, *pW2 = 0;
  Blob w1, w2, d;
  u64 diffFlags;

  login_check_credentials();
  rid1 = atoi(PD("a","0"));
  if( !g.perm.Hyperlink ){ login_needed(); return; }
  if( rid1==0 ) fossil_redirect_home();
  rid2 = atoi(PD("b","0"));
  zPageName = PD("name","");
  zTitle = mprintf("Changes To %s", zPageName);
  style_header(zTitle);
  free(zTitle);

  if( rid2==0 ){
    rid2 = db_int(0,
      "SELECT objid FROM event JOIN tagxref ON objid=rid AND tagxref.tagid="
                        "(SELECT tagid FROM tag WHERE tagname='wiki-%q')"
      " WHERE event.mtime<(SELECT mtime FROM event WHERE objid=%d)"
      " ORDER BY event.mtime DESC LIMIT 1",
      zPageName, rid1
    );
  }
  pW1 = manifest_get(rid1, CFTYPE_WIKI, 0);
  if( pW1==0 ) fossil_redirect_home();
  blob_init(&w1, pW1->zWiki, -1);
  blob_zero(&w2);
  if( rid2 && (pW2 = manifest_get(rid2, CFTYPE_WIKI, 0))!=0 ){
    blob_init(&w2, pW2->zWiki, -1);
  }
  blob_zero(&d);
  diffFlags = construct_diff_flags(1,0);
  text_diff(&w2, &w1, &d, 0, diffFlags | DIFF_HTML | DIFF_LINENO);
  cgi_printf("<pre class=\"udiff\">\n"
         "%s\n"
         "<pre>\n",(blob_str(&d)));
  manifest_destroy(pW1);
  manifest_destroy(pW2);
  style_footer();
}

/*
** prepare()s pStmt with a query requesting:
**
** - wiki page name
** - tagxref (whatever that really is!)
**
** Used by wcontent_page() and the JSON wiki code.
*/
void wiki_prepare_page_list( Stmt * pStmt ){
  db_prepare(pStmt, 
    "SELECT"
    "  substr(tagname, 6) as name,"
    "  (SELECT value FROM tagxref WHERE tagid=tag.tagid ORDER BY mtime DESC) as tagXref"
    "  FROM tag WHERE tagname GLOB 'wiki-*'"
    " ORDER BY lower(tagname) /*sort*/"
  );
}
/*
** WEBPAGE: wcontent
**
**     all=1         Show deleted pages
**
** List all available wiki pages with date created and last modified.
*/
void wcontent_page(void){
  Stmt q;
  int showAll = P("all")!=0;

  login_check_credentials();
  if( !g.perm.RdWiki ){ login_needed(); return; }
  style_header("Available Wiki Pages");
  if( showAll ){
    style_submenu_element("Active", "Only Active Pages", "%s/wcontent", g.zTop);
  }else{
    style_submenu_element("All", "All", "%s/wcontent?all=1", g.zTop);
  }
  cgi_printf("<ul>\n");
  wiki_prepare_page_list(&q);
  while( db_step(&q)==SQLITE_ROW ){
    const char *zName = db_column_text(&q, 0);
    int size = db_column_int(&q, 1);
    if( size>0 ){
      cgi_printf("<li>%z%h</a></li>\n",(href("%R/wiki?name=%T",zName)),(zName));
    }else if( showAll ){
      cgi_printf("<li>%z<s>%h</s></a></li>\n",(href("%R/wiki?name=%T",zName)),(zName));
    }
  }
  db_finalize(&q);
  cgi_printf("</ul>\n");
  style_footer();
}

/*
** WEBPAGE: wfind
**
** URL: /wfind?title=TITLE
** List all wiki pages whose titles contain the search text
*/
void wfind_page(void){
  Stmt q;
  const char * zTitle;
  login_check_credentials();
  if( !g.perm.RdWiki ){ login_needed(); return; }
  zTitle = PD("title","*");
  style_header("Wiki Pages Found");
  cgi_printf("<ul>\n");
  db_prepare(&q, 
    "SELECT substr(tagname, 6, 1000) FROM tag WHERE tagname like 'wiki-%%%q%%'"
    " ORDER BY lower(tagname) /*sort*/" ,
    zTitle);
  while( db_step(&q)==SQLITE_ROW ){
    const char *zName = db_column_text(&q, 0);
    cgi_printf("<li>%z%h</a></li>\n",(href("%R/wiki?name=%T",zName)),(zName));
  }
  db_finalize(&q);
  cgi_printf("</ul>\n");
  style_footer();
}

/*
** WEBPAGE: wiki_rules
*/
void wikirules_page(void){
  style_header("Wiki Formatting Rules");
  cgi_printf("<h2>Formatting Rule Summary</h2>\n"
         "<ol>\n"
         "<li>Blank lines are paragraph breaks</li>\n"
         "<li>Bullets are \"*\" surrounded by two spaces at the beginning of the\n"
         "line.</li>\n"
         "<li>Enumeration items are \"#\" surrounded by two spaces at the beginning of\n"
         "a line.</li>\n"
         "<li>Indented pargraphs begin with a tab or two spaces.</li>\n"
         "<li>Hyperlinks are contained with square brackets:  \"[target]\" or\n"
         "\"[target|name]\".</li>\n"
         "<li>Most ordinary HTML works.</li>\n"
         "<li>&lt;verbatim&gt; and &lt;nowiki&gt;.</li>\n"
         "</ol>\n"
         "<p>We call the first five rules above \"wiki\" formatting rules.  The\n"
         "last two rules are the HTML formatting rule.</p>\n"
         "<h2>Formatting Rule Details</h2>\n"
         "<ol>\n"
         "<li> <p><span class=\"wikiruleHead\">Paragraphs</span>.  Any sequence of one or more blank lines forms\n"
         "a paragraph break.  Centered or right-justified paragraphs are not\n"
         "supported by wiki markup, but you can do these things if you need them\n"
         "using HTML.</p></li>\n"
         "<li> <p><span class=\"wikiruleHead\">Bullet Lists</span>.\n"
         "A bullet list item is a line that begins with a single \"*\" character\n"
         "surrounded on\n"
         "both sides by two or more spaces or by a tab.  Only a single level\n"
         "of bullet list is supported by wiki.  For nested lists, use HTML.</p></li>\n"
         "<li> <p><span class=\"wikiruleHead\">Enumeration Lists</span>.\n"
         "An enumeration list item is a line that begins with a single \"#\" character\n"
         "surrounded on both sides by two or more spaces or by a tab.  Only a single\n"
         "level of enumeration list is supported by wiki.  For nested lists or for\n"
         "enumerations that count using letters or roman numerials, use HTML.</p></li>\n"
         "<li> <p><span class=\"wikiruleHead\">Indented Paragraphs</span>.\n"
         "Any paragraph that begins with two or more spaces or a tab and\n"
         "which is not a bullet or enumeration list item is rendered \n"
         "indented.  Only a single level of indentation is supported by wiki; use\n"
         "HTML for deeper indentation.</p></li>\n"
         "<li> <p><span class=\"wikiruleHead\">Hyperlinks</span>.\n"
         "Text within square brackets (\"[...]\") becomes a hyperlink.  The\n"
         "target can be a wiki page name, the artifact ID of a check-in or ticket,\n"
         "the name of an image, or a URL.  By default, the target is displayed\n"
         "as the text of the hyperlink.  But you can specify alternative text\n"
         "after the target name separated by a \"|\" character.</p>\n"
         "<p>You can also link to internal anchor names using [#anchor-name], providing\n"
         "you have added the necessary \"&lt;a name=\"anchor-name\"&gt;&lt;/a&gt;\"\n"
         "tag to your wiki page.</p></li>\n"
         "<li> <p><span class=\"wikiruleHead\">HTML</span>.\n"
         "The following standard HTML elements may be used:\n");
  show_allowed_wiki_markup();
  cgi_printf(". There are two non-standard elements available:\n"
         "&lt;verbatim&gt; and &lt;nowiki&gt;.\n"
         "No other elements are allowed.  All attributes are checked and\n"
         "only a few benign attributes are allowed on each element.\n"
         "In particular, any attributes that specify javascript or CSS\n"
         "are elided.</p></li>\n"
         "<li><p><span class=\"wikiruleHead\">Special Markup.</span>\n"
         "The &lt;nowiki&gt; tag disables all wiki formatting rules\n"
         "through the matching &lt;/nowiki&gt; element.\n"
         "The &lt;verbatim&gt; tag works like &lt;pre&gt; with the addition\n"
         "that it also disables all wiki and HTML markup\n"
         "through the matching &lt;/verbatim&gt;.</p></li>\n"
         "</ol>\n");
  style_footer();
}

/*
** Add a new wiki page to the repository.  The page name is
** given by the zPageName parameter.  isNew must be true to create
** a new page.  If no previous page with the name zPageName exists
** and isNew is false, then this routine throws an error.
**
** The content of the new page is given by the blob pContent.
*/
int wiki_cmd_commit(char const * zPageName, int isNew, Blob *pContent){
  Blob wiki;              /* Wiki page content */
  Blob cksum;             /* wiki checksum */
  int rid;                /* artifact ID of parent page */
  char *zDate;            /* timestamp */
  char *zUuid;            /* uuid for rid */

  rid = db_int(0,
     "SELECT x.rid FROM tag t, tagxref x"
     " WHERE x.tagid=t.tagid AND t.tagname='wiki-%q'"
     " ORDER BY x.mtime DESC LIMIT 1",
     zPageName
  );
  if( rid==0 && !isNew ){
#ifdef FOSSIL_ENABLE_JSON
    g.json.resultCode = FSL_JSON_E_RESOURCE_NOT_FOUND;
#endif
    fossil_fatal("no such wiki page: %s", zPageName);
  }
  if( rid!=0 && isNew ){
#ifdef FOSSIL_ENABLE_JSON
    g.json.resultCode = FSL_JSON_E_RESOURCE_ALREADY_EXISTS;
#endif
    fossil_fatal("wiki page %s already exists", zPageName);
  }

  blob_zero(&wiki);
  zDate = date_in_standard_format("now");
  blob_appendf(&wiki, "D %s\n", zDate);
  free(zDate);
  blob_appendf(&wiki, "L %F\n", zPageName );
  if( rid ){
    zUuid = db_text(0, "SELECT uuid FROM blob WHERE rid=%d", rid);
    blob_appendf(&wiki, "P %s\n", zUuid);
    free(zUuid);
  }
  user_select();
  if( g.zLogin ){
      blob_appendf(&wiki, "U %F\n", g.zLogin);
  }
  blob_appendf( &wiki, "W %d\n%s\n", blob_size(pContent),
                blob_str(pContent) );
  md5sum_blob(&wiki, &cksum);
  blob_appendf(&wiki, "Z %b\n", &cksum);
  blob_reset(&cksum);
  db_begin_transaction();
  wiki_put(&wiki, 0);
  db_end_transaction(0);
  return 1;
}

/*
** COMMAND: wiki*
**
** Usage: %fossil wiki (export|create|commit|list) WikiName
**
** Run various subcommands to work with wiki entries.
**
**     %fossil wiki export PAGENAME ?FILE?
**
**        Sends the latest version of the PAGENAME wiki
**        entry to the given file or standard output.
**
**     %fossil wiki commit PAGENAME ?FILE?
**
**        Commit changes to a wiki page from FILE or from standard
**        input.
**
**     %fossil wiki create PAGENAME ?FILE?
**
**        Create a new wiki page with initial content taken from
**        FILE or from standard input.
**
**     %fossil wiki list
**
**        Lists all wiki entries, one per line, ordered
**        case-insensitively by name.
**
*/
void wiki_cmd(void){
  int n;
  db_find_and_open_repository(0, 0);
  if( g.argc<3 ){
    goto wiki_cmd_usage;
  }
  n = strlen(g.argv[2]);
  if( n==0 ){
    goto wiki_cmd_usage;
  }

  if( strncmp(g.argv[2],"export",n)==0 ){
    char const *zPageName;        /* Name of the wiki page to export */
    char const *zFile;            /* Name of the output file (0=stdout) */
    int rid;                      /* Artifact ID of the wiki page */
    int i;                        /* Loop counter */
    char *zBody = 0;              /* Wiki page content */
    Blob body;                    /* Wiki page content */
    Manifest *pWiki = 0;          /* Parsed wiki page content */

    if( (g.argc!=4) && (g.argc!=5) ){
      usage("export PAGENAME ?FILE?");
    }
    zPageName = g.argv[3];
    rid = db_int(0, "SELECT x.rid FROM tag t, tagxref x"
      " WHERE x.tagid=t.tagid AND t.tagname='wiki-%q'"
      " ORDER BY x.mtime DESC LIMIT 1",
      zPageName 
    );
    if( (pWiki = manifest_get(rid, CFTYPE_WIKI, 0))!=0 ){
      zBody = pWiki->zWiki;
    }
    if( zBody==0 ){
      fossil_fatal("wiki page [%s] not found",zPageName);
    }
    for(i=strlen(zBody); i>0 && fossil_isspace(zBody[i-1]); i--){}
    zBody[i] = 0;
    zFile  = (g.argc==4) ? "-" : g.argv[4];
    blob_init(&body, zBody, -1);
    blob_append(&body, "\n", 1);
    blob_write_to_file(&body, zFile);
    blob_reset(&body);
    manifest_destroy(pWiki);
    return;
  }else
  if( strncmp(g.argv[2],"commit",n)==0
      || strncmp(g.argv[2],"create",n)==0 ){
    char *zPageName;
    Blob content;
    if( g.argc!=4 && g.argc!=5 ){
      usage("commit PAGENAME ?FILE?");
    }
    zPageName = g.argv[3];
    if( g.argc==4 ){
      blob_read_from_channel(&content, stdin, -1);
    }else{
      blob_read_from_file(&content, g.argv[4]);
    }
    if( g.argv[2][1]=='r' ){
      wiki_cmd_commit(zPageName, 1, &content);
      fossil_print("Created new wiki page %s.\n", zPageName);
    }else{
      wiki_cmd_commit(zPageName, 0, &content);
      fossil_print("Updated wiki page %s.\n", zPageName);
    }
    blob_reset(&content);
  }else
  if( strncmp(g.argv[2],"delete",n)==0 ){
    if( g.argc!=5 ){
      usage("delete PAGENAME");
    }
    fossil_fatal("delete not yet implemented.");
  }else
  if( strncmp(g.argv[2],"list",n)==0 ){
    Stmt q;
    db_prepare(&q, 
      "SELECT substr(tagname, 6) FROM tag WHERE tagname GLOB 'wiki-*'"
      " ORDER BY lower(tagname) /*sort*/"
    );
    while( db_step(&q)==SQLITE_ROW ){
      const char *zName = db_column_text(&q, 0);
      fossil_print( "%s\n",zName);
    }
    db_finalize(&q);
  }else
  {
    goto wiki_cmd_usage;
  }
  return;

wiki_cmd_usage:
  usage("export|create|commit|list ...");
}
