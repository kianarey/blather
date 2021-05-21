/*server_funcs.c
  To facilitate operations of bl_server's main program, this file provides service routines that mainly manipulate server_t structures. Each of these has a purpose to serve in the ultimate goal of the server.
*/
#include "blather.h"

client_t *server_get_client(server_t *server, int idx)
/*
  Gets a pointer to the client_t struct at the given index. If the index is beyond n_clients, the behavior of the function is unspecified and may cause a program to crash.
*/
{
  //if (idx > server->n_clients){
    //perror("Error: Index is beyond number of clients. Exiting.");
    //exit(1);
  //}
  return &server->client[idx]; // return the address TA said
}

void server_start(server_t *server, char *server_name, int perms)
/*
  Initializes and starts the server with the given name. A join fifo called "server_name.fifo" should be created. Removes any exisiting file of that name prior to creation. Opens the FIFO and stores its file descriptor in join_fd._

  ADVANCED: create the log file "serve_name.log" and write the initial empty who_t contents to its beginning. Ensure that the log_fd is position for appending to the end of the file. Create the POSIX semaphore "/server_name.sem" and initialize it to 1 to control access to the who_t portion of the log.
*/
{
  // Required section
  // Initializes
  server->n_clients = 0; // zero clients to start with
  server->join_ready = 0;
  // Start the server with the given name
  unlink(server->server_name);
  snprintf(server->server_name, MAXPATH, "%s.fifo", server_name);// fifo named after server_name

  mkfifo(server->server_name, perms);
  // Opens FIFO and stores its fd in join_fd._
  int fd = open(server->server_name, O_RDWR);// open FIFO read/write to avoid blocking
  //printf("server fd on start: %d\n", fd);
  server->join_fd = fd; // stores fd in join_fd
}

void server_shutdown(server_t *server)
/*
  Shut down the server. Close the join FIFO and unlink (remove) it so that no further clients can join. Send a BL_SHUTDOWN message to all clients and proceed to remove all clients in any order.

  ADVANCED: Close the log file. Close the log semaphore and unlink it.
*/
{
  close(server->join_fd);
  unlink(server->server_name);

  for(int i = 0; i < server->n_clients; i++){
    client_t *client = server_get_client(server, i);
    mesg_t message={};
    message.kind = BL_SHUTDOWN;
    int val = write(client->to_client_fd, &message, sizeof(mesg_t));

    if (val != sizeof(mesg_t)){
      printf("Error: write() does not return sizeof(mesg_t)\n");
    }

    //remove client by client
    close(client->to_client_fd);
    close(client->to_server_fd);
    unlink(client->to_client_fname);
    unlink(client->to_server_fname);
  }
}

