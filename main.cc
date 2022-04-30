#include "server_news_api.h"

// TODO: replace all cast to static_cast !!!

int main(int argc, char *argv[]) {
  unsigned short port = 9090;

  if (argc == 2)
  {
    port = atoi(argv[1]);
  }

  ServerNewsApi server;
  server.Run(port);
  return 0;
}
