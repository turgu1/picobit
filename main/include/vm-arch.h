/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#ifndef VM_ARCH_H
#define VM_ARCH_H

#ifdef VM_ARCH
# define PUBLIC
#else
# define PUBLIC extern
#endif

/** VM Architecture Definition.

  The Virtual Machine definition is composed of the following elements:

    Cells definitions
    Memory organisation
    Globals
    Standard constants
    VM Registers
    Machine instructions

 */

/**

Cells Definitions.

  Every cell is 40 bits long (5 bytes): Two 16 bits fields for values and one 8 bits
  field for flags and type information. To get proper memory compaction for the
  structure defining a cell, the following option must be used for GCC:

     -fpack-struct=1

  It is defined as a pragma in file inc/esp32-scheme-vm.h as follow:

     #pragma pack(1)

  On the ESP32, adress misalignment of 16 and 32 bit values doesn't seem to
  create any issue with the performance of the code. In any case, RAM heap cells are
  split in two separate vectors: ram_heap_data and ram_heap_flags. Each entry in
  the ram_heap_data vector is 4 bytes long. Each entry in the ram_heap_flasg is 1
  byte. As the ESP32 RAM memory is not continuous, this arrangement will help in
  having the largest amount of cells for the heap.

  The address space of the virtual heap cannot go beyond (64k * 5). To address
  64K cells, we need 16 bits (2ˆ16 = 64k).

  - The 2 first bits are not used by the interpreter but are defined as
    user_1 and user_2 flags. New primitives are allowed to use them. They are
    protected by the garbage collector.
  - The 4 following bits are the type of a cell
  - The 2 following bits are the GC bits
  - The rest of the cell depends of the type of cell

  Types 0 .. 7 are reserved for cell types having car pointing at other
  cells. Types 0 .. 3 are reserved for cells having cdr pointing at other
  cells. These are to optimize mm_mask() of the gc procedure. This must
  be always respected to insure proper memory management.

  We define the following cell types:

   cons (or pair)

      CAR and CDR are indexes in the heap

      +----+------+----+---------------+----------------+
      | 00 | 0000 | GC |      CAR      |       CDR      |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   continuation

      +----+------+----+---------------+----------------+
      | 00 | 0001 | GC |    PARENT     |   CLOSURE IDX  |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   closure

      +----+------+----+---------------+----------------+
      | 00 | 0100 | GC |  ENVIRONMENT  |    CODE PTR    |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   bignum

      A Bignum is composed of a list of 16 bits signed numerical parts, least
      significant portion is first.

      +----+------+----+---------------+----------------+
      | 00 | 0101 | GC |     NEXT      |    NUM PART    |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   string

      A string is a list of chars cells and is located in RAM in the heap.
      A zero length string will have NIL into chars ptr.

      +----+------+----+---------------+----------------+
      | 00 | 0110 | GC |  CHARS LIST   |      NIL       |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   fixnum

      +----+------+----+--------------------------------+
      | 00 | 1000 | GC |             VALUE              |
      +----+------+----+--------------------------------+
         2     4     2                 32

  cstring

      For now, no cstring are present. The following will be implemented later:

      A string constant is a null terminated string located in the ROM space,
      in the cstring pool (?).

      +----+------+----+---------------+----------------+
      | 00 | 1011 | GC |   CHARS PTR   |       0        |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   vector

      Lenght is in bytes (max 64k). To content the index
      (4 bytes aligned) in the vector space.

      +----+------+----+---------------+----------------+
      | 00 | 1100 | GC |    LENGTH     |   TO CONTENT   |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   symbol

      For now, no symbols information are present. Its address in the ROM
      constant area insure its uniqueness. A symbol as a constant will be showned
      that way:

      +----+------+----+---------------+----------------+
      | 00 | 1101 | GC |      NIL      |       NIL      |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

      The following could be implemented later:

      The Symbol table is in the ROM area. The char list is an index (byte index
      in the constant string pool) to a null terminated string containing the
      human representation of the symbol. Next point to the next symbol cell in
      the symbol table. The chars list pointer in unique for each symbol.

      +----+------+----+---------------+----------------+
      | 00 | 1101 | GC |   CHARS LST   |       NEXT     |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

 Memory organisation

  The ESP32 possesses 520 KBytes of RAM. This memory space offers interesting
  opportunities for running substantial Scheme programs, compared to other 8
  bits CPUs. However, as this space is not contiguous, it requires some
  flexibility to organise the memory requirements for a scheme interpreter.

  Splitting RAM heap cells in two parts (data and flags), at this point in
  time, we are able to run using 42 K RAM heap cells (~210 KBytes) and 60 KBytes
  for the u8vector space.

  A Scheme program is composed of code, constants, global variables, a heap of
  dynamically created data and contiguous space for byte vectors.

  Ram memory is organized to receive the following chunks:

    * data heap as two C vectors: flags and data
    * byte vectors content as one C vector
    * global variables, located at the beginning of the data heap

  The picobit binary program, once it as been compiled, is merged with the
  interpreter machine code before it is burned on the ESP32. It is then possible
  to have up to 64 KBytes of scheme code and constants, the chip allowing far more
  coding space. The 64 KBytes of scheme code is a compromise to keep the compact
  Scheme machine code as it was designed in the original PicoBit.

  The scheme code requires access to all data through various means: virtual adresses,
  global variable numbers, vector entries through indexes.

  RAM heap and ROM constant spaces are addressed with indexes in separate vectors.
  The address space is divided in three zone:

    0x0000 - 0xDFFF: RAM Heap Space
    0xE000 - 0xFDFF: ROM Constants Space
    0xFE00 - 0xFFFF: Coded small ints, true, false and ()

  With this addressing organisation, we are then limited to a theoritical
  maximum of 57344 cells for the RAM Heap and 7680 cells for the ROM constants.

 */

