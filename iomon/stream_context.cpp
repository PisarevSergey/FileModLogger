#include "common.h"

namespace
{
  class ref_counter_stream_context : public contexts::stream_context
  {
  public:
    ref_counter_stream_context() : ref_count(0)
    {
      KeInitializeSpinLock(&guard);
    }

    void adjust_refcount_on_post_create()
    {
      KLOCK_QUEUE_HANDLE lh;
      lock_counter(&lh);
      ++ref_count;
      unlock_counter(&lh);
    }

    bool adjust_refcount_on_pre_close_and_check_for_last_close(PFLT_CALLBACK_DATA data)
    {
      bool last_close(false);

      PFILE_OBJECT cache_fo(0);

      if (data->Iopb->TargetFileObject->SectionObjectPointer)
      {
        cache_fo = CcGetFileObjectFromSectionPtrsRef(data->Iopb->TargetFileObject->SectionObjectPointer);
      }

      KLOCK_QUEUE_HANDLE lh;
      lock_counter(&lh);

      if ((0 == (data->Iopb->TargetFileObject->Flags & FO_STREAM_FILE)) && (data->Iopb->TargetFileObject != cache_fo))
      {
        --ref_count;
      }

      if (0 == ref_count)
      {
        if (data->Iopb->TargetFileObject->SectionObjectPointer)
        {
          if ((0 == data->Iopb->TargetFileObject->SectionObjectPointer->DataSectionObject) &&
              (0 == data->Iopb->TargetFileObject->SectionObjectPointer->ImageSectionObject))
          {
            last_close = true;
          }
        }
        else
        {
          last_close = true;
        }
      }

      unlock_counter(&lh);

      if (cache_fo)
      {
        ObDereferenceObject(cache_fo);
      }

      return last_close;
    }

  private:
    void lock_counter(PKLOCK_QUEUE_HANDLE lh)
    {
      KeAcquireInStackQueuedSpinLock(&guard, lh);
    }

    void unlock_counter(PKLOCK_QUEUE_HANDLE lh)
    {
      KeReleaseInStackQueuedSpinLock(lh);
    }
  private:
    KSPIN_LOCK guard;
    unsigned __int64 ref_count;
  };

  class top_stream_context : public ref_counter_stream_context
  {};
}

const size_t contexts::get_stream_context_size() { return sizeof(top_stream_context); }

contexts::stream_context* contexts::allocate_stream_context(NTSTATUS& stat)
{
  PFLT_CONTEXT ctx(0);

  stat = FltAllocateContext(get_driver()->get_filter(), FLT_STREAM_CONTEXT, contexts::get_stream_context_size(), NonPagedPool, &ctx);
  if (!NT_SUCCESS(stat))
  {
    ctx = 0;
  }

  return static_cast<contexts::stream_context*>(ctx);
}

void contexts::stream_context_cleanup_callback(_In_ PFLT_CONTEXT ctx,
                                               _In_ FLT_CONTEXT_TYPE)
{
  delete static_cast<contexts::stream_context*>(ctx);
}