int server_add_client(server_t *server, join_t *join)
/*
  Adds a client to the server according to the parameter join which should have fields such as name filled in. The client data is copied into the client[] array and file descriptors are opened for its to-server and to-client FIFOs. Initializes the data_ready field for the client to 0. Returns 0 on success and non-zero if the server as no space for clients (n_clients == MAXCLIENTS).
*/
{
  if(server->n_clients > MAXCLIENTS){
    printf("Error: n_clients beyond MAXCLIENTS\n");
    return -1;
  }
  printf("server_add_client: Entered\n");
  server->n_clients++;

  client_t *client = server_get_client(server, server->n_clients-1);

  strncpy(client->name, join->name, MAXPATH);
  strncpy(client->to_client_fname, join->to_client_fname, MAXPATH);
  strncpy(client->to_server_fname, join->to_server_fname, MAXPATH);
  client->to_client_fd = open(join->to_client_fname, O_RDWR);
  client->to_server_fd = open(join->to_server_fname, O_RDONLY);


  printf("%s\n", client->to_client_fname);
  printf("%s\n", client->to_server_fname);

  printf("Added client is: %s\n", client->name);
  printf("server_add_client: Exited\n");
  return 0;
  // // ** how do we know which element in the client struct array to store this info? Do I have increment n_clients?
  // client_t client_actual={}; // TA said to create a client_t
  // client_t *client = &client_actual;
  //
  // if (server->n_clients < MAXCLIENTS){
  //   // ** need to use strncpy!
  //
  //   // Name of client
  //   strncpy(client->name, join->name, MAXPATH);
  //   // to_client_fname
  //   strncpy(client->to_client_fname, join->to_client_fname, MAXPATH);
  //   // to_server_fname
  //   strncpy(client->to_server_fname, join->to_server_fname, MAXPATH);
  //
  //
  //   int client_fd = open(client->to_client_fname, O_RDWR);
  //
  //   int server_fd = open(client->to_server_fname, O_RDWR);
  //
  //   // store file descriptors
  //   server->client->to_client_fd = client_fd;
  //   server->client->to_server_fd = server_fd;
  //
  //   // Initializes data_ready field for the client to 0
  //   server->client->data_ready = 0;
  //
  //   server->n_clients+= 1; // increment number of clients
  //   return 0; // return 0; meaning success
  // }
  // else{ // exceeded MAXCLIENTS, abort
  //   return -1; // n_clients == MAXCLIENTS; return -1
  // }

}

int server_remove_client(server_t *server, int idx)
/*
  Remove the given client likely due to its having departed or disconnected. Close fifos associated with the client and remove them. Shift the remaining clients to lower indices of the client[] preserving their order in the array; decreases n_clients.
*/
{
  printf("server_remove_client: Entered\n");
  // Use server_get_client for readability
  client_t *client = server_get_client(server, idx); // get client using this call
  printf("There are originally this many clients: %d\n", server->n_clients);
  printf("closing client: %s which is at idx: %d\n", client->name, idx);

  if(idx <= server->n_clients){ // if idx exists
    close(client->to_client_fd);
    close(client->to_server_fd);
    unlink(client->name); // remove client name
    unlink(client->to_client_fname);
    unlink(client->to_server_fname);

    printf("\nList of all clients\n");
    for (int i = 0; i < server->n_clients; i++){
      printf("idx: %d: %s\n", i, server->client[i].name);
    }
    printf("\n");

    //shift remaining clients to lower indices of the client[] preserving their order in the array
    printf("shifting clients\n");
    //int shift_start = server->n_clients - idx; // get starting shift point

    //printf("starting at idx: %d\n", shift_start);
    int i = idx;
    while(i < server->n_clients - 1){
      server->client[i] = server->client[i+1];
      i++;
    }

    // for (int i = shift_start; i < server->n_clients; i++){
    //   printf("client at %d is %s\n", i, server->client[i].name);
    //   server->client[i-1] = server->client[i]; // shift to remaining clients to lower indices
    //
    // }
    //decrememnt n_clients
    server->n_clients -= 1;
    printf("After shifting, there are this many clients: %d\n", server->n_clients);

    printf("\nList of all clients\n");
    for (int i = 0; i < server->n_clients; i++){
      printf("idx: %d: %s\n", i, server->client[i].name);
    }
    printf("\n");

  }
  printf("server_remove_client: Exited\n");
  return 0; // return success
}

int server_broadcast(server_t *server, mesg_t *mesg)
/*
  Send the given message to all clients connected to the server by writing it to the file descriptors associated with them.

  ADVANCED: Log the broadcast message unless it is a PING which should not be written to the log.
*/
{
  printf("server_broadcast: Entered\n");
  //int num_clients = server->n_clients;
  printf("Broadcaster name: %s\n", mesg->name);
  printf("Broadcasting body: %s\n", mesg->body);
  printf("Broadcast message kind: %d\n", mesg->kind);
  // ** to all n_clients
  for (int i = 0; i < server->n_clients; i++){
    client_t *client = server_get_client(server, i);
    write(client->to_client_fd, mesg, sizeof(mesg_t));
  }
  printf("server_broadcast: Exited\n");
  return 0; //return success

}

