#include "basic.h"

int seq_num_r ;
int n_r;
struct ackPacket ack;
struct Packet window_r[WINDOW_SIZE]; // buffer di appoggio dei pacchetti per essere scritti nel file
// e invariato e non da problemi
FILE *fp_r;
struct sockaddr_in  sad_r ; // variabile struttura della socket
socklen_t len_r ;
int sock_r; // descrittore socket
char *go = "go"; // messaggio di sincronizzazione fra client ee server
int k_r; // numero totale dei pacchetti
int rest_pkt; // variabile su cui scrivo i pacchetti restanti
int win_r; // finestra di scorrimento
long  size_tot; // numero di byte scritti
int error_rec; // variabile spia d errore massimo
int flag;

int main_receiver(int udpSocket, struct sockaddr_in *receiver_addr , char *pathname){
	// asssegnamento su variabili globali


	sock_r = udpSocket;
	sad_r= (*receiver_addr) ; // variabile struttura della socket
	len_r = sizeof(sad_r);
    seq_num_r = 0;
	k_r = -100;
	flag = 1;
	win_r = WINDOW_SIZE;
	size_tot = 0;
	rest_pkt = 0;
	error_rec = 0;
    fp_r = fopen(pathname, "wb");
	if (fp_r == NULL){
		puts("Closing Connection.");
		return -1;
	}
	// questo invio da il via all invio dei pacchetti
	n_r = sendto(sock_r, go, strlen(go), 0, (struct sockaddr *)&sad_r, len_r);
			if(n_r<0){
			perror("error to send packet from sender");
  			exit(EXIT_FAILURE);
		}
	printf("file succesfully opened\n");
	for (int i = 0; i < win_r; i++) { // inizializzazione della finestra di scorrimento
		struct Packet newPkt;
		newPkt.seqNum = seq_num_r;
		newPkt.received = 0;
		newPkt.byte_send = 0;
		seq_num_r++;
		window_r[i] = newPkt;
	}
	struct timeval end, start;
    gettimeofday(&start, NULL);
	recive_data_start();
	fclose(fp_r);
	gettimeofday(&end, NULL);

	if(error_rec ==2){
		if (remove(pathname) == 0) // cancella il file
			printf("The file is deleted successfully .\n");
		return -2;
	}
	if(n_r<0 || error_rec == 1 ){
      double tm=end.tv_sec-start.tv_sec+(double)(end.tv_usec-start.tv_usec)/1000000;
      double tp=size_tot/tm;
      printf("error transfer file - Transfer time: %f sec [%f KB/s]\n", tm, tp/1024);
	  if (remove(pathname) == 0) // cancella il file
        printf("The file is deleted successfully .\n");
	return -1;
   }
	double tm=end.tv_sec-start.tv_sec+(double)(end.tv_usec-start.tv_usec)/1000000;
    double tp=size_tot/tm;
   	int q=WINDOW_SIZE;

	while (q){
		struct ackPacket ack;
		ack.seqNum = -1; // mando un ack di fin per dire al server che ho ricevuto correttamente  l ultimo pacchetto
		ack.size = -1;
		ack.write_byte = size_tot;
		if(correct_send()){ // porzione di codice che invia ack di riscontro
			n_r = sendto(sock_r, &ack, sizeof(ack), 0, (struct sockaddr *)&sad_r, len_r);
			 if(n_r<0){perror("erroro to sendto in waitack");}
		}
		q--;
	}

	printf("File succesfully obtain...\nbyte totali scritti : %ld-Transfer time: %f sec [%f KB/s]\n",size_tot ,tm, tp/1024);
	return 0;
}

