;; bitwise not tests

(displayln (= (bitwise-not #xf0) -241))
(displayln (= (bitwise-not #x30)  -49))
(displayln (= (bitwise-not #x00)   -1))
(displayln (= (bitwise-not #xFF) -256))
(displayln (= (bitwise-not #x04)   -5))
(displayln (= (bitwise-not #x08)   -9))
(displayln (= (bitwise-not #x18)  -25))
(displayln (= (bitwise-not #x1823122312) -103667606291))
