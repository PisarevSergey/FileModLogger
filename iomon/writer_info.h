#pragma once

class writer_info : public LIST_ENTRY
{
public:
  virtual bool is_equal_writer_info(const writer_info* wi) const = 0;

  virtual HANDLE get_pid() const = 0;
  virtual UNICODE_STRING* get_name() const = 0;

  writer_info() { InitializeListHead(this); }
  void __cdecl operator delete(void*);
  virtual ~writer_info() {}
};

writer_info* create_writer_info(NTSTATUS& stat, PFLT_CALLBACK_DATA data);
