# include <string.h>  // memcpy
# include <stdio.h>   // printf
# include <stdlib.h>  // malloc, free

# include "paquet.h"
# include "list.h"
# include "tvl.h"

uint16_t write_paquet(unsigned char *buf, list* l){

  uint8_t magic = 95;
  memcpy(buf,&magic,1);
  uint8_t version = 1 ;
  memcpy(buf+1,&version,1);

  uint16_t bodylength = 0;
  tvl* tv;

  for (elem* e = l->first; e!=NULL; e=e->next ){
    tv = ( (tvl*)(e->data) ) ;
    int tvl_length =  2 + tv -> length;
    write_tvl(tv,buf+4+bodylength);
    bodylength += tvl_length ;
  }

  uint16_t bl = htons(bodylength);
  memcpy(buf+2,&bl,2);
  return bodylength+4;

}

void print_paquet(unsigned char *buf, int response, int debug){

  uint16_t bodylength = ntohs( *((uint16_t*)(buf+2)) ) ;

  if (!response){
    if (debug) printf("Magic %d\n",*((uint8_t*)(buf)));
    if (debug) printf("Version %d\n",*((uint8_t*)(buf+1)));
    if (debug) printf("Body Length %d\n", bodylength );
  }
  else {
    if (debug) printf("\t\tMagic %d\n",*((uint8_t*)(buf)));
    if (debug) printf("\t\tVersion %d\n",*((uint8_t*)(buf+1)));
    if (debug) printf("\t\tBody Length %d\n", bodylength );
  }

  int i=4;
  while ( i<bodylength+4 ){
    tvl* t = read_tvl(buf+i);
    int tvl_length = t->length;
    print_tvl(t,response,debug);
    i += 2+tvl_length;
    free(t);
  }

  //print_octet_by_octet(buf,bodylength+4);

}


int verif_paquet(unsigned char *buf){

  uint8_t magic = *((uint8_t*)(buf));
  if (magic!=95) return 0;
  uint8_t version = *((uint8_t*)(buf+1));
  if (version!=1) return 0;
  uint16_t bodylength = ntohs( *((uint16_t*)(buf+2)) ) ;
  if (bodylength<=2) return 0;
  return 1;

}
