#include "Support.h"
#include "Client.h"

Client::Client()
{
  this->raii_nomal = new RAII_nomal;
  this->raii_soc = new RAII_soc;
  this->raii_ep = new RAII_epoll;
  this->raii_pipe = new RAII_pipe;
}
Client::~Client()
{
  delete this->raii_nomal;
  delete this->raii_soc;
  delete this->raii_ep;
  delete this->raii_pipe;
}

void Client::set_Client()
{
  this->cliSoc_fd = socket(AF_INET, SOCK_STREAM, 0);
  this->raii_soc->vec.push_back({this->cliSoc_fd, "cliSoc_fd"});

  struct sockaddr_in st_socAddr = {};
  st_socAddr.sin_family = AF_INET;
  st_socAddr.sin_port = htons(12345);

  inet_pton(AF_INET, "127.0.0.1", &st_socAddr.sin_addr);
  socklen_t socLen = sizeof(st_socAddr);

  connect(this->cliSoc_fd, (struct sockaddr *)&st_socAddr, socLen);
}