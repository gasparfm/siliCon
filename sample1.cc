#include <iostream>
#include <string>
#include "silicon.h"

using namespace std;

std::string calculadora(Silicon* s, std::map<std::string, std::string> args, std::string input)
{
  return "42";
}

bool strtest(Silicon* s, std::string a, std::string b)
{
  return true;
}


int main()
{
  string templatestr = "Donde dije {{digo}}, digo {{Diego}}.\n{{Diego}} calcula: {!calcula/} grados. \nAhora {!calcula con argumentos/} y {!calcula con \"muchos \\\"argumentos\"/} y {!calcula con clave=\"valor 1\" clave2=   valor2 ea!/}\n ahora {!calcula cosa=KAKA}}ak{/calcula}}\n"
    "Condicion: \n {%if numero!test!\"12.3\"}}kakapedo{/if}}\n"
    "Silicon version {{SiliconVersion}}\n"
    "Personas:\n"
    "{%collection var=people loops=400}}\n"
    "  Nombre: {{people.nombre}}\n"
    "  Edad: {{people.edad}}\n"
    "  Iteracion: {{people._lineNumber}}\n"
    "  Total de lineas: {{people._totalLines}}\n"
    "  Total de lineas: {{people._totalIterations}}\n"
    "  {%if people._last}}SOY EL ULTIMOOOOO\n{/if}}"
    "---------------------\n"
    "{/collection}}\n"
    "===========================\n"
    "{%collection var=kaka}}"
    "  NAME: {{kaka.name}}  | AGE: {{kaka.age}}\n"
    "{/collection}}\n"
    "=========================\n"
    "{!block template=bloque.html/}\n"
    "Total keywords: {!SiliconTotalKeywords/}\n";
  Silicon::setGlobalKeyword("ProjectTitle", "Silicon Example");
  Silicon::setGlobalKeyword("Author", "Gaspar Fernández");
  Silicon::setGlobalKeyword("AuthorEmail", "gaspar.fernandez@totaki.com");
  /* Silicon t = Silicon::createFromFile("layout.html", "views/"); */
  Silicon t = Silicon::createFromStr(templatestr);
  std::vector<Silicon::StringMap> people;
  people.push_back({ {"nombre", "Gaspar"}, {"edad", "32"} });
  people.push_back({ {"nombre", "Mariah"}, {"edad", "31"} });
  people.push_back({ {"nombre", "Abel"}, {"edad", "18"} });
  people.push_back({ {"nombre", "Bartolome"}, {"edad", "49"} });
  people.push_back({ {"nombre", "Carlos"}, {"edad", "29"} });

  t.setBasePath("views/");
  /* t.setLayout("layout.html"); */
  t.setKeyword("PageTitle", "Main");
  t.setKeyword("digo", "diciendo");
  t.setKeyword("Diego", "Diegueitor");
  t.setKeyword("numero", "112.3");
  t.setFunction("calcula", calculadora);
  t.setOperator("test", strtest);
  /* Meter metodos para meter una linea en una coleccion, meter un elemento en una coleccion */
  t.addCollection("people", people);
  t.addToCollection("people", { {"nombre", "Diego"}, {"edad", "92"} });
  t.addToCollection("kaka", { { "name", "NOMBRE" }, {"age",  "32"} });
  long ndx = t.addToCollection("kaka", 3, "name", "PEDETE");
  std::cout << "ENEDEEQUIS: "<<ndx<<std::endl;
  t.addToCollection("kaka", ndx, "age", "923");
  try
    {
      cout << t.render()<<std::endl;
    }
  catch (SiliconException &e)
    {
      cout << "Exception!!! "<<e.what() << std::endl;
    }
}
