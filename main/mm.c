#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "testing.h"
#include "bignum.h"

#include <string.h>

#if STATISTICS
  #include <time.h>
#endif

#define MM
#include "mm.h"

#if DEBUGGING
  uint16_t free_allocated_count;
#endif

extern void show(cell_p p);

/** Deutsch-Schorr-Waite Garbage Collection.
 */

#if 0
// Mark algorithm from the original PicoBit. Can't get it work without
// fragmentation.
void mm_mark(cell_p p)
{
 	/* mark phase */

 	cell_p stack;
 	cell_p visit;

 	if (p < ram_heap_end) {
 		visit = NIL;

 push:
 		stack = visit;
 		visit = p;

 		if (HAS_LEFT_LINK(visit) && RAM_IS_MARKED(visit)) {
 		} else {
 			if (HAS_RIGHT_LINK(visit)) { // pairs and continuations

 				p = RAM_GET_CDR(visit);

 				if (p < ram_heap_end) {
 					RAM_SET_FLIP(visit);
 					RAM_SET_CDR(visit, stack);
 					goto push;
 				}

 				goto visit_field1;
 			}

 			if (HAS_LEFT_LINK(visit)) {

 visit_field1:
 				p = RAM_GET_CAR(visit);

 				if (p < ram_heap_end) {
 					RAM_SET_MARK(visit);
 					RAM_SET_CAR(visit, stack);
 					goto push;
 				}
 			}

 			RAM_SET_MARK(visit);
 		}

 pop:
 		if (stack < ram_heap_end) {
 			if (HAS_RIGHT_LINK(stack) && RAM_IS_FLIPPED(stack)) {
 				p = RAM_GET_CDR(stack);  /* pop through cdr */
 				RAM_SET_CDR(stack, visit);
 				visit = stack;
 				stack = p;

 				RAM_CLR_FLIP(visit);
 				// we unset the "1-left" bit

 				goto visit_field1;
 			}

 			p = RAM_GET_CAR(stack);  /* pop through car */
 			RAM_SET_CAR(stack, visit);
 			visit = stack;
 			stack = p;

 			goto pop;
 		}
 	}
}

#else

// My own design. No fragmentation. (GT)
// Seems to work very well. Any idea to get it better??
void mm_mark(cell_p p)
{
  cell_p current = p;
  cell_p prev    = NIL;
  cell_p next;

  for (;;) {
    while ((current < ram_heap_end) && RAM_IS_NOT_MARKED(current)) {
      RAM_SET_MARK(current);
      #if STATISTICS
        used_cells_count++;
      #endif
      if (HAS_LEFT_LINK(current)) {
        next = RAM_GET_CAR(current);
        RAM_SET_CAR(current, prev);
        prev = current;
        current = next;
      }
    }

    // Here, current left branch of the subtree is completed. We go up until we
    // find a node for which a right link was not processed or has no right
    // link.
    while ((prev < ram_heap_end) && RAM_IS_FLIPPED(prev)) {
      RAM_CLR_FLIP(prev);
      next = RAM_GET_CDR(prev); // next is the upper node
      RAM_SET_CDR(prev, current); // re-establish the link down
      current = prev;
      prev    = next;
    }

    if (prev >= ram_heap_end) break;

    // If we found a node with a right branch, mark it for return and
    // go down that branch to process its own left path (next loop).
    if (HAS_RIGHT_LINK(prev)) {
      next = RAM_GET_CAR(prev);
      RAM_SET_CAR(prev, current);
      RAM_SET_FLIP(prev);
      current = RAM_GET_CDR(prev);
      RAM_SET_CDR(prev, next);
    }
    else {
      // We go up until a node with a right link or top of the tree is detected
      while ((prev < ram_heap_end) && HAS_NO_RIGHT_LINK(prev)) {
        next = RAM_GET_CAR(prev);
        RAM_SET_CAR(prev, current);
        current = prev;
        prev = next;
      }
    }
  }
}

#endif

#if DEBUGGING
  void unmark_ram()
  {
    cell_p p = ram_heap_end - 1;

    do {
      RAM_CLR_MARK(p);
    } while (p-- > 0);
  }

  bool is_free(cell_p p)
  {
    cell_p f = free_cells;
    while (f != NIL) {
      if (f == p) return true;
      f = RAM_GET_CDR(f);
    }
    return false;
  }
