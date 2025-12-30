(do
  (define HEAP_START 20000)
  (define HEAP_END 30000)
  (define NULL 0)
  
  (define SIZE_OFFSET 0)
  (define NEXT_OFFSET 1)
  (define HEADER_SIZE 2)
  
  (define free-list-head HEAP_START)
  (define total-allocated 0)
  (define total-freed 0)
  (define num-allocations 0)
  
  (define (init-allocator)
    (do
      (define initial-size (- HEAP_END HEAP_START))
      (set free-list-head HEAP_START)
      0))
  
  (define (malloc size)
    (do
      (define needed (+ size HEADER_SIZE))
      (define result free-list-head)
      
      (if (> (+ result needed) HEAP_END)
          (do
            (print 8888)
            NULL)
          (do
            (set free-list-head (+ free-list-head needed))
            (set total-allocated (+ total-allocated needed))
            (set num-allocations (+ num-allocations 1))
            (+ result HEADER_SIZE)))))
  
  (define (get-used-memory)
    (- free-list-head HEAP_START))
  
  (define (get-free-memory)
    (- HEAP_END free-list-head))
  
  (define (print-stats)
    (do
      (print 7777)
      (print num-allocations)
      (print total-allocated)
      (print (get-used-memory))
      (print (get-free-memory))
      0))
  
  (define (test-allocator)
    (do
      (print 5000)
      (init-allocator)
      
      (print 5001)
      (define p1 (malloc 10))
      (print p1)
      
      (print 5002)
      (define p2 (malloc 20))
      (print p2)
      
      (print 5003)
      (define p3 (malloc 50))
      (print p3)
      
      (print 5004)
      (define p4 (malloc 100))
      (print p4)
      
      (print 5005)
      (print-stats)
      
      (print 5006)
      (define p5 (malloc 5000))
      (print p5)
      
      (print 5007)
      (print-stats)))
  
  (test-allocator))
