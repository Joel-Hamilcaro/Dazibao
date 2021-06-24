/*************************************************************************/
# ifndef CLIENT_MANAGER_H
# define CLIENT__MANAGER_H
/*************************************************************************/
# include "list.h"

# define SIZE 1024
# define REQ_SIZE 1024
# define REP_SIZE 1024

int my_udp_socket();
void build_req(unsigned char* req, int reqlen);
struct sockaddr_in6 my_ipv6server(int port);
void my_bind(int s, struct sockaddr_in6 server);
struct sockaddr_in6* ip_from_addrinfo(char* server_name, char* server_port, list* voisins);
void print_datagram(unsigned char* req, int reqlen, int response, int debug);
void print_client(struct sockaddr* client);

/*************************************************************************/
# endif
/*************************************************************************/
