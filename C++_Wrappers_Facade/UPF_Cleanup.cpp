#include "UPF_Cleanup.h"


UPF_Cleanup::UPF_Cleanup (void)
{
}

void
UPF_Cleanup::cleanup (void *)
{
  delete this;
}


UPF_Cleanup::~UPF_Cleanup (void)
{
}

/*****************************************************************************/

extern "C" void
UPF_CLEANUP_DESTROYER_NAME (UPF_Cleanup *object, void *param)
{
  object->cleanup (param);
}

/*****************************************************************************/

UPF_Cleanup_Info::UPF_Cleanup_Info (void)
  : object_ (0),
    cleanup_hook_ (0),
    param_ (0)
{
}

bool
UPF_Cleanup_Info::operator== (const UPF_Cleanup_Info &o) const
{
  return o.object_ == this->object_
    && o.cleanup_hook_ == this->cleanup_hook_
    && o.param_ == this->param_;
}

bool
UPF_Cleanup_Info::operator!= (const UPF_Cleanup_Info &o) const
{
  return !(*this == o);
}