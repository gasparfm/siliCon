/* @(#)silicon.h
 */

#ifndef _SILICON_H
#define _SILICON_H 1

#include <string>
#include <exception>
#include <functional>
#include <map>
#include <vector>

/** This will be our default maximum buffer length. Templates must not exceed
 this size in bytes */
#define MAXBUFFERLEN 16384

/**
 * Silicon debug, stores additional stats information.
 */
#ifndef SILICON_DEBUG
  #ifdef DEBUG
    #define SILICON_DEBUG 1
  #else
    #define SILICON_DEBUG 0
  #endif
#endif

/**
 * Silicon Exceptions
 * These exceptions store line and position to know if
 * we have any syntax error.
 */
class SiliconException : public std::exception
{
 public:
  /**
   * Silicon Exception
   * @param code
   * @param message
   * @param line
   * @param pos Character in line
   */
 SiliconException(const int& code, const std::string &message, long line, long pos): _code(code), _line(line), _pos(pos)
  {
    _message = "Error "+std::to_string(_code)+": "+message;
    #if SILICON_DEBUG
    _message+=" on line "+std::to_string(_line)+":"+std::to_string(_pos);
    #endif
  }

  virtual ~SiliconException() throw ()
  {
  }

  /**
   * Gets exception message. With code and position
   */
  const char* what() const throw()
  {
    return _message.c_str();
  }

  /**
   * Gets error code
   */
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

/**
 * Silicon main clase
 */
class Silicon
{
public:
  /**
   * Possible layout sources
   */
  enum LayoutType
  {
    FILE,
    DATA
  };

  /** StringMap is a map string:string used for:
   *   - keywords
   *   - collections (a collection is a vector of StringMap
   *   - function or condition arguments 
   */
  using StringMap = std::map<std::string, std::string>;

  /**
   * It describes an external function
   */
  using TemplateFunction = std::function<std::string(Silicon*, StringMap, std::string)>;

  /**
   * When we have several functions we call them by their name
   */
  using FunctionMap = std::map<std::string, TemplateFunction>;

  /**
   * Used to compare strings
   */
  using StringOperator = std::function<bool(Silicon*, std::string, std::string)>;

  /**
   * Used to campare long
   */
  using LongOperator = std::function<bool(Silicon*, long long, long long)>;

  /**
   * Used to compare double
   */
  using DoubleOperator = std::function<bool(Silicon*, long double, long double)>;

  /**
   * Destroy !!!
   */
  virtual ~Silicon();

  /**
   * Create instance from file.
   * @param file File to use as template
   * @param defaultPath Default path for files (layouts, blocks...)
   * @param maxBufferLen Default Maximum buffer length for each file
   */
  static Silicon createFromFile(std::string& file, std::string defaultPath="", long maxBufferLen=0);
  static Silicon createFromFile(const char* file, const char* defaultPath=NULL, long maxBufferLen=0);

  /**
   * Create instance from string
   * @param data String to use
   * @param maxBufferLen Maximum buffer length
   */
  static Silicon createFromStr(std::string& data, long maxBufferLen=0);
  static Silicon createFromStr(const char* data, long maxBufferLen=0);

  /**
   * Renders template
   *
   * @useLayout Also renders layout. The template will render inside the
   * layout
   * @return output string
   */
  std::string render(bool useLayout=true);

  Silicon(Silicon&& sil);

  /* Basic getters/setters */

  /**
   * Sets base path
   *
   * @param newval New base path. Applied locally
   */
  inline void setBasePath(std::string newval)
  {
    this->localConfig.basePath = newval;
  }

  /**
   * Sets base path globally, not just for this template.
   * It's static-called!
   *
   * @param newval New base path
   */
  static inline void SetBasePathGlobal(std::string newval);

  /**
   * Gets base path
   *
   * @return Current base path
   */
  inline std::string getBasePath()
  {
    return this->localConfig.basePath;
  }

  /**
   * Setter for LeaveUnmatchedKwds. Whether to leave or remove
   * unmatched keywords
   *
   * @param newval New value
   */
  inline void setLeaveUnmatchedKwds(bool newval)
  {
    this->localConfig.leaveUnmatchedKwds = newval;
  }

