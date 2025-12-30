(do
  (define free-list-head 16384)
  (define heap-start 16384)
  (define heap-size 49152)
  
  (define (allocate n)
    (do
      (define needed (+ n 1))
      (define result free-list-head)
      (set free-list-head (+ free-list-head needed))
      (if (> free-list-head 65536)
          (do
            (set free-list-head (- free-list-head needed))
            0)
          result)))
  
  (define obj1 (allocate 10))
  (print obj1)
  
  (define obj2 (allocate 20))
  (print obj2)
  
  (define obj3 (allocate 5))
  (print obj3)
  
  obj3)
