#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    packet_t pakiet;

    while ( stan!=InFinish ) {
	// debug("czekam na wiadomości, lamport = %d",lamportClock);
    
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        updateClock(pakiet.ts);

        debug("Otrzymałem %s od %d, lamport = %d\n", tag2string(status.MPI_TAG), pakiet.src, lamportClock);

        switch ( status.MPI_TAG ) {
	    case FINISH: 
            changeState(InFinish);
	        break;

	    case REQ_DOCK: 
            addDockRequest(pakiet.src, pakiet.ts);
            packet_t ackPkt;

            pthread_mutex_lock(&clockMut);
                ackPkt.ts = lamportClock;
                sendPacket(&ackPkt, pakiet.src, ACK_DOCK, ackPkt.ts);
            pthread_mutex_unlock(&clockMut);
            
	        break;

        case ACK_DOCK: 
            pthread_mutex_lock(&ackMut);
                ackDockCount++;
                // wake up anyone waiting on “dock access”
                pthread_cond_signal(&condDock);
            pthread_mutex_unlock(&ackMut);
	        break;

        case REL_DOCK: 
            removeDockRequest(pakiet.src);                
            
            // wake up anyone waiting on “dock access”
            pthread_mutex_lock(&ackMut);
            pthread_cond_signal(&condDock);
            pthread_mutex_unlock(&ackMut);
            
            break;

        case REQ_MECH: 
            addMechRequest(pakiet.src,pakiet.ts,pakiet.data);
            packet_t ackPkt2;

            pthread_mutex_lock(&clockMut);
                ackPkt2.ts = lamportClock;
                sendPacket(&ackPkt2, pakiet.src, ACK_MECH, ackPkt2.ts);
            pthread_mutex_unlock(&clockMut);

	        break;

        case ACK_MECH:
            pthread_mutex_lock(&mechMut);
                ackMechCount++;
                // wake up anyone waiting on “mech access”
                pthread_cond_signal(&condMech);
            pthread_mutex_unlock(&mechMut);
            break;

        case REL_MECH: 
            removeMechRequest(pakiet.src);

            // wake up anyone waiting on “mech access”
            pthread_mutex_lock(&mechMut);
            pthread_cond_signal(&condMech);
            pthread_mutex_unlock(&mechMut);
	        break;

	    default:
            debug("Dostałem pakiet od %d z danymi %d, lamport = %d",pakiet.src, pakiet.data,lamportClock);
	    break;
        }
    }
}
