/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#ifndef INTERPRETER_H
#define INTERPRETER_H

#ifdef INTERPRETER
  #define PUBLIC
#else
  #define PUBLIC extern
#endif

/** Code Interpreter.

  This is the principal function of the virtual machine. Usually, it will
  loop indefinitely unless a HALT instruction is received.

  code is the memory address where the code reside. It is considered as
  a read-only portion of the memory map and is byte addresable.

 */

PUBLIC uint8_t prepare_arguments(int8_t nbr_args);
PUBLIC void build_environment(uint8_t nbr_args);

PUBLIC void interpreter();

#undef PUBLIC
#endif
