#pragma once

namespace contexts
{
  class stream_handle_context
  {
  public:
    virtual ~stream_handle_context() {}
    void __cdecl operator delete(void*) {}
  };

  const size_t get_stream_handle_context_size();

  void stream_handle_context_cleanup_callback(_In_ PFLT_CONTEXT     Context,
    _In_ FLT_CONTEXT_TYPE ContextType);

  stream_handle_context* allocate_stream_handle_context(NTSTATUS& stat, PFLT_CALLBACK_DATA data);
}
