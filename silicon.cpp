/**
*************************************************************
* @file silicon.cpp
* @brief Output templating system for C++11
*
* Provides an easy way to use templates for text output, you
* can output to screen, to a web page (using CGI or C++ server)
*
* @author Gaspar Fernández <gaspar.fernandez@totaki.com>
* @version 0.2
*
* @date 30 aug 2015
*
* Changelog:
*   20150925 : Launched version 0.2
*
* To-do:
*   - More default keywords / conditions / functions
*   - Make setLayout static
*
*************************************************************/

#include "silicon.h"
#include <cstring>
#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <ctime>

#define SILICONVERSION "0.2"
#define DIRECTORY_SEPARATOR '/'

#define MAX(x ,y) ((size_t)(x) > (size_t)(y) ? (x) : (y))
#define MIN(x ,y) ((size_t)(x) < (size_t)(y) ? (x) : (y))

namespace
{
  static struct
  {
    /* data will be MAXBUFFERLEN bytes max. */
    /* why? if we put a corrupt string, it won't abort the program. */
    long maxBufferLen=MAXBUFFERLEN;

    /* If keyword didn't match, just write it */
    bool leaveUnmatchedKwds=true;

    /* Base view path */
    std::string basePath="./";
  } globalConfig;

  /**
   * Operate abstract class.
   * What Operate must have. Used to group operations with 
   * strings/longs/doubles and so.
   */
  struct AbsOperate
  {
    /**
     * Apply operation. Once we have a and b terms, apply operation op
     *
     * @param op Operation to perform (==, !=, >=, <=...)
     *
     * @return logic operation applied
     */
    virtual bool apply(std::string op) =0;
  };

  /**
   * Operates with type T, whatever it is
   */
  template <typename T>
  struct Operate : AbsOperate
  {
    /**
     * Constructor with operads and function
     *
     * @param a Operand a
     * @param b Operand b
     * @param opcb Non-standard operations will ask this function
     */
    Operate(T a, T b, std::function<bool(std::string, T, T)>opcb): a(a), b(b), opcallback(opcb)
    {
    }

    /**
     * Apply operation
     */
    bool apply(std::string op)
    {
      if (op == "==")
	return (a == b);
      else if (op == "!=")
	return (a != b);
      else if (op == ">=")
	return (a>b);
      else if (op == ">=")
	return (a>=b);
      else if (op == "<")
	return (a<b);
      else if (op == "<=")
	return (a<b);
      else if (op[0]=='!')
	return this->opcallback(op.substr(1,op.length()-2), a, b);
      else
	throw SiliconException(18, "Unknown operator "+op, 0, 0);
    }

  private:
    T a;
    T b;
    std::function<bool(std::string, T, T)> opcallback;
  };

  /**
   * Makes use of Operate transparent
   */
  struct OpController
  {
    /**
     * @param a Operand a
     * @param b Operand b
     * @param opcb More operations callback
    */
    OpController(std::string a, std::string b, std::function<bool(std::string, std::string, std::string)> opcb)
    {
      operate = new Operate<std::string>(a, b, opcb);
    }

    /**
     * @param a Operand a
     * @param b Operand b
     * @param opcb More operations callback
    */
    OpController(long double a, long double b, std::function<bool(std::string, long double, long double)> opcb)
    {
      operate = new Operate<long double>(a, b, opcb);
    }

    /**
     * @param a Operand a
     * @param b Operand b
     * @param opcb More operations callback
    */
    OpController(long long a, long long b, std::function<bool(std::string, long long, long long)> opcb)
    {
      operate = new Operate<long long>(a, b, opcb);
    }

    virtual ~OpController() { }

    /**
     * Apply operation
     */
    bool apply(std::string op)
    {
      return operate->apply(op);
    }
  private:
    AbsOperate* operate;
  };

  /**
   * Get filesize for C++
   * @param filename
   * @return file size
   */
  long filesize(std::string filename)
  {
    std::ifstream file( filename, std::ios::binary | std::ios::ate);
    return file.tellg();
  }

