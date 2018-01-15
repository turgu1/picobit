/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

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
