/*************************************************************************/
# ifndef DONNEE_H
# define DONNEE_H
/*************************************************************************/

# include <netdb.h> // uint64_t
# include "list.h"

# define D_CONTENT_SIZE_MAX 192
# define NODE_ID_SIZE 8
# define SEQNO_SIZE 2

typedef struct donnee donnee;

struct donnee {
  unsigned char* nodeID; // ID du noeud qui a envoyé la donnée
  uint16_t seqNo; // Numero de séquence
  unsigned char* d_content; // Contenu de la donnée (0 à 192 octets)
  int d_content_len;
};

donnee* new_donnee(unsigned char* id, uint16_t seqNo, unsigned char* text, int len);
void print_donnee(void* d);
int id_equals(unsigned char* id1, unsigned char* id2);
donnee* get_donnee_by_id(list* l, unsigned char* id);
void free_donnee(donnee* don);
/*************************************************************************/
# endif
/*************************************************************************/
