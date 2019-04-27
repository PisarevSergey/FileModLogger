#pragma once

namespace operations
{
  FLT_PREOP_CALLBACK_STATUS pre_acquire_for_section_sync(
    _Inout_ PFLT_CALLBACK_DATA    Data,
    _In_    PCFLT_RELATED_OBJECTS FltObjects,
    _Out_   PVOID                 *CompletionContext
  );

  FLT_POSTOP_CALLBACK_STATUS post_acquire_for_section_sync(
    _Inout_  PFLT_CALLBACK_DATA       Data,
    _In_     PCFLT_RELATED_OBJECTS    FltObjects,
    _In_opt_ PVOID                    CompletionContext,
    _In_     FLT_POST_OPERATION_FLAGS Flags
    );

  FLT_PREOP_CALLBACK_STATUS pre_release_for_section_sync(
    _Inout_ PFLT_CALLBACK_DATA    Data,
    _In_    PCFLT_RELATED_OBJECTS FltObjects,
    _Out_   PVOID                 *CompletionContext
    );
}