// primitives-esp32
// Builtin Indexes: 42

#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"
#include "testing.h"

#include "primitives.h"

#include "driver/gpio.h"

#define IS_PAIR(p) ((IN_RAM(p) && RAM_IS_PAIR(p)) || (IN_ROM(p) && ROM_IS_PAIR(p)))
#define GET_CAR(p) IN_RAM(p) ? RAM_GET_CAR(p) : ROM_GET_CAR(p)
#define GET_CDR(p) IN_RAM(p) ? RAM_GET_CDR(p) : ROM_GET_CDR(p)

#define GET_NEXT_VALUE(test, func, msg) \
  EXPECT((reg2 != NIL), func, msg); \
  reg3 = GET_CAR(reg2);    \
  a1   = decode_int(reg3); \
  reg2 = GET_CDR(reg2);    \
  EXPECT(test, func, msg)

#define INIT      0
#define READ      1
#define WRITE     2

#define PULL_UP   0
#define PULL_DOWN 1

#define OUTPUT    0
#define INPUT     1

#define LOW       0
#define HIGH      1

// (GPIO operation (params ...))
PRIMITIVE(GPIO, gpio, 2, 42)
{
  bool result = false;
  gpio_config_t io_conf;

  EXPECT(IS_PAIR(reg2), "gpio.1", "parameters list");
  a1 = decode_int(reg1);
  switch (a1) {
    case INIT:      // Init a GPIO bit, params: Pin# <Input|Output> [Pull-Up|Pull-Down|NIL]
      INFO_MSG("GPIO Init");
      io_conf.intr_type = GPIO_PIN_INTR_DISABLE;

      // Pin #
      GET_NEXT_VALUE((a1 >= 0) && (a1 <= 39), "gpio.2", "Pin# in range 0..39");
      io_conf.pin_bit_mask = 1ULL << a1;

      // Input | Output
      GET_NEXT_VALUE((a1 == OUTPUT) || (a1 == INPUT), "gpio.3", "Input | Output");
      io_conf.pin_bit_mask = (a1 == OUTPUT) ? GPIO_OUTPUT_PIN_SEL : GPIO_INPUT_PIN_SEL;

      // Pull_Up | Pull_Down | NIL
      io_conf.pull_down_en = 0;
      io_conf.pull_up_en   = 0;
      if (reg2 != NIL) {
        GET_NEXT_VALUE((reg3 == NIL) || (a1 == PULL_UP) || (a1 == PULL_DOWN), "gpio.4", "Pull-Up | Pull-Down | ()");
        if (a1 == PULL_UP) {
          io_conf.pull_up_en   = 1;
        }
        else {
          io_conf.pull_down_en = 1;
        }
      }

      gpio_config(&io_conf);
      reg3 = NIL;
      reg1 = TRUE;
      result = true;
      break;

    case READ:      // read value from IO bit
      INFO_MSG("GPIO Read");
      if (IS_PAIR(reg2)) {
        GET_NEXT_VALUE((a1 >= 0) && (a1 <= 39), "gpio.5", "Pin# in range 0..39");
      }
      else {
        a1 = decode_int(reg2);
        EXPECT((a1 >= 0) && (a1 <= 39), "gpio.6", "Pin# in range 0..39");

      }
      reg1 = encode_int(gpio_get_level(a1));
      result = true;
      break;

    case WRITE:      // read value from IO bit
      INFO_MSG("GPIO Write");
      GET_NEXT_VALUE((a1 >= 0) && (a1 <= 39), "gpio.7", "Pin# in range 0..39");
      a2 = a1;
      GET_NEXT_VALUE((a1 == LOW) && (a1 = HIGH), "gpio.8", "Low | High");
      gpio_set_level(a2, a1);
      reg1 = NIL;
      result = true;
      break;

    default:
      ERROR_MSG("gpio.3: Unknown operation: %d", a1);
      break;
  }

  reg2 = reg3 = NIL;
  if (!result) reg1 = FALSE;
}
