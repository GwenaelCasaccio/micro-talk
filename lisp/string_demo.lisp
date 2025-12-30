(do
  (define HEAP_START 30000)
  (define heap-pointer HEAP_START)
  
  (define (malloc size)
    (do
      (define result heap-pointer)
      (set heap-pointer (+ heap-pointer size))
      result))
  
  (define (str s0 s1 s2 s3 s4 s5 s6 s7)
    (do
      (define s (malloc 2))
      (poke s 8)
      (poke (+ s 1) (+ s0
                       (bit-shl s1 8)
                       (bit-shl s2 16)
                       (bit-shl s3 24)
                       (bit-shl s4 32)
                       (bit-shl s5 40)
                       (bit-shl s6 48)
                       (bit-shl s7 56)))
      s))
  
  (define (str16 len s0 s1 s2 s3 s4 s5 s6 s7 s8 s9 s10 s11 s12 s13 s14 s15)
    (do
      (define s (malloc 3))
      (poke s len)
      (poke (+ s 1) (+ s0
                       (bit-shl s1 8)
                       (bit-shl s2 16)
                       (bit-shl s3 24)
                       (bit-shl s4 32)
                       (bit-shl s5 40)
                       (bit-shl s6 48)
                       (bit-shl s7 56)))
      (poke (+ s 2) (+ s8
                       (bit-shl s9 8)
                       (bit-shl s10 16)
                       (bit-shl s11 24)
                       (bit-shl s12 32)
                       (bit-shl s13 40)
                       (bit-shl s14 48)
                       (bit-shl s15 56)))
      s))
  
  (define (demo)
    (do
      (print-string (str 72 101 108 108 111 33 0 0))
      
      (print-string (str 87 111 114 108 100 33 0 0))
      
      (print-string (str 80 111 105 110 116 40 41 0))
      
      (print-string (str 79 98 106 101 99 116 0 0))
      
      (print-string (str 65 114 114 97 121 91 93 0))
      
      (print-string (str16 15 83 109 97 108 108 116 97 108
                              107 32 79 98 106 101 99 116))
      
      (print-string (str 67 108 97 115 115 58 32 0))
      
      (print-string (str 83 108 111 116 91 48 93 0))
      
      0))
  
  (demo))
