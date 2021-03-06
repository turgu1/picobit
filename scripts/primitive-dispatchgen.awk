function primitivegen(primitiveclass, offset) {

  if(!pr[offset, "scheme_name"])
    return;

  print "      case " primitiveclass " :"
  #print "        #ifdef CONFIG_DEBUG_STRINGS"
  #print "          TRACE(\"  (%s)\", primitive_names[(instr & 0x0F) + " offset "]);"
  #print "        #else"
  #print "          TRACE(\"  (<primitive>)\");"
  #print "        #endif"
  #print ""
  print "        switch (instr & 0x0F) {"

  for (i = offset; i < offset + 16; i++) {
    if(!pr[i, "scheme_name"]) continue;

    print "          case " (i - offset) " :"
    print "            TRACE(\"  (%s <%d>)\", \"" pr[i, "scheme_name"] "\", " pr[i, "arguments"] ");"
    if (i == 0) {
      print "            return;"
      print "";
      continue;
    }

    args = pr[i, "arguments"]
    if (args < 0) args = -args
    if (args > 3) print "            reg4 = pop();"
    if (args > 2)	print "            reg3 = pop();"
    if (args > 1)	print "            reg2 = pop();"
    if (args > 0)	print "            reg1 = pop();"

    print "            primitive_" pr[i, "c_name"] "();"

    if(!match(pr[i, "scheme_options"], "unspecified-result"))
      print "            env = new_pair(reg1, env);"

    print "            break;"
    if ((i < (offset + 15)) && (pr[i + 1, "scheme_name"])) print ""
  }

  print "        }"
  print "        break;"
  print ""
}

END {
  primitivegen("PRIMITIVE1", 0);
  primitivegen("PRIMITIVE2", 16);
  primitivegen("PRIMITIVE3", 32);
  primitivegen("PRIMITIVE4", 48);
}
