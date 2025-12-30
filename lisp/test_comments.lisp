; This is a test file for comment support
; Author: Claude Code
; Date: 2025-12-30

; Define some variables
(define x 10)  ; x coordinate
(define y 20)  ; y coordinate

; Calculate the sum
; This should give us 30
(define sum (+ x y))

; Print the result
(print sum)

; Now let's try some arithmetic
; with multiple comments throughout

(define result
  (do
    ; First, multiply x by 2
    (define doubled (* x 2))

    ; Then add y
    (+ doubled y)))

; Display the final result
(print result)

; Test nested expressions with comments
(define complex
  (+
    ; First term
    (* 3 3)
    ; Second term
    (* 4 4)
    ; Third term
    (* 5 5)))

(print complex)

; All done!
; Expected outputs: 30, 40, 50
