#include "lydia.h"

LValue *l_eval_node(LNode *node, LClosure *closure) {
  LValue *value;
  switch(node->type) {
    case L_ERR_TYPE:
      value = l_eval_error_node(node, closure);
      l_handle_error(value, node, closure);
      break;
    case L_NUM_TYPE:
      value = l_eval_num_node(node, closure);
      break;
    case L_STR_TYPE:
      value = l_eval_string_node(node, closure);
      break;
    case L_VAR_TYPE:
      value = l_eval_var_node(node, closure);
      break;
    case L_LIST_TYPE:
      value = l_eval_list_node(node, closure);
      break;
    case L_FUNC_TYPE:
      value = l_eval_func_node(node, closure);
      break;
    case L_CALL_TYPE:
      value = l_eval_call_node(node, NULL, closure);
      break;
    case L_ASSIGN_TYPE:
      value = l_eval_assign_node(node, closure);
      break;
    default:
      printf("ERROR: unable to eval element");
  }
  return value;
}

LValue *l_eval_num_node(LNode *node, LClosure *closure) {
  LValue *value = l_value_new(L_NUM_TYPE, closure);
  mpz_init_set_str(value->core.num, node->val, 0);
  return value;
}

LValue *l_eval_string_node(LNode *node, LClosure *closure) {
  LValue *value = l_value_new(L_STR_TYPE, closure);
  if(strchr(node->val, '\\')) {
    value->core.str = make_stringbuf("");
    int i, len = strlen(node->val);
    char c[] = " ";
    for(i=0; i<len; i++) {
      if(node->val[i] == '\\' && i < len-1) {
        i++;
        switch(node->val[i]) {
          case 'a' : c[0] = '\a'; break;
          case 'b' : c[0] = '\b'; break;
          case 'f' : c[0] = '\f'; break;
          case 'n' : c[0] = '\n'; break;
          case 'r' : c[0] = '\r'; break;
          case 't' : c[0] = '\t'; break;
          case 'v' : c[0] = '\v'; break;
          case '\'': c[0] = '\''; break;
          case '"' : c[0] = '"' ; break;
          case '\\': c[0] = '\\'; break;
          case '?' : c[0] = '?' ; break;
        }
      } else {
        c[0] = node->val[i];
      }
      concat_stringbuf(value->core.str, c);
    }
  } else {
    value->core.str = make_stringbuf(node->val);
  }
  return value;
}

LValue *l_eval_error_node(LNode *node, LClosure *closure) {
  LValue *value = l_value_new(L_ERR_TYPE, closure);
  value->core.str = make_stringbuf(node->val);
  return value;
}

LValue *l_eval_assign_node(LNode *node, LClosure *closure) {
  LValue *value = l_eval_node(node->exprs[0], closure);
  l_closure_set(closure, node->val, value, false);
  return value;
}

LValue *l_eval_var_node(LNode *node, LClosure *closure) {
  LValue *value = l_closure_get(closure, node->val);
  if(value != NULL) {
    return value;
  } else {
    value = l_value_new(L_ERR_TYPE, closure);
    value->core.str = make_stringbuf(node->val);
    buffer_concat(value->core.str, " not found");
    l_handle_error(value, node, closure);
    return value;
  }
}

LValue *l_eval_list_node(LNode *node, LClosure *closure) {
  LValue *value = l_value_new(L_LIST_TYPE, closure);
  value->core.list = create_vector();
  LValue *v;
  int i;
  for(i=0; i<node->exprc; i++) {
    v = l_eval_node(node->exprs[i], closure);
    vector_add(value->core.list, v, sizeof(v));
  }
  return value;
}

LValue *l_eval_func_node(LNode *node, LClosure *closure) {
  LValue *value = l_value_new(L_FUNC_TYPE, closure);
  value->core.func.ptr = NULL;
  // pass node=NULL to l_closure_clone, so this lexical frame is not printed in a stack trace
  value->core.func.closure = l_closure_clone(closure, NULL);
  if(node->exprs[0]) {
    value->core.func.argc = node->exprs[0]->exprc;
    value->core.func.args = node->exprs[0]->exprs;
  } else {
    value->core.func.argc = 0;
  }
  value->core.func.exprc = node->exprs[1]->exprc;
  value->core.func.exprs = node->exprs[1]->exprs;
  return value;
}

void l_eval(const char *source, const char *source_file, LClosure *closure) {
  LAst ast = l_parse(source, source_file);
  list_iter_p iter = list_iterator(ast, FRONT);
  while(list_next(iter) != NULL) {
    l_eval_node((LNode*)list_current(iter), closure);
  }
}

void l_eval_path(const char *filename, LClosure *closure) {
  FILE *fp = fopen(filename, "r");
  if(fp == NULL) {
    printf("An error occurred while opening the file %s.\n", filename);
    exit(1);
  }

  LValue* f = l_value_new(L_STR_TYPE, closure);
  f->core.str = make_stringbuf((char*)filename);
  l_closure_set(closure, "-filename", f, true);

  stringbuf *source = make_stringbuf("");
  source->str = saferead(fp);

  l_eval(source->str, filename, closure);
}

void l_inspect(LValue *value) {
  char buf[255] = "";
  printf("    %s\n", l_inspect_to_str((LValue*)value, buf, 255));
}

char *l_inspect_to_str(LValue *value, char *buf, int bufLen) {
  char *repr;
  switch(value->type) {
    case L_ERR_TYPE:
      snprintf(buf, bufLen-1, "<Err: %s>", value->core.str->str);
      break;
    case L_NUM_TYPE:
      repr = l_str(value);
      snprintf(buf, bufLen-1, "<Num: %s>", repr);
      break;
    case L_STR_TYPE:
      snprintf(buf, bufLen-1, "<Str: %s>", value->core.str->str);
      break;
    case L_LIST_TYPE:
      if(value->core.list->length > 0) {
        repr = l_str(l_list_get(value, 0));
        snprintf(buf, bufLen-1, "<List with %d item(s), first=%s>", (int)value->core.list->length, repr);
      } else {
        snprintf(buf, bufLen-1, "<List with 0 item(s)>");
      }
      break;
    case L_FUNC_TYPE:
      if(value->core.func.ptr != NULL) {
        snprintf(buf, bufLen-1, "<Func Ptr with %d arg(s) and %d expr(s)>", value->core.func.argc, value->core.func.exprc);
      } else {
        snprintf(buf, bufLen-1, "<Func with %d arg(s) and %d expr(s)>", value->core.func.argc, value->core.func.exprc);
      }
      break;
    case L_TRUE_TYPE:
      snprintf(buf, bufLen-1, "<true>");
      break;
    case L_FALSE_TYPE:
      snprintf(buf, bufLen-1, "<false>");
      break;
    case L_NIL_TYPE:
      snprintf(buf, bufLen-1, "<nil>");
      break;
    default:
      snprintf(buf, bufLen-1, "unable to inspect element type %d", value->type);
  }
  return buf;
}
