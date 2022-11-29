#include "basic.h"

int seq_num_s; // numero di sequenza
struct Packet window_s[WINDOW_SIZE]; // finestra di pacchetti
int eof_set ; // numero di pacchetti costituenti il file
int n_s;
long long size_file;
long long counter;
int max_err ;
struct sockaddr_in  sad_s ; // variabile struttura della socket
socklen_t len_s ;
FILE *fp_src;
int sock_s;
int k_s;
int win;
int time_out;


int main_sender(int sockfd, struct sockaddr_in *sender_addr , char *pathname){
   sock_s = sockfd;
   sad_s=  (*sender_addr) ; // variabile struttura della socket
   len_s= sizeof(sad_s);
   seq_num_s = 0; // numero di sequenza
   eof_set = 0; // numero di pacchetti costituenti il file
   max_err = 0;
   counter=0;
   k_s =0 ;
   time_out =0 ;
   set_timeout(sockfd, TIMEOUT); // impostato un primo time out modificato durante la trasmissione
   fp_src = fopen(pathname, "rb");
   if (fp_src == NULL)
      printf("File open failed!\n");
   else
      printf("File successfully opened!\n");

   puts("Sending file START timer");
   fflush(stdout);
   struct timeval end, start; // stanzio variabili atte a calcolare i temi di trasmissione 
   fseek(fp_src, 0, SEEK_END);  // calcolo la grandezza del file (anche nelle successive 2 righe )
   size_file=ftell(fp_src);
   fseek(fp_src , 0, SEEK_SET);

    if(size_file % PKT_SIZE){ // calcolo i pacchetti che devo inviare 
      k_s= (size_file/PKT_SIZE)+1; // numero totale dei pacchetti
    }else{
      k_s= (size_file/PKT_SIZE);
    }
   if((win = k_s-WINDOW_SIZE)<0){
      win = k_s; // metto i pacchetti restanti
   }else{
      win = WINDOW_SIZE; //se il risutato e maggiore di 0 allora la finestra e WINDOW_SIZE
   }
   for (int i = 0; i < win; i++){ // genera la finestra iniziale
      struct Packet packet;
      size_t num_read = fread(packet.data, 1, PKT_SIZE, fp_src); // legge il file che deve impacchettare
      counter = counter+num_read; // variabile che indica i byte impacchettati
      packet.seqNum = seq_num_s; // inserisce il numero di seguenza
      packet.win = k_s; // inserisco il numero dei pacchetti attesi
      packet.byte_send = counter; // prende il byte
      packet.size = num_read; // grandezza del payload letto (dovrebbe essere PKT_SIZE)
      packet.sent = 0; // dichiaro i booleani in 0 indicando che non ho ancora inviato
      packet.ack = 0; // anche l ack perche logicamente non e stato ancora riscontrato
      window_s[i] = packet; // inserisco  il pacchetto nell array
      seq_num_s++; // contatore della sequenza dei pachhetti (utili per rimetterli in ordine nel receiver )
      if (seq_num_s >= WINDOW_SIZE) { // reset in caso il numero di pacchetti creati e piu grande di WINDOw_SIZE
         seq_num_s = 0; // gointi al completamenteo della finestra resetto il tutto
      }
   }
   gettimeofday(&start, NULL); // da qui inizia il trasferimento
   send_data_UDP_start(win);
   wait_ack(win);
   gettimeofday(&end, NULL);
   int ret=0;
   while(1){ // consumatore di pacchetti ancora in volo
      struct ackPacket newAck; // varibile ack su cui ricevo l ack
      n_s=recvfrom(sock_s, &newAck, sizeof(newAck), 0,  (struct sockaddr *)&sad_s, &len_s);
      ret = newAck.write_byte;
      if(n_s<0){
         break;
      }
   }
   if (fp_src != NULL)
      fclose(fp_src);
   set_timeout(sockfd, 0);

   if(ret == -10)
      return -2;



   if(max_err == MAX_ERR){ // in caso perdo tutti i pacchetti di una finestra allora il mio canale sarà cosi degradato che chiudo il tasferimento pacchetti
      double tm=end.tv_sec-start.tv_sec+(double)(end.tv_usec-start.tv_usec)/1000000;
      double tp=size_file/tm;
      printf("error to send file - Transfer time: %f sec [%f KB/s]\n", tm, tp/1024);
      int q=WINDOW_SIZE;
      while (q){
		 struct Packet packet; // pacchetto speciale che indica che si e verificato un errore
         packet.seqNum = -100; // inserisce il numero di seguenza'
         packet.win = 0; // inserisco il numero dei pacchetti attesi
         packet.byte_send = 0; // prende il byte
         packet.size = 0; // grandezza del payload letto (dovrebbe essere PKT_SIZE)
         packet.sent = 0; // dichiaro i booleani in 0 indicando che non ho ancora inviato
         packet.ack = 0; // anche l ack perche logicamente non e stato ancora riscontrato
         if(correct_send()){ // porzione di codice che invia ack di riscontro
			n_s = sendto(sock_s, &packet, sizeof(packet), 0, (struct sockaddr *)&sad_s, sizeof(sad_s)); // mando il pacchetto;
         if(n_s<0){
              perror("erroro to sendto");
         }
		   }q--;
      }
      if(ret == -10)
         return -2;
      return -1;
   }
   double tm=end.tv_sec-start.tv_sec+(double)(end.tv_usec-start.tv_usec)/1000000;
   double tp=size_file/tm;
   printf("File sending complete...\n-Transfer time: %f sec [%f KB/s]\n", tm, tp/1024 );
   return 0;
}

