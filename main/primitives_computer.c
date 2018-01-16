/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

// primitives-computer
// Builtin Indexes: 38..41

#include "esp32_scheme_vm.h"
#include "vm_arch.h"
#include "mm.h"
#include "testing.h"

#include "primitives.h"
#include "kb.h"

#include <sys/time.h>

// most of this is for the host architecture
// there's some PIC18 code in there too
// it should eventually be moved to its own architecture

PRIVATE void show_it (cell_p o)
{
  cell_p car;
  cell_p cdr;

  #if 0
    printf ("[%d]", o);
  #endif

  if (o == FALSE) {
    printf ("#f");
  } else if (o == TRUE) {
    printf ("#t");
  } else if (o == NIL) {
    printf ("()");
  } else if ((o >= SMALL_INT_START) && (o <= SMALL_INT_MAX)) {
    printf ("%d", decode_int(o));
  } else {
    if ((IN_RAM(o) && (RAM_IS_BIGNUM(o) || RAM_IS_FIXNUM(o))) || (IN_ROM(o) && (ROM_IS_BIGNUM(o) || ROM_IS_FIXNUM(o)))) {
      printf ("%d", decode_int(o));
    }
    else if ((IN_RAM(o) && RAM_IS_PAIR(o)) || (IN_ROM(o) && ROM_IS_PAIR(o))) {
      if (IN_RAM(o)) {
        car = RAM_GET_CAR(o);
        cdr = RAM_GET_CDR(o);
      }
      else {
        car = ROM_GET_CAR(o);
        cdr = ROM_GET_CDR(o);
      }

      printf("(");

loop:
      show_it(car);

      // if (RAM_IS_MARKED(o)) {
      //   printf(" ...");
      // }
      // else
      {
        if (cdr == NIL) {
          printf(")");
        }
        else if ((IN_RAM(cdr) && RAM_IS_PAIR(cdr))
                   || (IN_ROM(cdr) && ROM_IS_PAIR(cdr))) {
          if (IN_RAM(cdr)) {
            car = RAM_GET_CAR(cdr);
            cdr = RAM_GET_CDR(cdr);
          }
          else {
            car = ROM_GET_CAR(cdr);
            cdr = ROM_GET_CDR(cdr);
          }

          printf(" ");
          goto loop;
        }
        else {
          printf(" . ");
          show_it(cdr);
          printf(")");
        }
      }
    }
    else if ((IN_RAM(o) && RAM_IS_SYMBOL(o)) || (IN_ROM(o) && ROM_IS_SYMBOL(o))) {
      printf("#<symbol>");
    }
    else if (IN_RAM(o) && RAM_IS_STRING(o)) {
      o = RAM_STRING_GET_CHARS(o);
      while (o != NIL) {
        putchar(decode_int(RAM_GET_CAR(o)));
        o = RAM_GET_CDR(o);
      }
      fflush(stdout);
    }
    else if (IN_ROM(o) && ROM_IS_STRING(o)) {
      o =  ROM_STRING_GET_CHARS(o);
      while (o != NIL) {
        putchar(decode_int(ROM_GET_CAR(o)));
        o = ROM_GET_CDR(o);
      }
      fflush(stdout);
    }
    else if ((IN_RAM(o) && RAM_IS_CSTRING(o)) || (IN_ROM(o) && ROM_IS_CSTRING(o))) {
      printf("#<c_string>");
    }
    else if ((IN_RAM(o) && RAM_IS_VECTOR(o)) || (IN_ROM(o) && ROM_IS_VECTOR(o))) {
      printf("#<vector %d>", o);
    }
    else if (IN_RAM(o) && RAM_IS_CONTINUATION(o)) {
      printf("(");
      cdr = RAM_GET_CONT_PARENT(o);
      car = RAM_GET_CONT_CLOSURE(o);
      // ugly hack, takes advantage of the fact that pairs and
      // continuations have the same layout
      goto loop;
    }
    else if (IN_RAM(o) && RAM_IS_CLOSURE(o)) {
      cell_p env;
      code_p entry;

      env = RAM_GET_CLOSURE_ENV(o);
      entry = RAM_GET_CLOSURE_ENTRY_POINT(o);

      printf("{0x%04x ", (int) entry);
      show_it(env);
      printf("}");
    }
    else {
      FATAL("show_it", "received an unknown structure");
    }
  }

  fflush(stdout);
}

void show (cell_p o)
{
  show_it(o);
  //unmark_ram();
}
PRIVATE void print (cell_p o)
{
  show(o);
  printf ("\n");
  fflush (stdout);
}

PRIMITIVE_UNSPEC(print, print, 1, 38)
{
  print(reg1);

  reg1 = NIL;
}


// To limit the value to something inside 32 bits,
// a second is equal to 100 ticks.

PRIVATE uint32_t read_clock ()
{
  uint32_t now = 0;

  static uint32_t start = 0;
  struct timeval tv;

  if (gettimeofday (&tv, NULL) == 0) {
    now = tv.tv_sec * 100 + tv.tv_usec / 10000;

    if (start == 0) {
      start = now;
    }

    now -= start;
  }

  return now;
}

PRIMITIVE(clock, clock, 0, 39)
{
  reg1 = encode_int(read_clock());
}

