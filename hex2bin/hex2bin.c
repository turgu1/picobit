#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define    INFO_MSG(format, ...) fprintf(stderr,    "\nINFO - In " format ".\n", ## __VA_ARGS__)
#define WARNING_MSG(format, ...) fprintf(stderr, "\nWARNING - In " format ".\n", ## __VA_ARGS__)
#define   ERROR_MSG(format, ...) fprintf(stderr,   "\nERROR - In " format ".\n", ## __VA_ARGS__)
#define       ERROR(a, b)        fprintf(stderr,   "\nERROR - In %s: %s.\n", a, b)

#define EXPECT_TRUE(val, info) if (!(val)) fprintf(stderr, "\nERROR - In " info ".\n")

typedef enum { false, true } bool;

FILE    *f;
uint8_t  checksum;
int      max_addr;

uint8_t hex2bin(char ch)
{
  if ((ch >= '0') && (ch <= '9')) return ch - '0';
  if ((ch >= 'a') && (ch <= 'f')) return 10 + (ch - 'a');
  if ((ch >= 'A') && (ch <= 'F')) return 10 + (ch - 'A');
  return 0;
}

uint8_t hex2byte(char **str)
{
  uint8_t val;

  if (**str == 0) return 0;
  val = hex2bin(*(*str)++);
  if (**str == 0) return 0;
  val = (val << 4) + hex2bin(*(*str)++);
  checksum += val;

  return val;
}

uint16_t hex2short(char **str)
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
    ERROR_MSG("read_hex_file: Unable to open file %s", filename);
    return false;
  }

  completed = error = false;
  max_addr = 0;

  while (!feof(f) && !completed && !error) {
    if (fgets(line, 99, f) == NULL) {
      ERROR_MSG("read_hex_file: I/O Error reading file %s", filename);
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
          WARNING_MSG("read_hex_file: Unsupported record type: %d", type);
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

  INFO_MSG("read_hex_file: Load size: %u", max_addr);

  // Little endian...
  error = error || (buffer[0] != 0xD7) || (buffer[1] != 0xFB);

  return !error;
}

int main(int argc, char **argv)
{
  if (argc != 3) {
    fprintf(stderr,
      "\n"
      "Usage: hex2bin input_hex_file output_bin_file\n\n"
      "(c) GPL3 - Guy Turcotte - January 2018\n"
    );

    return 1;
  }
  else {
    uint8_t * buffer;
    bool      result;

    result = false;

    buffer = calloc(60000, 1);
    EXPECT_TRUE(buffer != NULL, "main: Buffer allocation problem");

    if (buffer != NULL) {
      result = read_hex_file(argv[1], buffer, 60000);

      EXPECT_TRUE(result, "main: Unable to read test.hex properly");

      if (result) {
        if ((f = fopen(argv[2], "wb")) == NULL) {
          ERROR_MSG("main: Unable to open file %s", argv[2]);
          result = false;
        }
        else {
          fwrite(buffer, 1, max_addr, f);
          fclose(f);
        }
      }
    }

    INFO_MSG("Completed %s", result ? "sucessfully" : "with error");

    return !result;
  }
}
