#line 1 "./src/style.c"
/*
** Copyright (c) 2006,2007 D. Richard Hipp
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
** This file contains code to implement the basic web page look and feel.
**
*/
#include "config.h"
#include "style.h"


/*
** Elements of the submenu are collected into the following
** structure and displayed below the main menu by style_header().
**
** Populate this structure with calls to style_submenu_element()
** prior to calling style_header().
*/
static struct Submenu {
  const char *zLabel;
  const char *zTitle;
  const char *zLink;
} aSubmenu[30];
static int nSubmenu = 0;

/*
** Remember that the header has been generated.  The footer is omitted
** if an error occurs before the header.
*/
static int headerHasBeenGenerated = 0;

/*
** remember, if a sidebox was used
*/
static int sideboxUsed = 0;


/*
** List of hyperlinks and forms that need to be resolved by javascript in
** the footer.
*/
char **aHref = 0;
int nHref = 0;
int nHrefAlloc = 0;
char **aFormAction = 0;
int nFormAction = 0;

/*
** Generate and return a anchor tag like this:
**
**        <a href="URL">
**  or    <a id="ID">
**
** The form of the anchor tag is determined by the g.javascriptHyperlink
** variable.  The href="URL" form is used if g.javascriptHyperlink is false.
** If g.javascriptHyperlink is true then the
** id="ID" form is used and javascript is generated in the footer to cause
** href values to be inserted after the page has loaded.  If
** g.perm.History is false, then the <a id="ID"> form is still
** generated but the javascript is not generated so the links never
** activate.
**
** If the user lacks the Hyperlink (h) property and the "auto-hyperlink"
** setting is true, then g.perm.Hyperlink is changed from 0 to 1 and
** g.javascriptHyperlink is set to 1.  The g.javascriptHyperlink defaults
** to 0 and only changes to one if the user lacks the Hyperlink (h) property
** and the "auto-hyperlink" setting is enabled.
**
** Filling in the href="URL" using javascript is a defense against bots.
**
** The name of this routine is deliberately kept short so that can be
** easily used within @-lines.  Example:
**
**      @ %z(href("%R/artifact/%s",zUuid))%h(zFN)</a>
**
** Note %z format.  The string returned by this function is always
** obtained from fossil_malloc() so rendering it with %z will reclaim
** that memory space.
**
** There are two versions of this routine: href() does a plain hyperlink
** and xhref() adds extra attribute text.
**
** g.perm.Hyperlink is true if the user has the Hyperlink (h) property.
** Most logged in users should have this property, since we can assume
** that a logged in user is not a bot.  Only "nobody" lacks g.perm.Hyperlink,
** typically.
*/
char *xhref(const char *zExtra, const char *zFormat, ...){
  char *zUrl;
  va_list ap;
  va_start(ap, zFormat);
  zUrl = vmprintf(zFormat, ap);
  va_end(ap);
  if( g.perm.Hyperlink && !g.javascriptHyperlink ){
    char *zHUrl = mprintf("<a %s href=\"%h\">", zExtra, zUrl);
    fossil_free(zUrl);
    return zHUrl;
  }
  if( nHref>=nHrefAlloc ){
    nHrefAlloc = nHrefAlloc*2 + 10;
    aHref = fossil_realloc(aHref, nHrefAlloc*sizeof(aHref[0]));
  }
  aHref[nHref++] = zUrl;
  return mprintf("<a %s id='a%d' href='%R/honeypot'>", zExtra, nHref);
}
char *href(const char *zFormat, ...){
  char *zUrl;
  va_list ap;
  va_start(ap, zFormat);
  zUrl = vmprintf(zFormat, ap);
  va_end(ap);
  if( g.perm.Hyperlink && !g.javascriptHyperlink ){
    char *zHUrl = mprintf("<a href=\"%h\">", zUrl);
    fossil_free(zUrl);
    return zHUrl;
  }
  if( nHref>=nHrefAlloc ){
    nHrefAlloc = nHrefAlloc*2 + 10;
    aHref = fossil_realloc(aHref, nHrefAlloc*sizeof(aHref[0]));
  }
  aHref[nHref++] = zUrl;
  return mprintf("<a id='a%d' href='%R/honeypot'>", nHref);
}

/*
** Generate <form method="post" action=ARG>.  The ARG value is inserted
** by javascript.
*/
void form_begin(const char *zOtherArgs, const char *zAction, ...){
  char *zLink;
  va_list ap;
  if( zOtherArgs==0 ) zOtherArgs = "";
  va_start(ap, zAction);
  zLink = vmprintf(zAction, ap);
  va_end(ap);
  if( g.perm.Hyperlink && !g.javascriptHyperlink ){
    cgi_printf("<form method=\"POST\" action=\"%z\" %s>\n",(zLink),(zOtherArgs));
  }else{
    int n;
    aFormAction = fossil_realloc(aFormAction, (nFormAction+1)*sizeof(char*));
    aFormAction[nFormAction++] = zLink;
    n = nFormAction;
    cgi_printf("<form id=\"form%d\" method=\"POST\" action='%R/login' %s>\n",(n),(zOtherArgs));
  }
}

