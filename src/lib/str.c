#include "../lydia.h"

LValue *l_func_str(LValue *args, LClosure *closure) {
  LValue *value = l_value_new(L_STR_TYPE, closure);
  value->core.str = make_stringbuf("");
  char *s;
  int i;
  for(i=0; i<args->core.list->length; i++) {
    s = l_str(l_list_get(args, i));
    concat_stringbuf(value->core.str, s);
  }
  return value;
}

LValue *l_func_str_add(LValue *args, LClosure *closure) {
  LValue *v1 = l_list_get(args, 0);
  LValue *v2 = l_list_get(args, 1);
  LValue *value = l_value_new(L_STR_TYPE, closure);
  value->core.str = make_stringbuf("");
  concat_stringbuf(value->core.str, v1->core.str->str);
  concat_stringbuf(value->core.str, v2->core.str->str);
  return value;
}

// FIXME this does not work as expected/desired for a multi-character delimiter
// since it uses strtok under the hood
LValue *l_func_str_split(LValue *args, LClosure *closure) {
  LValue *string = l_list_get(args, 0);
  LValue *delim = l_list_get(args, 1);
  l_assert_is(string, L_STR_TYPE, L_ERR_MISSING_STR, closure);
  l_assert_is(delim, L_STR_TYPE, L_ERR_MISSING_STR, closure);
  int i, size;
  char **strings = str_split(string->core.str->str, delim->core.str->str, &size);
  LValue *value = l_value_new(L_LIST_TYPE, closure);
  value->core.list = create_vector();
  LValue *s;
  for(i=0; i<size; i++) {
    s = l_value_new(L_STR_TYPE, closure);
    s->core.str = make_stringbuf(strings[i]);
    vector_add(value->core.list, s, sizeof(s));
  }
  return value;
}

bool l_str_eq(LValue *s1, LValue *s2) {
  return strcmp(s1->core.str->str, s2->core.str->str) == 0;
}

// returns a c string representation for the given LValue
// (be sure to free the string when you're done)
char *l_str(LValue *value) {
  char *str;
  stringbuf *str2;
  switch(value->type) {
    case L_NUM_TYPE:
      str = mpz_get_str(NULL, 10, value->core.num);
      break;
    case L_STR_TYPE:
      str = GC_MALLOC(sizeof(char) * (value->core.str->length + 1));
      strcpy(str, value->core.str->str);
      break;
    case L_LIST_TYPE:
      str2 = make_stringbuf("[");
      char *s;
      int i, len = value->core.list->length;
      for(i=0; i<len; i++) {
        s = l_str(l_list_get(value, i));
        concat_stringbuf(str2, s);
        if(i<len-1) buffer_concat(str2, " ");
      }
      buffer_concat(str2, "]");
      str = GC_MALLOC(sizeof(char) * (str2->length + 1));
      strcpy(str, str2->str);
      destroy_buffer(str2);
      break;
    case L_TRUE_TYPE:
      str = GC_MALLOC(sizeof(char) * 5);
      strcpy(str, "true");
      break;
    case L_FALSE_TYPE:
      str = GC_MALLOC(sizeof(char) * 6);
      strcpy(str, "false");
      break;
    case L_NIL_TYPE:
      str = GC_MALLOC(sizeof(char) * 4);
      strcpy(str, "nil");
      break;
    default:
      str = GC_MALLOC(sizeof(char) * 1);
      strcpy(str, "");
  }
  return str;
}
