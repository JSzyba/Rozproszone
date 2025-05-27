#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

struct tagNames_t{
    const char *name;
    int tag;
} tagNames[] = {
    { "pakiet aplikacyjny", APP_PKT },
    { "finish", FINISH },
    { "Potrzebe doku", REQ_DOCK },
    { "Potwierdzenie o Doku", ACK_DOCK },
    { "Zwalniam dok", REL_DOCK },
    { "Potrzebe Mechaników", REQ_MECH },
    { "Potwierdzenie o Mechanikach", ACK_MECH },
    { "Zwolnienie trzymanych mechaników", REL_MECH },
};

const char const *tag2string( int tag )
{
    for (int i=0; i <sizeof(tagNames)/sizeof(struct tagNames_t);i++) {
	if ( tagNames[i].tag == tag )  return tagNames[i].name;
    }
    return "<unknown>";
}

/* tworzy typ MPI_PAKIET_T
*/
void inicjuj_typ_pakietu()
{
    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    int       blocklengths[NITEMS] = {1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

/* opis patrz util.h */
void sendPacket(packet_t *pkt, int destination, int tag, int lamport)
{
    int freepkt=0;
    if (pkt==0) { pkt = malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d, lamport = %d", tag2string( tag), destination,lamport);
    if (freepkt) free(pkt);
}

void changeState( state_t newState )
{
    pthread_mutex_lock( &stateMut );
    if (stan==InFinish) { 
	pthread_mutex_unlock( &stateMut );
        return;
    }
    stan = newState;
    pthread_mutex_unlock( &stateMut );
}

void sendPacketToAll(packet_t *pkt, int tag,int lamport) {
    for (int i = 0; i < size; ++i) {
        sendPacket(pkt, i, tag,lamport);
    }
}

void incrementClock() {
    // pthread_mutex_lock(&clockMut);
    lamportClock++;
    // pthread_mutex_unlock(&clockMut);
}

void updateClock(int receivedTimestamp) {
    pthread_mutex_lock(&clockMut);
    if (receivedTimestamp >= lamportClock)
        lamportClock = receivedTimestamp +1;
    else{
        lamportClock++;
    }
    pthread_mutex_unlock(&clockMut);
}

void addDockRequest(int from, int timestamp) {
    pthread_mutex_lock(&clockMut);
    requestQueue[from] = timestamp;
    pthread_mutex_unlock(&clockMut);
}

void removeDockRequest(int from) {
    pthread_mutex_lock(&clockMut);
    requestQueue[from] = -1;
    pthread_mutex_unlock(&clockMut);
}

int hasDockAccess() {
    int result = FALSE;
    pthread_mutex_lock(&ackMut);

    int myTime = requestQueue[rank];
    int count = 0;

    for (int i = 0; i < size; i++) {
        if (requestQueue[i] != -1) {
            if (requestQueue[i] < myTime || 
               (requestQueue[i] == myTime && i < rank)) {
                count++;
            }
        }
    }

    //ACK od wszystkich oraz jest jednym z Ktych pierwszych
    if (count < NO_DOCKS && ackDockCount >= size) {
        result = TRUE;
    }

    pthread_mutex_unlock(&ackMut);
    return result;
}

void addMechRequest(int from, int timestamp, int howManny) {
    pthread_mutex_lock(&clockMut);
    mechRequestQueue[from].timestamp = timestamp;
    mechRequestQueue[from].requestedMech = howManny;
    pthread_mutex_unlock(&clockMut);
}

void removeMechRequest(int from) {
    pthread_mutex_lock(&clockMut);
    mechRequestQueue[from].timestamp = -1;
    mechRequestQueue[from].requestedMech = 0;
    pthread_mutex_unlock(&clockMut);
}

int hasMechAccess() {
    int result = FALSE;
    pthread_mutex_lock(&mechMut);

    int myTime = mechRequestQueue[rank].timestamp;
    int myNeed = mechRequestQueue[rank].requestedMech;
    int count = 0;

    for (int i = 0; i < size; i++) {
        if (mechRequestQueue[i].timestamp != -1) {
            if (mechRequestQueue[i].timestamp < myTime || 
               (mechRequestQueue[i].timestamp == myTime && i < rank)) {
                count += mechRequestQueue[i].requestedMech;
            }
        }
    }

    //ACK od wszystkich oraz jest liczba mechaników po zużyciu tych które mają pierwszeństwo + moja potrzeba nie przekracza liczby mechaników
    if ( (count + myNeed) <= NO_MECHANICS && ackMechCount >= size) {
        result = TRUE;
        // debug("Uważam, że jest %d użytych Mechaników na %d, lamport = %d",(count + myNeed),NO_MECHANICS,lamportClock);
    }

    pthread_mutex_unlock(&mechMut);
    return result;
}