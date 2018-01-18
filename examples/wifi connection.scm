
;; Example of connecting to a WiFi network
;;
;; One connection is established, the LED connected to pin 2 will
;; blink once every second. After 20 seconds, the system will reset
;; (reaching the end of the program).

(define WIFI-SSID     "your SSID")
(define WIFI-PSW      "password")

(define TAG "PGM")
(define LED Pin_2)

(GPIO Init LED Output)
(SYS Log-Level Info "*")

(define (blinkit blink-count)
        (let loop ((state 0)(count (* blink-count 2)))
             (GPIO Write LED state)
             (sleep 500)
             (if (eq? count 0)
                 (SYS Reset)
                 (loop (bitwise-xor state 1) (- count 1)))))


(let connect-loop ()
     (NET WiFi-Init WIFI-SSID WIFI-PSW)
     (let loop ((count 20))
          (sleep 400)
          (if (not (NET WiFi-Connected?))
              (if (not (eq? count 0)) (loop (- count 1)))))
     (if (not (NET WiFi-Connected?))
         (begin
           (SYS Log Info TAG "WiFi not connected. Trying again.")
           (NET WiFi-Stop)
           (connect-loop))))

(if (NET WiFi-Connected?)
    (SYS Log Info TAG "Yeah!!!")
    (SYS Log Info TAG "Yeark...."))

(blinkit 20)
