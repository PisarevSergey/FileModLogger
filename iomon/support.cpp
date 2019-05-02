#include "common.h"

bool support::does_file_have_mapped_views(PFILE_OBJECT fo)
{
  bool has(false);

  if (fo->SectionObjectPointer && fo->SectionObjectPointer->DataSectionObject)
  {
    auto ca(static_cast<undocumented_structs::control_area*>(fo->SectionObjectPointer->DataSectionObject));
    has = ca->NumberOfMappedViews ? true : false;
  }

  return has;
}
