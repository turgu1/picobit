// primitives-esp32
// Builtin Indexes: 42 .. 43

#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"
#include "testing.h"

#include "primitives.h"

#if ESP32
  #include <string.h>
  #include "driver/gpio.h"
  #include "esp_wifi.h"
  #include "esp_log.h"

  cell_p check(esp_err_t result) { return result == ESP_OK ? TRUE : FALSE; }

  #define COLOR(COLOR)  "\033[1;" COLOR "m"
  #define RESET_COLOR   "\033[0m"

  #define CBLACK   "30"
  #define CRED     "31"
  #define CGREEN   "32"
  #define CBROWN   "33"
  #define CBLUE    "34"
  #define CPURPLE  "35"
  #define CCYAN    "36"

  struct {
    char *name;
    esp_log_level_t level;
    char letter;
    char *color;
  } level_info[6] = {
    { "NONE",    ESP_LOG_NONE,    'N', COLOR(CBLACK)  },
    { "VERBOSE", ESP_LOG_VERBOSE, 'V', COLOR(CBLACK)  },
    { "DEBUG",   ESP_LOG_DEBUG,   'D', COLOR(CPURPLE) },
    { "INFO",    ESP_LOG_INFO,    'I', COLOR(CGREEN)  },
    { "WARNING", ESP_LOG_WARN,    'W', COLOR(CBROWN)  },
    { "ERROR",   ESP_LOG_ERROR,   'E', COLOR(CRED)    }
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

// SYS Definitions

#define RESET          99
#define DEEP_SLEEP     98
#define LIGHT_SLEEP    97
#define WATCHDOG        0
#define SLEEP           1
#define LOG             2
#define LOG_LEVEL       3
#define WIFI_INIT       4
#define WIFI_CONNECT    5
#define WIFI_DISCONNECT 6
#define WIFI_CONNECTED  7
#define WIFI_STOP       8
#define WIFI_START      9
#define WAKEUP_CAUSE   10

// Log Levels

#define LOG_ERROR       5
#define LOG_WARNING     4
#define LOG_INFO        3
#define LOG_DEBUG       2
#define LOG_VERBOSE     1
#define LOG_NONE        0

// WakeUp Causes

#define UNDEFINED       0
#define EXT0            1
#define EXT1            2
#define TIMER           3
#define TOUCHPAD        4
#define ULP             5

// GPIO Definitions

#define INIT            0
#define READ            1
#define WRITE           2
#define WAKEUP          3

#define PULL_UP         0
#define PULL_DOWN       1

#define OUTPUT          0
#define INPUT           1

#define LOW             0
#define HIGH            1

#define DISABLE         0
#define ENABLE          1

extern void show(cell_p p);
E32(extern bool wifi_connected);

// (SYS operation (params ...))
PRIMITIVE(#%sys, sys, 2, 43)
{
  char str1[50];
  char str2[120];
  E32(esp_err_t result);
  E32(esp_sleep_wakeup_cause_t cause);

  EXPECT(IS_SMALL_INT(reg1), "sys.0", "operation as small int");
  a1 = decode_int(reg1);

  switch (a1) {
    case RESET:
      E32(esp_restart());
      WKS(terminate());
      reg1 = NIL;
      break;

    case DEEP_SLEEP:
      if (reg2 != NIL) {
        GET_NEXT_VALUE((a1 >= 0) && (a1 <= 8000000), "sys.1", "sleep time <= 8000000ms");
        E32(ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(1000ULL * a1)));
        DEBUG("SYS", "WakeUp in %d ms", a1);
      }
      E32(esp_wifi_disconnect());
      E32(esp_wifi_stop());
      DEBUG("SYS", "Deep Sleep Start");
      E32(esp_deep_sleep_start());
      reg1 = TRUE;
      break;

    case LIGHT_SLEEP:
      if (reg2 != NIL) {
        GET_NEXT_VALUE((a1 >= 0) && (a1 <= 8000000), "sys.1", "sleep time <= 8000000ms");
        E32(ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(1000ULL * a1)));
        DEBUG("SYS", "WakeUp in %d ms", a1);
      }
      DEBUG("SYS", "Deep Sleep Start");
      E32(esp_light_sleep_start());
      reg1 = TRUE;
      break;

    case WATCHDOG:
      E32(taskYIELD());
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
      GET_NEXT_STRING(str1,  20, "sys.", "log tag");
      GET_NEXT_STRING(str2, 120, "sys.", "log message");
      #if ESP32
        if (LOG_LOCAL_LEVEL >= level_info[a1].level) {
          esp_log_write(
            level_info[a1].level,
            str1,
            "%s%c (%d) %s: %s" RESET_COLOR "\n",
            level_info[a1].color,
            level_info[a1].letter,
            esp_log_timestamp(),
            str1,
            str2);
        }
      #endif
      WKS(DEBUG("SYS", "Log %s for tag %s: %s", level_info[a1].name, str1, str2));
      reg1 = TRUE;
      break;

    case LOG_LEVEL:
      GET_NEXT_VALUE((a1 >= LOG_NONE) && (a1 <= LOG_ERROR), "sys.3", "One of Error, Warning, Info, Verbose, Debug or None");
      GET_NEXT_STRING(str1, 20, "sys.", "log tag");
      E32(esp_log_level_set(str1, level_info[a1].level));
      DEBUG("SYS", "Set Log Level %s for tag %s", level_info[a1].name, str1);
      reg1 = TRUE;
      break;

    case WIFI_INIT:
      GET_NEXT_STRING(str1, 32, "sys.", "SSID"    );
      GET_NEXT_STRING(str2, 64, "sys.", "password");
      #if ESP32
        tcpip_adapter_init();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        wifi_config_t sta_config;

        strcpy((char *) sta_config.sta.ssid,     str1);
        strcpy((char *) sta_config.sta.password, str2);
        sta_config.sta.bssid_set = false;

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_ERROR_CHECK(result = esp_wifi_connect());
        reg1 = result == ESP_OK ? TRUE : FALSE;
      #else
        reg1 = TRUE;
      #endif
      DEBUG("SYS", "WiFi Init with SSID: %s, result: %s", str1, reg1 == TRUE ? "OK" : "ERROR");
      break;

    case WIFI_CONNECT:
      E32(ESP_ERROR_CHECK(result = esp_wifi_connect()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG("SYS", "WiFi Connect, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

    case WIFI_DISCONNECT:
      E32(ESP_ERROR_CHECK(result = esp_wifi_disconnect()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG("SYS", "WiFi Disconnect, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

    case WIFI_CONNECTED:
      E32(reg1 = (wifi_connected ? TRUE : FALSE));
      WKS(reg1 = TRUE);
      DEBUG("SYS", "WiFi is %sconnected", reg1 == TRUE ? "" : "not ");
      break;

    case WIFI_STOP:
      E32(ESP_ERROR_CHECK(result = esp_wifi_stop()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG("SYS", "WiFi Stop, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

    case WIFI_START:
      E32(ESP_ERROR_CHECK(result = esp_wifi_start()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG("SYS", "WiFi Start, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

    case WAKEUP_CAUSE:
      #if ESP32
        cause = esp_sleep_get_wakeup_cause();
        switch (cause) {
          case ESP_SLEEP_WAKEUP_UNDEFINED: a1 = UNDEFINED; break;
          case ESP_SLEEP_WAKEUP_EXT0:      a1 = EXT0;      break;
          case ESP_SLEEP_WAKEUP_EXT1:      a1 = EXT1;      break;
          case ESP_SLEEP_WAKEUP_TIMER:     a1 = TIMER;     break;
          case ESP_SLEEP_WAKEUP_TOUCHPAD:  a1 = TOUCHPAD;  break;
          case ESP_SLEEP_WAKEUP_ULP:       a1 = ULP;       break;
          default:                         a1 = UNDEFINED; break;
        }
      #else
        a1 = UNDEFINED;
      #endif
      reg1 = encode_int(a1);
      break;

    default:
      ERROR("SYS", "Unknown operation: %d", a1);
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
      DEBUG("GPIO", "Init Pin %d %s %s", a2, a3 == INPUT ? "Input" : "Output", a1 == -1 ? "" : (a1 == PULL_UP ? "Pull-Up" : "Pull-Down"));
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
      DEBUG("GPIO", "Read Pin %d", a1);
      E32(reg1 = encode_int(gpio_get_level(a1)));
      WKS(reg1 = ZERO);
      break;

    case WRITE:      // write value to GPIO bit
      EXPECT(IS_PAIR(reg2), "gpio.8", "parameters list");
      GET_NEXT_VALUE((a1 >= 0) && (a1 <= 39), "gpio.9", "Pin# in range 0..39");
      a2 = a1;
      GET_NEXT_VALUE((a1 == LOW) || (a1 == HIGH), "gpio.10", "Low | High");
      E32(reg1 = check(gpio_set_level(a2, a1)));
      DEBUG("GPIO", "Write %d to pin %d", a1, a2);
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
