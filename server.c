#include "basic.h"
#include "message.h"



////////////////////////// function deglaration //////////////////

void signal_handler(int signum); // funzione di gestione del segnale
char *upper( char *msg); // funzione che voncerte le stringhe nella controparte maiuscola
int send_pkt(char *msg , int child_sock); // funzione di trasmissione messaggi server -> client
char *recv_pkt(char *msg ,  int sockfd , size_t size); // funzione di ricezione client-> server
int open_connect_client( int sockfd ); // instaura la connessione con il client con 3wayHandshake
int close_connect_client( int sockfd ); // chiude la connessione con la 3waiHandshackpkte
int create_socket(int port); // funzione di creazione della socket
int list_funct(int sockfd, int option , char *filename); // funzione di list dei file
int command(int sockfd, int num_client); // funzione atta ricevere i comandi
void connect_client( int sockfd ); // funzione di inizializzazione della connessione



////////////////////GLOBAL VARIABLES////////////////

int port = PORT; // Variabile port che prende in input la porta base
struct sockaddr_in  sad; // variabile struttura della socket locale e numero di porta
socklen_t len = sizeof(sad);


pid_t pid; // variabile di creazione del processo
char buffer[MAXLINE];
char lis[SIZE];
int *addr;

////////////////////////////////////////////////////

void signal_handler(int signum){ // signal handler per la termianzione del processo SERVER
    printf("close the server due to the signal %d\n",signum);
    munmap(addr, MAXLINE); // Libero l area di memoria condivisa prima di terminare il processo main del server
    exit(0);
}


void signal_handler_sigsegv(int signum){ // signal handler per la termianzione del processo SERVER
    printf("receive signal SIGSEGV verify if not corrupt ttransfert file %d\n",signum);
}

// ritorna un messaggio di errore in caso di fallimento altrimenti torna un messaggio buono
char *upper( char *msg){ // funzione che converte una stringa passata in maiuscolo (evita che l utente debba preoccuparsi di inserire comandi in maiuscolo )
    size_t len = strlen(msg) , i;
    char *baseid = msg; // do a baseid l indirizzo base di msg
    for(i=0; i<len ; i++){
         (*baseid) = toupper((*baseid));
         baseid++;
    }
    return msg;
}


int send_pkt(char *msg , int child_sock){
        //if(packpktet_lost(LOST_PROB)){ // se Packpktet lost ritorna 1 allor invia altrimenti non lo fà (il server non sa se e arrivato o no)
            int ret = sendto(child_sock, msg, strlen(msg), 0,(struct sockaddr *)&sad, sizeof(sad)); // da capire ste struture sad
            if(ret <0){
                perror("send messagge failed");
                exit(EXIT_FAILURE);
            }
    //   }
return 0;
}

// funzione che gli passi un bufferino vuoto e la socket su cui leggere
// la funzione legge la socket e ritorna lla stringa letta
// parametri bufferino  , descrittore socket , grandezza dell buffer passato
char *recv_pkt(char *msg ,  int sockfd , size_t size){ // passo come input il descrittore della socket

        int ret;
        memset(msg , 0 , size); // pulizia area di memoria
        ret = recvfrom(sockfd, msg, size, MSG_WAITALL, (struct sockaddr *)&sad, &len); // l ultimo parametro cambia proprio per definizione della funzione questa prende un indirizzo di memoria la send no
        if (ret  < 0 ) { // vado in recivefrom (e in attesa di un pacchetto di syn )
            perror("Error in recvfrom in function recv_pkt");
            strcpy(msg , str1); // scrivo in msg il messaggio di errore
            exit(EXIT_FAILURE);
        }
 return msg; // ritorna la striga
}


// funzione che stabiliscle la connessione con il client che ne fa richiesta

