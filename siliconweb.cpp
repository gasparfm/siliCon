/**
*************************************************************
* @file siliconweb.cpp
* @brief Silicon Web extensions
* Functions and Keywords intended to be used in web templates
*
* @author Gaspar Fernández <blakeyed@totaki.com>
* @version 0.1
* @date 28 sep 2015
* Changelog
*
*
*
*
*
*
*
*************************************************************/

#include "siliconweb.h"

namespace
{
  std::string addSlash(std::string path)
  {
    return ( (!path.empty()) && (path.back()!='/') )?path+'/':path;
  }
};

void SiliconWeb::load(Silicon * s)
{
  loadFunction("includeCss", SiliconWeb::includeCss, s);
}

std::string SiliconWeb::includeCss (Silicon* s, Silicon::StringMap args, std::string input)
{
  std::string out;
  auto baseURL = getBaseUrl(s);
  auto file=args.find("file");
  if (file==args.end())
    return "";

  auto media=args.find("media");
  out= "<link href=\""+getBaseUrl(s)+file->second+"\" rel=\"stylesheet\" type=\"text/css\"";
  if (media!=args.end())
    out+=" media=\""+media->second+"\">";

  return out;
}

std::string SiliconWeb::getBaseUrl(Silicon * s)
{
  std::string baseURL;
  bool havePath = s->getKeyword("_baseURL", baseURL);
  return (havePath)?addSlash(baseURL):addSlash(getDefaultUrl());
}

std::string SiliconWeb::getCssUrl(Silicon * s)
{
  std::string cssURL;
  bool haveCssUrl = s->getKeyword("_cssURL", cssURL);
  if (!haveCssUrl)
    cssURL = cssUrl();
}
