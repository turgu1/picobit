#ifndef TESTING_H
#define TESTING_H

#ifdef TESTING
  #define PUBLIC
#else
  #define PUBLIC extern
#endif

#if TESTS
  #define TESTM(name) printf("\n[][][][][][] Module %s [][][][][][]\n", name)
  #define TEST(name) printf("\n---> %s <---\n", name)

  PUBLIC void conduct_tests();
  PUBLIC void EXPECT_TRUE(bool val, char * info, ...);
#endif

#undef PUBLIC
#endif
