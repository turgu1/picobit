/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#ifndef KB_H
#define KB_H

#ifndef KB
  #define PUBLIC extern
#else
  #define PUBLIC
#endif

PUBLIC void kb_init();
PUBLIC void kb_restore();
PUBLIC char kb_getch(void);

#undef PUBLIC
#endif
