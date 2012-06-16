#include "lidija.h"

void l_heap_add(LHeap *heap, LValue *value) {
  g_ptr_array_add(heap, value);
}

LHeap *l_heap_new() {
  return g_ptr_array_sized_new(1000);
}

static void l_heap_size_iter(gpointer value, gpointer counter) {
  ((*(int*)counter))++;
}

int l_heap_size(LHeap *heap) {
  int counter = 0;
  g_ptr_array_foreach(heap, l_heap_size_iter, &counter);
  return counter;
}

static void l_inspect_heap_iter(gpointer val, gpointer user_data) {
  char buf[255] = "";
  printf("  %s (refcount: %d)\n", l_inspect((LValue*)val, buf, 255), ((LValue*)val)->ref_count);
}

void l_heap_gc(LHeap *heap) {
  int i;
  LValue *val;
  for(i=heap->len-1; i>=0; i--) {
    val = g_ptr_array_index(heap, i);
    if(val->ref_count == 0) {
#if L_DEBUG_GC == 1
      printf("GC: freeing slot\n");
      l_inspect_heap_iter(val, NULL);
#endif
      g_ptr_array_remove_index(heap, i);
      free(val);
    }
  }
}

// prints keys and vals in a closure
void l_inspect_heap(LHeap* heap) {
  printf("Heap contents:\n");
  g_ptr_array_foreach(heap, l_inspect_heap_iter, NULL);
}