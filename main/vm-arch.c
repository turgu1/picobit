/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#include "esp32-scheme-vm.h"

#define VM_ARCH
#include "vm-arch.h"

#include "mm.h"
#include "testing.h"

void vm_arch_init()
{
  keep_running = true;
}

cell_p pop()
{
  #if DEBUGGING
    // all the cons linking stack elements together are from the RAM Heap.
    if ((env != NIL) && !RAM_IS_PAIR(env)) {
      FATAL("pop.0", "HEAP BROKEN!!!");
    }
  #endif

  if (env == NIL) {
    #if DEBUGGING
      WARNING("pop.1", "Environment exhausted");
    #endif

    return NIL;
  }

  cell_p p = RAM_GET_CAR(env);

  cell_p f = env; // freed cell to be returned to the free list
  env = RAM_GET_CDR(env);

  #if 0 //DEBUGGING
    if (is_free(f) || (f >= ram_heap_size)) {
      FATAL("pop.2", "GC Broken");
    }
    if ((p < ram_heap_size) && is_free(p)) {
      FATAL("pop.3", "GC Broken");
    }
  #endif
  return_to_free_list(f);

  return p;
}

cell_p new_closure(cell_p env, code_p code)
{
  cell_p p = mm_new_ram_cell();

  EXPECT((env == NIL) || RAM_IS_PAIR(env), "new_closure.0", "pair");

  RAM_SET_TYPE(p, CLOSURE_TYPE);
  RAM_SET_CLOSURE_ENV(p, env);
  RAM_SET_CLOSURE_ENTRY_POINT(p, code);

  return p;
}

cell_p new_pair(cell_p car, cell_p cdr)
{
  cell_p p = mm_new_ram_cell();

  RAM_SET_TYPE(p, CONS_TYPE);
  RAM_SET_CAR(p, car);
  RAM_SET_CDR(p, cdr);

  return p;
}

cell_p new_cont(cell_p parent, cell_p closure)
{
  cell_p p = mm_new_ram_cell();

  EXPECT(RAM_IS_CLOSURE(closure), "new_cont.0", "closure");
  EXPECT((parent == NIL) || RAM_IS_CONTINUATION(parent), "new_cont.1", "continuation");

  RAM_SET_TYPE(p, CONTINUATION_TYPE);
  RAM_SET_CONT_CLOSURE(p, closure);
  RAM_SET_CONT_PARENT(p, parent);

  return p;
}

cell_p new_string(cell_p chars_p)
{
  cell_p p = mm_new_ram_cell();

  RAM_SET_TYPE(p, STRING_TYPE);
  RAM_STRING_SET_CHARS(p, chars_p);
  RAM_STRING_CLR_UNUSED(p);

  return p;
}

cell_p new_fixnum(int32_t value)
{
  cell_p p = mm_new_ram_cell();

  RAM_SET_TYPE(p, FIXNUM_TYPE);
  RAM_SET_FIXNUM_VALUE(p, value);

  return p;
}

cell_p new_bignum(int16_t value, cell_p high)
{
  cell_p p = mm_new_ram_cell();

  RAM_SET_TYPE(p, BIGNUM_TYPE);
  RAM_SET_BIGNUM_VALUE(p, value);
  RAM_SET_BIGNUM_HI(p, high);

  return p;
}

cell_p new_vector(uint16_t length)
{
  // As mm_new_vector_cell may call garbage collection, it is required
  // to use a reg to save the allocated cell during potential gc() call.

  reg4 = mm_new_ram_cell();

  RAM_SET_TYPE(reg4, VECTOR_TYPE);
  RAM_SET_VECTOR_START(reg4, mm_new_vector_cell(length, reg4));
  RAM_SET_VECTOR_LENGTH(reg4, length);

  cell_p p = reg4;
  reg4 = NIL;

  return p;
}

