#include "lidija.h"

LValue *l_call_func(LNode *node, LValue *func, LClosure *closure) {
  char *name;
  int argc;
  LNode **args;

  if(node != NULL) {
    name = node->val;
    argc = node->exprc;
    args = node->exprs;
  } else {
    name = "";
    argc = 0;
    args = NULL;
  }

  l_debug(L_DEBUG_CALL) {
    printf(">>> entering %s\n", name);
  }
  LValue *value;
  int i;
  LValue *v, *argsVal, **ref;

  // create a running scope to hold arguments
  // and a reference to self (for recursion)
  LClosure *cl = l_closure_clone(func->core.func.closure, node);
  if(strcmp(name, "") != 0)
    l_closure_set(cl, name, func, true);

  // initialize all args to nil
  for(i=0; i<func->core.func.argc; i++) {
    v = l_value_new(L_NIL_TYPE, cl);
    l_closure_set(cl, func->core.func.args[i]->val, v, true);
  }

  // setup the arguments
  argsVal = l_value_new(L_LIST_TYPE, cl);
  argsVal->core.list = create_vector();
  l_closure_set(cl, "args", argsVal, true);

  // set all passed args
  for(i=0; i<argc; i++) {
    if(args[i]->type == L_VAR_TYPE) {
      // L_VAR_TYPE is special case, since we must link the local to the parent closure var
      ref = l_closure_get_ref(closure, args[i]->val);
      if(ref != NULL) {
        v = l_closure_get(closure, args[i]->val);
        if(i < func->core.func.argc) {
          // store as named arg (uses ref so that it points to the same var)
          l_ref_put(cl->locals, func->core.func.args[i]->val, ref);
        }
      } else {
        v = l_value_new(L_ERR_TYPE, closure);
        v->core.str = make_stringbuf(args[i]->val);
        buffer_concat(v->core.str, " not found");
        l_handle_error(v, node, closure);
      }
    } else {
      // eval as normal and set the value
      v = l_eval_node(args[i], closure); // use calling closure
      if(i < func->core.func.argc) {
        l_closure_set(cl, func->core.func.args[i]->val, v, true);
      }
    }
    // append to 'args' variable
    vector_add(argsVal->core.list, v, sizeof(v));
  }

  if(func->core.func.ptr != NULL) {
    // native C code
    value = func->core.func.ptr(argsVal, cl);
  } else {
    // Lidija code
    int exprc = func->core.func.exprc;
    for(i=0; i<exprc; i++) {
      value = l_eval_node(func->core.func.exprs[i], cl);
    }
    if(exprc == 0) {
      value = l_value_new(L_NIL_TYPE, cl);
    }
  }

  l_closure_free(cl);

  l_debug(L_DEBUG_CALL) {
    printf("<<< returning from %s\n", name);
  }

  return value;
}

// inserts given function as given name in given closure
void l_insert_func(char *name, struct LValue* (*ptr)(struct LValue*, LClosure*), LClosure *closure) {
  LValue *f = l_func_new(ptr, closure);
  l_closure_set(closure, name, f, false);
}

// creates all built-in functions in the given closure
void l_create_funcs(LClosure *closure) {
  l_insert_func("+",       l_func_add,       closure);
  l_insert_func("-",       l_func_num_sub,   closure);
  l_insert_func("*",       l_func_num_mul,   closure);
  l_insert_func("/",       l_func_num_div,   closure);
  l_insert_func("->",      l_func_list_get,  closure);
  l_insert_func("str",     l_func_str,       closure);
  l_insert_func("|",       l_func_str_split, closure);
  l_insert_func("out",     l_func_out,       closure);
  l_insert_func("if",      l_func_if,        closure);
  l_insert_func("while",   l_func_while,     closure);
  l_insert_func("count",   l_func_count,     closure);
  l_insert_func("first",   l_func_first,     closure);
  l_insert_func("rest",    l_func_rest,      closure);
  l_insert_func("==",      l_func_eq,        closure);
  l_insert_func("!=",      l_func_neq,       closure);
  l_insert_func(">=",      l_func_num_gte,   closure);
  l_insert_func("<=",      l_func_num_lte,   closure);
  l_insert_func(">",       l_func_num_gt,    closure);
  l_insert_func("<",       l_func_num_lt,    closure);
  l_insert_func("&&",      l_func_and,       closure);
  l_insert_func("||",      l_func_or,        closure);
  l_insert_func("require", l_func_require,   closure);
  l_insert_func("type",    l_func_type,      closure);
}

// sets misc global vars
void l_create_globals(LClosure *closure) {
  l_closure_set(closure, "nil",   l_value_new(L_NIL_TYPE,   closure), false);
  l_closure_set(closure, "false", l_value_new(L_FALSE_TYPE, closure), false);
  l_closure_set(closure, "true",  l_value_new(L_TRUE_TYPE,  closure), false);
}

// loads the core library
void l_load_lib(LClosure *closure) {
  // FIXME use absolute paths
#if L_SKIP_LIB == 0
  l_eval_path("lib/core/list.lid", closure);
  l_eval_path("lib/core/math.lid", closure);
#endif
}
