# include <netdb.h>   // uint64_t
# include <stdio.h>   // perror, printf ...
# include <string.h>  // memset, memcpy
# include <sys/time.h> // gettimeofday
# include <stdlib.h> // srand
# include <sys/time.h> // gettimeofday
# include <time.h> // localtime

# include "calculator.h"
# include "rfc6234/sha.h"
# include "donnee.h"
# include "list.h"

int rand_between(int a, int b){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  srand((unsigned)tv.tv_usec); // "graine" du rand() = time en usec
  int x = rand()%(b-a+1) + a;  // entre 0 et RAND_MAX
  return x;
}

void print_octet_by_octet(unsigned char* buf, int len){
  for (int i=0; i<len; i++){
    printf("|%d|",(int)(*(buf+i)));
  }
  printf("\n");
}

void print_hex_by_hex(unsigned char* buf, int len){
  for (int i=0; i<len; i++){
    printf("%02x",(uint8_t)(*(buf+i)));
  }
  //printf("\n");
}

void print_char_by_char(unsigned char* buf, int len){
  for (int i=0; i<len; i++){
    printf("%c",(unsigned char)(*(buf+i)));
  }
  printf("\n");
}

uint16_t somme(uint16_t s, uint16_t n){
  return (s+n)&65535;
}

int isInferior(uint16_t s, uint16_t n){ // s < n
  return ( ( n - s ) & 32768 ) == 0 ;
}

void print_sha256(unsigned char* hash, int hashsize){
  for (int i = 0; i < hashsize; ++i) {
    putchar(hexdigits[(hash[i] >> 4) & 0xF]);
    putchar(hexdigits[hash[i] & 0xF]);
  }
  putchar('\n');
}

unsigned char* sha256(unsigned char* src, int srclen, unsigned char* hash){

    //static const char hexdigits[ ] = "0123456789abcdef";
    SHA256Context ctx;
    int rc;
    //memset(src,0,srclen);
    //unsigned char hash[hashsize];

    rc = SHA256Reset(&ctx);
    if(rc < 0){
      perror("SHA256Reset");
      return NULL;
    }

    rc = SHA256Input(&ctx, src,srclen);
    if(rc < 0){
      perror("SHA256Input");
      return NULL;
    }

    rc = SHA256Result(&ctx,hash);
    if(rc != 0){
      perror("SHA256Result");
      return NULL;
    }

    //printf("SHA : %s\n",src);
    //print_sha256(hash,hashsize);
    return hash;
}

void print_h(unsigned char* hash){
  print_sha256(hash,16);
}

unsigned char* h(unsigned char* src, int srclen, unsigned char hash[16]){
  unsigned char fullhash[HASHSIZE];
  sha256(src,srclen,fullhash);
  hash = memcpy(hash,fullhash,16);
  //print_h(hash);
  return hash;
}



unsigned char* hi(donnee* don, unsigned char hash[16]){
  // Conversion entier 16 bits -> Big Endian
  // On suppose size of donbuf == NODE_ID_SIZE+SEQNO_SIZE+d_contentSize
  uint16_t res = htons(don->seqNo);
  //printf("res hi: ");
  //print_octet_by_octet((unsigned char*)&res,2);
  memset(hash,0,16);
  // Concatenation
  int d_contentSize = don->d_content_len;

  //printf("d_cont hi: ");
  //print_octet_by_octet(don->d_content,d_contentSize);
  //print_char_by_char(don->d_content,d_contentSize);

  unsigned char donbuf[d_contentSize+NODE_ID_SIZE+SEQNO_SIZE];
  memcpy(donbuf,(don->nodeID),NODE_ID_SIZE);
  memcpy(donbuf+NODE_ID_SIZE,&res,SEQNO_SIZE);
  memcpy(donbuf+NODE_ID_SIZE+SEQNO_SIZE, (don->d_content), d_contentSize );

  //printf("Concatenation hi: ");
  //print_octet_by_octet(donbuf,d_contentSize+NODE_ID_SIZE+SEQNO_SIZE);
  //printf("Concatenation hi: ");
  //print_char_by_char(donbuf,d_contentSize+NODE_ID_SIZE+SEQNO_SIZE);

  hash = h(donbuf,NODE_ID_SIZE+SEQNO_SIZE+d_contentSize,hash);
  //printf("hi: ");
  //print_sha256(hash,16);
  //print_octet_by_octet(hash,16);
  //print_char_by_char(hash,16);
  return hash;

}


int network_size(list* l){
  // On suppose l liste de donnee
  // Calcul de la taille totale des données
  int res = 0;
  for (elem* e = l->first; e!=NULL; e=e->next ){
    struct donnee* ptDon = (struct donnee*)(e->data);
    res+=NODE_ID_SIZE;
    res+=SEQNO_SIZE;
    res+=sizeof( *(ptDon->d_content) );
  }
  return res;
}

unsigned char* H(list* l, unsigned char hash[16]){
    // On suppose l liste de donnee
    // On suppose l triée
    // On suppose buf de taille network_size

    //int maxdonsize = NODE_ID_SIZE+SEQNO_SIZE+D_CONTENT_SIZE_MAX;
    //unsigned char* hibuf[maxdonsize];
    //int size = network_size(l);
    unsigned char netbuf[16*(l->size)];
    int pos = 0;
    for (elem* e = l->first; e!=NULL; e=e->next ){
      struct donnee* ptDon = (struct donnee*)(e->data);
      //int donsize = NODE_ID_SIZE+SEQNO_SIZE+sizeof( *(ptDon->d_content) );
      //unsigned char donbuf[donsize];
      hash = hi(ptDon,hash);
      memcpy(netbuf+pos,hash,16);
      pos+=16;
    }
    hash = h(netbuf,16*(l->size),hash);
    return hash;
}

int hash_equals(unsigned char* hash1, unsigned char* hash2){
  for (int i=0; i<16; i++){
    if ( *(hash1+i) != *(hash2+i) ) return 0;
  }
  return 1;
}

void print_timeval(struct timeval tv){

  char* s = malloc(sizeof(char)*1024);
  struct tm* lt = localtime((const time_t*)&tv.tv_sec);
  strftime((char*)s, 1024,"%d/%m/%y %T",lt);
  //printf("%s (µsec : %06ld)\n",s,tv.tv_usec);
  printf("%s",s);
  free(s);

}