  /**
   * The file specified has absolute path?
   * Just for *nix systems. 
   *
   * @param filename
   * @return true if path is absolute
   */
  bool isAbsolutePath(std::string filename)
  {
    /* One day, it will be Windows compatible */
    return (filename[0]=='/');
  }

  /**
   * Concats filename and basePath. Making it a good path
   * basePath can end with / or not. Will be added if needed
   *
   * @param filename File Name
   * @param basePath Base path
   * @param usePath Must use basePath. Use false if filename
   *        will use relative path data.
   *
   * @return path+file
   */
  std::string fixPath(std::string filename, std::string basePath, bool usePath)
  {
    if ( (!usePath) || (basePath.empty()) || (isAbsolutePath(filename)) || (filename[0]=='.') )
      return filename;

    if (basePath.back()!=DIRECTORY_SEPARATOR)
      basePath+=DIRECTORY_SEPARATOR;

    return basePath+filename;
  }

  /**
   * Test if globals are loaded. They will be loaded the first time
   * we instance the class. But globals are static, so they will be
   * used for future Silicon instances
   */
  static struct 
  {
    bool keywords,
      functions,
      conditions;
  } configuredGlobals = {false, false, false};
}

/* As far as I know, GCC 5.2 implements put_time !!!!!!!! */
/* We don't have put_time in earlier GCC versions, so, if we
 use one of them, this function will be used instead.
*/
#if defined(__GNUC__) && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ <= 50200 )
  namespace std
  {
    static std::string put_time( const std::tm* tmb, const char* fmt )
    {
      std::string s( 128, '\0' );
      while( !strftime( &s[0], s.size(), fmt, tmb ) )
  	s.resize( s.size() + 128 );
      return s;
    }
  }
#endif

Silicon::StringMap Silicon::globalKeywords;
Silicon::FunctionMap Silicon::globalFunctions;
std::map<std::string, Silicon::StringOperator> Silicon::globalConditionStringOperators;
std::map<std::string, Silicon::LongOperator> Silicon::globalConditionLongOperators;
std::map<std::string, Silicon::DoubleOperator> Silicon::globalConditionDoubleOperators;
std::string Silicon::contentsKeyword="contents";
char* Silicon::layoutData=NULL;

Silicon Silicon::createFromFile(std::string & file, std::string defaultPath, long maxBufferLen)
{
  return Silicon(file.c_str(), (defaultPath.empty())?NULL:defaultPath.c_str(), maxBufferLen);
}

inline void Silicon::SetBasePathGlobal(std::string newval)
{
  globalConfig.basePath = newval;
}

inline void Silicon::setLeaveUnmatchedKwdsGlobal(bool newval)
{
  globalConfig.leaveUnmatchedKwds = newval;
}

inline void Silicon::setMaxBufferLenGlobal(long newval)
{
  globalConfig.maxBufferLen = newval;
}

Silicon::Silicon(const char * data, long maxBufferLen)
{
  this->localConfig.maxBufferLen = maxBufferLen;
  this->configure();

  this->copyBuffer(&this->_data, data);
}

Silicon::Silicon(const char* file, const char* defaultPath, long maxBufferLen)
{
  this->localConfig.maxBufferLen = maxBufferLen;
  this->localConfig.basePath = (defaultPath)?defaultPath:"";

  this->configure();

  this->extractFile(&this->_data, file);
}

void Silicon::extractFile(char **ptr, std::string filename, bool usePath)
{
  filename = fixPath(filename, this->localConfig.basePath, usePath);

  std::ifstream fd (filename, std::ios::binary | std::ios::ate);
  if (fd.fail())
    throw SiliconException(19, "File "+this->localConfig.basePath+filename+" not found", 0, 0);

  /* Find out file size */
  std::size_t len = MIN((long)fd.tellg(), this->localConfig.maxBufferLen);

  fd.seekg(std::ios::beg);
  *ptr = (char*) malloc(sizeof(char)*(len+1));
  fd.read(*ptr, len);
  (*ptr)[len]='\0';
}

