#pragma once

namespace undocumented_structs
{
  struct control_area
  {
    void* segment;
    LIST_ENTRY head;
    ULONG_PTR NumberOfSectionReferences;
    ULONG_PTR NumberOfPfnReferences    ;
    ULONG_PTR NumberOfMappedViews      ;
    ULONG_PTR NumberOfUserReferences   ;

  };
}