#define         CONS_TYPE   0
#define CONTINUATION_TYPE   1
#define      CLOSURE_TYPE   4
#define       BIGNUM_TYPE   5
#define       STRING_TYPE   6

#define       FIXNUM_TYPE   8
#define      CSTRING_TYPE  11
#define       VECTOR_TYPE  12
#define       SYMBOL_TYPE  13

// Easy enough: all pointer elements in cells are 16 bit long. So we define here
// the various pointer types as uint16...

typedef uint16_t IDX;

typedef IDX cell_p;
typedef IDX vector_p;
typedef IDX code_p; // TODO: Revisit....

typedef struct {
  cell_p cdr_p;
  cell_p car_p;
} pair_part;

typedef struct {
  cell_p closure_p;
  cell_p parent_p;
} continuation_part;

typedef struct {
  code_p entry_point_p;
  cell_p environment_p;
} closure_part;

typedef struct {
  int32_t value;
} fixnum_part;

typedef struct {
  int16_t num_part;
  cell_p next_p;
} bignum_part;

typedef struct {
  cell_p unused ;
  cell_p chars_p;
} string_part;

typedef struct {
  cell_p next_p;
  IDX chars_idx ;
} cstring_part;

typedef struct {
  vector_p start_p;
  uint16_t length; // in bytes
} vector_part;

typedef struct {
  cell_p next_p;
  IDX chars_idx;
} symbol_part;

typedef struct {
  union {
            pair_part cons;
    continuation_part continuation;
         closure_part closure;
          fixnum_part fixnum;
          bignum_part bignum;
          string_part string;
         cstring_part cstring;
          vector_part vector;
          symbol_part symbol;
  };
  union {
    struct {
      unsigned int gc_mark : 1;
      unsigned int gc_flip : 1;
      unsigned int    type : 4;
      unsigned int  user_1 : 1;
      unsigned int  user_2 : 1;
    };
    uint8_t bits;
  };
} cell;

typedef union {
            pair_part cons;
    continuation_part continuation;
         closure_part closure;
          fixnum_part fixnum;
          bignum_part bignum;
          string_part string;
         cstring_part cstring;
          vector_part vector;
          symbol_part symbol;
} cell_data;

typedef union {
  struct {
    unsigned int gc_mark : 1;
    unsigned int gc_flip : 1;
    unsigned int    type : 4;
    unsigned int  user_1 : 1;
    unsigned int  user_2 : 1;
  };
  uint8_t bits;
} cell_flags;