PRIMITIVE(#%getchar-wait, getchar_wait, 2, 40)
{
  decode_2_int_args();
  a1 = read_clock() + a1;

  if (a2 < 1 || a2 > 3) {
    ERROR("getchar-wait", "argument out of range");
  }

  reg1 = reg2 = NIL;
  a3 = 0;
  do {
    if ((a3 = kb_getch()))  {
      break;
    }
  } while (read_clock () < a1);

  reg1 = encode_int(a3);
}

PRIMITIVE_UNSPEC(#%putchar, putchar, 2, 41)
{
  decode_2_int_args ();

  if (a1 > 255 || a2 < 1 || a2 > 3) {
    ERROR("putchar", "argument out of range");
  }

  putchar (a1);
  fflush (stdout);

  reg1 = NIL;
  reg2 = NIL;
}

#if 0
PRIMITIVE(beep, beep, 2, 45)
{
  decode_2_int_args ();

  if (a1 < 1 || a1 > 255) {
    ERROR("beep", "argument out of range");
  }

  printf ("beep -> freq-div=%d duration=%d\n", a1, a2 );
  fflush (stdout);

  reg1 = NIL;
  reg2 = NIL;
}

PRIMITIVE(adc, adc, 1, 46)
{
  uint16 x;

  a1 = decode_int (reg1);

  if (a1 < 1 || a1 > 3) {
    ERROR("adc", "argument out of range");
  }

#ifdef  PICOBOARD2
  x = adc( a1 );
#endif

#ifdef CONFIG_ARCH_HOST
  x = read_clock () & 255;

  if (x > 127) {
    x = 256 - x;
  }

  x += 200;
#endif

//	reg1 = encode_int (x);
  reg1 = encode_int (0);
}

PRIMITIVE(sernum, sernum, 0, 47)
{
  uint16 x = 0;

  reg1 = encode_int (x);
}


/*---------------------------------------------------------------------------*/

// networking primitives
// to enable them, compilation must be done with the -lpcap option

PRIMITIVE_UNSPEC(network-init, network_init, 0, 48)
{
  // TODO maybe put in the initialization of the vm
#ifdef NETWORKING
  handle = pcap_open_live(INTERFACE, MAX_PACKET_SIZE, PROMISC, TO_MSEC, errbuf);

  if (handle == NULL) {
    ERROR("network-init", "interface not responding");
  }

#endif
}

PRIMITIVE_UNSPEC(network-cleanup, network_cleanup, 0, 49)
{
  // TODO maybe put in halt ?
#ifdef NETWORKING
  pcap_close(handle);
#endif
}

PRIMITIVE(receive-packet-to-u8vector, receive_packet_to_u8vector, 1, 50)
{
  // reg1 is the vector in which to put the received packet
  if (!RAM_VECTOR_P(reg1)) {
    TYPE_ERROR("receive-packet-to-u8vector", "vector");
  }

#ifdef NETWORKING
  // receive the packet in the buffer
  struct pcap_pkthdr header;
  const u_char *packet;

  packet = pcap_next(handle, &header);

  if (packet == NULL) {
    header.len = 0;
  }

  if (ram_get_car (reg1) < header.len) {
    ERROR("receive-packet-to-u8vector", "packet longer than vector");
  }

  if (header.len > 0) { // we have received a packet, write it in the vector
    reg2 = VEC_TO_RAM_OBJ(ram_get_cdr (reg1));
    reg1 = header.len; // we return the length of the received packet
    a1 = 0;

    while (a1 < reg1) {
      ram_set_fieldn (reg2, a1 % 4, (char)packet[a1]);
      a1++;
      reg2 += (a1 % 4) ? 0 : 1;
    }

    reg2 = FALSE;
  } else { // no packet to be read
    reg1 = FALSE;
  }

#endif
}

PRIMITIVE(send-packet-from-u8vector, send_packet_from_u8vector, 2, 51)
{
  // reg1 is the vector which contains the packet to be sent
  // reg2 is the length of the packet
  // TODO only works with ram vectors for now
  if (!RAM_VECTOR_P(reg1)) {
    TYPE_ERROR("send-packet-from-vector!", "vector");
  }

  a2 = decode_int (reg2); // TODO fix for bignums
  a1 = 0;

#ifdef NETWORKING

  // TODO test if the length of the packet is longer than the length of the vector
  if (ram_get_car (reg1) < a2) {
    ERROR("send-packet-from-u8vector", "packet cannot be longer than vector");
  }

  reg1 = VEC_TO_RAM_OBJ(ram_get_cdr (reg1));

  // copy the packet to the output buffer
  while (a1 < a2) {
    buf[a1] = ram_get_fieldn (reg1, a1 % 4);
    a1++;
    reg1 += (a1 % 4) ? 0 : 1;
  }

  // TODO maybe I could just give pcap the pointer to the memory

  if (pcap_sendpacket(handle, buf, a2) < 0) { // TODO an error has occurred, can we reuse the interface ?
    reg1 = FALSE;
  } else {
    reg1 = TRUE;
  }

#endif

  reg2 = FALSE;
}
#endif

#if TESTS
void primitives_computer_tests()
{
  TESTM("primitives-computer");

}
#endif