int open_connect_client( int sockfd ){
    char *buff = malloc(MAXLINE);
    int ret;

    printf("Listening..... \n"); // ascolto
    fflush(stdout);
    // ricezione di un pacchetto di syn
    recv_pkt(buff, sockfd , MAXLINE);
    if (strcmp (buff , syn)) {
        perror("error in recvfrom SYN");
        free(buff);
        return -1;
    }
    // invio del SYNackpkt
    ret = send_pkt( synack, sockfd);
    if (ret < 0) {
        perror("error in sendto SYNackpkt");
        free(buff);
        return -1;
    }
    // mi aspetto un ackpkt
  //  set_timeout(sockfd,  TIMEOUT);
    recv_pkt(buff, sockfd , MAXLINE);
    if(strcmp (buff , ackpkt)) {
        set_timeout(sockfd,  0);
        perror("error in recvfrom ackpkt");
        free(buff);
        return -1;
       }
//set_timeout(sockfd,  0);
printf("connection Estabilished with client\n");
free(buff);
return 0 ;
}

int close_connect_client( int sockfd ){
    char *buff = malloc(MAXLINE);
    // Pulisco area di memoria garbage
    memset(buff, 0 , MAXLINE);
    int ret;
    // ricezione di un pacchetto di FIN
    recv_pkt(buff, sockfd , MAXLINE);
    if (strcmp(buff , fin)!=0 ){
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
    ret = send_pkt( fin, sockfd);  // invio del FIN
    if (ret < 0) {
        perror("error in sendto FIN");
        free(buff);
        return -1;
    }
    // attesa di ricevere un finackpkt
    recv_pkt(buff, sockfd , MAXLINE); 
    if (strcmp(buff , finack)!=0 ){
        perror("error in recvfrom FINackpkt");
        free(buff);
        return -1;
    }
printf("connection with client is CLOSED\n");
fflush(stdout);
free(buff);
return 0 ;
}

// funzione volta a creare una socket
int create_socket(int port){ // accetta una porta e ne restituira la socket associata alla porta

    len=sizeof(struct sockaddr_in);
    int sock; // variabile descrittore di socket
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) <0){// creazione della variabile socket
        perror("Error Create Socket ");
        exit(-1);
    }
    memset((void *)&sad , 0 , sizeof(sad)); //pulizia della struttura sockaddr
    sad.sin_family = AF_INET ; // indica il protocollo di comunicazione (AF_INET -> IPv4)
    sad.sin_addr.s_addr = htonl(INADDR_ANY); //codifico in network order l ip della mia macchina
    sad.sin_port = htons(port); // codifica in network order la porta del server (esclusiva per il solo ascolto )

    if ((bind(sock ,(struct sockaddr *) &sad, sizeof(sad))) == -1) { // assegno ip e porta alla socket tramite bind
        perror("error in bind");
        exit(-1);
    }
    printf("Create Socket success : %d\n", port);
    return sock; // ritorno il descrittore della socket su cui comunicare
}

// funzione che lista i file contenuti in una directory specifica del server
int list_funct(int sockfd, int option , char *filename){ 
    int ret;
    DIR *dir;
    struct dirent *dp;
    memset(lis , 0 , sizeof(lis)); // pulisco l area di memoria del buffer
    // ci viene restituito uno stream che punta alle entry della directory
    if((dir = opendir("serverFiles"))== NULL){
        perror("Error, read field in the server ");
        exit(EXIT_FAILURE);
    }
    // cicla finche non arriva ala fine delle entry
	while ((dp = readdir(dir)) != NULL) { 
		if(dp->d_type == DT_REG) {
            if(option &&strcmp(filename , dp->d_name)==0){ // confronta la stringa
                closedir(dir);
                return 1; // se ho il matching
            }else if (option == 0){
                strcat(lis, dp->d_name);
                strcat(lis, "\n");
                if( strlen(lis) >= (MAXLINE-1)) {//caso di saturazione del buffer
                    send_pkt(lis , sockfd); // mando quanto letto
                    memset(lis,0,sizeof(lis)); // pulisco il buffer e ritorno nel while
                }
                else if(strlen(lis) >= (MAXLINE-1)){
                }
            }else{continue;}
        }
    }
    if((option == 0 ) &&strlen(buffer) > 0) {// caso in cui non saturo il buffer
        send_pkt(lis , sockfd); // mando quanto letto
    }
    //chiusura della directory dir
    closedir(dir); // chiudo lo stream che mi punta la directory
    return 0 ;
}



