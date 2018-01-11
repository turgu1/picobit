// primitives-list
// Builtin Indexes: 6..12

#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"
#include "testing.h"

#include "primitives.h"

PRIMITIVE(pair?, pair_p, 1, 6)
{
  if (IN_RAM(reg1)) {
    reg1 = ENCODE_BOOL(RAM_IS_PAIR(reg1));
  }
  else if (IN_ROM(reg1)) {
    reg1 = ENCODE_BOOL(ROM_IS_PAIR(reg1));
  }
  else {
    reg1 = FALSE;
  }
}

PRIMITIVE(cons, cons, 2, 7)
{
  reg1 = new_pair(reg1, reg2);
  reg2 = NIL;
}

PRIMITIVE(car, car, 1, 8)
{
  if (IN_RAM(reg1)) {
    EXPECT(RAM_IS_PAIR(reg1), "car.0", "pair");

    reg1 = RAM_GET_CAR(reg1);
  }
  else if (IN_ROM(reg1)) {
    EXPECT(ROM_IS_PAIR(reg1), "car.1", "pair");

    reg1 = ROM_GET_CAR(reg1);
  }
  else {
    TYPE_ERROR("car.2", "pair");
  }
}

PRIMITIVE(cdr, cdr, 1, 9)
{
  if (IN_RAM(reg1)) {
    EXPECT(RAM_IS_PAIR(reg1), "cdr.0", "pair");

    reg1 = RAM_GET_CDR(reg1);
  }
  else if (IN_ROM(reg1)) {
    EXPECT(ROM_IS_PAIR(reg1), "cdr.1", "pair");

    reg1 = ROM_GET_CDR(reg1);
  }
  else {
    TYPE_ERROR("cdr.2", "pair");
  }
}

PRIMITIVE_UNSPEC(set-car!, set_car_bang, 2, 10)
{
  if (IN_RAM(reg1)) {
    EXPECT(RAM_IS_PAIR(reg1), "set-car!.0", "pair");

    RAM_SET_CAR(reg1, reg2);
    reg1 = NIL;
    reg2 = NIL;
  }
  else {
    TYPE_ERROR("set-car!.1", "pair");
  }
}

PRIMITIVE_UNSPEC(set-cdr!, set_cdr_bang, 2, 11)
{
  if (IN_RAM(reg1)) {
    EXPECT(RAM_IS_PAIR(reg1), "set-cdr!.0", "pair");

    RAM_SET_CDR(reg1, reg2);
    reg1 = NIL;
    reg2 = NIL;
  }
  else {
    TYPE_ERROR("set-cdr!.1", "pair");
  }
}

PRIMITIVE(null?, null_p, 1, 12)
{
  reg1 = ENCODE_BOOL(reg1 == NIL);
}


#if TESTS
void primitives_list_tests()
{
  TESTM("primitives-list");

}
#endif
