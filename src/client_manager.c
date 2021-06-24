# include <netdb.h>     // getaddrinfo, hints
# include <errno.h>     // errno
# include <stdio.h>     // printf
# include <stdlib.h>    // exit
# include <string.h>    // memset, memcpy
# include <arpa/inet.h> // inet_ntop
# include <fcntl.h>     // fcntl
# include <sys/time.h> // gettimeofday

# include "client_manager.h"
# include "paquet.h"
# include "list.h"
# include "voisin.h"
// Rendre polymorphe la socket

void set_polymorph(int s){

  int val = 0; // 0 : polymorphe , 1 : monomorphe
  int rc = setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val));
  if (rc==-1){
    printf("Error polymorph : %s ... :-( \n",strerror(errno));
    exit(EXIT_FAILURE);
  }

}

// Mettre la socket en non-bloquant

void set_nonblock(int s){

  int rc = fcntl(s, F_GETFL);
  if (rc<0){
    printf("Error nonblock : %s ... :-( \n",strerror(errno));
    exit(EXIT_FAILURE);
  }
  rc = fcntl(s, F_SETFL, rc | O_NONBLOCK);
  if (rc<0){
    printf("Error nonblock : %s ... :-( \n",strerror(errno));
    exit(EXIT_FAILURE);
  }

}

// Fonction pour modifier les options de socket : Réutilisation

void set_reusable(int s){
  int val = 1; // 1 : autorise plusieurs connexions successives
  int rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
  if (rc==-1){
    printf("Error reusable : %s ... :-( \n",strerror(errno));
    exit(EXIT_FAILURE);
  }
}

// Socket UDP polymorphe

int my_udp_socket(){

  int s = socket(AF_INET6, SOCK_DGRAM, 0); // socket UDP
  if (s < 0){
    printf("Error socket udp : %s ... :-( \n",strerror(errno));
    exit(EXIT_FAILURE);
  }
  set_polymorph(s); // Polymorphe : le serveur peut accepter ipv6 et ipv4
  set_nonblock(s); // Non-bloquant
  set_reusable(s);
  return s;

}


//  Créer un adressse IPv6 à partir du port

struct sockaddr_in6 my_ipv6(int port,const char *node, char* ip){

  struct sockaddr_in6 sin6;
  memset(&sin6,0,sizeof(sin6));
  sin6.sin6_family = AF_INET6;
  sin6.sin6_port = htons(port);
  inet_pton(AF_INET6,node,&(sin6.sin6_addr));
  return sin6;

}


struct sockaddr_in6 my_ipv6server(int port){
  struct sockaddr_in6 server;
  memset(&server, 0, sizeof(server));
  server.sin6_family = AF_INET6;
  server.sin6_port = htons(port);
  return server;
}

void my_bind(int s, struct sockaddr_in6 server){
  int rc = bind(s, (struct sockaddr*)&server, sizeof(server));
  if (rc==-1){
    printf("Error bind ...%s\n",strerror(errno));
    exit(EXIT_FAILURE);
  }
}

// Creer le hint pour getaddrinfo

struct addrinfo my_hints(){

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_ALL | AI_V4MAPPED;
  return hints;

}

// Vérifications pour getaddrinfo [ Memory Leak ""normal"" (appel de getaddrinfo) ]

int my_getaddrinfo(const char *node, const char *service,
                   const struct addrinfo *hints,struct addrinfo **res){

  /* Memory leak ICI "normal" - Bug systeme (Cf : Mail du 7/05) */
  /* -------> */ int rc = getaddrinfo(node,service,hints,res); /* <--------- */
  /******************/

  if(rc != 0) {
    printf("Le serveur n'a pas été trouvé pour l\'hôte '%s'.\n",node);
    //printf("Etes-vous sûr que l\'hostname '%s' est correct ?\n",node);
    printf("Etes-vous bien connecté à internet ?\n");
    exit(1);
  }
  return rc;

}

// Se connecte au premier IP à partir de l'hostname
// Enregistre l'ip dans une liste des voisins potentiels

struct sockaddr_in6* ip_from_addrinfo(char* server_name, char* server_port, list* voisins){

