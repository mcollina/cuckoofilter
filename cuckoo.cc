#include "./cuckoo.h"
#include <napi.h>

template <size_t bits_per_item>
Napi::FunctionReference Cuckoo<bits_per_item>::constructor;

Napi::String Method(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::String::New(env, "world");
}

template <size_t bits_per_item>
Napi::Object Cuckoo<bits_per_item>::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "CuckooFilter" + std::to_string(bits_per_item), {
    InstanceMethod("add", &Cuckoo::Add),
    InstanceMethod("contain", &Cuckoo::Contain),
    InstanceMethod("delete", &Cuckoo::Delete),
    InstanceAccessor("size", &Cuckoo::Size, NULL),
    InstanceAccessor("bytes", &Cuckoo::SizeInBytes, NULL)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("CuckooFilter", func);
  return exports;
}

template <size_t bits_per_item>
Cuckoo<bits_per_item>::Cuckoo(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Cuckoo<bits_per_item>>(info)  {
  const Napi::Env env = info.Env();

  if (info.Length() != 1) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return;
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "You must pass the filter size").ThrowAsJavaScriptException();
    return;
  }

  Napi::Number num = info[0].As<Napi::Number>();

  this->filter = new cuckoofilter::CuckooFilter<Napi::String, bits_per_item, cuckoofilter::SingleTable, NapiStringHash>(num.Uint32Value());
}

template <size_t bits_per_item>
Cuckoo<bits_per_item>::~Cuckoo() {
  delete this->filter;
}

template <size_t bits_per_item>
Napi::Value Cuckoo<bits_per_item>::Add(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();

  if (info.Length() != 1) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Only strings are supported").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::String str = info[0].As<Napi::String>();

  // do not add twice, we cannot support unlimited adding
  // it will fail after ~10 add with the same parameter
  // otherwise
  if (this->filter->Contain(str) == cuckoofilter::Ok) {
    return info.This();
  }

  const int res = this->filter->Add(str);

  if (res == cuckoofilter::Ok) {
    return info.This();
  } else if (res == cuckoofilter::NotEnoughSpace) {
    Napi::Error::New(env, "Not enough space to add this key").ThrowAsJavaScriptException();
    return env.Null();
  } else {
    Napi::Error::New(env, "something went wrong during add").ThrowAsJavaScriptException();
    return env.Null();
  }
}

template <size_t bits_per_item>
Napi::Value Cuckoo<bits_per_item>::Contain(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();

  if (info.Length() != 1) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Only strings are supported").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::String str = info[0].As<Napi::String>();

  if (this->filter->Contain(str) == cuckoofilter::Ok) {
    return Napi::Boolean::New(env, true);
  } else {
    return Napi::Boolean::New(env, false);
  }
}

template <size_t bits_per_item>
Napi::Value Cuckoo<bits_per_item>::Delete(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();

  if (info.Length() != 1) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Only strings are supported").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::String str = info[0].As<Napi::String>();

  const int res = this->filter->Delete(str);

  if (res == cuckoofilter::Ok) {
    return info.This();
  } else if (res == cuckoofilter::NotFound) {
    // not found is ok too
    return info.This();
  } else {
    Napi::Error::New(env, "something went wrong during delete").ThrowAsJavaScriptException();
    return env.Null();
  }
}

template <size_t bits_per_item>
Napi::Value Cuckoo<bits_per_item>::Size(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Number::New(env, this->filter->Size());
}

template <size_t bits_per_item>
Napi::Value Cuckoo<bits_per_item>::SizeInBytes(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Number::New(env, this->filter->SizeInBytes());
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  Cuckoo<2>::Init(env, exports);
  Cuckoo<4>::Init(env, exports);
  Cuckoo<8>::Init(env, exports);
  Cuckoo<12>::Init(env, exports);
  Cuckoo<16>::Init(env, exports);
  Cuckoo<32>::Init(env, exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)