#endif

PRIVATE void mm_sweep()
{
  free_cells = NIL;

  #if DEBUGGING
    // Used to check for fragmentation the heap
    free_allocated_count = 0;
  #endif

  #if STATISTICS
    free_cells_count = 0;
  #endif

  cell_p p = ram_heap_end - 1;

  // Don't forget: p cannot be a negative number...
  do {
    if (RAM_IS_MARKED(p)) {
      RAM_CLR_MARK(p);
    }
    else {
      if (RAM_IS_VECTOR(p)) {
        VECTOR_SET_FREE(RAM_GET_VECTOR_START(p) - 1);
        RAM_SET_TYPE(p, CONS_TYPE);
      }
      RAM_SET_CDR(p, free_cells);
      free_cells = p;

      #if STATISTICS
        free_cells_count++;
      #endif
    }

    // Last loop will have p = reserved_cells_count...
    // This doesn't work: while (--p >= reserved_cells_count)
    //    as reserved_cells_count could be 0 and p is unsigned...
  } while (p-- > reserved_cells_count);

  // Reset mark bits in the globals area
  if (reserved_cells_count > 0) {
    do {
      RAM_CLR_MARK(p);
    } while (p-- > 0); // Last loop will have p = 0
  }
}

PRIVATE bool check_free_list(int count)
{
  cell_p next = free_cells;

  while (next != NIL) {
    count--;
    next = RAM_GET_CDR(next);
  }

  return count == reserved_cells_count;
}

void return_to_free_list(cell_p p)
{
  RAM_SET_TYPE(p, CONS_TYPE);
  RAM_SET_CDR(p, free_cells);
  free_cells = p;
}

void mm_gc()
{
  // INFO("mm_gc", "Garbage collection Started");
  E32(taskYIELD());

  #if STATISTICS
    used_cells_count = 0;
  #endif

  #if STATISTICS_GC
    gc_call_counter++;

    double gc_duration;
    clock_t start_time;
    clock_t end_time;
    start_time = clock();
  #endif

  for (uint8_t i = 0; i < reserved_cells_count; i++) mm_mark(i);

  mm_mark(reg1);
  mm_mark(reg2);
  mm_mark(reg3);
  mm_mark(reg4);
  mm_mark(cont);
  mm_mark(env);

  bignum_gc_mark();

  mm_sweep();

  #if STATISTICS_GC
    end_time = clock();
    gc_duration = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    if (gc_duration > max_gc_duration) {
      max_gc_duration = gc_duration;
    }
  #endif

  #if DEBUGGING
    if ((used_cells_count + free_cells_count) != ram_heap_size) {
      WARNING(
        "mm_gc", "HEAP FRAGMENTATION (heap_size: %d, total: %d)",
        ram_heap_size,
        used_cells_count + free_cells_count);
    }
  #endif

  // INFO("mm_gc", "Garbage Collection Completed");
}

/**
  Vector space compaction. All allocated spaces are pushed to the start of
  the vector heap to collect all free spaces in a single batch of bytes.
  */

PRIVATE void mm_compact_vector_space ()
{
  E32(taskYIELD());

  cell_p cur = 0;
  cell_p prev = NIL;

  uint16_t cur_size;

  while (cur < vector_free_cells) {
    cur_size  = VECTOR_GET_LENGTH(cur);

    if (cur_size == 0) FATAL("mm_compact_vector_space", "Vector Heap Structure is wrong");

    if ((prev != NIL) && VECTOR_IS_FREE(prev)) { // previous block is free
      if (VECTOR_IS_FREE(cur)) {
        // current is free too, merge free spaces
        // advance cur, but prev stays in place
        cur += cur_size;
      }
      else {
        // prev is free, but not cur, move cur to start at prev
        // fix header in the object heap to point to the data's new
        // location
        RAM_SET_VECTOR_START(VECTOR_GET_RAM_PTR(cur), prev + 1);

        // Move cur vector in its new place
        //char * src = (char *) &vector_heap[cur];
        //char * dst = (char *) &vector_heap[prev];
        //int len = cur_size * sizeof(cell);
        //while (len--) *dst++ = *src++;

        memcpy(&vector_heap[prev], &vector_heap[cur], cur_size * sizeof(cell));

        VECTOR_SET_FREE(cur);

        prev += cur_size;
        cur += cur_size;
      }
    }
    else {
      // Go to the next block, which is <size> away from cur.
      prev = cur;
      cur += cur_size;
    }
  }

  // free space is now all at the end
  vector_free_cells = (prev == NIL) ? 0 : prev;
}

