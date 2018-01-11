;; bitwise and tests

(displayln (= (bitwise-and #xf0 #x1f) #x10))
(displayln (= (bitwise-and #x30 #xE1) #x20))
(displayln (= (bitwise-and #x00 #x05) #x00))
(displayln (= (bitwise-and #xFF #xFF) #xFF))
(displayln (= (bitwise-and #x00 #x00) #x00))
(displayln (= (bitwise-and #x08 #x0C) #x08))
(displayln (= (bitwise-and #x18 #x35) #x10))
(displayln (= (bitwise-and #x1823122312 #x351234123456) 69257535506))
