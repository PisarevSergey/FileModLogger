#include "common.h"
#include "create_dispatch.tmh"

namespace
{
  NTSTATUS set_stream_context(PFLT_CALLBACK_DATA data)
  {
    NTSTATUS stat(STATUS_UNSUCCESSFUL);

    support::auto_flt_context<contexts::stream_context> new_ctx(contexts::allocate_stream_context(stat));
    if (NT_SUCCESS(stat))
    {
      info_message(CREATE_DISPATCH, "allocate_stream_context success");
      support::auto_flt_context<contexts::stream_context> old_ctx;
      stat = FltSetStreamContext(data->Iopb->TargetInstance, data->Iopb->TargetFileObject, FLT_SET_CONTEXT_KEEP_IF_EXISTS, new_ctx, old_ctx);

      if (NT_SUCCESS(stat))
      {
        info_message(CREATE_DISPATCH, "FltSetStreamContext success");
        new_ctx->adjust_refcount_on_post_create();
      }
      else if ((STATUS_FLT_CONTEXT_ALREADY_DEFINED == stat) && old_ctx.operator bool())
      {
        info_message(CREATE_DISPATCH, "context already set, using old context");
        old_ctx->adjust_refcount_on_post_create();
        stat = STATUS_SUCCESS;
      }
      else
      {
        error_message(CREATE_DISPATCH, "FltSetStreamContext failed with status %!STATUS!", stat);
      }
    }
    else
    {
      error_message(CREATE_DISPATCH, "allocate_stream_context failed with status %!STATUS!", stat);
    }

    return stat;
  }

  NTSTATUS set_stream_handle_context(PFLT_CALLBACK_DATA data)
  {
    NTSTATUS stat(STATUS_UNSUCCESSFUL);

    support::auto_flt_context<contexts::stream_handle_context> new_ctx(contexts::allocate_stream_handle_context(stat, data));
    if (NT_SUCCESS(stat))
    {
      info_message(CREATE_DISPATCH, "allocate_stream_handle_context success");
      support::auto_flt_context<contexts::stream_handle_context> old_ctx;
      stat = FltSetStreamHandleContext(data->Iopb->TargetInstance, data->Iopb->TargetFileObject, FLT_SET_CONTEXT_KEEP_IF_EXISTS, new_ctx, old_ctx);

      if (NT_SUCCESS(stat))
      {
        info_message(CREATE_DISPATCH, "FltSetStreamHandleContext success");
      }
      else if ((STATUS_FLT_CONTEXT_ALREADY_DEFINED == stat) && old_ctx.operator bool())
      {
        info_message(CREATE_DISPATCH, "stream handle context already set");
        stat = STATUS_SUCCESS;
      }
      else
      {
        error_message(CREATE_DISPATCH, "FltSetStreamHandleContext failed with status %!STATUS!", stat);
      }
    }
    else
    {
      error_message(CREATE_DISPATCH, "allocate_stream_handle_context failed with status %!STATUS!", stat);
    }

    return stat;
  }

}

FLT_POSTOP_CALLBACK_STATUS
operations::post_create(_Inout_  PFLT_CALLBACK_DATA       Data,
                        _In_     PCFLT_RELATED_OBJECTS    /*FltObjects*/,
                        _In_opt_ PVOID                    /*CompletionContext*/,
                        _In_     FLT_POST_OPERATION_FLAGS Flags)
{
  if (0 == (Flags & FLTFL_POST_OPERATION_DRAINING))
  {
    info_message(CREATE_DISPATCH, "post create is not draining");

    if (NT_SUCCESS(Data->IoStatus.Status) && (STATUS_REPARSE != Data->IoStatus.Status))
    {
      info_message(CREATE_DISPATCH, "create success and not reparse");

      BOOLEAN is_dir;
      NTSTATUS stat(FltIsDirectory(Data->Iopb->TargetFileObject, Data->Iopb->TargetInstance, &is_dir));
      if (NT_SUCCESS(stat) && (FALSE == is_dir))
      {
        info_message(CREATE_DISPATCH, "file opened is not directory");
        if (FALSE == FsRtlIsPagingFile(Data->Iopb->TargetFileObject))
        {
          info_message(CREATE_DISPATCH, "file opened is not paging file");
          stat = set_stream_context(Data);
          if (NT_SUCCESS(stat))
          {
            info_message(CREATE_DISPATCH, "set_stream_context success");
            stat = set_stream_handle_context(Data);
            if (NT_SUCCESS(stat))
            {
              info_message(CREATE_DISPATCH, "set_stream_handle_context success");
            }
            else
            {
              error_message(CREATE_DISPATCH, "set_stream_handle_context failed with status %!STATUS!", stat);
            }
          }
          else
          {
            error_message(CREATE_DISPATCH, "set_stream_context failed with status %!STATUS!", stat);
          }
        }
      }

      if (!NT_SUCCESS(stat) && (0 == (Data->Iopb->TargetFileObject->Flags & FO_HANDLE_CREATED)))
      {
        error_message(CREATE_DISPATCH, "post create dispatching failed with status %!STATUS!, canceling open", stat);
        FltCancelFileOpen(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject);
        Data->IoStatus.Status = stat;
      }

    }
  }

  return FLT_POSTOP_FINISHED_PROCESSING;
}
