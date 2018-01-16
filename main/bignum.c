/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#include "esp32_scheme_vm.h"
#include "vm_arch.h"
#include "mm.h"
#include "testing.h"

#define BIGNUM 1
#include "bignum.h"

/* Temps in bignum algorithms must be registered as roots too, since
 * GC can occur during bignum operations (they allocate).
 * Bignum ops can share variables as long as they don't interfere.
 *
 * Usage map:
 * bignum_tmp1 => ior xor add sub scale shr shl
 * bignum_tmp2 => shift_left mul
 * bignum_tmp3 => div mul
 * bignum_tmp4 => div mul
 * bignum_tmp5 => div
 */

static cell_p bignum_tmp1,
              bignum_tmp2,
              bignum_tmp3,
              bignum_tmp4,
              bignum_tmp5;

void bignum_gc_init()
{
	bignum_tmp1 = NIL;
	bignum_tmp2 = NIL;
	bignum_tmp3 = NIL;
	bignum_tmp4 = NIL;
	bignum_tmp5 = NIL;
}

void bignum_gc_mark()
{
	mm_mark(bignum_tmp1);
	mm_mark(bignum_tmp2);
	mm_mark(bignum_tmp3);
	mm_mark(bignum_tmp4);
	mm_mark(bignum_tmp5);
}

PRIVATE integer integer_hi(integer x)
{
	if (IN_RAM(x)) {
    EXPECT(RAM_IS_BIGNUM(x), "integer_hi.0", "bignum");
		return RAM_GET_BIGNUM_HI(x);
	}
  else if (IN_ROM(x)) {
    EXPECT(ROM_IS_BIGNUM(x), "integer_hi.1", "bignum");
		return ROM_GET_BIGNUM_HI(x);
	}
  else if (x == NEG1) {
		return NEG1;        /* negative small int */
	}
  else {
    EXPECT(IS_SMALL_INT(x), "integer_lo.2", "small int");
		return ZERO;        /* nonnegative small int */
	}
}

PRIVATE digit integer_lo (integer x)
{
	if (IN_RAM(x)) {
    EXPECT(RAM_IS_BIGNUM(x), "integer_lo.0", "bignum");
		return RAM_GET_BIGNUM_VALUE(x);
	}
  else if (IN_ROM(x)) {
    EXPECT(ROM_IS_BIGNUM(x), "integer_lo.1", "bignum");
    return ROM_GET_BIGNUM_VALUE(x);
	}
  else {
    EXPECT(IS_SMALL_INT(x), "integer_lo.2", "small int");
		return SMALL_INT_VALUE(x);
	}
}

PRIVATE integer norm(cell_p prefix, integer n)
{
	/* norm(prefix,n) returns a normalized integer whose value is the
	   integer n prefixed with the digits in prefix (a list of digits) */

	while (prefix != NIL) {
		digit d = integer_lo(prefix);
		cell_p temp = prefix;

		prefix = integer_hi(temp);

		if (obj_eq(n, ZERO)) {
			if (d <= MAX_SMALL_INT_VALUE) {
				n = ENCODE_SMALL_INT(d);
				continue;
			}
		}
    else if (obj_eq(n, NEG1)) {
			if (d >= (1 << digit_width) + MIN_SMALL_INT_VALUE) {
				n = ENCODE_SMALL_INT(d - (1 << digit_width));
				continue;
			}
		}

		RAM_SET_BIGNUM_HI(temp, n);
		n = temp;
	}

	return n;
}

PRIVATE bool negp(integer x)
{
	/* negp(x) returns true iff x is negative */

	do {
		x = integer_hi(x);

		if (obj_eq(x, ZERO)) {
			return false;
		}
	} while (!obj_eq(x, NEG1));

	return true;
}

PRIVATE uint16_t integer_length(integer x)
{
	/* integer_length(x) returns the number of bits in the binary
	   representation of the nonnegative integer x */

	uint16_t result = 0;
	integer next;
	digit d;

	while (!obj_eq((next = integer_hi(x)), ZERO)) {
		result += digit_width;
		x = next;
	}

	d = integer_lo(x);

	while (d > 0) {
		result++;
		d >>= 1;
	}

	return result;
}

