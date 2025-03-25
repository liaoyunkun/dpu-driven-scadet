// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
#ifndef X_HLS_NP_CHANNEL_H
#define X_HLS_NP_CHANNEL_H

#include "hls_stream.h"
#ifndef __SYNTHESIS__
#include <string.h>
#endif
namespace hls {
enum ALG {
  LOAD_BALANCING = 1,
  ROUND_ROBIN = 2,
  TAG_SELECT = 3
};

#ifdef __SYNTHESIS__
namespace split {
template <typename T, unsigned N_OUT_PORTS, unsigned DEPTH = 2>
class load_balance {
public:
  stream<T> in __attribute__((no_ctor));
  stream<T> out[N_OUT_PORTS] __attribute__((no_ctor));
  load_balance(const char *name = nullptr) {
    #pragma HLS inline
    (void) name;
    __fpga_nport_channel(&in, &out, 1, N_OUT_PORTS, DEPTH, LOAD_BALANCING, 0);
  }
};

template <typename T, unsigned N_OUT_PORTS, unsigned ONEPORT_DEPTH = 2, unsigned NPORT_DEPTH = 0>
class round_robin {
public:
  stream<T> in __attribute__((no_ctor));
  stream<T> out[N_OUT_PORTS] __attribute__((no_ctor));
  round_robin(const char *name = nullptr) {
    #pragma HLS inline
    (void) name;
    __fpga_nport_channel(&in, &out, 1, N_OUT_PORTS, NPORT_DEPTH, ROUND_ROBIN, ONEPORT_DEPTH);
  }
};
} // end of namespace split

namespace merge {
template <typename T, unsigned N_IN_PORTS, unsigned DEPTH = 2>
class load_balance {
public:
  stream<T> in[N_IN_PORTS] __attribute__((no_ctor));
  stream<T> out __attribute__((no_ctor));
  load_balance(const char *name = nullptr) {
    #pragma HLS inline
    (void) name;
    __fpga_nport_channel(&in, &out, N_IN_PORTS, 1, DEPTH, LOAD_BALANCING, 0);
  }
};

template <typename T, unsigned N_IN_PORTS, unsigned ONEPORT_DEPTH = 2, unsigned NPORT_DEPTH = 0>
class round_robin {
public:
  stream<T> in[N_IN_PORTS] __attribute__((no_ctor));
  stream<T> out __attribute__((no_ctor));
  round_robin(const char *name = nullptr) {
    #pragma HLS inline
    (void) name;
    __fpga_nport_channel(&in, &out, N_IN_PORTS, 1, NPORT_DEPTH, ROUND_ROBIN, ONEPORT_DEPTH);
  }
};
} // end of namespace merge
#else

#ifndef USE_THREAD_NP_CHANNEL
#ifdef HLS_STREAM_THREAD_SAFE
#define USE_THREAD_NP_CHANNEL
#else
#ifdef HLS_STREAM_THREAD_SAFE_EXPERIMENTAL
#define USE_THREAD_NP_CHANNEL
#endif
#endif
#endif

template <typename T, unsigned N_OUT_PORTS, unsigned N_IN_PORTS>
class load_balancing_np : public stream_delegate<sizeof(T)> {
private:
  std::string name;
  std::deque<std::array<char, sizeof(T)> > _data;
#ifdef USE_THREAD_NP_CHANNEL
  std::mutex _mutex;
  std::condition_variable _condition_var;
#endif
protected:
  load_balancing_np() {}
  load_balancing_np(const char *n) : name(n) {}

public:
  virtual size_t size() {
#ifdef USE_THREAD_NP_CHANNEL
    std::lock_guard<std::mutex> lg(_mutex);
#endif
    return _data.size();
  }

  virtual bool read(void *elem) {
#ifdef USE_THREAD_NP_CHANNEL
    std::unique_lock<std::mutex> ul(_mutex);
    while (_data.empty()) {
      _condition_var.wait(ul);
    }
#else
    if (_data.empty()) {
      std::cout << "WARNING: n-ports channel '"
                << name
                << "' is read while empty,"
                << " which may result in RTL simulation hanging."
                << std::endl;
      return false;
    }
#endif
    
    memcpy(elem, _data.front().data(), sizeof(T));
    _data.pop_front();
    return true;
  }

  virtual bool read_nb(void *elem) {
#ifdef USE_THREAD_NP_CHANNEL
    std::lock_guard<std::mutex> lg(_mutex);
#endif
    bool is_empty = _data.empty();
    if (!is_empty) {
      memcpy(elem, _data.front().data(), sizeof(T));
      _data.pop_front();
    }
    return !is_empty;
  }

