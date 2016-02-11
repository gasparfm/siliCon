/**
*************************************************************
* @file siliconweb.cpp
* @brief Silicon Web extensions
* Functions and Keywords intended to be used in web templates
*
* Keywords may be used for input/output of global template data.
*
* @author Gaspar Fernández <blakeyed@totaki.com>
* @version 0.1
* @date 28 sep 2015
*
* Changelog:
*   - 20160207 : Javascripts are not rel="stylesheet"!!
*   - 20160205 : includeCss / includeJs / directJs as members too
*   - 20151008 : bug fixed. When using includeCss with 
*       _renderResources="0" and no media argument, it gives
*       access violation error due to how includeCss extracts
*       media using argument list.
*   - 20151008 : bug fixed. Leave absolute CSS/JS paths as is, don't
*       concat path
*
*
*************************************************************/

/*
  Used keywords:
   _siliconWeb = "1": SiliconWeb is loaded (global)
   _baseURL         : URL with http:// to use as base path for all resources
   _cssURL          : _baseURL/_cssURL/ is where all css are stored
   _jsURL           : _baseURL/_jsURL/ is where all js are stored
   _renderResources : CSS / JS will be rendered directly or by calling
                      "renderCSS" / "renderJS" function. "0" is false,
		      otherwise, true

  Used collections:
  _CSS              : To store all CSS files to be rendered
  _JS               : To store all JS files to be rendered
  _directJS         : To store all JS code to be attached

  Available functions (for templates):
  includeCss ( file="cssfile" [media="media"] ) : include css file
  includeJs  ( file="jsfile" )                  : include js file
  extraJs (data = extra javascript)
  renderCss
  renderJs
  list : renders a list with a collection

  Available methods (for C++):
  */
#include "siliconweb.h"
#include <iostream>

namespace
{
  std::string addSlash(std::string path)
  {
    return ( (!path.empty()) && (path.back()!='/') )?path+'/':path;
  }

  bool isAbsolute(std::string url)
  {
    if (url.front()=='/')
      return true;

    std::string cut8=url.substr(0,8);
    return ( (cut8.substr(0,7)=="http://") || (cut8=="https://") );
  }
}

std::string SiliconWeb::_defaultUrl ="";
std::string SiliconWeb::_cssUrl ="";
std::string SiliconWeb::_jsUrl ="";
bool SiliconWeb::_renderDefault = true;

void SiliconWeb::load(Silicon * s)
{
  loadKeyword("_siliconWeb", "1", s);

  loadFunction("includeCss", SiliconWeb::_includeCss, s);
  loadFunction("includeJs", SiliconWeb::_includeJs, s);
  loadFunction("directJs", SiliconWeb::_directJs, s);
  loadFunction("renderCss", SiliconWeb::renderCss, s);
  loadFunction("renderJs", SiliconWeb::renderJs, s);
  loadFunction("list", SiliconWeb::list, s);
}

std::string SiliconWeb::list (Silicon* s, Silicon::StringMap args, std::string input)
{
  auto collection = args.find("collection");
  if (collection == args.end())
    return "";
  auto tagclass = args.find("class");
  auto tagid = args.find("id");
  auto _uselink = args.find("uselink");
  bool uselink = ( (_uselink != args.end()) && (_uselink->second!="0") );

  std::string col = collection->second;

  std::string templ = "<ul";
  if (tagclass != args.end())
    templ+=" class=\""+tagclass->second+"\"";

  if (tagid != args.end())
    templ+=" id=\""+tagid->second+"\"";

  templ+=">\n"
    "{%collection var="+col+"}}\n"
    "<li>";
  if (uselink)
    templ+="<a href=\"{{"+col+".link}}\" {%if "+col+".title}} title=\"{{"+col+".title}}\">{/if}}>";
  templ+="{{"+col+".text}}";
  if (uselink)
    templ+="</a>";
  templ+="</li>\n"
    "{/collection}}\n"
    "</ul>";
  return s->parse(templ);
}

/* Include CSS by code, just add it to the collection */
void SiliconWeb::includeCss(Silicon* s, std::string file, std::string media)
{
  std::string fileabs = (isAbsolute(file))?file:getCssUrl(s)+file;
  std::string out = "<link href=\""+fileabs+"\" rel=\"stylesheet\" type=\"text/css\"";
  if (!media.empty())
    out+=" media=\""+media+"\"";
  out+=" />";
  s->addToCollection("_CSS", {
	  { "file", file },
	  { "href", fileabs },
	  { "media", media },
	  { "code", out }
    });
}

/* Include JS by code, adding it to the collection */
void SiliconWeb::includeJs(Silicon* s, std::string file)
{
  std::string fileabs = (isAbsolute(file))?file:getJsUrl(s)+file;
  std::string out = "<script type=\"text/javascript\" src=\""+fileabs+"\" rel=\"stylesheet\"></script>";
  s->addToCollection("_JS", {
	  { "file", file },
	  { "src", fileabs },
	  { "code", out }
    });}