PRIVATE integer shr(integer x)   // TODO have shift_right
{
	/* shr(x) returns the integer x shifted one bit to the right */

	// Note: we don't need to register x with the GC, even though we
	//  assign it. Our caller registered the original value of x, and
	//  we only cdr it down. Thus, any local value of x is pointed to
	//  by the original x, so we're good. Same situation in most other
	//  bignum operations.

	bignum_tmp1 = NIL;
	digit d;

	for (;;) {
		if (obj_eq(x, ZERO) || obj_eq(x, NEG1)) {
			bignum_tmp1 = norm(bignum_tmp1, x);
			break;
		}

		d = integer_lo(x);
		x = integer_hi(x);
		bignum_tmp1 =
		        new_bignum((d >> 1) |
		                      ((integer_lo(x) & 1) ? (1 << (digit_width - 1)) : 0),
		                      bignum_tmp1);
	}

	// clear the root then return
	cell_p tmp = bignum_tmp1;
	bignum_tmp1 = NIL;
	return tmp;
}

PRIVATE integer negative_carry(integer carry)
{
	if (carry) {
		return NEG1;
	}
  else {
		return ZERO;
	}
}

PRIVATE integer shl(integer x)
{
	/* shl(x) returns the integer x shifted one bit to the left */

	// These two are always 0 or -1, never allocated values.
	// No need to register them as GC roots.
	// Same for other negc variables in other operations.
	integer negc = ZERO; /* negative carry */
	integer temp;

	bignum_tmp1 = NIL;
	digit d;

	for (;;) {
		if (obj_eq(x, negc)) {
			bignum_tmp1 = norm(bignum_tmp1, x);
			break;
		}

		d = integer_lo(x);
		x = integer_hi(x);
		temp = negc;
		negc = negative_carry(d & (1 << (digit_width - 1)));
		bignum_tmp1 =
		        new_bignum((d << 1) | obj_eq(temp, NEG1), bignum_tmp1);
	}

	// clear the root then return
	cell_p tmp = bignum_tmp1;
	bignum_tmp1 = NIL;
	return tmp;
}

PRIVATE integer shift_left(integer x, uint16_t n)
{
	/* shift_left(x,n) returns the integer x shifted n bits to the left */

	if (obj_eq(x, ZERO)) {
		return x;
	}

	bignum_tmp2 = x;

	while (n & (digit_width-1)) {
		bignum_tmp2 = shl(bignum_tmp2);
		n--;
	}

	while (n > 0) {
		bignum_tmp2 = new_bignum(0, bignum_tmp2);
		n -= digit_width;
	}

	// clear the root then return
	cell_p tmp = bignum_tmp2;
	bignum_tmp2 = NIL;
	return tmp;
}

PRIVATE integer invert(integer x)
{
	if (obj_eq(x, ZERO)) {
		return NEG1;
	}
  else {
		return ZERO;
	}
}

PRIVATE integer scale(digit n, integer x)
{
	/* scale(n,x) returns the integer n * x */

	digit carry;
	two_digit m;

	if ((n == 0) || obj_eq(x, ZERO)) {
		return ZERO;
	}

	if (n == 1) {
		return x;
	}

	bignum_tmp1 = NIL;
	carry = 0;

	for (;;) {
		if (obj_eq(x, ZERO)) {
			if (carry <= MAX_SMALL_INT_VALUE) {
				bignum_tmp1 = norm(bignum_tmp1, ENCODE_SMALL_INT(carry));
			}
      else {
				bignum_tmp1 = norm(bignum_tmp1, new_bignum(carry, ZERO));
			}

			break;
		}

		if (obj_eq(x, NEG1)) {
			carry = carry - n;

			// -1 as a literal is wrong with SIXPIC, thus the double negative
			if (carry >= ((1 << digit_width) + MIN_SMALL_INT_VALUE)) {
				bignum_tmp1 = norm(bignum_tmp1, ENCODE_SMALL_INT(carry));
			}
      else {
				bignum_tmp1 = norm(bignum_tmp1, new_bignum(carry, NEG1));
			}

			break;
		}

		m = integer_lo(x);
		m = m * n + carry;

		x = integer_hi(x);
		carry = m >> digit_width;
		bignum_tmp1 = new_bignum(m, bignum_tmp1);
	}

	// clear the root then return
	cell_p tmp = bignum_tmp1;
	bignum_tmp1 = NIL;
	return tmp;
}

