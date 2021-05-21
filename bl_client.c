/*bl_client.c
  Main purpose is to help keep track data for the server. It allows users to type into the terminal to send text to the server and receive text back from the server which is printed to the screen.
*/

#include "blather.h"
// All global okay?
simpio_t simpio_actual={};
simpio_t *simpio = &simpio_actual;
client_t client_actual={};
client_t *client = &client_actual;
//server_t server_actual={};
//server_t *server = &server_actual;
pthread_t user_thread;
pthread_t server_thread;

// Worker thread function to manage user input
void *user_worker(void *arg){

  while(!simpio->end_of_input){
    simpio_reset(simpio);
    iprintf(simpio, "");// print prompt
    while(!simpio->line_ready && !simpio->end_of_input){ // read until line is complete
      simpio_get_char(simpio);
    }
    if(simpio->line_ready){ // if ready create a mesg_t with the line and write it to the to-server FIFO
      mesg_t message = {}; // empty mesg_t struct
      //snprintf(message.body, MAXLINE, simpio->buf);
      message.kind = BL_MESG;
      strncpy(message.name, client->name, MAXNAME);
      strncpy(message.body, simpio->buf, MAXLINE);
      //iprintf(simpio, "name: '%s' message: '%s'\n",message.name, message.body);
      write(client->to_server_fd, &message, sizeof(mesg_t));

    }
  }
  // simpio->end_of_input, so do the following
  mesg_t message = {};
  message.kind = BL_DEPARTED;
  strncpy(message.name, client->name, MAXNAME);
  //iprintf(simpio, "Message Kind: '%d'\n", message.kind);
  //iprintf(simpio, "Departing client name: '%s'\n", message.name);
  int val = write(client->to_server_fd, &message, sizeof(mesg_t));
  if(val != sizeof(mesg_t)){
    printf("Did not write departing message to server\n");
  }
  pthread_cancel(server_thread); // kill the server thread
  return NULL;
}

// Server thread function. Listen to info from client
void *server_worker(void *arg){

  //int kind = message.kind;
  while(1){
    mesg_t message = {}; // empty mesg_t struct
    read(client->to_client_fd, &message, sizeof(mesg_t)); // read a mesg_t from to-client FIFO
    //printf("server_worker: nread = %d\n", nread);
    //printf("%s\n", message.name); // debugging purposes
    //iprintf(simpio, "Reading from client name: '%s'\n",message.name);
    //iprintf(simpio, "Message kind: '%d'\n",message.kind);
    if(message.kind == BL_MESG){
      //iprintf(simpio, " %s\n", message.name);
      iprintf(simpio, "[%s] : %s\n", message.name, message.body); //print appropriate response to terminal with simpio
    }
    if(message.kind == BL_JOINED){
      iprintf(simpio, "-- %s JOINED --\n", message.name);
    }
    if(message.kind == BL_DEPARTED){
      iprintf(simpio, "-- %s DEPARTED --\n", message.name);
    }
    if(message.kind == BL_SHUTDOWN){
      iprintf(simpio, "!!! server is shutting down !!!\n");
    }
  }
  pthread_cancel(user_thread); // kill the server thread
  return NULL;
}

int main(int argc, char *argv[]){
  // Read name of server and name of client
  if(argc < 2){
    printf("usage: %s <name>\n",argv[0]);
    exit(1);
  }

  if(argc >= 3){

    // Creates two fifos on start up
    remove(client->to_client_fname);
    remove(client->to_server_fname);
    snprintf(client->to_client_fname, MAXPATH, "%d.client.fifo", getpid());
    snprintf(client->to_server_fname, MAXPATH, "%d.server.fifo", getpid());


    // A client writes a join_t request to the server which includes the names of its to-client and to-server FIFOs

    // Create a join_t struct
    join_t join_actual={};
    join_t *join = &join_actual;
    snprintf(join->name, MAXPATH, "%s", argv[2]);
    snprintf(join->to_client_fname, MAXPATH, "%s",client->to_client_fname);
    snprintf(join->to_server_fname, MAXPATH, "%s", client->to_server_fname);

    char server_name[MAXPATH];
    snprintf(server_name, MAXPATH,"%s.fifo", argv[1]);
    int join_fd = open(server_name, O_RDWR); // open the server
    //printf("join_fd: %d\n", join_fd);
    //printf("server->join_fd: %d\n", server->join_fd);

    mkfifo(client->to_client_fname, S_IRUSR | S_IWUSR); // create to-client fifo
    mkfifo(client->to_server_fname, S_IRUSR | S_IWUSR); // create to-server fifo
    strncpy(client->name, join->name, MAXPATH);
    client->to_client_fd = open(join->to_client_fname, O_RDWR); // open them
    client->to_server_fd = open(join->to_server_fname, O_RDWR);

    // write the join_t request to the server
    write(join_fd, join, sizeof(join_t)); // not waking up the server?!

    //printf("write() returns: %d\n", val);
    //printf("sizeof(join_t): %ld\n", sizeof(join_t));

    char prompt[MAXNAME];
    snprintf(prompt, MAXNAME, "%s>>", argv[2]);
    simpio_set_prompt(simpio, prompt);// set the prompt
    simpio_reset(simpio);// initialize io
    simpio_noncanonical_terminal_mode();// set the terminal into a compatible mode
    // User Thread
    pthread_create(&user_thread, NULL, user_worker, NULL); // start user thread to read input
  // Server Thread
    pthread_create(&server_thread, NULL, server_worker, NULL); // start server thread
    pthread_join(user_thread, NULL);
    pthread_join(server_thread, NULL);

    simpio_reset_terminal_mode();
    printf("\n");// newline just to make returning to the terminal prettier
    return 0;
  }
}