/*
** Generate javascript that will set the href= attribute on all anchors.
*/
void style_resolve_href(void){
  int i;
  int nDelay = db_get_int("auto-hyperlink-delay",10);
  if( !g.perm.Hyperlink ) return;
  if( nHref==0 && nFormAction==0 ) return;
  cgi_printf("<script type=\"text/JavaScript\">\n"
         "/* <![CDATA[ */\n"
         "function setAllHrefs(){\n");
  if( g.javascriptHyperlink ){
    for(i=0; i<nHref; i++){
      cgi_printf("gebi(\"a%d\").href=\"%s\";\n",(i+1),(aHref[i]));
    }
  }
  for(i=0; i<nFormAction; i++){
    cgi_printf("gebi(\"form%d\").action=\"%s\";\n",(i+1),(aFormAction[i]));
  }
  cgi_printf("}\n");
  if( strglob("*Opera Mini/[1-9]*", P("HTTP_USER_AGENT")) ){
    /* Special case for Opera Mini, which executes JS server-side */
    cgi_printf("var isOperaMini = Object.prototype.toString.call(window.operamini)\n"
           "                  === \"[object OperaMini]\";\n"
           "if( isOperaMini ){\n"
           "  setTimeout(\"setAllHrefs();\",%d);\n"
           "}\n",(nDelay));
  }else if( db_get_boolean("auto-hyperlink-mouseover",0) ){
    /* Require mouse movement prior to activating hyperlinks */
    cgi_printf("document.getElementsByTagName(\"body\")[0].onmousemove=function(){\n"
           "  setTimeout(\"setAllHrefs();\",%d);\n"
           "  this.onmousemove = null;\n"
           "}\n",(nDelay));
  }else{
    /* Active hyperlinks right away */
    cgi_printf("setTimeout(\"setAllHrefs();\",%d);\n",(nDelay));
  }
  cgi_printf("/* ]]> */\n"
         "</script>\n");
}

/*
** Add a new element to the submenu
*/
void style_submenu_element(
  const char *zLabel,
  const char *zTitle,
  const char *zLink,
  ...
){
  va_list ap;
  assert( nSubmenu < sizeof(aSubmenu)/sizeof(aSubmenu[0]) );
  aSubmenu[nSubmenu].zLabel = zLabel;
  aSubmenu[nSubmenu].zTitle = zTitle;
  va_start(ap, zLink);
  aSubmenu[nSubmenu].zLink = vmprintf(zLink, ap);
  va_end(ap);
  nSubmenu++;
}

/*
** Compare two submenu items for sorting purposes
*/
static int submenuCompare(const void *a, const void *b){
  const struct Submenu *A = (const struct Submenu*)a;
  const struct Submenu *B = (const struct Submenu*)b;
  return fossil_strcmp(A->zLabel, B->zLabel);
}

/* Use this for the $current_page variable if it is not NULL.  If it is
** NULL then use g.zPath.
*/
static char *local_zCurrentPage = 0;

/*
** Set the desired $current_page to something other than g.zPath
*/
void style_set_current_page(const char *zFormat, ...){
  fossil_free(local_zCurrentPage);
  if( zFormat==0 ){
    local_zCurrentPage = 0;
  }else{
    va_list ap;
    va_start(ap, zFormat);
    local_zCurrentPage = vmprintf(zFormat, ap);
    va_end(ap);
  }
}

/*
** Create a TH1 variable containing the URL for the specified config resource.
** The resulting variable name will be of the form $[zVarPrefix]_url.
*/
static void url_var(
  const char *zVarPrefix,
  const char *zConfigName,
  const char *zPageName
){
  char *zMtime = db_get_mtime(zConfigName, 0, 0);
  char *zUrl = mprintf("%s/%s/%s%.5s", g.zTop, zPageName, zMtime,
                       MANIFEST_UUID);
  char *zVarName = mprintf("%s_url", zVarPrefix);
  Th_Store(zVarName, zUrl);
  free(zMtime);
  free(zUrl);
  free(zVarName);
}

/*
** Create a TH1 variable containing the URL for the specified config image.
** The resulting variable name will be of the form $[zImageName]_image_url.
*/
static void image_url_var(const char *zImageName){
  char *zVarPrefix = mprintf("%s_image", zImageName);
  char *zConfigName = mprintf("%s-image", zImageName);
  url_var(zVarPrefix, zConfigName, zImageName);
  free(zVarPrefix);
  free(zConfigName);
}

/*
** Draw the header.
*/
void style_header(const char *zTitleFormat, ...){
  va_list ap;
  char *zTitle;
  const char *zHeader = db_get("header", (char*)zDefaultHeader);
  login_check_credentials();

  va_start(ap, zTitleFormat);
  zTitle = vmprintf(zTitleFormat, ap);
  va_end(ap);

  cgi_destination(CGI_HEADER);

  cgi_printf("<!DOCTYPE html>\n");

  if( g.thTrace ) Th_Trace("BEGIN_HEADER<br />\n", -1);

  /* Generate the header up through the main menu */
  Th_Store("project_name", db_get("project-name","Unnamed Fossil Project"));
  Th_Store("title", zTitle);
  Th_Store("baseurl", g.zBaseURL);
  Th_Store("home", g.zTop);
  Th_Store("index_page", db_get("index-page","/home"));
  Th_Store("current_page", local_zCurrentPage ? local_zCurrentPage : g.zPath);
  Th_Store("csrf_token", g.zCsrfToken);
  Th_Store("release_version", RELEASE_VERSION);
  Th_Store("manifest_version", MANIFEST_VERSION);
  Th_Store("manifest_date", MANIFEST_DATE);
  Th_Store("compiler_name", COMPILER_NAME);
  url_var("stylesheet", "css", "style.css");
  image_url_var("logo");
  image_url_var("background");
  if( g.zLogin ){
    Th_Store("login", g.zLogin);
  }
  if( g.thTrace ) Th_Trace("BEGIN_HEADER_SCRIPT<br />\n", -1);
  Th_Render(zHeader);
  if( g.thTrace ) Th_Trace("END_HEADER<br />\n", -1);
  Th_Unstore("title");   /* Avoid collisions with ticket field names */
  cgi_destination(CGI_BODY);
  g.cgiOutput = 1;
  headerHasBeenGenerated = 1;
  sideboxUsed = 0;

  /* Make the gebi(x) function available as an almost-alias for
  ** document.getElementById(x) (except that it throws an error
  ** if the element is not found).
  **
  ** Maintenance note: this function must of course be available
  ** before it is called. It "should" go in the HEAD so that client
  ** HEAD code can make use of it, but because the client can replace
  ** the HEAD, and some fossil pages rely on gebi(), we put it here.
  */
  cgi_printf("<script>\n"
         "function gebi(x){\n"
         "if(/^#/.test(x)) x = x.substr(1);\n"
         "var e = document.getElementById(x);\n"
         "if(!e) throw new Error(\"Expecting element with ID \"+x);\n"
         "else return e;}\n"
         "</script>\n");
}

