#include "common.h"

FLT_PREOP_CALLBACK_STATUS operations::pre_create(_Inout_ PFLT_CALLBACK_DATA    Data,
                                                 _In_    PCFLT_RELATED_OBJECTS /*FltObjects*/,
                                                 _Out_   PVOID                 *CompletionContext)
{
  FLT_PREOP_CALLBACK_STATUS fs_stat(FLT_PREOP_COMPLETE);

  NTSTATUS stat(STATUS_UNSUCCESSFUL);
  auto sc(contexts::allocate_stream_context(stat));
  if (NT_SUCCESS(stat))
  {
    *CompletionContext = sc;
    fs_stat = FLT_PREOP_SUCCESS_WITH_CALLBACK;
  }

  if (!NT_SUCCESS(stat))
  {
    Data->IoStatus.Status = stat;
    ASSERT(FLT_PREOP_COMPLETE == fs_stat);
  }

  return fs_stat;
}

FLT_POSTOP_CALLBACK_STATUS operations::post_create(_Inout_  PFLT_CALLBACK_DATA       Data,
                                                   _In_     PCFLT_RELATED_OBJECTS    /*FltObjects*/,
                                                   _In_opt_ PVOID                    CompletionContext,
                                                   _In_     FLT_POST_OPERATION_FLAGS Flags)
{
  support::auto_flt_context<contexts::stream_context> new_ctx(CompletionContext);
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
          support::auto_flt_context<contexts::stream_context> old_ctx;
          stat = FltSetStreamContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, FLT_SET_CONTEXT_KEEP_IF_EXISTS, new_ctx, old_ctx);

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