cell_p mm_new_vector_cell(uint16_t length, cell_p from)
{
  // get minimum number of sizeof(cell) blocks (round to nearest sizeof(cell))
  // this includes a sizeof(cell)-byte vector space header
  length = ((length + sizeof(cell) - 1) / sizeof(cell)) + 1;

  if ((vector_heap_size - vector_free_cells) < length) {

    INFO("mm_new_vector_cell", "Vector Space compaction\n");

    mm_gc ();
    mm_compact_vector_space();

    // free space too small, trigger gc
    if ((vector_heap_size - vector_free_cells) < length) { // we gc'd, but no space is big enough for the vector
      FATAL("mm_new_vector_cell", "No room for vector");
    }
  }

  cell_p o = vector_free_cells;

  // advance the free pointer
  vector_free_cells += length;

  VECTOR_SET_LENGTH(o, length);
  VECTOR_SET_RAM_PTR(o, from);
  VECTOR_SET_USED(o);

  // return pointer to start of data, skipping the header
  return o + 1;
}


cell_p mm_new_ram_cell()
{
  if (free_cells == NIL) {
    // INFO("new_ram_cell", "Free Cells Allocated since last GC: %d\n", free_allocated_count);
    mm_gc();
    if (free_cells == NIL) {
      FATAL("mm_gc", "MEMORY EXHAUSTED!!");
    }
  }

  cell_p p = free_cells;
  free_cells = RAM_GET_CDR(free_cells);

  #if DEBUGGING
    free_allocated_count++;
  #endif

  return p;
}

bool mm_init(pgm_ptr program)
{
  E32(taskYIELD());

  reg1 =
  reg2 =
  reg3 =
  reg4 =
  cont =
  env  = NIL;

  bignum_gc_init();

  if ((program[0] != 0xD7) || (program[1] != 0xFB)) {
    ERROR("mm_init", "Program markers are wrong");
    return false;
  }

  global_count = program[3];

  reserved_cells_count = (global_count + 1) >> 1;

  #if STATISTICS
    gc_call_counter = 0;
  #endif

  #ifdef WORKSTATION

    #if STATISTICS
      max_gc_duration = 0;
    #endif

    if ((ram_heap_data = (cell_data_ptr) calloc(RAM_HEAP_ALLOCATED, sizeof(cell_data)))  == NULL) return false;
    if ((ram_heap_flags = (cell_flags_ptr) calloc(RAM_HEAP_ALLOCATED, sizeof(cell_flags)))  == NULL) return false;
    ram_heap_size = RAM_HEAP_ALLOCATED;

    if ((vector_heap = (cell_ptr) calloc(VECTOR_HEAP_ALLOCATED, sizeof(cell))) == NULL) return false;
    vector_heap_size = VECTOR_HEAP_ALLOCATED;

  #else // ESP32

    uint32_t byte_size = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    ram_heap_size = byte_size / sizeof(cell_data);
    if ((ram_heap_data = (cell_data_ptr) heap_caps_calloc(ram_heap_size, sizeof(cell_data), MALLOC_CAP_8BIT))  == NULL) return false;
    if ((ram_heap_flags = (cell_flags_ptr) heap_caps_calloc(ram_heap_size, sizeof(cell_flags), MALLOC_CAP_8BIT))  == NULL) return false;

    byte_size = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) - 20000;
    vector_heap_size = byte_size / sizeof(cell);
    if ((vector_heap = (cell_ptr) heap_caps_calloc(vector_heap_size, sizeof(cell), MALLOC_CAP_8BIT)) == NULL) return false;
  #endif

  rom_heap      = (cell_ptr) &program[4];

  vector_free_cells = 0;

  if (ram_heap_size >= ROM_START_ADDR) {
    ERROR("mm_init", "Ram heap size too large");
    return false;
  }

  ram_heap_end  = ram_heap_size;

  #if STATISTICS
    used_cells_count   = 0;
    free_cells_count   = 0;
    vector_cells_count = 0;
  #endif

  for (cell_p i = 0; i < reserved_cells_count; i++) {
    RAM_SET_TYPE(i, CONS_TYPE);
    RAM_SET_CAR(i, NIL);
    RAM_SET_CDR(i, NIL);
  }

  mm_sweep();

  if (!check_free_list(ram_heap_size)) return false;

  INFO("mm_init", "Globals Size: %u; ROM Constants Size: %u", program[3], program[2]);

  return true;
}


