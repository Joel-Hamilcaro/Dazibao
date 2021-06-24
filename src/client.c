# include <errno.h>     // errno
# include <stdio.h>     // perror, printf ...
# include <stdlib.h>    // exit
# include <string.h>    // memset, memcpy
# include <arpa/inet.h> // sendto, recvfrom
# include <unistd.h>    // close
# include <sys/time.h> // gettimeofday
# include "client.h"
# include "client_manager.h"
# include "noeud.h"
# include "donnee.h"
# include "list.h"
# include "voisin.h"
# include "calculator.h"
# include "tvl.h"
# include "paquet.h"


int main(int argc, char* argv[]){

  // =================================================================== //
  // Buffers, variables
  // =================================================================== //

  unsigned char nodeID[8]; // ID du noeud courant
  unsigned char nodeSTID[8]; // ID du noeud demandé par Node State Request
  unsigned char ipBuf[16]; // Buffer pour adresse IP
  unsigned char portBuf[2]; // Buffer pour numero de port
  unsigned char tvl_neighbour_buf[18]; // Buffer pour TVL Neighbour
  unsigned char networkhash[16]; // Buffer pour TVL Network Hash
  unsigned char req[SIZE]; // Buffer pour Datagramme à envoyer
  unsigned char reply[SIZE]; // Buffer pour Datagramme reçu
  list* tvl_sequence = new_list(); // Séquence TVL
  list* maybe_voisins = new_list(); // Table de voisins potentiels (utile au début)
  list* voisins = new_list(); // Table des voisins
  list* donnees = new_list(); // Table des données
  list* id_list = new_list(); // Liste de nodeID (utile pour mettre à jour la table des données)
  void (*pv)(void*) = &print_voisin; // Pointeur de fonction d'affichage
  void (*pd)(void*) = &print_donnee; // Pointeur de fonction d'affichage
  voisin* neighbour; // Pour stocker le voisin correspondant à l'envoie d'un TVL Neighbour
  char* server_name = "name_of_the_server"; // Nom de serveur initial
  char* server_port = "1212"; // Numero de port d'écoute du serveur initial
  int socket_cli, tmp=0, dontchange=0, which_one=0, seqNo=0, len, rc, boucle=0, random,
  choice=DEFAULT_NODE_STATE, secondes=0, debug = argc>1?1:0, node_hash_sent=0 ; // Variables diverses
  struct sockaddr_in6* server; // Pour stocker l'adresse du pair courant à contacter
  struct sockaddr_in6* jch_server; // Adresse du pair initial (à l'initialisation)

  struct timeval tv_last; // Utile pour les TVL à envoyer seulement toutes les 20 secondes
  gettimeofday(&tv_last,NULL); // Utile pour les TVL à envoyer seulement toutes les 20 secondes
  struct timeval tv_current; // Utile pour les TVL à envoyer seulement toutes les 20 secondes

  // =================================================================== //
  // Initialiser un ID
  // =================================================================== //


  uint64_t id = newNodeID();
  memcpy(nodeID,&id,8);
  printf("\n\n(Pour un affichage plus joli, veuillez mettre le Terminal en plein écran)\n\n");

  printf("=======================================================================\n");
  printf("INITIALISATION DU PAIR :\n\tNodeID :");
  print_hex_by_hex(nodeID,8);
  printf("\n");
  printf("=======================================================================\n");

  // =================================================================== //
  // Initialiser le serveur initial
  // =================================================================== //

  socket_cli = my_udp_socket(); // envoie les tvl
  jch_server = ip_from_addrinfo(server_name,server_port,maybe_voisins);
  // Memory Leak ici ""normal"" (appel de getaddrinfo - Cf Mail du 7/05)
  server = jch_server;
  printf("Lien établit avec le serveur : %s\n",server_name);
  printf("Numéro de port : %s\n",server_port);
  printf("=======================================================================\n");


  // =================================================================== //
  // L'algorithme :
  // =================================================================== //

  again :

    gettimeofday(&tv_current,NULL); // On récupère le temps courant
    long ecart = tv_current.tv_sec - tv_last.tv_sec;

    if (ecart >= 10 && choice==DEFAULT_NODE_STATE){
      choice=NODE_STATE_PERSO;
    }

    if (ecart > 20){

    }

    if (boucle<10){  // Initialisation
      if (boucle==0){
        tmp = 1;
        printf("Recherche de pairs à partir du serveur %s\n",server_name);
      }
      choice *= NEIGHBOURG_REQ;
      boucle++;
    }

    else if (boucle==10){ // Fin d'initialisation

      if (!debug) printf("---> RESULTAT : \n");
      if (!debug) printf("------------------------------------------------------------------------------------------------\n");
      if (!debug) print_list(maybe_voisins,(*pv));
      if (!debug) printf("------------------------------------------------------------------------------------------------\n");

      printf("\n\n\n\n ... DEMARRAGE DU PROTOCOLE ...\n\n\n");
      tmp = 0;
      boucle++;
    }

    else { // Cas normal (on a des pairs à contacter)

      if (!dontchange){ // On change le voisin à contacter (on choisi au hasard)
        if (maybe_voisins->size ==0){
          printf("Aucun pair n'a été trouvé.\n");
          return 0;
        }
        which_one = (which_one+1)%maybe_voisins->size;
        printf("=======================================================================\n");
        printf("Etablissement d'un contact vers un pair ... \n");
        voisin* pair = list_get_data_by_index(maybe_voisins,which_one);
        unsigned char voisin_ip_buf[SIZE];
        unsigned char voisin_port_buf[SIZE];
        memset(voisin_ip_buf,0,SIZE);
        memset(voisin_port_buf,0,SIZE);
        get_voisin_ip(pair,voisin_ip_buf,SIZE);
        get_voisin_port(pair, voisin_port_buf,SIZE);
        printf("IP du pair : %s\n",voisin_ip_buf);
        printf("Port d'écoute du pair : %s\n",voisin_port_buf);
        struct sockaddr_in6 *pair_server = ip_from_addrinfo((char*)voisin_ip_buf,(char*)voisin_port_buf,NULL);
        // Memory Leak ici ""normal"" (appel de getaddrinfo - Cf Mail du 7/05)
        server = pair_server;
      }

    }

    if (debug) printf("<---> CHOIX : %d <---> BOUCLE : %d\n",choice,boucle);
    if (debug) printf("Donnees size : %d , node_hash_sent : %d\n",donnees->size,node_hash_sent);
    // =================================================================== //
    // Construire la sequence TVL
    // =================================================================== //

    memset(req,0,SIZE);
    memset(reply,0,SIZE);
    memset(ipBuf,0,16);
    memset(portBuf,0,2);
    memset(tvl_neighbour_buf,0,18);
    list_remove_all(tvl_sequence);
    if (!debug && boucle>10) printf("Initialisation de la séquence TVL : \n");

    // Node hash ========================================== //

    if (node_hash_sent<donnees->size && node_hash_sent>0){
      choice = NODE_HASH;
    }

    if (choice%NODE_HASH==0){

      elem* e1 = donnees->first;
      unsigned char nhbuf[NODE_ID_SIZE+SEQNO_SIZE+HI_SIZE];
      for (int i=node_hash_sent; i<donnees->size; i++){
        donnee* don = (donnee*)(e1->data);
        tvl* ptv = new_tvl_node_hash(don, nhbuf);
        list_add(tvl_sequence,ptv,0);
        printf("_ Node Hash\n");
        node_hash_sent++;
        e1 = e1->next;
        if (tvl_sequence->size>=35 ) break;
      }

    }

    // Default node state : ==================================================== //

    if (choice%DEFAULT_NODE_STATE==0){

      unsigned char buf[1024];
      memset(buf,0,1024);
      donnee* don = new_donnee(nodeID,seqNo,buf,0); // MC03
      if (debug) print_donnee(don);
      tvl* ptv = new_tvl_node_state(don, buf);
      list_add(tvl_sequence,ptv,0);
      if (1 && boucle>10) printf("\t  _ TVL Node state par défaut (Donnée de longueur nulle)\n");
      free_donnee(don); // MC03
    }

    // Node state en réponse à un node state request =================== //

    if (choice%NODE_STATE==0){
      unsigned char buf[1024];
      memset(buf,0,1024);
      donnee* don = get_donnee_by_id(donnees,nodeSTID);
      if (don!=NULL){
        if (debug) print_donnee(don);
        tvl* ptv = new_tvl_node_state(don, buf);
        list_add(tvl_sequence,ptv,0);
        if (!debug && boucle>10) printf("\t  _ TVL Node state\n");
      }
    }

    // Node state personnel ============================================ //


    if (choice%NODE_STATE_PERSO==0){
     unsigned char buf[1024];
     memset(buf,0,1024);
     unsigned char* s = (unsigned char*)"OK";
     int ch_len = strlen((char*)s);
     if (ch_len>192) ch_len = 192;
     unsigned char* ch = malloc(ch_len);
     memcpy(ch,s,ch_len);
     donnee* don = new_donnee(nodeID,++seqNo,ch,(ch_len));
     if (debug) print_donnee(don);
     tvl* ptv = new_tvl_node_state(don, buf);
     list_add(tvl_sequence,ptv,0);
     free(ch);
     if (!debug && boucle>10) printf("\t  _ TVL Node state (Nouvelle donnée : %s)\n",s);
     free_donnee(don);
   }

    // Neighbour Request : ============================================= //

    if (choice%NEIGHBOURG_REQ==0){
      tvl* ptv = new_tvl_neighbour_req();
      list_add(tvl_sequence,ptv,0);
      if (!debug && boucle>10) printf("\t  _ TVL Neighbour request\n");
    }

    // Neighbour  : ==================================================== //

    if (choice%NEIGHBOURG==0){
      if (neighbour!=NULL){
        tvl* ptv = new_tvl_neighbour(neighbour->ip,neighbour->port,tvl_neighbour_buf);
        neighbour = NULL;
        list_add(tvl_sequence,ptv,0);
        if (!debug && boucle>10) printf("\t  _ TVL Neighbour \n");
      }
    }

    // Network hash  : ================================================= //



    if (ecart >= 20 || choice%NETWORK_HASH == 0){
      gettimeofday(&tv_last,NULL); // On récupère le temps courant

      // Parcourir la table des voisins
      for (int i=0; i<voisins->size; i++){
        elem* e = (list_get_by_index(voisins,i));
        voisin* v = (voisin*)e->data;
        long ect = tv_current.tv_sec - (v->date)->tv_sec;
        if (ect >= 70){
          list_remove(voisins,e);
          free_voisin(v);
          i--;
        }
      }


      if (!debug && !tmp) printf("=======================================================================\n");
      if (!debug && !tmp) printf("VOISINS\n");
      if (!debug && !tmp) printf("-----------------------------------------------------------------------------------------\n");
      if (!debug && !tmp) printf("Permanent\t Date\t\t\t\t\t IP\t\t\t Port\n");
      if (!debug && !tmp) printf("------------------------------------------------------------------------------------------------\n");
      if (!debug && !tmp) print_list(voisins,(*pv));
      if (!debug && !tmp) printf("------------------------------------------------------------------------------------------------\n");
      if (!debug && !tmp) printf("=======================================================================\n");



      unsigned char networkhash[16];
      H(donnees,networkhash);
      tvl* ptv = new_tvl_network_hash(networkhash);
      if (!debug && boucle>10) printf("\t  _ TVL Network Hash\n");
      list_add(tvl_sequence,ptv,0);
    }

    // Network state request ===================================================== //

    if (choice%NETWORK_STATE_REQ==0){

        tvl* ptv = new_tvl_network_state_req();
        list_add(tvl_sequence,ptv,0);
        if (!debug && boucle>10) printf("\t  _ TVL Network State Request\n");

    }

    // Node state request ===================================================== //

    if (choice%NODE_STATE_REQ==0){
      while(id_list->size>0){
        unsigned char* id = (unsigned char*)list_remove(id_list,id_list->last);
        tvl* ptv = new_tvl_node_state_req(id);
        list_add(tvl_sequence,ptv,0);
        if (!debug && boucle>10) printf("\t  _ TVL Node State Request\n");
        if (tvl_sequence->size>=35 ) break;
      }
    }

    // ================================================================= //
    // Envoi du paquet
    // ================================================================= //

    len = write_paquet(req,tvl_sequence);
    rc = sendto(socket_cli, req, len, 0, (struct sockaddr*)server, (socklen_t)sizeof(*server));
    if (debug) print_datagram(req,len,0,debug);

    if(rc < 0) {
      if(errno == EAGAIN) {
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(socket_cli, &writefds);
        select(socket_cli + 1, NULL, &writefds, NULL, NULL);
        printf("======================================================================\n\n\n\n");
        choice = DEFAULT_NODE_STATE;
        goto again;
      }
      perror("Le paquet n'a pas été envoyé : ");
      printf("======================================================================\n\n\n\n");
      choice = DEFAULT_NODE_STATE;
      goto again;
    }

    if (!debug && !tmp) printf("Le paquet a été envoyé !\n");
    if (!debug && !tmp) printf("======================================================================\n\n\n\n");

    if (choice%NODE_HASH==0 && node_hash_sent<donnees->size) goto again;
    if (choice%NODE_HASH==0 && node_hash_sent==donnees->size){
      node_hash_sent = 0;
      dontchange = 0;
      choice = DEFAULT_NODE_STATE;
      goto again;
    }
    if (id_list->size>0){
      choice = NODE_STATE_REQ;
      goto again;
    }

    choice = DEFAULT_NODE_STATE;
    // =================================================================== //
    // Lecture d'un paquet reçu
    // =================================================================== //

    int to = 1;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socket_cli, &readfds);
    struct timeval tv = {to, 0};
    rc = select(socket_cli + 1, &readfds, NULL, NULL, &tv);

    if(rc < 0) {
      perror("select");
      exit(1);
    }

    struct sockaddr_in6 client;
    socklen_t client_len = sizeof(client);

    if(rc > 0) {
      if(FD_ISSET(socket_cli, &readfds)) {

        rc = recvfrom(socket_cli, reply, SIZE, 0, (struct sockaddr*) &client, &client_len);

        if(rc < 0) {
          if(errno == EAGAIN) {
            // Le paquet a disparu depuis le select ?
            printf("======================================================================\n\n");
            goto again;
          }
          else {
            perror("recvfrom");
            exit(1);
          }
        }
      // réponse bien reçue, on peut la gérer
      } else {
        fprintf(stderr,
        "Eek ! Descripteur de fichier inattendu !\n");
        exit(1);
      }
    } else {
      // timeout
      if(to < 64) {
        to = to * 2;
        // backoff exponentiel
        //printf("TIMEOUT (%d sec) ! Pas de réponse.\n",to);
        //printf("======================================================================\n\n");
        goto again;
      } else {
        fprintf(stderr, "Timeout dépassé.\n");
        exit(1);
      }
    }

    // =================================================================== //
    // Traitement du paquet reçu (Partie 1)
    // =================================================================== //

    if (!debug && !tmp) printf("\n\n\n\n\t\t\t\t======================================================================\n\n");
    if (!debug && !tmp) printf("\t\t\t\tPaquet reçu :\n\t\t\t\tEmetteur :");
    if (!debug && !tmp) print_client((struct sockaddr*)&client);
    if (debug) print_datagram(reply,SIZE,1,debug);


    /*
    Si (noeud recoit un packet du noeud B){

     Verifier en tete{
        Si (incorrect) ignorer
         Sinon { // correct
            // Mise à jour table des voison
         }
      }
    */

    int vp = verif_paquet(reply);
    if (vp==0){
      choice = DEFAULT_NODE_STATE;
      printf("\t\t\t\tPaquet incorrect.\n");
      printf("\t\t\t\t======================================================================\n\n");
      goto again;
    }

    else { // Mise à jour table des voisins

      /*
        Rechercher l'emetteur dans la table des voisin
          Si (pas présent  && tableVoisin.length > 15 ) ignorer
          Sinon si (pas present && tableVoisin.length <= 15 ) {
            ajouter emetteur dans table voisin
            emetteur.booleen = transitoire
          }
          Si (emmetteur present){ // Rq : le cas où emetteur vient d'être ajouté marche
            update(emmeteur.date)
          }
         envoyer TVL network hash a l'adresse contenu dans TVL Neighbourg
      */

      memset(ipBuf,0,16);
      memset(portBuf,0,2);
      memcpy(ipBuf,&((&client)->sin6_addr),16);
      memcpy(portBuf,&((&client)->sin6_port),2);

      voisin* vs = get_voisin(voisins, ipBuf, portBuf);
      struct timeval tv;
      gettimeofday(&tv,NULL);

      if (vs==NULL && voisins->size<15){
        if (!debug && !tmp) printf("\t\t\t\tCe paquet provient d'un nouveau voisin ! Ajout dans la table des voisins.\n");
        vs = new_voisin(0,&tv,ipBuf,portBuf); // Transitoire
        list_add(voisins,vs,0);
      }

      else if (vs!=NULL){
        if (!debug && !tmp) printf("\t\t\t\tLe voisin existe déjà ! Mise à jour de la date.\n");
        memcpy(vs->date,&tv,sizeof(struct timeval));
      }

      else{ // Taille de la table >= 15
        printf("La table des voisins est pleine\n");
      }

      if (debug && !tmp) printf("=======================================================================\n");
      if (debug && !tmp) printf("VOISIN\n");
      if (debug && !tmp) printf("-----------------------------------------------------------------------------------------\n");
      if (debug && !tmp) printf("Permanent\t Date\t\t\t\t\t IP\t\t\t Port\n");
      if (debug && !tmp) printf("------------------------------------------------------------------------------------------------\n");
      if (debug && !tmp) print_list(voisins,(*pv));
      if (debug && !tmp) printf("------------------------------------------------------------------------------------------------\n");
      if (debug && !tmp) printf("=======================================================================\n");

      // =================================================================== //
      // Traitement du paquet reçu (Partie 2)
      // =================================================================== //

      choice = DEFAULT_NODE_STATE;
      if (!debug && !tmp) printf("\t\t\t\tSéquence TVL reçue : \n");
      uint16_t bodylength = ntohs( *((uint16_t*)(reply+2)) ) ;

      // Traitement TVL par TVL

      int i=4;
      while ( i<bodylength+4 ){
        donnee* don_i;
        tvl* t = read_tvl(reply+i);
        int tvl_length = t->length;
        uint8_t tvl_type = t->type;
        unsigned char* tvl_body = t->body;

        switch(tvl_type){

          default :

            break;

            // =================================================================== //
            // Case 2 : TVL Neighbourg Request
            // =================================================================== //

          case 2 : // TVL Neighbourg Request ======================================================== //
            /*
            Si (noeud recoit TLV Neighbourg Request) {
              voisin = tirer voisin hasard;
              envoyer (à l'emetteur du Neighbourg Request) un TLV Neighbourg contenant l'adresse de voisin
            }
            */
            printf("\t\t\t\t\t _ TVL Neighbour Request\n");
            random = rand_between(0,maybe_voisins->size);
            elem* elt = list_get_by_index(maybe_voisins,random);
            if (elt!=NULL){
              neighbour = (voisin*)elt->data;
              choice *= NEIGHBOURG;
            } else {
              choice *= DEFAULT_NODE_STATE;
            }
            break;

            // =================================================================== //
            // Case 3 : TVL Neighbourg
            // =================================================================== //

          case 3 : // TVL Neighbourg ======================================================== //

            if (!debug && !tmp) printf("\t\t\t\t\t _ TVL Neighbour\n");

            if (server==jch_server && boucle<=10){ // Cas n°1 : Initialisation (Trouver des pairs à contacter)

              memcpy(ipBuf,t->body,16);
              memcpy(portBuf,(t->body)+16,2);
              voisin* vs = get_voisin(maybe_voisins, ipBuf, portBuf);
              struct timeval tv;
              gettimeofday(&tv,NULL);

              if (vs==NULL && maybe_voisins->size<15){
                vs = new_voisin(1,&tv,ipBuf,portBuf);
                list_add(maybe_voisins,vs,0);
              }

              else if (vs!=NULL){
                memcpy(vs->date,&tv,sizeof(struct timeval));
              }

              else{
                printf("La table des voisins potentiels est pleine\n");
              }

              if (debug) printf("=======================================================================\n");
              if (debug) printf("PAIRS A CONTACTER (VOISINS POTENTIELS)\n");
              if (debug) printf("-----------------------------------------------------------------------------------------\n");
              if (debug) printf("Permanent\t Date\t\t\t\t\t IP\t\t\t Port\n");
              if (debug) printf("------------------------------------------------------------------------------------------------\n");
              if (debug) print_list(maybe_voisins,(*pv));
              if (debug) printf("------------------------------------------------------------------------------------------------\n");
              if (debug) printf("=======================================================================\n");

              if (vs!=NULL){
                unsigned char voisin_ip_buf[SIZE];
                unsigned char voisin_port_buf[SIZE];
                memset(voisin_ip_buf,0,SIZE);
                memset(voisin_port_buf,0,SIZE);
                get_voisin_ip(vs,voisin_ip_buf,SIZE);
                get_voisin_port(vs, voisin_port_buf,SIZE);
              }

              choice = DEFAULT_NODE_STATE;
              break;

            } // Fin Cas initialisation

            else { // Cas normal : On reçoit un TVL neighbour
              memset(ipBuf,0,16);
              memset(portBuf,0,2);
              memcpy(ipBuf,&((&client)->sin6_addr),16);
              memcpy(portBuf,&((&client)->sin6_port),2);
              voisin* pair = get_voisin(voisins, ipBuf, portBuf);
              //print_voisin(pair);
              unsigned char voisin_ip_buf[SIZE];
              unsigned char voisin_port_buf[SIZE];
              memset(voisin_ip_buf,0,SIZE);
              memset(voisin_port_buf,0,SIZE);
              get_voisin_ip(pair,voisin_ip_buf,SIZE);
              get_voisin_port(pair, voisin_port_buf,SIZE);
              printf("=======================================================================\n");
              printf("Répondre au pair après avoir reçu un TVL Neighbour : \n");
              printf("IP du pair : %s\n",voisin_ip_buf);
              printf("Port d'écoute du pair : %s\n",voisin_port_buf);
              struct sockaddr_in6 *pair_server = ip_from_addrinfo((char*)voisin_ip_buf,(char*)voisin_port_buf,NULL);
              server = pair_server;
              dontchange = 1;
              choice = NETWORK_HASH;
            }
            break;

            // =================================================================== //
            // Case 4 : TVL Network Hash
            // =================================================================== //

          case 4 : // TVL Network Hash

            printf("\t\t\t\t\t _ TVL Network Hash\n");
            H(donnees, networkhash);
            if (debug)
              print_hex_by_hex(networkhash,16);
            int identique = 1;

            for (int i=0; i<16; i++){
              if ( *(tvl_body+i)!=networkhash[i]){
                 identique = 0;
                 break;
               }
            }

            if (!identique){
              printf("\t\t\t\t ----> Network hash non-identique à celui calculé\n");

              memset(ipBuf,0,16);
              memset(portBuf,0,2);
              memcpy(ipBuf,&((&client)->sin6_addr),16);
              memcpy(portBuf,&((&client)->sin6_port),2);
              voisin* pair = get_voisin(voisins, ipBuf, portBuf);
              //print_voisin(pair);
              unsigned char voisin_ip_buf[SIZE];
              unsigned char voisin_port_buf[SIZE];
              memset(voisin_ip_buf,0,SIZE);
              memset(voisin_port_buf,0,SIZE);
              get_voisin_ip(pair,voisin_ip_buf,SIZE);
              get_voisin_port(pair, voisin_port_buf,SIZE);
              printf("=======================================================================\n");
              printf("Répondre au pair suite au TVL Network Hash reçu : \n");
              printf("IP du pair : %s\n",voisin_ip_buf);
              printf("Port d'écoute du pair : %s\n",voisin_port_buf);
              struct sockaddr_in6 *pair_server = ip_from_addrinfo((char*)voisin_ip_buf,(char*)voisin_port_buf,NULL);
              // Memory Leak ""normal"" (appel de getaddrinfo - Cf Mail du 7/05)
              server = pair_server;
              dontchange = 1;
              choice = NETWORK_STATE_REQ;
            }

            else {
              choice = DEFAULT_NODE_STATE;
              printf("\t\t\t\tLe Network hash est identique à celui calculé.\n");
            }

            break;

            // =================================================================== //
            // Case 5 : TVL Network State Request
            // =================================================================== //

          case 5 : // TVL Network State Request
            printf("\t\t\t\t\t _ TVL Network State Request\n");
            memset(ipBuf,0,16);
            memset(portBuf,0,2);
            memcpy(ipBuf,&((&client)->sin6_addr),16);
            memcpy(portBuf,&((&client)->sin6_port),2);
            voisin* pair = get_voisin(voisins, ipBuf, portBuf);
            //print_voisin(pair);
            unsigned char voisin_ip_buf[SIZE];
            unsigned char voisin_port_buf[SIZE];
            memset(voisin_ip_buf,0,SIZE);
            memset(voisin_port_buf,0,SIZE);
            get_voisin_ip(pair,voisin_ip_buf,SIZE);
            get_voisin_port(pair, voisin_port_buf,SIZE);
            printf("=======================================================================\n");
            printf("Répondre au pair suite au TVL Network State Request reçu : \n");
            printf("IP du pair : %s\n",voisin_ip_buf);
            printf("Port d'écoute du pair : %s\n",voisin_port_buf);
            struct sockaddr_in6 *pair_server = ip_from_addrinfo((char*)voisin_ip_buf,(char*)voisin_port_buf,NULL);
            // Memory Leak ""normal"" (appel de getaddrinfo - Cf Mail du 7/05)
            server = pair_server;
            dontchange = 1;
            choice = NODE_HASH;
            break;

            // =================================================================== //
            // Case 6 : TVL Node Hash
            // =================================================================== //

          case 6 : // TVL Node Hash
            printf("\t\t\t\t\t _ TVL Node Hash\n");
            don_i = get_donnee_by_id(donnees,tvl_body);
            if ( don_i==NULL ){
              unsigned char* str = malloc(8); // Memory leak
              memcpy(str,tvl_body,8);
              list_add(id_list,str,0);
              if (debug) printf("id_list->size = %d\n",id_list->size);
              choice *= NODE_STATE_REQ ; break;
            }
            else {
              unsigned char hashbuf_i[16];
              hi(don_i,(unsigned char*)hashbuf_i);
              if ( hash_equals( (unsigned char*)hashbuf_i, (unsigned char*)(tvl_body+NODE_ID_SIZE+SEQNO_SIZE) )){
                choice *= DEFAULT_NODE_STATE ; break;
              }
              else{
                unsigned char* str = malloc(8); // Memory leak
                memcpy(str,tvl_body,8);
                list_add(id_list,str,0);
                if (debug) printf("id_list->size = %d\n",id_list->size);
                choice *= NODE_STATE_REQ ; break;
              }
            }
            if (debug) printf("A faire : envoyer node state request : %d\n",i);

            // =================================================================== //
            // Case 7 : TVL Node State Request
            // =================================================================== //

          case 7 : // TVL Node State Request
            printf("\t\t\t\t\t _ TVL Node State Request\n");
            memcpy(nodeSTID,t->body,8);
            choice *= NODE_STATE; break;

            // =================================================================== //
            // Case 8 : TVL Node State
            // =================================================================== //

          case 8 : // TVL Node State

            printf("\t\t\t\t\t _ TVL Node State\n");

            donnee* don = new_donnee(tvl_body, ntohs( (uint16_t)(*(tvl_body+NODE_ID_SIZE) ) ) ,
                                  tvl_body+NODE_ID_SIZE+SEQNO_SIZE+HI_SIZE , tvl_length-HI_SIZE-NODE_ID_SIZE-SEQNO_SIZE );
            donnee* don_i = get_donnee_by_id(donnees,tvl_body);

            if ( don_i==NULL ){ // don n'existe pas dans la table
              list_add(donnees,don,0);
              printf("\t\t\t\tNouvelle donnée ajoutée (Nb données = %d): ", donnees->size);
              if (!debug && !tmp){
                print_donnee(don);
                printf("\n");
              }
            }

            else { // l'ID de don existe dans la table
              // Comparer les hash
              unsigned char hashBuf[16];
              hi(don_i,hashBuf);

              if ( hash_equals(hashBuf,tvl_body+NODE_ID_SIZE+SEQNO_SIZE) ){
                printf("\t\t\t\tDonnée déjà connue (Nb données = %d): ",donnees->size);
                if (!debug && !tmp){
                  print_donnee(don);
                  printf("\n");
                }
              }
              else {
                uint16_t seqNo2 = don->seqNo;
                uint16_t seqNo1 = don_i->seqNo;
                if ( id_equals(don->nodeID, nodeID) ){ // Notre propre ID
                  if (isInferior(seqNo1,seqNo2)){
                    (don_i->seqNo) = somme(seqNo2,1);
                  }
                }
                else if (isInferior(seqNo1,seqNo2)){ // ID different de nous
                  list_add(donnees,don,0);
                  printf("\t\t\t\tNouvelle donnée ajoutée (Nb données = %d): ", donnees->size);
                  if (!debug && !tmp){
                    print_donnee(don);
                    printf("\n");
                  }
                }

              }
            }
            //if (debug)
            if (debug) printf("------------------------------------------------------------------------------------------------\n");
            //if (debug)
            if (debug) printf("Node id\t\t\t Seqno\t\t\t\tData\n");
            //if (debug)
            if (debug) printf("------------------------------------------------------------------------------------------------\n");
            //if (debug)
            if (debug) print_list(donnees,(*pd));
            //if (debug)
            if (debug) printf("------------------------------------------------------------------------------------------------\n");
            //if (debug)
            if (debug) printf("\n\n\n");
            dontchange = 0;
            choice= DEFAULT_NODE_STATE; break;

          // =================================================================== //
          // Case 9 : TVL Warning
          // =================================================================== //

          case 9 : // TVL Warning
            print_tvl(t,1,debug);
            break;

        }

        i += 2+tvl_length;
        free(t);

      } // fin while i


    }

    //sleep(1);
    secondes = (secondes+1)%20;

    if (!debug && !tmp) printf("\t\t\t\t======================================================================\n\n");
    goto again;

    return 0;
}
