#include "Support.h"
#include "Client.h"

Client::Client()
{
  this->raii_Nomal = new RAII_nomal;
  this->raii_Soc = new RAII_soc;
  this->raii_Ep = new RAII_epoll;
  this->raii_Pipe = new RAII_pipe;

  this->wakeUp_Fd = eventfd(0, EFD_NONBLOCK);
  this->raii_Nomal->vec.push_back({this->wakeUp_Fd, "wakeUp_fd"});
  // raii

  pthread_mutex_init(&this->loop_Client_mux, nullptr);
}

Client::~Client()
{
  delete this->raii_Nomal;
  delete this->raii_Soc;
  delete this->raii_Ep;
  delete this->raii_Pipe;

  int ret_mux_1 = pthread_mutex_destroy(&this->loop_Client_mux);
  check::ck_r(string(__func__) + " pthread_mutex_destroy loop_Client_mux", ret_mux_1, 0);
}

void Client::set_loop_Client(bool set)
{
  pthread_mutex_lock(&this->loop_Client_mux);
  this->loop_Client = set;
  pthread_mutex_unlock(&this->loop_Client_mux);
}

void Client::set_Client()
{
  this->cli_Soc_Fd = socket(AF_INET, SOCK_STREAM, 0);
  this->raii_Soc->vec.push_back({this->cli_Soc_Fd, "cliSoc_fd"});
  // raii

  struct sockaddr_in st_socAddr = {};
  st_socAddr.sin_family = AF_INET;
  st_socAddr.sin_port = htons(12345);

  inet_pton(AF_INET, "127.0.0.1", &st_socAddr.sin_addr);
  socklen_t socLen = sizeof(st_socAddr);

  int ret_C = connect(this->cli_Soc_Fd, (struct sockaddr *)&st_socAddr, socLen);
  check::ck_r(string(__func__) + " connect", ret_C, 0);

  this->createTh_Recv();
  // 리시브 쓰레드 생성

  //===============================epoll====================================
  this->outPut_Ep_Fd = epoll_create1(0);
  this->raii_Nomal->vec.push_back({this->outPut_Ep_Fd, " outPut_Ep_Fd"});

  epoll_data_t ep_D = {};
  ep_D.fd = this->outPut_Ter_Fd;

  struct epoll_event ep_E = {};
  ep_E.events = EPOLLIN;
  ep_E.data = ep_D;

  epoll_ctl(this->outPut_Ep_Fd, EPOLL_CTL_ADD, this->outPut_Ter_Fd, &ep_E);
  struct st_RAII_epoll st_raii_1 = {};
  st_raii_1.ep_fd = this->outPut_Ep_Fd;
  st_raii_1.fd = this->outPut_Ter_Fd;
  st_raii_1.name_ep_fd = string("outPut_Ep_Fd  outPut_Ter_Fd");
  this->raii_Ep->vec.push_back(st_raii_1);

  epoll_data_t ep_D_Wk = {};
  ep_D_Wk.fd = this->wakeUp_Fd;

  struct epoll_event ep_E_Wk = {};
  ep_E_Wk.events = EPOLLIN;
  ep_E_Wk.data = ep_D_Wk;

  epoll_ctl(this->outPut_Ep_Fd, EPOLL_CTL_ADD, this->wakeUp_Fd, &ep_E_Wk);
  struct st_RAII_epoll st_raii_2 = {};
  st_raii_2.ep_fd = this->outPut_Ep_Fd;
  st_raii_2.fd = this->wakeUp_Fd;
  st_raii_2.name_ep_fd = string("outPut_Ep_Fd  wakeUp_Fd");
  this->raii_Ep->vec.push_back(st_raii_2);

  //===============================epoll====================================

  epoll_event ep_Arr[5];

  while (this->loop_Client)
  {
    int ret_ep = epoll_wait(this->outPut_Ep_Fd, ep_Arr, 5, -1);
    // check::ck(string(__func__) + "  epoll_wait", ret_ep, -1);

    for (int i = 0; i < ret_ep; i++)
    {
      if (ep_Arr[i].data.fd == this->outPut_Ter_Fd &&
          ep_Arr[i].events & (EPOLLIN))
      {
        sendToWk();
      }
      else if (ep_Arr[i].data.fd == this->wakeUp_Fd &&
               ep_Arr[i].events & (EPOLLIN))
      {
        this->set_loop_Client(false);
        break;
      }
    }
  }
  cout << " Exit main loop" << string(__func__) << "\n";
}

