(define-primitive #%halt 0 0 )
(define-primitive return 1 1 )
(define-primitive pop 0 2 #:unspecified-result)
(define-primitive get-cont 0 3 )
(define-primitive graft-to-cont 2 4 #:unspecified-result)
(define-primitive return-to-cont 2 5 )
(define-primitive pair? 1 6 )
(define-primitive cons 2 7 )
(define-primitive car 1 8 )
(define-primitive cdr 1 9 )
(define-primitive set-car! 2 10 #:unspecified-result)
(define-primitive set-cdr! 2 11 #:unspecified-result)
(define-primitive null? 1 12 )
(define-primitive number? 1 13 )
(define-primitive = 2 14 )
(define-primitive #%+ 2 15 )
(define-primitive #%- 2 16 )
(define-primitive #%mul-non-neg 2 17 )
(define-primitive #%div-non-neg 2 18 )
(define-primitive #%rem-non-neg 2 19 )
(define-primitive < 2 20 )
(define-primitive > 2 21 )
(define-primitive bitwise-ior 2 22 )
(define-primitive bitwise-xor 2 23 )
(define-primitive bitwise-and 2 24 )
(define-primitive bitwise-not 1 25 )
(define-primitive eq? 2 26 )
(define-primitive not 1 27 )
(define-primitive symbol? 1 28 )
(define-primitive boolean? 1 29 )
(define-primitive string? 1 30 )
(define-primitive string->list 1 31 )
(define-primitive list->string 1 32 )
(define-primitive u8vector? 1 33 )
(define-primitive #%make-u8vector 1 34 )
(define-primitive u8vector-ref 2 35 )
(define-primitive u8vector-set! 3 36 #:unspecified-result)
(define-primitive u8vector-length 1 37 )
(define-primitive print 1 38 #:unspecified-result)
(define-primitive clock 0 39 )
(define-primitive #%getchar-wait 2 40 )
(define-primitive #%putchar 2 41 #:unspecified-result)
(define-primitive #%sys 2 42 )
(define-primitive #%gpio 2 43 )
(define-primitive #%net 2 44 )