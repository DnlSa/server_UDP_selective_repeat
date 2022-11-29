 
#define PORT	10000 // porta di partenza
#define MAX_PORT 15000 // porta massima raggiungibile
#define MAXCLIENT MAX_PORT-PORT // il server prende questi client
#define MAXLINE     1024 // grandezza masssima dei pacchetti di comunicazione
#define SIZE     4096 // grandezza masssima dei pacchetti di comunicazione
//in microsecondi
#define TIMEOUT 8000// valore di timeout STATICO configurabile
#define LOST_PROB 20 // probabilita di perdita
#define WINDOW_SIZE 32// grandezza della finestra di ricezione
// finestra 900 da errore di segmentazione in quanto il server finisce e manda in socket una strigna di fine invece il clinet rimane ancora in ricezione
#define ADAPTIVE 1// imposta il timeout adattivo
#define TIME_UNIT 4000	//valore minimo di cui si può variare timeout
#define MAX_TIMEOUT 80000  //800 millisecondi timeout massimo scelto
#define MIN_TIMEOUT 8000	//2 millisecondi timeout minimo
#define MAX_ERR 25//  numero massimo di errori dopo il quale la get fallisce (throughput molto basso )
#define TIMEOUT_CON 3600 // tempo di timeout dopo il quale la connessione con il server viene chiusa
#define PKT_SIZE 1500 // Biyte del pacchetto che dobbiamo inviare (payload +header)


struct Packet { // struttura del pacchetto dati
	int seqNum; // numero di sequenza
	int win ; // includo la finestra dei pacchetti
	long byte_send; //la posizione del pacchetto su i byte totali
	int size; // totale pacchetti che compongono il file (l ultimo pacchetto potrebbe essere diverso da PKT_SIZE)
	char data[PKT_SIZE]; // payload del pacchetto
	int  sent; // se e stato mandato o no
	int  received; // se e stato ricevuto o no
	int ack; // se e stato riscontrato
};

struct ackPacket{ // struttura del pacchetto di riscontro
   int seqNum; // numero del pacchetto che si riscont ra
   int size;
   long write_byte;
};