/*
** Append ad unit text if appropriate.
*/
static void style_ad_unit(void){
  const char *zAd;
  if( g.perm.Admin && db_get_boolean("adunit-omit-if-admin",0) ){
    return;
  }
  if( g.zLogin && strcmp(g.zLogin,"anonymous")!=0
      && db_get_boolean("adunit-omit-if-user",0) ){
    return;
  }
  zAd = db_get("adunit", 0);
  if( zAd ) cgi_append_content(zAd, -1);
}

/*
** Draw the footer at the bottom of the page.
*/
void style_footer(void){
  const char *zFooter;

  if( !headerHasBeenGenerated ) return;

  /* Go back and put the submenu at the top of the page.  We delay the
  ** creation of the submenu until the end so that we can add elements
  ** to the submenu while generating page text.
  */
  cgi_destination(CGI_HEADER);
  if( nSubmenu>0 ){
    int i;
    cgi_printf("<div class=\"submenu\">\n");
    qsort(aSubmenu, nSubmenu, sizeof(aSubmenu[0]), submenuCompare);
    for(i=0; i<nSubmenu; i++){
      struct Submenu *p = &aSubmenu[i];
      if( p->zLink==0 ){
        cgi_printf("<span class=\"label\">%h</span>\n",(p->zLabel));
      }else{
        cgi_printf("<a class=\"label\" href=\"%h\">%h</a>\n",(p->zLink),(p->zLabel));
      }
    }
    cgi_printf("</div>\n");
  }
  style_ad_unit();
  cgi_printf("<div class=\"content\">\n");
  cgi_destination(CGI_BODY);

  if (sideboxUsed) {
    /* Put the footer at the bottom of the page.
    ** the additional clear/both is needed to extend the content
    ** part to the end of an optional sidebox.
    */
    cgi_printf("<div class=\"endContent\"></div>\n");
  }
  cgi_printf("</div>\n");

  /* Set the href= field on hyperlinks.  Do this before the footer since
  ** the footer will be generating </html> */
  style_resolve_href();

  zFooter = db_get("footer", (char*)zDefaultFooter);
  if( g.thTrace ) Th_Trace("BEGIN_FOOTER<br />\n", -1);
  Th_Render(zFooter);
  if( g.thTrace ) Th_Trace("END_FOOTER<br />\n", -1);

  /* Render trace log if TH1 tracing is enabled. */
  if( g.thTrace ){
    cgi_append_content("<span class=\"thTrace\"><hr />\n", -1);
    cgi_append_content(blob_str(&g.thLog), blob_size(&g.thLog));
    cgi_append_content("</span>\n", -1);
  }
}

/*
** Begin a side-box on the right-hand side of a page.  The title and
** the width of the box are given as arguments.  The width is usually
** a percentage of total screen width.
*/
void style_sidebox_begin(const char *zTitle, const char *zWidth){
  sideboxUsed = 1;
  cgi_printf("<div class=\"sidebox\" style=\"width:%s\">\n"
         "<div class=\"sideboxTitle\">%h</div>\n",(zWidth),(zTitle));
}

/* End the side-box
*/
void style_sidebox_end(void){
  cgi_printf("</div>\n");
}

/* @-comment: // */
/*
** The default page header.
*/
const char zDefaultHeader[] =
"<html>\n"
"<head>\n"
"<base href=\"$baseurl/$current_page\" />\n"
"<title>$<project_name>: $<title></title>\n"
"<link rel=\"alternate\" type=\"application/rss+xml\" title=\"RSS Feed\"\n"
"      href=\"$home/timeline.rss\" />\n"
"<link rel=\"stylesheet\" href=\"$stylesheet_url\" type=\"text/css\"\n"
"      media=\"screen\" />\n"
"</head>\n"
"<body>\n"
"<div class=\"header\">\n"
"  <div class=\"logo\">\n"
"    <img src=\"$logo_image_url\" alt=\"logo\" />\n"
"  </div>\n"
"  <div class=\"title\"><small>$<project_name></small><br />$<title></div>\n"
"  <div class=\"status\"><th1>\n"
"     if {[info exists login]} {\n"
"       puts \"Logged in as $login\"\n"
"     } else {\n"
"       puts \"Not logged in\"\n"
"     }\n"
"  </th1></div>\n"
"</div>\n"
"<div class=\"mainmenu\">\n"
"<th1>\n"
"html \"<a href='$home$index_page'>Home</a>\\n\"\n"
"if {[anycap jor]} {\n"
"  html \"<a href='$home/timeline'>Timeline</a>\\n\"\n"
"}\n"
"if {[hascap oh]} {\n"
"  html \"<a href='$home/tree?ci=tip'>Files</a>\\n\"\n"
"}\n"
"if {[hascap o]} {\n"
"  html \"<a href='$home/brlist'>Branches</a>\\n\"\n"
"  html \"<a href='$home/taglist'>Tags</a>\\n\"\n"
"}\n"
"if {[hascap r]} {\n"
"  html \"<a href='$home/reportlist'>Tickets</a>\\n\"\n"
"}\n"
"if {[hascap j]} {\n"
"  html \"<a href='$home/wiki'>Wiki</a>\\n\"\n"
"}\n"
"if {[hascap s]} {\n"
"  html \"<a href='$home/setup'>Admin</a>\\n\"\n"
"} elseif {[hascap a]} {\n"
"  html \"<a href='$home/setup_ulist'>Users</a>\\n\"\n"
"}\n"
"if {[info exists login]} {\n"
"  html \"<a href='$home/login'>Logout</a>\\n\"\n"
"} else {\n"
"  html \"<a href='$home/login'>Login</a>\\n\"\n"
"}\n"
"</th1></div>\n"
;

