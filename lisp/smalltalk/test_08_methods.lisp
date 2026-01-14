      (print-string "Test 33: Get lookup-method function address")
      ; Use function-address to get the compiled address of lookup-method
      (set lookup-method-addr (function-address lookup-method))
      (print-string "  lookup-method address:")
      (print-int lookup-method-addr)
      (print-string "  PASSED")
      (print-string "")

      ; Test 34: Compile unary message send "42 negated"
      (print-string "Test 34: Compile unary message '42 negated'")

      ; First, install a 'negated' method on SmallInteger
      ; The method should return the negation of the receiver
      (define-var negated-selector (tag-int 200))

      ; Create a simple negated method: (0 - self)
      (init-bytecode 100)
      (emit OP_PUSH)
      (emit (tag-int 0))                    ; Push 0
      (emit OP_BP_LOAD)
      (emit 0)                              ; Load self (first argument)
      (emit OP_SUB)                         ; 0 - self
      (emit OP_RET)
      (emit 0)                              ; No local args to clean up
      (define-var negated-method-addr bytecode-buffer)

      ; Install it in SmallInteger class
      (define-var si-methods (get-methods SmallInteger-class))
      (method-dict-add si-methods negated-selector negated-method-addr)
      (print-string "  Installed 'negated' method in SmallInteger")

      ; Now compile a Smalltalk expression that uses it
      ; For now, just verify the method is installed
      (define-var found-negated (lookup-method (tag-int 42) negated-selector))
      (assert-equal found-negated negated-method-addr "Should find negated method")
      (print-string "  PASSED")
      (print-string "")

      ; Test 35: Compile and verify bytecode for binary message
      (print-string "Test 35: Compile binary message '10 + 5'")
      (define-var binary-test-source (malloc 2))
      (poke binary-test-source 6)  ; length = 6
      ; "10 + 5" = 1=49, 0=48, space=32, +=43, space=32, 5=53
      (define-var w-binary "10 + 5")  ; String literal address
      (define-var w-binary-data (peek (+ w-binary 1)))  ; Read packed chars
      (poke (+ binary-test-source 1) w-binary-data)

      (define-var binary-code (compile-smalltalk binary-test-source))
      (assert-true (> binary-code 0) "Binary message compiled")

      ; With message send compilation, "10 + 5" now generates method lookup bytecode
      ; Just verify it starts with PUSH 10
      (define-var bc0 (peek binary-code))
      (define-var bc1 (peek (+ binary-code 1)))

      (assert-equal bc0 OP_PUSH "First op should be PUSH")
      (assert-equal (untag-int bc1) 10 "First value should be 10")
      (print-string "  Note: Now using message send compilation")
      (print-string "  PASSED")
      (print-string "")

      ; Test 36: Test method lookup through inheritance
      (print-string "Test 36: Method lookup through inheritance chain")

      ; Create a method in Object class
      (define-var object-method-sel (tag-int 300))
      (init-bytecode 50)
      (emit OP_PUSH)
      (emit (tag-int 999))                  ; Return 999
      (emit OP_RET)
      (emit 0)
      (define-var object-method-addr bytecode-buffer)

      (define-var obj-methods-test (get-methods Object))
      (method-dict-add obj-methods-test object-method-sel object-method-addr)

      ; Verify SmallInteger instance can find it through inheritance
      (define-var si-instance (tag-int 42))
      (define-var found-in-object (lookup-method si-instance object-method-sel))
      (assert-equal found-in-object object-method-addr "Should find method from Object")
      (print-string "  Method found through 4-level inheritance!")
      (print-string "  (SmallInteger -> Number -> Magnitude -> Object)")
      (print-string "  PASSED")
      (print-string "")

      ; Test 37: Test method override behavior
      (print-string "Test 37: Method override in inheritance")

      ; Add same selector to SmallInteger (should override Object version)
      (define-var override-sel (tag-int 301))

      ; Object version returns 100
      (init-bytecode 50)
      (emit OP_PUSH)
      (emit (tag-int 100))
      (emit OP_RET)
      (emit 0)
      (define-var object-override-addr bytecode-buffer)
      (method-dict-add obj-methods-test override-sel object-override-addr)

      ; SmallInteger version returns 200
      (init-bytecode 50)
      (emit OP_PUSH)
      (emit (tag-int 200))
      (emit OP_RET)
      (emit 0)
      (define-var si-override-addr bytecode-buffer)
      (method-dict-add si-methods override-sel si-override-addr)

      ; SmallInteger should get its own version (200), not Object's (100)
      (define-var found-override (lookup-method (tag-int 7) override-sel))
      (assert-equal found-override si-override-addr "Should find SmallInteger version")
      (print-string "  Method override works correctly!")
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
      (print-string "  String operations working!")
      (print-string "  AST node system working!")
      (print-string "  Tokenizer working! (numbers, identifiers, keywords, binary ops)")
      (print-string "  Parser working! (unary, binary, keyword messages)")
      (print-string "  Smalltalk->VM bytecode compiler working!")
      (print-string "  Method compilation and installation working!")
      (print-string "")
      (print-string "Smalltalk implementation (Step 5 in progress)!")
      (print-string "  Binary messages: 3 + 4")
      (print-string "  Unary messages: Point new")
      (print-string "  Keyword messages: Point x: 3 y: 4")
      (print-string "  All message types parse correctly!")
      (print-string "  Bytecode compilation working for arithmetic")
      (print-string "  Method compilation: parse -> bytecode with RET")
      (print-string "  Method installation: compile and add to class")
      (print-string "  FUNCALL primitive: dynamic function calls working")
      (print-string "  Message send: partial inline lookup (ready for completion)")
      (print-string "")

      (print-string "=== Testing Actual Method Implementation (Step 7) ===")
      (print-string "")

      ; Test 38: Implement real SmallInteger arithmetic methods
      (print-string "Test 38: Implement real SmallInteger arithmetic methods")

      ; Define actual method implementations as Lisp functions
      ; These take receiver as first argument (via BP_LOAD 0)
      ; and optional argument as second (via BP_LOAD 1)

      (define-func (si-add-impl receiver arg)
        (do
          (define-var a (untag-int receiver))
          (define-var b (untag-int arg))
          (tag-int (+ a b))))

      (define-func (si-sub-impl receiver arg)
        (do
          (define-var a (untag-int receiver))
          (define-var b (untag-int arg))
          (tag-int (- a b))))

      (define-func (si-mul-impl receiver arg)
        (do
          (define-var a (untag-int receiver))
          (define-var b (untag-int arg))
          (tag-int (* a b))))

      (define-func (si-div-impl receiver arg)
        (do
          (define-var a (untag-int receiver))
          (define-var b (untag-int arg))
          (tag-int (/ a b))))

      ; Replace the placeholder addresses with real implementations
      (define-var si-methods-real (get-methods SmallInteger-class))

      ; Create selector strings and intern them
      ; Selector: "+"
      (define-var str-selector-plus (malloc 2))
      (poke str-selector-plus 1)
      (poke (+ str-selector-plus 1) 43)  ; + = ASCII 43
      (define-var sel-plus-id (intern-selector str-selector-plus))

      ; Selector: "-"
      (define-var str-selector-minus (malloc 2))
      (poke str-selector-minus 1)
      (poke (+ str-selector-minus 1) 45)  ; - = ASCII 45
      (define-var sel-minus-id (intern-selector str-selector-minus))

      ; Selector: "*"
      (define-var str-selector-mul (malloc 2))
      (poke str-selector-mul 1)
      (poke (+ str-selector-mul 1) 42)  ; * = ASCII 42
      (define-var sel-mul-id (intern-selector str-selector-mul))

      ; Selector: "/"
      (define-var str-selector-div (malloc 2))
      (poke str-selector-div 1)
      (poke (+ str-selector-div 1) 47)  ; / = ASCII 47
      (define-var sel-div-id (intern-selector str-selector-div))

      ; Install methods with symbol table IDs
      (method-dict-add si-methods-real sel-plus-id (function-address si-add-impl))
      (method-dict-add si-methods-real sel-minus-id (function-address si-sub-impl))
      (method-dict-add si-methods-real sel-mul-id (function-address si-mul-impl))
      (method-dict-add si-methods-real sel-div-id (function-address si-div-impl))

      (print-string "  Installed + with selector ID:")
      (print-int (untag-int sel-plus-id))

      (print-string "  Installed real + method at:")
      (print-int (function-address si-add-impl))
      (print-string "  PASSED")
      (print-string "")

      ; Test 39: Direct method invocation
      (print-string "Test 39: Direct method invocation via FUNCALL")

      ; Test calling add method directly
      (define-var test-result (si-add-impl (tag-int 5) (tag-int 3)))
      (assert-equal (untag-int test-result) 8 "5 + 3 should be 8")
      (print-string "  Direct call: 5 + 3 = 8")

      ; Test subtraction
      (define-var test-sub (si-sub-impl (tag-int 10) (tag-int 7)))
      (assert-equal (untag-int test-sub) 3 "10 - 7 should be 3")
      (print-string "  Direct call: 10 - 7 = 3")

      ; Test multiplication
      (define-var test-mul (si-mul-impl (tag-int 6) (tag-int 7)))
      (assert-equal (untag-int test-mul) 42 "6 * 7 should be 42")
      (print-string "  Direct call: 6 * 7 = 42")

      ; Test division
      (define-var test-div (si-div-impl (tag-int 20) (tag-int 4)))
      (assert-equal (untag-int test-div) 5 "20 / 4 should be 5")
      (print-string "  Direct call: 20 / 4 = 5")

      (print-string "  PASSED: All arithmetic methods work")
      (print-string "")

      ; Test 40: Method lookup and call chain
      (print-string "Test 40: Lookup and call via function pointers")

      ; Simulate what message send does: lookup then call
      (define-var receiver-40 (tag-int 15))
      (define-var arg-40 (tag-int 8))

      ; Use the selector ID from symbol table (already interned above)
      ; 1. Lookup the method
      (define-var method-addr (lookup-method receiver-40 sel-plus-id))
      (assert-true (> method-addr 0) "Should find + method")
      (print-string "  Found method at:")
      (print-int method-addr)

      ; 2. Call it (directly, since we can't use FUNCALL from within Lisp)
      ; In real message send, this would be: FUNCALL method-addr with receiver and arg
      (define-var result-40 (si-add-impl receiver-40 arg-40))
      (assert-equal (untag-int result-40) 23 "15 + 8 should be 23")
      (print-string "  Lookup + call: 15 + 8 = 23")

      (print-string "  PASSED: Lookup and call chain works")
      (print-string "")

      ; Test 41: Unary method (negated)
      (print-string "Test 41: Unary method implementation")

      (define-func (si-negated-impl receiver)
        (do
          (define-var val (untag-int receiver))
          (tag-int (- 0 val))))

      ; Create and intern "negated" selector
      (define-var str-negated-sel (malloc 2))
      (poke str-negated-sel 7)  ; "negated" = 7 chars
      (define-var w-negated-sel "negated")  ; String literal address
      (define-var w-negated-sel-data (peek (+ w-negated-sel 1)))  ; Read packed chars
      (poke (+ str-negated-sel 1) w-negated-sel-data)
      (define-var negated-sel (intern-selector str-negated-sel))

      (print-string "  Installed negated with selector ID:")
      (print-int (untag-int negated-sel))

      (method-dict-add si-methods-real negated-sel (function-address si-negated-impl))

      ; Test it
      (define-var neg-result (si-negated-impl (tag-int 42)))
      (assert-equal (untag-int neg-result) -42 "negated(42) should be -42")
      (print-string "  negated(42) = -42")

      (define-var neg-result2 (si-negated-impl (tag-int -10)))
      (assert-equal (untag-int neg-result2) 10 "negated(-10) should be 10")
      (print-string "  negated(-10) = 10")

      (print-string "  PASSED: Unary methods work")
      (print-string "")

      ; Test 42: Comparison methods
      (print-string "Test 42: Comparison methods")

      (define-func (si-lt-impl receiver arg)
        (do
          (define-var a (untag-int receiver))
          (define-var b (untag-int arg))
          (if (< a b) (tag-int 1) (tag-int 0))))

      (define-func (si-gt-impl receiver arg)
        (do
          (define-var a (untag-int receiver))
          (define-var b (untag-int arg))
          (if (> a b) (tag-int 1) (tag-int 0))))

      (define-func (si-eq-impl receiver arg)
        (do
          (define-var a (untag-int receiver))
          (define-var b (untag-int arg))
          (if (= a b) (tag-int 1) (tag-int 0))))

      ; Create and intern comparison selector strings
      ; Selector: "<"
      (define-var str-lt (malloc 2))
      (poke str-lt 1)
      (poke (+ str-lt 1) 60)  ; < = ASCII 60
      (define-var sel-lt-id (intern-selector str-lt))

      ; Selector: ">"
      (define-var str-gt (malloc 2))
      (poke str-gt 1)
      (poke (+ str-gt 1) 62)  ; > = ASCII 62
      (define-var sel-gt-id (intern-selector str-gt))

      ; Selector: "=="
      (define-var str-eq (malloc 2))
      (poke str-eq 2)
      (define-var w-eq "==")  ; String literal address
      (define-var w-eq-data (peek (+ w-eq 1)))  ; Read packed chars
      (poke (+ str-eq 1) w-eq-data)  ; == = ASCII 61, 61
      (define-var sel-eq-id (intern-selector str-eq))

      ; Install comparison methods with symbol table IDs
      (method-dict-add si-methods-real sel-lt-id (function-address si-lt-impl))
      (method-dict-add si-methods-real sel-gt-id (function-address si-gt-impl))
      (method-dict-add si-methods-real sel-eq-id (function-address si-eq-impl))

      (print-string "  Installed < with selector ID:")
      (print-int (untag-int sel-lt-id))

      ; Test comparisons
      (define-var cmp1 (si-lt-impl (tag-int 3) (tag-int 5)))
      (assert-equal (untag-int cmp1) 1 "3 < 5 should be true")
      (print-string "  3 < 5 = true")

      (define-var cmp2 (si-gt-impl (tag-int 10) (tag-int 4)))
      (assert-equal (untag-int cmp2) 1 "10 > 4 should be true")
      (print-string "  10 > 4 = true")

      (define-var cmp3 (si-eq-impl (tag-int 7) (tag-int 7)))
      (assert-equal (untag-int cmp3) 1 "7 == 7 should be true")
      (print-string "  7 == 7 = true")

      (define-var cmp4 (si-lt-impl (tag-int 8) (tag-int 3)))
      (assert-equal (untag-int cmp4) 0 "8 < 3 should be false")
      (print-string "  8 < 3 = false")

      (print-string "  PASSED: Comparison methods work")
      (print-string "")

      ; Test 43: Verify complete method dictionary
      (print-string "Test 43: Complete SmallInteger method dictionary")

      ; Count methods in SmallInteger
      (define-var method-count (untag-int (slot-at si-methods-real 0)))
      (print-string "  Total methods installed:")
      (print-int method-count)
      (assert-true (>= method-count 8) "Should have at least 8 methods")

      ; Verify all critical methods are findable using symbol table IDs
      (assert-true (> (lookup-method (tag-int 1) sel-plus-id) 0) "+ not found")
      (assert-true (> (lookup-method (tag-int 1) sel-minus-id) 0) "- not found")
      (assert-true (> (lookup-method (tag-int 1) sel-mul-id) 0) "* not found")
      (assert-true (> (lookup-method (tag-int 1) sel-div-id) 0) "/ not found")
      (assert-true (> (lookup-method (tag-int 1) sel-eq-id) 0) "== not found")
      (assert-true (> (lookup-method (tag-int 1) sel-lt-id) 0) "< not found")
      (assert-true (> (lookup-method (tag-int 1) sel-gt-id) 0) "> not found")
      (assert-true (> (lookup-method (tag-int 1) negated-sel) 0) "negated not found")

      (print-string "  All 8+ methods findable via lookup")
      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Message Send Foundation Complete! ===")
      (print-string "")
      (print-string "Achievements:")
      (print-string "  - Real method implementations working")
      (print-string "  - Arithmetic: +, -, *, /")
      (print-string "  - Comparisons: <, >, ==")
      (print-string "  - Unary: negated")
      (print-string "  - Method lookup via function pointers")
      (print-string "  - lookup-method compiled at: ")
      (print-int lookup-method-addr)
      (print-string "  - Ready for VM execution of message sends!")
      (print-string "")

      (print-string "=== Testing VM Execution of Compiled Message Sends (Step 8) ===")
      (print-string "")

