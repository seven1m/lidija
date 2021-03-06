#ifndef NUM_H
#define NUM_H

bool l_num_eq(LValue *n1, LValue *n2);

LValue *l_func_num_add(LValue *args, LClosure *closure);
LValue *l_func_num_sub(LValue *args, LClosure *closure);
LValue *l_func_num_mul(LValue *args, LClosure *closure);
LValue *l_func_num_div(LValue *args, LClosure *closure);
LValue *l_func_num_gt(LValue *args, LClosure *closure);
LValue *l_func_num_gte(LValue *args, LClosure *closure);
LValue *l_func_num_lt(LValue *args, LClosure *closure);
LValue *l_func_num_lte(LValue *args, LClosure *closure);

#endif
