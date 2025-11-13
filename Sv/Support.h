#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

// C 표준 라이브러리
#include <cstdio>  // perror, printf
#include <cstdlib> // malloc, free, exit
#include <cstring> // memset, memcpy, strcpy

// UNIX system
#include <unistd.h> // close, read, write, usleep
#include <errno.h>  // errno

// 파일/디렉터리
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>

// IPC (System V & POSIX)
#include <sys/ipc.h>
#include <sys/msg.h> // msgget, msgrcv, msgsnd
#include <mqueue.h>  // mq_open, mq_send, mq_receive

// Signals
#include <signal.h>

// 소켓 관련
#include <sys/types.h> // socket(), bind() 등에 필요
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_addr, htons

using namespace std;

class Exception
{
private:
  string name;
  string err_name;
  int err_code;

public:
  Exception() {}
  Exception(string &name, string &err_name, int err_code)
  {
    this->name = name;
    this->err_name = err_name;
    this->err_code = err_code;
  }
  string get_name() { return this->name; }
  string get_err_name() { return this->err_name; }
  int get_err_code() { return this->err_code; }
  ~Exception() {}
};

class check
{
private:
public:
  check() {}
  ~check() {}

  template <typename T>
  static int ck(const char *name, T ret, T f_value)
  {
    if (ret == f_value)
    {
      string s_name = name;
      string err_name = strerror(errno);
      Exception err(s_name, err_name, errno);
      throw err;
    }
    printf("success %s\n", name);
    return 0;
  }

  template <typename T>
  static int ck_ENOENT(const char *name, T ret, T f_value)
  {
    if (ret == f_value && errno != ENOENT)
    {
      string s_name = name;
      string err_name = strerror(errno);
      Exception err(s_name, err_name, errno);
      throw err;
    }
    printf("success %s\n", name);
    return 0;
  }

  template <typename T>
  static int ck(const string name, T ret, T f_value)
  {
    if (ret == f_value)
    {
      string s_name = name;
      string err_name = strerror(errno);
      Exception err(s_name, err_name, errno);
      throw err;
    }
    printf("success %s\n", name.c_str());
    return 0;
  }

  template <typename T>
  static int ck_r(const string name, T ret, T f_value)
  {
    if (ret != f_value)
    {
      string s_name = name;
      string err_name = strerror(errno);
      Exception err(s_name, err_name, errno);
      throw err;
    }
    printf("success %s\n", name.c_str());
    return 0;
  }
};

//==========================================================================
//==========================================================================

class RAII_nomal
{
private:
public:
  RAII_nomal()
  {
  }

  ~RAII_nomal()
  {
    for (auto &v : vec)
    {
      close(v.first);
      cout << "Close ::  " + v.second << "\n";
    }
  }

  vector<pair<int, string>> vec;
};

//==========================================================================
//==========================================================================

class RAII_soc
{
private:
public:
  RAII_soc()
  {
  }

  ~RAII_soc()
  {
    for (auto &v : vec)
    {
      shutdown(v.first, SHUT_WR);
      close(v.first);
      cout << "Close ::  " + v.second << "\n";
    }
  }

  vector<pair<int, string>> vec;
};

//==========================================================================
//==========================================================================

struct st_RAII_epoll
{
  int ep_fd;
  int fd;
  string name;
};

class RAII_epoll
{
private:
public:
  RAII_epoll()
  {
  }

  ~RAII_epoll()
  {
    for (auto &v : vec)
    {
      epoll_ctl(v.ep_fd, EPOLL_CTL_DEL, v.fd, nullptr);
      cout << "delete ::  " + v.name << "\n";

      close(v.ep_fd);
      cout << "Close ::  " + v.name << "\n";
    }
  }

  vector<st_RAII_epoll> vec;
};

//==========================================================================
//==========================================================================

class RAII_pipe
{
private:
public:
  RAII_pipe()
  {
  }

  ~RAII_pipe()
  {
    for (auto &v : vec)
    {
      close(v.first[0]);
      close(v.first[1]);
      cout << "Close ::  " + v.second << "\n";
    }
  }

  vector<pair<int[2], string>> vec;
};