/*
** The default page footer
*/
const char zDefaultFooter[] =
"<div class=\"footer\">\n"
"This page was generated in about\n"
"<th1>puts [expr {([utime]+[stime]+1000)/1000*0.001}]</th1>s by\n"
"Fossil version $manifest_version $manifest_date\n"
"</div>\n"
"</body></html>\n"
;

/*
** The default Cascading Style Sheet.
** It's assembled by different strings for each class.
** The default css contains all definitions.
** The style sheet, send to the client only contains the ones,
** not defined in the user defined css.
*/
const char zDefaultCSS[] =
"/* General settings for the entire page */\n"
"body {\n"
"  margin: 0ex 1ex;\n"
"  padding: 0px;\n"
"  background-color: white;\n"
"  font-family: sans-serif;\n"
"}\n"
"\n"
"/* The project logo in the upper left-hand corner of each page */\n"
"div.logo {\n"
"  display: table-cell;\n"
"  text-align: center;\n"
"  vertical-align: bottom;\n"
"  font-weight: bold;\n"
"  color: #558195;\n"
"  min-width: 200px;\n"
"  white-space: nowrap;\n"
"}\n"
"\n"
"/* The page title centered at the top of each page */\n"
"div.title {\n"
"  display: table-cell;\n"
"  font-size: 2em;\n"
"  font-weight: bold;\n"
"  text-align: center;\n"
"  padding: 0 0 0 1em;\n"
"  color: #558195;\n"
"  vertical-align: bottom;\n"
"  width: 100%;\n"
"}\n"
"\n"
"/* The login status message in the top right-hand corner */\n"
"div.status {\n"
"  display: table-cell;\n"
"  text-align: right;\n"
"  vertical-align: bottom;\n"
"  color: #558195;\n"
"  font-size: 0.8em;\n"
"  font-weight: bold;\n"
"  min-width: 200px;\n"
"  white-space: nowrap;\n"
"}\n"
"\n"
"/* The header across the top of the page */\n"
"div.header {\n"
"  display: table;\n"
"  width: 100%;\n"
"}\n"
"\n"
"/* The main menu bar that appears at the top of the page beneath\n"
"** the header */\n"
"div.mainmenu {\n"
"  padding: 5px 10px 5px 10px;\n"
"  font-size: 0.9em;\n"
"  font-weight: bold;\n"
"  text-align: center;\n"
"  letter-spacing: 1px;\n"
"  background-color: #558195;\n"
"  border-top-left-radius: 8px;\n"
"  border-top-right-radius: 8px;\n"
"  color: white;\n"
"}\n"
"\n"
"/* The submenu bar that *sometimes* appears below the main menu */\n"
"div.submenu, div.sectionmenu {\n"
"  padding: 3px 10px 3px 0px;\n"
"  font-size: 0.9em;\n"
"  text-align: center;\n"
"  background-color: #456878;\n"
"  color: white;\n"
"}\n"
"div.mainmenu a, div.mainmenu a:visited, div.submenu a, div.submenu a:visited,\n"
"div.sectionmenu>a.button:link, div.sectionmenu>a.button:visited {\n"
"  padding: 3px 10px 3px 10px;\n"
"  color: white;\n"
"  text-decoration: none;\n"
"}\n"
"div.mainmenu a:hover, div.submenu a:hover, div.sectionmenu>a.button:hover {\n"
"  color: #558195;\n"
"  background-color: white;\n"
"}\n"
"\n"
"/* All page content from the bottom of the menu or submenu down to\n"
"** the footer */\n"
"div.content {\n"
"  padding: 0ex 1ex 1ex 1ex;\n"
"  border: solid #aaa;\n"
"  border-width: 1px;\n"
"}\n"
"\n"
"/* Some pages have section dividers */\n"
"div.section {\n"
"  margin-bottom: 0px;\n"
"  margin-top: 1em;\n"
"  padding: 1px 1px 1px 1px;\n"
"  font-size: 1.2em;\n"
"  font-weight: bold;\n"
"  background-color: #558195;\n"
"  color: white;\n"
"  white-space: nowrap;\n"
"}\n"
"\n"
"/* The \"Date\" that occurs on the left hand side of timelines */\n"
"div.divider {\n"
"  background: #a1c4d4;\n"
"  border: 2px #558195 solid;\n"
"  font-size: 1em; font-weight: normal;\n"
"  padding: .25em;\n"
"  margin: .2em 0 .2em 0;\n"
"  float: left;\n"
"  clear: left;\n"
"  white-space: nowrap;\n"
"}\n"
"\n"
"/* The footer at the very bottom of the page */\n"
"div.footer {\n"
"  clear: both;\n"
"  font-size: 0.8em;\n"
"  padding: 5px 10px 5px 10px;\n"
"  text-align: right;\n"
"  background-color: #558195;\n"
"  border-bottom-left-radius: 8px;\n"
"  border-bottom-right-radius: 8px;\n"
"  color: white;\n"
"}\n"
"\n"
"/* Hyperlink colors in the footer */\n"
"div.footer a { color: white; }\n"
"div.footer a:link { color: white; }\n"
"div.footer a:visited { color: white; }\n"
"div.footer a:hover { background-color: white; color: #558195; }\n"
"\n"
"/* verbatim blocks */\n"
"pre.verbatim {\n"
"  background-color: #f5f5f5;\n"
"  padding: 0.5em;\n"
"  white-space: pre-wrap;\n"
"}\n"
;


