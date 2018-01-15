;; SYS Definitions

(define Reset           0)
(define Deep-Sleep      1)
(define Light-Sleep     2)
(define Watch-Dog       3)
(define Sleep           4)
(define Log             5)
(define Log-Level       6)
(define WakeUp-Cause    7)

; Log Levels

(define Error     5)
(define Warning   4)
(define Info      3)
(define Debug     2)
(define Verbose   1)
(define None      0)

; WakeUp Causes

(define WakeUp-Cause-Undefined 0)
(define WakeUp-Cause-Ext0      1)
(define WakeUp-Cause-Ext1      2)
(define WakeUp-Cause-Timer     3)
(define WakeUp-Cause-Touchpad  4)
(define WakeUp-Cause-ULP       5)

; System operations:
;
; (SYS Reset)
; (SYS Deep-Sleep [wakeUp time is ms])
; (SYS Light-Sleep [wakeUp time is ms])
; (SYS Watch-Dog)
; (SYS Sleep <duration as ms>)
; (SYS Log <level> <tag string> <message string>)
; (SYS Log-Level <level> <tag string>) ; Set level of log for tag string. Use "*" for setting all tags
; (SYS Wake-Up-Cause) ; Returns the cause of a system wakeUp

(define LOG (lambda (level tag message)
                    (SYS Log level tag message)))

(define SYS (lambda (op . params)
  (#%sys op params)))

;; GPIO Definitions

(define Init      0)
(define Read      1)
(define Write     2)
(define Wake-Up   3)

(define Output    0)
(define Input     1)

(define Pull-Up   0)
(define Pull-Down 1)

(define Low       0)
(define High      1)

(define Disable   0)
(define Enable    1)

; These values are the same as GPIO_SEL_xx from ESP-IDF
(define Pin_0     0)
(define Pin_1     1)
(define Pin_2     2)
(define Pin_3     3)
(define Pin_4     4)
(define Pin_5     5)
(define Pin_6     6)
(define Pin_7     7)
(define Pin_8     8)
(define Pin_9     9)
(define Pin_10   10)
(define Pin_11   11)
(define Pin_12   12)
(define Pin_13   13)
(define Pin_14   14)
(define Pin_15   15)
(define Pin_16   16)
(define Pin_17   17)
(define Pin_18   18)
(define Pin_19   19)
;(define Pin_20 20) ; Cannot be used
(define Pin_21   21)
(define Pin_22   22)
(define Pin_23   23)
;(define Pin_24 24) ; Cannot be used
(define Pin_25   25)
(define Pin_26   26)
(define Pin_27   27)
;(define Pin_28 28) ; Cannot be used
;(define Pin_29 29) ; Cannot be used
;(define Pin_30 30) ; Cannot be used
;(define Pin_31 31) ; Cannot be used
(define Pin_32   32)
(define Pin_33   33)
(define Pin_34   34)
(define Pin_35   35)
(define Pin_36   36)
(define Pin_37   37)
(define Pin_38   38)
(define Pin_39   39)

; GPIO Operations:
;
; (GPIO Init <Pin#> <Input|Output> [Pull-Up|Pull-Down])
; (GPIO Write <Pin#> <High|Low>)
; (GPIO Read <Pin#>)
; (GPIO Wake-Up Enable <Pin#> <Low|High>)
; (GPIO Wake-Up Disable <Pin#>)

(define GPIO (lambda (op . params)
  (#%gpio op params)))

;; NET Definitions

(define WiFi-Init       0)
(define WiFi-Connect    1)
(define WiFi-Disconnect 2)
(define WiFi-Connected? 3)
(define WiFi-Stop       4)
(define WiFi-Start      5)

;; NET Operations

; (NET WiFi-Init <SSID string> <Password string>)
; (NET WiFi-Connect)
; (NET WiFi-Disconnect)
; (NET WiFi-Connected?)
; (NET WiFi-Stop)
; (NET WiFi-Start)

(define NET (lambda (op . params)
  (#%net op params)))
