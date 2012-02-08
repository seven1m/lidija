#ifndef VALUE_H
#define VALUE_H

// forward declare LClosure
struct LClosure;
typedef struct LClosure LClosure;

// structure for representing dynamic,
// evaluated objects
typedef struct LValue {
  int ref_count;
  enum LNodeType type;
  union {
    mpz_t num;
  } core;
} LValue;

// note: have to use void* because of circular ref
LValue *l_value_new(enum LNodeType type, LClosure *closure);

#endif
