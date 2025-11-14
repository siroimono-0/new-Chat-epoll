#pragma once

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
#include <sys/eventfd.h>

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

class RAII_nomal;
class RAII_soc;
class RAII_epoll;
class RAII_pipe;

class Client
{
public:
  Client();
  ~Client();

  void set_Client();
  void sendToWk(); // mux ok
  void set_loop_Client(bool set);

  //============================================================================
  void createTh_Recv(); // 리시브용 쓰레드
  static void *recv_EntryPoint(void *vp);
  void recv_EntryPoint_Loop();
  void formSv_Recv();
  //============================================================================

private:
  //============================================================================
  int cli_Soc_Fd; // raii ok
  int outPut_Ter_Fd = 0;
  int outPut_Ep_Fd;                // raii ok
  bool loop_Client = true;         // mux ok
  pthread_mutex_t loop_Client_mux; // init destroy ok
  //============================================================================

  //============================================================================
  int recv_Ep_Fd; // raii ok
  //============================================================================

  //============================================================================
  int wakeUp_Fd; // raii ok
  //============================================================================

  RAII_nomal *raii_Nomal;
  RAII_soc *raii_Soc;
  RAII_epoll *raii_Ep;
  RAII_pipe *raii_Pipe;
};
