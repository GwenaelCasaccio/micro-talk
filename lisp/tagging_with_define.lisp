(do
  (define TAG_MASK 7)
  (define TAG_INT 1)
  (define TAG_OOP 0)
  (define TAG_SPECIAL 7)
  
  (define NIL 7)
  (define TRUE 15)
  (define FALSE 23)
  
  (define tag-int
    (do
      (define value 42)
      (bit-or (bit-shl value 3) TAG_INT)))
  
  (define untag-int
    (do
      (define tagged 337)
      (bit-ashr tagged 3)))
  
  (define tag-oop
    (do
      (define addr 16384)
      (bit-or addr TAG_OOP)))
  
  (define untag-oop
    (do
      (define tagged 16384)
      (bit-and tagged (bit-xor -1 TAG_MASK))))
  
  (define is-int
    (do
      (define tagged 337)
      (= (bit-and tagged TAG_MASK) TAG_INT)))
  
  (define is-oop
    (do
      (define tagged 16384)
      (= (bit-and tagged TAG_MASK) TAG_OOP)))
  
  (define test-all
    (do
      (print tag-int)
      (print untag-int)
      (print tag-oop)
      (print untag-oop)
      (print is-int)
      (print is-oop)
      999))
  
  test-all)
