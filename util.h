#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;  

    int data;     

} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3

/* Typy wiadomości */
#define APP_PKT 1
#define FINISH 2
#define REQ_DOCK 3
#define ACK_DOCK 4
#define REL_DOCK 5
#define REQ_MECH 6
#define ACK_MECH 7
#define REL_MECH 8


extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

const char *tag2string(int tag);

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag, int lamport);
#endif

void sendPacketToAll(packet_t *pkt, int tag, int lamport);
