#include "Support.h"
#include "Sv.h"
#include "Wk.h"

using namespace std;

int main(int argc, char **argv)
{
  try
  {
    (void)argc;
    (void)argv;
    Sv sv;
    sv.set_Server();
  }
  //===================================================================
  //===================================================================
  catch (Exception err)
  {
    int err_code = err.get_err_code();
    string err_name = err.get_err_name();
    string name = err.get_name();
    printf("ERR\nname     :: %s\nerr type :: %s\nerr code :: %d\n",
           name.c_str(), err_name.c_str(), err_code);
    return -1;
  }
  return 0;
}