typedef          cell * cell_ptr;
typedef    const cell * rom_cell_ptr;
typedef     cell_data * cell_data_ptr;
typedef    cell_flags * cell_flags_ptr;
typedef const uint8_t * pgm_ptr;
typedef const uint16_t * pgm_sptr;

/** Globals.

  Global variables are located at the beginning of the RAM Heap. As this
  heap is allocated on cell boundaries, we are putting 2 globals per cell.
  Global access instructions limit to 256 variables.
 */

/** Standard Constants.

  To mitigate the amount of generated cells, standard constants are defined
  in the address space. Addresses 0xFE00 to 0xFFFF are reserved for
  predefined constants as follows:

  0xFE00 .. 0xFF00 : -1 to 255
  0xFFFD           : #f
  0xFFFE           : #t
  0xFFFF           : NIL

  ROM Space constants are receiving addresses starting at 0xE000.

  For the LDCS instruction, values are coded on 5 bits as follows:

  +---------+----------+------------------+
  |   ccccc |  result  | addr mapping     |
  +---------+----------+------------------+
  |       0 |       #f | 0xFFFD           |
  |       1 |       #t | 0xFFFE           |
  |       2 |      NIL | 0xFFFF           |
  | 3 .. 31 | -1 .. 27 | 0xFE00 .. 0xFE1C |
  +---------+----------+------------------+

  For the LDC instruction, values are coded on 12 bits as follows:

  +--------------+-----------+------------------+
  | cccccccccccc |   result  | addr mapping     |
  +--------------+-----------+------------------+
  |            0 |        #f | 0xFFFD           |
  |            1 |        #t | 0xFFFE           |
  |            2 |       NIL | 0xFFFF           |
  |     3 .. 259 | -1 .. 255 | 0xFE00 .. 0xFF00 |
  | 260 ..       |   ROM IDX | 0xE000 ..        |
  +--------------+-----------+------------------+

*/

#define FALSE ((cell_p) 0xFFFD)
#define TRUE  ((cell_p) 0xFFFE)
#define NIL   ((cell_p) 0xFFFF)
#define ZERO  ((cell_p) 0xFE01)
#define NEG1  ((cell_p) 0xFE00)
#define POS1  ((cell_p) 0xFE02)

/** Instructions.

  The instructions are the same as the PicoBit original virtual machine.
  Customary assembly language names have been defined to help distinguish
  virtual machine instructions from scheme functions from traces generated by
  the interpreter.

  LDCS    Load immediate small constant to TOS
          000ccccc

  LDSTK   Load stack to TOS
          001nnnnn

  LDS     Load global value to TOS, located at the beginning of the RAM Heap
          Space. Only the first 16 entries are accessible through this
          instruction. iiii is an index in the heap space.
          0100iiii

  STS     Store TOS to global variable. Only the first 16 entries are accessible
          through this instruction. iiii is an index in the heap space.
          0101iiii

  CALLC   Call procedure with closure on TOS, n is the number of arguments
          0110nnnn

  JUMPC   Jump to procedure with closure on TOS, n is the number of arguments
          0111nnnn

  JUMPS   Jump to procedure at adress pc + x
          1000xxxx

  BRSF    Branch to location pc + x if TOS is false
          1001xxxx

  LDC     Load immediate pre-defined constant to TOS
          1010cccc cccccccc

  CALL    Call top-level procedure at entry point a.
          10110000 aaaaaaaa aaaaaaaa

  JUMP    Jump to top-level procedure at entry point a.
          10110001 aaaaaaaa aaaaaaaa

  BR      Unconditional Branch to location
          10110010 aaaaaaaa aaaaaaaa

  BRF     Branch to location if TOS is false
          10110011 aaaaaaaa aaaaaaaa

  CLOS    Build closure from entry point a.
          10110100 aaaaaaaa aaaaaaaa

  CALLR   Call procedure at location pc + a - 128
          10110101 aaaaaaaa

  JUMPR   Jump to procedure at location pc + a - 128
          10110110 aaaaaaaa

  BRR     Unconditional Branch to location pc + a - 128
          10110111 aaaaaaaa

  BRRF    Branch to location  pc + a - 128 if TOS is false
          10111000 aaaaaaaa

  CLOSR   Build closure from entry point pc + a - 128
          10111001 aaaaaaaa

  LD      Load global value to TOS, located at the beginning of the RAM Heap
          Space. iiiiiiii is an index in the heap space.
          10111110 iiiiiiii

  ST      Store TOS to global variable, located at the beginning of the RAM Heap
          Space. iiiiiiii is an index in the heap space.
          10111111 iiiiiiii
  */

