#include "common.h"
#include "driver.tmh"

namespace
{
  NTSTATUS unload(FLT_FILTER_UNLOAD_FLAGS)
  {
    EventWriteUnloadEvent(0);
    info_message(DRIVER, "unloading");

    delete get_driver();

    return STATUS_SUCCESS;
  }

  NTSTATUS attach(_In_ PCFLT_RELATED_OBJECTS    /*FltObjects*/,
                  _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
                  _In_ DEVICE_TYPE              VolumeDeviceType,
                  _In_ FLT_FILESYSTEM_TYPE      /*VolumeFilesystemType*/)
  {
    NTSTATUS stat(STATUS_FLT_DO_NOT_ATTACH);

    if (FILE_DEVICE_DISK_FILE_SYSTEM == VolumeDeviceType)
    {
      info_message(DRIVER, "volume device type is disk file system");
      if (Flags & FLTFL_INSTANCE_SETUP_NEWLY_MOUNTED_VOLUME)
      {
        info_message(DRIVER, "new volume, attaching");
        stat = STATUS_SUCCESS;
      }
      else
      {
        info_message(DRIVER, "not a new volume, skipping attach");
      }
    }
    else
    {
      info_message(DRIVER, "skipping attach");
    }

    return stat;
  }

  class tracing_driver : public driver
  {
  public:
    tracing_driver()
    {
      WPP_INIT_TRACING(0, 0);
      EventRegisterIomon();
    }

    ~tracing_driver()
    {
      EventUnregisterIomon();
      WPP_CLEANUP(0);
    }
  };

  class driver_mem_alloc : public tracing_driver
  {
  public:
    void* __cdecl operator new(size_t, void* p) { return p; }
  };

  class filter_driver : public driver_mem_alloc
  {
  public:
    filter_driver(NTSTATUS& stat, PDRIVER_OBJECT driver) : filter(0)
    {
      FLT_REGISTRATION freg = {0};

      freg.Size = sizeof(freg);
      freg.Version = FLT_REGISTRATION_VERSION;

      FLT_CONTEXT_REGISTRATION creg[] =
      {
        {FLT_STREAM_CONTEXT,       0, contexts::stream_context_cleanup_callback       , contexts::get_stream_context_size(),        'crts'},
        {FLT_STREAMHANDLE_CONTEXT, 0, contexts::stream_handle_context_cleanup_callback, contexts::get_stream_handle_context_size(), 'tchs'},
        {FLT_CONTEXT_END}
      };
      freg.ContextRegistration = creg;

      FLT_OPERATION_REGISTRATION oreg[] =
      {
        {IRP_MJ_CREATE,                                          0,                                        0, operations::post_create},
        {IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,             0, operations::pre_acquire_for_section_sync, operations::post_acquire_for_section_sync},
        {IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION,             0, operations::pre_release_for_section_sync},
        {IRP_MJ_WRITE, FLTFL_OPERATION_REGISTRATION_SKIP_CACHED_IO, operations::pre_write},
        {IRP_MJ_CLOSE,                                           0, operations::pre_close},
        {IRP_MJ_OPERATION_END}
      };
      freg.OperationRegistration = oreg;

      freg.FilterUnloadCallback = unload;
      freg.InstanceSetupCallback = attach;

      stat = FltRegisterFilter(driver, &freg, &filter);
      if (NT_SUCCESS(stat))
      {
        info_message(DRIVER, "FltRegisterFilter success");
      }
      else
      {
        error_message(DRIVER, "FltRegisterFilter failed with status %!STATUS!", stat);
        filter = 0;
      }
    }

    ~filter_driver()
    {
      if (filter)
      {
        info_message(DRIVER, "unregistering filter");
        FltUnregisterFilter(filter);
        info_message(DRIVER, "filter unregistered");
      }
    }

    NTSTATUS start_filtering()
    {
      return FltStartFiltering(filter);
    }

    PFLT_FILTER get_filter()
    {
      return filter;
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
  if (NT_SUCCESS(stat))
  {
    info_message(DRIVER, "driver initialized successfully");
  }
  else
  {
    error_message(DRIVER, "driver initialization failed with status %!STATUS!", stat); //can use here, tracing init can't fail
    delete d;
    d = 0;
  }

  return d;
}

driver* get_driver()
{
  return reinterpret_cast<driver*>(driver_mem);
}
