#include "Sv.h"
#include "Wk.h"
#include "Support.h"

Sv::Sv()
{

  this->raii_Nomal = new RAII_nomal;
  this->raii_Soc = new RAII_soc;
  this->raii_Ep = new RAII_epoll;
  this->raii_Pipe = new RAII_pipe;

  this->wakeUp_Fd = eventfd(0, EFD_NONBLOCK);
  this->raii_Nomal->vec.push_back({this->wakeUp_Fd, "wakeUp_fd"});

  socketpair(AF_UNIX, SOCK_STREAM, 0, this->sv_Wk_PairSoc);
  this->raii_Pipe->vec.push_back(
      {{this->sv_Wk_PairSoc[0], this->sv_Wk_PairSoc[1]}, " sv_Wk_PairSoc"});

  pthread_mutex_init(&this->loop_Server_Mux, nullptr);
  pthread_mutex_init(&this->fd_Wk_Mux, nullptr);
}

Sv::~Sv()
{
  delete this->raii_Nomal;
  delete this->raii_Soc;
  delete this->raii_Ep;
  delete this->raii_Pipe;

  int ret_mux_1 = pthread_mutex_destroy(&this->loop_Server_Mux);
  check::ck_r(string(__func__) + " pthread_mutex_destroy loop_Server_Mux", ret_mux_1, 0);

  int ret_mux_2 = pthread_mutex_destroy(&this->fd_Wk_Mux);
  check::ck_r(string(__func__) + " pthread_mutex_destroy fd_Wk_Mux", ret_mux_2, 0);
}

void Sv::set_Loop_Server(bool set)
{
  pthread_mutex_lock(&this->loop_Server_Mux);
  this->loop_Server = set;
  pthread_mutex_unlock(&this->loop_Server_Mux);
  return;
}

void Sv::set_vec_Fd_Wk(int fd)
{
  pthread_mutex_lock(&this->fd_Wk_Mux);
  for (int i = 0; i < (int)this->vec_Fd_Wk.size(); i++)
  {
    if (this->vec_Fd_Wk[i].first == fd)
    {
      this->vec_Fd_Wk[i].first = -1;
    }
  }
  pthread_mutex_unlock(&this->fd_Wk_Mux);
  return;
}

void Sv::set_Server()
{
  this->svSoc_Fd = socket(AF_INET, SOCK_STREAM, 0);
  this->raii_Soc->vec.push_back({this->svSoc_Fd, "svSoc_fd"});
  // raii

  int tmp = 1;
  socklen_t tmpLen = sizeof(tmp);
  setsockopt(this->svSoc_Fd, SOL_SOCKET, SO_REUSEADDR, &tmp, tmpLen);

  struct sockaddr_in st_socAddr;
  st_socAddr.sin_family = AF_INET;
  st_socAddr.sin_port = htons(12345);

  inet_pton(AF_INET, "127.0.0.1", &st_socAddr.sin_addr);

  socklen_t socLen = sizeof(st_socAddr);
  int ret_b = bind(this->svSoc_Fd, (struct sockaddr *)&st_socAddr, socLen);
  check::ck_r(string(__func__) + " bind", ret_b, 0);

  listen(this->svSoc_Fd, 5);

  this->createTh_del();
  // 사망 확인용 쓰레드 생성

  this->createTh_Echo();
  // 에코 쓰레드 생성

  //===============================epoll====================================
  this->svEp_Fd = epoll_create1(0);
  this->raii_Nomal->vec.push_back({this->svEp_Fd, "svEp_Fd"});
  // raii

  epoll_data_t ep_D = {};
  ep_D.fd = this->svSoc_Fd;

  struct epoll_event ep_E = {};
  ep_E.events = EPOLLIN;
  ep_E.data = ep_D;

  epoll_ctl(this->svEp_Fd, EPOLL_CTL_ADD, this->svSoc_Fd, &ep_E);

  struct st_RAII_epoll st_raii_1 = {};
  st_raii_1.ep_fd = this->svEp_Fd;
  st_raii_1.fd = this->svSoc_Fd;
  st_raii_1.name_ep_fd = string("svEp_Fd  svSoc_Fd");
  this->raii_Ep->vec.push_back(st_raii_1);
  // raii

  epoll_data_t wk_ep_D = {};
  wk_ep_D.fd = this->wakeUp_Fd;

  struct epoll_event wk_ep_E = {};
  wk_ep_E.events = EPOLLIN;
  wk_ep_E.data = wk_ep_D;

  epoll_ctl(this->svEp_Fd, EPOLL_CTL_ADD, this->wakeUp_Fd, &ep_E);

  struct st_RAII_epoll st_raii_2 = {};
  st_raii_2.ep_fd = this->svEp_Fd;
  st_raii_2.fd = this->wakeUp_Fd;
  st_raii_2.name_ep_fd = string("svEp_Fd  wakeUp_Fd");
  this->raii_Ep->vec.push_back(st_raii_2);
  // raii

  //===============================epoll====================================

  epoll_event buf_ep[5];

  while (this->loop_Server)
  {
    int ret_ep = epoll_wait(this->svEp_Fd, buf_ep, 5, -1);
    check::ck(string(__func__) + "  epoll_wait", ret_ep, -1);

    for (int i = 0; i < ret_ep; i++)
    {
      if (buf_ep[i].data.fd == this->svSoc_Fd &&
          buf_ep[i].events & (EPOLLIN))
      {
        struct sockaddr_in st_acc = {};
        socklen_t accLen = sizeof(st_acc);
        int newCli_fd = accept(this->svSoc_Fd, (struct sockaddr *)&st_acc, &accLen);
        check::ck(string(__func__) + " accept", newCli_fd, -1);

        this->raii_Soc->vec.push_back({newCli_fd, string("newCli_fd  ::  ") + to_string(newCli_fd)});

        Wk *wk = new Wk;
        wk->createTh_Wk(this, this->sv_Wk_PairSoc[1], newCli_fd);

        this->vec_Fd_Wk.push_back({newCli_fd, wk});
      }
      else if (buf_ep[i].data.fd == this->wakeUp_Fd &&
               buf_ep[i].events & (EPOLLIN))
      {
        this->set_Loop_Server(false);
      }
    }
  }
  return;
}

