#include "common.h"

namespace
{
  class sh_context_with_pid : public contexts::stream_handle_context
  {
  public:
    sh_context_with_pid(PFLT_CALLBACK_DATA data) : pid(FltGetRequestorProcessIdEx(data))
    {}
  private:
    HANDLE pid;
  };

  class sh_context_with_name : public sh_context_with_pid
  {
  public:
    sh_context_with_name(NTSTATUS& stat, PFLT_CALLBACK_DATA data) : sh_context_with_pid(data), fni(0)
    {
      stat = FltGetFileNameInformation(data, FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_DEFAULT, &fni);
      if (!NT_SUCCESS(stat))
      {
        fni = 0;
      }
    }

    ~sh_context_with_name()
    {
      if (fni)
      {
        FltReleaseFileNameInformation(fni);
      }
    }
  private:
    PFLT_FILE_NAME_INFORMATION fni;
  };

  class top_sh_context : public sh_context_with_name
  {
  public:
    top_sh_context(NTSTATUS& stat, PFLT_CALLBACK_DATA data) : sh_context_with_name(stat, data)
    {}

    void* __cdecl operator new(size_t, void* p) { return p; }
  };
}

const size_t contexts::get_stream_handle_context_size() { return sizeof(top_sh_context); }

void contexts::stream_handle_context_cleanup_callback(_In_ PFLT_CONTEXT ctx,
                                                      _In_ FLT_CONTEXT_TYPE)
{
  delete static_cast<contexts::stream_handle_context*>(ctx);
}

contexts::stream_handle_context* contexts::allocate_stream_handle_context(NTSTATUS& stat, PFLT_CALLBACK_DATA data)
{
  PFLT_CONTEXT ctx(0);
  stat = FltAllocateContext(get_driver()->get_filter(), FLT_STREAMHANDLE_CONTEXT, contexts::get_stream_handle_context_size(), PagedPool, &ctx);
  if (NT_SUCCESS(stat))
  {
    new(ctx)top_sh_context(stat, data);
    if (NT_SUCCESS(stat))
    {
    }
    else
    {
      FltReleaseContext(ctx);
    }
  }
  else
  {
    ctx = 0;
  }

  return static_cast<contexts::stream_handle_context*>(ctx);
}
