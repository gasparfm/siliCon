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

  /* for js URL */
  static inline std::string jsUrl(std::string url)
  {
    _jsUrl = url;
    return _jsUrl;
  }

  static inline std::string jsUrl()
  {
    return _jsUrl;
  }

  /* for render Default */
  static inline bool renderDefault(bool val)
  {
    _renderDefault = val;
    return _renderDefault;
  }

  static inline bool renderDefault()
  {
    return _renderDefault;
  }

 protected:
  /* options file=cssfile media=media (optional) */
  static std::string includeCss (Silicon* s, Silicon::StringMap args, std::string input);
  /* options file=jsfile */
  static std::string includeJs (Silicon* s, Silicon::StringMap args, std::string input);
  /* no options, but reads data */
  static std::string directJs (Silicon* s, Silicon::StringMap args, std::string input);

  /* options comments=1 (optional) renders starting and ending comments */
  static std::string renderCss (Silicon* s, Silicon::StringMap args, std::string input);

  /* options comments=1 (optional) renders starting and ending comments
             files=0    (optional) don't render file inclusions
             direct=0    (optional) don't render direct javascript
  */
  static std::string renderJs (Silicon* s, Silicon::StringMap args, std::string input);

  /*
    renders a list with a collection
   */
  static std::string list (Silicon* s, Silicon::StringMap args, std::string input);

  static std::string getBaseUrl (Silicon* s);
  static std::string getCssUrl (Silicon* s);
  static std::string getJsUrl (Silicon* s);
  static bool getDoRender(Silicon* s);
 private:
  /* Default base URL with http:// */
  static std::string _defaultUrl;

  /* Default CSS URL */
  static std::string _cssUrl;

  /* Default JS URL */
  static std::string _jsUrl;

  /* Renders css/js by default */
  static bool _renderDefault;
};

#endif /* _SILICONWEB_H */

