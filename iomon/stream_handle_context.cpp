#include "common.h"
#include "stream_handle_context.tmh"

namespace
{
  class sh_context_with_name : public contexts::stream_handle_context
  {
  public:
    sh_context_with_name(NTSTATUS& stat, PFLT_CALLBACK_DATA data) : fni(0)
    {
      stat = FltGetFileNameInformation(data, FLT_FILE_NAME_NORMALIZED
        | FLT_FILE_NAME_QUERY_DEFAULT, &fni);
      if (NT_SUCCESS(stat))
      {
        info_message(STREAM_HANDLE_CONTEXT, "FltGetFileNameInformation success, file name is %wZ", &fni->Name);
      }
      else
      {
        error_message(STREAM_HANDLE_CONTEXT, "FltGetFileNameInformation failed with status %!STATUS!", stat);
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

    virtual UNICODE_STRING* get_file_name() const
    {
      return &fni->Name;
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
    info_message(STREAM_HANDLE_CONTEXT, "FltAllocateContext success");
    new(ctx)top_sh_context(stat, data);
    if (NT_SUCCESS(stat))
    {
      info_message(STREAM_HANDLE_CONTEXT, "stream handle context init success");
    }
    else
    {
      error_message(STREAM_HANDLE_CONTEXT, "stream handle context init failed with status %!STATUS!", stat);
      FltReleaseContext(ctx);
    }
  }
  else
  {
    error_message(STREAM_HANDLE_CONTEXT, "FltAllocateContext failed with status %!STATUS!", stat);
    ctx = 0;
  }

  return static_cast<contexts::stream_handle_context*>(ctx);
}
