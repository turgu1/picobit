#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"

#include <stdarg.h>

#if TESTS

#define TESTING 1
#include "testing.h"
#include "tests.h"

static int failed_count;
static int tested_count;

void EXPECT_TRUE(bool val, char * info, ...)
{
  va_list ap;
  static char buf[200];

  tested_count++;
  putchar('.');
  fflush(stdout);
  if (!(val)) {
    va_start(ap, info);
    strcpy(buf, "\nTest failed: ");
    strncat(buf, info, 200 - 15);
    vfprintf(stderr, buf, ap);
    fputc('\n', stderr);
    failed_count++;
  }
}

void conduct_tests()
{
  tested_count = failed_count = 0;

  //kb_tests(); // Requires user interaction
  mm_tests();
  vm_arch_tests();
  primitives_computer_tests();
  primitives_control_tests();
  primitives_list_tests();
  primitives_numeric_tests();
  primitives_util_tests();
  primitives_vector_tests();
  hexfile_tests();
  interpreter_tests();

  fprintf(stderr,
    "\n\n--------------------\nTests completed: %d\nTests failed: %d\n--------------------\n",
    tested_count,
    failed_count);
}

#endif

#if TRACING
extern void show (cell_p o);

void TRACE(char * format, ...)
{
  va_list ap;
  if (trace) {
    //printf("  Env ptr: %d - ", env);
    //show(env);
    //putchar('\n');
    va_start(ap, format);
    fprintf(stderr, "[%ld]", last_pc.c - program);
    vfprintf(stderr, format, ap);
    fflush(stdout);
  }
}
#endif
