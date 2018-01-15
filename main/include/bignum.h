/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#ifndef BIGNUM_H
#define BIGNUM_H

#if BIGNUM
  #define PUBLIC
#else
  #define PUBLIC extern
#endif

typedef cell_p integer;

/*
 * A `digit' is a numeric representation of one entry
 * of a bignum linked list. A `two_digit` is a numeric
 * representation for the cases where a result of
 * an operation is wider than a `digit'.
 */

typedef uint16_t digit;
typedef uint32_t two_digit;

#define digit_width (sizeof(digit) * 8)
#define two_digit_width (sizeof(two_digit) * 8)

#define obj_eq(x,y) ((x) == (y))

PUBLIC uint8_t         cmp(integer x, integer y);
PUBLIC integer         add(integer x, integer y);
PUBLIC integer         sub(integer x, integer y);
PUBLIC integer   mulnonneg(integer x, integer y);
PUBLIC integer   divnonneg(integer x, integer y);
PUBLIC integer bitwise_xor(integer x, integer y);
PUBLIC integer bitwise_ior(integer x, integer y);
PUBLIC integer bitwise_and(integer x, integer y);
PUBLIC integer bitwise_not(integer x);

PUBLIC void bignum_gc_init();
PUBLIC void bignum_gc_mark();

#undef PUBLIC
#endif
