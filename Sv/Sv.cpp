#include "Sv.h"
#include "Wk.h"
#include "Support.h"

Sv::Sv()
{

  this->raii_nomal = new RAII_nomal;
  this->raii_soc = new RAII_soc;
  this->raii_ep = new RAII_epoll;
  this->raii_pipe = new RAII_pipe;

  this->wakeUp_fd = eventfd(0, EFD_NONBLOCK);
  this->raii_nomal->vec.push_back({this->wakeUp_fd, "wakeUp_fd"});
}

Sv::~Sv()
{
  delete this->raii_nomal;
  delete this->raii_soc;
  delete this->raii_ep;
  delete this->raii_pipe;
}

void Sv::set_Server()
{
  this->svSoc_fd = socket(AF_INET, SOCK_STREAM, 0);
  this->raii_soc->vec.push_back({this->svSoc_fd, "svSoc_fd"});

  struct sockaddr_in st_socAddr;
  st_socAddr.sin_family = AF_INET;
  st_socAddr.sin_port = htons(12345);

  inet_pton(AF_INET, "127.0.0.1", &st_socAddr.sin_addr);

  socklen_t socLen = sizeof(st_socAddr);
  int ret_b = bind(this->svSoc_fd, (struct sockaddr *)&st_socAddr, socLen);
  check::ck_r(string(__func__) + "bind", ret_b, 0);

  listen(this->svSoc_fd, 5);

  //===============================epoll====================================
  this->svEp_fd = epoll_create1(0);

  epoll_data_t ep_D = {};
  ep_D.fd = this->svSoc_fd;

  struct epoll_event ep_E = {};
  ep_E.events = EPOLLIN;
  ep_E.data = ep_D;

  epoll_ctl(this->svEp_fd, EPOLL_CTL_ADD, this->svSoc_fd, &ep_E);

  epoll_data_t wk_ep_D = {};
  wk_ep_D.fd = this->wakeUp_fd;

  struct epoll_event wk_ep_E = {};
  wk_ep_E.events = EPOLLIN;
  wk_ep_E.data = wk_ep_D;

  epoll_ctl(this->svEp_fd, EPOLL_CTL_ADD, this->wakeUp_fd, &ep_E);
  //===============================epoll====================================

  epoll_event buf_ep[5];

  int tmp = 5;
  while (tmp != 0)
  {
    int ret_ep = epoll_wait(this->svEp_fd, buf_ep, 5, -1);
    check::ck(string(__func__) + "epoll_wait", ret_ep, -1);

    struct sockaddr_in st_acc = {};
    socklen_t accLen = sizeof(st_acc);
    int newCli_fd = accept(this->svSoc_fd, (struct sockaddr *)&st_acc, &accLen);
    check::ck(string(__func__) + " accept", newCli_fd, -1);

    this->raii_soc->vec.push_back({newCli_fd, string("newCli_fd  ::  ") + to_string(newCli_fd)});
    this->vec_newCli_fd.push_back(newCli_fd);
    tmp--;
  }
  return;
}