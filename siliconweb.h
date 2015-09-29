/* @(#)siliconweb.h
 */

#ifndef _SILICONWEB_H
#define _SILICONWEB_H 1

#include <string>
#include "silicon.h"
#include "siliconloader.h"

class SiliconWeb : public SiliconLoader
{
 public:
  static void load(Silicon* s=NULL);

 /* protected: */
 /*  static void loadKeyword(Silicon* s=NULL, std::string kw, std::string val); */

 /* private: */
 /*  static void loadLocalKeyword(Silicon* s=NULL, std::string kw, std::string val); */
 /*  static void localGlobalKeyword(Silicon* s=NULL, std::string kw, std::string val); */
};

#endif /* _SILICONWEB_H */