uint8_t cmp(integer x, integer y)
{
	/* cmp(x,y) return 0 when x<y, 2 when x>y, and 1 when x=y */

	uint8_t result = 1;
	digit xlo;
	digit ylo;

	for (;;) {
		if (obj_eq(x, ZERO) || obj_eq(x, NEG1)) {
			if (!obj_eq(x, y)) {
				if (negp(y)) {
					result = 2;
				}
        else {
					result = 0;
				}
			}

			break;
		}

		if (obj_eq(y, ZERO) || obj_eq(y, NEG1)) {
			if (negp(x)) {
				result = 0;
			}
      else {
				result = 2;
			}

			break;
		}

		xlo = integer_lo(x);
		ylo = integer_lo(y);
		x = integer_hi(x);
		y = integer_hi(y);

		if (xlo != ylo) {
			if (xlo < ylo) {
				result = 0;
			}
      else {
				result = 2;
			}
		}
	}

	return result;
}

integer add(integer x, integer y)
{
	/* add(x,y) returns the sum of the integers x and y */

	integer negc = ZERO; /* negative carry */
	bignum_tmp1 = NIL; /* #f terminated for the norm function */
	digit dx;
	digit dy;

	for (;;) {
		if (obj_eq(x, negc)) {
			bignum_tmp1 = norm(bignum_tmp1, y);
			break;
		}

		if (obj_eq(y, negc)) {
			bignum_tmp1 = norm(bignum_tmp1, x);
			break;
		}

		dx = integer_lo(x);
		dy = integer_lo(y);
		dx = dx + dy; /* may wrap around */

		if (obj_eq(negc, ZERO)) {
			negc = negative_carry(dx < dy);
		}
    else {
			dx++; /* may wrap around */
			negc = negative_carry(dx <= dy);
		}

		x = integer_hi(x);
		y = integer_hi(y);

		bignum_tmp1 = new_bignum(dx, bignum_tmp1);
	}

	// clear the root then return
	cell_p tmp = bignum_tmp1;
	bignum_tmp1 = NIL;
	return tmp;
}

integer sub(integer x, integer y)
{
	/* sub(x,y) returns the difference of the integers x and y */
	integer negc = NEG1; /* negative carry */
	bignum_tmp1 = NIL;
	digit dx;
	digit dy;

	for (;;) {
		if (obj_eq(x, negc) && (obj_eq(y, ZERO) || obj_eq(y, NEG1))) {
			bignum_tmp1 = norm(bignum_tmp1, invert(y));
			break;
		}

		if (obj_eq(y, invert(negc))) {
			bignum_tmp1 = norm(bignum_tmp1, x);
			break;
		}

		dx = integer_lo(x);
		dy = ~integer_lo(y);
		dx = dx + dy; /* may wrap around */

		if (obj_eq(negc, ZERO)) {
			negc = negative_carry(dx < dy);
		}
    else {
			dx++; /* may wrap around */
			negc = negative_carry(dx <= dy);
		}

		x = integer_hi(x);
		y = integer_hi(y);

		bignum_tmp1 = new_bignum(dx, bignum_tmp1);
	}

	// clear the root then return
	cell_p tmp = bignum_tmp1;
	bignum_tmp1 = NIL;
	return tmp; // TODO have macro for that.
}

integer mulnonneg(integer x, integer y)
{
	/* mulnonneg(x,y) returns the product of the integers x and y
	   where x is nonnegative */

	bignum_tmp3 = NIL;
	bignum_tmp4 = scale(integer_lo(x), y);

	for (;;) {
		bignum_tmp3 = new_bignum(integer_lo(bignum_tmp4), bignum_tmp3);
		bignum_tmp4 = integer_hi(bignum_tmp4);
		x = integer_hi(x);

		if (obj_eq(x, ZERO)) {
			break;
		}

		// We need to register the result of scale because add can cause GC.
		bignum_tmp2 = scale(integer_lo(x), y);
		bignum_tmp4 = add(bignum_tmp4, bignum_tmp2);
	}

	cell_p tmp1 = bignum_tmp3;
	cell_p tmp2 = bignum_tmp4;
	bignum_tmp2 = NIL;
	bignum_tmp3 = NIL;
	bignum_tmp4 = NIL;

	return norm(tmp1, tmp2);
}