// Can decode up to 24 bits
int32_t decode_int(cell_p p)
{
  int val;

  if (p >= SMALL_INT_START) {
    if ((p == NIL) || (p == FALSE)) {
      val = 0;
    }
    else if (p == TRUE) {
      val = 1;
    }
    else {
      EXPECT(IS_SMALL_INT(p), "decode_int.0", "snall int");
      val = SMALL_INT_VALUE(p);
    }
  }
  else if (IN_RAM(p)) {
    EXPECT(RAM_IS_BIGNUM(p), "decode_int.1", "bignum");
    EXPECT(IS_SMALL_INT(RAM_GET_BIGNUM_HI(p)), "decode_int.3", "small int");
    val = RAM_GET_BIGNUM_VALUE(p) + (SMALL_INT_VALUE(RAM_GET_BIGNUM_HI(p)));
  }
  else if (IN_ROM(p)) {
    EXPECT(ROM_IS_BIGNUM(p), "decode_int.4", "bignum");
    EXPECT(IS_SMALL_INT(ROM_GET_BIGNUM_HI(p)), "decode_int.5", "small int");
    val = ROM_GET_BIGNUM_VALUE(p) + (SMALL_INT_VALUE(ROM_GET_BIGNUM_HI(p)));
  }
  else {
    TYPE_ERROR("decode_int.6", "fixnum");
    val = 0;
  }

  return val;
}

void decode_2_int_args ()
{
  a1 = decode_int(reg1);
  a2 = decode_int(reg2);
}

cell_p encode_int(int32_t val)
{
  if ((val >= -1) && (val <= 255)) {
    return ENCODE_SMALL_INT(val);
  }
  else {
    EXPECT(val < 0x07FFFFF, "encode_int", "Value too large");
    return new_bignum(val & 0x0000FFFF, ENCODE_SMALL_INT(val >> 16));
  }
}

void string2cstring(cell_p p, char *str, uint16_t length)
{
  EXPECT(IS_STRING(p), "string2cstring", "string");

  p = IN_RAM(p) ? RAM_STRING_GET_CHARS(p) : ROM_STRING_GET_CHARS(p);

  while ((length-- > 1) && (p != NIL)) {
    *str++ = (char) decode_int(GET_CAR(p));
    p = GET_CDR(p);
  }
  *str = 0;
}


#if TESTS
void vm_arch_tests()
{
  int i;
  cell_p p, q;

  vm_arch_init();

  TESTM("vm-arch");

  TEST("Cells Structure");

    p = new_pair(0x0203, 0x0201);
    EXPECT_TRUE(&RAM_GET_CAR(p) > &RAM_GET_CDR(p), "Cells Structure is wrong");
    EXPECT_TRUE((void *) &RAM_GET_BITS(p) > (void *) &RAM_GET_CAR(p), "Cells Structure is wrong (type address must be larger than car address)");

  TEST("new_pair()");

    p = new_pair(NIL, FALSE);
    EXPECT_TRUE(RAM_IS_PAIR(p), "new_pair() does't not return a cons");
    EXPECT_TRUE(RAM_GET_CAR(p) == NIL, "CAR not properly set");
    EXPECT_TRUE(RAM_GET_CDR(p) == FALSE, "CDR not properly set");

  TEST("pop()");

    env = NIL;
    // A warning message will appear on console if debugging enabled
    EXPECT_TRUE(pop() == NIL, "Empty environment doesn't return NIL on pop()");

    env = new_pair(encode_int(1), encode_int(2));
    env = new_pair(p, env);

    q = pop();
    EXPECT_TRUE(RAM_GET_CAR(q) == NIL && RAM_GET_CDR(q) == FALSE, "pop() doesn't return the top env cell");

  TEST("new_closure()");


  TEST("new_cont()");


  TEST("new_fixnum()");


  TEST("new_bignum()");
  // Not yet

  TEST("new_string()");


  TEST("new_cstring()");
  // Not yet

  TEST("new_vector()");


  TEST("encode_int()");

    for (i = -1; i < 256; i++) {
      EXPECT_TRUE(encode_int(i) == (SMALL_INT_START + i + 1), "Small Int encoding wrong");
    }
    p = encode_int(1000);
    EXPECT_TRUE(RAM_IS_BIGNUM(p), "Integer encoding does not return a FIXNUM");
    EXPECT_TRUE(decode_int(p) == 1000, "Expected encoded value is not 1000");

  TEST("decode_int()");

    for (q = SMALL_INT_START, i = -1; q <= SMALL_INT_MAX; q++, i++) {
      EXPECT_TRUE(decode_int(q) == i, "Small Int decoding wrong");
    }
    EXPECT_TRUE(decode_int(p) == 1000, "Expected decoded value is not 1000");

  env = NIL;
  mm_gc();
}
#endif