#if TESTS

extern void show(cell_p p);

void mm_tests()
{
  TESTM("mm");

  TEST("mm Initialisation");

    uint8_t pgm[7] = { 0xD7, 0xFB, 0, 25, 0, 0, 0 };

    EXPECT_TRUE(mm_init(pgm),                              "mm Initialisation failed"); // 13 cells required...
    EXPECT_TRUE(sizeof(cell) == 5,                         "Size of a single cell not equal to 5");
    EXPECT_TRUE((((char *) &vector_heap[0]) + sizeof(cell)) == ((char *) &vector_heap[1]), "Cell structure alignment problems");
    EXPECT_TRUE(check_free_list(ram_heap_size),            "Check Ram Heap Free Size return wrong count");
    EXPECT_TRUE(global_count == 25,                        "Globals count != 25");
    EXPECT_TRUE(reserved_cells_count == 13,                "Reserved cells count != 13");
    EXPECT_TRUE(ram_heap_data != NULL,                     "Ram Heap data pointer is NULL");
    EXPECT_TRUE(ram_heap_flags != NULL,                    "Ram Heap flags pointer is NULL");
    EXPECT_TRUE(vector_heap != NULL,                       "Vector Heap pointer is NULL");
    EXPECT_TRUE(ram_heap_size == RAM_HEAP_ALLOCATED,       "Ram Heap Size is wrong");
    EXPECT_TRUE(vector_heap_size == VECTOR_HEAP_ALLOCATED, "Vector Heap Size is wrong");
    EXPECT_TRUE(vector_free_cells == 0,                    "Vector Free Cells pointer is wrong");
    EXPECT_TRUE(ram_heap_end == ram_heap_size,             "Ram Heap End and Size not equal");
    EXPECT_TRUE(free_cells == 13,                          "Free Cells pointer must be pointing at index 0");
    EXPECT_TRUE((reg1 == NIL) && (reg2 == NIL) && (reg3 == NIL) && (reg4 == NIL) && (env == NIL) && (cont == NIL), "All registers not initialized to NIL");

  TEST("Heap allocations");

    cell_p p = mm_new_ram_cell();
    EXPECT_TRUE(p == 13, "Allocated Cell must be at index 13");
    EXPECT_TRUE(free_cells == 14, "Free Cells pointer expected to be at 14");

    for (int i = 0; i < 5000; i++) {
      RAM_SET_TYPE(p, CONS_TYPE);
      RAM_SET_CDR(p, env);
      RAM_SET_CAR(p, FALSE);
      env = p;
      p = mm_new_ram_cell();
    }

    EXPECT_TRUE(free_cells == 5014, "Free Cells pointer expected to be at 5014");

    RAM_SET_CAR(1000, 1005);
    mm_gc();
    EXPECT_TRUE(
      free_cells_count == (ram_heap_size - 13 - 5000),
      "Free Cells Count expected to be at %d but is %d",
      (ram_heap_size - 13 - 5000),
      free_cells_count
    );

    RAM_SET_CDR(2000, RAM_GET_CDR(2000) - 1000);
    mm_gc();
    EXPECT_TRUE(
      free_cells_count == (ram_heap_size - 13 - 4000),
      "Free Cells count expected to be at %d but is %d",
      (ram_heap_size - 13 - 4000),
      free_cells_count
    );

    env = NIL;
    mm_gc();
    EXPECT_TRUE(free_cells_count == (ram_heap_size - 13),
      "Free Cells count expected to be at %d but is %d",
      ram_heap_size - 13,
      free_cells_count
    );

  TEST("Garbage Collector");

    env = new_pair(FALSE, new_pair(TRUE, NIL));
    show(env); putchar('\n');
    mm_gc();
    show(env); putchar('\n');

    env = new_pair(new_pair(encode_int(1500), FALSE), new_pair(TRUE, NIL));
    show(env); putchar('\n');
    mm_gc();
    show(env); putchar('\n');

    env = new_pair(
      new_pair(NIL, new_closure(new_pair(TRUE, new_pair(FALSE, NIL)), 123)),
      new_pair(new_pair(FALSE, TRUE), NIL)
    );
    show(env); putchar('\n');
    mm_gc();
    show(env); putchar('\n');

    env = new_pair(
      new_pair(
        new_cont(
          NIL,
          new_closure(
            new_pair(FALSE, new_pair(FALSE, NIL)),
            123
          )
        ),
        new_pair(TRUE, NIL)
      ),
      new_pair(
        FALSE, NIL
      )
    );
    show(env); putchar('\n');
    mm_gc();
    show(env); putchar('\n');

    env = NIL;
    mm_gc();

  TEST("Vector allocations");

    vector_p v = mm_new_vector_cell(77, p);
    EXPECT_TRUE(v == 1,                         "Vector not pointing at first byte");
    EXPECT_TRUE(vector_free_cells == 17,        "Vector Free Cells not pointing at index 17");
    EXPECT_TRUE(VECTOR_GET_RAM_PTR(v - 1) == p, "Vector heap not pointing at vector header");
    EXPECT_TRUE(VECTOR_GET_LENGTH(v - 1) == 17, "Vector length in cells is wrong");
    EXPECT_TRUE(VECTOR_IS_USED(v - 1),          "Vector is not marked as used");
    VECTOR_SET_BYTE(v, 35, 3);
    EXPECT_TRUE(VECTOR_GET_BYTE(v, 35) == 3,    "Vector set/get not working");

  TEST("Vector compaction");

    cell_p p2, p3;
    vector_p v2, v3;
    p2 = mm_new_ram_cell();
    p3 = mm_new_ram_cell();

    v2 = mm_new_vector_cell(30, p2);
    RAM_SET_VECTOR_START(p2, v2);

    v3 = mm_new_vector_cell(140, p3);
    RAM_SET_VECTOR_START(p3, v3);
    RAM_SET_VECTOR_START(p, v);

    VECTOR_SET_BYTE(v3, 3, 7);
    VECTOR_SET_BYTE(v3, 131, 13);

    EXPECT_TRUE(VECTOR_GET_RAM_PTR(v2 - 1) == p2, "p2 vector not set properly");
    EXPECT_TRUE(VECTOR_GET_RAM_PTR(v3 - 1) == p3, "p3 vector not set properly");
    EXPECT_TRUE(RAM_GET_VECTOR_START(p2) == v2, "p2 not pointing at v2");
    EXPECT_TRUE(RAM_GET_VECTOR_START(p3) == v3, "p3 not pointing at v3");
    EXPECT_TRUE(VECTOR_IS_USED(v2 - 1), "Vector v2 is not marked as used");
    EXPECT_TRUE(VECTOR_IS_USED(v3 - 1), "Vector v3 is not marked as used");

    VECTOR_SET_FREE(v2 - 1);
    EXPECT_TRUE(VECTOR_IS_FREE(v2 - 1), "Vector v2 is not free");
    mm_compact_vector_space();

    EXPECT_TRUE(RAM_GET_VECTOR_START(p2) == RAM_GET_VECTOR_START(p3), "Vector p3 not properly realigned after compaction");
    EXPECT_TRUE(RAM_GET_VECTOR_START(p) == 1, "Vector p start byte address not set properly");
    EXPECT_TRUE(VECTOR_GET_BYTE(RAM_GET_VECTOR_START(p3), 3) == 7, "Vector reallocation not complete (index 3 is wrong)");
    EXPECT_TRUE(VECTOR_GET_BYTE(RAM_GET_VECTOR_START(p3), 131) == 13, "Vector reallocation not complete (index 131 is wrong)");

    VECTOR_SET_FREE(v - 1);
    mm_compact_vector_space();
    EXPECT_TRUE(RAM_GET_VECTOR_START(p3) == 1, "Vector p3 not properly realigned after other vectors freed and compaction");

    VECTOR_SET_FREE(RAM_GET_VECTOR_START(p3) - 1);
    mm_compact_vector_space();

    EXPECT_TRUE(vector_free_cells == 0, "Vector Free Cells pointer is wrong");
}
#endif
