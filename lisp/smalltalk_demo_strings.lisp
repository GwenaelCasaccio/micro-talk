(do
  (define HEAP_START 30000)
  (define HEAP_END 50000)
  (define NULL 0)
  (define ALIGNMENT 8)
  
  (define heap-pointer HEAP_START)
  
  (define TAG_INT 1)
  (define TAG_OOP 0)
  (define TAG_MASK 7)
  
  (define CLASS_OFFSET 0)
  (define SHAPE_OFFSET 1)
  (define NAMED_SLOTS_OFFSET 2)
  (define OBJECT_HEADER_SIZE 3)
  
  (define SHAPE_FIXED 0)
  (define SHAPE_INDEXABLE 1)
  
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
  
  (define (malloc size)
    (do
      (define aligned-size (align-up size ALIGNMENT))
      (define result heap-pointer)
      (if (> (+ result aligned-size) HEAP_END)
          NULL
          (do
            (set heap-pointer (+ heap-pointer aligned-size))
            result))))
  
  (define (tag-int value)
    (bit-or (bit-shl value 3) TAG_INT))
  
  (define (untag-int tagged)
    (bit-ashr tagged 3))
  
  (define (tag-oop addr)
    (bit-or addr TAG_OOP))
  
  (define (untag-oop tagged)
    (bit-and tagged (bit-xor -1 TAG_MASK)))
  
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
  
  (define (slot-at obj index)
    (peek (+ (untag-oop obj) OBJECT_HEADER_SIZE index)))
  
  (define (slot-at-put obj index value)
    (do
      (poke (+ (untag-oop obj) OBJECT_HEADER_SIZE index) value)
      value))
  
  (define (indexed-at obj index)
    (do
      (define addr (untag-oop obj))
      (define named-count (peek (+ addr NAMED_SLOTS_OFFSET)))
      (peek (+ addr OBJECT_HEADER_SIZE named-count index))))
  
  (define (indexed-at-put obj index value)
    (do
      (define addr (untag-oop obj))
      (define named-count (peek (+ addr NAMED_SLOTS_OFFSET)))
      (poke (+ addr OBJECT_HEADER_SIZE named-count index) value)
      value))
  
  (define (demo)
    (do
      (print-string "=== Smalltalk Object Model Demo ===")
      (print-string "")
      
      (print-string "Creating Object class...")
      (define Object (new-class (tag-int 100) NULL (tag-int 0)))
      (print-int Object)
      
      (print-string "Creating Point class (2 slots: x, y)...")
      (define Point (new-class (tag-int 200) Object (tag-int 2)))
      (print-int Point)
      
      (print-string "Creating Point instance...")
      (define p1 (new-instance Point 2 0 SHAPE_FIXED))
      (print-string "Point address:")
      (print-int p1)
      
      (print-string "Setting x = 10, y = 20...")
      (slot-at-put p1 0 (tag-int 10))
      (slot-at-put p1 1 (tag-int 20))
      
      (print-string "Getting x:")
      (print-int (untag-int (slot-at p1 0)))
      (print-string "Getting y:")
      (print-int (untag-int (slot-at p1 1)))
      
      (print-string "")
      (print-string "Creating Array class...")
      (define Array (new-class (tag-int 300) Object (tag-int 0)))
      
      (print-string "Creating Array[5]...")
      (define arr (new-instance Array 0 5 SHAPE_INDEXABLE))
      (print-int arr)
      
      (print-string "Filling array with values...")
      (indexed-at-put arr 0 (tag-int 100))
      (indexed-at-put arr 1 (tag-int 200))
      (indexed-at-put arr 2 (tag-int 300))
      
      (print-string "Array[0]:")
      (print-int (untag-int (indexed-at arr 0)))
      (print-string "Array[1]:")
      (print-int (untag-int (indexed-at arr 1)))
      (print-string "Array[2]:")
      (print-int (untag-int (indexed-at arr 2)))
      
      (print-string "")
      (print-string "=== Demo Complete! ===")
      0))
  
  (demo))
