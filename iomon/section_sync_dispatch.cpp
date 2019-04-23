#include "common.h"

FLT_PREOP_CALLBACK_STATUS operations::pre_acquire_for_section_sync(
  _Inout_ PFLT_CALLBACK_DATA    Data,
  _In_    PCFLT_RELATED_OBJECTS /*FltObjects*/,
  _Out_   PVOID                 *CompletionContext
)
{
  FLT_PREOP_CALLBACK_STATUS fs_stat(FLT_PREOP_SUCCESS_NO_CALLBACK);

  if ((SyncTypeCreateSection == Data->Iopb->Parameters.AcquireForSectionSynchronization.SyncType) &&
      (PAGE_READWRITE == (PAGE_READWRITE & Data->Iopb->Parameters.AcquireForSectionSynchronization.PageProtection)))
  {
    support::auto_flt_context<contexts::stream_context> sc;
    NTSTATUS stat(FltGetStreamContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, sc));
    if (NT_SUCCESS(stat))
    {
      fs_stat = FLT_PREOP_SUCCESS_WITH_CALLBACK;
      *CompletionContext = sc.release();
    }
  }

  return fs_stat;
}

FLT_POSTOP_CALLBACK_STATUS operations::post_acquire_for_section_sync(
  _Inout_  PFLT_CALLBACK_DATA       Data,
  _In_     PCFLT_RELATED_OBJECTS    /*FltObjects*/,
  _In_opt_ PVOID                    CompletionContext,
  _In_     FLT_POST_OPERATION_FLAGS Flags)
{
  support::auto_flt_context<contexts::stream_context> sc(CompletionContext);

  if (0 == (Flags & FLTFL_POST_OPERATION_DRAINING))
  {
    if (NT_SUCCESS(Data->IoStatus.Status))
    {
      sc->set_file_acquired_for_section_creation();
      sc->set_number_of_section_refs(Data);
    }

  }

  return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS operations::pre_release_for_section_sync(
  _Inout_ PFLT_CALLBACK_DATA    Data,
  _In_    PCFLT_RELATED_OBJECTS /*FltObjects*/,
  _Out_   PVOID*  /*CompletionContext*/)
{
  support::auto_flt_context<contexts::stream_context> sc;
  NTSTATUS stat(FltGetStreamContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, sc));
  if (NT_SUCCESS(stat))
  {
    if (sc->is_file_acquired_for_section_creation())
    {
      if (sc->is_section_ref_increased(Data))
      {
      }

      sc->reset_file_acquired_for_section_creation();
    }
  }

  return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

