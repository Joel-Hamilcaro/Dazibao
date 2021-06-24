/*************************************************************************/
# ifndef TVL_H
# define TVL_H
/*************************************************************************/
# include "donnee.h"
# include "calculator.h"

typedef struct tvl tvl;

struct tvl{
  uint8_t type;
  uint8_t length;
  unsigned char* body;
};

tvl* new_tvl(unsigned char type, unsigned char length, unsigned char* body);

void write_tvl(tvl* t, unsigned char* tvlbuf);
tvl* read_tvl(unsigned char* tvlbuf);
void print_tvl(tvl* tvl, int response, int debug);
tvl* new_tvl_pad1();
tvl* new_tvl_padN(int n);
tvl* new_tvl_neighbour_req();
tvl* new_tvl_neighbour(unsigned char* ip,unsigned char* port, unsigned char* buf);
tvl* new_tvl_network_hash(unsigned char* networkhash);
tvl* new_tvl_network_state_req();
tvl* new_tvl_node_hash(donnee* don,unsigned char* buf);
tvl* new_tvl_node_state_req(unsigned char* nodeID);
tvl* new_tvl_node_state(donnee* don,unsigned char* buf);
tvl* new_tvl_warning(unsigned char* msg, int len);

/*************************************************************************/
# endif
/*************************************************************************/
