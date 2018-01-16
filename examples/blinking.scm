;; This program will make LED connected to PIN 2 to
;; blink forever.

(define LED Pin_2)

(GPIO Init LED Output)

(let loop ((state 0))
     (GPIO Write LED state)
     (sleep 500)
     (loop (bitwise-xor state 1)))
