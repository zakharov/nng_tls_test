
#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/supplemental/tls/tls.h>

#include <thread>
#include <vector>

#include "helper.h"


void test(int id, const char* URL, const char* CRT) {

  printf("id: %d connecting\n", id);

  nng_socket sock{};
  nng_dialer dialer{};
  nng_tls_config* tls_cfg{nullptr};
  int rv;
  if ((rv = nng_sub_open(&sock)) != 0) {
    fatal("Failed to open sub socket", rv);
  }

  if ((rv = nng_dialer_create(&dialer, sock, URL)) != 0) {
    fatal("Failed to create dialer", rv);
  }

  if (strncmp(URL, "wss", 3) == 0) {

    /* Create TLS config */
    if ((rv = nng_tls_config_alloc(&tls_cfg, NNG_TLS_MODE_CLIENT)) != 0) {
      fatal("Failed to allocate tls config", rv);
    }

    /* Load TLS certificate */
    if ((rv = nng_tls_config_ca_file(tls_cfg, CRT)) != 0) {
      fatal("Failed to load tls certificate file", rv);
    }

    /* Apply TLS config to the listener */
    if ((rv = nng_dialer_setopt_ptr(dialer, NNG_OPT_TLS_CONFIG, tls_cfg)) != 0) {
      // We can wind up with EBUSY from the server already running.
      if (rv != NNG_EBUSY) {
        fatal("Failed to add TLS config to the dialer", rv);
      }
    }
  }

  /* Starting the listener */
  if ((rv = nng_dialer_start(dialer, NNG_FLAG_NONBLOCK)) != 0) {
    fatal("Failed to start the dialer", rv);
  }

  nng_setopt(sock, NNG_OPT_SUB_SUBSCRIBE, "", 0);

  uint8_t* buffer = new uint8_t[MAX_SIZE * 2]{};
  for (int i = 0; i < 1000; i++) {
    size_t size{};
    int rv = nng_recv(sock, buffer, &size, 0);
    if (rv != 0) {
      fatal("Failed to send message", rv);
    }
  }

  delete[] buffer;
  if (tls_cfg) {
    nng_tls_config_free(tls_cfg);
  }
  nng_close(sock);

  printf("id: %d disconnected\n", id);

}


int main(int argc, char** argv) {

  const char* CRT = "client.crt";
  char URL[255]{"wss://localhost:5555"};

  if (argc > 1) {
    strcpy(URL, argv[1]);
  }

  printf("Dial to %s, certificate: %s\n", URL, CRT);

  std::vector<std::thread> threads;
  struct timespec rem{};
  struct timespec requested_time{0, 100000000};
  for (int i = 0; i < 50; i++) {
    threads.push_back(std::thread{test, i, URL, CRT});
    nanosleep(&requested_time, &rem);
  }

  for (auto& t : threads) {
    t.join();
  }

  return 0;
}