void recive_data_start() {
	int i , n_r, num;
	long nn_r;
	int count = 0 ; // contatore dei pacchetti
     while(k_r !=0){
		struct Packet newPacket;
		; // istruzione vuota perchè a wsl non piacciono le label vicino alle dichiarazioni
		retry:
		n_r = recvfrom(sock_r, &newPacket, sizeof(newPacket), 0, (struct sockaddr *)&sad_r, &len_r); // ricevo il pacchetto dal server (la chiamata e bloccante )
 		if(n_r<0){ // in caso di errore
			perror("error to recvfrom");
			exit(EXIT_FAILURE);
		}
		if(newPacket.seqNum == -100){
			error_rec = 1;
			return ;
		}
		if(newPacket.seqNum == -10){
			error_rec = 2;
			return ;
		}

		num = newPacket.seqNum;
		nn_r=newPacket.byte_send;
		if(flag){
			k_r = newPacket.win; // prendo il numero totale dei pacchetti (operazione fatta solamente dal primo pacchetto che ricevo )
			flag= 0 ;
		}
		// (CASO 2 -> relazione) se mi arriva un pacchetto gia riscontrato allora invero solamente l ack al mittente dicendogli che l ho ricevuto .
		// la terza condizione ha l effetto di far scrivere in ordine i pacchetti scartando la ricezione di alcuni vecchi

		if (window_r[num].received || size_tot < window_r[num].byte_send || nn_r <= size_tot  ) { // nel caso mi arrivasse una pacchetto gia riscontrato nella stessa finestra di ricezione
           // printf("receive duplicate packet %ld \n", nn_r );
			struct ackPacket ack;
			ack.seqNum = num;
			ack.size = nn_r;
			ack.write_byte = size_tot;
            if(correct_send()){ // porzione di codice che invia ack di riscontro
                n_r= sendto(sock_r, &ack, sizeof(ack), 0, (struct sockaddr *)&sad_r, len_r);
                if(n_r<0){perror("erroro to sendto in waitack");}            
            }
        	goto retry;
		}
		else{ // caso in cui mi e giunto un nuovo pacchetto (CASO 1 relazione)
			window_r[num] = newPacket; // lo inserisco nell array di pacchetti di PKT_SIZE (in quanto per errori di rete potrebbero arrivare pacchetti anche fuori sequenza )
			window_r[num].received = 1; // inidica che ho ricevuto il pacchetto
			//printf("packet received: %d  , pkt_expected: %d , %ld ,%d\n", num ,k_r , nn_r,count); // stampo il messaggio di ricezione
			while ( window_r[count].received){ // funzione di scorrimento parte da 0 scrive e passa al successivo
				struct Packet packet = window_r[count]; // creo un pacchetto
				size_tot =packet.byte_send; // aggiorno il contatore
				fwrite(&packet.data, 1, packet.size, fp_r); // scrivo il payload del pacchetto qui dentro al file
				printf("write : %ld Byte - pkt : %d\n",size_tot , k_r); // print dei byte scritti
				//printf("write : %lld MB\n",size_tot/1048576); // print dei MB scritti
				k_r--; // decremento i pacchetti che attendo solamente dopo che viene scritto nel file di destinazione
				struct Packet nullPkt; // genero un pacchetto NULLO
				nullPkt.seqNum = -2;
				nullPkt.received = 0;
				nullPkt.byte_send = -20;
				if((rest_pkt=k_r-WINDOW_SIZE)<0){ // quando arrivo alla fne
					window_r[count] = nullPkt;
					count++; //incremento il contatore
					if(count == WINDOW_SIZE) // quando raggiungo l'ultimo pacchetto faccio il reset del contatore
						count=0;
					if(window_r[count].received  == 0)
						break;
					else{
						continue;
					}
				 }else{
					window_r[count] = nullPkt; //metto il null pkt al posto di quello che ho scritto
					count++; //incremento il contatore
					if(count == WINDOW_SIZE) // qiando raggiungo l ultimo pacchetto faccio il reset del contatore
						count=0;
				}
			}
			struct ackPacket ack; // creo la variabile stuttura dell ack
			ack.seqNum = num; // inserisco il numero di sequenza del mio pacchetto arrivato
			ack.size =  nn_r ; // Restituisco al server il numero di byte ricevuti
			ack.write_byte = size_tot;// numero di byte scritti (sono utili per creare il cumulative ack )
			if(correct_send()){ // porzione di codice che invia ack di riscontro
				n_r = sendto(sock_r, &ack, sizeof(ack), 0, (struct sockaddr *)&sad_r, len_r);
				if(n_r<0){perror("erroro to sendto in waitack");}
            }
			window_r[num].ack = 1; // indica che ho inivato il pacchetto (fuori dalla correct send perche il chi manda non deve capire se e stato fatto o no )
		}
   }
}
