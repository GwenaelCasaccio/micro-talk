(do
  (define HEAP_START 30000)
  (define HEAP_END 50000)
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
  
  (define (align-up size alignment)
    (do
      (define mask (- alignment 1))
      (define aligned (+ size mask))
      (- aligned (% aligned alignment))))
  
  (define (max a b)
    (if (> a b) a b))
  
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
            
            (poke (+ block SIZE_OFFSET) actual-size)
            (poke (+ block NEXT_OFFSET) NULL)
            
            (print 8000)
            (print block)
            (print actual-size)
            
            (+ block HEADER_SIZE)))))
  
  (define (get-block-size block)
    (peek (+ block SIZE_OFFSET)))
  
  (define (get-block-next block)
    (peek (+ block NEXT_OFFSET)))
  
  (define (set-block-next block next)
    (poke (+ block NEXT_OFFSET) next))
  
  (define (free ptr)
    (do
      (if (= ptr NULL)
          0
          (do
            (define block (- ptr HEADER_SIZE))
            (define size (get-block-size block))
            
            (print 8003)
            (print block)
            (print size)
            
            (set-block-next block free-list-head)
            (set free-list-head block)
            (set num-frees (+ num-frees 1))
            0))))
  
  (define (print-free-list)
    (do
      (print 9000)
      (define current free-list-head)
      (while (> current NULL)
        (do
          (print current)
          (print (get-block-size current))
          (set current (get-block-next current))))
      0))
  
  (define (print-stats)
    (do
      (print 7777)
      (print num-allocations)
      (print num-frees)
      (print (- heap-pointer HEAP_START))
      (print (- HEAP_END heap-pointer))
      0))
  
  (define (test-allocator)
    (do
      (print 5000)
      (init-allocator)
      
      (print 5001)
      (define p1 (malloc 10))
      (print p1)
      (print (get-block-size (- p1 2)))
      
      (print 5002)
      (define p2 (malloc 20))
      (print p2)
      (print (get-block-size (- p2 2)))
      
      (print 5003)
      (define p3 (malloc 30))
      (print p3)
      (print (get-block-size (- p3 2)))
      
      (print 5004)
      (print-stats)
      
      (print 5005)
      (free p2)
      (print-free-list)
      
      (print 5006)
      (free p1)
      (print-free-list)
      
      (print 5007)
      (print-stats)))
  
  (test-allocator))
