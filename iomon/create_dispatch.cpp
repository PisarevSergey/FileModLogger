#include "common.h"

namespace
{
  NTSTATUS set_stream_context(PFLT_CALLBACK_DATA data)
  {
    NTSTATUS stat(STATUS_UNSUCCESSFUL);

    support::auto_flt_context<contexts::stream_context> new_ctx(contexts::allocate_stream_context(stat));
    if (NT_SUCCESS(stat))
    {
      support::auto_flt_context<contexts::stream_context> old_ctx;
      stat = FltSetStreamContext(data->Iopb->TargetInstance, data->Iopb->TargetFileObject, FLT_SET_CONTEXT_KEEP_IF_EXISTS, new_ctx, old_ctx);

      if (NT_SUCCESS(stat))
      {
        new_ctx->adjust_refcount_on_post_create();
      }
      else if ((STATUS_FLT_CONTEXT_ALREADY_DEFINED == stat) && old_ctx.operator bool())
      {
        old_ctx->adjust_refcount_on_post_create();
        stat = STATUS_SUCCESS;
      }
    }

    return stat;
  }
}

FLT_POSTOP_CALLBACK_STATUS operations::post_create(_Inout_  PFLT_CALLBACK_DATA       Data,
                                                   _In_     PCFLT_RELATED_OBJECTS    /*FltObjects*/,
                                                   _In_opt_ PVOID                    /*CompletionContext*/,
                                                   _In_     FLT_POST_OPERATION_FLAGS Flags)
{
  if (0 == (Flags & FLTFL_POST_OPERATION_DRAINING))
  {
    if (NT_SUCCESS(Data->IoStatus.Status) && (STATUS_REPARSE != Data->IoStatus.Status))
    {
      BOOLEAN is_dir;
      NTSTATUS stat(FltIsDirectory(Data->Iopb->TargetFileObject, Data->Iopb->TargetInstance, &is_dir));
      if (NT_SUCCESS(stat) && (FALSE == is_dir))
      {
        if (FALSE == FsRtlIsPagingFile(Data->Iopb->TargetFileObject))
        {
          stat = set_stream_context(Data);
        }

        if (!NT_SUCCESS(stat) && (0 == (Data->Iopb->TargetFileObject->Flags & FO_HANDLE_CREATED)))
        {
          FltCancelFileOpen(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject);
          Data->IoStatus.Status = stat;
        }

      }

    }
  }

  return FLT_POSTOP_FINISHED_PROCESSING;
}
