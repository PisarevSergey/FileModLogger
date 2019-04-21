#include "common.h"

namespace
{
  FLT_CONTEXT_REGISTRATION creg[] =
  {
    {FLT_CONTEXT_END}
  };
}

FLT_CONTEXT_REGISTRATION* contexts::get_context_registrations()
{
  return creg;
}
