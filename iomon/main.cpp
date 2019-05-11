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
    info_message(MAIN, "create_driver success");
    EventWriteFunctionCall(0, "create_driver", stat);

    stat = d->start_filtering();
    EventWriteFunctionCall(0, "start_filtering", stat);
    if (!NT_SUCCESS(stat))
    {
      error_message(MAIN, "start_filtering failed with status %!STATUS!", stat);
      break;
    }
    info_message(MAIN, "start_filtering success");
    EventWriteStartEvent(0, stat);

  } while (false);


  if (!NT_SUCCESS(stat) && d)
  {
    error_message(MAIN, "DriverEntry failed with status %!STATUS!", stat);
    delete d;
  }

  return stat;
}
