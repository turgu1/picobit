/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

// primitives-vector
// Builtin Indexes: 33..37

#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"
#include "testing.h"

#include "primitives.h"

PRIMITIVE(u8vector?, u8vector_p, 1, 33)
{
  if (IN_RAM(reg1)) {
    reg1 = ENCODE_BOOL(RAM_IS_VECTOR(reg1));
  }
  else if (IN_ROM(reg1)) {
    reg1 = ENCODE_BOOL(ROM_IS_VECTOR(reg1));
  }
  else {
    ERROR("u8vector?", "pointer in ROM or RAM");
    reg1 = FALSE;
  }
}

PRIMITIVE(#%make-u8vector, make_u8vector, 1, 34)
{
  reg1 = new_vector(decode_int(reg1));
}

PRIMITIVE(u8vector-ref, u8vector_ref, 2, 35)
{
  a2 = decode_int(reg2);

  if (IN_RAM(reg1)) {
    EXPECT(RAM_IS_VECTOR(reg1), "u8vector-ref.0", "vector");

    if (RAM_GET_VECTOR_LENGTH(reg1) <= a2) {
      ERROR("u8vector-ref.1", "Vector index invalid");
    }

    reg1 = RAM_GET_VECTOR_START(reg1);

    reg1 = encode_int(VECTOR_GET_BYTE(reg1, a2));
    reg2 = NIL;
  }
  else if (IN_ROM(reg1)) {
    EXPECT(ROM_IS_VECTOR(reg1), "u8vector-ref.2", "vector");

    if (ROM_GET_VECTOR_LENGTH(reg1) <= a2) {
      ERROR("u8vector-ref.3", "Vector index invalid");
    }

    reg1 = ROM_GET_VECTOR_START(reg1);

    while (a2--) {
      reg1 = ROM_GET_CDR(reg1);
    }

    reg1 = ROM_GET_CAR(reg1);
    reg2 = NIL;
    return;
  }
  else {
    TYPE_ERROR("u8vector-ref.4", "vector");
  }

}

PRIMITIVE_UNSPEC(u8vector-set!, u8vector_set, 3, 36)
  // TODO a lot in common with ref, abstract that
{
  a2 = decode_int(reg2); // TODO adapt for bignums
  a3 = decode_int(reg3);

  if ((a3 < 0) || (a3 > 255)) {
    ERROR("u8vector-set!.0", "byte vectors can only contain bytes");
  }

  if (IN_RAM(reg1)) {
    EXPECT(RAM_IS_VECTOR(reg1), "u8vector-set!.1", "vector");

    if (RAM_GET_VECTOR_LENGTH(reg1) <= a2) {
      ERROR("u8vector-set!.2", "vector index invalid");
    }

    a1 = RAM_GET_VECTOR_START(reg1);
  }
  else {
    TYPE_ERROR("u8vector-set!.3", "vector");
  }

  VECTOR_SET_BYTE(a1, a2, a3);

  reg1 = reg2 = reg3 = NIL;
}

PRIMITIVE(u8vector-length, u8vector_length, 1, 37)
{
  if (IN_RAM(reg1)) {
    EXPECT(RAM_IS_VECTOR(reg1), "u8vector-length.0", "vector");

    reg1 = encode_int(RAM_GET_VECTOR_LENGTH(reg1));
  }
  else if (IN_ROM(reg1)) {
    EXPECT(ROM_IS_VECTOR(reg1), "u8vector-length.1", "vector");

    reg1 = encode_int(ROM_GET_VECTOR_LENGTH(reg1));
  }
  else {
    TYPE_ERROR("u8vector-length.2", "vector");

    reg1 = ZERO;
  }
}


#if TESTS
void primitives_vector_tests()
{
  TESTM("primitives-vector");

  TEST("Make u8 vector");

    reg1 = encode_int(7);
    primitive_make_u8vector();
    cell_p v = reg1;
    EXPECT_TRUE(RAM_IS_VECTOR(v), "make_u8vector doesn't return a vector");
    EXPECT_TRUE(RAM_GET_VECTOR_LENGTH(v) == 7, "Vector length is wrong");

  TEST("u8 vector length");

    primitive_u8vector_length();
    EXPECT_TRUE(RAM_GET_VECTOR_LENGTH(v) == decode_int(reg1), "Vector length not returned properly");

  TEST("u8 vector predicate");

    reg1 = v;
    primitive_u8vector_p();
    EXPECT_TRUE(reg1 == TRUE, "Vector not recognised as such");

  TEST("u8 vector set! and ref");

    reg1 = v;
    reg2 = encode_int(2);
    reg3 = encode_int(23);
    primitive_u8vector_set();

    reg1 = v;
    reg2 = encode_int(2);
    primitive_u8vector_ref();

    EXPECT_TRUE(decode_int(reg1) == 23, "unable to set and ref a vector entry");
}
#endif
