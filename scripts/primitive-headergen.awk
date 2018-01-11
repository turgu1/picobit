END {
  print "#include \"esp32-scheme-vm.h\""
  print "#include \"primitives.h\""
  print ""
  print "#ifdef CONFIG_DEBUG_STRINGS"
  print "const char * const primitive_names[] = {"

  for(i = 0; i <= max_idx; i++) {
    sep = (i < max_idx) ? "," : ""
    print "  \"" pr[i, "scheme_name"] "\"" sep
  }

  print "};"
  print "#endif /* CONFIG_DEBUG_STRINGS */"

  print ""

  for(i = 1; i <= max_idx; i++) {
    print "extern void primitive_" pr[i, "c_name"] "();"
  }
}
