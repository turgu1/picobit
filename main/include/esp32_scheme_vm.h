/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#ifndef ESP32_SCHEME_VM_H
#define ESP32_SCHEME_VM_H

#define VERSION_MAJOR  1
#define VERSION_MINOR  0

extern void terminate();

#if __linux__ || __APPLE__
  #define WORKSTATION 1
#elif ESP_PLATFORM
  #define ESP32 1
#else
  #error "Not a supported platform"
#endif

#define MQTT 1

#ifdef WORKSTATION
  #include <stdint.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <unistd.h>
  #include <string.h>

  typedef enum { false, true } bool;

  #define STATS     1
  #define DEBUGGING 1
  #define TRACING   1
  #define TESTS     1

  #define E32(instr)
  #define WKS(instr) instr
#endif

#if ESP32
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "esp_system.h"
  #include "esp_spi_flash.h"
  #include "esp_heap_caps.h"
  #include "esp_err.h"

  #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
  #include "esp_log.h"

  #define STATS     0
  #define DEBUGGING 0
  #define TRACING   0
  #define TESTS     0

  #define E32(instr) instr
  #define WKS(instr)
#endif

#define CONFIG_BIGNUM_LONG 1

#define STATISTICS (STATS || DEBUGGING)

// STATISTICS_GC will allow the computation of the maximum time used
// to process garbage collection. Note that it is the only part of
// the code that uses floating point calculation.

#define STATISTICS_GC 0

#if TRACING
  #if WORKSTATION
    #define TRACE(a, ...)   { \
      if (trace) { \
        fprintf(stderr, "\n[%p]", (void *) (last_pc.c - program)); \
        fprintf(stderr, a, __VA_ARGS__); \
        fflush(stderr); \
      } \
    }
  #else
    #define TRACE(a, ...)
  #endif
#else
  #define TRACE(a, ...)
#endif

#if DEBUGGING
  #if WORKSTATION
    #define   FATAL(a, ...)   {     fputs("\nFATAL - In " a ": ", stderr); fprintf(stderr, __VA_ARGS__); terminate(); }
    #define   ERROR(a, ...)   {     fputs("\nERROR - In " a ": ", stderr); fprintf(stderr, __VA_ARGS__); }
    #define WARNING(a, ...)   {   fputs("\nWARNING - In " a ": ", stderr); fprintf(stderr, __VA_ARGS__); }
    #define    INFO(a, ...)   if (verbose) {    fputs("\nINFO - In " a ": ", stderr); fprintf(stderr, __VA_ARGS__); }
    #define   DEBUG(a, ...)   if (verbose) {   fputs("\nDEBUG - In " a ": ", stderr); fprintf(stderr, __VA_ARGS__); }
    #define VERBOSE(a, ...)   if (verbose) { fputs("\nVERBOSE - In " a ": ", stderr); fprintf(stderr, __VA_ARGS__); }
  #else
    #define   FATAL(a, ...) { ESP_LOGE(a, __VA_ARGS__); terminate(); }
    #define   ERROR(a, ...)   ESP_LOGE(a, __VA_ARGS__)
    #define WARNING(a, ...)   ESP_LOGW(a, __VA_ARGS__)
    #define    INFO(a, ...)   ESP_LOGI(a, __VA_ARGS__)
    #define   DEBUG(a, ...)   ESP_LOGD(a, __VA_ARGS__)
    #define VERBOSE(a, ...)   ESP_LOGV(a, __VA_ARGS__)
  #endif

  #define TYPE_ERROR(proc, exp) FATAL(proc, "Expecting \"" exp "\"")
  #define EXPECT(test, proc, exp) { if (!(test)) { fprintf(stderr, "\nAt [%p]: ", (void *)(last_pc.c - program)); TYPE_ERROR(proc, exp); } }
#else
  #if WORKSTATION
    #define   FATAL(a, ...) terminate()
    #define   ERROR(a, ...)
    #define WARNING(a, ...)
    #define   DEBUG(a, ...)
    #define    INFO(a, ...)
    #define VERBOSE(a, ...)
  #else
    #define   FATAL(a, ...) { ESP_LOGE(a, __VA_ARGS__); terminate(); }
    #define   ERROR(a, ...)   ESP_LOGE(a, __VA_ARGS__)
    #define WARNING(a, ...)   ESP_LOGW(a, __VA_ARGS__)
    #define    INFO(a, ...)   // ESP_LOGI(a, __VA_ARGS__)
    #define   DEBUG(a, ...)   ESP_LOGD(a, __VA_ARGS__)
    #define VERBOSE(a, ...)   // ESP_LOGV(a, __VA_ARGS__)
  #endif
  #define      TRACE(a, ...)
  #define TYPE_ERROR(proc, exp)
  #define     EXPECT(test, proc, exp)
#endif

#pragma pack(1)

#define PRIVATE static

bool trace;
bool verbose;

#endif
