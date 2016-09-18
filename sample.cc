#include <iostream>
#include <string>
#include "silicon.h"

using namespace std;

string includeCss (Silicon* s, Silicon::StringMap args, std::string input)
{
  auto file=args.find("file");
  if (file==args.end())
    return "";

  return "<link href=\""+file->second+"\" rel=\"stylesheet\" type=\"text/css\">";
}

int main(int argc, char* argv[])
{
  int sections[6] = {1, 0, 1, 1, 1, 1};

  for (int i=1; i<argc; ++i)
    {
      if (i>6)
	break;
      sections[i-1] = (atoi(argv[i])!=0);
    }

  std::cout << "Enabled sections: ";
  for (int i=0; i<6; ++i)
    cout << sections[i]<<"\t";
  cout << endl;

  Silicon::setGlobalKeyword("ProjectTitle", "Silicon Example");
  Silicon::setGlobalKeyword("Author", "Gaspar Fernández");
  Silicon::setGlobalKeyword("AuthorEmail", "gaspar.fernandez@totaki.com");
  Silicon t = Silicon::createFromFile("sample0.html", "views/");
  //  t.setBasePath("views/");
  t.setLayout("sample_layout0.html");
  t.setKeyword("PageTitle", "Main");
  for (int i=0; i<6; ++i)
    t.setKeyword("Section"+to_string(i), to_string(sections[i]));
  t.setFunction("includeCss", includeCss);

  try
    {
      cout << t.render()<<std::endl;
    }
  catch (SiliconException &e)
    {
      cout << "Exception!!! "<<e.what() << std::endl;
    }
}
