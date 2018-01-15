
;; Example of use of Deep Sleep
;;
;; Led will be blinking 5 times. After that, the ESP32 will
;; be put in deep sleep mode for 10 seconds. It will restart
;; to do the same process again and again.

(define TAG "PGM")
(define LED Pin_2)

(define (blinkit blink-count)
  (let loop ((state 0)(count (* blink-count 2)))
    (GPIO Write LED state)
    (sleep 500)
    (if (not (eq? count 0))
      (loop (bitwise-xor state 1) (- count 1))))
  )

(SYS Log-Level Info "*")

(display   "WakeUp Cause: ")
(displayln (SYS WakeUp-Cause))

(GPIO Init LED Output)

(blinkit 5)
(SYS Deep-Sleep 10000)

;; Will never come here
