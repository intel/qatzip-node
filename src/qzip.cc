/*
* Copyright (C) <2023-2023> Intel Corporation
* SPDX-License-Identifier: MIT
*/

#include <iostream>
#include <string>
#include <thread>

#include <napi.h>
#include <qatzip.h>
// #include <iomanip>

using namespace std;
using namespace Napi;

QzSession_T sess;
QzSessionParamsDeflate_T qz_params;

int InitQzipSession(QzSession_T& session, QzSessionParamsDeflate_T& params, int _level) {
  int rc = qzInit(&session, 1);
  if (rc < 0) {
    return rc;
  }

  rc = qzGetDefaultsDeflate(&params);
  if (rc < 0) {
    return rc;
  }

  int level = _level;

  params.data_fmt = QZ_DEFLATE_GZIP_EXT;
  params.huffman_hdr = QZ_HUFF_HDR_DEFAULT;
  params.common_params.comp_lvl = level;
  params.common_params.comp_algorithm = QZ_DEFLATE;
  params.common_params.direction = QZ_DIRECTION_DEFAULT;
  params.common_params.hw_buff_sz = QZ_HW_BUFF_SZ;
  params.common_params.polling_mode = QZ_BUSY_POLLING;
  params.common_params.input_sz_thrshold = QZ_COMP_THRESHOLD_DEFAULT;
  params.common_params.req_cnt_thrshold = 32;
  params.common_params.max_forks = QZ_MAX_FORK_DEFAULT;
  params.common_params.sw_backup = 1;

  rc = qzSetupSessionDeflate(&session, &params);
  if (rc != QZ_OK) {
    std::cerr << "qzSetupSessionDeflate failed with error: " << rc << std::endl;
    return rc;
  }
  return rc;
}

