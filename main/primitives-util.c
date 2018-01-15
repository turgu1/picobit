/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

// primitives-utils
// Builtin Indexes: 26..32

#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"
#include "testing.h"

#include "primitives.h"

PRIMITIVE(eq?, eq_p, 2, 26)
{
  reg1 = ENCODE_BOOL(reg1 == reg2);
  reg2 = NIL;
}

PRIMITIVE(not, not, 1, 27)
{
  reg1 = ENCODE_BOOL(reg1 == FALSE);
}

PRIMITIVE(symbol?, symbol_p, 1, 28)
{
  if (IN_RAM(reg1)) {
    reg1 = ENCODE_BOOL(RAM_IS_SYMBOL(reg1));
  }
  else if (IN_ROM(reg1)) {
    reg1 = ENCODE_BOOL(ROM_IS_SYMBOL(reg1));
  }
  else {
    reg1 = FALSE;
  }
}

PRIMITIVE(boolean?, boolean_p, 1, 29)
{
  reg1 = ENCODE_BOOL((reg1 == FALSE) || (reg1 == TRUE));
}

PRIMITIVE(string?, string_p, 1, 30)
{
  if (IN_RAM(reg1)) {
    reg1 = ENCODE_BOOL(RAM_IS_STRING(reg1));
  }
  else if (IN_ROM(reg1)) {
    reg1 = ENCODE_BOOL(ROM_IS_STRING(reg1));
  }
  else {
    reg1 = FALSE;
  }
}

PRIMITIVE(string->list, string2list, 1, 31)
{
  if (IN_RAM(reg1)) {
    EXPECT(RAM_IS_STRING(reg1), "string->list.0", "string");

    reg1 = RAM_STRING_GET_CHARS(reg1);
  }
  else if (IN_ROM(reg1)) {
    EXPECT(ROM_IS_STRING(reg1), "string->list.1", "string");

    reg1 = ROM_STRING_GET_CHARS(reg1);
  }
  else {
    TYPE_ERROR("string->list.2", "string");
  }
}

PRIMITIVE(list->string, list2string, 1, 32)
{
  reg1 = new_string(reg1);
}


#if TESTS
void primitives_util_tests()
{
  TESTM("primitives-util");

}
#endif
