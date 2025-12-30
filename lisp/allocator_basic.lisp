(do
  (define HEAP_START 16384)
  (define HEAP_END 65536)
  (define HEAP_SIZE 49152)
  
  (define HEADER_SIZE 2)
  
  (define free-list-head HEAP_START)
  (define alloc-count 0)
  (define free-count 0)
  
  (define (allocate size)
    (do
      (define needed (+ size HEADER_SIZE))
      (define addr free-list-head)
      (set free-list-head (+ free-list-head needed))
      
      (if (> free-list-head HEAP_END)
          (do
            (set free-list-head (- free-list-head needed))
            (print 9999)
            0)
          (do
            (set alloc-count (+ alloc-count 1))
            addr))))
  
  (define (get-stats)
    (do
      (print 1111)
      (print alloc-count)
      (print free-count)
      (print free-list-head)
      (- free-list-head HEAP_START)))
  
  (print 2000)
  (define obj1 (allocate 8))
  (print obj1)
  
  (print 2001)
  (define obj2 (allocate 16))
  (print obj2)
  
  (print 2002)
  (define obj3 (allocate 32))
  (print obj3)
  
  (print 2003)
  (define obj4 (allocate 64))
  (print obj4)
  
  (print 2004)
  (get-stats))
