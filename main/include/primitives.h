#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#ifdef CONFIG_DEBUG_STRINGS
  extern const char* const primitive_names[];
#endif /* CONFIG_DEBUG_STRINGS */

/* For the primitive scanning pass. */
#ifndef NO_PRIMITIVE_EXPAND

  #define PRIMITIVE(scheme_name, c_name, args, index) void primitive_ ## c_name ()
  #define PRIMITIVE_UNSPEC PRIMITIVE

#endif /* NO_PRIMITIVE_EXPAND */

#endif
