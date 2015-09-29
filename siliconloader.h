/* @(#)siliconloader.h
 */

#ifndef _SILICONLOADER_H
#define _SILICONLOADER_H 1

#include <string>
#include "silicon.h"

class SiliconLoader
{
 public:
  static void load(Silicon* s=NULL) { }

 protected:
  static void loadKeyword(std::string kw, std::string val, Silicon* s=NULL);
  static void loadFunction(std::string name, Silicon::TemplateFunction fun, Silicon* s=NULL);
};

#endif /* _SILICONWEB_H */

