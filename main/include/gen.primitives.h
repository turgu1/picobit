#include "esp32-scheme-vm.h"
#include "primitives.h"

#ifdef CONFIG_DEBUG_STRINGS
const char * const primitive_names[] = {
  "#%halt",
  "return",
  "pop",
  "get-cont",
  "graft-to-cont",
  "return-to-cont",
  "pair?",
  "cons",
  "car",
  "cdr",
  "set-car!",
  "set-cdr!",
  "null?",
  "number?",
  "=",
  "#%+",
  "#%-",
  "#%mul-non-neg",
  "#%div-non-neg",
  "#%rem-non-neg",
  "<",
  ">",
  "bitwise-ior",
  "bitwise-xor",
  "bitwise-and",
  "bitwise-not",
  "eq?",
  "not",
  "symbol?",
  "boolean?",
  "string?",
  "string->list",
  "list->string",
  "u8vector?",
  "#%make-u8vector",
  "u8vector-ref",
  "u8vector-set!",
  "u8vector-length",
  "print",
  "clock",
  "#%getchar-wait",
  "#%putchar",
  "GPIO"
};
#endif /* CONFIG_DEBUG_STRINGS */

extern void primitive_return();
extern void primitive_pop();
extern void primitive_get_cont();
extern void primitive_graft_to_cont();
extern void primitive_return_to_cont();
extern void primitive_pair_p();
extern void primitive_cons();
extern void primitive_car();
extern void primitive_cdr();
extern void primitive_set_car_bang();
extern void primitive_set_cdr_bang();
extern void primitive_null_p();
extern void primitive_number_p();
extern void primitive_equal();
extern void primitive_add();
extern void primitive_sub();
extern void primitive_mul_non_neg();
extern void primitive_div_non_neg();
extern void primitive_rem_non_neg();
extern void primitive_lt();
extern void primitive_gt();
extern void primitive_bitwise_ior();
extern void primitive_bitwise_xor();
extern void primitive_bitwise_and();
extern void primitive_bitwise_not();
extern void primitive_eq_p();
extern void primitive_not();
extern void primitive_symbol_p();
extern void primitive_boolean_p();
extern void primitive_string_p();
extern void primitive_string2list();
extern void primitive_list2string();
extern void primitive_u8vector_p();
extern void primitive_make_u8vector();
extern void primitive_u8vector_ref();
extern void primitive_u8vector_set();
extern void primitive_u8vector_length();
extern void primitive_print();
extern void primitive_clock();
extern void primitive_getchar_wait();
extern void primitive_putchar();
extern void primitive_gpio();
