#include "Sv.h"
#include "Support.h"
#include "Wk.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Sv::Sv()
{
}

Sv::~Sv()
{
}

void Sv::start_Server()
{
  this->svSoc = socket(AF_INET, SOCK_STREAM, 0);
  RAII_soc r_svSoc(this->svSoc, "svSoc");

  struct sockaddr_in st_addr = {};
  st_addr.sin_family = AF_INET;
  st_addr.sin_port = htons(12345);
  inet_pton(AF_INET, "127.0.0.1", &st_addr.sin_addr);
  socklen_t addr_len = sizeof(st_addr);
  int ret_b = bind(this->svSoc, (struct sockaddr *)&st_addr, addr_len);
  check::ck(string(__func__) + " bind", ret_b, -1);

  listen(this->svSoc, 5);

  this->ep_svLoop = epoll_create1(0);
  RAII_nomal r_ep_svLoop(this->ep_svLoop, "ep_svLoop");

  epoll_data_t ep_D = {};
  ep_D.fd = this->svSoc;
  struct epoll_event ep_E = {};
  ep_E.data = ep_D;
  ep_E.events = EPOLLIN;

  epoll_ctl(this->ep_svLoop, EPOLL_CTL_ADD, this->svSoc, &ep_E);

  epoll_event ep_Arr[5];

  while (this->sv_Life == true)
  {
    epoll_wait(this->ep_svLoop, ep_Arr, 5, -1);

    for (int i = 0; i < 5; i++)
    {
      if (ep_Arr[i].data.fd == this->svSoc &&
          (ep_Arr[i].events & EPOLLIN))
      {
        int sv_cliSoc = accept(this->svSoc, nullptr, nullptr);
        Wk *worker = new Wk();
      }
    }
  }

  epoll_ctl(this->ep_svLoop, EPOLL_CTL_DEL, this->svSoc, nullptr);
}