#define INSTR_LDCS1                ((uint8_t) 0x00)
#define INSTR_LDCS2                ((uint8_t) 0x10)
#define INSTR_LDSTK1               ((uint8_t) 0x20)
#define INSTR_LDSTK2               ((uint8_t) 0x30)
#define INSTR_LDS                  ((uint8_t) 0x40)
#define INSTR_STS                  ((uint8_t) 0x50)
#define INSTR_CALLC                ((uint8_t) 0x60)
#define INSTR_JUMPC                ((uint8_t) 0x70)
#define INSTR_JUMPS                ((uint8_t) 0x80)
#define INSTR_BRSF                 ((uint8_t) 0x90)
#define INSTR_LDC                  ((uint8_t) 0xA0)
#define INSTR_CALL                 ((uint8_t) 0xB0)
#define INSTR_JUMP                 ((uint8_t) 0xB1)
#define INSTR_BR                   ((uint8_t) 0xB2)
#define INSTR_BRF                  ((uint8_t) 0xB3)
#define INSTR_CLOS                 ((uint8_t) 0xB4)
#define INSTR_CALLR                ((uint8_t) 0xB5)
#define INSTR_JUMPR                ((uint8_t) 0xB6)
#define INSTR_BRR                  ((uint8_t) 0xB7)
#define INSTR_BRRF                 ((uint8_t) 0xB8)
#define INSTR_CLOSR                ((uint8_t) 0xB9)
#define INSTR_LD                   ((uint8_t) 0xBE)
#define INSTR_ST                   ((uint8_t) 0xBF)

#define PRIMITIVE1                 ((uint8_t) 0xC0)
#define PRIMITIVE2                 ((uint8_t) 0xD0)
#define PRIMITIVE3                 ((uint8_t) 0xE0)
#define PRIMITIVE4                 ((uint8_t) 0xF0)

#define MAX_SMALL_INT_VALUE        255
#define MIN_SMALL_INT_VALUE        -1

#define SMALL_INT_START            ((IDX) 0xFE00)
#define SMALL_INT_MAX              ((IDX) 0xFE00 + 256)
#define SMALL_INT_MASK             0x01FF

#define SMALL_INT_VALUE(p)         (((int16_t)(p & SMALL_INT_MASK)) - 1)
#define ENCODE_SMALL_INT(v)        ((cell_p) ((v) + 1) | SMALL_INT_START)

#define ROM_START_ADDR             ((IDX) 0xE000)
#define ROM_MAX_ADDR               ((IDX) 0xFE00)
#define ROM_IDX(p)                 (p & 0x1FFF)

#define RAM_IS_PAIR(p)             (ram_heap_flags[p].type ==         CONS_TYPE)
#define RAM_IS_CONTINUATION(p)     (ram_heap_flags[p].type == CONTINUATION_TYPE)
#define RAM_IS_CLOSURE(p)          (ram_heap_flags[p].type ==      CLOSURE_TYPE)
#define RAM_IS_FIXNUM(p)           (ram_heap_flags[p].type ==       FIXNUM_TYPE)
#define RAM_IS_BIGNUM(p)           (ram_heap_flags[p].type ==       BIGNUM_TYPE)
#define RAM_IS_STRING(p)           (ram_heap_flags[p].type ==       STRING_TYPE)
#define RAM_IS_CSTRING(p)          (ram_heap_flags[p].type ==      CSTRING_TYPE)
#define RAM_IS_VECTOR(p)           (ram_heap_flags[p].type ==       VECTOR_TYPE)
#define RAM_IS_SYMBOL(p)           (ram_heap_flags[p].type ==       SYMBOL_TYPE)
#define RAM_IS_NUMBER(p)           (RAM_IS_FIXNUM(p) || RAM_IS_BIGNUM(p))

