# progetto-iot
Questo progetto è stato realizzato da Francesco Cristoforo Conti, Marco Recca e Matteo Gallina per il corso di Internet Of Things dell'universita di Catania. 
Lo scopo del progetto trovare un modo per garantire la sicurezza dei lavoratori in un tunnel.
La soluzione proposta dal nostro team consiste in un dispositivo mobile, dei dispositivi fissi installati nel tunnel, un server backend che gestisce tutte le operazioni nel tunnel e un front end da cui è possibile supervionari tutti i dati del backend.
I dispositivi utilizzati sono dei ttgo lora 2.1.6, e per alcuni scopi anche dei Arduino nano. 

## I dispositivi mobili 
Essi vengono dati ai lavoratori che entrano del tunnel e, usando un set di sensori biomedici, monitorano lo stato di salute globale tramite una rete neurale costruita ad hoc partendo da uno dei dataset più validati disponibili in rete. Questi dispositivi usano BLE per permettere di interagire con essi tramite smartphone. 

## I dispositivi fissi 
Essi, anche chiamati ancore, sono la parte principale del progetto. Nella versione definitiva utilizzano BLE sia per il settaggio iniziale con le altre ancore, sia per comunicare con i dispositivi mobili. Lo scambio di messaggi avviene tramite LoRa, utilizzando un algorirmo adhoc progettato dal team. 

La prima e l'ultima ancora del tunnel (gateway) non usano ble e sono connessi alla rete per poter inviare e ricevere messaggi dal back end grazie anche a un server HTTPs avviato su esse. 

## Back end

Esso è stato realizzato in Python usando una combinazione della libreria Flask websocket e sfrutta MongoDB come database.

## Front end 

E' stato totalmente realizzato in HTML Javascript e CSS senza l'utilizzo di framework, come una single page application dalla quale è possibile monitorare lo stato delle persone all'interno del tunnel. 

# Organizzazione della repo 
Di seguito si espone come è organizzato il codice della repo e come funziona l'installazione del sistema.

## Ancore

Il codice per le ancore è contenuto nella cartella Ancore. Una volta flashata l'ancora è pronta per essere installata. Decommenta riga 408 per non permettere all'ancora di cercare altre ancore per il settaggio.

I gateway sono caratterizzati da due codici specifici contenuti nelle cartelle "gateway" e "terminatore di destra". Cambia ssid e pass all'interno del codice per fare in modo che si possano connettere ad un'altra rete. Sostituisci l'url in riga 56 con l'url del server in locale.
Infine la cartella "reset ancore" contiene il codice che serve per resettare i dati che le ancore si salvano nella memoria a lungo tempo.

##  Dispositvi mobili
Il codice per i dispositivi mobili è contenuto nelle cartelle "braccialetto" e "braccialetto nano esp" a seconda di che dispositivi si utilizzano.

## Back end e Front end

Infine il codice per il back end e per il front end è contenuto nelle "cartelle back" end e "frontEnd".

# Installazione e utilizzo 

Per installare il sistema si deve procedere collegando per primo un dispositivo con flashato il codice del gateway, poi si puo procedere installando quanti dispositivi si vuole contenenti il codice delle Ancore, una volta soddisfatti dal numero di Ancore connesse si installa il dispositivo contenente il codice del terminatore di destra.

L'utilizzo è totalmente automatico e totalemente gestibile dal FrontEnd realizzato.

Per l'utilizzo completo si puo leggere la relazione completa del progetto.



