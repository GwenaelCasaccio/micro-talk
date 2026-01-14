      ; Test 18: Tokenize "3 + 4"
      (print-string "Test 18: Tokenize '3 + 4'")

      ; Create test string "3 + 4" manually
      (define-var test-source (malloc 2))
      (poke test-source 5)  ; length = 5
      (define-var w0 "3 + 4")  ; String literal address
      (define-var w0-data (peek (+ w0 1)))  ; Read packed chars
      (poke (+ test-source 1) w0-data)  ; "3 + 4" (51=3, 32=space, 43=+, 32=space, 52=4)

      (define-var tokens (tokenize test-source))

      ; Should have 4 tokens: NUMBER(3), BINARY_OP(+), NUMBER(4), EOF
      (define-var tok0 (array-at tokens 0))
      (define-var tok1 (array-at tokens 1))
      (define-var tok2 (array-at tokens 2))
      (define-var tok3 (array-at tokens 3))

      (assert-equal (token-type tok0) TOK_NUMBER "First token should be NUMBER")
      (assert-equal (untag-int (token-value tok0)) 3 "First token value should be 3")

      (assert-equal (token-type tok1) TOK_BINARY_OP "Second token should be BINARY_OP")
      (assert-equal (untag-int (token-value tok1)) 43 "Second token value should be + (43)")

      (assert-equal (token-type tok2) TOK_NUMBER "Third token should be NUMBER")
      (assert-equal (untag-int (token-value tok2)) 4 "Third token value should be 4")

      (assert-equal (token-type tok3) TOK_EOF "Fourth token should be EOF")

      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing Parser ===")
      (print-string "")

      ; Test 19: Parse "3 + 4" into AST
      (print-string "Test 19: Parse '3 + 4' into AST")

      (define-var ast (parse test-source))

      ; AST should be: BINARY_MSG(+, NUMBER(3), NUMBER(4))
      (assert-equal (ast-type ast) AST_BINARY_MSG "Root should be BINARY_MSG")
      (assert-equal (untag-int (ast-value ast)) 43 "Operator should be + (43)")

      (define-var left-child (ast-child ast 0))
      (define-var right-child (ast-child ast 1))

      (assert-equal (ast-type left-child) AST_NUMBER "Left child should be NUMBER")
      (assert-equal (untag-int (ast-value left-child)) 3 "Left value should be 3")

      (assert-equal (ast-type right-child) AST_NUMBER "Right child should be NUMBER")
      (assert-equal (untag-int (ast-value right-child)) 4 "Right value should be 4")

      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing Smalltalk Compiler ===")
      (print-string "")

      ; Test 20: Compile "3 + 4" to bytecode
      (print-string "Test 20: Compile '3 + 4' to VM bytecode")

      (define-var code-addr (compile-smalltalk test-source))

      ; Verify bytecode was generated
      (assert-true (> code-addr 0) "Code address should be valid")

      ; Inspect compiled bytecode
      (print-string "  Compiled bytecode:")
      (print-string "    Address:")
      (print-int code-addr)

      ; With message send compilation, "3 + 4" now generates:
      ; PUSH 3, DUP, PUSH selector, PUSH lookup-addr, PUSH 2, FUNCALL, (method save/load), compile 4, FUNCALL, HALT
      ; Just verify it starts with PUSH and has valid opcodes
      (define-var b0 (peek code-addr))
      (define-var b1 (peek (+ code-addr 1)))

      (print-string "    First opcode:")
      (print-int b0)
      (print-string "    First operand:")
      (print-int b1)

      ; Verify first instruction is PUSH 3
      (assert-equal b0 OP_PUSH "First opcode should be PUSH")
      (assert-equal (untag-int b1) 3 "First operand should be 3")

      (print-string "  Note: Now using message send with method lookup instead of direct ADD")

      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing Extended Messages (Step 4) ===")
      (print-string "")

      ; Test 21: Tokenize identifier
      (print-string "Test 21: Tokenize 'Point new'")

      ; Create test string "Point new"
      (define-var test-unary (malloc 3))
      (poke test-unary 9)  ; length = 9
      ; "Point new" = P=80, o=111, i=105, n=110, t=116, space=32, n=110, e=101, w=119
      (define-var w-unary0 "Point new")  ; String literal address
      (define-var w-unary0-data (peek (+ w-unary0 1)))  ; Read first 8 chars
      (define-var w-unary1 119)  ; 'w'
      (poke (+ test-unary 1) w-unary0-data)
      (poke (+ test-unary 2) w-unary1)

      (define-var tokens-unary (tokenize test-unary))
      (define-var tok-p (array-at tokens-unary 0))
      (define-var tok-new (array-at tokens-unary 1))
      (define-var tok-eof1 (array-at tokens-unary 2))

      (assert-equal (token-type tok-p) TOK_IDENTIFIER "First token should be IDENTIFIER")
      (assert-equal (token-type tok-new) TOK_IDENTIFIER "Second token should be IDENTIFIER")
      (assert-equal (token-type tok-eof1) TOK_EOF "Third token should be EOF")
      (print-string "  PASSED")

      ; Test 22: Parse unary message "Point new"
      (print-string "Test 22: Parse 'Point new'")
      (define-var ast-unary (parse test-unary))

      ; Should be: UNARY_MSG(new, IDENTIFIER(Point))
      (assert-equal (ast-type ast-unary) AST_UNARY_MSG "Root should be UNARY_MSG")

      (define-var unary-receiver (ast-child ast-unary 0))
      (assert-equal (ast-type unary-receiver) AST_IDENTIFIER "Receiver should be IDENTIFIER")
      (print-string "  PASSED")

      ; Test 23: Tokenize keyword message
      (print-string "Test 23: Tokenize 'x: 3 y: 4'")

      ; Create test string "x: 3 y: 4" (9 chars)
      (define-var test-keyword (malloc 3))
      (poke test-keyword 9)  ; length = 9
      ; "x: 3 y: 4" = x=120, :=58, space=32, 3=51, space=32, y=121, :=58, space=32, 4=52
      (define-var w-kw0 "x: 3 y: 4")  ; String literal address
      (define-var w-kw0-data (peek (+ w-kw0 1)))  ; Read first 8 chars
      (define-var w-kw1 52)  ; '4'
      (poke (+ test-keyword 1) w-kw0-data)
      (poke (+ test-keyword 2) w-kw1)

      (define-var tokens-kw (tokenize test-keyword))
      (define-var tok-x (array-at tokens-kw 0))
      (define-var tok-3 (array-at tokens-kw 1))
      (define-var tok-y (array-at tokens-kw 2))
      (define-var tok-4 (array-at tokens-kw 3))

      (assert-equal (token-type tok-x) TOK_KEYWORD "First token should be KEYWORD")
      (assert-equal (token-type tok-3) TOK_NUMBER "Second token should be NUMBER")
      (assert-equal (token-type tok-y) TOK_KEYWORD "Third token should be KEYWORD")
      (assert-equal (token-type tok-4) TOK_NUMBER "Fourth token should be NUMBER")
      (print-string "  PASSED")

      ; Test 24: Tokenize binary message "5 + 3"
      (print-string "Test 24: Tokenize '5 + 3'")

      ; Create test string "5 + 3" (5 chars)
      (define-var test-binary-tok (malloc 2))
      (poke test-binary-tok 5)  ; length = 5
      ; "5 + 3" = 5=53, space=32, +=43, space=32, 3=51
      (define-var w-bin-tok "5 + 3")  ; String literal address
      (define-var w-bin-tok-data (peek (+ w-bin-tok 1)))  ; Read packed chars
      (poke (+ test-binary-tok 1) w-bin-tok-data)

      (define-var tokens-bin (tokenize test-binary-tok))
      (define-var tok-5 (array-at tokens-bin 0))
      (define-var tok-plus (array-at tokens-bin 1))
      (define-var tok-3-bin (array-at tokens-bin 2))

      (assert-equal (token-type tok-5) TOK_NUMBER "First token should be NUMBER")
      (assert-equal (untag-int (token-value tok-5)) 5 "First token value should be 5")
      (assert-equal (token-type tok-plus) TOK_BINARY_OP "Second token should be BINARY_OP")
      (assert-equal (untag-int (token-value tok-plus)) 43 "Second token value should be + (43)")
      (assert-equal (token-type tok-3-bin) TOK_NUMBER "Third token should be NUMBER")
      (assert-equal (untag-int (token-value tok-3-bin)) 3 "Third token value should be 3")
      (print-string "  PASSED")

