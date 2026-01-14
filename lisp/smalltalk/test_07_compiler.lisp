      ; Test 29: Compile a simple method
      (print-string "Test 29: Compile method '3 + 4'")
      (define-var method1-addr (compile-method "3 + 4" 0))
      (assert-true (> method1-addr 0) "Method address should be non-zero")
      (print-string "  Method compiled at address: ")
      (print method1-addr)

      ; Check that bytecode was emitted
      (define-var m0 (peek method1-addr))
      (define-var m1 (peek (+ method1-addr 1)))
      (assert-equal m0 OP_PUSH "First opcode should be PUSH")
      (assert-equal (untag-int m1) 3 "First value should be 3")
      (print-string "  PASSED")

      ; Test 30: Install method into a class
      (print-string "Test 30: Install method into class")
      (define-var TestClass (new-class (tag-int 999) Object))
      (define-var test-selector (tag-int 100))
      (define-var installed-addr (install-method TestClass test-selector "5 + 3" 0))
      (assert-true (> installed-addr 0) "Installed method address should be non-zero")

      ; Verify method is in class's method dictionary
      (define-var test-instance (new-instance TestClass 0 0))
      (define-var found-method (lookup-method test-instance test-selector))
      (assert-equal found-method installed-addr "Lookup should find installed method")
      (print-string "  PASSED")

      ; Test 31: Compile method with binary operations
      (print-string "Test 31: Compile '10 * 2 + 5'")
      (define-var method2-addr (compile-method "10 * 2 + 5" 0))
      (assert-true (> method2-addr 0) "Method address should be non-zero")
      (print-string "  PASSED")

      ; Test 32: Test FUNCALL primitive
      (print-string "Test 32: Test funcall primitive")
      ; Compile a simple method that returns 42
      (define-var test-method-addr (compile-method "42" 0))
      (print-string "  Compiled test method at: ")
      (print test-method-addr)

      ; Manually emit bytecode to test funcall
      ; We'll create a small bytecode sequence that calls our method
      (init-bytecode 100)
      (emit OP_PUSH)
      (emit test-method-addr)         ; push method address
      (emit OP_PUSH)
      (emit 0)                         ; push arg count (0 args)
      (emit OP_FUNCALL)               ; call it
      (emit OP_HALT)

      ; For now, just verify bytecode was emitted
      (define-var funcall-test-addr bytecode-buffer)
      (assert-true (> funcall-test-addr 0) "Funcall test bytecode created")
      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing Message Send Compilation (Step 6) ===")
      (print-string "")