void Silicon::copyBuffer(char **ptr, const char* origin)
{
  std::size_t len = MIN(strlen(origin), this->localConfig.maxBufferLen);

  *ptr = (char*) malloc(sizeof(char)*(len+1));
  *ptr = strncpy(*ptr, origin, len);

  (*ptr)[len] = '\0';	 /* guarantee terminated strings. */

}

void Silicon::configure()
{
  if (this->localConfig.basePath.empty())
    this->localConfig.basePath = globalConfig.basePath;

  /* Configure this instance max buffer length */
  if (this->localConfig.maxBufferLen==0)
    this->localConfig.maxBufferLen = globalConfig.maxBufferLen;

  this->localConfig.leaveUnmatchedKwds = globalConfig.leaveUnmatchedKwds;

  /* Fill global keywords, functions and conditions */
  if (!configuredGlobals.keywords)
    {				/* Just once, when first template is instanced */
      Silicon::setGlobalKeyword("SiliconVersion", SILICONVERSION);
      std::cout << "METO SILICONVERSION"<<std::endl;
      configuredGlobals.keywords = true;
    }

  if (!configuredGlobals.functions)
    {
      Silicon::setGlobalFunction(
				 "SiliconTotalKeywords", 
				 [this] (Silicon* s, StringMap, std::string) { 
				   return std::to_string(globalKeywords.size()+this->localKeywords.size()); 
				 });
      Silicon::setGlobalFunction("date", std::bind(&Silicon::globalFuncDate, this, std::placeholders::_1, std::placeholders::_2));
      Silicon::setGlobalFunction("block", std::bind(&Silicon::globalFuncBlock, this, std::placeholders::_1, std::placeholders::_2));
      configuredGlobals.functions = true;
    }

  if (!configuredGlobals.conditions)
    {
      /* Conditions globals */
      configuredGlobals.conditions = true;
    }
}

std::string Silicon::globalFuncDate(Silicon* s, Silicon::StringMap options)
{
  auto fmt = options.find("format");
  std::time_t now = time(NULL);
  std::tm tm;
  localtime_r( &now, &tm );

  std::string format = ( fmt !=options.end())?fmt->second:"%Y%m%d";
  return std::put_time(&tm, format.c_str());
}

std::string Silicon::globalFuncBlock(Silicon* s, Silicon::StringMap options)
{
  auto tplt = options.find("template");
  if (tplt == options.end())
    throw SiliconException(20, "Block template not found.", getCurrentLine(), getCurrentPos());

  char *blockData;
  std::string res;
  this->extractFile(&blockData, tplt->second);
  this->parse(res, blockData);
  free(blockData);

  return res;
}

void Silicon::addCollection(std::string kw, std::vector<Silicon::StringMap> coll)
{
  localCollections[kw] = coll;
}

void Silicon::addToCollection(std::string kw, StringMap content)
{
  auto el = localCollections.find(kw);
  if (el == localCollections.end())
    localCollections[kw] = std::vector<StringMap>({content});
  else
    localCollections[kw].push_back(content);
}

long Silicon::addToCollection(std::string kw, long pos, std::string key, std::string val)
{
  auto el = localCollections.find(kw);
  if (el == localCollections.end())
    {
      localCollections[kw] = std::vector<StringMap>({ { {key, val} } });
      return 0;			/* position 0*/
    }
  else if ( (pos == -1) || (pos>=(long)el->second.size()) )
    {
      localCollections[kw].push_back( { {key, val} });
      return localCollections[kw].size()-1;
    }
  else
    {
      localCollections[kw][(size_t)pos].insert({key, val});
      return pos;
    }
}


Silicon::~Silicon()
{
  if (_data)
    free(_data);
}

Silicon Silicon::createFromFile(const char * file, const char* defaultPath, long maxBufferLen)
{
  return Silicon(file, defaultPath, maxBufferLen);
}

Silicon Silicon::createFromStr(std::string & data, long maxBufferLen)
{
  return Silicon(data.c_str(), maxBufferLen);
}

Silicon Silicon::createFromStr(const char * data, long maxBufferLen)
{
  return Silicon(data, maxBufferLen);
}

