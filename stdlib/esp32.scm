;; SYS Definitions

(define Reset          99)
(define Watch-Dog       0)
(define Sleep           1)
(define Log             2)
(define Log-Level       3)
(define WiFi-Init       4)
(define WiFi-Connect    5)
(define WiFi-Disconnect 6)
(define WiFi-Connected? 7)

; Log Levels

(define Error     5)
(define Warning   4)
(define Info      3)
(define Debug     2)
(define Verbose   1)
(define None      0)

; System operations:
;
; (SYS Reset)
; (SYS Watch-Dog)
; (SYS Sleep <duration as ms>)
; (SYS Log <level> <tag string> <message string>)
; (SYS Log-Level <level> <tag string>) ; Set level of log for tag string. Use "*" for setting all tags
; (SYS WiFi-Init <SSID string> <Password string>)
; (SYS WiFi-Connect)
; (SYS WiFi-Disconnect)
; (SYS WiFi-Connected?)

(define SYS (lambda (op . params)
  (#%sys op params)))

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

; These values are the same as GPIO_SEL_xx from ESP-IDF
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
;(define Pin_20 20) ; Cannot be used
(define Pin_21 21)
(define Pin_22 22)
(define Pin_23 23)
;(define Pin_24 24) ; Cannot be used
(define Pin_25 25)
(define Pin_26 26)
(define Pin_27 27)
;(define Pin_28 28) ; Cannot be used
;(define Pin_29 29) ; Cannot be used
;(define Pin_30 30) ; Cannot be used
;(define Pin_31 31) ; Cannot be used
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
; (GPIO Init <Pin#> <Input|Output> [Pull-Up|Pull-Down])
; (GPIO Write <Pin#> <High|Low>)
; (GPIO Read <Pin#>)
; (GPIO Wake-Up Enable <Pin#> <Low|High>)
; (GPIO Wake-Up Disable <Pin#>)

(define GPIO (lambda (op . params)
  (#%gpio op params)))
