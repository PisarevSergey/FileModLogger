#include "common.h"
#include "main.tmh"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING)
{
  WPP_INIT_TRACING(0, 0);

  EventRegisterIomon();

  NTSTATUS stat(STATUS_UNSUCCESSFUL);

  do
  {
    auto d = create_driver(stat, driver);
    EventWriteFunctionCall(0, "create_driver", stat);
    if (!NT_SUCCESS(stat))
    {
      em(MAIN, "create_driver failed with status %!STATUS!", stat);
      break;
    }
    im(MAIN, "create_driver success");

    stat = d->start_filtering();
    EventWriteFunctionCall(0, "start_filtering", stat);
    if (!NT_SUCCESS(stat))
    {
      delete d;
      em(MAIN, "start_filtering failed with status %!STATUS!", stat);
      break;
    }
    im(MAIN, "start_filtering success");

  } while (false);

  EventWriteStartEvent(0, stat);

  if (!NT_SUCCESS(stat))
  {
    em(MAIN, "DriverEntry failed with status %!STATUS!", stat);
    EventUnregisterIomon();
    WPP_CLEANUP(0);
  }

  return stat;
}
