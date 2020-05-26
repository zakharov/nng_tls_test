
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/supplemental/tls/tls.h>

#include "helper.h"

void pipeEvent(nng_pipe pipe, nng_pipe_ev pipe_ev, void* ptr) {
  switch (pipe_ev) {
    case NNG_PIPE_EV_ADD_POST: {
      nng_sockaddr ra{};
      auto z = sizeof(nng_sockaddr);
      nng_pipe_getopt(pipe, NNG_OPT_REMADDR, &ra, &z);
      uint8_t* addr_ip4 = reinterpret_cast<uint8_t*>(&ra.s_in.sa_addr);
      printf("Publisher: Client connected id: %x address: %d.%d.%d.%d:%d\n",
             pipe.id, addr_ip4[0], addr_ip4[1], addr_ip4[2], addr_ip4[3], ra.s_in.sa_port);
    }
      break;
    case NNG_PIPE_EV_REM_POST:
      printf("Publisher: Client disconnected id: %x\n", pipe.id);
      break;
    default:
      printf("Publisher: Unknown event!");
  }
}

int main(int argc, char** argv) {

  const char* CRT{"server.pem"};
  char URL[255]{"wss://*:5555"};

  if (argc > 1) {
    strcpy(URL, argv[1]);
  }

  printf("Bind to %s, certificate: %s\n", URL, CRT);

  nng_socket sock{};
  nng_listener listener{};
  nng_tls_config* tls_cfg{nullptr};
  int rv;
  if ((rv = nng_pub_open(&sock)) != 0) {
    fatal("Failed to open pub socket", rv);
  }

  if ((rv = nng_listener_create(&listener, sock, URL)) != 0) {
    fatal("Failed to create listener", rv);
  }

  if (strncmp(URL, "wss", 3) == 0) {

    /* Create TLS config */
    if ((rv = nng_tls_config_alloc(&tls_cfg, NNG_TLS_MODE_SERVER)) != 0) {
      fatal("Failed to allocate tls config", rv);
    }

    /* Load TLS certificate */
    if ((rv = nng_tls_config_cert_key_file(tls_cfg, CRT, nullptr)) != 0) {
      fatal("Failed to load tls certificate file", rv);
    }

    /* Apply TLS config to the listener */
    if ((rv = nng_listener_setopt_ptr(listener, NNG_OPT_TLS_CONFIG, tls_cfg)) != 0) {
      // We can wind up with EBUSY from the server already running.
      if (rv != NNG_EBUSY) {
        fatal("Failed to add TLS config to the listener", rv);
      }
    }
  }

  /* Starting the listener */
  if ((rv = nng_listener_start(listener, 0)) != 0) {
    fatal("Failed to start the listener", rv);
  }

  // nng_pipe_notify registers a callback to be executed when the
  if ((rv = nng_pipe_notify(sock, nng_pipe_ev::NNG_PIPE_EV_ADD_POST, pipeEvent, nullptr)) != 0) {
    fatal("Failed to register socket callback", rv);
  }

  if ((rv = nng_pipe_notify(sock, nng_pipe_ev::NNG_PIPE_EV_REM_POST, pipeEvent, nullptr)) != 0) {
    fatal("Failed to register socket callback", rv);
  }

  uint8_t* buffer = new uint8_t[MAX_SIZE]{};

  struct timespec rem{};
  struct timespec requested_time{0, 100000};
  while (running()) {
    int rv = nng_send(sock, buffer, MAX_SIZE, 0);
    if (rv != 0) {
      fatal("Failed to send message", rv);
    }
    nanosleep(&requested_time, &rem);
  }

  delete[] buffer;
  if (tls_cfg) {
    nng_tls_config_free(tls_cfg);
  }
  nng_close(sock);

  printf("bye...\n");

  return 0;
}