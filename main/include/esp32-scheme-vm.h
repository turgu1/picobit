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
#endif

#if ESP32
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "esp_system.h"
  #include "esp_spi_flash.h"
  #include "esp_heap_caps.h"

  #define STATS     0
  #define DEBUGGING 1
  #define TRACING   0
  #define TESTS     0
  #define VERBOSE   true
#endif

#define CONFIG_BIGNUM_LONG 1

#define STATISTICS (STATS || DEBUGGING)

#if DEBUGGING
  #define DEBUG(format, ...) fprintf(stderr, format, ## __VA_ARGS__)
#else
  #define DEBUG(format, ...)
#endif

#if DEBUGGING
  #define   FATAL_MSG(format, ...) { fprintf(stderr, "\nFATAL - In " format, ## __VA_ARGS__); fflush(stdout); terminate(); }
  #define   ERROR_MSG(format, ...) { fprintf(stderr, "\nERROR - In " format, ## __VA_ARGS__); }
  #define WARNING_MSG(format, ...) { fprintf(stderr, "\nWARNING - In " format, ## __VA_ARGS__); }
  #define    INFO_MSG(format, ...) { if (verbose) { fprintf(stderr,    "\nINFO - In " format, ## __VA_ARGS__); } }

  #define   FATAL(a, b) { fprintf(stderr,  "\nFATAL - In %s: %s.", a, b); fflush(stdout); terminate(); }
  #define   ERROR(a, b)   fprintf(stderr,  "\nERROR - In %s: %s.", a, b)
  #define WARNING(a, b)   fprintf(stderr,"\nWARNING - In %s: %s.", a, b)
  #define    INFO(a, b)   if (verbose) { fprintf(stderr, "\nINFO - In %s: %s.", a, b); }

  #define TYPE_ERROR(proc, exp) FATAL(proc, "Expecting \"" exp "\"")
  #define EXPECT(test, proc, exp) { if (!(test)) { fprintf(stderr, "\nAt [%p]: ", (void *)(last_pc.c - program)); TYPE_ERROR(proc, exp); } }
  #define MARK(c) { if (verbose) { fputc(c, stderr); fflush(stderr); } }
#else
  #define   FATAL_MSG(format, ...) terminate()
  #define   ERROR_MSG(format, ...)
  #define WARNING_MSG(format, ...)
  #define    INFO_MSG(format, ...)

  #define   FATAL(a, b) terminate()
  #define   ERROR(a, b)
  #define WARNING(a, b)
  #define    INFO(a, b)

  #define TYPE_ERROR(proc, exp)
  #define EXPECT(test, proc, exp)
  #define MARK(c)
#endif

#pragma pack(1)

#define PRIVATE static

bool trace;
bool verbose;

#endif
