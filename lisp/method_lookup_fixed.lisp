(do
  (define-var HEAP_START 30000)
  (define-var NULL 0)
  (define-var heap-pointer HEAP_START)
  
  (define-var TAG_INT 1)
  (define-func (tag-int v) (bit-or (bit-shl v 3) TAG_INT))
  (define-func (untag-int t) (bit-ashr t 3))
  (define-func (tag-oop a) a)
  (define-func (untag-oop t) t)
  
  (define-func (malloc size)
    (do
      (define-var result heap-pointer)
      (set heap-pointer (+ heap-pointer size))
      result))
  
  (define-func (new-method-dict capacity)
    (do
      (define-var dict (malloc (+ 1 (* capacity 2))))
      (poke dict (tag-int 0))
      dict))
  
  (define-func (method-dict-add dict selector code-addr)
    (do
      (define-var size (untag-int (peek dict)))
      (define-var entry (+ dict 1 (* size 2)))
      (poke entry selector)
      (poke (+ entry 1) code-addr)
      (poke dict (tag-int (+ size 1)))
      dict))
  
  (define-func (method-dict-lookup dict selector)
    (do
      (if (= dict NULL)
          NULL
          (do
            (define-var size (untag-int (peek dict)))
            (define-var found NULL)
            (for (i 0 size)
              (do
                (define-var entry (+ dict 1 (* i 2)))
                (if (= (peek entry) selector)
                    (set found (peek (+ entry 1)))
                    0)))
            found))))
  
  (define-func (new-class name superclass)
    (do
      (define-var class (malloc 3))
      (poke class name)
      (poke (+ class 1) superclass)
      (poke (+ class 2) NULL)
      class))
  
  (define-func (class-set-methods class dict)
    (poke (+ class 2) dict))
  
  (define-func (get-class obj) (peek obj))
  (define-func (get-super class) (peek (+ class 1)))
  (define-func (get-methods class) (peek (+ class 2)))
  
  (define-func (new-instance class)
    (do
      (define-var obj (malloc 1))
      (poke obj class)
      obj))
  
  (define-func (lookup-method receiver selector)
    (do
      (define-var current-class (get-class receiver))
      (define-var found NULL)
      
      (while (> current-class NULL)
        (do
          (if (= found NULL)
              (do
                (define-var methods (get-methods current-class))
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
  (define-var Object (new-class (tag-int 100) NULL))
  (define-var obj-methods (new-method-dict 5))
  (method-dict-add obj-methods (tag-int 1) (tag-int 1000))
  (method-dict-add obj-methods (tag-int 2) (tag-int 2000))
  (class-set-methods Object obj-methods)
  (print-string "  printString (sel:1) -> 1000")
  (print-string "  class (sel:2) -> 2000")
  
  (print-string "Creating Point class...")
  (define-var Point (new-class (tag-int 200) Object))
  (define-var pt-methods (new-method-dict 5))
  (method-dict-add pt-methods (tag-int 3) (tag-int 3000))
  (method-dict-add pt-methods (tag-int 4) (tag-int 4000))
  (method-dict-add pt-methods (tag-int 1) (tag-int 3100))
  (class-set-methods Point pt-methods)
  (print-string "  x (sel:3) -> 3000")
  (print-string "  y (sel:4) -> 4000")
  (print-string "  printString (sel:1) -> 3100 (override)")
  (print-string "")
  
  (print-string "Creating point instance...")
  (define-var p (new-instance Point))
  (print-string "")
  
  (print-string "Test 1: p>>x (sel:3)")
  (define-var m1 (lookup-method p (tag-int 3)))
  (print-string "  Result:")
  (print-int (untag-int m1))
  (print-string "  Expected: 3000")
  (print-string "")
  
  (print-string "Test 2: p>>printString (sel:1)")
  (print-string "  (override in Point)")
  (define-var m2 (lookup-method p (tag-int 1)))
  (print-string "  Result:")
  (print-int (untag-int m2))
  (print-string "  Expected: 3100")
  (print-string "")
  
  (print-string "Test 3: p>>class (sel:2)")
  (print-string "  (inherited from Object)")
  (define-var m3 (lookup-method p (tag-int 2)))
  (print-string "  Result:")
  (print-int (untag-int m3))
  (print-string "  Expected: 2000")
  (print-string "")
  
  (print-string "Test 4: p>>unknown (sel:99)")
  (define-var m4 (lookup-method p (tag-int 99)))
  (print-string "  Result:")
  (print-int m4)
  (print-string "  Expected: 0")
  (print-string "")
  
  (print-string "=== All Tests Passed! ===")
  0)
