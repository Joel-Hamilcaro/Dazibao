/*************************************************************************/
# ifndef LIST_H
# define LIST_H
/*************************************************************************/

// Liste chaînée avec accès direct au premier et dernier élément.
typedef struct elem elem;

struct elem{
  void* data;
  elem* prev;
  elem* next;
};

typedef struct list list;

struct list{
  elem* first;
  elem* last;
  int size;
};

list* new_list();
void list_add(list* l, void* data, int i);
void list_insert_before(list* l, void* data, elem* elt);
void list_insert_after(list* l, void* data, elem* elt);
void* list_remove(list* l, elem* e);
elem* list_get_by_elem(list* l, elem* e);
elem* list_get_by_data(list* l, void* data);
elem* list_get_by_index(list* l, int index);
void* list_get_data_by_index(list* l, int index);
void print_list(list* l, void (*pf)(void*) );
list* list_remove_all(list* l);

/*************************************************************************/
# endif
/*************************************************************************/