Napi::Value DeflateSync(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsTypedArray() || !info[1].IsObject()) {
    Napi::TypeError::New(env, "Wrong type of argumentsarguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Object options = info[1].ToObject();
  int level = options.Get("level").ToNumber().Int32Value();

  int rc = InitQzipSession(sess, qz_params, level);
  if (rc < 0) {
    std::cout << "Session setup failed with error: " << rc << std::endl;
    return env.Null();
  }

  // get src_buf
  Napi::TypedArray src_arr = info[0].As<Napi::TypedArray>();
  const unsigned char* src_buf =
    static_cast<const unsigned char*>(src_arr.ArrayBuffer().Data()) +
    src_arr.ByteOffset();
  unsigned int src_len = src_arr.ByteLength();

  unsigned int dst_len = qzMaxCompressedLength(src_len, &sess);
  unsigned char *dst_buf = new(std::nothrow) unsigned char[dst_len];
  if(dst_buf == nullptr) {
    std::cerr << "Memory allocation failed!" << std::endl;
    return env.Null();
  }

  //compression
  rc = qzCompress(&sess, src_buf, &src_len, dst_buf, &dst_len, 1);
  if (rc != QZ_OK) {
    std::cout << "qzCompress wrong return value : " << rc << endl;
    delete[] dst_buf;
    return env.Null();
  }

  Napi::Buffer<unsigned char> dst_wrap = Napi::Buffer<unsigned char>::Copy(env, dst_buf, dst_len);

  delete[] dst_buf;
  dst_buf = nullptr;
  qzTeardownSession(&sess);
  qzClose(&sess);
  return dst_wrap;
}


// decompression
// Usage: const dst_inflate = qzip.inflateSync(compressedData);
/*
 1. get env
 2. check args.num == 1, args[1].type == buffer
*/

Napi::Value InflateSync(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsBuffer()) {
    Napi::TypeError::New(env, "First argument must be a Buffer")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  // Get the compressed buffer
  Napi::TypedArray src_arr = info[0].As<Napi::TypedArray>();
  const unsigned char* src_buf =
    static_cast<const unsigned char*>(src_arr.ArrayBuffer().Data()) +
    src_arr.ByteOffset();
  unsigned int src_len = src_arr.ByteLength();

  Napi::Object options = {};
  int level = 1;

  int rc = InitQzipSession(sess, qz_params, level);
  if (rc < 0) {
    std::cout << "Session setup failed with error: " << rc << std::endl;
    return env.Null();
  }

  unsigned int out_size = 3 * src_len;
  unsigned char* out_buffer = new(std::nothrow) unsigned char[out_size];
  if(out_buffer == nullptr) {
    std::cerr << "Memory allocation failed!" << std::endl;
    return env.Null();
  }

  rc = qzDecompress(&sess, src_buf, &src_len, out_buffer, &out_size);

  if (rc != QZ_OK) {
    std::cerr << "Decompression failed with error: " << rc << std::endl;
    delete[] out_buffer;
    return env.Null();
  }

  Napi::Buffer<unsigned char> dst_wrap = Napi::Buffer<unsigned char>::Copy(env, out_buffer, out_size);

  delete[] out_buffer;
  out_buffer = nullptr;
  qzTeardownSession(&sess);
  qzClose(&sess);
  return dst_wrap;
}

// async deflate/inflate 
class DeflateWorker : public Napi::AsyncWorker {
public:
  DeflateWorker(const Napi::Function& callback,
                const unsigned char* src_buf,
                unsigned int src_len,
                int level, int isDeflate) 
    : Napi::AsyncWorker(callback),
      _level(level),
      _src_len(src_len),
      _out_size(0),
      _isDeflate(isDeflate)
  {
    // Allocate memory for the deep copy
    _src_buf = new unsigned char[src_len];

    // Perform the deep copy
    memcpy(_src_buf, src_buf, src_len);
  }

  ~DeflateWorker() {
    if (_src_buf) {
      delete[] _src_buf;
      _src_buf = nullptr;
    }

    if (_out_buffer) {
      delete[] _out_buffer;
      _out_buffer = nullptr;
    }
  }

  void Execute();
  void OnOK();
  void OnError();

private:
  Napi::Value _result;
  int _level;
  unsigned char* _src_buf = nullptr;
  unsigned int _src_len;
  unsigned char* _out_buffer = nullptr;
  unsigned int _out_size;
  bool _isDeflate;
};

void DeflateWorker::Execute() {
  thread_local QzSession_T sess;
  thread_local QzSessionParamsDeflate_T qz_params;

  int rc = InitQzipSession(sess, qz_params, _level);
  if (rc < 0) {
    std::cout << "Session setup failed with error: " << rc << std::endl;
  }

  if (_isDeflate) {
    _out_size = qzMaxCompressedLength(_src_len, &sess);
    _out_buffer = new unsigned char[_out_size];
    if (!_out_buffer) {
      std::cerr << "Memory allocation for _out_buffer failed in Execute()" << std::endl;
      throw std::runtime_error("Memory allocation for _out_buffer failed");
    }

    rc = qzCompress(&sess, _src_buf, &_src_len, _out_buffer, &_out_size, 1);

    if (rc != QZ_OK) {
      std::cerr << "Deflate Worker: qzCompress() failed with error: " << rc << std::endl;
      delete[] _out_buffer;
      _out_buffer = nullptr;
      return;
    }} 
  else {
    _out_size = 3 * _src_len;
    _out_buffer = new unsigned char[_out_size];
    if (!_out_buffer) {
      throw std::runtime_error("Memory allocation for _out_buffer failed");
    }

    rc = qzDecompress(&sess, _src_buf, &_src_len, _out_buffer, &_out_size);

    if (rc != QZ_OK) {
      std::cerr << "Deflate Worker: qzDecompress() failed with error: " << rc << std::endl;
      delete[] _out_buffer;
      _out_buffer = nullptr;
      return;
    }
  }

  qzTeardownSession(&sess);
  qzClose(&sess);
  return;
}

void DeflateWorker::OnOK() {
  Napi::Env env = Env();
  Napi::HandleScope scope(env);

  _result = Napi::Buffer<unsigned char>::Copy(env, _out_buffer, _out_size);
  Callback().Call({env.Null(), _result});
}

void DeflateWorker::OnError() {
  // handle error
  cerr << "DeflateWorker::OnError()" << endl;
}

Napi::Value DeflateAsync(const Napi::CallbackInfo& info) {
  // parase args
  Napi::Env env = info.Env();
  Napi::HandleScope scope(info.Env());

  if (info.Length() < 3) {
    Napi::TypeError::New(env, "Wrong number of arguments")
    .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsTypedArray() || !info[1].IsObject() || !info[2].IsFunction()) {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  // new DeflateWorker and call Excute()
  Napi::TypedArray srcArr = info[0].As<Napi::TypedArray>();
  Napi::Object options = info[1].ToObject();
  Napi::Function callback = info[2].As<Napi::Function>();
  int level = options.Get("level").ToNumber().Int32Value();
  const unsigned char* src_buf = static_cast<const unsigned char*>(srcArr.ArrayBuffer().Data()) +
                                  srcArr.ByteOffset();
  unsigned int src_len = srcArr.ByteLength();

  DeflateWorker* worker = new DeflateWorker(callback, src_buf, src_len, level, true);
  worker->Queue();

  return env.Undefined();
}

Napi::Value InflateAsync(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(info.Env());

  if (info.Length() < 3) {
    Napi::TypeError::New(env, "Wrong number of arguments")
    .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsTypedArray() || !info[1].IsObject() || !info[2].IsFunction()) {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::TypedArray srcArr = info[0].As<Napi::TypedArray>();
  Napi::Object options = {}; // Empty object for InflateAsync
  Napi::Function callback = info[2].As<Napi::Function>();
  int level = 1; // level will not be used in inflate API
  const unsigned char* src_buf = static_cast<const unsigned char*>(srcArr.ArrayBuffer().Data()) +
                                  srcArr.ByteOffset();
  unsigned int src_len = srcArr.ByteLength();

  DeflateWorker* worker = new DeflateWorker(callback, src_buf, src_len, level, false);
  worker->Queue();

  return env.Undefined();
}


Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "deflateSync"),
              Napi::Function::New(env, DeflateSync));
  exports.Set(Napi::String::New(env, "inflateSync"),
              Napi::Function::New(env, InflateSync));
  exports.Set(Napi::String::New(env, "deflateAsync"),
              Napi::Function::New(env, DeflateAsync));
  exports.Set(Napi::String::New(env, "inflateAsync"),
              Napi::Function::New(env, InflateAsync));      
  return exports;
}

NODE_API_MODULE(qzip, Init)
