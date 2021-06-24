# include <stdlib.h>  // malloc, free
# include <stdio.h>
# include <string.h> // memcpy

# include "donnee.h"
# include "calculator.h"
# include "list.h"

donnee* new_donnee(unsigned char* id, uint16_t seqNo, unsigned char* text, int len){

  donnee* don = malloc(sizeof(donnee)); // MC00
  if (don==NULL) exit(1);
  don->nodeID = malloc(NODE_ID_SIZE); // MC01
  memcpy(don->nodeID,id,NODE_ID_SIZE);
  don->seqNo = seqNo;
  don->d_content = malloc(len); // MC02
  memcpy(don->d_content,text,len);
  don->d_content_len = len;
  return don;
}

void free_donnee(donnee* don){
  free(don->nodeID); // MC01
  free(don->d_content); // MC02
  free(don); // MC01
}

int id_equals(unsigned char* id1, unsigned char* id2){
  for (int i=0; i<8; i++){
    if ( *(id1+i) != *(id2+i) ) return 0;
  }
  return 1;
}

donnee* get_donnee_by_id(list* l, unsigned char* id){
  if (l==NULL) return NULL;
  elem* e1 = l->first;
  elem* e2 = l->last;
  for (int i=0; i<l->size; i++){
    if
    ( id_equals( ( ( (donnee*)(e1->data) )->nodeID ) , id ) ) return ( (donnee*)(e1->data) );
    if
    ( id_equals( ( ( (donnee*)(e2->data) )->nodeID ) , id ) ) return ( (donnee*)(e2->data) );
    e1 = e1->next;
    e2 = e2->prev;
  }
  return NULL;
}


void print_donnee(void* d){
  donnee* don = (donnee*)d;
  //printf("Donnee:\n");
  //print_octet_by_octet(don->nodeID,NODE_ID_SIZE);
  //printf("Node ID : %lu\t", *((uint64_t*)don->nodeID) );
  print_hex_by_hex(don->nodeID,NODE_ID_SIZE);
  //print_octet_by_octet( (unsigned char*)(&(don->seqNo)),SEQNO_SIZE);
  //print_octet_by_octet(don->d_content,don->d_content_len);

  //printf("don->nodeID :%lx\n",don->nodeID);
  printf("\t%u\t",don->seqNo);
  print_char_by_char(don->d_content,don->d_content_len);
  //printf("don->d_content : %lx\n",(unsigned char*)don->d_content);
}