std::string Silicon::render(bool useLayout)
{
  std::string tplt;
  resetStats();
  parse(tplt, this->_data);
  if ((Silicon::layoutData==NULL) || (!useLayout) )
    return tplt;

  setKeyword(Silicon::contentsKeyword, tplt);
  tplt.clear();
  parse(tplt, Silicon::layoutData);
  return tplt;
}

Silicon::Silicon(Silicon && sil)
{
  this->_data = sil._data;
  sil._data=NULL;
}

long Silicon::parse(std :: string & destination, char * strptr, bool write, std::string nested, int level)
{
  bool end = false;
  std::string temp;
  std::string tempData;
  StringMap tempArgs; /* Arguments*/
  char *current = strptr;
  long moved;
  bool autoClosed;
  int type;

  while ( (*strptr!='\0') && (!end) )
    {
      if (*strptr == '{')	// } : put this symbol to make member functions work
	{
	  if ( (moved=parseKeyword(strptr, temp)) >0 )
	    {
	      if (write)
		destination+=putKeyword(temp);
	      strptr+=moved;
	    }
	  else if ( (moved=parseFunction(strptr, type, temp, tempArgs, autoClosed)) >0 )
	    {
	      tempData.clear();
	      /* std::cout << temp << " es una funcion con argumentos."<<std::endl; */
	      /* for (auto x : tempArgs) */
	      /* 	{ */
	      /* 	  std::cout << "   - "<<x.first<<" = *"<<x.second<<"*"<<std::endl; */
	      /* 	} */
	      strptr+=moved;
	      if (type == 0)	/* User function*/
		{
		  if (!autoClosed)
		    {
		      ahead(&strptr);
		      moved = parse(tempData, strptr, write, temp, level+1);
		      strptr+=moved;
		    }
		  if (write)
		    {
		      auto f = getFunction(temp);

		      destination+=f(this, tempArgs, tempData);
		    }
		}
	      else if (type == 1) /* Builtin methods*/
		{
		  if ( (moved=computeBuiltin(strptr, destination, temp, tempArgs, autoClosed, write, level)) >0 )
		    {
		      strptr+=moved;
		    }
		}
	      else
		throw SiliconException(9, "Not implemented function type "+std::to_string(type)+" for function "+temp+".", getCurrentLine(), getCurrentPos());
	    }
	  else if ( (!nested.empty()) && ( (moved=parseCloseNested(strptr, nested)) >0) )
	    {
	      strptr+=moved;
	      return strptr-current+1;
	    }
	  else
	    {
	      destination+='{';	/* Put this in the resulting string*/
	    }
	  /* Maybe a keyword or sth. */
	}
      else if (write)
	destination+=*strptr;

      ahead(&strptr);
    }

  if (level)
    throw SiliconException(7, "Didn't close nested action "+nested+". "+std::to_string(level)+" levels left.", getCurrentLine(), getCurrentPos());

  return strptr-current+1;
}

long Silicon::parseKeyword(char * strptr, std :: string & keyword)
{
  if ( (strptr[1] != '{') || (strptr[2] == '\0') )
    return 0;			/* Not a keyword */

  char* cursor = strptr;			/* Ahead two chars, just the { and read next*/

  #if SILICON_DEBUG
  /* std::cout <<"KW: "<<strptr<<std::endl; */
  #endif
  keyword.clear();

  ahead(&cursor, 2);

  while (*cursor!='\0')
    {
      if ( (*cursor=='}') && (cursor[1]=='}') )
      	return cursor-strptr+1;
      else
	keyword+=*cursor;
      ahead(&cursor);
    }

  /* If we are here, it's probably a corrupt file */
  throw SiliconException(1, "Unterminated keyword string", getCurrentLine(), getCurrentPos());
}

