// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
#ifndef X_HLS_TASK_H
#define X_HLS_TASK_H
#include <functional>
#ifdef __SYNTHESIS__
#include "hls_stream.h"
namespace hls {
#ifdef EXPERIMENTAL_KPN
class KPN_region;

class IO_task {
  friend KPN_region;
  int tag;
public:
  template <class T, class... Args>
  IO_task(T &&fn, Args&&... args) {
  #pragma HLS inline
      __attribute__((xlx_task(&tag)))fn(args...);
  }
};
#endif

class task {
#ifdef EXPERIMENTAL_KPN
  friend KPN_region;
#endif
  int tag;
public:
  template <class T, class... Args>
  task(T &&fn, Args&&... args) {
  #pragma HLS inline
     __attribute__((xlx_infinite_task(&tag))) fn(args...);
  }
};

#define hls_thread_local

#ifdef EXPERIMENTAL_KPN
typedef IO_task KPN_sub_region;

class KPN_region {
public:
  template <class... Args>
  KPN_region(Args&&... args) {
  #pragma HLS inline
    add_tasks(args...);
  }
 
private:
  int tag;
  void add_tasks() {
  #pragma HLS inline
  }
  template <class... Args>
  void add_tasks(IO_task &t, Args&&... args) {
    #pragma HLS inline
    __fpga_add_task(&tag, &t.tag);
    add_tasks(args...);
  }

  template <class... Args>
  void add_tasks(task &t, Args&&... args) {
    #pragma HLS inline
    __fpga_add_task(&tag, &t.tag);
    add_tasks(args...);
  }

};

typedef KPN_region task_region;

template <typename T, int DEPTH = 0>
using KPN_stream = stream<T, DEPTH>;
#endif
}
#else
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
// hls_task requires multi-thread version of hls::stream
#ifdef X_HLS_STREAM_THREAD_UNSAFE_SIM_H
#ifndef HLS_STREAM_THREAD_SAFE_EXPERIMENTAL
#error "Don't include hls_stream.h before hls_task.h"
#endif
#else
#ifndef X_HLS_STREAM_THREAD_SAFE_SIM_H
#ifndef HLS_STREAM_THREAD_SAFE
#define HLS_STREAM_THREAD_SAFE_EXPERIMENTAL 1
#endif
#include "hls_stream.h"
#endif
#endif

namespace hls {
#ifdef EXPERIMENTAL_KPN
class KPN_region;
#endif
class task {
#ifdef EXPERIMENTAL_KPN
  friend KPN_region;
#endif
public:
  template <class T, class... Args>
  task(T fn, Args&&... args) {
    auto t_wrapper = [&] (T fn1, Args... args1) { // A wrapper to add while(1) around the real task function.
      while(1) {
        fn1(args1...);
      }
    };
    std::thread tmp(t_wrapper, fn, std::ref(args)...);
    t = std::move(tmp);
    t.detach();
  }
private:
  std::thread t;
};

#define hls_thread_local thread_local

#ifdef EXPERIMENTAL_KPN
class IO_task {
  friend KPN_region;
public:
  template <class T, class... Args>
  IO_task(T fn, Args&&... args) : wait_for_notify(false), ready(false) {
    auto t_wrapper = [&] (T fn1, Args... args1) { // A wrapper to add while(1) around the real task function.
      std::unique_lock<std::mutex> lck(m);
      wait_for_notify.store(true, std::memory_order_release);
      start_cv.wait(lck);
      ready.store(true, std::memory_order_release);
      fn1(args1...);
    };
    std::thread tmp(t_wrapper, fn, std::ref(args)...);
    t = std::move(tmp);
  }

private:
  std::thread t;
  std::condition_variable start_cv;
  std::atomic<bool> wait_for_notify;
  std::atomic<bool> ready;
  std::mutex m;
};

typedef IO_task KPN_sub_region;

class KPN_region {
public:
  template <class... Args>
  KPN_region(Args&&... args) {
    // Add tasks to region
    add_task(args...);
    for (std::thread &t : task_vec)
      t.join();
  }

private:
  void add_task() {}
  template <class... Args>
  void add_task(task &t, Args&&... args) {
    add_task(args...);
  }

  template <class... Args>
  void add_task(IO_task &t, Args&&... args) {
    while (!t.wait_for_notify.load(std::memory_order_acquire)) ;
    std::unique_lock<std::mutex> lck(t.m);
    lck.unlock();
    t.start_cv.notify_all();
    while (!t.ready.load(std::memory_order_acquire)) ;
    task_vec.push_back(std::move(t.t));
    add_task(args...);
  }

  std::vector<std::thread> task_vec;
  std::vector<std::thread> KPN_task_vec;
};

template <typename T, int DEPTH = 0>
class KPN_stream {
  stream<T, DEPTH> *p;
public:
  KPN_stream() {
    p = new stream<T, DEPTH>();
  }

  KPN_stream(const char* name) {
    p = new stream<T, DEPTH>(name);
  }

  operator stream<T, DEPTH>&() {
    return *p;
  }
};

typedef KPN_region task_region;

#endif
} 
#endif
#endif
