/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#ifndef MM_H
#define MM_H

#ifdef MM
# define PUBLIC
#else
# define PUBLIC extern
#endif

#ifdef WORKSTATION
  #define RAM_HEAP_ALLOCATED    40000 // (8192 - 1280)
  #define VECTOR_HEAP_ALLOCATED 30000 // 8192
#endif

/** Memory management.

   The VM Architecture, in relation with the capability offered
   with the ESP32 requiers the following memory types:

    - RAM Heap space
    - RAM Vector space
    - ROM Heap space
    - ROM Vector space

   The RAM spaces are dynamically managed using the mm module functions
   described below. The ROM spaces are considered static and cannot be
   modified in any way. They are provided with the esp32-scheme-vm
   program in support of the various scheme structures embedded with the
   vm.

*/

/** Memory Management Initialisation.

  Allocates heap space for RAM Heap and Vector Space.
  Retrieve ROM heap and vector space addresses

 */

#ifdef MM

  /**
     RAM and ROM heaps are combined in a "virtual" heap through the indices in use.
     RAM indices start at 0 and end with the size of available contiguous allocatable
     space from the ESP-IDF. ROM indices start at 0xE000 up to the lenght of the ROM
     cells array.  The maximum indice to access ROM must be lower than
     65536 - 512: 0xFE00.
   */

  PRIVATE cell_p   free_cells;
  PRIVATE vector_p vector_free_cells;

  #if DEBUGGING || STATISTICS
    PRIVATE uint16_t   free_cells_count;
    PRIVATE uint16_t   used_cells_count;
    PRIVATE uint16_t vector_cells_count;
  #endif

#endif

PUBLIC bool   mm_init(pgm_ptr program);
PUBLIC void   mm_mark(cell_p p);
PUBLIC void   mm_gc();
PUBLIC cell_p mm_new_ram_cell();
PUBLIC cell_p mm_new_vector_cell(uint16_t length, cell_p from);
PUBLIC void   return_to_free_list(cell_p p);
PUBLIC void   unmark_ram();

#if DEBUGGING
  PUBLIC void unmark_ram();
  PUBLIC bool is_free(cell_p p);
#endif

#if STATISTICS_GC
  PUBLIC double max_gc_duration;
#endif

#if STATISTICS_GC
  PUBLIC int gc_call_counter;
#endif

#undef PUBLIC
#endif