long Silicon::parseFunction(char* strptr, int &type, std::string& fname, Silicon::StringMap &arguments, bool &autoClosed)
{
  type = -1;
  if (strptr[1] == '!')
    type=0;			/* User functions */
  else if (strptr[1] == '%')
    type=1;			/* Built-in methods. Won't parse equal */

  if ( (type ==-1) || (strptr[2] == '\0') )
    return 0;			/* Not a Function */
  
  char* cursor = strptr;			/* Ahead two chars, just the { and read next*/

  std::string temp;				/* Temporary string*/
  std::string key;				/* Current key */
  int autoKey = 0;				/* Autokey to use when there's no key */
  int status = 0;				/* 0 - filling function name, 1 - filling param. key, 2 - filling param. value */
  bool enclosed = false;

  #if SILICON_DEBUG
  /* std::cout <<"FUNC: "<<strptr<<std::endl; */
  #endif
  autoClosed=false;

  fname.clear();
  arguments.clear();

  ahead(&cursor, 2);

  while (*cursor!='\0')
    {
      if ( (*cursor=='}') && (cursor[1]=='}') )
      	break;
      else if ( (*cursor=='/') && (cursor[1]=='}') )
	{
	  autoClosed=true;
	  break;
	}
      else if ( (*cursor==' ') && (!enclosed) && (!temp.empty()) )
	{
	  functionParserFill(status, fname, arguments, temp, key, autoKey);
	}
      else if (*cursor=='"')
	{
	  enclosed=!enclosed;
	  if (type==1)		/* For builtins we must see the enclosing later*/
	    temp+=*cursor;
	}
      else if ( (*cursor=='=') && (!enclosed) && (key.empty()) && (status==2) && (type!=1) )
	{
	  status=1;
	  functionParserFill(status, fname, arguments, temp, key, autoKey);
	}
      else if ( (*cursor=='\\') && ( (cursor[1]=='"') || (cursor[1]=='}') || (cursor[1]=='=') ) )
	{
	  temp+=cursor[1];
	  ahead(&cursor);
	}
      else
	{
	  if ( (*cursor!=' ') || (enclosed) || (!temp.empty()) )
	    temp+=*cursor;
	}

      ahead(&cursor);
    }

  if (*cursor=='\0')
    throw SiliconException(2, "Unterminated function string", getCurrentLine(), getCurrentPos());

  if (enclosed)
    throw SiliconException(4, "Unfinished enclosed string", getCurrentLine(), getCurrentPos());

  functionParserFill(status, fname, arguments, temp, key, autoKey);
  return cursor-strptr+1;
}

long Silicon::parseCloseNested(char* strptr, std::string closeName)
{
  if ( (strptr[1] != '/') || (strptr[2] == '\0') )
    return 0;			/* Not a close nested */

  char* cursor = strptr;			/* Ahead two chars, just the { and read next*/
  std::string temp;

  #if SILICON_DEBUG
  /* std::cout <<"CLOSE: "<<strptr<<std::endl; */
  #endif

  ahead(&cursor, 2);

  while (*cursor!='\0')
    {
      if ( (*cursor=='}') && (cursor[1]=='}') )
	{
	  if (temp != closeName)
	    throw SiliconException(6, "Unmatching close string", getCurrentLine(), getCurrentPos());

	  return cursor-strptr+1;
	}
      else
	temp+=*cursor;

      ahead(&cursor);
    }

  /* If we are here, it's probably a corrupt file */
  throw SiliconException(5, "Unterminated keyword close string", getCurrentLine(), getCurrentPos());
}

Silicon::TemplateFunction Silicon::getFunction(std::string fun)
{
  auto f = localFunctions.find(fun);
  if (f != localFunctions.end())
    return f->second;

  f = globalFunctions.find(fun);
  if (f != globalFunctions.end())
    return f->second;

  throw SiliconException(8, "Undefined funtion "+fun+".", getCurrentLine(), getCurrentPos());
}

long Silicon::computeBuiltin(char* strptr, std::string &destination, std::string bif, Silicon::StringMap &arguments, bool &autoClosed, bool write, int level)
{
  if ( (autoClosed) && ( (bif == "if") || (bif == "while") || (bif == "for" ) || (bif == "collection") ) )
    throw SiliconException(10, "Builtin "+bif+" can't be autoclosed", getCurrentLine(), getCurrentPos());

  if (bif == "if")
    return computeBuiltinIf(strptr, destination, arguments, write, level);
  else if (bif == "collection")
    return computeBuiltinCollection(strptr, destination, arguments, write, level);
  else
    throw SiliconException(11, "Builtin function "+bif+" not implemented", getCurrentLine(), getCurrentPos());

  return 0;
}