  /**
   * Setter for global  leave unmatched keywords setting
   * It's static-called!
   *
   * @param newval New value
   */
  static inline void setLeaveUnmatchedKwdsGlobal(bool newval);

  /** 
   * Getter  
   * 
   * @return Current leaveUnmatchedKwds value
   */
  inline bool getLeaveUnmatchedKwds()
  {
    return this->localConfig.leaveUnmatchedKwds;
  }

  /**
   * Setter for max. buffer length
   *
   * @param newval New value
   */
  inline void setMaxBufferLen(long newval)
  {
    this->localConfig.maxBufferLen = newval;
  }

  /**
   * Setter for global max. buffer length setting
   *
   * @param newval New value
   */
  static inline void setMaxBufferLenGlobal(long newval);

  /** 
   * Getter for max. buffer length
   *
   * @return Current max. buffer length
   */
  inline long getMaxBufferLen()
  {
    return this->localConfig.maxBufferLen;
  }

  /* Layouts related methods */

  /**
   * Sets layout. It's global, but must instance the class
   *
   * @param ltype Layout type (FILE, DATA)
   * @param layout File name or string
   */
  void setLayout(LayoutType ltype, const char* layout);

  /**
   * Sets file layout
   *
   * @param file Filename to use as layout
   */
  void setLayout(std::string file);

  /* Keywords related methods */

  /**
   * Set new local keyword
   *
   * @param kw Keyword. Without {{ }}
   * @param text Text to replace the keyword.
   */
  void setKeyword(std::string kw, std::string text);

  /**
   * Sets global keyword for all instances.
   *
   * @param kw Keyword. Without {{ }}
   * @param text Texto to replace the keyword
   */
  static void setGlobalKeyword(std::string kw, std::string text);

  /**
   * Gets keyword. First try local, then global
   *
   * @param kw Keyword
   *
   * @return Text if found, empty string if not
   */
  std::string getKeyword(std::string kw);

  /**
   * Gets keyword, better method to know if it's defined or not
   *
   * @param kw Keyword
   * @param text Output keyword's text, if found
   *
   * @return whether if keyword exists or not
   */
  bool getKeyword(std::string kw, std::string &text);

  /**
   * Sets special keyword for layouts to obtain template contents
   *
   * @param newck New keyword
   */
  static void setContentsKeyword(std::string newck);

  /**
   * Gets the special keyword's name
   *
   * @return keyword
   */
  static std::string getContentsKeyword();

  /* Collections related methods*/

  /**
   * Adds new collection, associated to network. You must pass the
   * whole collection vector. Maybe it's more useful to use
   * addToCollection methods.
   *
   * @param kw Keyword
   * @param coll Collection vector
   */
  void addCollection(std::string kw, std::vector<StringMap> coll);

  /**
   * Adds map to collection, as one element of collection vector.
   * If the collection doesn't exist, creates new one
   *
   * @param kw Keyword
   * @param content Content map for collection
   */
  void addToCollection(std::string kw, StringMap content);

  /**
   * Adds string pair to collection vector in the given position.
   * If the collection doesn't exist, creates new one
   * If the position is negative or doesn't exist, creates new position
   *
   * @param kw Keyword
   * @param pos Position
   * @param key String map key
   * @param val String map value
   */
  long addToCollection(std::string kw, long pos, std::string key, std::string val);

  /* Functions related methods */

  /**
   * Adds or replaces function
   *
   * @param name Name of function
   * @param callable C++ function to call. Functions are
   *                 std::string(Silicon* s, std::map<std::string, std::string> arguments, std::string input)
   */
  void setFunction(std::string name, TemplateFunction callable);

  /**
   * Adds or replaces global function (not just for this instance)
   *
   * @param name Name of function
   * @param callable (@see setFunction)
   */
  static void setGlobalFunction(std::string name, TemplateFunction callable);

  /* Operators related function */

  /**
   * Sets operator for strings. It will be a function to compare two strings
   *
   * @param name Operator's name
   * @param func Callback (std::function<bool(Silicon*, std::string, std::string)>)
   */
  void setOperator(std::string name, StringOperator func);

  /**
   * Sets operator for longs. It will be a function to compare two numbers
   *
   * @param name Operator's name
   * @param func Callback (std::function<bool(Silicon*, long, long)>)
   */
  void setOperator(std::string name, LongOperator func);

