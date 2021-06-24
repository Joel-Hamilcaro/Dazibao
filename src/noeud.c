# include <stdio.h> // printf
# include <stdlib.h> // srand
# include <netdb.h> // uint64_t
# include <time.h> // time
# include <string.h>    // memset, memcpy
# include <sys/time.h> // gettimeofday

# include "noeud.h"

uint64_t newNodeID(){

  struct timeval tv;
  gettimeofday(&tv,NULL);
  srand((unsigned)tv.tv_sec); // "graine" du rand() = time en sec
  uint64_t x = (unsigned int)rand();  // entre 0 et RAND_MAX
  //printf("x=%llx\n",x);
  srand((unsigned)tv.tv_usec); // "graine" du rand() = time en µsec
  uint64_t y = (unsigned int)rand(); // entre 0 et RAND_MAX
  //printf("y=%llx\n",y);
  uint64_t res = y << 32; // Decaller y dans les 32 premiers bits
  //printf("res=%llx\n",res);
  res = res | x/5768;
  //printf("Res=%llx\n",res);
  // On divise x par 5768 (on choisi ce nombre arbitrairement)
  // Pour augmenter les chances que l'ID soit unique.
  return res;
}

void init_id(unsigned char* nodeID){
  uint64_t id = newNodeID();
  memcpy(nodeID,&id,8);
}
