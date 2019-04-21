#include "common.h"
#include "flt_registration.tmh"

namespace
{
  NTSTATUS unload(FLT_FILTER_UNLOAD_FLAGS)
  {
    delete get_driver();

    EventWriteUnloadEvent(0);
    im(FLT_REGISTRATION, "unloading");

    EventUnregisterIomon();
    WPP_CLEANUP(0);

    return STATUS_SUCCESS;
  }

  NTSTATUS attach(_In_ PCFLT_RELATED_OBJECTS    /*FltObjects*/,
                  _In_ FLT_INSTANCE_SETUP_FLAGS /*Flags*/,
                  _In_ DEVICE_TYPE              VolumeDeviceType,
                  _In_ FLT_FILESYSTEM_TYPE      /*VolumeFilesystemType*/)
  {
    NTSTATUS stat(STATUS_FLT_DO_NOT_ATTACH);

    if (FILE_DEVICE_DISK_FILE_SYSTEM == VolumeDeviceType)
    {
      stat = STATUS_SUCCESS;
    }

    return stat;
  }


  FLT_REGISTRATION freg = { 0 };
}

FLT_REGISTRATION* flt_registration::get_filter_registration()
{
  freg.Size = sizeof(freg);
  freg.Version = FLT_REGISTRATION_VERSION;
  freg.ContextRegistration = contexts::get_context_registrations();
  freg.OperationRegistration = operations::get_operations();
  freg.FilterUnloadCallback = unload;
  freg.InstanceSetupCallback = attach;

  return &freg;
}

