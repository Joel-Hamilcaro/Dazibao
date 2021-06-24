/*************************************************************************/
# ifndef PAQUET_H
# define PAQUET_H
/*************************************************************************/
# include "list.h"
# include "tvl.h"

uint16_t write_paquet(unsigned char *buf, list* l);
void print_paquet(unsigned char *buf, int response, int debug);
int verif_paquet(unsigned char *buf);

/*************************************************************************/
# endif
/*************************************************************************/
