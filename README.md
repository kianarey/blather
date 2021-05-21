# Blather Chat Server and Client
This project focuses on developing a simple chat server and client called blather. The basic usage is in two parts: 
1) Server 
    - Some user starts bl_server which manages the chat "room." The server is non-interactive and will likely only print debugging output as it runs. 
2) Client 
    - Any user who wishes to chat runs bl_client which takes input typed on the keyboard and sends it to the server. The server broadcasts the input to all other clients who can respond by typing their own input.

This project explores the following systems programming topics:

* Multiple communicating processes: clients to servers
* Communication through FIFOs
* Signal handling for graceful server shutdown
* Alarm signals for periodic behavior
* Input multiplexing with poll()
* Multiple threads in the client to handle typed input versus info from the server

#### How to Run Code:
Be sure to include all files within same directory before running code. Open up a new command line shell (e.g. Terminal), navigate to the directory where files are saved, and type/enter the following:

`$ make`

First, run a server:

`$ ./bl_server Server-Name` (e.g. './bl_server Server1')

Open a new Terminal to start a session with new client:

`$ ./bl_client Server-Name Client-Name` (e.g. './bl_client Server1 Lisa')

To have a new client join this session, repeat the preceding step by opening up a new Terminal and entering the following:

`$ ./bl_client Server-Name Client-Name` (e.g. './bl_client Server1 Tony')


