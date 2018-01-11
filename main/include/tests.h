#ifndef TESTS_H
#define TESTS_H

#define PUBLIC extern

PUBLIC void primitives_computer_tests();
PUBLIC void primitives_control_tests();
PUBLIC void primitives_list_tests();
PUBLIC void primitives_numeric_tests();
PUBLIC void primitives_util_tests();
PUBLIC void primitives_vector_tests();
PUBLIC void hexfile_tests();
PUBLIC void interpreter_tests();
PUBLIC void kb_tests();
PUBLIC void mm_tests();
PUBLIC void vm_arch_tests();

#undef PUBLIC
#endif
