#ifndef ESP32_UTILS_H
#define ESP32_UTILS_H

#if ESP32_UTILS
  #define PUBLIC
#else
  #define PUBLIC extern
#endif

#if ESP32
  PUBLIC cell_p check(esp_err_t result);
  PUBLIC bool esp32_init();
  
  PUBLIC bool wifi_connected;
#endif

#undef PUBLIC
#endif