/* The following table contains bits of default CSS that must
** be included if they are not found in the application-defined
** CSS.
*/
const struct strctCssDefaults {
  char const * const elementClass;  /* Name of element needed */
  char const * const comment;       /* Comment text */
  char const * const value;         /* CSS text */
} cssDefaultList[] = {
  { "",
    "",
    zDefaultCSS
  },
  { "div.sidebox",
    "The nomenclature sidebox for branches,..",
    "  float: right;\n"
    "  background-color: white;\n"
    "  border-width: medium;\n"
    "  border-style: double;\n"
    "  margin: 10px;\n"
  },
  { "div.sideboxTitle",
    "The nomenclature title in sideboxes for branches,..",
    "  display: inline;\n"
    "  font-weight: bold;\n"
  },
  { "div.sideboxDescribed",
    "The defined element in sideboxes for branches,..",
    "  display: inline;\n"
    "  font-weight: bold;\n"
  },
  { "span.disabled",
    "The defined element in sideboxes for branches,..",
    "  color: red;\n"
  },
  { "span.timelineDisabled",
    "The suppressed duplicates lines in timeline, ..",
    "  font-style: italic;\n"
    "  font-size: small;\n"
  },
  { "table.timelineTable",
    "the format for the timeline data table",
    "  border: 0;\n"
  },
  { "td.timelineTableCell",
    "the format for the timeline data cells",
    "  vertical-align: top;\n"
    "  text-align: left;\n"
  },
  { "tr.timelineCurrent td.timelineTableCell",
    "the format for the timeline data cell of the current checkout",
    "  padding: .1em .2em;\n"
    "  border: 1px dashed #446979;\n"
  },
  { "span.timelineLeaf",
    "the format for the timeline leaf marks",
    "  font-weight: bold;\n"
  },
  { "a.timelineHistLink",
    "the format for the timeline version links",
    "\n"
  },
  { "span.timelineHistDsp",
    "the format for the timeline version display(no history permission!)",
    "  font-weight: bold;\n"
  },
  { "td.timelineTime",
    "the format for the timeline time display",
    "  vertical-align: top;\n"
    "  text-align: right;\n"
    "  white-space: nowrap;\n"
  },
  { "td.timelineGraph",
    "the format for the grap placeholder cells in timelines",
    "width: 20px;\n"
    "text-align: left;\n"
    "vertical-align: top;\n"
  },
  { "a.tagLink",
    "the format for the tag links",
    "\n"
  },
  { "span.tagDsp",
    "the format for the tag display(no history permission!)",
    "  font-weight: bold;\n"
  },
  { "span.wikiError",
    "the format for wiki errors",
    "  font-weight: bold;\n"
    "  color: red;\n"
  },
  { "span.infoTagCancelled",
    "the format for fixed/canceled tags,..",
    "  font-weight: bold;\n"
    "  text-decoration: line-through;\n"
  },
  { "span.infoTag",
    "the format for tags,..",
    "  font-weight: bold;\n"
  },
  { "span.wikiTagCancelled",
    "the format for fixed/cancelled tags,.. on wiki pages",
    "  text-decoration: line-through;\n"
  },
  { "table.browser",
    "format for the file display table",
    "/* the format for wiki errors */\n"
    "  width: 100%;\n"
    "  border: 0;\n"
  },
  { "td.browser",
    "format for cells in the file browser",
    "  width: 24%;\n"
    "  vertical-align: top;\n"
  },
  { "ul.browser",
    "format for the list in the file browser",
    "  margin-left: 0.5em;\n"
    "  padding-left: 0.5em;\n"
    "  white-space: nowrap;\n"
  },
  { ".filetree",
    "tree-view file browser",
    "  margin: 1em 0;\n"
    "  line-height: 1.5;\n"
  },
  { ".filetree ul",
    "tree-view lists",
    "  margin: 0;\n"
    "  padding: 0;\n"
    "  list-style: none;\n"
  },
  { ".filetree ul ul",
    "tree-view lists below the root",
    "  position: relative;\n"
    "  margin: 0 0 0 21px;\n"
  },
  { ".filetree li",
    "tree-view lists items",
    "  position: relative;\n"
  },
  { ".filetree li li:before",
    "tree-view node lines",
    "  content: '';\n"
    "  position: absolute;\n"
    "  top: -.8em;\n"
    "  left: -14px;\n"
    "  width: 14px;\n"
    "  height: 1.5em;\n"
    "  border-left: 2px solid #aaa;\n"
    "  border-bottom: 2px solid #aaa;\n"
  },
  { ".filetree ul ul:before",
    "tree-view directory lines",
    "  content: '';\n"
    "  position: absolute;\n"
    "  top: -1.5em;\n"
    "  bottom: 0;\n"
    "  left: -35px;\n"
    "  border-left: 2px solid #aaa;\n"
  },
  { ".filetree li:last-child > ul:before",
    "hide lines for last-child directories",
    "  display: none;\n"
  },
  { ".filetree a",
    "tree-view links",
    "  position: relative;\n"
    "  z-index: 1;\n"
    "  display: inline-block;\n"
    "  min-height: 16px;\n"
    "  padding-left: 21px;\n"
    "  background-image: url(data:image/gif;base64,R0lGODlhEAAQAJEAAP\\/\\/\\/yEhIf\\/\\/\\/wAAACH5BAEHAAIALAAAAAAQABAAAAIvlIKpxqcfmgOUvoaqDSCxrEEfF14GqFXImJZsu73wepJzVMNxrtNTj3NATMKhpwAAOw==);\n"
    "  background-position: center left;\n"
    "  background-repeat: no-repeat;\n"
  },
  { ".filetree .dir > a",
    "tree-view directory links",
    "  background-image: url(data:image/gif;base64,R0lGODlhEAAQAJEAAP/WVCIiIv\\/\\/\\/wAAACH5BAEHAAIALAAAAAAQABAAAAInlI9pwa3XYniCgQtkrAFfLXkiFo1jaXpo+jUs6b5Z/K4siDu5RPUFADs=);\n"
  },
  { "table.login_out",
    "table format for login/out label/input table",
    "  text-align: left;\n"
    "  margin-right: 10px;\n"
    "  margin-left: 10px;\n"
    "  margin-top: 10px;\n"
  },
  { "div.captcha",
    "captcha display options",
    "  text-align: center;\n"
    "  padding: 1ex;\n"
  },
  { "table.captcha",
    "format for the layout table, used for the captcha display",
    "  margin: auto;\n"
    "  padding: 10px;\n"
    "  border-width: 4px;\n"
    "  border-style: double;\n"
    "  border-color: black;\n"
  },
  { "td.login_out_label",
    "format for the label cells in the login/out table",
    "  text-align: center;\n"
  },
  { "span.loginError",
    "format for login error messages",
    "  color: red;\n"
  },
  { "span.note",
    "format for leading text for notes",
    "  font-weight: bold;\n"
  },
  { "span.textareaLabel",
    "format for textarea labels",
    "  font-weight: bold;\n"
  },
  { "table.usetupLayoutTable",
    "format for the user setup layout table",
    "  outline-style: none;\n"
    "  padding: 0;\n"
    "  margin: 25px;\n"
  },
  { "td.usetupColumnLayout",
    "format of the columns on the user setup list page",
    "  vertical-align: top\n"
  },
  { "table.usetupUserList",
    "format for the user list table on the user setup page",
    "  outline-style: double;\n"
    "  outline-width: 1px;\n"
    "  padding: 10px;\n"
  },
  { "th.usetupListUser",
    "format for table header user in user list on user setup page",
    "  text-align: right;\n"
    "  padding-right: 20px;\n"
  },
  { "th.usetupListCap",
    "format for table header capabilities in user list on user setup page",
    "  text-align: center;\n"
    "  padding-right: 15px;\n"
  },
  { "th.usetupListCon",
    "format for table header contact info in user list on user setup page",
    "  text-align: left;\n"
  },
  { "td.usetupListUser",
    "format for table cell user in user list on user setup page",
    "  text-align: right;\n"
    "  padding-right: 20px;\n"
    "  white-space:nowrap;\n"
  },
  { "td.usetupListCap",
    "format for table cell capabilities in user list on user setup page",
    "  text-align: center;\n"
    "  padding-right: 15px;\n"
  },
  { "td.usetupListCon",
    "format for table cell contact info in user list on user setup page",
    "  text-align: left\n"
  },
  { "div.ueditCapBox",
    "layout definition for the capabilities box on the user edit detail page",
    "  float: left;\n"
    "  margin-right: 20px;\n"
    "  margin-bottom: 20px;\n"
  },
  { "td.usetupEditLabel",
    "format of the label cells in the detailed user edit page",
    "  text-align: right;\n"
    "  vertical-align: top;\n"
    "  white-space: nowrap;\n"
  },
  { "span.ueditInheritNobody",
    "color for capabilities, inherited by nobody",
    "  color: green;\n"
  },
  { "span.ueditInheritDeveloper",
    "color for capabilities, inherited by developer",
    "  color: red;\n"
  },
  { "span.ueditInheritReader",
    "color for capabilities, inherited by reader",
    "  color: black;\n"
  },
  { "span.ueditInheritAnonymous",
    "color for capabilities, inherited by anonymous",
    "  color: blue;\n"
  },
  { "span.capability",
    "format for capabilities, mentioned on the user edit page",
    "  font-weight: bold;\n"
  },
  { "span.usertype",
    "format for different user types, mentioned on the user edit page",
    "  font-weight: bold;\n"
  },
  { "span.usertype:before",
    "leading text for user types, mentioned on the user edit page",
    "  content:\"'\";\n"
  },
  { "span.usertype:after",
    "trailing text for user types, mentioned on the user edit page",
    "  content:\"'\";\n"
  },
  { "div.selectedText",
    "selected lines of text within a linenumbered artifact display",
    "  font-weight: bold;\n"
    "  color: blue;\n"
    "  background-color: #d5d5ff;\n"
    "  border: 1px blue solid;\n"
  },
  { "p.missingPriv",
    "format for missing privileges note on user setup page",
    " color: blue;\n"
  },
  { "span.wikiruleHead",
    "format for leading text in wikirules definitions",
    "  font-weight: bold;\n"
  },
  { "td.tktDspLabel",
    "format for labels on ticket display page",
    "  text-align: right;\n"
  },
  { "td.tktDspValue",
    "format for values on ticket display page",
    "  text-align: left;\n"
    "  vertical-align: top;\n"
    "  background-color: #d0d0d0;\n"
  },
  { "span.tktError",
    "format for ticket error messages",
    "  color: red;\n"
    "  font-weight: bold;\n"
  },
  { "table.rpteditex",
    "format for example tables on the report edit page",
    "  float: right;\n"
    "  margin: 0;\n"
    "  padding: 0;\n"
    "  width: 125px;\n"
    "  text-align: center;\n"
    "  border-collapse: collapse;\n"
    "  border-spacing: 0;\n"
  },
  { "table.report",
    "Ticket report table formatting",
    "  border-collapse:collapse;\n"
    "  border: 1px solid #999;\n"
    "  margin: 1em 0 1em 0;\n"
    "  cursor: pointer;\n"
  },
  { "td.rpteditex",
    "format for example table cells on the report edit page",
    "  border-width: thin;\n"
    "  border-color: #000000;\n"
    "  border-style: solid;\n"
  },
  { "input.checkinUserColor",
    "format for user color input on checkin edit page",
    "/* no special definitions, class defined, to enable color pickers, f.e.:\n"
    "**  add the color picker found at http:jscolor.com  as java script include\n"
    "**  to the header and configure the java script file with\n"
    "**   1. use as bindClass :checkinUserColor\n"
    "**   2. change the default hash adding behaviour to ON\n"
    "** or change the class defition of element identified by id=\"clrcust\"\n"
    "** to a standard jscolor definition with java script in the footer. */\n"
  },
  { "div.endContent",
    "format for end of content area, to be used to clear page flow.",
    "  clear: both;\n"
  },
  { "p.generalError",
    "format for general errors",
    "  color: red;\n"
  },
  { "p.tktsetupError",
    "format for tktsetup errors",
    "  color: red;\n"
    "  font-weight: bold;\n"
  },
  { "p.xfersetupError",
    "format for xfersetup errors",
    "  color: red;\n"
    "  font-weight: bold;\n"
  },
  { "p.thmainError",
    "format for th script errors",
    "  color: red;\n"
    "  font-weight: bold;\n"
  },
  { "span.thTrace",
    "format for th script trace messages",
    "  color: red;\n"
  },
  { "p.reportError",
    "format for report configuration errors",
    "  color: red;\n"
    "  font-weight: bold;\n"
  },
  { "blockquote.reportError",
    "format for report configuration errors",
    "  color: red;\n"
    "  font-weight: bold;\n"
  },
  { "p.noMoreShun",
    "format for artifact lines, no longer shunned",
    "  color: blue;\n"
  },
  { "p.shunned",
    "format for artifact lines beeing shunned",
    "  color: blue;\n"
  },
  { "span.brokenlink",
    "a broken hyperlink",
    "  color: red;\n"
  },
  { "ul.filelist",
    "List of files in a timeline",
    "  margin-top: 3px;\n"
    "  line-height: 100%;\n"
  },
  { "table.sbsdiffcols",
    "side-by-side diff display (column-based)",
    "  width: 90%;\n"
    "  border-spacing: 0;\n"
    "  font-size: xx-small;\n"
  },
  { "table.sbsdiffcols td",
    "sbs diff table cell",
    "  padding: 0;\n"
    "  vertical-align: top;\n"
  },
  { "table.sbsdiffcols pre",
    "sbs diff pre block",
    "  margin: 0;\n"
    "  padding: 0;\n"
    "  border: 0;\n"
    "  font-size: inherit;\n"
    "  background: inherit;\n"
    "  color: inherit;\n"
  },
  { "div.difflncol",
    "diff line number column",
    "  padding-right: 1em;\n"
    "  text-align: right;\n"
    "  color: #a0a0a0;\n"
  },
  { "div.difftxtcol",
    "diff text column",
    "  width: 45em;\n"
    "  overflow-x: auto;\n"
  },
  { "div.diffmkrcol",
    "diff marker column",
    "  padding: 0 1em;\n"
  },
  { "span.diffchng",
    "changes in a diff",
    "  background-color: #c0c0ff;\n"
  },
  { "span.diffadd",
    "added code in a diff",
    "  background-color: #c0ffc0;\n"
  },
  { "span.diffrm",
    "deleted in a diff",
    "  background-color: #ffc8c8;\n"
  },
  { "span.diffhr",
    "suppressed lines in a diff",
    "  display: inline-block;\n"
    "  margin: .5em 0 1em;\n"
    "  color: #0000ff;\n"
  },
  { "span.diffln",
    "line numbers in a diff",
    "  color: #a0a0a0;\n"
  },
  { "span.modpending",
    "Moderation Pending message on timeline",
    "  color: #b03800;\n"
    "  font-style: italic;\n"
  },
  { "pre.th1result",
    "format for th1 script results",
    "  white-space: pre-wrap;\n"
    "  word-wrap: break-word;\n"
  },
  { "pre.th1error",
    "format for th1 script errors",
    "  white-space: pre-wrap;\n"
    "  word-wrap: break-word;\n"
    "  color: red;\n"
  },
  { "table.tale-value th",
    "The label/value pairs on (for example) the ci page",
    "  vertical-align: top;\n"
    "  text-align: right;\n"
    "  padding: 0.2ex 2ex;\n"
  },
  { ".statistics-report-graph-line",
    "for the /stats_report views",
    "  background-color: #446979;\n"
  },
  { ".statistics-report-table-events th",
    "",
    "  padding: 0 1em 0 1em;\n"
  },
  { ".statistics-report-table-events td",
    "",
    "  padding: 0.1em 1em 0.1em 1em;\n"
  },
  { ".statistics-report-row-year",
    "",
    "  text-align: left;\n"
  },
  { ".statistics-report-graph-line",
    "for the /stats_report views",
    "  background-color: #446979;\n"
  },
  { ".statistics-report-week-number-label",
    "for the /stats_report views",
    "text-align: right;\n"
    "font-size: 0.8em;\n"
  },
  { ".statistics-report-week-of-year-list",
    "for the /stats_report views",
    "font-size: 0.8em;\n"
  },
  { "tr.row0",
    "even table row color",
    "/* use default */\n"
  },
  { "tr.row1",
    "odd table row color",
    "/* Use default */\n"
  },
  { "#canvas", "timeline graph node colors",
    "color: black;\n"
    "background-color: white;\n"
  },
  { 0,
    0,
    0
  }
};

