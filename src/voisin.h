/*************************************************************************/
# ifndef VOISIN_H
# define VOISIN_H
/*************************************************************************/
# include "list.h"

# define TRANSITOIRE 0
# define PERMANENT 1

typedef struct voisin voisin;

struct voisin {
  int permanent; // 0 (transitoire) <> 1 (permanent)
  struct timeval* date; // Date de réception dernier message
  unsigned char* ip; // addresse ip
  unsigned char* port; // port
};

voisin* new_voisin(int permanent, struct timeval* date, unsigned char* ip, unsigned char* port);
void free_voisin(voisin* v);
voisin* get_voisin(list* l, unsigned char* ip, unsigned char* port);
void print_voisin(void* d);

unsigned char* get_voisin_ip(voisin* v, unsigned char* buf, int len);
voisin* get_voisin_by_ip_only(list* l, unsigned char* ip);

unsigned char* get_voisin_port(voisin* v, unsigned char* buf, int len);


/*************************************************************************/
# endif
/*************************************************************************/
