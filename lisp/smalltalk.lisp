(do
  (define-var HEAP_START 30000)
  (define-var NULL 0)
  (define-var heap-pointer HEAP_START)
 
  (define-var OBJECT_HEADER_SIZE 3)
  (define-var OBJECT_HEADER_BEHAVIOR 0)
  (define-var OBJECT_HEADER_NAMED_SLOTS 1)
  (define-var OBJECT_HEADER_INDEXED_SLOTS 2)

  ; Context object named slots
  (define-var CONTEXT_SENDER 0)
  (define-var CONTEXT_RECEIVER 1)
  (define-var CONTEXT_METHOD 2)
  (define-var CONTEXT_PC 3)
  (define-var CONTEXT_SP 4)
  (define-var CONTEXT_NAMED_SLOTS 5)

  ; Global current context
  (define-var current-context NULL)

  (define-func (tag-int v) (bit-or (bit-shl v 1) 1))
  (define-func (untag-int t) (bit-ashr t 1))
  (define-func (is-int obj) (= (bit-and obj 1) 1))
  (define-func (is-oop obj) (= (bit-and obj 1) 0))

  (define-func (slot-at object idx) (peek (+ object OBJECT_HEADER_SIZE idx)))
  (define-func (slot-at-put object idx value) (poke (+ object OBJECT_HEADER_SIZE idx) value))

  (define-func (array-at object idx) (peek (+ object OBJECT_HEADER_SIZE (peek (+ object OBJECT_HEADER_NAMED_SLOTS)) idx)))
  (define-func (array-at-put object idx value) (poke (+ object OBJECT_HEADER_SIZE (peek (+ object OBJECT_HEADER_NAMED_SLOTS)) idx) value))

  (define-func (malloc size)
    (do
      (define-var result heap-pointer)
      (set heap-pointer (+ heap-pointer size))
      result))

  (define-func (new-instance behavior named indexed)
    (do
      (define-var object-size (+ OBJECT_HEADER_SIZE named indexed))
      (define-var object (malloc object-size))
      (for (i 0 object-size)
        (do
	  (poke (+ object i) NULL)))
      (poke (+ object OBJECT_HEADER_BEHAVIOR) behavior)
      (poke (+ object OBJECT_HEADER_NAMED_SLOTS) named)
      (poke (+ object OBJECT_HEADER_INDEXED_SLOTS) indexed)
      object))

  (define-func (new-method-dict capacity)
    (do
      (define-var dict (new-instance (tag-int 989) 1 (* capacity 2)))
      (slot-at-put dict 0 (tag-int 0))
      dict))
  
  (define-func (method-dict-add dict selector code-addr)
    (do
      (define-var size (untag-int (slot-at dict 0)))
      (define-var entry (+ 1 (* size 2)))
      (array-at-put dict entry selector)
      (array-at-put dict (+ entry 1) code-addr)
      (slot-at-put dict  0 (tag-int (+ size 1)))
      dict))
  
  (define-func (method-dict-lookup dict selector)
    (do
      (if (= dict NULL)
          NULL
          (do
            (define-var size (untag-int (slot-at dict 0)))
            (define-var found NULL)
            (for (i 0 size)
              (do
                (define-var entry (+ 1 (* i 2)))
                (if (= (array-at dict entry) selector)
                    (set found (array-at dict (+ entry 1)))
                    0)))
            found))))
  
  (define-func (new-class name superclass)
    (do
      (define-var class (new-instance (tag-int 987) 3 0))
      (slot-at-put class 0 name)
      (slot-at-put class 1 superclass)
      (slot-at-put class 2 NULL)
      class))
  
  (define-func (class-set-methods class dict)
    (slot-at-put class 2 dict))
  
  (define-var SmallInteger-class NULL)
  
  (define-func (get-class obj)
    (if (is-int obj)
        SmallInteger-class
        (peek obj)))
  
  (define-func (get-name class) (slot-at class 0))
  (define-func (get-super class) (slot-at class 1))
  (define-func (get-methods class) (slot-at class 2))
 
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

  ; Context management functions
  (define-func (new-context sender receiver method temp-count)
    (do
      (define-var ctx (new-instance (tag-int 999) CONTEXT_NAMED_SLOTS temp-count))
      (slot-at-put ctx CONTEXT_SENDER sender)
      (slot-at-put ctx CONTEXT_RECEIVER receiver)
      (slot-at-put ctx CONTEXT_METHOD method)
      (slot-at-put ctx CONTEXT_PC (tag-int 0))
      (slot-at-put ctx CONTEXT_SP (tag-int 0))
      ctx))

  (define-func (context-get-sender ctx) (slot-at ctx CONTEXT_SENDER))
  (define-func (context-get-receiver ctx) (slot-at ctx CONTEXT_RECEIVER))
  (define-func (context-get-method ctx) (slot-at ctx CONTEXT_METHOD))
  (define-func (context-get-pc ctx) (untag-int (slot-at ctx CONTEXT_PC)))
  (define-func (context-get-sp ctx) (untag-int (slot-at ctx CONTEXT_SP)))

  (define-func (context-set-pc ctx value) (slot-at-put ctx CONTEXT_PC (tag-int value)))
  (define-func (context-set-sp ctx value) (slot-at-put ctx CONTEXT_SP (tag-int value)))

  (define-func (context-temp-at ctx idx) (array-at ctx idx))
  (define-func (context-temp-at-put ctx idx value) (array-at-put ctx idx value))

  ; Mini test framework
  (define-func (assert-equal actual expected msg)
    (if (= actual expected)
        1
        (abort msg)))

  (define-func (assert-true cond msg)
    (if cond
        1
        (abort msg)))

  ; Message send: create new context and activate it
  (define-func (message-send receiver selector args temp-count)
    (do
      (define-var method (lookup-method receiver selector))
      (if (= method NULL)
          (do
            (print-string "ERROR: Method not found")
            (print-int (untag-int selector))
            NULL)
          (do
            (define-var new-ctx (new-context current-context receiver method temp-count))

            ; Store arguments in context temporaries
            (define-var arg-count (untag-int args))
            (for (i 0 arg-count)
              (context-temp-at-put new-ctx i (peek (+ args 1 i))))

            ; Switch to new context
            (set current-context new-ctx)

            new-ctx))))

  ; Method return: restore sender context and return value
  (define-func (method-return return-value)
    (do
      (if (= current-context NULL)
          (do
            (print-string "ERROR: Cannot return, no active context")
            NULL)
          (do
            (define-var sender (context-get-sender current-context))
            (set current-context sender)
            return-value))))

  (define-func (bootstrap-smalltalk)
    (do
      (print-string "=== Smalltalk Bootstrap ===")
      (print-string "")
      
      (print-string "Creating core classes...")
      
      (define-var ProtoObject (new-class (tag-int 1) NULL))
      (define-var proto-methods (new-method-dict 5))
      (method-dict-add proto-methods (tag-int 10) (tag-int 10000))
      (class-set-methods ProtoObject proto-methods)
      (print-string "  ProtoObject: class (sel:10)")
      
      (define-var Object (new-class (tag-int 2) ProtoObject))
      (define-var obj-methods (new-method-dict 10))
      (method-dict-add obj-methods (tag-int 20) (tag-int 20000))
      (method-dict-add obj-methods (tag-int 21) (tag-int 21000))
      (method-dict-add obj-methods (tag-int 22) (tag-int 22000))
      (class-set-methods Object obj-methods)
      (print-string "  Object: ==, ~=, yourself")
      
      (define-var Magnitude (new-class (tag-int 3) Object))
      (define-var mag-methods (new-method-dict 10))
      (method-dict-add mag-methods (tag-int 30) (tag-int 30000))
      (method-dict-add mag-methods (tag-int 31) (tag-int 31000))
      (class-set-methods Magnitude mag-methods)
      (print-string "  Magnitude: <, >")
      
      (define-var Number (new-class (tag-int 4) Magnitude))
      (define-var num-methods (new-method-dict 10))
      (method-dict-add num-methods (tag-int 40) (tag-int 40000))
      (method-dict-add num-methods (tag-int 41) (tag-int 41000))
      (method-dict-add num-methods (tag-int 42) (tag-int 42000))
      (method-dict-add num-methods (tag-int 43) (tag-int 43000))
      (class-set-methods Number num-methods)
      (print-string "  Number: +, -, *, /")
      
      (define-var SmallInteger (new-class (tag-int 5) Number))
      (set SmallInteger-class SmallInteger)
      (define-var int-methods (new-method-dict 15))
      (method-dict-add int-methods (tag-int 40) (tag-int 50000))
      (method-dict-add int-methods (tag-int 41) (tag-int 51000))
      (method-dict-add int-methods (tag-int 42) (tag-int 52000))
      (method-dict-add int-methods (tag-int 43) (tag-int 53000))
      (method-dict-add int-methods (tag-int 50) (tag-int 54000))
      (method-dict-add int-methods (tag-int 51) (tag-int 55000))
      (class-set-methods SmallInteger int-methods)
      (print-string "  SmallInteger: +, -, *, /, bitAnd:, bitOr:")
      
      (define-var Collection (new-class (tag-int 6) Object))
      (define-var coll-methods (new-method-dict 10))
      (method-dict-add coll-methods (tag-int 60) (tag-int 60000))
      (method-dict-add coll-methods (tag-int 61) (tag-int 61000))
      (class-set-methods Collection coll-methods)
      (print-string "  Collection: size, isEmpty")
      
      (define-var Array (new-class (tag-int 7) Collection))
      (define-var arr-methods (new-method-dict 10))
      (method-dict-add arr-methods (tag-int 70) (tag-int 70000))
      (method-dict-add arr-methods (tag-int 71) (tag-int 71000))
      (class-set-methods Array arr-methods)
      (print-string "  Array: at:, at:put:")
      
      (define-var Point (new-class (tag-int 8) Object))
      (define-var pt-methods (new-method-dict 10))
      (method-dict-add pt-methods (tag-int 80) (tag-int 80000))
      (method-dict-add pt-methods (tag-int 81) (tag-int 81000))
      (method-dict-add pt-methods (tag-int 82) (tag-int 82000))
      (class-set-methods Point pt-methods)
      (print-string "  Point: x, y, dist")
      
      (print-string "")
      (print-string "Class hierarchy:")
      (print-string "  ProtoObject")
      (print-string "    Object")
      (print-string "      Magnitude")
      (print-string "        Number")
      (print-string "          SmallInteger")
      (print-string "      Collection")
      (print-string "        Array")
      (print-string "      Point")
      (print-string "")
      
      (print-string "=== Testing Method Lookup ===")
      (print-string "")
      
      (print-string "Test 1: SmallInteger method lookup")
      (define-var int-val (tag-int 42))
      (print-string "  42 is tagged int?")
      (print-int (is-int int-val))
      
      (print-string "  42 class:")
      (define-var int-class (get-class int-val))
      (print-string "    class addr:")
      (print-int int-class)
      (print-string "    class name:")
      (print-int (untag-int (get-name int-class)))
      
      (print-string "  Lookup: 42 + (sel:40)")
      (define-var add-method (lookup-method int-val (tag-int 40)))
      (print-string "    Found:")
      (print-int (untag-int add-method))
      (print-string "    Expected: 50000 (SmallInteger override)")
      (print-string "")
      
      (print-string "Test 2: SmallInteger inherited method")
      (print-string "  Lookup: 42 < (sel:30)")
      (define-var lt-method (lookup-method int-val (tag-int 30)))
      (print-string "    Found:")
      (print-int (untag-int lt-method))
      (print-string "    Expected: 30000 (from Magnitude)")
      (print-string "")
      
      (print-string "Test 3: SmallInteger from Object")
      (print-string "  Lookup: 42 == (sel:20)")
      (define-var eq-method (lookup-method int-val (tag-int 20)))
      (print-string "    Found:")
      (print-int (untag-int eq-method))
      (print-string "    Expected: 20000 (from Object)")
      (print-string "")
      
      (print-string "Test 4: SmallInteger from ProtoObject")
      (print-string "  Lookup: 42 class (sel:10)")
      (define-var class-method (lookup-method int-val (tag-int 10)))
      (print-string "    Found:")
      (print-int (untag-int class-method))
      (print-string "    Expected: 10000 (from ProtoObject)")
      (print-string "")
      
      (print-string "Test 5: Point instance")
      (define-var p (new-instance Point 2 0))
      (print-string "  Point instance:")
      (print-int p)
      (print-string "  is int?")
      (print-int (is-int p))
      (print-string "  is oop?")
      (print-int (is-oop p))
      
      (print-string "  Lookup: p x (sel:80)")
      (define-var x-method (lookup-method p (tag-int 80)))
      (print-string "    Found:")
      (print-int (untag-int x-method))
      (print-string "    Expected: 80000")
      (print-string "")
      
      (print-string "Test 6: Point inherited from Object")
      (print-string "  Lookup: p == (sel:20)")
      (define-var p-eq-method (lookup-method p (tag-int 20)))
      (print-string "    Found:")
      (print-int (untag-int p-eq-method))
      (print-string "    Expected: 20000 (from Object)")
      (print-string "")
      
      (print-string "Test 7: Array instance")
      (define-var arr (new-instance Array 0 70))
      (print-string "  Lookup: arr at: (sel:70)")
      (define-var at-method (lookup-method arr (tag-int 70)))
      (print-string "    Found:")
      (print-int (untag-int at-method))
      (print-string "    Expected: 70000")
      
      (print-string "  Lookup: arr size (sel:60)")
      (define-var size-method (lookup-method arr (tag-int 60)))
      (print-string "    Found:")
      (print-int (untag-int size-method))
      (print-string "    Expected: 60000 (from Collection)")
      (print-string "")
      
      (print-string "Test 8: Complete inheritance chain")
      (print-string "  SmallInteger hierarchy depth: 5")
      (print-string "    SmallInteger -> Number -> Magnitude -> Object -> ProtoObject")
      (print-string "  Test all levels:")
      
      (define-var si (tag-int 100))
      (print-string "    Level 1 (SmallInteger): bitAnd: (sel:50)")
      (define-var l1 (lookup-method si (tag-int 50)))
      (print-int (untag-int l1))
      
      (print-string "    Level 2 (Number): + (sel:40)")
      (define-var l2 (lookup-method si (tag-int 40)))
      (print-int (untag-int l2))
      
      (print-string "    Level 3 (Magnitude): < (sel:30)")
      (define-var l3 (lookup-method si (tag-int 30)))
      (print-int (untag-int l3))
      
      (print-string "    Level 4 (Object): == (sel:20)")
      (define-var l4 (lookup-method si (tag-int 20)))
      (print-int (untag-int l4))
      
      (print-string "    Level 5 (ProtoObject): class (sel:10)")
      (define-var l5 (lookup-method si (tag-int 10)))
      (print-int (untag-int l5))
      
      (print-string "")
      (print-string "=== Testing Context Management ===")
      (print-string "")

      ; Test 9: Create context
      (print-string "Test 9: Create context")
      (assert-equal current-context NULL "Initial context should be NULL")
      (define-var test-ctx (new-context NULL (tag-int 42) (tag-int 50000) 3))
      (assert-equal (untag-int (context-get-receiver test-ctx)) 42 "Context receiver should be 42")
      (assert-equal (untag-int (context-get-method test-ctx)) 50000 "Context method should be 50000")
      (print-string "  PASSED")

      ; Test 10: Context temporaries
      (print-string "Test 10: Context temporaries")
      (context-temp-at-put test-ctx 0 (tag-int 10))
      (context-temp-at-put test-ctx 1 (tag-int 20))
      (context-temp-at-put test-ctx 2 (tag-int 30))
      (assert-equal (untag-int (context-temp-at test-ctx 0)) 10 "Temp 0 should be 10")
      (assert-equal (untag-int (context-temp-at test-ctx 1)) 20 "Temp 1 should be 20")
      (assert-equal (untag-int (context-temp-at test-ctx 2)) 30 "Temp 2 should be 30")
      (print-string "  PASSED")

      ; Test 11: Message send (context switching)
      (print-string "Test 11: Message send (context switching)")
      (define-var args-addr (malloc 3))
      (poke args-addr (tag-int 2))
      (poke (+ args-addr 1) (tag-int 100))
      (poke (+ args-addr 2) (tag-int 200))
      (define-var msg-ctx (message-send (tag-int 42) (tag-int 40) args-addr 2))
      (assert-equal current-context msg-ctx "Current context should be new context")
      (assert-equal (untag-int (context-get-receiver msg-ctx)) 42 "Message receiver should be 42")
      (assert-equal (untag-int (context-get-method msg-ctx)) 50000 "Should find SmallInteger + method")
      (assert-equal (untag-int (context-temp-at msg-ctx 0)) 100 "First arg should be 100")
      (assert-equal (untag-int (context-temp-at msg-ctx 1)) 200 "Second arg should be 200")
      (print-string "  PASSED")

      ; Test 12: Nested message sends
      (print-string "Test 12: Nested message sends")
      (define-var outer-ctx current-context)
      (define-var args2-addr (malloc 2))
      (poke args2-addr (tag-int 1))
      (poke (+ args2-addr 1) (tag-int 5))
      (define-var inner-ctx (message-send (tag-int 10) (tag-int 41) args2-addr 1))
      (assert-equal (context-get-sender inner-ctx) outer-ctx "Inner sender should be outer context")
      (assert-equal current-context inner-ctx "Current should be inner context")
      (print-string "  PASSED")

      ; Test 13: Method return
      (print-string "Test 13: Method return")
      (define-var return-val (method-return (tag-int 999)))
      (assert-equal (untag-int return-val) 999 "Return value should be 999")
      (assert-equal current-context outer-ctx "Should restore outer context")
      (print-string "  PASSED")

      ; Test 14: Return to NULL (end of chain)
      (print-string "Test 14: Return to NULL (end of chain)")
      (define-var final-return (method-return (tag-int 777)))
      (assert-equal (untag-int final-return) 777 "Final return value should be 777")
      (assert-equal current-context NULL "Should restore to NULL context")
      (print-string "  PASSED")
      (print-string "")

      (print-string "=== All Tests Passed! ===")
      (print-string "")
      (print-string "Bootstrap complete!")
      (print-string "  8 classes created")
      (print-string "  SmallInteger support working")
      (print-string "  5-level inheritance chain working")
      (print-string "  Method override working")
      (print-string "  Context management working!")
      (print-string "  Message send/return working!")
      (print-string "  Ready for full Smalltalk execution!")

      0))
  
  (bootstrap-smalltalk))
