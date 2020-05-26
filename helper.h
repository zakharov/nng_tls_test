//
// Created by alexey on 26-05-20.
//

#ifndef NNG_TLS_PUB_HELPER_H
#define NNG_TLS_PUB_HELPER_H

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <atomic>

constexpr size_t MAX_SIZE{16384};

bool sig_initialized{};
std::atomic<bool> sig_terminated{};

void signal_callback_handler(int signum) { // ctrl - c handler
  sig_terminated.store(true);
}

bool running() {
  if (!sig_initialized) {
    signal(SIGINT, signal_callback_handler);
    signal(SIGHUP, signal_callback_handler);
    signal(SIGTERM, signal_callback_handler);
    signal(SIGABRT, signal_callback_handler);
    sig_initialized = true;
  }
  return !sig_terminated;
}

void fatal(const char* func, int rv) {
  fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
  exit(1);
}

#endif //NNG_TLS_PUB_HELPER_H