int command(int sockfd, int num_client){
    char *cmd = malloc(MAXLINE);
    int ret;
    while(1){
        redo:
        memset(cmd, 0 , MAXLINE); // pulisco l'area di memoria che ho usato prima
        cmd=recv_pkt(cmd , sockfd , MAXLINE); // aspetto che arrivi un comando dal client
        cmd = upper(cmd); // utile perche almeno il client non si deve preoccupare di dover passare l imput esatto
        fflush(stdout);
       if(strcmp(cmd , list)==0){ // LIST
           if(ret = list_funct(sockfd, 0 , "dummy")){ // passo il campo option
               printf("Client %d - List Command Success\n",num_client);
           }
       }else if(strcmp(cmd , getFile)==0){ // GET
            memset(cmd , 0 , MAXLINE);
            send_pkt(str5, sockfd);  // invio  di riscontro che sono nella get

            char filename [MAXLINE] ;
            recv_pkt(filename , sockfd , MAXLINE); // ricevo il filename
            if(ret = list_funct(sockfd, 1 , filename)){ // questa list invecce compara gli argomenti della cartella con il filename passato
               send_pkt(filename , sockfd); // mando il messaggio che ho trovato il file che cerco reinviando il filename del file
           }else{
               send_pkt(notfound , sockfd);
               goto redo ; // lo faccio uscire direttamente dalla procedura di get tornando all inzio
           }
           char * pathname  = malloc(MAXLINE);
           sprintf(pathname , "serverFiles/%s", filename); // concateno e creo un pathname su cui devo lavorare
           set_timeout(sockfd,  MAX_TIMEOUT); // do un tempo massimo alla socket di ricezione 
           ret = -1; // preimposto a -1 in caso non mi tornasse il messaggio di invio 
           recv_pkt(cmd , sockfd , MAXLINE); // ricevo un pacchetto che da il via al trasferimento

           if(strcmp(cmd , "go")==0){
                set_timeout(sockfd,  0); // ripristino del timeout della socket
                ret = main_sender(sockfd, &sad, pathname); // passo la socket e il descrittore associato ma non manda ne riceve nulla
           }
           set_timeout(sockfd, 0); // ripristino del timeout della socket 
           free(pathname);
           if(ret==0)
                send_pkt(str6, sockfd);  // invio stringa dell esito dell operazione
            else if(ret ==-1)
                send_pkt(str7, sockfd);  // invio stringa dell esito dell operazione
            else{
                close(sockfd);
                return 0 ; // la funzione ritorna 0 in caso di chiusura corretta
            }
       }else if(strcmp(cmd , put)==0){ // PUT
           send_pkt(put , sockfd); // messaggio che mi trovo nella put
           memset(cmd , 0 , MAXLINE);
           recv_pkt(cmd , sockfd , MAXLINE); // ricevo il filename
           if(strcmp(cmd , str8)==0){
                goto redo ;
            }
           if(list_funct(sockfd, 1 , cmd)){ // questa list invce compara gli argomenti della cartella con il filename passato
               send_pkt(str10 , sockfd); // mando il messaggio che ho trovato il file che cerco reinviando il filename del file
               goto redo ;
           }
           send_pkt(str11 , sockfd);
           char * pathname  = malloc(MAXLINE);
           sprintf(pathname , "serverFiles/%s", cmd); // concateno e creo un pathname 
           ret = main_receiver(sockfd, &sad , pathname); // chiamo la receiver
           if(ret == -2){
               free(pathname);
                close(sockfd);
                return 0 ; // la funzione ritorna 0 in caso di chiusura corretta
           }
           memset(pathname , 0 , MAXLINE);
           while(1){ // consumatore di pacchetti ancora in volo 
            set_timeout(sockfd, TIMEOUT);
            recvfrom(sockfd,pathname , strlen(str6), 0, (struct sockaddr *)&sad, &len); 
            if(strcmp(pathname , str6)==0 ){
                set_timeout(sockfd, 0);
                break;
            }
        }
        free(pathname);

      }else if(strcmp(cmd , closed)==0){  //CLOSE
          send_pkt(closed, sockfd);  // invio una stringa di riscontro al client 
          printf("\nCLOSE CONNECTION\n");
          fflush(stdout);
          ret = close_connect_client( sockfd ); // vado in chiusura della connessione
          close(sockfd);
          return ret ; // la funzione ritorna 0 in caso di chiusura corretta

      }else if(strcmp(cmd , str9)==0){  //CLOSE -> in caso di sigquit
          printf("\nCLOSE CONNECTION\n");
          close(sockfd);
          return 0 ; // la funzione ritorna 0 in caso di chiusura corretta

      }else
         send_pkt(str3 , sockfd);
    }
}


