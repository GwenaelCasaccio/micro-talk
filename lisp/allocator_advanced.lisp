(do
  (define-var HEAP_START 20000)
  (define-var HEAP_END 40000)
  (define-var NULL 0)
  (define-var ALIGNMENT 8)
  (define-var MIN_BLOCK_SIZE 8)
  
  (define-var SIZE_OFFSET 0)
  (define-var NEXT_OFFSET 1)
  (define-var HEADER_SIZE 2)
  
  (define-var free-list-head NULL)
  (define-var heap-pointer HEAP_START)
  (define-var total-allocated 0)
  (define-var total-freed 0)
  (define-var num-allocations 0)
  (define-var num-frees 0)
  (define-var bytes-in-use 0)
  
  (define-func (align-up size alignment)
    (do
      (define-var mask (- alignment 1))
      (define-var aligned (+ size mask))
      (define-var result (- aligned (% aligned alignment)))
      result))
  
  (define-func (max a b)
    (if (> a b) a b))
  
  (define-func (init-allocator)
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
  
  (define-func (malloc size)
    (do
      (define-var aligned-size (align-up size ALIGNMENT))
      (define-var actual-size (max aligned-size MIN_BLOCK_SIZE))
      (define-var needed (+ actual-size HEADER_SIZE))
      (define-var result heap-pointer)
      
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
  
  (define-func (free ptr)
    (do
      (if (= ptr NULL)
          0
          (do
            (define-var block (- ptr HEADER_SIZE))
            (set num-frees (+ num-frees 1))
            0))))
  
  (define-func (get-used-memory)
    bytes-in-use)
  
  (define-func (get-free-memory)
    (- (- HEAP_END HEAP_START) bytes-in-use))
  
  (define-func (get-heap-size)
    (- HEAP_END HEAP_START))
  
  (define-func (get-fragmentation)
    (do
      (define-var total (get-heap-size))
      (define-var used bytes-in-use)
      (define-var wasted (- (- heap-pointer HEAP_START) used))
      (if (= used 0)
          0
          (/ (* wasted 100) used))))
  
  (define-func (print-stats)
    (do
      (print 7777)
      (print num-allocations)
      (print num-frees)
      (print total-allocated)
      (print bytes-in-use)
      (print (get-free-memory))
      (print (get-fragmentation))
      0))
  
  (define-func (test-alignment size)
    (do
      (define-var ptr (malloc size))
      (print 6000)
      (print size)
      (print ptr)
      (define-var offset (% (- ptr HEAP_START) ALIGNMENT))
      (print offset)
      ptr))
  
  (define-func (test-aligned-allocator)
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
      (define-var p1 (malloc 100))
      (define-var p2 (malloc 200))
      (define-var p3 (malloc 300))
      
      (print 5004)
      (print-stats)
      
      (print 5005)
      (free p2)
      
      (print 5006)
      (print-stats)
      
      (print 5007)
      (define-var p4 (malloc 5000))
      (print p4)
      
      (print 5008)
      (print-stats)))
  
  (test-aligned-allocator))
