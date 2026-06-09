#pragma once

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
#include <sys/eventfd.h>
#include <sys/stat.h>

// IPC (System V & POSIX)
#include <mqueue.h> // mq_open, mq_send, mq_receive
#include <pthread.h>
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

class RAII_nomal;
class RAII_soc;
class RAII_epoll;
class RAII_pipe;

class Wk
{
public:
  Wk();
  ~Wk();

  //============================================================================
  void create_Th_Send();
  static void *EntryPoint_Send(void *vp);
  void set_sv_cliSoc(int set);
  //============================================================================
  //============================================================================

private:
  //============================================================================
  int sv_cliSoc;
  pthread_t Send_tid;
  //============================================================================

  //============================================================================
  //============================================================================

  //============================================================================
  //============================================================================
};
