#pragma once

namespace operations
{
  FLT_PREOP_CALLBACK_STATUS pre_close(
    _Inout_ PFLT_CALLBACK_DATA    Data,
    _In_    PCFLT_RELATED_OBJECTS FltObjects,
    _Out_   PVOID                 *CompletionContext
  );
}