#define ROM_IS_PAIR(p)             (rom_heap[ROM_IDX(p)].type ==         CONS_TYPE)
#define ROM_IS_CONTINUATION(p)     (rom_heap[ROM_IDX(p)].type == CONTINUATION_TYPE)
#define ROM_IS_CLOSURE(p)          (rom_heap[ROM_IDX(p)].type ==      CLOSURE_TYPE)
#define ROM_IS_FIXNUM(p)           (rom_heap[ROM_IDX(p)].type ==       FIXNUM_TYPE)
#define ROM_IS_BIGNUM(p)           (rom_heap[ROM_IDX(p)].type ==       BIGNUM_TYPE)
#define ROM_IS_STRING(p)           (rom_heap[ROM_IDX(p)].type ==       STRING_TYPE)
#define ROM_IS_CSTRING(p)          (rom_heap[ROM_IDX(p)].type ==      CSTRING_TYPE)
#define ROM_IS_VECTOR(p)           (rom_heap[ROM_IDX(p)].type ==       VECTOR_TYPE)
#define ROM_IS_SYMBOL(p)           (rom_heap[ROM_IDX(p)].type ==       SYMBOL_TYPE)
#define ROM_IS_NUMBER(p)           (ROM_IS_FIXNUM(p) || ROM_IS_BIGNUM(p))

#define IN_RAM(p)                  (p >= reserved_cells_count) && (p < ram_heap_size)
#define IN_ROM(p)                  (p >= ROM_START_ADDR) && (p < ROM_MAX_ADDR)

#define IS_SMALL_INT(p)            ((p >= SMALL_INT_START) && (p <= SMALL_INT_MAX))
#define IS_BIGNUM(p)               ((IN_RAM(p) && RAM_IS_BIGNUM(p)) || (IN_ROM(p) && ROM_IS_BIGNUM(p)))
#define IS_NUMBER(p)               IS_SMALL_INT(p) || IS_BIGNUM(p)

#define IS_STRING(p)               ((IN_RAM(p) && RAM_IS_STRING(p)) || (IN_ROM(p) && ROM_IS_STRING(p)))

#define RAM_GET_TYPE(p)            ram_heap_flags[p].type
#define RAM_SET_TYPE(p, t)         ram_heap_flags[p].type = t

#define RAM_GET_BITS(p)            ram_heap_flags[p].bits

#define RAM_GET_CAR(p)             ram_heap_data[p].cons.car_p
#define RAM_GET_CDR(p)             ram_heap_data[p].cons.cdr_p

#define RAM_SET_CAR(p, v)          ram_heap_data[p].cons.car_p = v
#define RAM_SET_CDR(p, v)          ram_heap_data[p].cons.cdr_p = v

#define ROM_GET_CAR(p)             rom_heap[ROM_IDX(p)].cons.car_p
#define ROM_GET_CDR(p)             rom_heap[ROM_IDX(p)].cons.cdr_p

#define RAM_IS_MARKED(p)           (ram_heap_flags[p].gc_mark == 1)
#define RAM_IS_NOT_MARKED(p)       (ram_heap_flags[p].gc_mark == 0)
#define RAM_SET_MARK(p)            ram_heap_flags[p].gc_mark = 1
#define RAM_CLR_MARK(p)            ram_heap_flags[p].gc_mark = 0

#define RAM_IS_FLIPPED(p)          (ram_heap_flags[p].gc_flip == 1)
#define RAM_SET_FLIP(p)            ram_heap_flags[p].gc_flip = 1
#define RAM_CLR_FLIP(p)            ram_heap_flags[p].gc_flip = 0

#define RAM_IS_USER_1_SET(p)       (ram_heap_flags[p].user_1 == 1)
#define RAM_SET_USER_1(p)          ram_heap_flags[p].user_1 = 1
#define RAM_CLR_USER_1(p)          ram_heap_flags[p].user_1 = 0

