/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

// primitives-esp32
// Builtin Indexes: 43

#include "esp32_scheme_vm.h"
#include "vm_arch.h"
#include "mm.h"
#include "testing.h"

#include "primitives.h"

#if ESP32
  #include "esp32_utils.h"
#endif

// GPIO Definitions

#define GPIO_INIT       0
#define GPIO_READ       1
#define GPIO_WRITE      2
#define GPIO_WAKEUP     3

#define PULL_UP         0
#define PULL_DOWN       1

#define OUTPUT          0
#define INPUT           1

#define LOW             0
#define HIGH            1

#define DISABLE         0
#define ENABLE          1

// (GPIO operation (params ...))
PRIMITIVE(#%gpio, gpio, 2, 43)
{
  E32(gpio_config_t io_conf);

  EXPECT(IS_SMALL_INT(reg1), "gpio.0", "operation as small int");
  a1 = decode_int(reg1);

  switch (a1) {
    case GPIO_INIT:      // Init a GPIO bit, params: Pin# <Input|Output> [Pull-Up|Pull-Down|NIL]
      E32(io_conf.intr_type = GPIO_PIN_INTR_DISABLE);
      EXPECT(IS_PAIR(reg2), "gpio.1", "parameters list");
      // Pin #
      GET_NEXT_VALUE((a1 >= 0) && (a1 <= 39), "gpio.2", "Pin# in range 0..39");
      E32(io_conf.pin_bit_mask = 1ULL << a1);
      a2 = a1;

      // Input | Output
      GET_NEXT_VALUE((a1 == OUTPUT) || (a1 == INPUT), "gpio.3", "Input | Output");
      E32(io_conf.mode = (a1 == OUTPUT) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
      a3 = a1;

      // Pull_Up | Pull_Down | NIL
      if (reg2 != NIL) {
        GET_NEXT_VALUE((reg3 == NIL) || (a1 == PULL_UP) || (a1 == PULL_DOWN), "gpio.4", "Pull-Up | Pull-Down | ()");
        if (a1 == PULL_UP) {
          E32(io_conf.pull_up_en   = 1);
          E32(io_conf.pull_down_en = 0);
        }
        else if (a1 == PULL_DOWN) {
          E32(io_conf.pull_down_en = 1);
          E32(io_conf.pull_up_en   = 0);
        }
      }
      else {
        a1 = -1;
      }

      E32(reg1 = check(gpio_config(&io_conf)));
      DEBUG("GPIO", "Init Pin %d %s %s", a2, a3 == INPUT ? "Input" : "Output", a1 == -1 ? "" : (a1 == PULL_UP ? "Pull-Up" : "Pull-Down"));
      break;

    case GPIO_READ:      // read value from GPIO bit
      if (IS_PAIR(reg2)) {
        GET_NEXT_VALUE((a1 >= 0) && (a1 <= 39), "gpio.5", "Pin# in range 0..39");
      }
      else {
        EXPECT(IS_SMALL_INT(reg2), "gpio.6", "Pin# in range 0..39");
        a1 = decode_int(reg2);
        EXPECT((a1 >= 0) && (a1 <= 39), "gpio.7", "Pin# in range 0..39");
      }
      DEBUG("GPIO", "Read Pin %d", a1);
      E32(reg1 = encode_int(gpio_get_level(a1)));
      WKS(reg1 = ZERO);
      break;

    case GPIO_WRITE:      // write value to GPIO bit
      EXPECT(IS_PAIR(reg2), "gpio.8", "parameters list");
      GET_NEXT_VALUE((a1 >= 0) && (a1 <= 39), "gpio.9", "Pin# in range 0..39");
      a2 = a1;
      GET_NEXT_VALUE((a1 == LOW) || (a1 == HIGH), "gpio.10", "Low | High");
      E32(reg1 = check(gpio_set_level(a2, a1)));
      DEBUG("GPIO", "Write %d to pin %d", a1, a2);
      WKS(reg1 = TRUE);
      break;

    case GPIO_WAKEUP:
      EXPECT(IS_PAIR(reg2), "gpio.11", "parameters list");
      GET_NEXT_VALUE((a1 == ENABLE) || (a1 == DISABLE), "gpio.12", "Enable | Disable");
      a2 = a1;
      GET_NEXT_VALUE((a1 >= 0) && (a1 <= 39), "gpio.13", "Pin# in range 0..39");
      a3 = a1;
      if (a2 == ENABLE) {
        GET_NEXT_VALUE((a1 == LOW) || (a1 == HIGH), "gpio.14", "Low | High");
        E32(reg1 = check(gpio_wakeup_enable(a3, a1 == LOW ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL)));
        DEBUG("GPIO", "WakeUp %s on Pin %d %s", a2 == ENABLE ? "Enable" : "Disable", a3, a1 == LOW ? "Low" : "High");
      }
      else {
        E32(reg1 = check(gpio_wakeup_disable(a3)));
        DEBUG("GPIO", "WakeUp Disable on Pin %d", a3);
      }
      WKS(reg1 = TRUE);
      break;

    default:
      ERROR("gpio", "Unknown operation: %d", a1);
      reg1 = FALSE;
      break;
  }

  reg2 = reg3 = NIL;
}
