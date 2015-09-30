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

  /* Getters and setters */

  /* for default URL */
  static inline std::string defaultUrl(std::string url)
  {
    _defaultUrl = url;
    return _defaultUrl;
  }

  static inline std::string defaultUrl()
  {
    return _defaultUrl;
  }

  /* for css URL */
  static inline std::string cssUrl(std::string url)
  {
    _cssUrl = url;
    return _cssUrl;
  }

  static inline std::string cssUrl()
  {
    return _cssUrl;
  }

 protected:
  static std::string includeCss (Silicon* s, Silicon::StringMap args, std::string input);

  static std::string getBaseUrl (Silicon* s);
  static std::string getCssUrl (Silicon* s);
 private:
  std::string _defaultUrl;
  std::string _cssUrl;
};

#endif /* _SILICONWEB_H */

