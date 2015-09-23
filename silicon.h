/* @(#)silicon.h
 */

#ifndef _SILICON_H
#define _SILICON_H 1

#include <string>
#include <exception>
#include <functional>
#include <map>

#define MAXBUFFERLEN 16384

#ifndef SILICON_DEBUG
  #ifdef DEBUG
    #define SILICON_DEBUG 1
  #else
    #define SILICON_DEBUG 0
  #endif
#endif

class SiliconException : public std::exception
{
 public:
 SiliconException(const int& code, const std::string &message, long line, long pos): _code(code), _message(message), _line(line), _pos(pos)
  {
  }

  virtual ~SiliconException() throw ()
  {
  }

  const char* what() const throw()
  {
    std::string msg;
    msg="Error "+std::to_string(_code)+": "+_message;
    #if SILICON_DEBUG
    msg+=" on line "+std::to_string(_line)+":"+std::to_string(_pos);
    #endif
    return msg.c_str();
  }

  int code() const
  {
    return _code;
  }

protected:
  /** Error code */
  int _code;
  /** Error message  */
  std::string _message;
  /* Error line */
  long _line;
  /* Error pos */
  long _pos;
};

class Silicon
{
public:
  using TemplateFunction = std::function<std::string(Silicon*, std::map<std::string, std::string>, std::string)>;

  virtual ~Silicon();

  /* static Silicon & createFromFile(std::string& file, long maxBufferLen=0); */
  static Silicon createFromFile(const char* file, const char* defaultPath=NULL, long maxBufferLen=0);
  static Silicon createFromStr(std::string& data, long maxBufferLen=0);
  /* static Silicon & createFromStr(const char* data, long maxBufferLen=0); */

  std::string render();

  Silicon(Silicon&& sil);

  /* Base path */
  inline void setBasePath(std::string newBasePath)
  {
    this->localConfig.basePath = newBasePath;
  }

  inline std::string getBasePath()
  {
    return this->localConfig.basePath;
  }

  /* Keywords */
  void setKeyword(std::string kw, std::string text);
  static void setGlobalKeyword(std::string kw, std::string text);
  std::string getKeyword(std::string kw);
  bool getKeyword(std::string kw, std::string &text);

  /* Functions */
  void setFunction(std::string name, TemplateFunction callable);
  static void setGlobalFunction(std::string name, TemplateFunction callable);

  /* Operators */
  void setOperator(std::string, std::function<bool(Silicon*, std::string, std::string)> func);
  void setOperator(std::string, std::function<bool(Silicon*, long long, long long)> func);
  void setOperator(std::string, std::function<bool(Silicon*, long double, long double)> func);
  void setGlobalOperator(std::string, std::function<bool(Silicon*, std::string, std::string)> func);
  void setGlobalOperator(std::string, std::function<bool(Silicon*, long long, long long)> func);
  void setGlobalOperator(std::string, std::function<bool(Silicon*, long double, long double)> func);

protected:
  Silicon(const char* data, long maxBufferLen);
  Silicon(const char* file, const char* defaultPath, long maxBufferLen);
  long parse(std::string& destination, char* strptr, bool write=true, std::string nested="", int level=0);
  long parseKeyword(char* strptr, std::string& keyword);
  long parseFunction(char* strptr, int &type, std::string& fname, std::map<std::string, std::string> &arguments, bool &autoClosed);
  long parseCloseNested(char* strptr, std::string closeName);

  std::string putKeyword(std::string keyword);

  long computeBuiltin(char* strptr, std::string &destination, std::string bif, std::map<std::string, std::string> &arguments, bool &autoClosed, bool write, int level);
  long computeBuiltinIf(char* strptr, std::string &destination, std::map<std::string, std::string> &arguments, bool write, int level);

  TemplateFunction getFunction(std::string fun);

  bool evaluateCondition(std::string condition);

  /* Operators' stuff */
  bool conditionStringOperator(std::string op, std::string a, std::string b);
  bool conditionDoubleOperator(std::string op, long double a, long double b);
  bool conditionLongOperator(std::string op, long long a, long long b);

  long getCurrentLine();
  long getCurrentPos();

  /* Default global functions */
  std::string globalFuncDate(Silicon* s, std::map<std::string, std::string> options);

private:
  /* std::string _data;			/\* Template string *\/ */
  char* _data = NULL;

  struct
  {
    /* instance configuration */
    long maxBufferLen;

    /* Leave unmatched keywords */
    bool leaveUnmatchedKwds;

    /* Base view path */
    std::string basePath;
  } localConfig;

  std::map<std::string, std::string> localKeywords;
  std::map<std::string, TemplateFunction> localFunctions;

  static std::map<std::string, std::string> globalKeywords;
  static std::map<std::string, TemplateFunction> globalFunctions;

  /* operators */
  std::map<std::string, std::function<bool(Silicon*, std::string, std::string)>> localConditionStringOperators;
  std::map<std::string, std::function<bool(Silicon*, long long, long long)>> localConditionLongOperators;
  std::map<std::string, std::function<bool(Silicon*, long double, long double)>> localConditionDoubleOperators;

  static std::map<std::string, std::function<bool(Silicon*, std::string, std::string)>> globalConditionStringOperators;
  static std::map<std::string, std::function<bool(Silicon*, long long, long long)>> globalConditionLongOperators;
  static std::map<std::string, std::function<bool(Silicon*, long double, long double)>> globalConditionDoubleOperators;

  /* caches and so... */

  std::string getOperator(std::string condition, size_t pos, std::string &b);
  short conditionNumericAB(std::string a, std::string b, long long &lla, long long &llb);
  short conditionDoubleAB(std::string a, std::string b, long double &lda, long double &ldb);

  void configure();

  #if SILICON_DEBUG
  struct
  {
    long line;
    long pos;
    long keywords;
    long functions;
  } Stats;
  #endif

  inline void resetStats()
  {
    #if SILICON_DEBUG
    Stats.line =1;
    Stats.pos =1;
    Stats.keywords =0;
    Stats.functions =0;
    #endif
  }

  inline void addKeywordToStats()
  {
    #if SILICON_DEBUG
    Stats.keywords++;
    #endif
  }

  inline void ahead(char ** ptr, long howmany=1)
  {
    #if SILICON_DEBUG
    while ( (howmany-->0) && (**ptr != '\0') )
      {
	++*ptr;
	++Stats.pos;
	if (**ptr == '\n')
	  {
	    Stats.line++;
	    Stats.pos=1;
	  }
      }
    #else
      *ptr+=howmany;
    #endif
  }

  inline void functionParserFill(int &status,
				 std::string& fname, 
				 std::map<std::string, std::string> &arguments,
				 std::string &currentString,
				 std::string &tempKey,
				 int &autoKey)
  {
    switch (status)
      {
      case 0: fname=currentString;
	status = 2;		/* fill param value */
	break;
      case 1: tempKey=currentString;
	status = 2;
	break;
      case 2: 
	if (tempKey.empty())
	  {
	    /* empty key, use autoKey number */
	    arguments.insert({std::to_string(autoKey++), currentString});
	  }
	else
	  {
	    /* temporary key present, use key and value */
	    arguments.insert({tempKey, currentString});
	    tempKey.clear();
	  }
	break;
      default:
	throw SiliconException(3, "Wrong parser fill value!! Programming failure!", getCurrentLine(), getCurrentPos());
      }

    currentString.clear();
  }
};

#endif /* _SILICON_H */
