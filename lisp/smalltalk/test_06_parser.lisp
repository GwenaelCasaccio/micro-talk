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

      ; Test 25: Parse binary message "5 + 3"
      (print-string "Test 25: Parse '5 + 3'")

      (define-var ast-binary (parse test-binary-tok))

      ; Should be: BINARY_MSG(+, NUMBER(5), NUMBER(3))
      (assert-equal (ast-type ast-binary) AST_BINARY_MSG "Root should be BINARY_MSG")
      (assert-equal (untag-int (ast-value ast-binary)) 43 "Operator should be + (43)")

      (define-var bin-left (ast-child ast-binary 0))
      (define-var bin-right (ast-child ast-binary 1))

      (assert-equal (ast-type bin-left) AST_NUMBER "Left child should be NUMBER")
      (assert-equal (untag-int (ast-value bin-left)) 5 "Left value should be 5")
      (assert-equal (ast-type bin-right) AST_NUMBER "Right child should be NUMBER")
      (assert-equal (untag-int (ast-value bin-right)) 3 "Right value should be 3")
      (print-string "  PASSED")

      ; Test 26: Tokenize "Point x: 3" first to debug
      (print-string "Test 26: Tokenize 'Point x: 3'")

      ; Create test string "Point x: 3" (10 chars)
      ; Need: 1 word for length + (10+7)/8 = 2 words for chars = 3 words total
      (define-var test-point-kw (malloc 3))
      (poke test-point-kw 10)  ; length = 10
      ; "Point x: 3" = P=80, o=111, i=105, n=110, t=116, space=32, x=120, :=58
      (define-var w-point-kw0 "Point x: 3")  ; String literal address
      (define-var w-point-kw0-data (peek (+ w-point-kw0 1)))  ; Read first 8 chars
      ; " 3" = space=32, 3=51
      (define-var w-point-kw1 " 3")  ; String literal address
      (define-var w-point-kw1-data (peek (+ w-point-kw1 1)))  ; Read last 2 chars
      (poke (+ test-point-kw 1) w-point-kw0-data)
      (poke (+ test-point-kw 2) w-point-kw1-data)

      (define-var tokens-point-kw (tokenize test-point-kw))

      ; Should have: IDENTIFIER(Point), KEYWORD(x:), NUMBER(3), EOF
      (define-var tok-Point (array-at tokens-point-kw 0))
      (define-var tok-x-colon (array-at tokens-point-kw 1))
      (define-var tok-3-kw (array-at tokens-point-kw 2))
      (define-var tok-eof-kw (array-at tokens-point-kw 3))

      (assert-equal (token-type tok-Point) TOK_IDENTIFIER "Token 0 should be IDENTIFIER")
      (assert-equal (token-type tok-x-colon) TOK_KEYWORD "Token 1 should be KEYWORD")
      (assert-equal (token-type tok-3-kw) TOK_NUMBER "Token 2 should be NUMBER")
      (assert-equal (token-type tok-eof-kw) TOK_EOF "Token 3 should be EOF")
      (print-string "  PASSED")

      ; Test 27: Parse simple number "7" to verify parser works
      (print-string "Test 27: Parse '7'")

      ; Create test string "7" (1 char)
      (define-var test-seven (malloc 2))
      (poke test-seven 1)  ; length = 1
      (poke (+ test-seven 1) 55)  ; '7' = ASCII 55

      (define-var ast-seven (parse test-seven))

      ; Should be: NUMBER(7)
      (assert-equal (ast-type ast-seven) AST_NUMBER "Root should be NUMBER")
      (assert-equal (untag-int (ast-value ast-seven)) 7 "Value should be 7")
      (print-string "  PASSED")

      ; Test 28: Parse keyword message "Point x: 3"
      (print-string "Test 28: Parse 'Point x: 3'")

      ; Create test string "Point x: 3" (10 chars)
      ; Need: 1 word for length + (10+7)/8 = 2 words for chars = 3 words total
      (define-var test-kw-full (malloc 3))
      (poke test-kw-full 10)  ; length = 10
      ; "Point x: 3" = P=80, o=111, i=105, n=110, t=116, space=32, x=120, :=58
      (define-var w-kw-full0 "Point x: 3")  ; String literal address
      (define-var w-kw-full0-data (peek (+ w-kw-full0 1)))  ; Read first 8 chars
      ; " 3" = space=32, 3=51
      (define-var w-kw-full1 " 3")  ; String literal address
      (define-var w-kw-full1-data (peek (+ w-kw-full1 1)))  ; Read last 2 chars
      (poke (+ test-kw-full 1) w-kw-full0-data)
      (poke (+ test-kw-full 2) w-kw-full1-data)

      (define-var ast-kw-full (parse test-kw-full))

      ; Should be: KEYWORD_MSG(x:, IDENTIFIER(Point), NUMBER(3))
      (assert-equal (ast-type ast-kw-full) AST_KEYWORD_MSG "Root should be KEYWORD_MSG")

      (define-var kw-full-receiver (ast-child ast-kw-full 0))
      (define-var kw-full-arg (ast-child ast-kw-full 1))

      (assert-equal (ast-type kw-full-receiver) AST_IDENTIFIER "Receiver should be IDENTIFIER")
      (assert-equal (ast-type kw-full-arg) AST_NUMBER "Argument should be NUMBER")
      (assert-equal (untag-int (ast-value kw-full-arg)) 3 "Argument value should be 3")
      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing Method Compilation (Step 5) ===")
      (print-string "")

