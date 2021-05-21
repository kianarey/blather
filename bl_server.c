/*bl_server.c
  Program that is response for allowing clients to connect, become aware of one another, and communicate. It's main functionality is to broadcast messages from one client to all others and to broadcast status changes such as new clients that join or depart.
*/

#include "blather.h"
server_t server_actual;
server_t *server = &server_actual; // create a server

void handle_SIGINT(int sig_num) {
  server_shutdown(server);
  exit(0);
}

// Function run when a SIGTERM is sent to the program
void handle_SIGTERM(int sig_num) {
  server_shutdown(server);
  exit(0);
}


int main(int argc, char *argv[]){


  // Set handling function
  struct sigaction my_sa = {};
  sigemptyset(&my_sa.sa_mask); // don't block any other signals during handling; standard add option, but necessary?
  my_sa.sa_flags = SA_RESTART; // always restart system calls on signals possible

  my_sa.sa_handler = handle_SIGTERM;         // run function handle_SIGTERM
  sigaction(SIGTERM, &my_sa, NULL);          // register SIGTERM with given action

  my_sa.sa_handler = handle_SIGINT;          // run function handle_SIGINT
  sigaction(SIGINT,  &my_sa, NULL);          // register SIGINT with given action

  setvbuf(stdout, NULL, _IONBF, 0);

  printf("server_start()\n");
  server_start(server, argv[1], S_IRUSR | S_IWUSR); // Start server
  printf("server_start(): end\n");

  // Wait for clients to join
  while(1){ // loop forever awaiting client
    server_check_sources(server);
    if (server_join_ready(server)) {
      server_handle_join(server);
    }
    for (int i = 0; i < server->n_clients; i++) {
      if (server_client_ready(server,i)) {
        server_handle_client(server,i);
      }
    }
    // server_check_sources(server); // check all sources: poll()
    // int handle_join_stat = server_handle_join(server);
    // if (handle_join_stat == 0){ // 0 means ready
    //   //for each client, handle data if ready.
    //   for (int i = 0; i < server->n_clients; i++){
    //     // if the client is ready, handle data from it
    //     int client_ready = server_handle_client(server, i);
    //
    //     if(client_ready < 0){
    //       printf("Client not ready.\n");
    //       continue;
    //     }
    //   }
    // }
  }
  return 0;
}
