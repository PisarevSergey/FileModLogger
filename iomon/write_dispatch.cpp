#include "common.h"
#include "write_dispatch.tmh"

FLT_PREOP_CALLBACK_STATUS
operations::pre_write(
  _Inout_ PFLT_CALLBACK_DATA    Data,
  _In_    PCFLT_RELATED_OBJECTS /*FltObjects*/,
  _Out_   PVOID*  CompletionContext
)
{
  FLT_PREOP_CALLBACK_STATUS fs_stat(FLT_PREOP_SUCCESS_NO_CALLBACK);

  if (Data->Iopb->IrpFlags & IRP_PAGING_IO)
  {
    if (Data->Iopb->TargetFileObject->SectionObjectPointer && MmDoesFileHaveUserWritableReferences(Data->Iopb->TargetFileObject->SectionObjectPointer))
    {
      if (support::does_file_have_mapped_views(Data->Iopb->TargetFileObject))
      {
        support::auto_flt_context<contexts::stream_context> sc;
        NTSTATUS stat = FltGetStreamContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, sc);
        if (NT_SUCCESS(stat))
        {
          info_message(WRITE_DISPATCH, "FltGetStreamContext success");
          *CompletionContext = sc.release();
          fs_stat = FLT_PREOP_SUCCESS_WITH_CALLBACK;
        }
        else
        {
          error_message(WRITE_DISPATCH, "FltGetStreamContext failed with status %!STATUS!", stat);
        }
      }
    }
  }

  return fs_stat;
}

FLT_POSTOP_CALLBACK_STATUS
operations::post_write(
  _Inout_  PFLT_CALLBACK_DATA       Data,
  _In_     PCFLT_RELATED_OBJECTS    /*FltObjects*/,
  _In_opt_ PVOID                    CompletionContext,
  _In_     FLT_POST_OPERATION_FLAGS Flags
)
{
  FLT_POSTOP_CALLBACK_STATUS fs_stat(FLT_POSTOP_FINISHED_PROCESSING);

  support::auto_flt_context<contexts::stream_context> sc(CompletionContext);

  if (0 == (FLTFL_POST_OPERATION_DRAINING & Flags))
  {
    if (NT_SUCCESS(Data->IoStatus.Status))
    {
      sc->increase_total_written_len(Data->Iopb->Parameters.Write.Length);
    }
  }

  return fs_stat;
}
