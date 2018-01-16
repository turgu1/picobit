
;; NET Operations

; (NET WiFi-Init <SSID string> <Password string>)
; (NET WiFi-Connect)
; (NET WiFi-Disconnect)
; (NET WiFi-Connected?)
; (NET WiFi-Stop)
; (NET WiFi-Start)

(define NET (lambda (op . params)
                    (#%net op params)))

;; NET Definitions

(define WiFi-Init       0)
(define WiFi-Connect    1)
(define WiFi-Disconnect 2)
(define WiFi-Connected? 3)
(define WiFi-Stop       4)
(define WiFi-Start      5)