  /**
   * Sets operator for doubles. It will be a function to compare two doubles
   *
   * @param name Operator's name
   * @param func Callback (std::function<bool(Silicon*, double, double)>)
   */
  void setOperator(std::string name, DoubleOperator func);

  /**
   * Sets global operator for strings. It will be a function to compare two strings
   *
   * @param name Operator's name
   * @param func Callback (std::function<bool(Silicon*, std::string, std::string)>)
   */
  void setGlobalOperator(std::string name, StringOperator func);

  /**
   * Sets global operator for longs. It will be a function to compare two numbers
   *
   * @param name Operator's name
   * @param func Callback (std::function<bool(Silicon*, long, long)>)
   */
  void setGlobalOperator(std::string name, LongOperator func);

  /**
   * Sets global operator for doubles. It will be a function to compare two doubles
   *
   * @param name Operator's name
   * @param func Callback (std::function<bool(Silicon*, double, double)>)
   */
  void setGlobalOperator(std::string name, DoubleOperator func);

protected:
  /* Protected methods. Constructor */

  /**
   * Constructor to create instance from string
   *
   * @param data String
   * @param maxBufferLen Change max Buffer Len. If 0 will use global setting
   */
  Silicon(const char* data, long maxBufferLen);

  /**
   * Constructor to create instance from file
   *
   * @param file File name
   * @param defaultPath Default path for all views. 
   *                    If empty string, will use global setting
   * @param maxBufferLen Change max Buffer Len. If 0 will use global setting
   */
  Silicon(const char* file, const char* defaultPath, long maxBufferLen);

  /* Parsing and string building */

  /**
   * Parses template data
   *
   * @param destination Destination string
   * @param strptr Pointer to data source
   * @param write Whether to write parsed contents to destination or not
   * @param nested When we are parsing a function or condition. It's a nested case
   * @param level Nesting level we are parsing now
   *
   * @return Data read from strptr
   */
  long parse(std::string& destination, char* strptr, bool write=true, std::string nested="", int level=0);

  /**
   * Parse keyword {{keyword}}
   *
   * @param strptr Pointer to data source
   * @param keyword Returns the keyword we've parsed
   *
   * @return Data read from strptr (0 if not a keyword and nothing parsed)
   */
  long parseKeyword(char* strptr, std::string& keyword);

  /**
   * Parse function {{!function}} or {{%function}}
   *
   * @param strptr Pointer to data source
   * @param type   Function type (0 - User function, 1 - Builtin method
   * @param fname  Returns function name we've parsed
   * @param arguments Returns arguments extracted
   * @param autoClosed Returns whether the function is autoClosed or not.
   *                   If not autoClosed, will parse data inside the function
   *
   * @return Data read from strptr (0 if not a function and nothing parsed)
   */
  long parseFunction(char* strptr, int &type, std::string& fname, StringMap &arguments, bool &autoClosed);

  /**
   * Parse closing tag {/clostag}}
   *
   * @param strptr Pointer to data source
   * @param closeName Name of tag to close
   *
   * @return Data read from strptr ((0 if not a closing tag and nothing parsed)
   */
  long parseCloseNested(char* strptr, std::string closeName);

  /**
   * Return keyword or leave it like this, depending on configuration
   *
   * @param keyword Keyword to put
   *
   * @return Contents of the keyword
   */
  std::string putKeyword(std::string keyword);

  /* Helpers */

  /**
   * We'd want to get a numeric argument as a number. But it may have
   * errors, or it may not exist, so we make a secure method
   *
   * @param args Arguments map
   * @param argument Numeric argument we are asking for
   * @param defaultVal Default value as long
   * @param required Don't return defaultVal, throw an exception
   *
   * @return Numeric value
   */
  long getNumericArgument(StringMap &args, std::string argument, long defaultVal=0, bool required=false);

  /**
   * Compute internal builtin function
   *
   * @param strptr Pointer to data source
   * @param destination Destination string
   * @param bif Built-In Function
   * @param arguments Arguments for builtin function
   * @param autoClosed Is it autoClosed?
   * @param write whether to write on destination or not
   * @param level Nesting level (for debugging or limiting)
   *
   * @return Data read from strptr (0 if nothing read)
   */
  long computeBuiltin(char* strptr, std::string &destination, std::string bif, StringMap &arguments, bool &autoClosed, bool write, int level);

