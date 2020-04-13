Fase 1.5 del BIKAYA Project, Corso di Sistemi Operativi @Unibo 2019/2020
Federico De Marchi, Stefano Sgarzi, Manuel Vannini

In questa fase sono state implementate parzialmente alcune delle funzionalità del nucleo del SO BIKAYA.
Nel dettaglio, vengono inizializzati i PCB e le aree di memoria relative alle eccezioni e vengono gestiti gli interrupt relativi al timer e la SYSCALL3 per terminare il processo corrente.
Viene inoltre implementato un primo abbozzo di scheduler che permette di eseguire un test che fa alternare 3 processi di test 1, 2 e 3 che insieme eseguono una stampa.


#COME COMPILARE#
Il codice viene compilato utilizzando SCONS 3.1.2 (https://scons.org/).
Per la compilazione è necessario avere Python 2.7 o 3.x e la rispettiva libreria SCons, installabile tramite pip installer con 'pip install scons' da terminale o scaricabile su https://scons.org/pages/download.html . Necessari inoltre i pacchetti gcc-arm-none-eabi per la compilazione uARM e gcc-mipsel-linux-gnu per la compilazione uMPS.
La compilazione avviene con il comando 'scons TARGET' nella directory principale, dove TARGET è uarm o umps / umps2. Per pulire i file per un target si aggiunge la flag -c ('scons TARGET -c').
Compilare prima per un target e successivamente per un altro non richiede di pulire i file. Importante che venga mantenuta la gerarchia di directory perché il file SConstruct nella directory superiore si appoggia a dei file SConscript nelle directory sottostanti.