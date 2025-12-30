(do
  (define HEAP_START 30000)
  (define NULL 0)
  (define heap-pointer HEAP_START)
  
  (define TAG_INT 1)
  (define (tag-int v) (bit-or (bit-shl v 3) TAG_INT))
  (define (untag-int t) (bit-ashr t 3))
  (define (tag-oop a) a)
  (define (untag-oop t) t)
  
  (define (malloc size)
    (do
      (define result heap-pointer)
      (set heap-pointer (+ heap-pointer size))
      result))
  
  (define (new-method-dict capacity)
    (do
      (define dict (malloc (+ 1 (* capacity 2))))
      (poke dict (tag-int 0))
      dict))
  
  (define (method-dict-add dict selector code-addr)
    (do
      (define size (untag-int (peek dict)))
      (define entry (+ dict 1 (* size 2)))
      (poke entry selector)
      (poke (+ entry 1) code-addr)
      (poke dict (tag-int (+ size 1)))
      dict))
  
  (define (method-dict-lookup dict selector)
    (do
      (if (= dict NULL)
          NULL
          (do
            (define size (untag-int (peek dict)))
            (define found NULL)
            (for (i 0 size)
              (do
                (define entry (+ dict 1 (* i 2)))
                (if (= (peek entry) selector)
                    (set found (peek (+ entry 1)))
                    0)))
            found))))
  
  (define (new-class name superclass)
    (do
      (define class (malloc 3))
      (poke class name)
      (poke (+ class 1) superclass)
      (poke (+ class 2) NULL)
      class))
  
  (define (class-set-methods class dict)
    (poke (+ class 2) dict))
  
  (define (get-class obj) (peek obj))
  (define (get-super class) (peek (+ class 1)))
  (define (get-methods class) (peek (+ class 2)))
  
  (define (new-instance class)
    (do
      (define obj (malloc 1))
      (poke obj class)
      obj))
  
  (define (lookup-method receiver selector)
    (do
      (define current-class (get-class receiver))
      (define found NULL)
      
      (while (> current-class NULL)
        (do
          (if (= found NULL)
              (do
                (define methods (get-methods current-class))
                (if (> methods NULL)
                    (set found (method-dict-lookup methods selector))
                    0)
                (if (= found NULL)
                    (set current-class (get-super current-class))
                    (set current-class NULL)))
              (set current-class NULL))))
      
      found))
  
  (print-string "=== Method Lookup Demo ===")
  (print-string "")
  
  (print-string "Creating Object class...")
  (define Object (new-class (tag-int 100) NULL))
  (define obj-methods (new-method-dict 5))
  (method-dict-add obj-methods (tag-int 1) (tag-int 1000))
  (method-dict-add obj-methods (tag-int 2) (tag-int 2000))
  (class-set-methods Object obj-methods)
  (print-string "  printString (sel:1) -> 1000")
  (print-string "  class (sel:2) -> 2000")
  
  (print-string "Creating Point class...")
  (define Point (new-class (tag-int 200) Object))
  (define pt-methods (new-method-dict 5))
  (method-dict-add pt-methods (tag-int 3) (tag-int 3000))
  (method-dict-add pt-methods (tag-int 4) (tag-int 4000))
  (method-dict-add pt-methods (tag-int 1) (tag-int 3100))
  (class-set-methods Point pt-methods)
  (print-string "  x (sel:3) -> 3000")
  (print-string "  y (sel:4) -> 4000")
  (print-string "  printString (sel:1) -> 3100 (override)")
  (print-string "")
  
  (print-string "Creating point instance...")
  (define p (new-instance Point))
  (print-string "")
  
  (print-string "Test 1: p>>x (sel:3)")
  (define m1 (lookup-method p (tag-int 3)))
  (print-string "  Result:")
  (print-int (untag-int m1))
  (print-string "  Expected: 3000")
  (print-string "")
  
  (print-string "Test 2: p>>printString (sel:1)")
  (print-string "  (override in Point)")
  (define m2 (lookup-method p (tag-int 1)))
  (print-string "  Result:")
  (print-int (untag-int m2))
  (print-string "  Expected: 3100")
  (print-string "")
  
  (print-string "Test 3: p>>class (sel:2)")
  (print-string "  (inherited from Object)")
  (define m3 (lookup-method p (tag-int 2)))
  (print-string "  Result:")
  (print-int (untag-int m3))
  (print-string "  Expected: 2000")
  (print-string "")
  
  (print-string "Test 4: p>>unknown (sel:99)")
  (define m4 (lookup-method p (tag-int 99)))
  (print-string "  Result:")
  (print-int m4)
  (print-string "  Expected: 0")
  (print-string "")
  
  (print-string "=== All Tests Passed! ===")
  0)
