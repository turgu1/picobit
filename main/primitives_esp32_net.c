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

#define TAG "NET"

#if ESP32
  #include <string.h>
  #include "esp_wifi.h"
  #include "esp32_utils.h"
  #if MQTT
    #include "mqtt.h"

    mqtt_client * esp32_mqtt_client    = NULL;
    bool          esp32_mqtt_connected = false;

    PRIVATE void mqtt_connected_cb(mqtt_client * self, mqtt_event_data_t * params)
    {
        esp32_mqtt_client    = self;
        esp32_mqtt_connected = true;
        DEBUG(TAG, "MQTT Connected Callback");
    }

    PRIVATE void mqtt_disconnected_cb(mqtt_client * self, mqtt_event_data_t * params)
    {
        esp32_mqtt_connected = false;
        esp32_mqtt_client = NULL;
        DEBUG(TAG, "MQTT Disconnected Callback");
    }

    PRIVATE void mqtt_subscribe_cb(mqtt_client * self, mqtt_event_data_t * params)
    {
      // ESP_LOGI(MQTT_TAG, "[APP] Subscribe ok, test publish msg");
      // mqtt_client *client = (mqtt_client *)self;
      // mqtt_publish(client, "/test", "abcde", 5, 0, 0);
      DEBUG(TAG, "MQTT Suscribe Callback");
    }

    PRIVATE void mqtt_publish_cb(mqtt_client * self, mqtt_event_data_t * params)
    {
      DEBUG(TAG, "MQTT Publish Callback");
    }

    PRIVATE void mqtt_data_cb(mqtt_client * self, mqtt_event_data_t * params)
    {
        DEBUG(TAG, "MQTT Data Callback");
        // mqtt_client *client = (mqtt_client *)self;
        // mqtt_event_data_t *event_data = (mqtt_event_data_t *)params;
        //
        // if(event_data->data_offset == 0) {
        //
        //     char *topic = malloc(event_data->topic_length + 1);
        //     memcpy(topic, event_data->topic, event_data->topic_length);
        //     topic[event_data->topic_length] = 0;
        //     ESP_LOGI(MQTT_TAG, "[APP] Publish topic: %s", topic);
        //     free(topic);
        // }
        //
        // // char *data = malloc(event_data->data_length + 1);
        // // memcpy(data, event_data->data, event_data->data_length);
        // // data[event_data->data_length] = 0;
        // ESP_LOGI(MQTT_TAG, "[APP] Publish data[%d/%d bytes]",
        //          event_data->data_length + event_data->data_offset,
        //          event_data->data_total_length);
        // // data);
        // // free(data);
    }

    PRIVATE mqtt_settings esp32_mqtt_settings =
    {
      .connected_cb    = mqtt_connected_cb,
      .disconnected_cb = mqtt_disconnected_cb,

      .subscribe_cb    = mqtt_subscribe_cb,
      .publish_cb      = mqtt_publish_cb,
      .data_cb         = mqtt_data_cb,

      .port            = 1883,
      .lwt_topic       = "", // disabled "/lwt",
      .lwt_msg         = "offline",
      .lwt_msg_len     = 7,
      .lwt_qos         = 0,
      .lwt_retain      = 0,
      .clean_session   = 0,
      .keepalive       = 120,
      .auto_reconnect  = true
    };
  #endif
#endif

// NET Definitions

#define NET_WIFI_INIT       0
#define NET_WIFI_CONNECT    1
#define NET_WIFI_DISCONNECT 2
#define NET_WIFI_READY      3
#define NET_WIFI_STOP       4
#define NET_WIFI_START      5
#define NET_MQTT_INIT       6
#define NET_MQTT_START      7
#define NET_MQTT_STOP       8
#define NET_MQTT_CONNECTED  9
#define NET_MQTT_SUB       10
#define NET_MQTT_UNSUB     11
#define NET_MQTT_PUBLISH   12
#define NET_MQTT_GOT_DATA  13
#define NET_MQTT_DATA      14

E32(extern bool wifi_connected);