  /**
   * Compute conditionals (internal builtin function if)
   *
   * @param strptr Pointer to data source
   * @param destination Destination string
   * @param arguments Arguments for builtin function
   * @param write whether to write on destination or not
   * @param level Nesting level (for debugging or limiting)
   *
   * @return Data read from strptr (0 if nothing read)
   */
  long computeBuiltinIf(char* strptr, std::string &destination, StringMap &arguments, bool write, int level);

  /**
   * Compute loops in collections (builtin function collection)
   *
   * @param strptr Pointer to data source
   * @param destination Destination string
   * @param arguments Arguments for builtin function
   * @param write whether to write on destination or not
   * @param level Nesting level (for debugging or limiting)
   *
   * @return Data read from strptr (0 if nothing read)
   */
  long computeBuiltinCollection(char* strptr, std::string &destination, StringMap &arguments, bool write, int level);

  /**
   * Looks for function. First in local functions, then in global functions
   *
   * @param fun Function name
   *
   * @return function
   */
  TemplateFunction getFunction(std::string fun);

  /**
   * Evaluate boolean condition
   *
   * @param condition condition as string
   *
   * @return is it true or false?
   */
  bool evaluateCondition(std::string condition);

  /**
   * Separate arguments will reorder keys and values when
   * arguments have equal sign.
   *
   * @param arguments Original arguments
   *
   * @return New StringMap with right arguments
   */
  StringMap separateArguments(StringMap &arguments);

  /* Operators' stuff */

  /**
   * Perform special boolean operations on strings
   *
   * @param op Operation (must be declared in @see localConditionStringOperators)
   * @param a  First element to compare
   * @param b  Second element to compare
   *
   * @param result
   */
  bool conditionStringOperator(std::string op, std::string a, std::string b);

  /**
   * Perform special boolean operations on long
   *
   * @param op Operation (must be declared in @see localConditionLongOperators)
   * @param a  First element to compare
   * @param b  Second element to compare
   *
   * @param result
   */
  bool conditionDoubleOperator(std::string op, long double a, long double b);

  /**
   * Perform special boolean operations on doubles
   *
   * @param op Operation (must be declared in @see localConditionDoubleOperators)
   * @param a  First element to compare
   * @param b  Second element to compare
   *
   * @param result
   */
  bool conditionLongOperator(std::string op, long long a, long long b);

  /**
   * Gets current line
   *
   * @return current line when parsing
   */
  long getCurrentLine();

  /**
   * Gets current position
   *
   * @return current character when parsing
   */
  long getCurrentPos();

  /* Default global functions */

  /**
   * Function date. Accepts options:
   *     "format"="xxxxxx" where xxxxxx is a string with special keywords
   *                       to specify the format, as put_time or strftime
   *
   * @param s Silicon instance
   * @param options Options for function
   *
   * @return return string
   */
  std::string globalFuncDate(Silicon* s, StringMap options);

  /**
   * Function block. Gets contents of a block template
   *
   * @param s Silicon instance
   * @param options Options for function
   *
   * @return return string
   */
  std::string globalFuncBlock(Silicon* s, StringMap options);

  /**
   * Sets the value of a variable (here, keyword)
   *
   * @param s Silicon instance
   * @param options Options for function (keyword=value, keyword2=value2, and so)
   *
   * @return Empty string
   */
  std::string globalFuncSet(Silicon* s, StringMap options);

  /**
   * Increment the value of a variable
   *
   * @param s Silicon instance
   * @param options Options for function (variable="discarded", variable2="discarded", and so)
   *                Just get the keys, all values are discarded
   *
   * @note If keyword does not exist, creates one with value 1
   * @note If keyword is not numeric, replaces it with 1
   * @return Empty string
   */
  std::string globalFuncInc(Silicon* s, StringMap options);

  /**
   * Gets current directory. No arguments, if you put sth. will
   * be ignored
   *
   * @param s Silicon instance
   * @param options Options for function
   *
   * @return return string
   */
  std::string globalFuncPwd(Silicon* s, StringMap options);

private:
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
