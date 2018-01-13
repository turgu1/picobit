#!/usr/bin/env racket
#lang racket

; Tool to compile and produce binary for ESP32 inclusion
; The binary is put in file main/program.bin

(define (usage)
  (printf "usage: pb path/to/file.scm\n")
  (exit 1))

(if (= (vector-length (current-command-line-arguments)) 1)
    (let ([file (vector-ref (current-command-line-arguments) 0)])
      (void (and (system* "picobit" (path-replace-suffix file ".scm"))
                 (system* "h2b" (path-replace-suffix file ".hex") "main/program.bin"))))
    (usage))
