#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// C 표준 라이브러리
#include <cstdio>  // perror, printf
#include <cstdlib> // malloc, free, exit
#include <cstring> // memset, memcpy, strcpy

// UNIX system
#include <errno.h>  // errno
#include <unistd.h> // close, read, write, usleep

// 파일/디렉터리
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h>

// IPC (System V & POSIX)
#include <mqueue.h> // mq_open, mq_send, mq_receive
#include <sys/ipc.h>
#include <sys/msg.h> // msgget, msgrcv, msgsnd

// Signals
#include <signal.h>

// 소켓 관련
#include <arpa/inet.h>  // inet_addr, htons
#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h>
#include <sys/types.h> // socket(), bind() 등에 필요

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
  string get_name()
  {
    return this->name;
  }
  string get_err_name()
  {
    return this->err_name;
  }
  int get_err_code()
  {
    return this->err_code;
  }
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
  RAII_nomal(int fd, string name)
  {
    this->fd = fd;
    this->name = name;
  }

  ~RAII_nomal()
  {
    close(fd);
    cout << "Close ::  " + name << "\n";
  }

  int fd;
  string name;
};

//==========================================================================
//==========================================================================

class RAII_soc
{
private:
public:
  RAII_soc(int fd, string name)
  {
    this->fd = fd;
    this->name = name;
  }

  ~RAII_soc()
  {
    shutdown(fd, SHUT_WR);
    close(fd);
    cout << "Close ::  " << name << "\n";
  }
  int fd;
  string name;
}

;

//==========================================================================
//==========================================================================

struct st_RAII_epoll
{
  int ep_fd;
  int fd;
  string name_ep_fd;
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
      cout << "delete ::  " + v.name_ep_fd << "\n";
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
      close(v.first.first);
      close(v.first.second);
      cout << "Close ::  " + v.second << "\n";
    }
  }

  vector<pair<pair<int, int>, string>> vec;
};