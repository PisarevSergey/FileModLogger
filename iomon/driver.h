#pragma once

class driver
{
public:
  virtual NTSTATUS start_filtering() = 0;
  virtual PFLT_FILTER get_filter() = 0;

  virtual ~driver() {}
  void __cdecl operator delete(void*) {}
};

driver* create_driver(NTSTATUS& stat, PDRIVER_OBJECT driver);
driver* get_driver();
