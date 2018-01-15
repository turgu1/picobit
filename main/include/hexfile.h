/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#ifndef HEXFILE_H
#define HEXFILE_H

#if WORKSTATION

  #ifdef HEXFILE
    #define PUBLIC
  #else
    #define PUBLIC extern
  #endif

  bool read_hex_file(char * filename, uint8_t * buffer, int size);

  #undef PUBLIC

#endif // WORKSTATION

#endif
