(do
  (define HEAP_START 20000)
  (define HEAP_END 30000)
  (define NULL 0)
  (define ALIGNMENT 1)
  
  (define SIZE_OFFSET 0)
  (define NEXT_OFFSET 1)
  (define HEADER_SIZE 2)
  (define FREE_MAGIC 0)
  (define USED_MAGIC 1)
  
  (define free-list-head NULL)
  (define heap-pointer HEAP_START)
  (define total-allocated 0)
  (define total-freed 0)
  (define num-allocations 0)
  (define num-frees 0)
  
  (define (align-size size)
    (do
      (define remainder (% size ALIGNMENT))
      (if (= remainder 0)
          size
          (+ size (- ALIGNMENT remainder)))))
  
  (define (init-allocator)
    (do
      (set free-list-head NULL)
      (set heap-pointer HEAP_START)
      (set total-allocated 0)
      (set total-freed 0)
      (set num-allocations 0)
      (set num-frees 0)
      (print 1000)
      0))
  
  (define (malloc size)
    (do
      (define aligned-size (align-size size))
      (define needed (+ aligned-size HEADER_SIZE))
      (define result heap-pointer)
      
      (if (> (+ result needed) HEAP_END)
          (do
            (print 9999)
            NULL)
          (do
            (set heap-pointer (+ heap-pointer needed))
            (set total-allocated (+ total-allocated needed))
            (set num-allocations (+ num-allocations 1))
            (+ result HEADER_SIZE)))))
  
  (define (free ptr)
    (do
      (if (= ptr NULL)
          0
          (do
            (set num-frees (+ num-frees 1))
            (set free-list-head ptr)
            0))))
  
  (define (get-used-memory)
    (- heap-pointer HEAP_START))
  
  (define (get-free-memory)
    (- HEAP_END heap-pointer))
  
  (define (get-total-freed)
    total-freed)
  
  (define (print-stats)
    (do
      (print 7777)
      (print num-allocations)
      (print num-frees)
      (print total-allocated)
      (print (get-used-memory))
      (print (get-free-memory))
      0))
  
  (define (test-aligned-allocator)
    (do
      (print 5000)
      (init-allocator)
      
      (print 5001)
      (define p1 (malloc 7))
      (print p1)
      
      (print 5002)
      (define p2 (malloc 13))
      (print p2)
      
      (print 5003)
      (define p3 (malloc 25))
      (print p3)
      
      (print 5004)
      (define p4 (malloc 99))
      (print p4)
      
      (print 5005)
      (print-stats)
      
      (print 5006)
      (free p2)
      (print 5007)
      (free p4)
      
      (print 5008)
      (print-stats)
      
      (print 5009)
      (define p5 (malloc 1000))
      (print p5)
      
      (print 5010)
      (print-stats)))
  
  (test-aligned-allocator))
