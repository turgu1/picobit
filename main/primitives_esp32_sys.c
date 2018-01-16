/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

// primitives-esp32
// Builtin Indexes: 42

#include "esp32_scheme_vm.h"
#include "vm_arch.h"
#include "mm.h"
#include "testing.h"

#include "primitives.h"

#if ESP32
  #include <string.h>
  #include "esp32_utils.h"
  #include "esp_wifi.h"
  
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

// SYS Definitions

#define SYS_RESET           0
#define SYS_DEEP_SLEEP      1
#define SYS_LIGHT_SLEEP     2
#define SYS_WATCHDOG        3
#define SYS_SLEEP           4
#define SYS_LOG             5
#define SYS_LOG_LEVEL       6
#define SYS_WAKEUP_CAUSE    7

// Log Levels

#define LOG_ERROR       5
#define LOG_WARNING     4
#define LOG_INFO        3
#define LOG_DEBUG       2
#define LOG_VERBOSE     1
#define LOG_NONE        0

// WakeUp Causes

#define WC_UNDEFINED    0
#define WC_EXT0         1
#define WC_EXT1         2
#define WC_TIMER        3
#define WC_TOUCHPAD     4
#define WC_ULP          5

// (SYS operation (params ...))
PRIMITIVE(#%sys, sys, 2, 42)
{
  char str1[50];
  char str2[120];
  // E32(esp_err_t result);
  E32(esp_sleep_wakeup_cause_t cause);

  EXPECT(IS_SMALL_INT(reg1), "sys.0", "operation as small int");
  a1 = decode_int(reg1);

  switch (a1) {
    case SYS_RESET:
      E32(esp_restart());
      WKS(terminate());
      reg1 = NIL;
      break;

    case SYS_DEEP_SLEEP:
      if (reg2 != NIL) {
        GET_NEXT_VALUE(a1, (a1 >= 0) && (a1 <= 8000000), "sys.1", "sleep time <= 8000000ms");
        E32(ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(1000ULL * a1)));
        DEBUG("SYS", "WakeUp in %d ms", a1);
      }
      E32(if (wifi_connected) esp_wifi_stop());
      DEBUG("SYS", "Deep Sleep Start");
      E32(esp_deep_sleep_start());
      reg1 = TRUE;
      break;

    case SYS_LIGHT_SLEEP:
      if (reg2 != NIL) {
        GET_NEXT_VALUE(a1, (a1 >= 0) && (a1 <= 8000000), "sys.1", "sleep time <= 8000000ms");
        E32(ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(1000ULL * a1)));
        DEBUG("SYS", "WakeUp in %d ms", a1);
      }
      DEBUG("SYS", "Light Sleep Start");
      E32(esp_light_sleep_start());
      reg1 = TRUE;
      break;

    case SYS_WATCHDOG:
      E32(taskYIELD());
      reg1 = TRUE;
      break;

    case SYS_SLEEP:
      GET_NEXT_VALUE(a1, (a1 >= 0) && (a1 <= 30000), "sys.1", "sleep time <= 30000ms");
      E32(vTaskDelay(a1 / 10));
      DEBUG("SYS", "Sleep %d ms", a1);
      WKS(usleep(1000UL * a1));
      reg1 = TRUE;
      break;

    case SYS_LOG:
      GET_NEXT_VALUE(a1, (a1 >= LOG_VERBOSE) && (a1 <= LOG_ERROR), "sys.2", "One of Error, Warning, Info, Verbose, Debug or None");
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

    case SYS_LOG_LEVEL:
      GET_NEXT_VALUE(a1, (a1 >= LOG_NONE) && (a1 <= LOG_ERROR), "sys.3", "One of Error, Warning, Info, Verbose, Debug or None");
      GET_NEXT_STRING(str1, 20, "sys.", "log tag");
      E32(esp_log_level_set(str1, level_info[a1].level));
      DEBUG("SYS", "Set Log Level %s for tag %s", level_info[a1].name, str1);
      reg1 = TRUE;
      break;

    case SYS_WAKEUP_CAUSE:
      #if ESP32
        cause = esp_sleep_get_wakeup_cause();
        switch (cause) {
          case ESP_SLEEP_WAKEUP_UNDEFINED: a1 = WC_UNDEFINED; break;
          case ESP_SLEEP_WAKEUP_EXT0:      a1 = WC_EXT0;      break;
          case ESP_SLEEP_WAKEUP_EXT1:      a1 = WC_EXT1;      break;
          case ESP_SLEEP_WAKEUP_TIMER:     a1 = WC_TIMER;     break;
          case ESP_SLEEP_WAKEUP_TOUCHPAD:  a1 = WC_TOUCHPAD;  break;
          case ESP_SLEEP_WAKEUP_ULP:       a1 = WC_ULP;       break;
          default:                         a1 = WC_UNDEFINED; break;
        }
      #else
        a1 = WC_UNDEFINED;
      #endif
      reg1 = encode_int(a1);
      break;

    default:
      ERROR("SYS", "Unknown operation: %d", a1);
      reg1 = FALSE;
      break;
  }

  reg2 = reg3 = NIL;
}
