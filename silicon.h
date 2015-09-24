/* @(#)silicon.h
 */

#ifndef _SILICON_H
#define _SILICON_H 1

#include <string>
#include <exception>
#include <functional>
#include <map>
#include <vector>

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
  enum LayoutType
  {
    FILE,
    DATA
  };
  using StringMap = std::map<std::string, std::string>;
  using TemplateFunction = std::function<std::string(Silicon*, StringMap, std::string)>;
  using FunctionMap = std::map<std::string, TemplateFunction>;
  using StringOperator = std::function<bool(Silicon*, std::string, std::string)>;
  using LongOperator = std::function<bool(Silicon*, long long, long long)>;
  using DoubleOperator = std::function<bool(Silicon*, long double, long double)>;

  virtual ~Silicon();

  static Silicon createFromFile(std::string& file, std::string defaultPath="", long maxBufferLen=0);
  static Silicon createFromFile(const char* file, const char* defaultPath=NULL, long maxBufferLen=0);
  static Silicon createFromStr(std::string& data, long maxBufferLen=0);
  static Silicon createFromStr(const char* data, long maxBufferLen=0);

  std::string render(bool useLayout=true);

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

  /* Layouts */
  void setLayout(LayoutType ltype, const char* layout);
  void setLayout(std::string file);

  /* Keywords */
  void setKeyword(std::string kw, std::string text);
  static void setGlobalKeyword(std::string kw, std::string text);
  std::string getKeyword(std::string kw);
  bool getKeyword(std::string kw, std::string &text);
  static void setContentsKeyword(std::string newck);
  static std::string getContentsKeyword();

  /* Collections */
  void addCollection(std::string kw, std::vector<StringMap> coll);

  /* Functions */
  void setFunction(std::string name, TemplateFunction callable);
  static void setGlobalFunction(std::string name, TemplateFunction callable);

  /* Operators */
  void setOperator(std::string, StringOperator func);
  void setOperator(std::string, LongOperator func);
  void setOperator(std::string, DoubleOperator func);
  void setGlobalOperator(std::string, StringOperator func);
  void setGlobalOperator(std::string, LongOperator func);
  void setGlobalOperator(std::string, DoubleOperator func);

protected:
  Silicon(const char* data, long maxBufferLen);
  Silicon(const char* file, const char* defaultPath, long maxBufferLen);
  long parse(std::string& destination, char* strptr, bool write=true, std::string nested="", int level=0);
  long parseKeyword(char* strptr, std::string& keyword);
  long parseFunction(char* strptr, int &type, std::string& fname, StringMap &arguments, bool &autoClosed);
  long parseCloseNested(char* strptr, std::string closeName);

  long getNumericArgument(std::map<std::string, std::string>::iterator);
  std::string putKeyword(std::string keyword);

  long computeBuiltin(char* strptr, std::string &destination, std::string bif, StringMap &arguments, bool &autoClosed, bool write, int level);
  long computeBuiltinIf(char* strptr, std::string &destination, StringMap &arguments, bool write, int level);
  long computeBuiltinCollection(char* strptr, std::string &destination, StringMap &arguments, bool write, int level);

  TemplateFunction getFunction(std::string fun);

  bool evaluateCondition(std::string condition);

  /* Operators' stuff */
  bool conditionStringOperator(std::string op, std::string a, std::string b);
  bool conditionDoubleOperator(std::string op, long double a, long double b);
  bool conditionLongOperator(std::string op, long long a, long long b);

  long getCurrentLine();
  long getCurrentPos();

  StringMap separateArguments(StringMap &arguments);

  /* Default global functions */
  std::string globalFuncDate(Silicon* s, StringMap options);
  std::string globalFuncBlock(Silicon* s, StringMap options);
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

  void extractFile(char **ptr, std::string filename, bool usePath=true);
  void copyBuffer(char **ptr, const char* origin);

  StringMap localKeywords;
  FunctionMap localFunctions;
  std::map<std::string, std::vector<StringMap > > localCollections;

  static std::string contentsKeyword;
  static char* layoutData;
  static StringMap globalKeywords;
  static FunctionMap globalFunctions;

  /* operators */
  std::map<std::string, StringOperator> localConditionStringOperators;
  std::map<std::string, LongOperator> localConditionLongOperators;
  std::map<std::string, DoubleOperator> localConditionDoubleOperators;

  static std::map<std::string, StringOperator> globalConditionStringOperators;
  static std::map<std::string, LongOperator> globalConditionLongOperators;
  static std::map<std::string, DoubleOperator> globalConditionDoubleOperators;

  /* caches and so... */

  /* operator helpers */
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
    bool update;
  } Stats;
  #endif

  inline void resetStats()
  {
    #if SILICON_DEBUG
    Stats.line =1;
    Stats.pos =1;
    Stats.keywords =0;
    Stats.functions =0;
    Stats.update = true;
    #endif
  }

  inline void addKeywordToStats()
  {
    #if SILICON_DEBUG
    Stats.keywords++;
    #endif
  }

  inline void stopStatsUpdate()
  {
    #if SILICON_DEBUG
    Stats.update = false;
    #endif
  }

  inline void ahead(char ** ptr, long howmany=1)
  {
    #if SILICON_DEBUG
    while ( (howmany-->0) && (**ptr != '\0') )
      {
	++*ptr;
	if (Stats.update)
	  {
	    ++Stats.pos;
	    if (**ptr == '\n')
	      {
		Stats.line++;
		Stats.pos=1;
	      }
	  }
      }
    Stats.update = true;
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
