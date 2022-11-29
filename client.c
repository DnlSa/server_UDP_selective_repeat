#include "basic.h"
#include "message.h"

#define fflush(stdin) while(getchar()!='\n')


/////////////////////


void signal_handler (int sgnumber);
char *upper( char *msg );
int send_pkt(char *msg , int child_sock);
char *recv_pkt(char *msg ,  int sockfd , size_t size);
char *writer(char *string,int sockfd ,int opt);
int server_open_connection( int sockfd );
int server_close_connection( int sockfd );
int create_socket(char *ip, int port);
int compare_filename( char *filename);



int sockfd;
char *string ;
char *buffercv;
struct sockaddr_in  sad ; // variabile struttura della socket
socklen_t len = sizeof(sad);
int rt; // contatore dei tentativi di connessione se falliscono tutti allora chiudo il client
int sockfd;
int getflag;
int putflag;
 char *pathname;
///////////////////////////////////////////////////////////

void signal_handler (int sgnumber){
	int byte_w;
     int n_r;
    if(sgnumber == 2 && getflag == 1){
        printf("%s", cli_str0);
        if (remove(pathname) == 0) // cancella il file
			printf("The file is deleted successfully .\n");
        int q=MAX_ERR;
        while (q){ // pacchwetto speciale di uscita
            struct ackPacket ack;
            ack.seqNum = -10; // mando un ack di fin per dire al server che ho ricevuto correttamente  l ultimo pacchetto
            ack.size = -10;
            ack.write_byte = -10;
            if(correct_send()){ // porzione di codice che invia ack di riscontro
                n_r =sendto(sockfd, &ack, sizeof(ack), 0,(struct sockaddr *)&sad, sizeof(sad));
                if(n_r<0){perror("erroro to sendto in waitack");}
            }
            q--;
        }
    }else if(sgnumber == 2 && putflag == 1){
        printf("%s", cli_str0);
        int q=MAX_ERR;
      while (q){
		 struct Packet packet; // pacchetto speciale che indica che si e verificato un errore
         packet.seqNum = -10; // inserisce il numero di seguenza'
         packet.win = 0; // inserisco il numero dei pacchetti attesi
         packet.byte_send = 0; // prende il byte
         packet.size = 0; // grandezza del payload letto (dovrebbe essere PKT_SIZE)
         packet.sent = 0; // dichiaro i booleani in 0 indicando che non ho ancora inviato
         packet.ack = 0; // anche l ack perche logicamente non e stato ancora riscontrato
         if(correct_send()){ // porzione di codice che invia ack di riscontro
			n_r = sendto(sockfd, &packet, sizeof(packet),0,(struct sockaddr *)&sad, sizeof(sad)); // mando il pacchetto;
         if(n_r<0){
              perror("erroro to sendto");
         }
		   }q--;
      }
    }else if(sgnumber == 3){
        printf("%s", cli_str0);
        send_pkt(str9, sockfd); // manda il messaggio di chiusura
    }else
        printf("%s",cli_str1);

     free(string); // rilascio la stringa dato che ha una malloc implicita nella scanf
     free(buffercv); // rilascio il bufferino visto che ha una malloc
     free(pathname);
     close(sockfd); //chiusura del descrittore
     sleep(1);
     exit(0); // terminazione del processo
}

void signal_handler_sigsegv(int signum){ // signal handler per la termianzione del processo SERVER
    printf("receive signal SIGSEGV verify if not corrupt ttransfert file %d\n",signum);
}


char *upper( char *msg ){
    size_t i,len = strlen(msg);
    char *baseid = msg; // do a baseid l indirizzo base di msg
    for(i=0; i<len ; i++){
         (*baseid) = toupper((*baseid));
         baseid++;
    }
    return msg;
}

int compare_filename( char *filename){ // funzione che lista i file contenuti in una directory specifica del server
    int ret;
    DIR *dir;
    struct dirent *dp;
    if((dir = opendir("clientFiles"))== NULL){ // ci viene restituito uno stream che punta alle entry della directory
        perror("Error, read field in the server ");
        exit(EXIT_FAILURE);
    }
	while ((dp = readdir(dir)) != NULL) { // cicla finche non arriva ala fine delle entry
		if(dp->d_type == DT_REG) {
            if(strcmp(filename , dp->d_name)==0){ // confronta la stringa
                closedir(dir);
                return 1; // se ho il matching
            }else{continue;}
        }
    }
    //chiusura della directory dir
    closedir(dir); // chiudo lo stream che mi punta la directory
    return 0 ;
}



