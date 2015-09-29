/**
*************************************************************
* @file siliconloader.cpp
* @brief Loads different keywords/functions configurations
*        for siliCon
*
* @author Gaspar Fernández <blakeyed@totaki.com>
* @version 0.1
* @date 29 sep 2015
* Changelog:
*
*
*
*
*
*
*************************************************************/

#include "siliconloader.h"

void SiliconLoader::loadKeyword(std :: string kw, std :: string val, Silicon * s)
{
  if (s!=NULL)
    s->setKeyword(kw, val);
  else
    Silicon::setGlobalKeyword(kw, val);
}

void SiliconLoader::loadFunction(std :: string name, Silicon :: TemplateFunction fun, Silicon * s)
{
  if (s!=NULL)
    s->setFunction(name, fun);
  else
    Silicon::setGlobalFunction(name, fun);
}
