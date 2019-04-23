#pragma once

namespace contexts
{
  class stream_context
  {
  public:
    virtual void adjust_refcount_on_post_create() = 0;
    virtual bool adjust_refcount_on_pre_close_and_check_for_last_close(PFLT_CALLBACK_DATA) = 0;

    virtual bool is_file_acquired_for_section_creation() const = 0;
    virtual void set_file_acquired_for_section_creation()      = 0;
    virtual void reset_file_acquired_for_section_creation()    = 0;

    virtual void set_number_of_section_refs(PFLT_CALLBACK_DATA data) = 0;
    virtual bool is_section_ref_increased(PFLT_CALLBACK_DATA data) = 0;

    virtual NTSTATUS insert_writer(PFLT_CALLBACK_DATA data) = 0;

    virtual ~stream_context() {}
    void __cdecl operator delete(void*) {}
  };

  const size_t get_stream_context_size();

  void stream_context_cleanup_callback(_In_ PFLT_CONTEXT     Context,
                                       _In_ FLT_CONTEXT_TYPE ContextType);

  stream_context* allocate_stream_context(NTSTATUS& stat);
}
