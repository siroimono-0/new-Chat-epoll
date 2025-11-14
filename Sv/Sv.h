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
class Wk;

class Sv
{
public:
  Sv();
  ~Sv();

  void set_Server();
  // void end_Server();

  //=========================================================================
  void createTh_Shut(); // 셧다운 명령용 쓰레드
  static void *Shut_EntryPoint(void *vp);
  void Shut_EntryPoint_Loop(); // fd검사 레이스 컨디션 mux ok
  //=========================================================================

  //=========================================================================
  void createTh_del(); // 죽은 Wk객체 delete용 Thread
  static void *del_EntryPoint(void *vp);
  void del_EntryPoint_Loop();
  //=========================================================================

  //=========================================================================
  void set_Loop_Server(bool set); // mutex ok
  void set_vec_Fd_Wk(int fd);     // Wk가 자신의 fd를 -1 설정함으로 사망 알림
                                  // mutex ok
  //=========================================================================

  //=========================================================================
  void createTh_Echo(); // 채팅 브로드 캐스트용 Th
  static void *echo_EntryPoint(void *vp);
  void echo_EntryPoint_Loop();
  void formWk_ToCli_Echo();
  //=========================================================================

private:
  int svSoc_Fd;                    // raii ok
  int svEp_Fd;                     // raii ok
  bool loop_Server = true;         // mutex ok
  pthread_mutex_t loop_Server_Mux; // init, destory ok
  pthread_mutex_t fd_Wk_Mux;       // init, destory ok

  //=============================================
  int wakeUp_Fd; // raii ok
  //=============================================
  int sv_Wk_PairSoc[2]; // 0번 사용 서버는
  int echoEp_Fd;        // raii ok
  //=============================================
  vector<pair<int, Wk *>> vec_Fd_Wk; // 자신이 죽으면 Wk가 fd를 -1로 변경
                                     // 그럼 특정 연결된 wk 도 정리

  RAII_nomal *raii_Nomal;
  RAII_soc *raii_Soc;
  RAII_epoll *raii_Ep;
  RAII_pipe *raii_Pipe;
};
// 타이머 써서 시그널 박을바에
// 회수 전용 쓰레드 만들어서 죽은얘들 회수해오자
