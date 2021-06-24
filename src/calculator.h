/*************************************************************************/
# ifndef CALCULATOR_H
# define CALCULATOR_H
/*************************************************************************/
# include "donnee.h"
# include "list.h"


# define HASHSIZE 32
# define HI_SIZE 16


int rand_between(int a, int b);

void print_octet_by_octet(unsigned char* buf, int len);
void print_char_by_char(unsigned char* buf, int len);
void print_hex_by_hex(unsigned char* buf, int len);

uint16_t somme(uint16_t s, uint16_t n);

int isInferior(uint16_t s, uint16_t n);

void print_sha256(unsigned char* hash, int hashsize);

unsigned char* sha256(unsigned char* buf, int buflen, unsigned char hash[32]);

unsigned char* h(unsigned char* buf, int buflen, unsigned char hash[16]);

void print_h(unsigned char* hash);

unsigned char* hi(donnee* don, unsigned char hash[16]);

int network_size(list* l);

unsigned char* H(list* l, unsigned char hash[16]);

int hash_equals(unsigned char* hash1, unsigned char* hash2);

void print_timeval(struct timeval tv);

/*************************************************************************/
# endif
/*************************************************************************/
