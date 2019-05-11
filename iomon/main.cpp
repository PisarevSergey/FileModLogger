#include "common.h"
#include "main.tmh"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING)
{
  NTSTATUS stat(STATUS_UNSUCCESSFUL);

  driver* d(0);

  do
  {
    d = create_driver(stat, driver_object);
    if (!NT_SUCCESS(stat))
    {
      break;
    }
    im(MAIN, "create_driver success");
    EventWriteFunctionCall(0, "create_driver", stat);

    stat = d->start_filtering();
    EventWriteFunctionCall(0, "start_filtering", stat);
    if (!NT_SUCCESS(stat))
    {
      em(MAIN, "start_filtering failed with status %!STATUS!", stat);
      break;
    }
    im(MAIN, "start_filtering success");
    EventWriteStartEvent(0, stat);

  } while (false);


  if (!NT_SUCCESS(stat) && d)
  {
    em(MAIN, "DriverEntry failed with status %!STATUS!", stat);
    delete d;
  }

  return stat;
}
