;; SYS Definitions

(define Reset    99)
(define Watch-Dog 0)

(define SYS (lambda (op . params))
  (#%sys op params))

;; GPIO Definitions

(define Init    0)
(define Read    1)
(define Write   2)
(define Wake-Up 3)

(define Output 0)
(define Input  1)

(define Pull-Up   0)
(define Pull-Down 1)

(define Low  0)
(define High 1)

(define Disable 0)
(define Enable  1)

(define Pin_0 0)
(define Pin_1 1)
(define Pin_2 2)
(define Pin_3 3)
(define Pin_4 4)
(define Pin_5 5)
(define Pin_6 6)
(define Pin_7 7)
(define Pin_8 8)
(define Pin_9 9)
(define Pin_10 10)
(define Pin_11 11)
(define Pin_12 12)
(define Pin_13 13)
(define Pin_14 14)
(define Pin_15 15)
(define Pin_16 16)
(define Pin_17 17)
(define Pin_18 18)
(define Pin_19 19)
(define Pin_20 20)
(define Pin_21 21)
(define Pin_22 22)
(define Pin_23 23)
(define Pin_24 24)
(define Pin_25 25)
(define Pin_26 26)
(define Pin_27 27)
(define Pin_28 28)
(define Pin_29 29)
(define Pin_30 30)
(define Pin_31 31)
(define Pin_32 32)
(define Pin_33 33)
(define Pin_34 34)
(define Pin_35 35)
(define Pin_36 36)
(define Pin_37 37)
(define Pin_38 38)
(define Pin_39 39)

; GPIO Operations:
;
; (GPIO Init Pin# <Input|Output> [Pull-Up|Pull-Down])
; (GPIO Write Pin# <High|Low>)
; (GPIO Read Pin #)
; (GPIO Wake-Up Enable Pin# <Low|High>)
; (GPIO Wake-Up Disable Pin#)

(define GPIO (lambda (op . params)
  (#%gpio op params)))
