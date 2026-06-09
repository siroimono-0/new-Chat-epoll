#include "Wk.h"
#include "Support.h"
#include "Sv.h"

using namespace std;

Wk::Wk()
{

  return;
}

Wk::~Wk()
{
}

void Wk::set_sv_cliSoc(int set)
{
  this->sv_cliSoc = set;
}

void Wk::create_Th_Send()
{
  int ret_t =
      pthread_create(&this->Send_tid, NULL, EntryPoint_Send, (void *)this);
  check::ck_r(string(__func__) + " pthread_create", ret_t, 0);
}

void *Wk::EntryPoint_Send(void *vp)
{
}