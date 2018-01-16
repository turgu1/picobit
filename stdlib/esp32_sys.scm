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

; (LOG <level> <tag string> <message string>))

(define LOG (lambda (level tag message)
                    (SYS Log level tag message)))

(define SYS (lambda (op . params)
                    (#%sys op params)))

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