void Client::sendToWk()
{
  char *buf_Recv = new char[256];
  memset(buf_Recv, 0, 256);

  int read_Len = read(this->outPut_Ter_Fd, buf_Recv, 256);

  if (read_Len <= 0)
  {
    // read 에러 or EOF 처리
    this->set_loop_Client(false);
    delete[] buf_Recv;
    return;
  }

  buf_Recv[read_Len - 1] = '\0';

  if (strcmp(buf_Recv, "end") == 0)
  {
    this->set_loop_Client(false);
    delete[] buf_Recv;
    return;
  }

  send(this->cli_Soc_Fd, buf_Recv, read_Len, 0);
  // int ret_s = send(this->cli_Soc_Fd, buf_Recv, read_Len, 0);
  // check::ck("send ok", ret_s, -1);

  delete[] buf_Recv;
  return;
}
//=================================Recv Thread==================================
//=================================Recv Thread==================================
//=================================Recv Thread==================================
//=================================Recv Thread==================================
//=================================Recv Thread==================================
void Client::createTh_Recv()
{
  pthread_t tid;
  int ret_P = pthread_create(&tid, nullptr, recv_EntryPoint, (void *)this);
  check::ck_r(string(__func__) + " pthread_create", ret_P, 0);

  int ret_D = pthread_detach(tid);
  check::ck_r(string(__func__) + " pthread_detach", ret_D, 0);
  return;
}

void *Client::recv_EntryPoint(void *vp)
{
  Client *p_this = (Client *)vp;
  try
  {
    p_this->recv_EntryPoint_Loop();
  }
  catch (Exception err)
  {
    int err_code = err.get_err_code();
    string err_name = err.get_err_name();
    string name = err.get_name();
    printf("ERR\nname     :: %s\nerr type :: %s\nerr code :: %d\n",
           name.c_str(), err_name.c_str(), err_code);
    return nullptr;
  }
  return nullptr;
}

void Client::recv_EntryPoint_Loop()
{
  //===============================epoll====================================
  this->recv_Ep_Fd = epoll_create1(0);
  this->raii_Nomal->vec.push_back({this->recv_Ep_Fd, "recv_Ep_Fd"});
  // raii

  epoll_data_t ep_D = {};
  ep_D.fd = this->cli_Soc_Fd;

  struct epoll_event ep_E = {};
  ep_E.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
  ep_E.data = ep_D;

  int ret_ctl_1 = epoll_ctl(this->recv_Ep_Fd, EPOLL_CTL_ADD, this->cli_Soc_Fd, &ep_E);
  check::ck_r(string(__func__) + " ret_ctl_1", ret_ctl_1, 0);

  struct st_RAII_epoll st_raii_1 = {};
  st_raii_1.ep_fd = this->recv_Ep_Fd;
  st_raii_1.fd = this->cli_Soc_Fd;
  st_raii_1.name_ep_fd = string("recv_Ep_Fd  cli_Soc_Fd");
  this->raii_Ep->vec.push_back(st_raii_1);
  // raii

  epoll_data_t wk_ep_D = {};
  wk_ep_D.fd = this->wakeUp_Fd;

  struct epoll_event wk_ep_E = {};
  wk_ep_E.events = EPOLLIN;
  wk_ep_E.data = wk_ep_D;

  int ret_ctl_2 =
      epoll_ctl(this->recv_Ep_Fd, EPOLL_CTL_ADD, this->wakeUp_Fd, &wk_ep_E);
  check::ck_r(string(__func__) + " ret_ctl_2", ret_ctl_2, 0);

  struct st_RAII_epoll st_raii_2 = {};
  st_raii_2.ep_fd = this->recv_Ep_Fd;
  st_raii_2.fd = this->wakeUp_Fd;
  st_raii_2.name_ep_fd = string("echoEp_Fd  wakeUp_Fd");
  this->raii_Ep->vec.push_back(st_raii_2);
  // raii

  //===============================epoll====================================

  epoll_event ep_Arr[5];

  while (this->loop_Client)
  {
    int ret_ep = epoll_wait(this->recv_Ep_Fd, ep_Arr, 5, -1);
    // check::ck(string(__func__) + " epoll_wait", ret_ep, -1);

    for (int i = 0; i < ret_ep; i++)
    {
      if (ep_Arr[i].data.fd == this->cli_Soc_Fd &&
          ep_Arr[i].events & (EPOLLERR | EPOLLHUP))
      {
        this->set_loop_Client(false);
        cout << "Server :: EPOLLHUP | EPOLLERR" << "\n";
      }
      else if (ep_Arr[i].data.fd == this->cli_Soc_Fd &&
               ep_Arr[i].events & (EPOLLRDHUP))
      {
        this->set_loop_Client(false);
        cout << "Server :: EPOLLRDHUP" << "\n";
      }
      else if (ep_Arr[i].data.fd == this->cli_Soc_Fd &&
               ep_Arr[i].events & (EPOLLIN))
      {
        this->formSv_Recv();
      }
      else if (ep_Arr[i].data.fd == this->wakeUp_Fd &&
               ep_Arr[i].events & (EPOLLIN))
      {
        this->set_loop_Client(false);
        cout << "Server :: Wakeup" << "\n";
      }
    }
  }
}

void Client::formSv_Recv()
{
  char *buf_Recv = new char[256];
  memset(buf_Recv, 0, 256);

  int ret_Recv_Len = recv(this->cli_Soc_Fd, buf_Recv, 255, 0);
  if (ret_Recv_Len <= 0)
  {
    this->set_loop_Client(false);
    return;
  }
  cout << "Chat :: " << buf_Recv << "\n";

  delete[] buf_Recv;
  return;
}
//=================================Recv Thread==================================
//=================================Recv Thread==================================
//=================================Recv Thread==================================
//=================================Recv Thread==================================
//=================================Recv Thread==================================