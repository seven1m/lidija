#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <gmp.h>
#include "ast.h"
#include "parser.h"
#include "value.h"
#include "heap.h"
#include "closure.h"
#include "eval.h"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
