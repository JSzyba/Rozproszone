#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "util.h"

/* boolean */
#define TRUE 1
#define FALSE 0
#define SEC_IN_STATE 1

#define ROOT 0

// WŁASNE 
typedef struct { //struktura do komunikacji o mechaników
    int timestamp;
    int requestedMech;
} MechRequest;

#define NO_MECHANICS 40 //No of Mechanics
#define NO_DOCKS 4 //No of Docks
#define MAX_PROCESSES 128 //tylko do stworzenia Queue, można by poszerzyć bez komplikacji

extern MechRequest mechRequestQueue[MAX_PROCESSES];
extern int requestQueue[MAX_PROCESSES]; // -1 jeśli bez REQ

extern int lamportClock;
extern int ackDockCount;
extern int ackMechCount;

extern pthread_mutex_t clockMut;    
extern pthread_mutex_t ackMut;       
extern pthread_mutex_t mechMut;     
extern pthread_cond_t condDock;   // signal “dock access available”
extern pthread_cond_t condMech;   // signal “mech access available”

void incrementClock();
void updateClock(int receivedTimestamp);

int hasDockAccess();
void addDockRequest(int from, int timestamp);
void removeDockRequest(int from);

int hasMechAccess();
void addMechRequest(int from, int timestamp, int howManny);
void removeMechRequest(int from);
//

extern int rank;
extern int size;
typedef enum {InRun, InMonitor, InSend, InFinish} state_t;
extern state_t stan;
extern pthread_t threadKom, threadMon;

extern pthread_mutex_t stateMut;

/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta 
   
   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
					   "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape. 
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów
                                            
*/
#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

// makro println - to samo co debug, ale wyświetla się zawsze
#define println(FORMAT,...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);

void changeState( state_t );

#endif
