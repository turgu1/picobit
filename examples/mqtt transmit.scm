;; Example of connecting to a WiFi network
;;
;; One connection is established, the LED connected to pin 2 will
;; blink once every second. After 20 seconds, the system will reset.

(define TAG "PGM")
(define LED Pin_2)

(GPIO Init LED Output)

(define (blinkit blink-count)
        (let loop ((state 0)(count (* blink-count 2)))
             (GPIO Write LED state)
             (sleep 500)
             (if (eq? count 0)
                 (SYS Reset)
                 (loop (bitwise-xor state 1) (- count 1)))))

;; ---- WiFi statup ----

(let connect-loop ()
     (NET WiFi-Init "ssid" "password")
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
      (NET MQTT-Init "10.1.0.8" 1883 "ESP32" "" "")
      (let mqtt-connect-loop ((count 5))
        (let loop ((count 20))
          (if (not (NET MQTT-Connected?))
              (begin
                (sleep 400)
                (if (not (eq? count 0))
                    (loop (- count 1))))))
        (if (not (NET MQTT-Connected?))
          (begin
            (SYS Log Warning TAG "MQTT Not connected. Trying again.")
            (if (not (eq? count 0))
              (begin
                (NET MQTT-Stop)
                (NET MQTT-Start)
                (mqtt-connect-loop (- count 1)))))))
      )
    (SYS Log Info TAG "Yeark...."))

;; ---- Publish Something ----

(if (NET MQTT-Connected?)
    (NET MQTT-Publish "test_it" "Allo from ESP32" 0 #f)
    (SYS Log Error TAG "Unable to complete the MQTT example"))

(blinkit 20)
