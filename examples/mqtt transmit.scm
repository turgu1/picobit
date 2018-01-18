;; Example of connecting to a WiFi network and transmission through MQTT
;;
;; One connection is established, the LED connected to pin 2 will
;; blink once every second.

(define WIFI-SSID     "your SSID")
(define WIFI-PSW      "password")
(define MQTT-HOST     "MQTT host address")
(define MQTT-PORT     1883)
(define MQTT-USER     "")
(define MQTT-USER-PSW "")
(define MQTT-TOPIC    "ESP32_Trial")
(define MQTT-MSG      "Hello from PicoBit ESP32!!!")

(define TAG "PGM")
(define LED Pin_2)

(GPIO Init LED Output)

(define (blinkit blink-count)
        (let loop ((state 0)(count (* blink-count 2)))
             (GPIO Write LED state)
             (sleep 500)
             (if (not (eq? count 0))
                 (loop (bitwise-xor state 1) (- count 1)))))

;; ---- WiFi statup ----

(let connect-loop ()
     (NET WiFi-Init WIFI-SSID WIFI-PSW)
     (let loop ((count 20))
        (if (not (NET WiFi-Ready?))
            (begin
              (sleep 400)
              (if (not (eq? count 0))
                  (loop (- count 1))))))

     (if (not (NET WiFi-Ready?))
         (begin
           (SYS Log Info TAG "WiFi not ready. Trying again.")
           (NET WiFi-Stop)
           (connect-loop))))

;; ---- MQTT Startup ----

(if (NET WiFi-Ready?)
    (begin
      (SYS Log Info TAG "Now starting MQTT")
      (NET MQTT-Init MQTT-HOST MQTT-PORT MQTT-CLIENT-ID MQTT-USER MQTT-USER-PSW)
      (let mqtt-connect-loop ((count 5))
        (let loop ((count 20))
          (if (not (NET MQTT-Connected?))
              (begin
                (sleep 400)
                (if (not (eq? count 0))
                    (loop (- count 1))))))
        (if (not (NET MQTT-Connected?))
          (begin
            (SYS Log Info TAG "MQTT Not connected. Trying again.")
            (if (not (eq? count 0))
              (begin
                (NET MQTT-Stop)
                (NET MQTT-Start)
                (mqtt-connect-loop (- count 1)))))))
      )
    (SYS Log Info TAG "Yeark...."))

;; ---- Publish Something ----

(if (NET MQTT-Connected?)
    (let loop ((count 0))
      (SYS Log Info TAG "Sending a message")
      (NET MQTT-Publish MQTT-TOPIC MQTT-MSG QOS-At-Most-Once #f)
      (blinkit 1)
      (loop (+ count 1)))
    (displayln "Unable to complete the MQTT example"))

(blinkit 20)
