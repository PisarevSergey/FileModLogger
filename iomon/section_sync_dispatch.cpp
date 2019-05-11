#include "common.h"
#include "section_sync_dispatch.tmh"

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
    info_message(SECTION_SYNC_DISPATCH, "acquiring file for section creation with read/write access");
    support::auto_flt_context<contexts::stream_context> sc;
    NTSTATUS stat(FltGetStreamContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, sc));
    if (NT_SUCCESS(stat))
    {
      info_message(SECTION_SYNC_DISPATCH, "FltGetStreamContext success");
      fs_stat = FLT_PREOP_SUCCESS_WITH_CALLBACK;
      *CompletionContext = sc.release();
    }
    else
    {
      error_message(SECTION_SYNC_DISPATCH, "FltGetStreamContext failed with status %!STATUS!", stat);
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
    info_message(SECTION_SYNC_DISPATCH, "operation is not draining");
    if (NT_SUCCESS(Data->IoStatus.Status))
    {
      info_message(SECTION_SYNC_DISPATCH, "file was acquired successfully for section creation");
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
    info_message(SECTION_SYNC_DISPATCH, "FltGetStreamContext success");
    if (sc->is_file_acquired_for_section_creation())
    {
      info_message(SECTION_SYNC_DISPATCH, "this file acquired for section creation");
      if (sc->is_section_ref_increased(Data))
      {
        stat = sc->insert_writer(Data);
      }

      sc->reset_file_acquired_for_section_creation();
    }
  }
  else
  {
    error_message(SECTION_SYNC_DISPATCH, "FltGetStreamContext failed with status %!STATUS!", stat);
  }

  return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

