/**
*************************************************************
* @file silicon.cpp
* @brief Breve descripción
* Pequeña documentación del archivo
*
*
*
*
*
* @author Gaspar Fernández <blakeyed@totaki.com>
* @version
* @date 18 sep 2015
* Historial de cambios:
*
*
*
*
*
*
*
*************************************************************/

#include "silicon.h"
#include <cstring>
#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>

#define MAX(x ,y) ((x) > (y) ? (x) : (y))
#define MIN(x ,y) ((x) < (y) ? (x) : (y))

namespace
{
  static struct
  {
    /* data will be MAXBUFFERLEN bytes max. */
    /* why? if we put a corrupt string, it won't abort the program. */
    long maxBufferLen=MAXBUFFERLEN;

    /* If keyword didn't match, just write it */
    bool leaveUnmatchedKwds=true;

  } globalConfig;

  struct AbsOperate
  {
    virtual bool apply(std::string op) =0;
  };

  template <typename T>
  struct Operate : AbsOperate
  {
    Operate(T a, T b, std::function<bool(std::string, T, T)>opcb): a(a), b(b), opcallback(opcb)
    {
    }

    bool apply(std::string op)
    {
      if (op == "==")
	return (a == b);
      else if (op == "!=")
	return (a != b);
      else if (op == ">)")
	return (a>b);
      else if (op == ">=")
	return (a>=b);
      else if (op == "<")
	return (a<b);
      else if (op == "<=")
	return (a<b);
      else if (op[0]=='!')
	return this->opcallback(op, a, b);
      else
	throw SiliconException(18, "Unknown operator "+op, 0, 0);
      /* functions... */
    }

  private:
    T a;
    T b;
    std::function<bool(std::string, T, T)> opcallback;
  };

  struct OpController
  {
    /* The function is a callback for external operator */
    OpController(std::string a, std::string b, std::function<bool(std::string, std::string, std::string)> opcb)
    {
      operate = new Operate<std::string>(a, b, opcb);
    }

    OpController(long double a, long double b, std::function<bool(std::string, long double, long double)> opcb)
    {
      operate = new Operate<long double>(a, b, opcb);
    }

    OpController(long long a, long long b, std::function<bool(std::string, long long, long long)> opcb)
    {
      operate = new Operate<long long>(a, b, opcb);
    }

    virtual ~OpController() { }
    bool apply(std::string op)
    {
      return operate->apply(op);
    }
  private:
    AbsOperate* operate;
  };
};
/* Silicon & Silicon::createFromFile(std :: std::string & file, long maxBufferLen) */
/* { */
/* } */

Silicon::Silicon(const char * data, long maxBufferLen):maxBufferLen(maxBufferLen)
{
  /* Configure this instance max buffer length */
  if (this->maxBufferLen==0)
    this->maxBufferLen = globalConfig.maxBufferLen;

  this->leaveUnmatchedKwds = globalConfig.leaveUnmatchedKwds;

  std::size_t len = MIN(strlen(data), this->maxBufferLen);
  /* this->_data.reserve(len); */
  this->_data = (char*) malloc(sizeof(char)*(len+1));
  /* this->_data.insert(0, data, this->maxBufferLen); /\* strncpy does not *\/ */
  this->_data = strncpy(this->_data, data, len);

  this->_data[len] = '\0';	 /* guarantee terminated strings. */
}

Silicon::~Silicon()
{
  if (_data)
    free(_data);
}

/* Silicon & Silicon::createFromFile(const char * file, long maxBufferLen) */
/* { */
/* } */

Silicon Silicon::createFromStr(std::string & data, long maxBufferLen)
{
  return Silicon(data.c_str(), maxBufferLen);
}

/* Silicon & Silicon::createFromStr(const char * data, long maxBufferLen) */
/* { */
/* } */

std::string Silicon::render()
{
  std::string out;
  resetStats();
  parse(out, this->_data);
  return out;
}

Silicon::Silicon(Silicon && sil)
{
  std::cout<< "MOOOOOVE"<<std::endl;
  this->_data = sil._data;
  sil._data=NULL;
}

Silicon::Silicon(const Silicon & sil)
{
  std::cout << "COPIA!!"<<std::endl;
}

