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
  (define num-allocations 0)
  (define num-frees 0)
  
  (define scratch0 0)
  (define scratch1 0)
  (define scratch2 0)
  
  (define (align-up size alignment)
    (do
      (define mask (- alignment 1))
      (define aligned (+ size mask))
      (define result (- aligned (% aligned alignment)))
      result))
  
  (define (max a b)
    (if (> a b) a b))
  
  (define (mem-write addr value)
    (do
      (set scratch0 addr)
      (set scratch1 value)
      (set scratch0 scratch1)
      0))
  
  (define (mem-read addr)
    (do
      (set scratch0 addr)
      scratch0))
  
  (define (write-block-size block size)
    (do
      (print 8001)
      (print block)
      (print size)
      (mem-write block size)))
  
  (define (write-block-next block next)
    (do
      (print 8002)
      (print block)
      (print next)
      (mem-write (+ block 1) next)))
  
  (define (read-block-size block)
    (mem-read block))
  
  (define (read-block-next block)
    (mem-read (+ block 1)))
  
  (define (init-allocator)
    (do
      (set free-list-head NULL)
      (set heap-pointer HEAP_START)
      (set num-allocations 0)
      (set num-frees 0)
      (print 1000)
      0))
  
  (define (malloc size)
    (do
      (define aligned-size (align-up size ALIGNMENT))
      (define actual-size (max aligned-size MIN_BLOCK_SIZE))
      (define needed (+ actual-size HEADER_SIZE))
      (define block heap-pointer)
      
      (if (> (+ block needed) HEAP_END)
          (do
            (print 8888)
            NULL)
          (do
            (set heap-pointer (+ heap-pointer needed))
            (set num-allocations (+ num-allocations 1))
            
            (print 8000)
            (write-block-size block actual-size)
            (write-block-next block NULL)
            
            (+ block HEADER_SIZE)))))
  
  (define (free ptr)
    (do
      (if (= ptr NULL)
          0
          (do
            (define block (- ptr HEADER_SIZE))
            (define size (read-block-size block))
            
            (print 8003)
            (print block)
            (print size)
            
            (write-block-next block free-list-head)
            (set free-list-head block)
            (set num-frees (+ num-frees 1))
            0))))
  
  (define (print-stats)
    (do
      (print 7777)
      (print num-allocations)
      (print num-frees)
      (print heap-pointer)
      (print free-list-head)
      0))
  
  (define (test-allocator-with-metadata)
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
      (define p3 (malloc 30))
      (print p3)
      
      (print 5004)
      (print-stats)
      
      (print 5005)
      (free p2)
      
      (print 5006)
      (print-stats)
      
      (print 5007)
      (free p1)
      
      (print 5008)
      (print-stats)))
  
  (test-allocator-with-metadata))
