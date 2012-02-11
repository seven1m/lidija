#ifndef CLOSURE_H
#define CLOSURE_H

#include <glib.h>

typedef struct LClosure {
  LHeap *heap;
  GHashTable *vars;
  struct LClosure *parent;
  bool cloneable;
} LClosure;

LClosure *l_closure_new();
LClosure *l_closure_clone(LClosure *parent);
LClosure *l_closure_root(LClosure *closure);
void l_closure_set(LClosure *closure, char *name, LValue *value);
void l_closure_set_funcs(LClosure *closure);
LValue *l_closure_get(LClosure *closure, char *name);

void l_inspect_closure(LClosure* closure);
int l_closure_size(LClosure *closure);

#endif
