/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see
             // LICENSE.md for rights.
// Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc

#include "thread/Thread.hpp"
#include "chrono.hpp"

#include <cstdio>
#include <errno.h>
using namespace thread;

Thread::Thread(const Construct &options) {
  init(options.stack_size(), options.detach_state() == DetachState::detached);
}

Thread::~Thread() {
  // what if thread is still running?
  if (is_running()) {
    // must be stopped or assert
  }

  if (!is_id_error()) {
    pthread_attr_destroy(&m_pthread_attr);
  }
}

bool Thread::is_valid() const { return !is_id_pending() && !is_id_error(); }

int Thread::init(int stack_size, bool detached) {
  API_RETURN_VALUE_IF_ERROR(-1);
  set_id_error();

  API_SYSTEM_CALL(
    "",
    pthread_attr_init(&m_pthread_attr));

  if (status().is_error()) {
    return -1;
  }

  API_SYSTEM_CALL(
    "",
    pthread_attr_setstacksize(&m_pthread_attr, stack_size));

  if (status().is_error()) {
    return -1;
  }

  if (detached == true) {

    API_SYSTEM_CALL(
      "",
      pthread_attr_setdetachstate(&m_pthread_attr, PTHREAD_CREATE_DETACHED));

    if (status().is_error()) {
      return -1;
    }
  } else {

    API_SYSTEM_CALL(
      "",
      pthread_attr_setdetachstate(&m_pthread_attr, PTHREAD_CREATE_JOINABLE));

    if (status().is_error()) {
      return -1;
    }
  }

  set_id_pending();
  return 0;
}

int Thread::get_stacksize() const {
  API_RETURN_VALUE_IF_ERROR(-1);
  u32 stacksize = 0;
  API_SYSTEM_CALL(
    "",
    pthread_attr_getstacksize(&m_pthread_attr, (size_t *)&stacksize));
  return stacksize;
}

Thread::DetachState Thread::get_detachstate() const {
  API_RETURN_VALUE_IF_ERROR(DetachState::detached);
  int value = 0;
  API_SYSTEM_CALL(
    "",
    pthread_attr_getdetachstate(&m_pthread_attr, &value));
  return static_cast<DetachState>(value);
}

Thread &Thread::set_detachstate(DetachState value) {
  API_RETURN_VALUE_IF_ERROR(*this);

  if (is_running()) {
    API_SYSTEM_CALL("", -1);
    return *this;
  }

  API_SYSTEM_CALL(
    "",
    pthread_attr_setdetachstate(&m_pthread_attr, static_cast<int>(value)));

  return *this;
}

Thread &Thread::set_priority(int prio, Sched::Policy policy) {
  API_RETURN_VALUE_IF_ERROR(*this);
  if (is_valid()) {
    struct sched_param param = {0};
    param.sched_priority = prio;
    API_SYSTEM_CALL(
      "",
      pthread_setschedparam(m_id, static_cast<int>(policy), &param));
  } else {
    API_SYSTEM_CALL("", -1);
  }
  return *this;
}

int Thread::get_priority() const {
  API_RETURN_VALUE_IF_ERROR(-1);

  if (is_valid()) {
    struct sched_param param;
    int policy;
    API_SYSTEM_CALL(
      "",
      pthread_getschedparam(m_id, &policy, &param));

    if (status().is_error()) {
      return -1;
    }

    return param.sched_priority;
  }

  API_SYSTEM_CALL("", -1);
  return -1;
}

Thread &Thread::cancel() {
  API_RETURN_VALUE_IF_ERROR(*this);
  API_SYSTEM_CALL("", pthread_cancel(id()));
  return *this;
}

Thread &Thread::set_cancel_type(CancelType cancel_type) {
  API_RETURN_VALUE_IF_ERROR(*this);
  int old = 0;
  API_SYSTEM_CALL(
    "",
    pthread_setcanceltype(static_cast<int>(cancel_type), &old));
  return *this;
}

Thread &Thread::set_cancel_state(CancelState cancel_state) {
  API_RETURN_VALUE_IF_ERROR(*this);
  int old = 0;
  API_SYSTEM_CALL(
    "",
    pthread_setcancelstate(static_cast<int>(cancel_state), &old));
  return *this;
}

int Thread::get_policy() const {
  API_RETURN_VALUE_IF_ERROR(-1);
  struct sched_param param;
  int policy;
  if (is_valid()) {

    API_SYSTEM_CALL(
      "",
      pthread_getschedparam(m_id, &policy, &param));

    if (status().is_error()) {
      return -1;
    }
  } else {
    API_SYSTEM_CALL("", -1);
    return -1;
  }
  return policy;
}

Thread &Thread::create(const Create &options) {
  API_RETURN_VALUE_IF_ERROR(*this);
  reset();

  if (status().is_error()) {
    return *this;
  }

  if (!is_id_pending()) {
    API_SYSTEM_CALL("", -1);
    return *this;
  }

  if (
    API_SYSTEM_CALL(
      "",
      pthread_attr_setschedpolicy(
        &m_pthread_attr,
        static_cast<int>(options.policy())))
    < 0) {
    return *this;
  }

  struct sched_param param = {0};
  param.sched_priority = options.priority();
  if (
    API_SYSTEM_CALL(
      "",
      pthread_attr_setschedparam(&m_pthread_attr, &param))
    < 0) {
    return *this;
  }

  // First create the thread
  API_SYSTEM_CALL(
    "",
    pthread_create(
      &m_id,
      &m_pthread_attr,
      options.function(),
      options.argument()));

  return *this;
}

bool Thread::is_running() {
  // check to see if the thread is running
  if (is_id_pending() || is_id_error()) {
    return false;
  }
  if (pthread_kill(m_id, 0) == 0) {
    return true;
  }

  reset();
  return false;
}

Thread &Thread::wait(void **ret, chrono::MicroTime interval) {

  void *dummy;

  if (is_valid()) {

    // if thread is joinable, then join it
    if (is_joinable()) {
      if (ret != 0) {
        join(ret);
      } else {
        join(&dummy);
      }
    } else {
      // just keep sampling until the thread completes
      while (is_running()) {
        chrono::wait(interval);
      }
    }
  }
  return *this;
}

Thread &Thread::reset() {
  API_RETURN_VALUE_IF_ERROR(*this);

  if (is_id_pending()) {
    return *this;
  }

  if (is_valid() && (pthread_kill(m_id, 0) < 0)) {
    bool detached = !is_joinable();
    u32 stacksize = get_stacksize();

    if (
      API_SYSTEM_CALL(
        "",
        pthread_attr_destroy(&m_pthread_attr))
      < 0) {
      return *this;
    }

    init(stacksize, detached);
    return *this;
  }

  API_SYSTEM_CALL("", -1);
  return *this;
}

Thread &Thread::join(void **value) {
  API_RETURN_VALUE_IF_ERROR(*this);

  void *tmp_ptr;
  void **ptr = value == nullptr ? &tmp_ptr : value;

  const int local_result = API_SYSTEM_CALL("", pthread_join(id(), ptr));
  if (local_result == 0) {
    // resets the thread that just completed
    is_running();
  }
  return *this;
}

bool Thread::is_joinable() const {
  return get_detachstate() == DetachState::joinable;
}
