/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

// primitives-esp32
// Builtin Indexes: 44

#include "esp32_scheme_vm.h"
#include "vm_arch.h"
#include "mm.h"
#include "testing.h"

#include "primitives.h"

#if ESP32
  #include <string.h>
  #include "esp_wifi.h"
  #include "esp32_utils.h"
  #if MQTT
    #include "esp_mqtt.h"
  #endif
#endif

// NET Definitions

#define NET_WIFI_INIT       0
#define NET_WIFI_CONNECT    1
#define NET_WIFI_DISCONNECT 2
#define NET_WIFI_CONNECTED  3
#define NET_WIFI_STOP       4
#define NET_WIFI_START      5
#define NET_MQTT_INIT       6
#define NET_MQTT_START      7
#define NET_MQTT_STOP       8
#define NET_MQTT_SUB        9
#define NET_MQTT_UNSUB     10
#define NET_MQTT_PUBLISH   11

E32(extern bool wifi_connected);

// (NET operation (params ...))
PRIMITIVE(#%net, net, 2, 44)
{
  char str1[50];
  char str2[120];
  #if MQTT
    char str3[50];
    char str4[50];
  #endif
  E32(esp_err_t result);

  EXPECT(IS_SMALL_INT(reg1), "net.0", "operation as small int");
  a1 = decode_int(reg1);

  switch (a1) {
    case NET_WIFI_INIT:
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

    case NET_WIFI_CONNECT:
      E32(ESP_ERROR_CHECK(result = esp_wifi_connect()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG("SYS", "WiFi Connect, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

    case NET_WIFI_DISCONNECT:
      E32(ESP_ERROR_CHECK(result = esp_wifi_disconnect()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG("SYS", "WiFi Disconnect, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

    case NET_WIFI_CONNECTED:
      E32(reg1 = (wifi_connected ? TRUE : FALSE));
      WKS(reg1 = TRUE);
      DEBUG("SYS", "WiFi is %sconnected", reg1 == TRUE ? "" : "not ");
      break;

    case NET_WIFI_STOP:
      E32(ESP_ERROR_CHECK(result = esp_wifi_stop()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG("SYS", "WiFi Stop, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

    case NET_WIFI_START:
      E32(ESP_ERROR_CHECK(result = esp_wifi_start()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG("SYS", "WiFi Start, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

#if MQTT
    case NET_MQTT_INIT:
      break;

    case NET_MQTT_START:
      GET_NEXT_STRING(str2, 120, "NET", "host");
      GET_NEXT_VALUE((a1 >= 0) && (a1 <= 65535), "NET", "port #");
      GET_NEXT_STRING(str1, 50, "NET", "client id");
      GET_NEXT_STRING(str3, 50, "NET", "username");
      GET_NEXT_STRING(str4  50, "NET", "password");
      E32(esp_mqtt_start(str2, a1, str1, str3, str4));
      reg1 = TRUE;
      break;

    case NET_MQTT_STOP:
      E32(esp_mqtt_stop());
      reg1 = TRUE;
      break;

    case NET_MQTT_SUB:
      GET_NEXT_STRING(str2, 120, "NET", "topic");
      GET_NEXT_VALUE((a1 >= 0) && (a1 < 3), "NET", "qos");
      E32(result = esp_mqtt_subscribe(str2, a1));
      E32(reg1 = result ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      break;

    case NET_MQTT_UNSUB:
      GET_NEXT_STRING(str2, 120, "NET", "topic");
      E32(result = esp_mqtt_unsubscribe(str2));
      E32(reg1 = result ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      break;

    case NET_MQTT_PUBLISH:
      break;
#endif

    default:
      ERROR("net", "Unknown operation: %d", a1);
      reg1 = FALSE;
      break;
  }

  reg2 = reg3 = NIL;
}
