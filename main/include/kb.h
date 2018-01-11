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