//=================================dell Thread==================================
//=================================dell Thread==================================
//=================================dell Thread==================================
//=================================dell Thread==================================
//=================================dell Thread==================================
void Sv::createTh_del()
{
  pthread_t tid;
  int ret_P = pthread_create(&tid, nullptr, del_EntryPoint, (void *)this);
  check::ck_r(string(__func__) + " pthread_create", ret_P, 0);

  int ret_D = pthread_detach(tid);
  check::ck_r(string(__func__) + " pthread_detach", ret_D, 0);
}

void *Sv::del_EntryPoint(void *vp)
{
  Sv *p_this = (Sv *)vp;
  try
  {
    p_this->del_EntryPoint_Loop();
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

void Sv::del_EntryPoint_Loop()
{

  while (this->loop_Server)
  {
    sleep(5);

    bool del_Flag = false;
    int num = 0;
    int life_num = 0;
    for (int i = 0; i < (int)this->vec_Fd_Wk.size(); i++)
    {
      if (this->vec_Fd_Wk[i].first == -1)
      {
        delete this->vec_Fd_Wk[i].second;
        this->vec_Fd_Wk[i].first = -12345;
        del_Flag = true;
        num++;
      }
      else if (this->vec_Fd_Wk[i].first == -12345)
      {
        num++;
      }
      else
      {
        life_num++;
      }
    }

    if (del_Flag == true)
    {
      cout << "============ Delete Thread count :: " << to_string(num) << "\n";
      cout << "============ current Thread count :: " << to_string(life_num) << "\n";
    }
  }
}
//=================================Dell Thread==================================
//=================================Dell Thread==================================
//=================================Dell Thread==================================
//=================================Dell Thread==================================
//=================================Dell Thread==================================

//=================================Echo Thread==================================
//=================================Echo Thread==================================
//=================================Echo Thread==================================
//=================================Echo Thread==================================
//=================================Echo Thread==================================
void Sv::createTh_Echo()
{
  pthread_t tid;
  int ret_P = pthread_create(&tid, nullptr, echo_EntryPoint, (void *)this);
  check::ck_r(string(__func__) + " pthread_create", ret_P, 0);

  int ret_D = pthread_detach(tid);
  check::ck_r(string(__func__) + " pthread_detach", ret_D, 0);
}

void *Sv::echo_EntryPoint(void *vp)
{
  Sv *p_this = (Sv *)vp;
  try
  {
    p_this->echo_EntryPoint_Loop();
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

void Sv::echo_EntryPoint_Loop()
{
  //===============================epoll====================================
  this->echoEp_Fd = epoll_create1(0);
  this->raii_Nomal->vec.push_back({this->echoEp_Fd, "echoEp_Fd"});
  // raii

  epoll_data_t ep_D = {};
  ep_D.fd = this->sv_Wk_PairSoc[0];

  struct epoll_event ep_E = {};
  ep_E.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
  ep_E.data = ep_D;

  int ret_ctl_1 = epoll_ctl(this->echoEp_Fd, EPOLL_CTL_ADD, this->sv_Wk_PairSoc[0], &ep_E);
  check::ck_r(string(__func__) + " ret_ctl_1", ret_ctl_1, 0);

  struct st_RAII_epoll st_raii_1 = {};
  st_raii_1.ep_fd = this->echoEp_Fd;
  st_raii_1.fd = this->sv_Wk_PairSoc[0];
  st_raii_1.name_ep_fd = string("echoEp_Fd  sv_Wk_PairSoc");
  this->raii_Ep->vec.push_back(st_raii_1);
  // raii

  epoll_data_t wk_ep_D = {};
  wk_ep_D.fd = this->wakeUp_Fd;

  struct epoll_event wk_ep_E = {};
  wk_ep_E.events = EPOLLIN;
  wk_ep_E.data = wk_ep_D;

  int ret_ctl_2 =
      epoll_ctl(this->echoEp_Fd, EPOLL_CTL_ADD, this->wakeUp_Fd, &wk_ep_E);
  check::ck_r(string(__func__) + " ret_ctl_2", ret_ctl_2, 0);

  struct st_RAII_epoll st_raii_2 = {};
  st_raii_2.ep_fd = this->echoEp_Fd;
  st_raii_2.fd = this->wakeUp_Fd;
  st_raii_2.name_ep_fd = string("echoEp_Fd  wakeUp_Fd");
  this->raii_Ep->vec.push_back(st_raii_2);
  // raii

  //===============================epoll====================================

  epoll_event ep_Arr[5];

  while (this->loop_Server)
  {
    int ret_ep = epoll_wait(this->echoEp_Fd, ep_Arr, 5, -1);
    check::ck(string(__func__) + " epoll_wait", ret_ep, -1);

    for (int i = 0; i < ret_ep; i++)
    {
      if (ep_Arr[i].data.fd == this->sv_Wk_PairSoc[0] &&
          ep_Arr[i].events & (EPOLLERR | EPOLLHUP))
      {
        this->set_Loop_Server(false);
        cout << "Server :: EPOLLHUP | EPOLLERR" << "\n";
      }
      else if (ep_Arr[i].data.fd == this->sv_Wk_PairSoc[0] &&
               ep_Arr[i].events & (EPOLLRDHUP))
      {
        this->set_Loop_Server(false);
        cout << "Server :: EPOLLRDHUP" << "\n";
      }
      else if (ep_Arr[i].data.fd == this->sv_Wk_PairSoc[0] &&
               ep_Arr[i].events & (EPOLLIN))
      {
        this->formWk_ToCli_Echo();
        // cout << " yes" << "\n";
      }
      else if (ep_Arr[i].data.fd == this->wakeUp_Fd &&
               ep_Arr[i].events & (EPOLLIN))
      {
        this->set_Loop_Server(false);
        cout << "Server :: Wakeup" << "\n";
      }
    }
  }
}

void Sv::formWk_ToCli_Echo()
{
  char *buf_Recv = new char[256];
  memset(buf_Recv, 0, 256);

  int ret_Recv_Len = recv(this->sv_Wk_PairSoc[0], buf_Recv, 255, 0);
  if (ret_Recv_Len <= 0)
  {
    this->set_Loop_Server(false);
    return;
  }

  buf_Recv[ret_Recv_Len] = '\0';

  for (int i = 0; i < (int)this->vec_Fd_Wk.size(); i++)
  {
    if (this->vec_Fd_Wk[i].first >= 0)
    {
      send(this->vec_Fd_Wk[i].first, buf_Recv, ret_Recv_Len, 0);
    }
  }

  delete[] buf_Recv;
  return;
}
//=================================Echo Thread==================================
//=================================Echo Thread==================================
//=================================Echo Thread==================================
//=================================Echo Thread==================================
//=================================Echo Thread==================================