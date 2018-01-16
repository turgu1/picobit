/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

// primitives-control
// Builtin Indexes: 1..5

#include "esp32_scheme_vm.h"
#include "vm_arch.h"
#include "mm.h"
#include "testing.h"

#include "interpreter.h"
#include "primitives.h"

PRIMITIVE(return, return, 1, 1)
{
  EXPECT(RAM_IS_CONTINUATION(cont), "return", "continuation");

  reg2  = RAM_GET_CONT_CLOSURE(cont);

  EXPECT(RAM_IS_CLOSURE(reg2), "return.1", "closure");

  entry = RAM_GET_CLOSURE_ENTRY_POINT(reg2);
  env   = RAM_GET_CLOSURE_ENV(reg2);
  cont  = RAM_GET_CONT_PARENT(cont);
  reg2  = NIL;
  pc.c  = program + entry;
}

PRIMITIVE_UNSPEC(pop, pop, 0, 2)
{
  pop();
}

PRIMITIVE(get-cont, get_cont, 0, 3)
{
  EXPECT(RAM_IS_CONTINUATION(cont), "get-cont", "continuation");

  reg1 = cont;
}

PRIMITIVE_UNSPEC(graft-to-cont, graft_to_cont, 2, 4)
{
  /* reg2 is thunk to call, reg1 is continuation */
  cont = reg1;

  EXPECT(RAM_IS_CONTINUATION(cont), "graft-to-cont", "continuation");

  reg1 = reg2;
  env  = new_pair(reg1, env);

  build_environment(prepare_arguments(0));

  env  = reg1;
  pc.c = program + entry;

  reg1 = NIL;
  reg2 = NIL;
}

PRIMITIVE(return-to-cont, return_to_cont, 2, 5)
{
  /* reg2 is value to return, reg1 is continuation */
  cont = reg1;

  EXPECT(RAM_IS_CONTINUATION(cont), "return-to-cont", "continuation");

  reg1 = reg2;

  reg2  = RAM_GET_CONT_CLOSURE(cont);

  EXPECT(RAM_IS_CLOSURE(reg2), "return-to-cont.1", "closure");

  entry = RAM_GET_CLOSURE_ENTRY_POINT(reg2);
  env   = RAM_GET_CLOSURE_ENV(reg2);
  cont  = RAM_GET_CONT_PARENT(cont);

  pc.c = program + entry;

  reg2 = NIL;
}


#if TESTS
void primitives_control_tests()
{
  TESTM("primitives-control");

}
#endif