// (NET operation (params ...))
PRIMITIVE(#%net, net, 2, 44)
{
  char str1[121];
  char str2[121];
  #if MQTT
    char str3[33];
    char str4[33];
  #endif
  E32(esp_err_t result);

  EXPECT(IS_SMALL_INT(reg1), "net.0", "operation as small int");
  a1 = decode_int(reg1);

  switch (a1) {
    case NET_WIFI_INIT:
      GET_NEXT_STRING(str1, 32, "sys.", "SSID"    );
      GET_NEXT_STRING(str2, 64, "sys.", "password");
      #if ESP32
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        wifi_config_t sta_config;

        strcpy((char *) sta_config.sta.ssid,     str1);
        strcpy((char *) sta_config.sta.password, str2);
        sta_config.sta.bssid_set = false;

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
        ESP_ERROR_CHECK(result = esp_wifi_start());
        reg1 = result == ESP_OK ? TRUE : FALSE;
      #else
        reg1 = TRUE;
      #endif
      DEBUG(TAG, "WiFi Init with SSID: %s, result: %s", str1, reg1 == TRUE ? "OK" : "ERROR");
      break;

    case NET_WIFI_CONNECT:
      E32(ESP_ERROR_CHECK(result = esp_wifi_connect()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG(TAG, "WiFi Connect, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

    case NET_WIFI_DISCONNECT:
      E32(ESP_ERROR_CHECK(result = esp_wifi_disconnect()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG(TAG, "WiFi Disconnect, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

    case NET_WIFI_READY:
      E32(reg1 = (wifi_ready ? TRUE : FALSE));
      WKS(reg1 = TRUE);
      DEBUG(TAG, "WiFi is %sready", reg1 == TRUE ? "" : "not ");
      break;

    case NET_WIFI_STOP:
      E32(ESP_ERROR_CHECK(result = esp_wifi_stop()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG(TAG, "WiFi Stop, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

    case NET_WIFI_START:
      E32(ESP_ERROR_CHECK(result = esp_wifi_start()));
      E32(reg1 = result == ESP_OK ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG(TAG, "WiFi Start, result: %s", reg1 == TRUE ? "OK" : "ERROR");
      break;

#if MQTT
    case NET_MQTT_INIT:
      GET_NEXT_STRING(str1, 64, TAG, "host");
      GET_NEXT_VALUE(a1, (a1 >= 0) && (a1 <= 65535), TAG, "port #");
      GET_NEXT_STRING(str2, 32, TAG, "client id");
      GET_NEXT_STRING(str3, 32, TAG, "username");
      GET_NEXT_STRING(str4, 32, TAG, "password");
      E32(strcpy(esp32_mqtt_settings.host,      str1));
      E32(strcpy(esp32_mqtt_settings.client_id, str2));
      E32(strcpy(esp32_mqtt_settings.username,  str3));
      E32(strcpy(esp32_mqtt_settings.password,  str4));
      E32(esp32_mqtt_settings.port = a1);
      DEBUG(TAG, "MQTT Init: host(%s) port(%d) clientId(%s) username(%s)",
        str1, a1, str2, str3);
        reg1 = TRUE;
      DEBUG(TAG, "MQTT Start");
      E32(mqtt_start(&esp32_mqtt_settings));
      reg1 = TRUE;
      break;

    case NET_MQTT_START:
      E32(mqtt_start(&esp32_mqtt_settings));
      DEBUG(TAG, "MQTT Start");
      reg1 = TRUE;
      break;

    case NET_MQTT_STOP:
      E32(if (esp32_mqtt_connected) mqtt_stop());
      DEBUG(TAG, "MQTT Stop");
      reg1 = TRUE;
      break;

    case NET_MQTT_CONNECTED:
      E32(reg1 = esp32_mqtt_connected ? TRUE : FALSE);
      WKS(reg1 = TRUE);
      DEBUG(TAG, "MQTT Connected?: %s", (reg1 == TRUE) ? "True" : "False");
      break;

    case NET_MQTT_SUB:
      GET_NEXT_STRING(str1, 121, TAG, "topic");
      GET_NEXT_VALUE(a1, (a1 >= 0) && (a1 < 3), TAG, "qos");
      E32(mqtt_subscribe(esp32_mqtt_client, str1, a1));
      DEBUG(TAG, "MQTT Subscribe: topic(%s) qos(%d)", str1, a1)
      reg1 = TRUE;
      break;

    case NET_MQTT_UNSUB:
      GET_NEXT_STRING(str1, 121, TAG, "topic");
      E32(mqtt_unsubscribe(esp32_mqtt_client, str1));
      DEBUG(TAG, "MQTT Unsubscribe: topic(%s)", str1)
      reg1 = TRUE;
      break;

    case NET_MQTT_PUBLISH:
      GET_NEXT_STRING(str1, 121, TAG, "topic");
      GET_NEXT_STRING(str2, 121, TAG, "data");
      GET_NEXT_VALUE(a1, (a1 >= 0) && (a1 <= 3), TAG, "qos");
      GET_NEXT_BOOL(a2, TAG, "retain?");
      E32(mqtt_publish(esp32_mqtt_client, str1, str2, strlen(str2), a1, a2));
      DEBUG(TAG, "MQTT Publish: topic(%s) data(%s) qos(%d) retain(%s)",
        str1, str2, a1, a2 ? "true" : "false");
      reg1 = TRUE;
      break;

    case NET_MQTT_GOT_DATA:
      reg1 = FALSE;
      break;

    case NET_MQTT_DATA:
      reg1 = NIL;
      break;
#endif

    default:
      ERROR(TAG, "Unknown operation: %d", a1);
      reg1 = FALSE;
      break;
  }

  reg2 = reg3 = NIL;
}