integer divnonneg(integer x, integer y)
{
	/* divnonneg(x,y) returns the quotient and remainder of
	   the integers x and y where x and y are nonnegative */

	// x and y end up pointing to newly allocated bignums, so we need
	// to register them with the GC.
	bignum_tmp4 = x;
	bignum_tmp5 = y;

	bignum_tmp3 = ZERO;
	uint16_t lx = integer_length(bignum_tmp4);
	uint16_t ly = integer_length(bignum_tmp5);

	if (lx >= ly) {
		lx = lx - ly;

		bignum_tmp5 = shift_left(bignum_tmp5, lx);

		do {
			bignum_tmp3 = shl(bignum_tmp3);

			if (cmp(bignum_tmp4, bignum_tmp5) >= 1) {
				bignum_tmp4 = sub(bignum_tmp4, bignum_tmp5);
				bignum_tmp3 = add(POS1, bignum_tmp3);
			}

			bignum_tmp5 = shr(bignum_tmp5);
		} while (lx-- != 0);
	}

	cell_p tmp = bignum_tmp3;
	bignum_tmp3 = NIL;
	bignum_tmp4 = NIL;
	return tmp;
}

integer bitwise_ior(integer x, integer y)
{
	/* returns the bitwise inclusive or of x and y */

	bignum_tmp1 = NIL;

	for (;;) {
		if (obj_eq(x, ZERO)) {
			cell_p tmp = bignum_tmp1;
			bignum_tmp1 = NIL;
			return norm(tmp, x);
		}

		if (obj_eq(x, NEG1)) {
			cell_p tmp = bignum_tmp1;
			bignum_tmp1 = NIL;
			return norm(tmp, x);
		}

		bignum_tmp1 = new_bignum(integer_lo(x) | integer_lo(y),
		                           bignum_tmp1);
		x = integer_hi(x);
		y = integer_hi(y);
	}
}

integer bitwise_and(integer x, integer y)
{
	/* returns the bitwise inclusive and of x and y */

	bignum_tmp1 = NIL;

	for (;;) {
		if (obj_eq(x, ZERO)) {
			cell_p tmp = bignum_tmp1;
			bignum_tmp1 = NIL;
			return norm(tmp, ZERO);
		}

		if (obj_eq(x, NEG1)) {
			cell_p tmp = bignum_tmp1;
			bignum_tmp1 = NIL;
			return norm(tmp, y);
		}

		bignum_tmp1 = new_bignum(integer_lo(x) & integer_lo(y),
		                           bignum_tmp1);
		x = integer_hi(x);
		y = integer_hi(y);
	}
}

integer bitwise_not(integer x)
{
	/* returns the bitwise not of x */

	bignum_tmp1 = NIL;

	for (;;) {
		if (obj_eq(x, ZERO)) {
			cell_p tmp = bignum_tmp1;
			bignum_tmp1 = NIL;
			return norm(tmp, NEG1);
		}

		if (obj_eq(x, NEG1)) {
			cell_p tmp = bignum_tmp1;
			bignum_tmp1 = NIL;
			return norm(tmp, ZERO);
		}

		bignum_tmp1 = new_bignum(~integer_lo(x), bignum_tmp1);

		x = integer_hi(x);
	}
}

integer bitwise_xor(integer x, integer y)   // TODO similar to ior (only diff is the test), abstract ?
{
	/* returns the bitwise inclusive or of x and y */

	bignum_tmp1 = NIL;

	for (;;) {
		if (obj_eq(x, ZERO)) {
			cell_p tmp = bignum_tmp1;
			bignum_tmp1 = NIL;
			return norm(tmp, y);
		}

		if (obj_eq(x, NEG1)) {
			cell_p tmp = bignum_tmp1;
			bignum_tmp1 = NIL;
			return norm(tmp, x);
		}

		bignum_tmp1 = new_bignum(integer_lo(x) ^ integer_lo(y),
		                           bignum_tmp1);
		x = integer_hi(x);
		y = integer_hi(y);
	}
}