long Silicon::parse(std :: string & destination, char * strptr, bool write, std::string nested, int level)
{
  bool end = false;
  std::string temp;
  std::string tempData;
  std::map<std::string, std::string> tempArgs; /* Arguments*/
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
	      std::cout << temp << " es una keyword"<<std::endl;
	      if (write)
		destination+=putKeyword(temp);
	      strptr+=moved;
	    }
	  else if ( (moved=parseFunction(strptr, type, temp, tempArgs, autoClosed)) >0 )
	    {
	      tempData.clear();
	      std::cout << temp << " es una funcion con argumentos."<<std::endl;
	      for (auto x : tempArgs)
		{
		  std::cout << "   - "<<x.first<<" = *"<<x.second<<"*"<<std::endl;
		}
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
		      auto f = localFunctions.find(temp);
		      if (f == localFunctions.end())
			throw SiliconException(8, "Undefined funtion "+temp+".", getCurrentLine(), getCurrentPos());

		      destination+=f->second(tempArgs, tempData);
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
	      std::cout << "no hay keyword"<<std::endl;
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
  std::cout <<"KW: "<<strptr<<std::endl;
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

long Silicon::parseFunction(char* strptr, int &type, std::string& fname, std::map<std::string, std::string> &arguments, bool &autoClosed)
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
  std::cout <<"FUNC: "<<strptr<<std::endl;
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
	  std::cout << "ENCLOSANDO"<<std::endl;
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
  std::cout <<"CLOSE: "<<strptr<<std::endl;
  #endif

  ahead(&cursor, 2);

  while (*cursor!='\0')
    {
      if ( (*cursor=='}') && (cursor[1]=='}') )
	{
	  if (temp != closeName)
	    {
	      std::cout << "*"<<temp<<"***"<<closeName<<"****"<<std::endl;
	      throw SiliconException(6, "Unmatching close string", getCurrentLine(), getCurrentPos());
	    }
	  return cursor-strptr+1;
	}
      else
	temp+=*cursor;

      ahead(&cursor);
    }

  /* If we are here, it's probably a corrupt file */
  throw SiliconException(5, "Unterminated keyword close string", getCurrentLine(), getCurrentPos());
}

long Silicon::computeBuiltin(char* strptr, std::string &destination, std::string bif, std::map<std::string, std::string> &arguments, bool &autoClosed, bool write, int level)
{
  if ( (autoClosed) && ( (bif == "if") || (bif == "while") || (bif == "for" ) ) )
    throw SiliconException(10, "Builtin "+bif+" can't be autoclosed", getCurrentLine(), getCurrentPos());

  if (bif == "if")
    return computeBuiltinIf(strptr, destination, arguments, write, level);
  else
    throw SiliconException(11, "Builtin function "+bif+" not implemented", getCurrentLine(), getCurrentPos());

  return 0;
}

long Silicon::computeBuiltinIf(char* strptr, std::string &destination, std::map<std::string, std::string> &arguments, bool write, int level)
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
	return (!getKeyword(condition).empty());
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
	  b = b.substr(1, -1);
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

      std::cout << "NUMERIC: "<<numeric<<std::endl;
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
  size_t oplen=-1;
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

void Silicon::setOperator(std::string name, std::function<bool(Silicon*, std::string, std::string)> func)
{
  localConditionStringOperators["!"+name+"!"] = func;
}

void Silicon::setOperator(std::string name, std::function<bool(Silicon*, long long, long long)> func)
{
  localConditionLongOperators[name] = func;
}

void Silicon::setOperator(std::string name, std::function<bool(Silicon*, long double, long double)> func)
{
  localConditionDoubleOperators[name] = func;
}

void Silicon::setKeyword(std::string kw, std::string text)
{
  localKeywords[kw] = text;
}

bool Silicon::getKeyword(std::string kw, std::string &text)
{
  auto index = localKeywords.find(kw);
  if (index != localKeywords.end())
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
      if (this->leaveUnmatchedKwds)
	return "{{"+keyword+"}}";

      return "";
    }
  else
    return text;
}

void Silicon::setFunction(std::string name, TemplateFunction callable)
{
  localFunctions[name] = callable;
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
