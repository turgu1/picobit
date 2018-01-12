// primitives-esp32
// Builtin Indexes: 42 .. 43

#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"
#include "testing.h"

#include "primitives.h"

#if ESP32
  #include "driver/gpio.h"
  #include "esp_wifi.h"
  #include "esp_log.h"

  cell_p check(esp_err_t result) { return result == ESP_OK ? TRUE : FALSE; }

  struct {
    char *name;
    esp_log_level_t level;
  } level_info[6] = {
    { "NONE",    ESP_LOG_NONE    },
    { "VERBOSE", ESP_LOG_VERBOSE },
    { "DEBUG",   ESP_LOG_DEBUG   },
    { "INFO",    ESP_LOG_INFO    },
    { "WARNING", ESP_LOG_WARN    },
    { "ERROR",   ESP_LOG_ERROR   }
  };
#endif

#if WORKSTATION
  #include <time.h>

  #define E32(instr)
  #define WKS(instr) instr

  struct {
    char *name;
  } level_info[6] = {
    { "NONE"    },
    { "VERBOSE" },
    { "DEBUG"   },
    { "INFO"    },
    { "WARNING" },
    { "ERROR"   }
  };
#endif

#define GET_NEXT_VALUE(test, func, msg)             \
  EXPECT(IS_PAIR(reg2), func, "[pair] " msg);       \
  reg3 = GET_CAR(reg2);                             \
  EXPECT(IS_NUMBER(reg3), func, "[number] " msg );  \
  a1   = decode_int(reg3);                          \
  reg2 = GET_CDR(reg2);                             \
  EXPECT(test, func, "[test] " msg)

#define GET_NEXT_STRING(var, length, func, msg)     \
  EXPECT(IS_PAIR(reg2), func, "[pair] " msg);       \
  reg3 = GET_CAR(reg2);                             \
  EXPECT(IS_STRING(reg3), func, "[string] " msg);   \
  reg2 = GET_CDR(reg2);                             \
  string2cstring(reg3, var, length);

#define RESET      99
#define WATCHDOG    0
#define SLEEP       1
#define LOG         2
#define LOG_LEVEL   3

#define LOG_ERROR   5
#define LOG_WARNING 4
#define LOG_INFO    3
#define LOG_DEBUG   2
#define LOG_VERBOSE 1
#define LOG_NONE    0

#define INIT        0
#define READ        1
#define WRITE       2
#define WAKEUP      3

#define PULL_UP     0
#define PULL_DOWN   1

#define OUTPUT      0
#define INPUT       1

#define LOW         0
#define HIGH        1

#define DISABLE     0
#define ENABLE      1

extern void show(cell_p p);

