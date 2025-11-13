#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
//===============================
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
//===============================

using namespace std;

class Exception
{
  private:
    string name;
    string err_name;
    int err_code;

  public:
    Exception(){}
    Exception(string& name, string& err_name, int err_code) 
    {
      this->name = name;
      this->err_name = err_name;
      this->err_code = err_code;
    }
    string get_name() {return this->name;}
    string get_err_name() {return this->err_name;}
    int get_err_code() {return this->err_code;}
    ~Exception() {}

};

class check_err
{
  private:
  public:
    check_err () {}
    ~check_err () {}

    template<typename T>
      static int check(const char* name, T ret, T f_value)
      {
        if(ret == f_value)
        {
          string s_name = name;
          string err_name = strerror(errno);
          Exception err (s_name, err_name, errno);
          throw err;
        }
        printf("success %s\n", name);
        return 0;
      }

    template<typename T>
      static int check_ENOENT(const char* name, T ret, T f_value)
      {
        if(ret == f_value && errno != ENOENT)
        {
          string s_name = name;
          string err_name = strerror(errno);
          Exception err (s_name, err_name, errno);
          throw err;
        }
        printf("success %s\n", name);
        return 0;
      }

    template<typename T>
      static int check(const string name, T ret, T f_value)
      {
        if(ret == f_value)
        {
          string s_name = name;
          string err_name = strerror(errno);
          Exception err (s_name, err_name, errno);
          throw err;
        }
        printf("success %s\n", name);
        return 0;
      }
};

class RALL_fd
{
  private:
    int fd;
    string name;

  public:
    RALL_fd(int fd, const char* name) 
    {
      this->fd = fd;
      this->name = name;
    }

    ~RALL_fd()
    {
      int ret_close = close(this->fd);
      string check_name("~RALL_fd ");
      check_name += this->name;
      check_name += "close() :: ";
      check_name += to_string(fd);
      check_err::check(check_name, this->fd, -1);
    }


};

//==========================================================================
//==========================================================================

int main(int argc, char **argv) 
{
  try{

  }
  //===================================================================
  //===================================================================
  catch(Exception err)
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