    struct addrinfo hints = my_hints();
    struct addrinfo* serv_info;
    my_getaddrinfo(server_name, server_port, &hints, &serv_info);

    char buf[INET_ADDRSTRLEN + 1];
    char buf6[INET6_ADDRSTRLEN + 1];
    unsigned char *addr, *addr_vois;

    if (serv_info->ai_family == AF_INET) {
      addr = (unsigned char*)inet_ntop(AF_INET, &(((struct sockaddr_in *)serv_info->ai_addr)->sin_addr), buf, sizeof(buf) );
    }
    else if (serv_info->ai_family == AF_INET6) {
      addr = (unsigned char*)inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)serv_info->ai_addr)->sin6_addr), buf6, sizeof(buf6));
    }

    if (addr == NULL) {
        printf("Erreur IP\n");
        exit(1);
    }

    // ================================================================= //

    for (struct addrinfo* e = serv_info ; e != NULL; e = e->ai_next){

      struct timeval tv;
      gettimeofday(&tv,NULL);
      voisin* v;

      if (e->ai_family == AF_INET) {
        addr_vois = (unsigned char*)inet_ntop(AF_INET, &(((struct sockaddr_in *)e->ai_addr)->sin_addr), buf, sizeof(buf) );
        printf("IPv4: %s\n", addr_vois);
        print_octet_by_octet((unsigned char*)&(((struct sockaddr_in6 *)e->ai_addr)->sin6_addr),16);
        printf("Erreur IP4 not mapped\n");
        exit(1);
      }

      else if (e->ai_family == AF_INET6) {
        addr_vois = (unsigned char*)inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)e->ai_addr)->sin6_addr), buf6, sizeof(buf6));
        //printf("IPv6: %s\n", addr_vois);
        //print_octet_by_octet((unsigned char*)&(((struct sockaddr_in6 *)e->ai_addr)->sin6_addr),16);
        struct in6_addr* ip6_vois = &(((struct sockaddr_in6 *)e->ai_addr)->sin6_addr);
        uint16_t port_vois = ( (((struct sockaddr_in6 *)e->ai_addr)->sin6_port) );
        //printf("pport : %hd\n", port_vois);
        //print_octet_by_octet( (unsigned char*)(&port_vois),2);
        if (voisins!=NULL) v = new_voisin(1,&tv,(unsigned char*)ip6_vois,(unsigned char*)(&port_vois));
      }

      //print_voisin(v);
      if (voisins!=NULL) list_add(voisins,v,0);
      //printf("list len :%d\n",voisins->size);
    }

    // ================================================================= //
    //printf("\nCONNEXION AU SERVEUR REUSSIE !\n");
    //printf("Hostname: %s\tIPv6/IPv4-Mapped: %s\tPort: %s\n", server_name, addr,server_port);
    return ((struct sockaddr_in6*)(serv_info->ai_addr));
}

/**********************************/

// Constuire la requete

void print_datagram(unsigned char* req, int reqlen, int response, int debug){

  if (response){
    printf("\n\t\t____________________________\n\t\tREPONSE : \t\n");
  }
  else {
    printf("\n____________________________\nREQUETE : \t\n");
  }

  print_paquet(req,response,debug);
  if (!response){
    printf("____________________________\n\n");
  }
  else {
    printf("\t\t____________________________\n\n");
  }

}

void print_client(struct sockaddr* client){

  char buf[INET_ADDRSTRLEN + 1];
  char buf6[INET6_ADDRSTRLEN + 1];
  //char bufPort[SIZE];
  unsigned char *addr;

  if (client->sa_family == AF_INET) {
    addr = (unsigned char*)inet_ntop(AF_INET, &(((struct sockaddr_in *)client)->sin_addr), buf, sizeof(buf) );
  }
  else if (client->sa_family == AF_INET6) {
    addr = (unsigned char*)inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)client)->sin6_addr), buf6, sizeof(buf6));
  }

  printf("%s   ", addr);
  printf("\t%d\n", ntohs( (uint16_t)((((struct sockaddr_in6 *)client)->sin6_port)) ));

}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
