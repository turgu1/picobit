
;; NET Operations

; (NET WiFi-Init <SSID string> <Password string>)
; (NET WiFi-Connect)
; (NET WiFi-Disconnect)
; (NET WiFi-Connected?)
; (NET WiFi-Stop)
; (NET WiFi-Start)
;
; (NET MQTT-Init <servername> <port#> <clientId> <username> <password>)
; (NET MQTT-Start)
; (NET MQTT-Stop)
; (NET MQTT-Connected?)
; (NET MQTT-Publish <topic> <data (max 120 bytes)> <qos> <retain? (#t/#f)>)
; (NET MQTT-Subscribe <topic> <qos>)
; (NET MQTT-Unsuscribe <topic>)
; (NET MQTT-GotData?) ; returns #t or #f
; (NET MQTT-Get-Data) ; returns next data as a list (<topic (string)> . <data (string)>) or NIL

(define NET (lambda (op . params)
                    (#%net op params)))

;; NET Definitions

(define WiFi-Init        0)
(define WiFi-Connect     1)
(define WiFi-Disconnect  2)
(define WiFi-Ready?      3)
(define WiFi-Stop        4)
(define WiFi-Start       5)

(define MQTT-Init        6)
(define MQTT-Start       7)
(define MQTT-Stop        8)
(define MQTT-Connected?  9)
(define MQTT-Subscribe  10)
(define MQTT-Unsuscribe 11)
(define MQTT-Publish    12)
(define MQTT-GotData?   13)
(define MQTT-Data       14)
