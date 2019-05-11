#include "common.h"
#include "writer_info.tmh"

namespace
{
  class writer_info_with_pid : public writer_info
  {
  public:
    writer_info_with_pid(PFLT_CALLBACK_DATA data) : pid(FltGetRequestorProcessIdEx(data))
    {}

    HANDLE get_pid() const { return pid; }

  private:
    HANDLE pid;
  };

  class writer_info_with_file_name : public writer_info_with_pid
  {
  public:
    writer_info_with_file_name(NTSTATUS& stat, PFLT_CALLBACK_DATA data) : writer_info_with_pid(data), file_name(0)
    {
      support::auto_flt_context<contexts::stream_handle_context> shc;
      stat = FltGetStreamHandleContext(data->Iopb->TargetInstance, data->Iopb->TargetFileObject, shc);
      if (NT_SUCCESS(stat))
      {
        info_message(WRITER_INFO, "FltGetStreamHandleContext success");

        const USHORT name_len(sizeof(file_name[0]) + shc->get_file_name()->MaximumLength);
        file_name = static_cast<UNICODE_STRING*>(ExAllocatePoolWithTag(PagedPool, name_len, 'nlif'));
        if (file_name)
        {
          info_message(WRITER_INFO, "file name buffer allocated successfully");

          file_name->Buffer = reinterpret_cast<wchar_t*>(file_name + 1);
          file_name->Length = shc->get_file_name()->Length;
          file_name->MaximumLength = shc->get_file_name()->MaximumLength;
          RtlCopyUnicodeString(file_name, shc->get_file_name());
          stat = STATUS_SUCCESS;
        }
        else
        {
          stat = STATUS_INSUFFICIENT_RESOURCES;
          error_message(WRITER_INFO, "failed to allocate file name buffer");
        }
      }
      else
      {
        error_message(WRITER_INFO, "FltGetStreamHandleContext failed with status %!STATUS!", stat);
      }
    }

    ~writer_info_with_file_name()
    {
      if (file_name)
      {
        ExFreePool(file_name);
      }
    }

    bool is_equal_writer_info(const writer_info* wi) const
    {
      bool equal(false);
      if (get_pid() == wi->get_pid())
      {
        if (TRUE == RtlEqualUnicodeString(get_name(), wi->get_name(), TRUE))
        {
          equal = true;
        }
      }

      return equal;
    }

    UNICODE_STRING* get_name() const
    {
      return file_name;
    }

  private:
    UNICODE_STRING* file_name;
  };

  class top_writer_info : public writer_info_with_file_name
  {
  public:
    top_writer_info(NTSTATUS& stat, PFLT_CALLBACK_DATA data) : writer_info_with_file_name(stat, data)
    {}

    void* __cdecl operator new(size_t sz)
    {
      return ExAllocatePoolWithTag(NonPagedPoolNx, sz, 'fniw');
    }
  };
}

void __cdecl writer_info::operator delete(void* p)
{
  if (p)
  {
    ExFreePool(p);
  }
}

writer_info* create_writer_info(NTSTATUS& stat, PFLT_CALLBACK_DATA data)
{
  stat = STATUS_INSUFFICIENT_RESOURCES;
  auto wi = new top_writer_info(stat, data);
  if (!NT_SUCCESS(stat))
  {
    delete wi;
    wi = 0;
  }

  return wi;
}
