
#include "esp32_scheme_vm.h"
#include "vm_arch.h"

#if ESP32

#define ESP32_UTILS 1
#include "esp32_utils.h"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"

esp_err_t event_handler(void *ctx, system_event_t *event)
{
  switch(event->event_id) {

    case SYSTEM_EVENT_STA_START:
      INFO("NET", "Wifi Started");
      ESP_ERROR_CHECK(esp_wifi_connect());
      break;

    case SYSTEM_EVENT_STA_STOP:
      INFO("NET", "Wifi Stopped");
      break;
      
    case SYSTEM_EVENT_STA_CONNECTED:
      INFO("NET", "WiFi connected");
      break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
      INFO("NET", "WiFi disconnected");
      wifi_ready = false;
      break;

    case SYSTEM_EVENT_STA_GOT_IP:
      INFO("NET", "Got IP");
      wifi_ready = true;
      break;

    default:
      break;
  }
  return ESP_OK;
}

cell_p check(esp_err_t result)
{
  return result == ESP_OK ? TRUE : FALSE;
}

bool esp32_init()
{
  esp_err_t result;

  wifi_ready = false;

  result = nvs_flash_init();
  if (result == ESP_ERR_NVS_NO_FREE_PAGES) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      result = nvs_flash_init();
  }
  ESP_ERROR_CHECK(result);

  if (result != ESP_OK) return false;

  tcpip_adapter_init();

  ESP_ERROR_CHECK(result = esp_event_loop_init(event_handler, NULL));

  if (result != ESP_OK) return false;

  return true;
}

#endif
