BEGIN {
	# #%halt is a bit special
	pr[0, "scheme_name"] = "#%halt"
	pr[0, "arguments"] = 0
  pr[0, "index"] = 0

  max_idx = 0
}

/^\W*PRIMITIVE(_UNSPEC|)/ {

	match($0, /\([ \t]*(.+?)[ \t]*,[ \t]*(\w+)[ \t]*,[ \t]*(\w+)[ \t]*,[ \t]*(\w+)[ \t]*\)/, opts)

  idx = strtonum(opts[4])

  if (match($0, "PRIMITIVE_UNSPEC")) {
		pr[idx, "scheme_options"] = "#:unspecified-result"
	}

  if (pr[idx, "scheme_name"]) {
    print "Index " idx " already in used: " opts[1] " is a duplicate." > "/dev/stderr"
    exit 1
  }

  pr[idx, "scheme_name"] = opts[1]
	pr[idx, "c_name"     ] = opts[2]
	pr[idx, "arguments"  ] = opts[3]
  pr[idx, "index"      ] = opts[4]

  if (idx > max_idx) max_idx = idx

  if (idx >= 64) {
		print "" > "/dev/stderr"
		print "  ERROR: More than 64 (0..63) primitives are defined." > "/dev/stderr"
		print "    The bytecode cannot reference more than 64 different" > "/dev/stderr"
		print "    primitives at the moment." > "/dev/stderr"
		print "" > "/dev/stderr"
		exit 1
	}
}