void connect_client( int sockfd ){ // funzione dove il client entrera a comunicare con il server
    long len , ret;
    int i;
    addr =(int *)mmap(NULL, MAXCLIENT, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0); // instanzio una memoria condivida per generare una pseudo bitmask condivisa
    memset(addr , 0 , MAXCLIENT);
    while (1){
    retry:
        if(open_connect_client(sockfd)==0){ // apre la conessione con il client . (genera un attsa indotta dalla recivefrom)
            for(i =0 ; i<MAXCLIENT ; i++){ // il prim controllo che viene eseguito e la disponibilita di posto all client che si connette
                if((*(addr+i))==0){
                    *(addr+i)=1; // imposto lo slot occupato
                    break;
                }else if(i==(MAXCLIENT-1)){ // caso in cui non trovo posto
                    send_pkt(str0 ,sockfd); // mando al client la stringa che comunica che il server e pieno
                    goto retry; // con un goto torno in listen il client riporverà in seguito e vedra se si e liberato un posto
                }else
                    printf("bitmask %d\n",*(addr+i));
                    continue;
            }
            // comunico la nuova porta al client
            port = PORT+i; // Incremento il numero di porta in base allo slot libero
            sprintf(buffer, "%d", port);
            send_pkt(buffer , sockfd); // Mando un nuovo numero di porta al client
            printf("clien nr. %d with port: %d is Connected\n",i , port);


            pid = fork(); // genera un nuovo processo
            if(pid<0){ // caso di errore nella socket
                perror("error to create child process");
                exit(-1);
            }
            else if(pid == 0){ // caso in cui ha successo la fork e genera il processo figlio
                int child_sock ;
                signal(SIGSEGV , signal_handler_sigsegv);
                close(sockfd); // chiudo la socket del processo padre in quanto nel processo figlio non mi serve
                child_sock=create_socket(port); // invoco la creazione della socket con la porta esatta

                ret = command(child_sock , i);
                *(addr+i)=0; // imposto a 0 l entry della bitmask condvisa
                printf("Close child Nr. %d with port Nr. %d and code termination %ld \n",i,port ,ret );
                fflush(stdout);
                exit(ret); // chiudo il processo con il codice di trminazione restituito da command
            }else{
                continue; // parte del processo padre(inserita in caso mi dovesse servire mettere qualcosa qui )
            }
        }
    }
}

// la soket che instanzio nell main e quella del padre sara lui a prendere le connessioni in entrata stringere il 3 wai hanschake e poi aprire un processo e creare una socket dedicata il quale sara comunicata al client che la sfruttera come canale di comunicazione
int main(void){

   int sock_listen  = create_socket(PORT-1);// variabile di instanziazione della socket di ascolto
// parte di codice che chiude i sengali e lascia aperto solamente la SIGINT e SIGALARM
   sigset_t set; // variabile locale di una (signal Mask)
   sigfillset(&set); // imposto tutti i segnali a 1 (bloccati )
   sigdelset(&set , 14); // sigalarm (alarm di inattivita del client )
   sigdelset(&set , 2); // sigint  (chiusuea dell admin del server )
   sigdelset(&set , 1); // sighup  (chiusura brutale della finestra)
   sigprocmask(SIG_BLOCK,&set,NULL); // imposto la maschera segnali
   connect_client(sock_listen); // ne passo il file descriptor della socket
while(1){pause();} // pausa indefinita anche se abbastanza inutile perche la connect clien  che gestira le cpnnesisoni non tornera mai nel amin

}