void server_check_sources(server_t *server)
/*
  Checks all sources of data for the server to determine if any are ready for reading. Sets the servers join_ready flag and the data_ready flags of each of client if data is ready for them. Makes use of the poll() system call to efficiently determine which sources are ready.
*/
{
  //client_t *client = server_get_client(server);
  //int size = server->n_clients + 1
  struct pollfd pfds[server->n_clients + 1]; // array structures for poll, 1 per fd to be monitored, plus one for the join_fd
  //printf("server->n_clients + 1: %d\n", server->n_clients + 1);
  //printf("Initializing poll array\n");
  //Initializes poll array
  int i = 0;
  while (i < server->n_clients){
    //printf("i: %d\n", i);
    client_t *client = server_get_client(server, i);
    client->data_ready = 0;
    pfds[i].fd = client->to_server_fd;
    pfds[i].events = POLLIN;
    pfds[i].revents = 0;// initialize revents field to 0
    i++;
  }
  //printf("i: %d\n", i);
  //poll for server, last element
  pfds[i].fd = server->join_fd;
  pfds[i].events = POLLIN;// check for ready to read() without blocking
  pfds[i].revents = 0;// initialize revents field to 0
  //printf("server->join_fd: %d\n", server->join_fd);

  int ready = poll(pfds, server->n_clients + 1, -1);// block until OS notifies input is ready
  //printf("poll() returned: %d\n", ready);
  if(ready < 0){
    perror("poll() failed");
  }
  //printf("Polling now\n");

  int j = 0;
  while(j < server->n_clients){
    client_t *client = server_get_client(server, j);
    //printf("j: %d\n", j);
    if(pfds[j].revents & POLLIN){// had input as indicated by 'revents'
      client->data_ready = 1; //indicating ready
      //printf("data_ready = %d", client->data_ready);
    }
    j++;
  }

  //printf("j: %d\n", j);
  if(pfds[j].revents & POLLIN ){// had input as indicated by 'revents'
    server->join_ready = 1; //indicating ready
    //printf("join_ready = %d", server->join_ready);
  }
}

int server_join_ready(server_t *server)
/*
  Return the join_ready flag from the server which indicates whether a call to server_handle_join() is safe.
*/
{
  int flag = server->join_ready;
  //printf("server_join_ready flag is: %d\n", flag);
  return flag;
}

int server_handle_join(server_t *server)
/*
  Call this function only if server_join_ready() returns true. Read a join request and add the new client to the server. After finishing, set the servers join_ready flag to 0.
*/
{
  printf("server_handle_join: Entered\n");
  join_t join_actual={};
  join_t *join = &join_actual;
  mesg_t message_actual = {};
  mesg_t *message = &message_actual;

  read(server->join_fd, join, sizeof(join_t)); // read join_t request
  //printf("nread: %d\n", nread);
  //printf("size of join_t: %ld", sizeof(join_t));

  printf("Adding client\n");

  server_add_client(server,join);
    //message->name = join->name;
  strncpy(message->name, join->name, MAXNAME);

  message->kind = BL_JOINED;
  server_broadcast(server, message);
  server->join_ready = 0;
  printf("server_handle_join: Exited\n");
  return 0;
  // int result = server_join_ready(server);
  //
  // printf("result %d\n", result);
  //
  // if(result == 1){ // 1 means true
  //   // ** Read a join request..
  //   read(server->join_fd, join, sizeof(join_t));
  //
  //   // adds client with server_add_client()
  //   printf("Adding client\n");
  //   int val = server_add_client(server, join);
  //   printf("val is %d\n", val); // 0 means success
  //   if (val == 0){ // 0 indicates client joined successfully
  //     message->kind = BL_JOINED;
  //     int status = server_broadcast(server, message);
  //     if(status == 0){ // 0 means success
  //       server->join_ready = 0; // set join_ready flag to 0
  //     }
  //   }
  //   return 0; // meaning success
  //   printf("server_handle_join: Success\n");
  // }
  // return -1; // meaning failure
  // printf("server_handle_join: Failed\n");
}

