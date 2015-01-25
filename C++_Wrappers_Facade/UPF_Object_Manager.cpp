#include "UPF_Object_Manager.h"


// Singleton pointer.
UPF_Object_Manager *UPF_Object_Manager::instance_ = 0;

void *UPF_Object_Manager::preallocated_object[UPF_Object_Manager::UPF_PREALLOCATED_OBJECTS] = { 0 };
void *UPF_Object_Manager::preallocated_array[UPF_Object_Manager::UPF_PREALLOCATED_ARRAYS] = { 0 };


int
UPF_Object_Manager::starting_up (void)
{
  return UPF_Object_Manager::instance_  ?  instance_->starting_up_i ()  :  1;
}

int
UPF_Object_Manager::shutting_down (void)
{
  return UPF_Object_Manager::instance_  ?  instance_->shutting_down_i ()  :  1;
}