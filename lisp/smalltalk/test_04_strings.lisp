      ; Test 15: String operations with manual string
      (print-string "Test 15: String operations")

      ; Create test string "Hi" manually (H=72, i=105)
      (define-var test-str (malloc 2))
      (poke test-str 2)  ; length = 2
      (define-var word "Hi")  ; String literal address
      (define-var word-data (peek (+ word 1)))  ; Read packed chars from string literal
      (poke (+ test-str 1) word-data)  ; Store the packed chars

      (assert-equal (string-length test-str) 2 "String length should be 2")
      (assert-equal (string-char-at test-str 0) 72 "First char should be 'H'=72")
      (assert-equal (string-char-at test-str 1) 105 "Second char should be 'i'=105")
      (print-string "  PASSED")

      ; Test 16: Character classification
      (print-string "Test 16: Character classification")
      (assert-equal (is-digit 48) 1 "'0'=48 is digit")
      (assert-equal (is-digit 53) 1 "'5'=53 is digit")
      (assert-equal (is-digit 65) 0 "'A'=65 is not digit")
      (assert-equal (is-letter 65) 1 "'A'=65 is letter")
      (assert-equal (is-letter 122) 1 "'z'=122 is letter")
      (assert-equal (is-letter 48) 0 "'0'=48 is not letter")
      (assert-equal (is-whitespace 32) 1 "Space=32 is whitespace")
      (assert-equal (is-whitespace 10) 1 "Newline=10 is whitespace")
      (assert-equal (is-whitespace 65) 0 "'A'=65 is not whitespace")
      (print-string "  PASSED")

      ; Test 17: AST node creation
      (print-string "Test 17: AST node creation")
      (define-var ast-num (new-ast-node AST_NUMBER (tag-int 42) 0))
      (assert-equal (ast-type ast-num) AST_NUMBER "AST type should be NUMBER")
      (assert-equal (untag-int (ast-value ast-num)) 42 "AST value should be 42")

      (define-var ast-binop (new-ast-node AST_BINARY_MSG (tag-int 43) 2))
      (ast-child-put ast-binop 0 ast-num)
      (ast-child-put ast-binop 1 ast-num)
      (assert-equal (ast-type ast-binop) AST_BINARY_MSG "AST type should be BINARY_MSG")
      (assert-equal (ast-child ast-binop 0) ast-num "First child should be ast-num")
      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing Tokenizer ===")
      (print-string "")

