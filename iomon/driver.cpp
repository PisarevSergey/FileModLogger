#include "common.h"
#include "driver.tmh"

namespace
{
  class driver_mem_alloc : public driver
  {
  public:
    void* __cdecl operator new(size_t, void* p) { return p; }
  };

  class filter_driver : public driver_mem_alloc
  {
  public:
    filter_driver(NTSTATUS& stat, PDRIVER_OBJECT driver) : filter(0)
    {
      stat = FltRegisterFilter(driver, flt_registration::get_filter_registration(), &filter);
      if (NT_SUCCESS(stat))
      {
      }
      else
      {
        filter = 0;
      }
    }

    ~filter_driver()
    {
      if (filter)
      {
        FltUnregisterFilter(filter);
      }
    }

    NTSTATUS start_filtering()
    {
      return FltStartFiltering(filter);
    }
  private:
    PFLT_FILTER filter;
  };

  class top_driver : public filter_driver
  {
  public:
    top_driver(NTSTATUS& stat, PDRIVER_OBJECT driver) : filter_driver(stat, driver)
    {}
  };

  char driver_mem[sizeof(top_driver)];
}

driver* create_driver(NTSTATUS& stat, PDRIVER_OBJECT driver)
{
  auto d(new (driver_mem) top_driver(stat, driver));
  if (!NT_SUCCESS(stat))
  {
    delete d;
    d = 0;
  }

  return d;
}

driver* get_driver()
{
  return reinterpret_cast<driver*>(driver_mem);
}