//ritorna un intero se lo ha correttamente inviato o no
int send_pkt(char *msg , int sockfd){
        int ret = sendto(sockfd, msg, strlen(msg), 0,(struct sockaddr *)&sad, sizeof(sad)); // da capire ste struture sad
        if(ret <0){
            perror("send messagge filed\n");
            return -1 ;
        }
return 0;
}

// funzione che gli passi un bufferino vuoto e la socket su cui leggere
// la funzione legge la socket e ritorna lla stringa letta
// parametri bufferino  , descrittore socket , grandezza dell buffer passato
char *recv_pkt(char *msg ,  int sockfd , size_t size){ // passo come input il descrittore della socket
    	socklen_t len = sizeof(sad);
        int ret;
        memset(msg , 0 , size); // pulizia area di memoria
        ret = recvfrom(sockfd, msg, size, 0, (struct sockaddr *)&sad, &len); // l ultimo parametro cambia proprio per definizione della funzione questa prende un indirizzo di memoria la send no
        if (ret  < 0 ) { // vado in recivefrom (e in attesa di un pacchetto di syn )
        perror("errore in recvfrom");
        strcpy(msg , str1); // scrivo in msg il messaggio di errore
       }
 return msg; // ritorna la striga
}



// e superfluo dichiarare e far tornare la stringa che e definita globalmente però casomai avessi bisogno di passargli una stringa locale posso farlo
char *writer_r(char *string,int sockfd ,int opt, int print_legend){
    int ret;
    int byte_w;
    redo:
        memset(string , 0 , MAXLINE);
        if(print_legend){
            printf("%s\n", legend); // stampa la legenda
        }

        printf("> ");
        ret=scanf("%[^\n]", string);
        fflush(stdin);
        if(ret<1){ // evita di passare stringhe vuote cosi non va in segmentation fault
            printf("%s",cli_str2);
            goto redo;
        }
retransmit:
        ret = send_pkt( string, sockfd); // manda il comando al server
        if (ret < 0) {
            perror("error in sendto");
            memset(string , 0 , MAXLINE);
            strcpy(string , str2); // scrivo in msg il messaggio di error
        }
        recv_pkt(buffercv, sockfd , MAXLINE);
        printf("%s\n", buffercv);
        if(opt==1) // utile al client per confrontare il comando
             string = upper(string);
return string;
}

// funzione che stabiliscle la connessione con il server

int server_open_connection( int sockfd ){
    char *buff = malloc(MAXLINE);
    int ret;

    // invio del SYN
    ret = send_pkt( syn, sockfd);
    if (ret < 0) {
        perror("error in sendto");
        free(buff);
        return -1;
    }
    // ricezione di un pacchetto di synackpkt
    recv_pkt(buff, sockfd , MAXLINE);
    if (strcmp (buff , synack)) {
        perror("error in recvfrom");
        free(buff);
        return -1;
    }

    // invio del ackpkt
    ret = send_pkt( ackpkt, sockfd);
    if (ret < 0) {
        perror("error in sendto");
        free(buff);
        return -1;
    }
printf("connection Estabilished with server\n");
free(buff);
return 0 ;
}

// funzione che stabiliscle la connessione con il server
int server_close_connection( int sockfd ){
    char *buff = malloc(MAXLINE);
    int ret;
    // invio del FIN
    ret = send_pkt( fin, sockfd);
    if (ret < 0) {
        perror("error in sendto FIN");
        free(buff);
        return -1;
    }
    // ricezione di un pacchetto di FINackpkt
    recv_pkt(buff, sockfd , MAXLINE);
    if (strcmp (buff , finack)) {
        perror("error in recvfrom FINackpkt");
        free(buff);
        return -1;
    }
     // ricezione di un pacchetto di FIN
    recv_pkt(buff, sockfd , MAXLINE);
    if (strcmp (buff , fin)) {
        perror("error in recvfrom FIN");
        free(buff);
        return -1;
    }
    // invio del FINackpkt
    ret = send_pkt( finack, sockfd);
    if (ret < 0) {
        perror("error in sendto FINackpkt");
        free(buff);
        return -1;
    }
printf("connection  with server CLOSED\n");
free(buff);
return 0 ;
}

