#include "Sv.h"
#include "Wk.h"
#include "Support.h"

using namespace std;

Wk::Wk()
{
  this->raii_nomal = new RAII_nomal;
  this->raii_soc = new RAII_soc;
  this->raii_ep = new RAII_epoll;
  this->raii_pipe = new RAII_pipe;

  this->wk_WakeUp_Fd = eventfd(0, EFD_NONBLOCK);
  this->raii_nomal->vec.push_back({this->wk_WakeUp_Fd, "wk_WakeUp_Fd"});
  // raii

  pthread_mutex_init(&this->loop_Echo_Mutex, nullptr);
  return;
}

Wk::~Wk()
{
  delete this->raii_nomal;
  delete this->raii_soc;
  delete this->raii_ep;
  delete this->raii_pipe;

  int ret_Mx_de = pthread_mutex_destroy(&this->loop_Echo_Mutex);
  check::ck_r(string(__func__) + " pthread_mutex_destroy", ret_Mx_de, 0);
  return;
}

void Wk::wk_wakeUp_Now()
{
  uint64_t g = 1;
  write(this->wk_WakeUp_Fd, &g, sizeof(g));
}

//=========================================================================
void Wk::set_Loop_Echo(bool set)
{
  pthread_mutex_lock(&this->loop_Echo_Mutex);
  this->loop_Echo = set;
  pthread_mutex_unlock(&this->loop_Echo_Mutex);
}
//=========================================================================
void Wk::createTh_Wk(Sv *p_Sv, int pairSoc, int newCli_Soc)
{
  this->p_Sv = p_Sv;
  this->wk_Sv_Wk_PairSoc = pairSoc;
  this->newCli_Soc_Fd = newCli_Soc;
  this->raii_soc->vec.push_back({this->newCli_Soc_Fd, "newCli_Soc_Fd"});
  // raii

  pthread_t tid;
  int ret_P = pthread_create(&tid, nullptr, wk_EntryPoint, (void *)this);
  check::ck_r(string(__func__) + " pthread_create", ret_P, 0);

  int ret_D = pthread_detach(tid);
  check::ck_r(string(__func__) + " pthread_detach", ret_D, 0);
}

void *Wk::wk_EntryPoint(void *vp)
{
  Wk *p_this = (Wk *)vp;
  try
  {
    p_this->wk_EntryPoint_Loop();
    p_this->p_Sv->set_vec_Fd_Wk(p_this->newCli_Soc_Fd);
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

void Wk::wk_EntryPoint_Loop()
{
  //===============================epoll====================================
  this->newCli_Ep_Fd = epoll_create1(0);
  this->raii_nomal->vec.push_back({this->newCli_Ep_Fd, "newCli_Ep_Fd"});
  // raii

  epoll_data_t ep_D = {};
  ep_D.fd = this->newCli_Soc_Fd;

  struct epoll_event ep_E = {};
  ep_E.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
  ep_E.data = ep_D;

  int ret_ctl_1 = epoll_ctl(this->newCli_Ep_Fd, EPOLL_CTL_ADD, this->newCli_Soc_Fd, &ep_E);
  check::ck_r(string(__func__) + " ret_ctl_1", ret_ctl_1, 0);

  struct st_RAII_epoll st_raii_1 = {};
  st_raii_1.ep_fd = this->newCli_Ep_Fd;
  st_raii_1.fd = this->newCli_Soc_Fd;
  st_raii_1.name_ep_fd = string("newCli_Ep_Fd  newCli_Soc_Fd");
  this->raii_ep->vec.push_back(st_raii_1);
  // raii

  epoll_data_t wk_ep_D = {};
  wk_ep_D.fd = this->wk_WakeUp_Fd;

  struct epoll_event wk_ep_E = {};
  wk_ep_E.events = EPOLLIN;
  wk_ep_E.data = wk_ep_D;

  int ret_ctl_2 = epoll_ctl(this->newCli_Ep_Fd, EPOLL_CTL_ADD, this->wk_WakeUp_Fd, &wk_ep_E);
  check::ck_r(string(__func__) + " ret_ctl_2", ret_ctl_2, 0);

  struct st_RAII_epoll st_raii_2 = {};
  st_raii_2.ep_fd = this->newCli_Ep_Fd;
  st_raii_2.fd = this->wk_WakeUp_Fd;
  st_raii_2.name_ep_fd = string("newCli_Ep_Fd  wk_WakeUp_Fd");
  this->raii_ep->vec.push_back(st_raii_2);
  // raii

  //===============================epoll====================================

  epoll_event ep_Arr[5];

  while (this->loop_Echo)
  {
    int ret_ep = epoll_wait(this->newCli_Ep_Fd, ep_Arr, 5, -1);
    check::ck(string(__func__) + " epoll_wait", ret_ep, -1);

    for (int i = 0; i < ret_ep; i++)
    {
      if (ep_Arr[i].data.fd == this->newCli_Soc_Fd &&
          ep_Arr[i].events & (EPOLLHUP | EPOLLERR))
      {
        this->set_Loop_Echo(false);
        cout << "Wk :: EPOLLHUP | EPOLLERR" << "\n";
      }
      else if (ep_Arr[i].data.fd == this->newCli_Soc_Fd &&
               ep_Arr[i].events & (EPOLLRDHUP))
      {
        this->set_Loop_Echo(false);
        cout << "Wk :: EPOLLRDHUP" << "\n";
      }
      else if (ep_Arr[i].data.fd == this->newCli_Soc_Fd &&
               ep_Arr[i].events & (EPOLLIN))
      {
        this->fromCli_ToSv_Echo();
      }

      else if (ep_Arr[i].data.fd == this->wk_WakeUp_Fd &&
               ep_Arr[i].events & (EPOLLIN))
      {
        this->set_Loop_Echo(false);
        cout << "Wk :: WakeUp" << "\n";
        uint64_t one;
        read(this->wk_WakeUp_Fd, &one, sizeof(one));
      }
    }
  }
}

void Wk::fromCli_ToSv_Echo()
{
  char *buf_Recv = new char[256];
  memset(buf_Recv, 0, 256);

  int ret_Recv_Len = recv(this->newCli_Soc_Fd, buf_Recv, 255, 0);
  if (ret_Recv_Len <= 0)
  {
    this->set_Loop_Echo(false);
    return;
  }

  buf_Recv[ret_Recv_Len] = '\0';

  send(this->wk_Sv_Wk_PairSoc, buf_Recv, ret_Recv_Len, 0);

  delete[] buf_Recv;
  return;
}