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

#define TAG "GPIO"

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

  EXPECT(IS_SMALL_INT(reg1), TAG, "operation as small int");
  a1 = decode_int(reg1);

  switch (a1) {
    case GPIO_INIT:      // Init a GPIO bit, params: Pin# <Input|Output> [Pull-Up|Pull-Down|NIL]
      E32(io_conf.intr_type = GPIO_PIN_INTR_DISABLE);
      EXPECT(IS_PAIR(reg2), TAG, "parameters list");
      // Pin #
      GET_NEXT_VALUE(a1, (a1 >= 0) && (a1 <= 39), TAG, "Pin# in range 0..39");
      E32(io_conf.pin_bit_mask = 1ULL << a1);

      // Input | Output
      GET_NEXT_VALUE(a2, (a2 == OUTPUT) || (a2 == INPUT), TAG, "Input | Output");
      E32(io_conf.mode = (a2 == OUTPUT) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);

      // Pull_Up | Pull_Down | NIL
      if (reg2 != NIL) {
        GET_NEXT_VALUE(a3, (reg3 == NIL) || (a3 == PULL_UP) || (a3 == PULL_DOWN), TAG, "Pull-Up | Pull-Down | ()");
        if (a3 == PULL_UP) {
          E32(io_conf.pull_up_en   = 1);
          E32(io_conf.pull_down_en = 0);
        }
        else if (a3 == PULL_DOWN) {
          E32(io_conf.pull_down_en = 1);
          E32(io_conf.pull_up_en   = 0);
        }
      }
      else {
        a3 = -1;
      }

      E32(reg1 = check(gpio_config(&io_conf)));
      DEBUG(TAG, "Init Pin %d %s %s", a1, a2 == INPUT ? "Input" : "Output", a3 == -1 ? "" : (a3 == PULL_UP ? "Pull-Up" : "Pull-Down"));
      break;

    case GPIO_READ:      // read value from GPIO bit
      if (IS_PAIR(reg2)) {
        GET_NEXT_VALUE(a1, (a1 >= 0) && (a1 <= 39), TAG, "Pin# in range 0..39");
      }
      else {
        EXPECT(IS_SMALL_INT(reg2), TAG, "Pin# in range 0..39");
        a1 = decode_int(reg2);
        EXPECT((a1 >= 0) && (a1 <= 39), TAG, "Pin# in range 0..39");
      }
      DEBUG(TAG, "Read Pin %d", a1);
      E32(reg1 = encode_int(gpio_get_level(a1)));
      WKS(reg1 = ZERO);
      break;

    case GPIO_WRITE:      // write value to GPIO bit
      EXPECT(IS_PAIR(reg2), TAG, "parameters list");
      GET_NEXT_VALUE(a1, (a1 >= 0) && (a1 <= 39), TAG, "Pin# in range 0..39");
      GET_NEXT_VALUE(a2, (a2 == LOW) || (a2 == HIGH), TAG, "Low | High");
      E32(reg1 = check(gpio_set_level(a1, a2)));
      DEBUG(TAG, "Write %d to pin %d", a2, a1);
      WKS(reg1 = TRUE);
      break;

    case GPIO_WAKEUP:
      EXPECT(IS_PAIR(reg2), TAG, "parameters list");
      GET_NEXT_VALUE(a1, (a1 == ENABLE) || (a1 == DISABLE), TAG, "Enable | Disable");
      GET_NEXT_VALUE(a2, (a2 >= 0) && (a2 <= 39), TAG, "Pin# in range 0..39");
      if (a1 == ENABLE) {
        GET_NEXT_VALUE(a3, (a3 == LOW) || (a3 == HIGH), TAG, "Low | High");
        E32(reg1 = check(gpio_wakeup_enable(a1, a3 == LOW ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL)));
        DEBUG(TAG, "WakeUp %s on Pin %d %s", a1 == ENABLE ? "Enable" : "Disable", a2, a3 == LOW ? "Low" : "High");
      }
      else {
        E32(reg1 = check(gpio_wakeup_disable(a2)));
        DEBUG(TAG, "WakeUp Disable on Pin %d", a2);
      }
      WKS(reg1 = TRUE);
      break;

    default:
      ERROR(TAG, "Unknown operation: %d", a1);
      reg1 = FALSE;
      break;
  }

  reg2 = reg3 = NIL;
}