// (SYS operation (params ...))
PRIMITIVE(#%sys, sys, 2, 43)
{
  char tag[20];
  char msg[120];

  EXPECT(IS_SMALL_INT(reg1), "sys.0", "operation as small int");
  a1 = decode_int(reg1);

  switch (a1) {
    case RESET:
      E32(esp_restart());
      WKS(terminate());
      reg1 = NIL;
      break;

    case WATCHDOG:
      E32(vTaskDelay(1));
      reg1 = TRUE;
      break;

    case SLEEP:
      GET_NEXT_VALUE((a1 >= 0) && (a1 <= 30000), "sys.1", "sleep time <= 30000ms");
      E32(vTaskDelay(a1 / 10));
      WKS(usleep(1000L * a1));
      reg1 = TRUE;
      break;

    case LOG:
      GET_NEXT_VALUE((a1 >= LOG_VERBOSE) && (a1 <= LOG_ERROR), "sys.2", "One of Error, Warning, Info, Verbose, Debug or None");
      GET_NEXT_STRING(tag,  20, "sys.", "log tag");
      GET_NEXT_STRING(msg, 120, "sys.", "log message");
      E32(esp_log_write(a1, tag, msg));
      INFO_MSG("sys: Log %s for tag %s: %s", level_info[a1].name, tag, msg);
      reg1 = TRUE;
      break;

    case LOG_LEVEL:
      GET_NEXT_VALUE((a1 >= LOG_NONE) && (a1 <= LOG_ERROR), "sys.3", "One of Error, Warning, Info, Verbose, Debug or None");
      GET_NEXT_STRING(tag, 20, "sys.", "log tag");
      E32(esp_log_level_set(tag, level_info[a1].level));
      INFO_MSG("sys: Set Log Level %s for tag %s", level_info[a1].name, tag);
      reg1 = TRUE;
      break;

    default:
      ERROR_MSG("sys.4: Unknown operation: %d", a1);
      reg1 = FALSE;
      break;
  }
}

// (GPIO operation (params ...))
PRIMITIVE(#%gpio, gpio, 2, 42)
{
  E32(gpio_config_t io_conf);

  EXPECT(IS_SMALL_INT(reg1), "gpio.0", "operation as small int");
  a1 = decode_int(reg1);

  switch (a1) {
    case INIT:      // Init a GPIO bit, params: Pin# <Input|Output> [Pull-Up|Pull-Down|NIL]
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
      INFO_MSG("gpio: Init Pin %d %s %s", a2, a3 == INPUT ? "Input" : "Output", a1 == -1 ? "" : (a1 == PULL_UP ? "Pull-Up" : "Pull-Down"));
      break;

    case READ:      // read value from GPIO bit
      if (IS_PAIR(reg2)) {
        GET_NEXT_VALUE((a1 >= 0) && (a1 <= 39), "gpio.5", "Pin# in range 0..39");
      }
      else {
        EXPECT(IS_SMALL_INT(reg2), "gpio.6", "Pin# in range 0..39");
        a1 = decode_int(reg2);
        EXPECT((a1 >= 0) && (a1 <= 39), "gpio.7", "Pin# in range 0..39");
      }
      INFO_MSG("gpio: Read Pin %d", a1);
      E32(reg1 = encode_int(gpio_get_level(a1)));
      WKS(reg1 = ZERO);
      break;

    case WRITE:      // write value to GPIO bit
      EXPECT(IS_PAIR(reg2), "gpio.8", "parameters list");
      GET_NEXT_VALUE((a1 >= 0) && (a1 <= 39), "gpio.9", "Pin# in range 0..39");
      a2 = a1;
      GET_NEXT_VALUE((a1 == LOW) || (a1 == HIGH), "gpio.10", "Low | High");
      E32(reg1 = check(gpio_set_level(a2, a1)));
      INFO_MSG("gpio: Write %d to pin %d", a1, a2);
      WKS(reg1 = TRUE);
      break;

    case WAKEUP:
      EXPECT(IS_PAIR(reg2), "gpio.11", "parameters list");
      GET_NEXT_VALUE((a1 == ENABLE) || (a1 == DISABLE), "gpio.12", "Enable | Disable");
      a2 = a1;
      GET_NEXT_VALUE((a1 >= 0) && (a1 <= 39), "gpio.13", "Pin# in range 0..39");
      a3 = a1;
      if (a2 == ENABLE) {
        GET_NEXT_VALUE((a1 == LOW) || (a1 == HIGH), "gpio.14", "Low | High");
        E32(reg1 = check(gpio_wakeup_enable(a3, a1 == LOW ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL)));
        INFO_MSG("gpio: WakeUp %s on Pin %d %s", a2 == ENABLE ? "Enable" : "Disable", a3, a1 == LOW ? "Low" : "High");
      }
      else {
        E32(reg1 = check(gpio_wakeup_disable(a3)));
        INFO_MSG("gpio: WakeUp Disable on Pin %d", a3);
      }
      WKS(reg1 = TRUE);
      break;

    default:
      ERROR_MSG("gpio.15: Unknown operation: %d", a1);
      reg1 = FALSE;
      break;
  }

  reg2 = reg3 = NIL;
}
