/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#include "esp32-scheme-vm.h"

#if WORKSTATION

#include "vm-arch.h"

#include "testing.h"

#define HEXFILE 1
#include "hexfile.h"

PRIVATE FILE *f;
PRIVATE uint8_t checksum;

PRIVATE uint8_t hex2bin(char ch)
{
  if ((ch >= '0') && (ch <= '9')) return ch - '0';
  if ((ch >= 'a') && (ch <= 'f')) return 10 + (ch - 'a');
  if ((ch >= 'A') && (ch <= 'F')) return 10 + (ch - 'A');
  return 0;
}

PRIVATE uint8_t hex2byte(char **str)
{
  uint8_t val;

  if (**str == 0) return 0;
  val = hex2bin(*(*str)++);
  if (**str == 0) return 0;
  val = (val << 4) + hex2bin(*(*str)++);
  checksum += val;

  return val;
}

PRIVATE uint16_t hex2short(char **str)
{
  uint16_t val;

  val = hex2byte(str);
  val = (val << 8) + hex2byte(str);

  return val;
}

bool read_hex_file(char * filename, uint8_t * buffer, int size)
{
  char line[100];

  uint8_t  len;
  uint16_t addr;
  uint8_t  type;
  char   * ptr;
  bool     completed;
  bool     error;

  if ((f = fopen(filename, "r")) == NULL) {
    ERROR("read_hex_file", "Unable to open file %s.", filename);
    return false;
  }

  completed = error = false;
  max_addr = 0;

  while (!feof(f) && !completed && !error) {
    if (fgets(line, 99, f) == NULL) {
      ERROR("read_hex_file", "I/O Error reading file %s.", filename);
      return false;
    }
    ptr = line;
    while (*ptr == ' ') ptr++;
    if (*ptr++ == ':') {
      checksum = 0;
      len  = hex2byte(&ptr);
      addr = hex2short(&ptr);
      type = hex2byte(&ptr);

      switch (type) {
        case 0 :  // Data
          if ((addr + len) > size) {
            ERROR("read_hex_file", "buffer too short");
            error = true;
          }
          while (len--) buffer[addr++] = hex2byte(&ptr);
          break;

        case 1 :  // EOF
          while (len--) hex2byte(&ptr);
          completed = true;
          break;

        default :
          WARNING("read_hex_file", "Unsupported record type: %d.", type);
          while (len--) hex2byte(&ptr);
          break;
      }

      if (addr > max_addr) max_addr = addr;

      if (!error) {
        hex2byte(&ptr);
        if (checksum != 0) {
          ERROR("read_hex_file", "Bad Checksum");
          error = true;
        }
      }
    }
  }

  fclose (f);

  #if DEBUGGING
    INFO("read_hex_file", "Load size: %u", max_addr);
  #endif

  // Little endian...
  error = error || (buffer[0] != 0xD7) || (buffer[1] != 0xFB);

  return !error;
}

#if TESTS
void hexfile_tests()
{
  TESTM("hexfile");

  uint8_t * buffer;

  buffer = calloc(40000, 1);
  EXPECT_TRUE(buffer != NULL, "Buffer allocation problem");
  if (buffer != NULL) {
    bool result = read_hex_file("test.hex", buffer, 40000);
    EXPECT_TRUE(result, "Unable to read test.hex properly");
    if (result) {
      // Little endian...
      EXPECT_TRUE(buffer[0] == 0xD7, "Marker wrong at code location 0");
      EXPECT_TRUE(buffer[1] == 0xFB, "Marker wrong at code location 1");
      printf("Globals: %d Constants: %d", buffer[2], buffer[3]);
    }
  }
}
#endif

#endif // WORKSTATION
