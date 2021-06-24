# include <netdb.h>   // uint64_t
# include <stdio.h>   // perror, printf ...
# include <string.h>  // memset
# include <stdlib.h>  // malloc, free
# include <arpa/inet.h> // htons, ntohs ...
# include "tvl.h"


tvl* new_tvl(uint8_t type, uint8_t length, unsigned char* body){
  tvl* t = malloc(sizeof(tvl));
  if (t==NULL) return NULL;
  t->type=type;
  t->length=length;
  t->body=body;
  return t;
}

void write_tvl(tvl* t, unsigned char* tvlbuf){
  unsigned char type = (t->type);
  unsigned char length = (t->length);
  memcpy(tvlbuf,&type,1);
  memcpy(tvlbuf+1,&length,1);
  memcpy(tvlbuf+2,t->body,t->length);
}

tvl* read_tvl(unsigned char* tvlbuf){
  uint8_t type = ( (tvlbuf[0]) );
  uint8_t length = ( (tvlbuf[1]) );
  if (length==0) return new_tvl( type ,  length , NULL );
  return new_tvl( type ,  length , tvlbuf+2 );
}

// 0

tvl* new_tvl_pad1(){
  return new_tvl(0,0,NULL);
}

// 1

tvl* new_tvl_padN(int n){
  unsigned char* body = malloc(n);
  if (body==NULL) return new_tvl_pad1();
  memset(body,0,n);
  return new_tvl(1,n,body);
}

// 2

tvl* new_tvl_neighbour_req(){
  return new_tvl(2,0,NULL);
}

// Notation IPv6  a::b::c::d::e::f::g::h (chaque groupe = 2o, soit 16o)
// Notation IPv4  a.b.c.d (chaque groupe = 1o, soit 4o)
// Notation IPv6 mapped  ::ffff:a.b.c.d (10o de zéros + 2o de 1 + 4o de l'IPv4, soit 16o)


// 3
tvl* new_tvl_neighbour(unsigned char* ip,unsigned char* port, unsigned char* buf){
  memcpy(buf,ip,16);
  memcpy(buf+16,port,2);
  return new_tvl(3,18,buf);
}

// 4
tvl* new_tvl_network_hash(unsigned char* networkhash){
  return new_tvl(4,16,networkhash);
}

// 5
tvl* new_tvl_network_state_req(){
  return new_tvl(5,0,NULL);
}

// 6

tvl* new_tvl_node_hash(donnee* don,unsigned char* buf){

  unsigned char hash[16];
  hi(don,hash);
  memcpy(buf,&(don->nodeID),NODE_ID_SIZE);
  memcpy(buf+NODE_ID_SIZE,&(don->seqNo),SEQNO_SIZE);
  memcpy(buf+NODE_ID_SIZE+SEQNO_SIZE,hash,16);
  return new_tvl(6,NODE_ID_SIZE+SEQNO_SIZE+16,buf);

}

// 7

tvl* new_tvl_node_state_req(unsigned char* nodeID){
  return new_tvl(7,NODE_ID_SIZE,(unsigned char*)nodeID);
}

// 8

tvl* new_tvl_node_state(donnee* don,unsigned char* buf){
  unsigned char hash[16];
  memset(hash,0,16);
  uint16_t res = htons(don->seqNo);
  hi(don,hash);
  //printf("res in tvl : %d\n",res);
  //print_octet_by_octet((unsigned char*)&res,2);
  int d_contentSize = don->d_content_len;
  memcpy(buf,(don->nodeID),NODE_ID_SIZE);
  memcpy(buf+NODE_ID_SIZE,&(res),SEQNO_SIZE);
  memcpy(buf+NODE_ID_SIZE+SEQNO_SIZE,hash,16);
  memcpy(buf+NODE_ID_SIZE+SEQNO_SIZE+16,don->d_content,d_contentSize);
  //printf("Concatenation tvl: ");
  //print_octet_by_octet(buf,d_contentSize+NODE_ID_SIZE+SEQNO_SIZE+16);
  //printf("Concatenation tvl: ");
  //print_char_by_char(buf,d_contentSize+NODE_ID_SIZE+SEQNO_SIZE+16);
  return new_tvl(8,NODE_ID_SIZE+SEQNO_SIZE+16+d_contentSize,buf);
}

// 9

tvl* new_tvl_warning(unsigned char* msg, int len){
  return new_tvl(9,len,msg);
}

//

