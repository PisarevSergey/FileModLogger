#include "common.h"
#include "write_dispatch.tmh"

FLT_PREOP_CALLBACK_STATUS operations::pre_write(
  _Inout_ PFLT_CALLBACK_DATA    Data,
  _In_    PCFLT_RELATED_OBJECTS /*FltObjects*/,
  _Out_   PVOID*  /*CompletionContext*/
)
{
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
          im(WRITE_DISPATCH, "FltGetStreamContext success");
          sc->increase_total_written_len(Data->Iopb->Parameters.Write.Length);
        }
        else
        {
          em(WRITE_DISPATCH, "FltGetStreamContext failed with status %!STATUS!", stat);
        }
      }
    }
  }

  return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
