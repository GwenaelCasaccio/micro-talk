(do
  (define HEAP_START 20000)
  (define HEAP_END 40000)
  (define NULL 0)
  (define ALIGNMENT 8)
  (define MIN_BLOCK_SIZE 8)
  
  (define SIZE_OFFSET 0)
  (define NEXT_OFFSET 1)
  (define HEADER_SIZE 2)
  
  (define free-list-head NULL)
  (define heap-pointer HEAP_START)
  (define total-allocated 0)
  (define total-freed 0)
  (define num-allocations 0)
  (define num-frees 0)
  (define bytes-in-use 0)
  
  (define (align-up size alignment)
    (do
      (define mask (- alignment 1))
      (define aligned (+ size mask))
      (define result (- aligned (% aligned alignment)))
      result))
  
  (define (max a b)
    (if (> a b) a b))
  
  (define (init-allocator)
    (do
      (set free-list-head NULL)
      (set heap-pointer HEAP_START)
      (set total-allocated 0)
      (set total-freed 0)
      (set num-allocations 0)
      (set num-frees 0)
      (set bytes-in-use 0)
      (print 1000)
      0))
  
  (define (malloc size)
    (do
      (define aligned-size (align-up size ALIGNMENT))
      (define actual-size (max aligned-size MIN_BLOCK_SIZE))
      (define needed (+ actual-size HEADER_SIZE))
      (define result heap-pointer)
      
      (if (> (+ result needed) HEAP_END)
          (do
            (print 8888)
            NULL)
          (do
            (set heap-pointer (+ heap-pointer needed))
            (set total-allocated (+ total-allocated needed))
            (set bytes-in-use (+ bytes-in-use needed))
            (set num-allocations (+ num-allocations 1))
            (+ result HEADER_SIZE)))))
  
  (define (free ptr)
    (do
      (if (= ptr NULL)
          0
          (do
            (define block (- ptr HEADER_SIZE))
            (set num-frees (+ num-frees 1))
            0))))
  
  (define (get-used-memory)
    bytes-in-use)
  
  (define (get-free-memory)
    (- (- HEAP_END HEAP_START) bytes-in-use))
  
  (define (get-heap-size)
    (- HEAP_END HEAP_START))
  
  (define (get-fragmentation)
    (do
      (define total (get-heap-size))
      (define used bytes-in-use)
      (define wasted (- (- heap-pointer HEAP_START) used))
      (if (= used 0)
          0
          (/ (* wasted 100) used))))
  
  (define (print-stats)
    (do
      (print 7777)
      (print num-allocations)
      (print num-frees)
      (print total-allocated)
      (print bytes-in-use)
      (print (get-free-memory))
      (print (get-fragmentation))
      0))
  
  (define (test-alignment size)
    (do
      (define ptr (malloc size))
      (print 6000)
      (print size)
      (print ptr)
      (define offset (% (- ptr HEAP_START) ALIGNMENT))
      (print offset)
      ptr))
  
  (define (test-aligned-allocator)
    (do
      (print 5000)
      (init-allocator)
      
      (print 5001)
      (test-alignment 1)
      (test-alignment 7)
      (test-alignment 8)
      (test-alignment 9)
      (test-alignment 15)
      (test-alignment 16)
      (test-alignment 17)
      (test-alignment 23)
      (test-alignment 24)
      (test-alignment 25)
      
      (print 5002)
      (print-stats)
      
      (print 5003)
      (define p1 (malloc 100))
      (define p2 (malloc 200))
      (define p3 (malloc 300))
      
      (print 5004)
      (print-stats)
      
      (print 5005)
      (free p2)
      
      (print 5006)
      (print-stats)
      
      (print 5007)
      (define p4 (malloc 5000))
      (print p4)
      
      (print 5008)
      (print-stats)))
  
  (test-aligned-allocator))
