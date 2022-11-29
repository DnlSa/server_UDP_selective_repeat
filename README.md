# server_UDP_selective_repeat
Server UDP with Selective repeite implementation . 

Before the used this project :
1) create directory with name "serverFiles" in same directory where present the server part than project;
2) create directory with name " clientFiles" in same directory where present the client part than project;

Ps: the libraries are shared between client and server

Installation 
1)  open terminal and invoke "make";
2) open an other terminal in the project folder and lounch "./server"
3) open an other terminal in the project folder and lounch "./client <ip-address server >"

If you want run this program using loopback interface lunch 
1) first terminal  -> "./server"
2) second terminal -> "./client 127.0.0.1" -> IP loppback interface