Silicon::StringMap Silicon::separateArguments(Silicon::StringMap &arguments)
{
  StringMap tmp;

  for (auto j : arguments)
    {
      auto op = j.second.find("=");
      if (op != std::string::npos)
      	{
	  tmp.insert({j.second.substr(0, op), j.second.substr(op+1)});
      	}
      else
	{
	  tmp.insert(j);
	}
    }
  return tmp;
}

long Silicon::getNumericArgument(Silicon::StringMap &args, std::string argument, long defaultVal, bool required)
{
  auto _arg = args.find(argument);
  if (_arg==args.end())
    {
      if (required)
	throw SiliconException(23, "Required argument "+argument+" not found", getCurrentLine(), getCurrentPos());
      else
	return defaultVal;
    }

  try
    {
      std::string::size_type sz = 0;
      long res = std::stoll(_arg->second, &sz);
      if (sz != _arg->second.length())	/* Everything is not a number*/
	throw SiliconException(24, "Argument "+argument+" MUST be numeric", getCurrentLine(), getCurrentPos());

      return res;
    }
  catch (const std::invalid_argument &inv)
    {
      throw SiliconException(25, "Argument "+argument+" MUST be numeric", getCurrentLine(), getCurrentPos());
    }
}

long Silicon::computeBuiltinCollection(char* strptr, std::string &destination, Silicon::StringMap &arguments, bool write, int level)
{
  arguments =this->separateArguments(arguments);
  auto _var = arguments.find("var");

  if (_var == arguments.end())
    throw SiliconException(21, "Collection not specified", getCurrentLine(), getCurrentPos());

  auto coll = localCollections.find(_var->second);
  if (coll == localCollections.end())
    throw SiliconException(22, "Collection "+_var->second+" not found", getCurrentLine(), getCurrentPos());

  long n = 0;
  long line = 0;
  long totalLines = coll->second.size();

  long iterations = getNumericArgument(arguments, "loops", totalLines);
  if (iterations>totalLines)
    iterations = totalLines;

  /* if (_iterations == arguments.end()) */
  /*   iterations = totalLines; */
  ahead(&strptr);
  this->setKeyword(_var->second+"._totalLines", std::to_string(totalLines));
  this->setKeyword(_var->second+"._totalIterations", std::to_string(iterations));

  for (auto i : coll->second)
    {
      if (line == iterations)
	break;
      else
	this->setKeyword(_var->second+"._last", (line == iterations-1)?"1":"0");

      this->setKeyword(_var->second+"._even", (line%2==0)?"1":"0");

      this->setKeyword(_var->second+"._lineNumber", std::to_string(line));
      for (auto z : i)
	{
	  /* Meter mas variables como el numero de linea,
	     El total de lineas, si la linea es la última o no.
	     Si la línea es par o impar
	     Verificar que %if "0" funciona... */
	  this->setKeyword(_var->second+"."+z.first, z.second);
	}
      if (line>0)
	stopStatsUpdate();

      n = parse(destination, strptr, write, "collection", level+1);

      ++line;
    }

  return n;
}

long Silicon::computeBuiltinIf(char* strptr, std::string &destination, Silicon::StringMap &arguments, bool write, int level)
{
  bool logicResult=false;
  int n = 0;

  if (write)
    {				/* Evaluate expression if write is enabled */
      for (auto x : arguments)
	{
	  if (n)
	    {
	      /* Test OR, AND... */
	    }

	  bool currentCond = evaluateCondition(x.second);
	  if (!n)
	    logicResult = currentCond;
	}
    }
  ahead(&strptr);
  return parse(destination, strptr, logicResult, "if", level+1);
}

