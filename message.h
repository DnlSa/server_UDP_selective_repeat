 

char *ackpkt="ACK"; //stringa di ack
char *synack = "SYNACK"; // stringa si synack
char *syn = "SYN"; // messaggio di syn
char *fin="FIN"; //stringa di fin
char *finack = "FINACK"; // stringa si finack
char *ready = "READY"; // messaggio di inizio comunicazione per indicare che il server e pronto a ricevere


// lista comandi disponibili
char *list = "LIST"; // lista tutti i file dentro la cartella
char *getFile = "GET"; // richiede un fileS
char *put  = "PUT" ; // manda un file
char *closed  = "CLOSE" ; // chiudo la connessione
char *found = "Filename -> FOUND" ; // stringa che indica che ho trovato il file che cerco
char *notfound = "Filename -> NOT_FOUND" ; // stringa che indica che ho trovato il file che cerco
char *go_transfert = "go";

char *str0 = "thre server is comopletly busy";
char *str1 = "error_recv";
char *str2 = "error_send";
char *str3 = "command not found";
char *str4 = "file Not Found";
char *str5 = "please insert filename";
char *str6 = "transfert COMPLETED ";
char *str7 = "transfert ERROR ";
char *str8 = "abort";
char *str9 = "SIGQUIT";
char *str10= "file-exist";
char *str11 = "ok";

char *legend = "\nThe available commands are:\nlist of files available on the server -> LIST\nDownload file from the server -> GET\nUpload file in the server -> PUT\nClose the connection -> CLOSE\n";
// stringhe usate dal client
char *cli_str0 = "Session stopped by client \n";
char *cli_str1 = "Timeout Session\n";
char *cli_str2 = "Invalid input retry\n";


