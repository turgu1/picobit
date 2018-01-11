(define (factorial n)
  (acc-factorial n 1))


;; auxiliary function that takes an additional parameter (the accumulator,
;; i.e. the result computed so far)
(define (acc-factorial n sofar)
  (if (= n 0)
      sofar
      (acc-factorial (- n 1) (* sofar n))))

(displayln (factorial 90))