/*
** Append all of the default CSS to the CGI output.
*/
void cgi_append_default_css(void) {
  int i;

  for (i=0;cssDefaultList[i].elementClass;i++){
    if (cssDefaultList[i].elementClass[0]){
      cgi_printf("/* %s */\n%s {\n%s\n}\n\n",
                 cssDefaultList[i].comment,
                 cssDefaultList[i].elementClass,
                 cssDefaultList[i].value
                );
    }else{
      cgi_printf("%s",
                 cssDefaultList[i].value
                );
    }
  }
}

/*
** WEBPAGE: style.css
*/
void page_style_css(void){
  Blob css;
  int i;

  cgi_set_content_type("text/css");
  blob_init(&css, db_get("css",(char*)zDefaultCSS), -1);

  /* add special missing definitions */
  for(i=1; cssDefaultList[i].elementClass; i++){
    if( strstr(blob_str(&css), cssDefaultList[i].elementClass)==0 ){
      blob_appendf(&css, "/* %s */\n%s {\n%s}\n",
          cssDefaultList[i].comment,
          cssDefaultList[i].elementClass,
          cssDefaultList[i].value);
    }
  }

  /* Process through TH1 in order to give an opportunity to substitute
  ** variables such as $baseurl.
  */
  Th_Store("baseurl", g.zBaseURL);
  Th_Store("home", g.zTop);
  image_url_var("logo");
  image_url_var("background");
  Th_Render(blob_str(&css));

  /* Tell CGI that the content returned by this page is considered cacheable */
  g.isConst = 1;
}

