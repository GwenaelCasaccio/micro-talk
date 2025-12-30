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
  
  (define TAG_INT 1)
  (define TAG_OOP 0)
  (define TAG_MASK 7)
  
  (define CLASS_OFFSET 0)
  (define SHAPE_OFFSET 1)
  (define NAMED_SLOTS_OFFSET 2)
  (define OBJECT_HEADER_SIZE 3)
  
  (define SHAPE_FIXED 0)
  (define SHAPE_INDEXABLE 1)
  (define SHAPE_BYTES 2)
  
  (define CLASS_NAME_OFFSET 0)
  (define CLASS_SUPERCLASS_OFFSET 1)
  (define CLASS_SLOT_COUNT_OFFSET 2)
  (define CLASS_HEADER_SIZE 3)
  
  (define (align-up size alignment)
    (do
      (define mask (- alignment 1))
      (define aligned (+ size mask))
      (- aligned (% aligned alignment))))
  
  (define (max a b)
    (if (> a b) a b))
  
  (define (tag-int value)
    (bit-or (bit-shl value 3) TAG_INT))
  
  (define (untag-int tagged)
    (bit-ashr tagged 3))
  
  (define (is-int obj)
    (= (bit-and obj TAG_MASK) TAG_INT))
  
  (define (tag-oop addr)
    (bit-or addr TAG_OOP))
  
  (define (untag-oop tagged)
    (bit-and tagged (bit-xor -1 TAG_MASK)))
  
  (define (is-oop obj)
    (= (bit-and obj TAG_MASK) TAG_OOP))
  
  (define (init-allocator)
    (do
      (set free-list-head NULL)
      (set heap-pointer HEAP_START)
      (set num-allocations 0)
      0))
  
  (define (malloc size)
    (do
      (define aligned-size (align-up size ALIGNMENT))
      (define actual-size (max aligned-size MIN_BLOCK_SIZE))
      (define needed (+ actual-size HEADER_SIZE))
      (define block heap-pointer)
      
      (if (> (+ block needed) HEAP_END)
          NULL
          (do
            (set heap-pointer (+ heap-pointer needed))
            (set num-allocations (+ num-allocations 1))
            (poke (+ block SIZE_OFFSET) actual-size)
            (poke (+ block NEXT_OFFSET) NULL)
            (+ block HEADER_SIZE)))))
  
  (define (new-class name superclass slot-count)
    (do
      (define obj (malloc CLASS_HEADER_SIZE))
      (if (= obj NULL)
          NULL
          (do
            (poke (+ obj CLASS_NAME_OFFSET) name)
            (poke (+ obj CLASS_SUPERCLASS_OFFSET) superclass)
            (poke (+ obj CLASS_SLOT_COUNT_OFFSET) slot-count)
            (tag-oop obj)))))
  
  (define (get-class-name class)
    (peek (+ (untag-oop class) CLASS_NAME_OFFSET)))
  
  (define (get-class-superclass class)
    (peek (+ (untag-oop class) CLASS_SUPERCLASS_OFFSET)))
  
  (define (get-class-slot-count class)
    (peek (+ (untag-oop class) CLASS_SLOT_COUNT_OFFSET)))
  
  (define (new-instance class named-slots indexed-slots shape)
    (do
      (define total-slots (+ OBJECT_HEADER_SIZE named-slots indexed-slots))
      (define obj (malloc total-slots))
      
      (if (= obj NULL)
          NULL
          (do
            (poke (+ obj CLASS_OFFSET) class)
            (poke (+ obj SHAPE_OFFSET) shape)
            (poke (+ obj NAMED_SLOTS_OFFSET) named-slots)
            
            (for (i 0 (+ named-slots indexed-slots))
              (poke (+ obj OBJECT_HEADER_SIZE i) NULL))
            
            (tag-oop obj)))))
  
  (define (get-class obj)
    (peek (+ (untag-oop obj) CLASS_OFFSET)))
  
  (define (get-shape obj)
    (peek (+ (untag-oop obj) SHAPE_OFFSET)))
  
  (define (get-named-slot-count obj)
    (peek (+ (untag-oop obj) NAMED_SLOTS_OFFSET)))
  
  (define (slot-at obj index)
    (do
      (define addr (untag-oop obj))
      (peek (+ addr OBJECT_HEADER_SIZE index))))
  
  (define (slot-at-put obj index value)
    (do
      (define addr (untag-oop obj))
      (poke (+ addr OBJECT_HEADER_SIZE index) value)
      value))
  
  (define (indexed-at obj index)
    (do
      (define addr (untag-oop obj))
      (define named-count (get-named-slot-count obj))
      (peek (+ addr OBJECT_HEADER_SIZE named-count index))))
  
  (define (indexed-at-put obj index value)
    (do
      (define addr (untag-oop obj))
      (define named-count (get-named-slot-count obj))
      (poke (+ addr OBJECT_HEADER_SIZE named-count index) value)
      value))
  
  (define (print-object obj)
    (do
      (print 8000)
      (print obj)
      (print (get-class obj))
      (print (get-shape obj))
      (print (get-named-slot-count obj))
      0))
  
  (define (test-smalltalk-objects)
    (do
      (print 5000)
      (init-allocator)
      
      (print 5001)
      (define Object (new-class (tag-int 100) NULL (tag-int 0)))
      (print Object)
      
      (print 5002)
      (define Point (new-class (tag-int 200) Object (tag-int 2)))
      (print Point)
      (print (get-class-name Point))
      (print (untag-int (get-class-slot-count Point)))
      
      (print 5003)
      (define p1 (new-instance Point 2 0 SHAPE_FIXED))
      (print p1)
      (print-object p1)
      
      (print 5004)
      (slot-at-put p1 0 (tag-int 10))
      (slot-at-put p1 1 (tag-int 20))
      
      (print 5005)
      (define x (slot-at p1 0))
      (define y (slot-at p1 1))
      (print x)
      (print (untag-int x))
      (print y)
      (print (untag-int y))
      
      (print 5006)
      (define Array (new-class (tag-int 300) Object (tag-int 0)))
      (define arr (new-instance Array 0 5 SHAPE_INDEXABLE))
      (print arr)
      (print-object arr)
      
      (print 5007)
      (indexed-at-put arr 0 (tag-int 100))
      (indexed-at-put arr 1 (tag-int 200))
      (indexed-at-put arr 2 (tag-int 300))
      
      (print 5008)
      (print (untag-int (indexed-at arr 0)))
      (print (untag-int (indexed-at arr 1)))
      (print (untag-int (indexed-at arr 2)))
      
      (print 5009)
      (define Person (new-class (tag-int 400) Object (tag-int 2)))
      (define alice (new-instance Person 2 0 SHAPE_FIXED))
      (slot-at-put alice 0 (tag-int 1000))
      (slot-at-put alice 1 (tag-int 25))
      
      (print 5010)
      (print (untag-int (slot-at alice 0)))
      (print (untag-int (slot-at alice 1)))
      
      (print 9999)
      0))
  
  (test-smalltalk-objects))
