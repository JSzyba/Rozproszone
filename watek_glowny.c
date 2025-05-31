#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    // srandom(rank);
    srand(time(NULL) + rank);
    int tag;
    packet_t pkt;

    while (stan != InFinish) {
    
        //Śpij przez jakiś czas (SYMULACJA WALKI)
        int sleepTime = 1 + random()%5;
        int neededMechanics = (random() % NO_MECHANICS) + 1;
        sleep(sleepTime);

        //ZOSTAŁ POSTRZELONY, ROZPOCZĘCIE LOGIKI
        debug("Zostałem postrzelony, Chcę naprawę, lamport = %d", lamportClock);
        
        //RESET ODEBRANYCH ACK_DOCK
        pthread_mutex_lock(&ackMut);
            ackDockCount = 0;
        pthread_mutex_unlock(&ackMut);


        //ZWIĘKSZ WŁASNY CLOCK I USTAW GO DO WYSŁANIA
        pthread_mutex_lock(&clockMut);
            incrementClock();
            pkt.ts = lamportClock;
            pkt.data = 0;
            
            //request brodcast
            sendPacketToAll(&pkt, REQ_DOCK,pkt.ts);
            debug("Żądanie o dok poszło do wszystkich, Chcę dostęp do sekcji krytycznej 1, lamport = %d", lamportClock);
        pthread_mutex_unlock(&clockMut);

        // wait until ACK_DOCK
        pthread_mutex_lock(&ackMut);
            while (!hasDockAccess()) {
                pthread_cond_wait(&condDock, &ackMut);
            }
        pthread_mutex_unlock(&ackMut);


        debug("Wylądowałem w doku, Jestem w sekcji krytycznej 1, lamport = %d",lamportClock);
        
            // LOGIKA DO MECHANIKÓW

            // Reset ack do mechaników
            pthread_mutex_lock(&mechMut);
                ackMechCount = 0;
            pthread_mutex_unlock(&mechMut);

            // Przygotuj wiadomość
            pthread_mutex_lock(&clockMut);
                incrementClock();
                pkt.ts = lamportClock;
                pkt.data = neededMechanics;

                // send all
                sendPacketToAll(&pkt, REQ_MECH,pkt.ts);
                debug("Żadanie o %d Mechaników poszło do wszystkich, Chcę dostęp do sekcji krytycznej 2, lamport = %d", neededMechanics,lamportClock);
            pthread_mutex_unlock(&clockMut);


            pthread_mutex_lock(&mechMut);
                while (!hasMechAccess()) {
                    pthread_cond_wait(&condMech, &mechMut);
                }
            pthread_mutex_unlock(&mechMut);

            
            debug("Dostałem %d Mechaników, start naprawy, Jestem w sekcji krytycznej 2, lamport = %d",neededMechanics,lamportClock);
            
            neededMechanics = 0;
            sleep(sleepTime);

            debug("Naprawili mnie, zwalniam mechaników, Wychodzę z sekcji krytycznej 2, lamport = %d",lamportClock);

            //Zwolnij Mechaników
            
            pthread_mutex_lock(&clockMut);
                incrementClock();
                pkt.ts = lamportClock;
                sendPacketToAll(&pkt, REL_MECH,pkt.ts);
                debug("Koniec naprawy, zwalniam dok, wracam do walki,Wyszedłem z sekcji krytycznej 2 oraz jestem w trakcie wychodzenia z sekcji krytycznej 1, lamport = %d",lamportClock);
            pthread_mutex_unlock(&clockMut);
        
        //Zwolnij Dok
        pthread_mutex_lock(&clockMut);
            incrementClock();
            pkt.ts = lamportClock;
            sendPacketToAll(&pkt, REL_DOCK,pkt.ts);
        pthread_mutex_unlock(&clockMut);
        
    }
}