/*
** WEBPAGE: test_env
*/
void page_test_env(void){
  char c;
  int i;
  int showAll;
  char zCap[30];
  static const char *azCgiVars[] = {
    "COMSPEC", "DOCUMENT_ROOT", "GATEWAY_INTERFACE",
    "HTTP_ACCEPT", "HTTP_ACCEPT_CHARSET", "HTTP_ACCEPT_ENCODING",
    "HTTP_ACCEPT_LANGUAGE", "HTTP_CONNECTION", "HTTP_HOST",
    "HTTP_USER_AGENT", "HTTP_REFERER", "PATH_INFO", "PATH_TRANSLATED",
    "QUERY_STRING", "REMOTE_ADDR", "REMOTE_PORT", "REQUEST_METHOD",
    "REQUEST_URI", "SCRIPT_FILENAME", "SCRIPT_NAME", "SERVER_PROTOCOL",
  };

  login_check_credentials();
  if( !g.perm.Admin && !g.perm.Setup && !db_get_boolean("test_env_enable",0) ){
    login_needed();
    return;
  }
  for(i=0; i<count(azCgiVars); i++) (void)P(azCgiVars[i]);
  style_header("Environment Test");
  showAll = atoi(PD("showall","0"));
  if( !showAll ){
    style_submenu_element("Show Cookies", "Show Cookies",
                          "%s/test_env?showall=1", g.zTop);
  }else{
    style_submenu_element("Hide Cookies", "Hide Cookies",
                          "%s/test_env", g.zTop);
  }
#if !defined(_WIN32)
  cgi_printf("uid=%d, gid=%d<br />\n",(getuid()),(getgid()));
#endif
  cgi_printf("g.zBaseURL = %h<br />\n"
         "g.zTop = %h<br />\n",(g.zBaseURL),(g.zTop));
  for(i=0, c='a'; c<='z'; c++){
    if( login_has_capability(&c, 1) ) zCap[i++] = c;
  }
  zCap[i] = 0;
  cgi_printf("g.userUid = %d<br />\n"
         "g.zLogin = %h<br />\n"
         "g.isHuman = %d<br />\n"
         "capabilities = %s<br />\n"
         "<hr>\n",(g.userUid),(g.zLogin),(g.isHuman),(zCap));
  P("HTTP_USER_AGENT");
  cgi_print_all(showAll);
  if( showAll && blob_size(&g.httpHeader)>0 ){
    cgi_printf("<hr>\n"
           "<pre>\n"
           "%h\n"
           "</pre>\n",(blob_str(&g.httpHeader)));
  }
  if( g.perm.Setup ){
    const char *zRedir = P("redirect");
    if( zRedir ) cgi_redirect(zRedir);
  }
  style_footer();
  if( g.perm.Admin && P("err") ) fossil_fatal("%s", P("err"));
}

/*
** This page is a honeypot for spiders and bots.
**
** WEBPAGE: honeypot
*/
void honeypot_page(void){
  cgi_set_status(403, "Forbidden");
  cgi_printf("<p>Please enable javascript or log in to see this content</p>\n");
}