// funzione volta a creare una socket
int create_socket(char *ip, int port){ // accetta una porta e ne restituira la socket associata alla porta
    int result;
    int sock;
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){// creazione della variabile socket
        perror("Error Create Socket ");
        exit(-1);
    }
    memset((void *)&sad , 0 , sizeof(sad));
    sad.sin_family = AF_INET ;
    sad.sin_port = htons(port);

    // errore per il passiaggio dell ip in formato non corretto
    result = inet_aton(ip, &(sad.sin_addr));
    if( result<= 0){
        fprintf(stderr, "Error pass an valid IP %s\n",ip);
        exit(1);
    }
    printf("creata la socket : %s , %d\n", ip , port);
    return sock; // ritorno il descrittore della socket su cui comunicare
}




int main(int argc , char **argv){

    int n,port;
    buffercv = malloc(MAXLINE);
    string= malloc(MAXLINE);
    rt =10; // inizializzazione dei tentativi di connessione
    int ret;
    pathname = malloc(MAXLINE);
    printf("**************************\n");
    printf("raise a SIGQUIT signal than not response client or server \n");
	printf("CONFIGURATION PARAMETERS\nWindow = %d\nLoss probability = %d\nTimeout data = %d\nAdaptive = %d\n", WINDOW_SIZE, LOST_PROB, TIMEOUT,ADAPTIVE);

	printf("**************************\n");



    // parte della gestione sengali
    signal(SIGALRM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    sigset_t set; // variabile locale di una bit mask (signal Mask)
    sigfillset(&set);
    sigdelset(&set , 14); // abilito la sigalarm
    sigdelset(&set , 1); // abilito la sighup -> segnale mandato quando chiudo il terminale
    sigdelset(&set , 3); // abilito la sigquit -> uscita da tastiera (ctrl + \)
    sigdelset(&set , 9);  // abilito la sigkill -> segnale che uccide il processo
    sigdelset(&set , 15);  // abilito la sigterm -> segnale di terminazine del processo
    sigprocmask(SIG_BLOCK,&set,NULL);

    if(argc != 2){
        perror("insert SERVER_IP : <IP server>");
        exit(1);
    }
    sockfd=create_socket(argv[1], (PORT-1)); // Crea una socket di comunicazione iniziale

retry:
    ret = server_open_connection(sockfd);

    if(rt !=0 && ret !=0){
      printf("error Connection refused retry");
      fflush(stdout);
      ret--;
      goto retry;

    }else if(rt==0){
        raise(SIGINT); //invoco il sigint per chiudere il  client

    }else{
        recv_pkt(buffercv, sockfd , MAXLINE); // attendo che mi viene comunicata la porta nuova
        port = atoi(buffercv);
        close(sockfd); // chiudo il vecchio canale di comunicazione
        sockfd = create_socket(argv[1],port); // crea la nuova socket con al nuova porta per poter comunicare
    }

  while (1){
start:
     alarm(TIMEOUT_CON);
     if(strcmp(string , closed)==0){ // se string ha close allora chiudo la connessione
         ret = server_close_connection(sockfd);
         if(ret){
             printf("error to close connection with server\n");
             exit(-1);
        }else{
            free(string); // rilascio la stringa dato che ha una malloc implicita nella scanf
            free(buffercv); // rilascio il bufferino visto che ha una malloc
            close(sockfd); //chiusura del descrittore
            exit(0); // terminazione del processo
        }
    }else if(strcmp(string , getFile)==0){  // GET


        memset(buffercv , 0 , MAXLINE);
        string = writer_r(string, sockfd , 0,0);// mi appoggio alla variabile stringa per passare il filename.
        if(strcmp(buffercv, notfound)==0){
            printf("filename NOT FOUND ");
            memset(string , 0 , MAXLINE); // reset della stringa
            goto start;
        }
        sigprocmask(SIG_UNBLOCK, &set , NULL); //sblocchiamo la maschera di bit
        sigfillset(&set); // blocco tutti i sengali per riattivare solo quelli necessari
        sigdelset(&set , 2);  // sigint
        sigprocmask(SIG_BLOCK,&set,NULL);
        getflag=1;
        memset(pathname ,0 , MAXLINE);

        sprintf(pathname , "clientFiles/%s", buffercv); // concateno e creo un
        ret = main_receiver(sockfd, &sad , pathname);

        while(1){ // questo while ha il solo scopo di consumare tutti i pacchetti duplicati mandati dal sender causa eventuali ack persi o time out interrotti
            set_timeout(sockfd, TIMEOUT);
            if(ret==0)
                recvfrom(sockfd, buffercv, strlen(str6), 0, (struct sockaddr *)&sad, &len); // deve consumare i pacchetti e stampare il messaggio
            if(ret==-1)
                recvfrom(sockfd, buffercv, strlen(str7), 0, (struct sockaddr *)&sad, &len); // deve consumare i pacchetti e stampare il messaggio
            if(strcmp(buffercv , str6)==0 || strcmp(buffercv , str7)==0  ){
                set_timeout(sockfd, 0);
                break;
            }
        }
        printf("%s\n", buffercv); // deve tornare il messaggio di esito dell operazione
        sigprocmask(SIG_UNBLOCK, &set , NULL); //sblocchiamo la maschera di bit
        sigfillset(&set);
        sigdelset(&set , 14); // abilito la sigalarm
        sigdelset(&set , 1); // abilito la sighup -> segnale mandato quando chiudo il terminale
        sigdelset(&set , 3); // abilito la sigquit -> uscita da tastiera (ctrl + \)
        sigdelset(&set , 9);  // abilito la sigkill -> segnale che uccide il processo
        sigdelset(&set , 15);  // abilito la sigterm -> segnale di terminazine del processo
        sigprocmask(SIG_BLOCK,&set,NULL);
        getflag=0;

    }else if(strcmp(string , put)==0){ // PUT

        memset(string , 0 , MAXLINE); // reset della stringa
        char filename [MAXLINE] ;
    redo:
        memset(filename ,0 , MAXLINE);
        printf("insert file to transfert: ");
        ret = scanf("%[^\n]",filename);
        if(ret>MAXLINE){
            printf("filename too long\n");
            memset(filename , 0 , MAXLINE);
            getc(stdin); // svuoto lo stdout
            goto redo;
        }
        getc(stdin);
        // comparare i file della cartella (prima di inviare il filename al server devo controllare al se il file e effettivamente present)
         if(ret = compare_filename(filename)){ // questa list invecce compara gli argomenti della cartella con il filename passato
             send_pkt(filename , sockfd); // mando il filename al server
             recv_pkt(string , sockfd , MAXLINE); // di responso
             if(strcmp(string , str10)==0){
                  memset(string , 0 , MAXLINE); // reset della stringa
                  printf("file exist on the server");
                  goto start; // lo faccio uscire direttamente dalla procedura di get tornando all inzio
            }
        }else{
               printf("invalid filename please retry\n");
               send_pkt(str8, sockfd); // mando una stringa di segnalazione dicendo che al server che deve tornare in attesa di un comando
               memset(filename ,0 , MAXLINE);
               goto start; // lo faccio uscire direttamente dalla procedura di get tornando all inzio
        }
        sigprocmask(SIG_UNBLOCK, &set , NULL); //sblocchiamo la maschera di bit
        sigfillset(&set); // blocco tutti i sengali per riattivare solo quelli necessari
        sigdelset(&set , 2);  // sigint
        sigprocmask(SIG_BLOCK,&set,NULL);
        putflag=1;
        memset(pathname ,0 , MAXLINE);
        sprintf(pathname , "clientFiles/%s", filename); // concateno e creo un pathname su cui devo lavorare
        memset(filename ,0 , MAXLINE);
        recv_pkt(filename , sockfd , MAXLINE); // ricevo un pacchetto che da il via al trasferimento
        if(strcmp(filename , "go")==0){
            main_sender(sockfd, &sad, pathname); // passo la socket e il descrittore associato ma non manda ne riceve nulla
        }
         sigprocmask(SIG_UNBLOCK, &set , NULL); //sblocchiamo la maschera di bit
        sigfillset(&set);
        sigdelset(&set , 14); // abilito la sigalarm
        sigdelset(&set , 1); // abilito la sighup -> segnale mandato quando chiudo il terminale
        sigdelset(&set , 3); // abilito la sigquit -> uscita da tastiera (ctrl + \)
        sigdelset(&set , 9);  // abilito la sigkill -> segnale che uccide il processo
        sigdelset(&set , 15);  // abilito la sigterm -> segnale di terminazine del processo
        sigprocmask(SIG_BLOCK,&set,NULL);
        putflag=0;

        send_pkt(str6, sockfd);  // invio stringa dell esito dell operazione
    }else {
        memset(buffercv , 0 , MAXLINE);
        string = writer_r(string, sockfd , 1,1);// writer
    }
  }
}


