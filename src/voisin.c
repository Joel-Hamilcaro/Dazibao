# include <sys/time.h> // timeval
# include <stdlib.h>    // malloc
# include <time.h> // localtime
# include <stdio.h> // printf
# include <sys/time.h> // gettimeofday
# include <errno.h>     // errno
# include <string.h> // memcpy
# include <arpa/inet.h> // inet_ntop
# include "voisin.h"
# include "list.h"
# include "calculator.h"


voisin* new_voisin(int permanent, struct timeval* date, unsigned char* ip, unsigned char* port){
  voisin* v = malloc(sizeof(voisin));
  if (v==NULL) exit(1);
  v->permanent = permanent;
  v->date = malloc(sizeof(struct timeval));
  if (v->date==NULL) exit(1);
  memcpy(v->date,date,sizeof(struct timeval));
  v->ip = malloc(16);
  memcpy(v->ip,ip,16);
  v->port = malloc(2);
  memcpy(v->port,port,2);
  return v;
}

void free_voisin(voisin* v){
  free(v->date);
  free(v->ip);
  free(v->port);
  free(v);
}


int ip_equals(unsigned char* ip1, unsigned char* ip2){
  for (int i=0; i<16; i++){
    if ( *(ip1+i) != *(ip2+i) ) return 0;
  }
  return 1;
}

voisin* get_voisin(list* l, unsigned char* ip, unsigned char* port){
  if (l==NULL) return NULL;
  elem* e1 = l->first;
  elem* e2 = l->last;
  for (int i=0; i<l->size; i++){
    if
    ( ip_equals( ( ( (voisin*)(e1->data) )->ip ) , ip )
      &&
      ( (uint16_t)( *( ( (voisin*)(e1->data) )->port ) ) == (uint16_t)(*port) )
    ) return ( (voisin*)(e1->data) );
    if
    ( ip_equals( ( ( (voisin*)(e2->data) )->ip ) , ip )
      &&
      ( (uint16_t)( *( ( (voisin*)(e2->data) )->port ) ) == (uint16_t)(*port))
    ) return ( (voisin*)(e2->data) );
    e1 = e1->next;
    e2 = e2->prev;
  }
  return NULL;
}


voisin* get_voisin_by_ip_only(list* l, unsigned char* ip){
  if (l==NULL) return NULL;
  elem* e1 = l->first;
  elem* e2 = l->last;
  for (int i=0; i<l->size; i++){
    if
    ( ip_equals( ( ( (voisin*)(e1->data) )->ip ) , ip )

    ) return ( (voisin*)(e1->data) );
    if
    ( ip_equals( ( ( (voisin*)(e2->data) )->ip ) , ip )
    ) return ( (voisin*)(e2->data) );
    e1 = e1->next;
    e2 = e2->prev;
  }
  return NULL;
}


void print_voisin(void* d){
  voisin* v = (voisin*)d;
  printf("------------------------------------------------------------------------------------------------\n");

  if (v==NULL){
    printf("NULL\n");
    return;
  }

  if (v->permanent){
    printf("OUI\t");
  }
  else{
    printf("NON\t");
  }

  if ( (v->date)!=NULL) print_timeval( *(v->date) ) ;
  else printf("no date");
  printf("\t");

  if ((v->ip)!=NULL){

    char buf6[INET6_ADDRSTRLEN + 1];
    unsigned char *addr;
    addr = (unsigned char*)inet_ntop(AF_INET6, v->ip, buf6, sizeof(buf6));
    printf("%39s\t\t",addr);
    //print_octet_by_octet(v->ip,16);
  }

  if ((v->port)!=NULL){
    uint16_t* port = (uint16_t*)(v->port);
    printf("%hu\n",ntohs(*port));
    //printf("Port : %hu\n",(*port));
    //printf("Port htons: %hu\n",htons(*port));
    //print_octet_by_octet(v->port,2);
  }
  //printf("--------------------\n");
}

unsigned char* get_voisin_ip(voisin* v, unsigned char* buf, int len){

  if ((v->ip)!=NULL){
    // unsigned char *addr;
    (unsigned char*)inet_ntop(AF_INET6, v->ip, (char*)buf, len);
    //printf("%39s\t\t",addr);
    return buf;
    //print_octet_by_octet(v->ip,16);
  }
  return NULL;
}

unsigned char* get_voisin_port(voisin* v, unsigned char* buf, int len){

    if ((v->port)!=NULL){

      uint16_t* port = (uint16_t*)(v->port);
      sprintf((char*)buf, "%hu" ,ntohs(*port));
      //printf("str : %s\n",str);
      return buf;
      //printf("Port : %hu\n",(*port));
      //printf("Port htons: %hu\n",htons(*port));
      //print_octet_by_octet(v->port,2);

    }

    return NULL;
}
