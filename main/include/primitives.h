/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#ifdef CONFIG_DEBUG_STRINGS
  extern const char* const primitive_names[];
#endif /* CONFIG_DEBUG_STRINGS */

/* For the primitive scanning pass. */
#ifndef NO_PRIMITIVE_EXPAND

  #define PRIMITIVE(scheme_name, c_name, args, index) void primitive_ ## c_name ()
  #define PRIMITIVE_UNSPEC PRIMITIVE

#endif /* NO_PRIMITIVE_EXPAND */

// Tools to used when a primitive receives a list of arguments as 
// a second parameter
#define GET_NEXT_VALUE(var, test, func, msg)             \
  EXPECT(IS_PAIR(reg2), func, "[pair] " msg);       \
  reg3 = GET_CAR(reg2);                             \
  EXPECT(IS_NUMBER(reg3), func, "[number] " msg );  \
  var  = decode_int(reg3);                          \
  reg2 = GET_CDR(reg2);                             \
  EXPECT(test, func, "[test] " msg)

#define GET_NEXT_STRING(var, length, func, msg)     \
  EXPECT(IS_PAIR(reg2), func, "[pair] " msg);       \
  reg3 = GET_CAR(reg2);                             \
  EXPECT(IS_STRING(reg3), func, "[string] " msg);   \
  reg2 = GET_CDR(reg2);                             \
  string2cstring(reg3, var, length);

#define GET_NEXT_BOOL(var, func, msg)     \
  EXPECT(IS_PAIR(reg2), func, "[pair] " msg);       \
  reg3 = GET_CAR(reg2);                             \
  EXPECT(IS_BOOL(reg3), func, "[boolean] " msg);   \
  reg2 = GET_CDR(reg2);                             \
  var = (reg3 == TRUE) ? true : false;

#endif
