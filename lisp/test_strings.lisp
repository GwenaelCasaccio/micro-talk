(do
  (define HEAP_START 30000)
  (define HEAP_END 50000)
  (define NULL 0)
  (define ALIGNMENT 8)
  
  (define heap-pointer HEAP_START)
  
  (define (align-up size alignment)
    (do
      (define mask (- alignment 1))
      (define aligned (+ size mask))
      (- aligned (% aligned alignment))))
  
  (define (malloc size)
    (do
      (define aligned-size (align-up size ALIGNMENT))
      (define result heap-pointer)
      (if (> (+ result aligned-size) HEAP_END)
          NULL
          (do
            (set heap-pointer (+ heap-pointer aligned-size))
            result))))
  
  (define (make-string-from-chars len c0 c1 c2 c3 c4 c5 c6 c7)
    (do
      (define words (+ 1 (/ (+ len 7) 8)))
      (define str (malloc words))
      (poke str len)
      (define word0 (+ c0
                       (bit-shl c1 8)
                       (bit-shl c2 16)
                       (bit-shl c3 24)
                       (bit-shl c4 32)
                       (bit-shl c5 40)
                       (bit-shl c6 48)
                       (bit-shl c7 56)))
      (poke (+ str 1) word0)
      str))
  
  (define (make-string-16 len 
                          c0 c1 c2 c3 c4 c5 c6 c7
                          c8 c9 c10 c11 c12 c13 c14 c15)
    (do
      (define words (+ 1 (/ (+ len 7) 8)))
      (define str (malloc words))
      (poke str len)
      (define word0 (+ c0
                       (bit-shl c1 8)
                       (bit-shl c2 16)
                       (bit-shl c3 24)
                       (bit-shl c4 32)
                       (bit-shl c5 40)
                       (bit-shl c6 48)
                       (bit-shl c7 56)))
      (define word1 (+ c8
                       (bit-shl c9 8)
                       (bit-shl c10 16)
                       (bit-shl c11 24)
                       (bit-shl c12 32)
                       (bit-shl c13 40)
                       (bit-shl c14 48)
                       (bit-shl c15 56)))
      (poke (+ str 1) word0)
      (poke (+ str 2) word1)
      str))
  
  (define (test-strings)
    (do
      (print 5000)
      
      (print 5001)
      (define hello (make-string-from-chars 5 72 101 108 108 111 0 0 0))
      (print-int hello)
      (print-string hello)
      
      (print 5002)
      (define world (make-string-from-chars 5 87 111 114 108 100 0 0 0))
      (print-int world)
      (print-string world)
      
      (print 5003)
      (define point (make-string-from-chars 7 80 111 105 110 116 40 41 0))
      (print-int point)
      (print-string point)
      
      (print 5004)
      (define smalltalk (make-string-from-chars 8 83 109 97 108 108 116 97 107))
      (print-int smalltalk)
      (print-string smalltalk)
      
      (print 5005)
      (define longer (make-string-16 13
                                     72 101 108 108 111 32 87 111
                                     114 108 100 33 33 0 0 0))
      (print-int longer)
      (print-string longer)
      
      (print 9999)
      0))
  
  (test-strings))
