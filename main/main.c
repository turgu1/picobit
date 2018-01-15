/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"
#include "hexfile.h"
#include "kb.h"
#include "interpreter.h"
#include "testing.h"

#if WORKSTATION
  #include <signal.h>

  void intHandler(int dummy) {
    keep_running = false;
  }

  bool initialisations(char * program_filename)
  {
    uint8_t * buffer;
    if ((buffer = calloc(65536, 1)) == NULL) {
      FATAL("initialisations", "Unable to allocate program space");
    }

    if (!read_hex_file(program_filename, buffer, 65536)) return false;

    program = buffer;

    if (!mm_init(program)) return false;

    vm_arch_init();

    kb_init();

    signal(SIGINT, intHandler);

    return true;
  }

  void terminate()
  {
    #if STATISTICS
      INFO("terminate", "GC Processing Count: %d.", gc_call_counter);
      #if WORKSTATION
        INFO("terminate", "Max GC Duration: %10.7f Sec.\n", max_gc_duration);
      #endif
    #endif

    if (!keep_running) fprintf(stderr, "\nInterrupted.\n");

    fflush(stderr);
    fflush(stdout);
    kb_restore();

    exit(0);
  }

  void usage(char * exename)
  {
    fprintf(stderr,
      "Usage: %s [options] intelhex_filename\n"
      "\nOptions:\n"
      #if TRACING
        "  -t  Trace\n"
      #endif
      #if TESTS
        "  -T  Tests\n"
      #endif
      "  -v  Verbose\n"
      "  -V  Version\n"
      "  -?  Print this message\n", exename);
  }

  char * options = "vV?"
  #if TRACING
    "t"
  #endif
  #if TESTS
    "T"
  #endif
  ;

  int main(int argc, char **argv)
  {
    int opt = 0;
    trace = false;
    verbose = false;
    char *fname;
    while ((opt = getopt(argc, argv, options)) != -1) {
      switch (opt) {
        case 'v':
          verbose = true;
          break;
        #if TESTS
          case 'T':
            conduct_tests();
            return 0;
            break;
        #endif
        #if TRACING
          case 't':
            trace = true;
            break;
        #endif
        case '?':
          usage(argv[0]);
          return 1;
          break;
        case 'V':
          printf("%d.%d\n", VERSION_MAJOR, VERSION_MINOR);
          return 0;
        default:
          printf("Unknown option!\n");
          usage(argv[0]);
          return 1;
      }
    }

    if ((optind > 0) && (argc > optind)) {
      fname = argv[optind];
    }
    else {
      usage(argv[0]);
      return 1;
    }
    if (!initialisations(fname)) {
      ERROR("main", "Unable to properly initialise the environment");
    }

    interpreter();

    #if DEBUGGING
      INFO("main", "Execution completed");
    #endif

    terminate();

    return 0;
  }
#endif // WORKSTATION

#if ESP32

  #include "nvs_flash.h"
  #include "esp_event.h"
  #include "esp_event_loop.h"

  // File component.mk contains an entry called COMPONENT_EMBED_FILES that
  // permit the integration of the binary code produced with the PicoBit compiler.
  // As the compiler produce an IntelHex file, it has to be translated to
  // a binary file using hex2bin program. The file needs to be called
  // "program.bin"
  extern const uint8_t program_bin_start[] asm("_binary_program_bin_start");
  extern const uint8_t program_bin_end[]   asm("_binary_program_bin_end");

  bool wifi_connected = false;

  esp_err_t event_handler(void *ctx, system_event_t *event)
  {
    switch(event->event_id) {
      case SYSTEM_EVENT_STA_CONNECTED:
        INFO("SYS", "WiFi connected");
        wifi_connected = true;
        break;

      case SYSTEM_EVENT_STA_DISCONNECTED:
        INFO("SYS", "WiFi disconnected");
        wifi_connected = false;
        break;

      default:
        break;
    }
    return ESP_OK;
  }

  bool initialisations()
  {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    verbose = VERBEUX;

    program = program_bin_start;
    max_addr = program_bin_end - program_bin_start;

    #if DEBUGGING
      esp_chip_info_t chip_info;
      esp_chip_info(&chip_info);
      printf("\n\nThis is ESP32 chip with %d CPU cores, WiFi%s%s, ",
             chip_info.cores,
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

      printf("silicon revision %d, ", chip_info.revision);

      printf("%dMB %s flash\n\n", spi_flash_get_chip_size() / (1024 * 1024),
             (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    #endif

    if (!mm_init(program)) return false;

    #if STATISTICS
      printf("\nProgram Size: %d\n", max_addr);
      printf("Ram Heap Size: %d\n", ram_heap_size);
      printf("Vector Heap Size: %d\n", vector_heap_size);
    #endif
    vm_arch_init();

    //kb_init();

    return true;
  }

  void terminate()
  {
    #if STATISTICS
      INFO("terminate", "GC Processing Count: %d.", gc_call_counter);
      INFO("terminate", "Max GC Duration: %10.7f Sec.", max_gc_duration);
      fputc('\n', stderr);
    #endif

    fflush(stderr);
    fflush(stdout);

    //kb_restore();

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\r", i);
        fflush(stdout);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("\nRestarting now.\n");
    fflush(stdout);

    esp_restart();    //if (!mm_init(program)) return false;
  }

  void app_main()
  {
    if (initialisations()) {
      interpreter();
    }

    terminate();
  }

#endif // ESP32