  virtual void write(const void *elem) {
#ifdef USE_THREAD_NP_CHANNEL
    std::unique_lock<std::mutex> ul(_mutex);
#endif
    std::array<char, sizeof(T)> elem_data;
    memcpy(elem_data.data(), elem, sizeof(T));
    _data.push_back(elem_data);
#ifdef USE_THREAD_NP_CHANNEL
    _condition_var.notify_one();
#endif
  }
};

namespace split {
template <typename T, unsigned N_OUT_PORTS, unsigned DEPTH = 2>
class load_balance : public load_balancing_np<T, N_OUT_PORTS, 1> { 
public:
  stream<T> in;
  stream<T> out[N_OUT_PORTS];
  load_balance() {
    in.set_delegate(this);
    for (int i = 0; i < N_OUT_PORTS; i++)
      out[i].set_delegate(this);
  }

  load_balance(const char *name) : load_balancing_np<T, N_OUT_PORTS, 1>(name) {
    in.set_delegate(this);
    for (int i = 0; i < N_OUT_PORTS; i++)
      out[i].set_delegate(this);
  }
};

template <typename T, unsigned N_OUT_PORTS, unsigned ONEPORT_DEPTH = 2, unsigned NPORT_DEPTH = 0>
class round_robin : public stream_delegate<sizeof(T)> {
private:
#ifdef USE_THREAD_NP_CHANNEL
  std::mutex _mutex;
#endif
  std::string name;
  int pos;
public:
  stream<T> in;
  stream<T> out[N_OUT_PORTS];
  round_robin() : pos(0) {
    in.set_delegate(this);
  }

  round_robin(const char *n) : name(n), pos(0) {
    in.set_delegate(this);
  }

  virtual size_t size() {
    return 0;
  }

  virtual bool read(void *elem) {
    std::cout << "WARNING: the 'in' port of n-ports channel '"
              << name
              << "' cannot be read."
              << std::endl;
    return false;
  }

  virtual bool read_nb(void *elem) {
    return read(elem);
  }

  virtual void write(const void *elem) {
#ifdef USE_THREAD_NP_CHANNEL
    std::lock_guard<std::mutex> lg(_mutex);
#endif
    out[pos].write(*(T *)elem);
    pos = (pos + 1) % N_OUT_PORTS;
  }
};
} // end of namespace split

namespace merge {

template <typename T, unsigned N_IN_PORTS, unsigned DEPTH = 2>
class load_balance : public load_balancing_np<T, 1, N_IN_PORTS> { 
public:
  stream<T> in[N_IN_PORTS];
  stream<T> out;
  load_balance() {
    out.set_delegate(this);
    for (int i = 0; i < N_IN_PORTS; i++)
      in[i].set_delegate(this);
  }

  load_balance(const char *name) : load_balancing_np<T, 1, N_IN_PORTS>(name) {
    out.set_delegate(this);
    for (int i = 0; i < N_IN_PORTS; i++)
      in[i].set_delegate(this);
  }
};

template <typename T, unsigned N_IN_PORTS, unsigned ONEPORT_DEPTH = 2, unsigned NPORT_DEPTH = 0>
class round_robin : public stream_delegate<sizeof(T)> { 
private:
#ifdef USE_THREAD_NP_CHANNEL
  std::mutex _mutex;
#endif
  std::string name;
  int pos;
public:
  stream<T> in[N_IN_PORTS];
  stream<T> out;
  round_robin() : pos(0) {
    out.set_delegate(this);
  }

  round_robin(const char *n) : name(n), pos(0) {
    out.set_delegate(this);
  }

  virtual size_t size() {
    return in[pos].size();
  }

  virtual bool read(void *elem) {
#ifdef USE_THREAD_NP_CHANNEL
    std::lock_guard<std::mutex> lg(_mutex);
#else
    if (in[pos].empty())
      return false;
#endif
    in[pos].read(*(T *)elem);
    pos = (pos + 1) % N_IN_PORTS;
    return true;
  }

  virtual bool read_nb(void *elem) {
#ifdef USE_THREAD_NP_CHANNEL
    std::lock_guard<std::mutex> lg(_mutex);
#endif
    if (in[pos].read_nb(*(T *)elem)) {
      pos = (pos + 1) % N_IN_PORTS;
      return true;
    }
    return false; 
  }  

  virtual void write(const void *elem) {
    std::cout << "WARNING: the 'out' port of n-ports channel '"
              << name
              << "' cannot be written."
              << std::endl;
  }
};
} // end of namespace merge
#endif
} // end of namespace hls
#endif
