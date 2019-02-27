#ifndef CUCKOO_H
#define CUCKOO_H

#include <napi.h>
#include "cuckoofilter.h"

class NapiStringHash {
  public:
    uint64_t operator()(Napi::String str) const {
      return cuckoofilter::HashUtil::BobHash(str);
    }
};

template <size_t bits_per_item>
class Cuckoo : public Napi::ObjectWrap<Cuckoo<bits_per_item>> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Cuckoo(const Napi::CallbackInfo& info);
  ~Cuckoo();

 private:
  Napi::Value Add(const Napi::CallbackInfo& info);
  Napi::Value Contain(const Napi::CallbackInfo& info);
  Napi::Value Delete(const Napi::CallbackInfo& info);
  Napi::Value Size(const Napi::CallbackInfo& info);
  Napi::Value SizeInBytes(const Napi::CallbackInfo& info);

  static Napi::FunctionReference constructor;
  cuckoofilter::CuckooFilter<Napi::String, bits_per_item, cuckoofilter::SingleTable, NapiStringHash> *filter;
};

#endif
