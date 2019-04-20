#include "common.h"
#include "flt_registration.tmh"

namespace
{
  NTSTATUS unload(FLT_FILTER_UNLOAD_FLAGS)
  {
    delete get_driver();

    WPP_CLEANUP(0);

    return STATUS_SUCCESS;
  }

  FLT_REGISTRATION freg = { 0 };
}

FLT_REGISTRATION* flt_registration::get_filter_registration()
{
  return &freg;
}

