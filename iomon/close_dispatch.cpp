#include "common.h"
#include "close_dispatch.tmh"

FLT_PREOP_CALLBACK_STATUS operations::pre_close(
  _Inout_ PFLT_CALLBACK_DATA    Data,
  _In_    PCFLT_RELATED_OBJECTS /*FltObjects*/,
  _Out_   PVOID*  /*CompletionContext*/
)
{
  support::auto_flt_context<contexts::stream_context> sc;
  NTSTATUS stat(FltGetStreamContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, sc));
  if (NT_SUCCESS(stat))
  {
    info_message(CLOSE_DISPATCH, "FltGetStreamContext success");
    if (sc->adjust_refcount_on_pre_close_and_check_for_last_close(Data))
    {
      if (sc->get_total_written_len())
      {
        while (auto wi = sc->extract_next_writer())
        {
          info_message(CLOSE_REPORT, "write by process %p for file %wZ", wi->get_pid(), wi->get_name());

          delete wi;
        }
        info_message(CLOSE_REPORT, "totally written %I64x", sc->get_total_written_len());
      }
    }
  }
  else
  {
    error_message(CLOSE_DISPATCH, "FltGetStreamContext failed with status %!STATUS!", stat);
  }

  return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