void send_data_UDP_start(int steps) { // funzione che  manda i pacchetti in pipeline solo all inizio 
     for (int i = 0; i < steps; i++) { // ciclo su i pacchetti
            if (window_s[i].ack == 0){ // if di controllo se il pacchetot non e stato riscontrato
               if(correct_send()){ // perdita del pacchetto
                  n_s = sendto(sock_s, &window_s[i], sizeof(window_s[i]),0, (struct sockaddr *)&sad_s, sizeof(sad_s)); // mando il pacchetto
                   if(n_s<0){
                     perror("erroro to sendto");
               }
              if (window_s[i].size == 0 || n_s ==0 ) // quando arrivo alla fine del file
                  break;
               }
         }
     }
}

void wait_ack(int win){ // funzione di attesa dell ack
   int n_s, ack_num;
   int nn_s ;
   int count_s=0;
   size_t num_read;

   while(1){
      wait_for_ack:
      ; // istruzione vuota perche a WSL non piacciono gli assegnamenti vicino le label
      struct ackPacket newAck; // varibile ack su cui ricevo l ack 
      n_s = recvfrom(sock_s, &newAck, sizeof(newAck), 0,  (struct sockaddr *)&sad_s, &len_s); // aspetto un ack
      if (n_s < 0 ){ // se il timer scade allora trovo il primo pacchetto non riscontrato e lo mando
         max_err++;
         if(max_err == MAX_ERR){  // funzione che capta errori
            return ;
         }
         if(ADAPTIVE) { 
			increase_timeout(sock_s); // incremento del timeout
		 }
          for (int i = 0; i < win; i++) { // for che rintraccia i pacchetti non riscontrati
             if(window_s[i].ack == 0 || window_s[i].byte_send > nn_s  ){ // se i byte scritti dal client e piu piccolo del byte che contiene il pacchetto invio
               if(correct_send()){ // perdita del pacchetto
                  n_s = sendto(sock_s, &window_s[i], sizeof(window_s[i]), MSG_CONFIRM, (struct sockaddr *)&sad_s, sizeof(sad_s)); // ritrasmissione pacchetto
                   if(n_s<0){
                     perror("erroro to sendto in waitack");
                  }
               }
             }
          }
           goto wait_for_ack; // spostando questa istruzione qui fuori ogni volta che fallisce la receive vengono rimandati iterativamente tutti i pacchetti
      }
      //printf("%ld/%lld - %d\n",  newAck.write_byte , size_file  , max_err );
      max_err=0;
      if(newAck.seqNum == -1 || newAck.write_byte > size_file ||newAck.write_byte == -10  ){ // in caso si riceva il pacchetto finale che mi indica che il file e arrivato correttamente
         return ;
      }
      if(ADAPTIVE) { // caso in cui si riceve correttamente il pacchetto
		    decrease_timeout(sock_s);
      }
      ack_num = newAck.seqNum;
      if(newAck.size>nn_s) // if che aggiorna quantità di byte ricevuti , ho messo questo perche i pacchetti che possono arrivare possono esere fuori ordine
         nn_s= newAck.size; // infica i byte scritti dal client

      if (window_s[ack_num].ack == 1) { // se il pacchetto e gia stato riscontrato passo al successivo ritornando su
        continue; // mi occorre giusto per saltare l else
     }else { // altirmenti
         window_s[ack_num].ack = 1; // altrimento lo flaggo a 1 perche e quello che mi e arrivato
         //printf("ack received: %d - pkt_expected: %d - byte send :  %d\n", ack_num , k_s , nn_s );
         while ( window_s[count_s].ack){ // funzione di scorrimento parte da 0 scrive e passa al successivo
            k_s--;
            struct Packet packet;
            num_read = fread(packet.data, 1, PKT_SIZE, fp_src); // legge il file che deve impacchettare
            if(num_read == 0){// indica che e arrivato alla fine del file 
               break;       
            }
            else { 
               counter = counter+num_read; // variabile che indica i byte impacchettati
               packet.seqNum = count_s; // inserisce il numero di seguenza
               packet.win = k_s; // inserisco il numero dei pacchetti attesi
               packet.byte_send = counter; // prende il byte
               packet.size = num_read; // grandezza del payload letto (dovrebbe essere PKT_SIZE
               packet.sent = 0; // dichiaro i booleani in 0 indicando che non ho ancora inviato
               packet.ack = 0; // anche l ack perche logicamente non e stato ancora riscontrato
               window_s[count_s] = packet; //inserisco il nuovo pacchetto nello slot che e stato letto dal client e che puo essere rimpizzato
               if(correct_send()){ // perdita del pacchetto
                  n_s = sendto(sock_s, &window_s[count_s], sizeof(window_s[count_s]), 0, (struct sockaddr *)&sad_s, sizeof(sad_s)); // mando il pacchetto
                  if(n_s<0){
                     perror("erroro to sendto in waitack");
                  }
               }
               if (window_s[count_s].size == 0) // quando arrivo alla fine del file
                 break;
               if (n_s == 0) // caso di errore
                  break;
            }
            count_s++; //incremento l identificatore del pacchetto che sto attendendo 
            if(count_s == win) // qiando raggiungo l ultimo pacchetto faccio il reset del contatore
               count_s=0;
         }
      }
   }
}


