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
    writer_info_with_file_name(NTSTATUS& stat, PFLT_CALLBACK_DATA data) : writer_info_with_pid(data), fni(0)
    {
      stat = FltGetFileNameInformation(data, FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_DEFAULT, &fni);
      if (!NT_SUCCESS(stat))
      {
        fni = 0;
      }
    }

    ~writer_info_with_file_name()
    {
      if (fni)
      {
        FltReleaseFileNameInformation(fni);
      }
    }

    UNICODE_STRING* get_name() const
    {
      return &fni->Name;
    }

  private:
    PFLT_FILE_NAME_INFORMATION fni;
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