/* Add direct Js to the collection  */
void SiliconWeb::directJs(Silicon* s, std::string code)
{
  if (code.empty())
    return;

  s->addToCollection("_directJS", { { "code", code } });
}

std::string SiliconWeb::renderCss (Silicon* s, Silicon::StringMap args, std::string input)
{
  auto list = s->getCollection("_CSS");
  if (list.empty())
    return "";

  auto comments = args.find("comments");
  bool printComment = ( (comments != args.end()) && (comments->second != "0") );

  std::string out;
  if (printComment)
    out+="<!-- Start styles -->\n";

  for (auto s : list)
    {
      auto code = s.find("code");
      out+=code->second+"\n";
    }

  if (printComment)
    out+="<!-- End styles -->\n";

  return out;
}

std::string SiliconWeb::_includeCss (Silicon* s, Silicon::StringMap args, std::string input)
{
  std::string out;
  auto _file=args.find("file");
  if (_file==args.end())
    return "";

  auto media=args.find("media");
  std::string file = (isAbsolute(_file->second))?_file->second:getCssUrl(s)+_file->second;
  out= "<link href=\""+file+"\" rel=\"stylesheet\" type=\"text/css\"";
  if (media!=args.end())
    out+=" media=\""+media->second+"\"";
  out+=" />";

  if (getDoRender(s))
    return out;
  else
      s->addToCollection("_CSS", {
	  { "file", _file->second },
	  { "href", file },
	  { "media", (media!=args.end())?media->second:"" },
	  { "code", out }
	});

  return "";
}

std::string SiliconWeb::getBaseUrl(Silicon * s)
{
  std::string baseURL;
  bool havePath = s->getKeyword("_baseURL", baseURL);
  return (havePath)?addSlash(baseURL):addSlash(defaultUrl());
}

std::string SiliconWeb::getCssUrl(Silicon * s)
{
  std::string cssURL;
  bool haveCssUrl = s->getKeyword("_cssURL", cssURL);
  if (!haveCssUrl)
    cssURL = cssUrl();

  std::string basePath = getBaseUrl(s);
  if ( (basePath.empty()) && (cssURL.empty()) )
    return "";

  if (cssURL.front()=='/')
    cssURL.substr(1);

  return addSlash(basePath+cssURL);
}

bool SiliconWeb::getDoRender(Silicon * s)
{
  auto render = s->getKeyword("_renderResources");
  return (render.empty())?_renderDefault:(render!="0");
}

std::string SiliconWeb::_includeJs(Silicon * s, Silicon :: StringMap args, std :: string input)
{
  std::string out;
  auto _file=args.find("file");
  if (_file==args.end())
    return "";

  std::string file = (isAbsolute(_file->second))?_file->second:getJsUrl(s)+_file->second;
  out= "<script type=\"text/javascript\" src=\""+file+"\"></script>";

  if (getDoRender(s))
    return out;
  else
    s->addToCollection("_JS", { { "file", _file->second }, { "src", file }, { "code", out } });

  return "";
}

std::string SiliconWeb::_directJs(Silicon * s, Silicon :: StringMap args, std :: string input)
{
  if (input.empty())
    return "";

  s->addToCollection ("_directJS", { { "code", input } });

 return "";
}

std::string SiliconWeb::renderJs(Silicon * s, Silicon :: StringMap args, std :: string input)
{
  std::string out;

  auto comments = args.find("comments");
  bool printComment = ( (comments != args.end()) && (comments->second != "0") );
  if (printComment)
    out+="<!-- Start scripts -->\n";

  auto test = args.find("files");
  bool renderFiles = ( (test == args.end()) || (test->second!="0") );
  test = args.find("direct");
  bool renderDirect = ( (test == args.end()) || (test->second!="0") );

  if (renderFiles)
    {
      auto list = s->getCollection("_JS");
      if (!list.empty())
	{
	  for (auto s : list)
	    {
	      auto code = s.find("code");
	      out+=code->second+"\n";
	    }
	}
    }
  if (renderDirect)
    {
      auto list = s->getCollection("_directJS");
      if (!list.empty())
	{
	  out+="<script type=\"text/javascript\">";
	  for (auto s : list)
	    {
	      auto code = s.find("code");
	      out+=code->second+"\n";
	    }
	  out+="</script>\n";
	}
    }
  if (printComment)
    out+="<!-- End scripts -->\n";

  return out;
}

std::string SiliconWeb::getJsUrl(Silicon * s)
{
  std::string jsURL;
  bool haveJsUrl = s->getKeyword("_jsURL", jsURL);
  if (!haveJsUrl)
    jsURL = jsUrl();

  std::string basePath = getBaseUrl(s);
  if ( (basePath.empty()) && (jsURL.empty()) )
    return "";

  if (jsURL.front()=='/')
    jsURL.substr(1);

  return addSlash(basePath+jsURL);
}