bool Silicon::evaluateCondition(std::string condition)
{
  auto op = condition.find_first_of("!<>=");
  if (op == std::string::npos)
    {				/* No operator*/
      /* Numeric statement */
      if (std::all_of(condition.begin(), condition.end(), ::isdigit))
	return (std::stoi(condition));
      else
	{
	  std::string kw = getKeyword(condition);
	  if (std::all_of(kw.begin(), kw.end(), ::isdigit))
	    return (std::stoi(kw));

	  return (!kw.empty());
	}
    }
  else
    {
      std::string a = getKeyword(condition.substr(0, op));
      std::string b;
      std::string _op = getOperator(condition, op, b);
      if (b.empty())
	throw SiliconException(13, "Right value can't be empty", getCurrentLine(), getCurrentPos());

      short numeric;
      long double lda, ldb;
      long long lla, llb;

      if ( (b.front()=='"') && (b.back()=='"') )
	{
	  b = b.substr(1, b.length()-2);
	  numeric = 0;
	}
      else
	{
	  /* Gets long long or long double... */
	  numeric = conditionNumericAB(a, b, lla, llb);
	  if (!numeric)
	    numeric = conditionDoubleAB(a, b, lda, ldb);
	}

      OpController* opc;
      if (numeric == 0)
	opc = new OpController(a, b, std::bind(&Silicon::conditionStringOperator, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      else if (numeric == 1)
      	opc = new OpController(lla, llb, std::bind(&Silicon::conditionLongOperator, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      else if (numeric == 2)
      	opc = new OpController(lda, ldb, std::bind(&Silicon::conditionDoubleOperator, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      else
	throw SiliconException(14, "Numeric type "+std::to_string(numeric)+" not implemented.", getCurrentLine(), getCurrentPos());

      bool res = opc->apply(_op);
      delete opc;
      return res;
    }

  return 0;
}

short Silicon::conditionNumericAB(std::string a, std::string b, long long &lla, long long &llb)
{
  try
    {
      std::string::size_type sz = 0;
      lla = std::stoll(a, &sz);
      if (sz != a.length())	/* Everything is not a number*/
	return 0;

      llb = std::stoll(b, &sz);
      if (sz != b.length())
	return 0;

      return 1;
    }
  catch (const std::invalid_argument &inv)
    {
      return 0;
    }
}

short Silicon::conditionDoubleAB(std::string a, std::string b, long double &lda, long double &ldb)
{
  try
    {
      std::string::size_type sz =0;

      lda = std::stold(a, &sz);
      if (sz != a.length())	/* Everything is not a number*/
	return 0;

      ldb = std::stold(b, &sz);
      if (sz != b.length())	/* Everything is not a number*/
	return 0;
      return 2;
    }
  catch (const std::invalid_argument &inv)
    {
      return 0;
    }
}

/* Operators:
    =  (alias of ==
    == (alias of =)
    !=
    <> (alias of !=>
    >
    >=
    <
    <=
    !i=! (case insensitive equals)
 */
std::string Silicon::getOperator(std::string condition, size_t pos, std::string &b)
{
  long oplen=-1;
  std::string op;

  if (condition[pos]=='!')
    {
      if (condition[pos+1]=='=')
	op="!=";
      else
	{
	  /* may be any string */
	  op = condition.substr(pos, condition.find("!", pos+1)-pos+1);
	  std::transform(op.begin(), op.end(), op.begin(), ::tolower);
	}
    }
  else if (condition[pos]=='=')
    {
      op = "==";
      if (condition[pos+1]!='=')
	oplen=1;
    }
  else if (condition[pos]=='<')
    {
      switch (condition[pos+1])
	{
	case '=':
	  op = "<=";
	  break;
	case '>':
	  op = "!=";
	  break;
	default:
	  op = "<";
	}
    }
  else if (condition[pos]=='>')
    {
      op = (condition[pos+1]=='=')?">=":">";
    }
  if (op.empty())
    throw SiliconException(12, "Unknown operator used in "+condition, getCurrentLine(), getCurrentPos());

  if (oplen==-1)
    {
      oplen = op.length();
    }
  b = condition.substr(pos+oplen);

  return op;
}

bool Silicon::conditionStringOperator(std::string op, std::string a, std::string b)
{
  auto f = this->localConditionStringOperators.find(op);
  if (f != this->localConditionStringOperators.end())
    return f->second(this, a, b);

  throw SiliconException(17, "Invalid condition operator "+op+" for string", getCurrentLine(), getCurrentPos());
}

bool Silicon::conditionDoubleOperator(std::string op, long double a, long double b)
{
  auto f = this->localConditionDoubleOperators.find(op);
  if (f != this->localConditionDoubleOperators.end())
    return f->second(this, a, b);

  throw SiliconException(15, "Invalid condition operator "+op+" for double", getCurrentLine(), getCurrentPos());
}

bool Silicon::conditionLongOperator(std::string op, long long a, long long b)
{
  auto f = this->localConditionLongOperators.find(op);
  if (f != this->localConditionLongOperators.end())
    return f->second(this, a, b);

  throw SiliconException(16, "Invalid condition operator "+op+" for long", getCurrentLine(), getCurrentPos());
}

void Silicon::setOperator(std::string name, Silicon::StringOperator func)
{
  localConditionStringOperators[name] = func;
}

void Silicon::setOperator(std::string name, Silicon::LongOperator func)
{
  localConditionLongOperators[name] = func;
}

void Silicon::setOperator(std::string name, Silicon::DoubleOperator func)
{
  localConditionDoubleOperators[name] = func;
}

void Silicon::setGlobalOperator(std::string name, Silicon::StringOperator func)
{
  globalConditionStringOperators[name] = func;
}

void Silicon::setGlobalOperator(std::string name, Silicon::LongOperator func)
{
  globalConditionLongOperators[name] = func;
}

void Silicon::setGlobalOperator(std::string name, Silicon::DoubleOperator func)
{
  globalConditionDoubleOperators[name] = func;
}

void Silicon::setKeyword(std::string kw, std::string text)
{
  localKeywords[kw] = text;
}

void Silicon::setGlobalKeyword(std::string kw, std::string text)
{
  globalKeywords[kw] = text;
}

bool Silicon::getKeyword(std::string kw, std::string &text)
{
  /* Is a local keyword? */
  auto index = localKeywords.find(kw);
  if (index != localKeywords.end())
    {
      text = index->second;
      return true;
    }

  /* Is a global keyword? */
  index = globalKeywords.find(kw);
  if (index != globalKeywords.end())
    {
      text = index->second;
      return true;
    }

  return false;
}

std::string Silicon::getKeyword(std::string kw)
{
  std::string text;
  if (!getKeyword(kw, text))
    text="";

  return text;
}


std::string Silicon::putKeyword(std::string keyword)
{
  std::string text;

  addKeywordToStats();		/* Stats*/

  if (!getKeyword(keyword, text))
    {
      if (this->localConfig.leaveUnmatchedKwds)
	return "{{"+keyword+"}}";

      return "";
    }
  else
    return text;
}

void Silicon::setFunction(std::string name, Silicon::TemplateFunction callable)
{
  localFunctions[name] = callable;
}

void Silicon::setGlobalFunction(std::string name, Silicon::TemplateFunction callable)
{
  globalFunctions[name] = callable;
}

void Silicon::setLayout(Silicon::LayoutType ltype, const char* layout)
{
  if (Silicon::layoutData!=NULL)
    free(Silicon::layoutData);

  if (ltype==FILE)
    {
      this->extractFile(&Silicon::layoutData, layout);
    }
  else
    {
      this->copyBuffer(&Silicon::layoutData, layout);
    }
}

void Silicon::setLayout(std::string file)
{
  this->setLayout(Silicon::FILE, file.c_str());
}

void Silicon::setContentsKeyword(std::string newck)
{
  Silicon::contentsKeyword = newck;
}

std::string Silicon::getContentsKeyword()
{
  return Silicon::contentsKeyword;
}


long Silicon::getCurrentLine()
{
  #if SILICON_DEBUG
  return Stats.line;
  #endif
  return 0;
}

long Silicon::getCurrentPos()
{
  #if SILICON_DEBUG
  return Stats.pos;
  #endif
  return 0;
}