#define RAM_IS_USER_2_SET(p)       (ram_heap_flags[p].user_2 == 1)
#define RAM_SET_USER_2(p)          ram_heap_flags[p].user_2 = 1
#define RAM_CLR_USER_2(p)          ram_heap_flags[p].user_2 = 0

#define HAS_NO_RIGHT_LINK(p)       ((ram_heap_flags[p].bits & 0x30) != 0)
#define HAS_RIGHT_LINK(p)          ((ram_heap_flags[p].bits & 0x30) == 0)
#define HAS_LEFT_LINK(p)           ((ram_heap_flags[p].bits & 0x20) == 0)

#define IS_PAIR(p) ((IN_RAM(p) && RAM_IS_PAIR(p)) || (IN_ROM(p) && ROM_IS_PAIR(p)))
#define GET_CAR(p) IN_RAM(p) ? RAM_GET_CAR(p) : ROM_GET_CAR(p)
#define GET_CDR(p) IN_RAM(p) ? RAM_GET_CDR(p) : ROM_GET_CDR(p)

// Fixnum

#define RAM_GET_FIXNUM_VALUE(p)     ram_heap_data[p].fixnum.value
#define ROM_GET_FIXNUM_VALUE(p)     rom_heap[ROM_IDX(p)].fixnum.value

#define RAM_SET_FIXNUM_VALUE(p, v)  ram_heap_data[p].fixnum.value = v

// Bignum

#define RAM_GET_BIGNUM_VALUE(p)     ram_heap_data[p].bignum.num_part
#define ROM_GET_BIGNUM_VALUE(p)     rom_heap[ROM_IDX(p)].bignum.num_part

#define RAM_GET_BIGNUM_HI(p)        ram_heap_data[p].bignum.next_p
#define ROM_GET_BIGNUM_HI(p)        rom_heap[ROM_IDX(p)].bignum.next_p

#define RAM_SET_BIGNUM_HI(p,h)      ram_heap_data[p].bignum.next_p = h
#define RAM_SET_BIGNUM_VALUE(p,v)   ram_heap_data[p].bignum.num_part = v

// Vector

#define RAM_GET_VECTOR_LENGTH(p)    ram_heap_data[p].vector.length
#define ROM_GET_VECTOR_LENGTH(p)    rom_heap[ROM_IDX(p)].vector.length

#define RAM_SET_VECTOR_LENGTH(p, v) ram_heap_data[p].vector.length = v

#define RAM_SET_VECTOR_START(p, v)  ram_heap_data[p].vector.start_p = v
#define RAM_GET_VECTOR_START(p)     ram_heap_data[p].vector.start_p
#define ROM_GET_VECTOR_START(p)     rom_heap[ROM_IDX(p)].vector.start_p

#define VECTOR_GET_LENGTH(p)        vector_heap[p].vector.length
#define VECTOR_GET_RAM_PTR(p)       vector_heap[p].vector.start_p

#define VECTOR_SET_LENGTH(p, l)     vector_heap[p].vector.length = l
#define VECTOR_SET_RAM_PTR(p, r)    vector_heap[p].vector.start_p = r

#define VECTOR_IS_USED(p)           (vector_heap[p].gc_mark == 1)
#define VECTOR_IS_FREE(p)           (vector_heap[p].gc_mark == 0)

#define VECTOR_SET_USED(p)          vector_heap[p].gc_mark = 1
#define VECTOR_SET_FREE(p)          vector_heap[p].gc_mark = 0

#define VECTOR_GET_BYTE(p, i)       *(((uint8_t *) &vector_heap[p]) + i)
#define VECTOR_SET_BYTE(p, i, b)    *(((uint8_t *) &vector_heap[p]) + i) = b

// String

#define RAM_STRING_GET_CHARS(p)    ram_heap_data[p].string.chars_p
#define ROM_STRING_GET_CHARS(p)    rom_heap[ROM_IDX(p)].string.chars_p

#define RAM_STRING_SET_CHARS(p, v) ram_heap_data[p].string.chars_p = v
#define RAM_STRING_CLR_UNUSED(p)   ram_heap_data[p].string.unused = 0


// Continuation

#define RAM_GET_CONT_CLOSURE(p)    ram_heap_data[p].continuation.closure_p
#define RAM_GET_CONT_PARENT(p)     ram_heap_data[p].continuation.parent_p

