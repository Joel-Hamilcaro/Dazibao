# include <stdio.h>
# include <stdlib.h>
# include "list.h"

// ===================================================================== //

elem* new_elem(void* data){
  elem* n;
  if ((n=malloc(sizeof(elem))) == NULL){
    printf("Mémoire insuffisante : malloc new_elem\n");
    exit(EXIT_FAILURE);
  }
  n->data = data;
  n->prev = NULL;
  n->next = NULL;
  return n;
}

// ===================================================================== //

elem* elem_insert_after(elem* e, void* data){
  if (e==NULL) return new_elem(data);
  elem* n = e->next;
  elem* m = new_elem(data);
  m->prev = e;
  e->next = m;
  m->next = n;
  if (n!=NULL) n->prev = m;
  return m;
}

// ===================================================================== //

elem* elem_insert_before(elem* e, void* data){
  if (e==NULL) return new_elem(data);

  // e not first
  if (e->prev!=NULL) return elem_insert_after(e->prev,data);

  // e first
  else {
    elem* m = new_elem(data);
    m->next = e;
    e->prev = m;
    return m;
  }
}

// ===================================================================== //

void* elem_remove(elem* e){
  if (e==NULL) return NULL;
  elem* p = e->prev;
  elem* n = e->next;
  if (p!=NULL) p->next = n;
  if (n!=NULL) n->prev = p;
  void* data = e->data;
  free(e);
  return data;
}

// ===================================================================== //

list* new_list(){
  list* l;
  if ((l=malloc(sizeof(list)))== NULL){
    printf("Mémoire insuffisante : malloc new_list\n");
    exit(EXIT_FAILURE);
  }
  l->first = NULL;
  l->last = NULL;
  l->size = 0;
  return l;
}

// ===================================================================== //

void list_add(list* l, void* data, int i){
  if (l==NULL) return;
  if (i<0 || i>l->size) return;
  // insert first
  if (i==0){
    elem* e = elem_insert_before(l->first,data);
    l->first = e;
    if ( ++(l->size) == 1) l->last = e;
    return;
  }
  // insert last
  if (i==l->size){
    elem* e = elem_insert_after(l->last,data);
    l->last = e;
    (l->size)++;
    return;
  }
  // insert not first and not last
  elem* e = l->first;
  for (int k=0 ; k<i; k++) e = e->next;
  elem_insert_before(e,data);
  (l->size)++;
}

// ===================================================================== //

void list_insert_before(list* l, void* data, elem* elt){
  if (l==NULL || elt==NULL) return;
  // insert first
  if (l->first==elt){
    elem* e = elem_insert_before(l->first,data);
    l->first = e;
    if ( ++(l->size) == 1) l->last = e;
    return;
  }
  // insert not first
  elem_insert_before(elt,data);
  (l->size)++;
}

// ===================================================================== //

void list_insert_after(list* l, void* data, elem* elt){
  if (l==NULL || elt==NULL) return;

  // insert last
  if (l->last==elt){
    elem* e = elem_insert_after(l->last,data);
    l->last = e;
    if ( ++(l->size) == 1) l->first = e;
    return;
  }
  // insert not first
  elem_insert_after(elt,data);
  (l->size)++;
}

// ===================================================================== //

void* list_remove(list* l, elem* e){
  if (l==NULL) return NULL;
  if (l->first==e) l->first = e->next;
  if (l->last==e) l->last = e->prev;
  (l->size)--;
  return elem_remove(e);
}


// ===================================================================== //

elem* list_get_by_elem(list* l, elem* e){
  if (l==NULL) return NULL;
  elem* e1 = l->first;
  elem* e2 = l->last;
  for (int i=0; i<l->size; i++){
    if (e1==e) return e;
    if (e2==e) return e;
    e1 = e1->next;
    e2 = e2->prev;
  }
  return NULL;
}

// ===================================================================== //

elem* list_get_by_data(list* l, void* data){
  if (l==NULL) return NULL;
  elem* e1 = l->first;
  elem* e2 = l->last;
  for (int i=0; i<l->size; i++){
    if (e1->data==data) return e1;
    if (e2->data==data) return e2;
    e1 = e1->next;
    e2 = e2->prev;
  }
  return NULL;
}

// ===================================================================== //

elem* list_get_by_index(list* l, int index){
  int k1 = 0;
  int k2 = l->size-1;
  elem* e1 = l->first;
  elem* e2 = l->last;
  for (int i=0; i<l->size; i++){
    if (k1++==index) return e1;
    if (k2--==index) return e2;
    e1 = e1->next;
    e2 = e2->prev;
  }
  return NULL;
}


void* list_get_data_by_index(list* l, int index){
  int k1 = 0;
  int k2 = l->size-1;
  elem* e1 = l->first;
  elem* e2 = l->last;
  for (int i=0; i<l->size; i++){
    if (k1++==index) return e1->data;
    if (k2--==index) return e2->data;
    e1 = e1->next;
    e2 = e2->prev;
  }
  return NULL;
}

// ===================================================================== //

void print_list(list* l, void (*pf)(void*) ){
  //printf("LIST -> [\n\n");
  for (elem* e = l->first; e!=NULL; e=e->next ){
    (*pf)(e->data);
    if(e->next==NULL) break;
    //else printf("");
  }
  //printf("\n\n] END_LIST\n\n");
  /*
  printf(" <--> [");
  for (elem* e = l->last; e!=NULL; e=e->prev ){
    (*pf)(e->data);
    if(e->prev==NULL) break;
    else printf(";");
  }
  printf("]\n");
  */
}

// ===================================================================== //

list* list_remove_all(list* l){
  if (l==NULL) return NULL;
  while(l->size>0){
    void* data = list_remove(l,l->first);
    free(data);
  }
  return l;
}
// ===================================================================== //
