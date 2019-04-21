#include "common.h"

namespace
{
  FLT_OPERATION_REGISTRATION oreg[] =
  {
    {IRP_MJ_OPERATION_END}
  };
}

FLT_OPERATION_REGISTRATION* operations::get_operations()
{
  return oreg;
}