#define RAM_SET_CONT_CLOSURE(p, v) ram_heap_data[p].continuation.closure_p = v
#define RAM_SET_CONT_PARENT(p,v)   ram_heap_data[p].continuation.parent_p = v

// Closure

#define RAM_GET_CLOSURE_ENV(p)            ram_heap_data[p].closure.environment_p
#define RAM_GET_CLOSURE_ENTRY_POINT(p)    ram_heap_data[p].closure.entry_point_p

#define RAM_SET_CLOSURE_ENV(p, v)         ram_heap_data[p].closure.environment_p = v
#define RAM_SET_CLOSURE_ENTRY_POINT(p, v) ram_heap_data[p].closure.entry_point_p = v

// Globals

#define GLOBAL_GET(i)            ((i & 1) ? RAM_GET_CAR(i >> 1) : RAM_GET_CDR(i >> 1))
#define GLOBAL_SET(i, p)         if (i & 1) RAM_SET_CAR(i >> 1, p); else RAM_SET_CDR(i >> 1, p)

#define ENCODE_BOOL(bool_value)  (bool_value) ? TRUE : FALSE

/** VM Registers.

 env          index on environments. The first one is from de code (Read only)
 cont         index on continuations
 entry        index in code of a procedure entry point
 reg1 .. reg4 registers for function parameters and evaluation
 pc           program counter (pointer into ROM code space)

 Golden rules for those who want to create their own primitives or want to
 modify the virtual machine:

 1. Never allocate new cells without having them pointed directly or indirectly
    by one of the registers (env cont reg1 .. reg4) or global variables. When
    garbage collection is fired, only the cells connected to these registers
    and the global variables are kept. All the other cells are put back to the
    free list and their content will be lost.

  2. Never used registers (env cont reg1 .. reg4) and global variables as
     scratch space for anything else than scheme cells adresses or encoded
     values. The garbage collector could become mixed up if those registers
     contain values that are not related to the address space and encoded values
     managed by the virtual machine.

  3. Never directly access virtual machine union and structure defined in file
     vm-arch.h. Use the macro defined in that same file to access those
     structures. This will ensure coherence on changes made to the virtual
     machine internals.

  In the preceding paragraphs, global values are the ones generated by the
  compiler and located in the RAM heap starting at index 0 (two global
  variables per cell).

 */

PUBLIC cell_p env, cont, reg1, reg2, reg3, reg4;
PUBLIC code_p entry;
PUBLIC pgm_ptr program;
PUBLIC uint16_t max_addr;
PUBLIC int32_t a1, a2, a3;

PUBLIC union {
  pgm_ptr c;
  pgm_sptr s;
} pc, last_pc;

PUBLIC void vm_arch_init();

PUBLIC cell_p pop();

PUBLIC cell_p new_closure(cell_p env, code_p code);
PUBLIC cell_p    new_cont(cell_p parent, cell_p closure);
PUBLIC cell_p    new_pair(cell_p car, cell_p cdr);
PUBLIC cell_p  new_string(cell_p chars_p);
PUBLIC cell_p  new_fixnum(int32_t value);
PUBLIC cell_p  new_bignum(int16_t lo, cell_p high);
PUBLIC cell_p  new_vector(uint16_t length);

PUBLIC int32_t decode_int(cell_p p);
PUBLIC cell_p  encode_int(int32_t val);

PUBLIC void string2cstring(cell_p p, char *str, uint16_t length);

PUBLIC void decode_2_int_args();

PUBLIC uint16_t global_count;
PUBLIC uint8_t reserved_cells_count;

PUBLIC cell_data_ptr  ram_heap_data;   // read/write
PUBLIC cell_flags_ptr ram_heap_flags;  // read/write
PUBLIC cell_p         ram_heap_end;
PUBLIC IDX            ram_heap_size;   // as a number of cells

PUBLIC rom_cell_ptr rom_heap;              // read only

PUBLIC cell_ptr vector_heap;
PUBLIC IDX      vector_heap_size;


PUBLIC volatile bool keep_running; // Used to stop the interpreter

#undef PUBLIC
#endif