int server_client_ready(server_t *server, int idx)
/*
  Return the data_ready field of the given client which indicates whether the client has data ready to be read from it.
*/
{
  client_t *client = server_get_client(server, idx);
  //printf("client->data_ready: %d\n", client->data_ready);
  return client->data_ready;
}

int server_handle_client(server_t *server, int idx)
/*
  Process a message from the specified client. This function should only be called if server_client_ready() returns true. Read a message from to_server_fd and analyze the message kind. Departure and Message types should be broadcast to all other clients. Ping responses should only change the last_contact_time below. Behavior for other message types is not specified. Clear the client's data_ready flag so it has value 0.

  ADVANCED: Update the last_contact_time of the client to the current server time_sec.
*/
{
  printf("server_handle_client: Entered\n");
  client_t *client = server_get_client(server, idx);

  mesg_t message_actual = {};
  //mesg_t *message = &message_actual;
  printf("Handling client\n");

  if(server_client_ready(server, idx)){
    int val = read(client->to_server_fd, &message_actual, sizeof(mesg_t));
    if (val != sizeof(mesg_t)){
      printf("Error: read does not return sizeof(mesg_t)\n");
    }
    //printf("server_handle_client nread: %d\n", nread);
    //printf("size of mesg_t: %ld\n", sizeof(mesg_t));
    printf("Name of client %s\n", message_actual.name);
    printf("Message kind is: %d\n", message_actual.kind);

    if(message_actual.kind == BL_DEPARTED){ // if message kind is BL_DEPARTED
      printf("Removing this client: %s\n", message_actual.name);
      server_remove_client(server, idx); // remove this client
    }

    server_broadcast(server, &message_actual);
    client->data_ready = 0;

  }
  printf("server_handle_client: Exited\n");
  return 0;


  // int result = server_client_ready(server, idx);
  // if(result == 1){ // means true, data is ready
  //   // Read a message from to_server_fd
  //   read(client->to_server_fd, message, sizeof(mesg_t));
  //   // Part 6.3 Process message properly and broadcasts if needed
  //   int status = server_broadcast(server, message);
  //   if (status == 0){
  //     //clear the client's data_ready flag to 0
  //     client->data_ready = 0;
  //     printf("server_handle_client: Success\n");
  //     return 0; //return success
  //
  //   }
  // }
  // printf("server_handle_client: Failed\n");
  // return -1;

}

void server_tick(server_t *server)
/*
  ADVANCED: Increment the time for the server
*/
{

}

void server_ping_clients(server_t *server)
/*
  ADVANCED: Ping all clients in the server by broadcasting a ping.
*/
{

}

void server_remove_disconnected(server_t *server, int disconnect_secs)
/*
  ADVANCED: Check all clients to see if they have contacted the server recently. Any client with a last_contact_time field equal to or greater than the parameter disconnect_secs should be removed. Broadcast that the client was disconnected to remaining clients. Process clients from lowest to highest and take care of loop indexing as clients may be removed during the loop necessitating index adjustments.
*/
{

}

void server_write_who(server_t *server)
/*
  ADVANCED: Write the current set of clients logged into the server to the BEGINNING the log_fd. Ensure that the write is protected by locking the semaphore associated with the log file. Since it may take some time to complete this operation (acquire semaphore then write) it should likely be done in its own thread to prevent the main server operations from stalling.

  For threaded I/O, consider using the pwrite() function to write to a specific location in an open file descriptor which will not alter the position of log_fd so that appends continue to write to the end of the file.
*/
{

}

void server_log_message(server_t *server, mesg_t *mesg)
/*
  ADVANCED: Write the given message to the end of log file associated with the server.
*/
{

}
