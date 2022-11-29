
int main_sender(int udpSocket, struct sockaddr_in *sender_addr , char *pathname);
void send_data_UDP_start(int steps);
void wait_ack(int win);