void print_tvl(tvl* tvl, int response, int debug){

  if (!response){
    if (tvl->type==0){
      printf("-------\nTVL Pad1: \n");
      if (debug) printf("tvl type: %d\n",tvl->type);
      if (debug) printf("tvl length: %d\n",tvl->length);
      if (debug) printf("tvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("-------\n");
    }
    else if (tvl->type==1){
      printf("-------\nTVL PadN: \n");
      if (debug) printf("tvl type: %d\n",tvl->type);
      if (debug) printf("tvl length: %d\n",tvl->length);
      if (debug) printf("tvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("-------\n");
    }
    else if (tvl->type==2){
      printf("-------\nTVL Neighbourg Request: \n");
      if (debug) printf("tvl type: %d\n",tvl->type);
      if (debug) printf("tvl length: %d\n",tvl->length);
      if (debug) printf("tvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("-------\n");
    }
    else if (tvl->type==3){
      printf("-------\nTVL Neighbour: \n");
      if (debug) printf("tvl type: %d\n",tvl->type);
      if (debug) printf("tvl length: %d\n",tvl->length);
      if (debug) printf("tvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else{
        char buf6[INET6_ADDRSTRLEN + 1];
        unsigned char *addr;
        addr = (unsigned char*)inet_ntop(AF_INET6, tvl->body, buf6, sizeof(buf6));
        if (debug) printf("IP : %39s\t\t\t\t",addr);
        uint16_t* port = (uint16_t*)(tvl->body+16);
        if (debug) printf("%hu\n",ntohs(*port));
        //print_octet_by_octet((tvl->body),(tvl->length));
       }
      printf("-------\n");
    }
    else if (tvl->type==4){
      printf("-------\nTVL Network Hash: \n");
      if (debug) printf("tvl type: %d\n",tvl->type);
      if (debug) printf("tvl length: %d\n",tvl->length);
      if (debug) printf("tvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("-------\n");
    }
    else if (tvl->type==5){
      printf("-------\nTVL Network state request: \n");
      if (debug) printf("tvl type: %d\n",tvl->type);
      if (debug) printf("tvl length: %d\n",tvl->length);
      if (debug) printf("tvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("-------\n");
    }
    else if (tvl->type==6){
      printf("-------\nTVL Node Hash: \n");
      if (debug) printf("tvl type: %d\n",tvl->type);
      if (debug) printf("tvl length: %d\n",tvl->length);
      if (debug) printf("tvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else{
        if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      }
      printf("-------\n");
    }
    else if (tvl->type==7){
      printf("-------\nTVL Node State request: \n");
      if (debug) printf("tvl type: %d\n",tvl->type);
      if (debug) printf("tvl length: %d\n",tvl->length);
      if (debug) printf("tvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("-------\n");
    }
    else if (tvl->type==8){
      printf("-------\nTVL Node state: \n");
      if (debug) printf("tvl type: %d\n",tvl->type);
      if (debug) printf("tvl length: %d\n",tvl->length);
      if (debug) printf("tvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else{
        if (debug) print_hex_by_hex((tvl->body),(NODE_ID_SIZE));
        if (debug) printf("\t%hu\t", ntohs( *( (uint16_t*)(tvl->body+NODE_ID_SIZE))) );
        if (debug) print_hex_by_hex((tvl->body+NODE_ID_SIZE+SEQNO_SIZE),(HI_SIZE));
        if (debug) printf("\t");
        if (debug) print_char_by_char(tvl->body+NODE_ID_SIZE+SEQNO_SIZE+HI_SIZE,tvl->length-HI_SIZE-NODE_ID_SIZE-SEQNO_SIZE);
        //print_octet_by_octet((tvl->body),(tvl->length));
      }
      printf("-------\n");
    }
    else if (tvl->type==9){
      printf("-------\nTVL Warning: \n");
      if (debug) printf("tvl type: %d\n",tvl->type);
      if (debug) printf("tvl length: %d\n",tvl->length);
      printf("tvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else print_char_by_char((tvl->body),(tvl->length));
      printf("-------\n");
    }

  }
  else {
    if (tvl->type==0){
      printf("\t\t\t\t-------\n\t\t\t\tTVL Pad1 : \n");
      if (debug) printf("\t\t\t\ttvl type: %d\n",tvl->type);
      if (debug) printf("\t\t\t\ttvl length: %d\n",tvl->length);
      if (debug) printf("\t\t\t\ttvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("\t\t\t\t-------\n");
    }
    else if (tvl->type==1){
      printf("\t\t\t\t-------\n\t\t\t\tTVL PadN: \n");
      if (debug) printf("\t\t\t\ttvl type: %d\n",tvl->type);
      if (debug) printf("\t\t\t\ttvl length: %d\n",tvl->length);
      if (debug) printf("\t\t\t\ttvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("\t\t\t\t-------\n");
    }
    else if (tvl->type==2){
      printf("\t\t\t\t-------\n\t\t\t\tTVL Neighbourg Request: \n");
      if (debug) printf("\t\t\t\ttvl type: %d\n",tvl->type);
      if (debug) printf("\t\t\t\ttvl length: %d\n",tvl->length);
      if (debug) printf("\t\t\t\ttvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("\t\t\t\t-------\n");
    }
    else if (tvl->type==3){
      printf("\t\t\t\t-------\n\t\t\t\tTVL Neighbour: \n");
      if (debug) printf("\t\t\t\ttvl type: %d\n",tvl->type);
      if (debug) printf("\t\t\t\ttvl length: %d\n",tvl->length);
      if (debug) printf("\t\t\t\ttvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else{
        char buf6[INET6_ADDRSTRLEN + 1];
        unsigned char *addr;
        addr = (unsigned char*)inet_ntop(AF_INET6, tvl->body, buf6, sizeof(buf6));
        if (debug) printf("\t\t\t\tIP : %39s\t\t\t\t",addr);
        uint16_t* port = (uint16_t*)(tvl->body+16);
        if (debug) printf("%hu\n",ntohs(*port));
        //print_octet_by_octet((tvl->body),(tvl->length));
       }
      printf("\t\t\t\t-------\n");
    }
    else if (tvl->type==4){
      printf("\t\t\t\t-------\n\t\t\t\tTVL Network Hash: \n");
      if (debug) printf("\t\t\t\ttvl type: %d\n",tvl->type);
      if (debug) printf("\t\t\t\ttvl length: %d\n",tvl->length);
      if (debug) printf("\t\t\t\ttvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("\t\t\t\t-------\n");
    }
    else if (tvl->type==5){
      printf("\t\t\t\t-------\n\t\t\t\tTVL Network state request: \n");
      if (debug) printf("\t\t\t\ttvl type: %d\n",tvl->type);
      if (debug) printf("\t\t\t\ttvl length: %d\n",tvl->length);
      if (debug) printf("\t\t\t\ttvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("\t\t\t\t-------\n");
    }
    else if (tvl->type==6){
      printf("\t\t\t\t-------\n\t\t\t\tTVL Node Hash: \n");
      if (debug) printf("\t\t\t\ttvl type: %d\n",tvl->type);
      if (debug) printf("\t\t\t\ttvl length: %d\n",tvl->length);
      if (debug) printf("\t\t\t\ttvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("\t\t\t\t-------\n");
    }
    else if (tvl->type==7){
      printf("\t\t\t\t-------\n\t\t\t\tTVL Node State request: \n");
      if (debug) printf("\t\t\t\ttvl type: %d\n",tvl->type);
      if (debug) printf("\t\t\t\ttvl length: %d\n",tvl->length);
      if (debug) printf("\t\t\t\ttvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else if (debug) print_octet_by_octet((tvl->body),(tvl->length));
      printf("\t\t\t\t-------\n");
    }
    else if (tvl->type==8){
      printf("\t\t\t\t-------\n\t\t\t\tTVL Node state: \n");
      if (debug) printf("\t\t\t\ttvl type: %d\n",tvl->type);
      if (debug) printf("\t\t\t\ttvl length: %d\n",tvl->length);
      if (debug) printf("\t\t\t\ttvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else{
        if (debug) print_hex_by_hex((tvl->body),(NODE_ID_SIZE));
        if (debug) printf("\t%hu\t", ntohs( *( (uint16_t*)(tvl->body+NODE_ID_SIZE))) );
        if (debug) print_hex_by_hex((tvl->body+NODE_ID_SIZE+SEQNO_SIZE),(HI_SIZE));
        if (debug) printf("\t");
        if (debug) print_char_by_char(tvl->body+NODE_ID_SIZE+SEQNO_SIZE+HI_SIZE,tvl->length-HI_SIZE-NODE_ID_SIZE-SEQNO_SIZE);
        //print_octet_by_octet((tvl->body),(tvl->length));
      }
      printf("\t\t\t\t-------\n");
    }
    else if (tvl->type==9){
      printf("\t\t\t\t-------\n\t\t\t\tTVL Warning: \n");
      if (debug) printf("\t\t\t\ttvl type: %d\n",tvl->type);
      if (debug) printf("\t\t\t\ttvl length: %d\n",tvl->length);
      printf("\t\t\t\ttvl body: ");
      if (tvl->body==NULL){ if (debug) printf("NULL\n"); }
      else print_char_by_char((tvl->body),(tvl->length));
      printf("\t\t\t\t-------\n");
    }

  }
}
