
;; Example of use of Deep Sleep
;;
;; Led will be blinking 20 times. After that, the ESP32 will
;; be put in deep sleep mode for 20 seconds. It will restart
;; to do the same process again and again.

(define TAG "PGM")
(define LED Pin_2)

(GPIO Init LED Output)

(define (blinkit blink-count)
  (let loop ((state 0)(count (* blink-count 2)))
    (GPIO Write LED state)
    (sleep 500)
    (if (eq? count 0)
      (SYS Reset)
      (loop (bitwise-xor state 1) (- count 1))))
  )

(blinkit 20)
(SYS Deep-Sleep 20000)
