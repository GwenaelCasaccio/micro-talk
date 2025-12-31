; Smalltalk Parser with Typed String Parameters
; Demonstrates the Lisp-to-C++ transpiler with typed functions

(do
  ; ============================================================================
  ; CHARACTER CLASSIFICATION HELPERS
  ; ============================================================================
  (define (is-digit (ch int)) int
    (if (< ch 48) 0 (if (> ch 57) 0 1)))

  (define (is-letter (ch int)) int
    (if (< ch 65) 0
      (if (<= ch 90) 1
        (if (< ch 97) 0
          (if (<= ch 122) 1 0)))))

  (define (is-whitespace (ch int)) int
    (if (= ch 32) 1
      (if (= ch 9) 1
        (if (= ch 10) 1
          (if (= ch 13) 1 0)))))

  (define (is-alnum (ch int)) int
    (if (is-letter ch) 1 (if (is-digit ch) 1 0)))

  ; ============================================================================
  ; TOKENIZER FUNCTIONS
  ; ============================================================================

  ; Skip whitespace and return new position
  (define (skip-whitespace (source string) (pos int)) int
    (do
      (define len (string-length source))
      (while (if (< pos len) (is-whitespace (char-at source pos)) 0)
        (set pos (+ pos 1)))
      pos))

  ; Scan while condition is true
  (define (scan-while-digit (source string) (pos int)) int
    (do
      (define len (string-length source))
      (while (if (< pos len) (is-digit (char-at source pos)) 0)
        (set pos (+ pos 1)))
      pos))

  (define (scan-while-alnum (source string) (pos int)) int
    (do
      (define len (string-length source))
      (while (if (< pos len) (is-alnum (char-at source pos)) 0)
        (set pos (+ pos 1)))
      pos))

  ; Scan a string literal (returns position after closing quote)
  (define (scan-string-lit (source string) (pos int)) int
    (do
      (define len (string-length source))
      ; Skip opening quote
      (set pos (+ pos 1))
      ; Scan until closing quote
      (while (if (< pos len) (if (= (char-at source pos) 39) 0 1) 0)
        (set pos (+ pos 1)))
      ; Skip closing quote if present
      (if (< pos len) (+ pos 1) pos)))

  ; ============================================================================
  ; TOKEN TYPE NAMES (for display)
  ; ============================================================================
  (define (token-type-name (tok-type int)) string
    (if (= tok-type 1) "NUM"
      (if (= tok-type 2) "ID "
        (if (= tok-type 3) "KEY"
          (if (= tok-type 4) "STR"
            (if (= tok-type 5) "SYM"
              (if (= tok-type 6) "OP "
                "UNK")))))))

  ; ============================================================================
  ; MAIN TOKENIZER
  ; Token types: 1=NUM, 2=ID, 3=KEY, 4=STR, 5=SYM, 6=OP
  ; ============================================================================
  (define (tokenize-and-display (source string)) int
    (do
      (define len (string-length source))
      (define pos 0)
      (define count 0)

      (c++ "std::cout << \"Tokenizing: \" << source << std::endl")
      (c++ "std::cout << \"Length: \" << len << \" characters\\n\" << std::endl")

      (while (< pos len)
        (do
          ; Skip whitespace
          (set pos (skip-whitespace source pos))

          (if (< pos len)
            (do
              (define ch (char-at source pos))
              (define start pos)
              (define end pos)
              (define tok-type 0)

              ; Classify and scan token
              (if (is-digit ch)
                (do
                  (set end (scan-while-digit source pos))
                  (set tok-type 1))  ; NUM

                (if (is-letter ch)
                  (do
                    (set end (scan-while-alnum source pos))
                    ; Check for keyword (trailing colon)
                    (if (< end len)
                      (if (= (char-at source end) 58)
                        (do
                          (set end (+ end 1))
                          (set tok-type 3))  ; KEY
                        (set tok-type 2))    ; ID
                      (set tok-type 2)))     ; ID

                  (if (= ch 39)  ; Single quote
                    (do
                      (set end (scan-string-lit source pos))
                      (set tok-type 4))      ; STR

                    (if (= ch 35)  ; Hash
                      (do
                        (set end (+ pos 1))
                        (set end (scan-while-alnum source end))
                        (set tok-type 5))    ; SYM

                      ; Operator
                      (do
                        (set end (+ pos 1))
                        ; Check for :=
                        (if (= ch 58)
                          (if (< end len)
                            (if (= (char-at source end) 61)
                              (set end (+ end 1))
                              0)
                            0)
                          0)
                        (set tok-type 6))))))  ; OP

              ; Print token
              (define token-text (substring source start end))
              (define type-name (token-type-name tok-type))

              (c++ "std::cout << \"  \" << (count + 1) << \". \"
                   << type_name << \" [\" << start << \"-\" << end << \"]: '\"
                   << token_text << \"'\" << std::endl")

              (set count (+ count 1))
              (set pos end))
            0)))

      (c++ "std::cout << \"\\nTotal tokens: \" << count << std::endl")
      count))

  ; ============================================================================
  ; PATTERN ANALYZER - Detects common Smalltalk patterns
  ; ============================================================================
  (define (contains-substr (haystack string) (needle string)) int
    (c++ "(haystack.find(needle) != std::string::npos ? 1LL : 0LL)"))

  (define (analyze-syntax (source string)) string
    (do
      (define pattern "Unknown")

      ; Check for assignment
      (if (contains-substr source ":=")
        (do (set pattern "Assignment") 0)
        0)

      ; Check for keyword message
      (if (= pattern "Unknown")
        (do
          (if (contains-substr source ":")
            (do (set pattern "Keyword message") 0)
            0)
          0)
        0)

      ; Check for symbol
      (if (= pattern "Unknown")
        (do
          (if (> (string-length source) 0)
            (do
              (if (= (char-at source 0) 35)
                (do (set pattern "Symbol literal") 0)
                0)
              0)
            0)
          0)
        0)

      ; Check for string
      (if (= pattern "Unknown")
        (do
          (if (> (string-length source) 0)
            (do
              (if (= (char-at source 0) 39)
                (do (set pattern "String literal") 0)
                0)
              0)
            0)
          0)
        0)

      ; Check for number
      (if (= pattern "Unknown")
        (do
          (if (> (string-length source) 0)
            (do
              (if (is-digit (char-at source 0))
                (do (set pattern "Number literal") 0)
                0)
              0)
            0)
          0)
        0)

      ; Check for simple identifier
      (if (= pattern "Unknown")
        (do
          (define all-letters 1)
          (define i 0)
          (while (< i (string-length source))
            (do
              (if (is-letter (char-at source i))
                0
                (set all-letters 0))
              (set i (+ i 1))))
          (if all-letters
            (do (set pattern "Variable reference") 0)
            0)
          0)
        0)

      pattern))

  ; ============================================================================
  ; TEST SUITE
  ; ============================================================================
  (c++ "std::cout << \"=== Smalltalk Parser with Typed Strings ===\\n\" << std::endl")

  ; Test 1: Simple identifier
  (c++ "std::cout << \"Test 1: Simple identifier\\n\" << std::endl")
  (define test1 "hello")
  (tokenize-and-display test1)
  (define pattern1 (analyze-syntax test1))
  (c++ "std::cout << \"Pattern: \" << pattern1 << \"\\n\" << std::endl")

  ; Test 2: Keyword message
  (c++ "std::cout << \"Test 2: Keyword message\\n\" << std::endl")
  (define test2 "obj message: 42")
  (tokenize-and-display test2)
  (define pattern2 (analyze-syntax test2))
  (c++ "std::cout << \"Pattern: \" << pattern2 << \"\\n\" << std::endl")

  ; Test 3: Multi-keyword message
  (c++ "std::cout << \"Test 3: Multi-keyword message\\n\" << std::endl")
  (define test3 "obj at: 1 put: 'value'")
  (tokenize-and-display test3)
  (define pattern3 (analyze-syntax test3))
  (c++ "std::cout << \"Pattern: \" << pattern3 << \"\\n\" << std::endl")

  ; Test 4: Assignment
  (c++ "std::cout << \"Test 4: Assignment\\n\" << std::endl")
  (define test4 "x := 42")
  (tokenize-and-display test4)
  (define pattern4 (analyze-syntax test4))
  (c++ "std::cout << \"Pattern: \" << pattern4 << \"\\n\" << std::endl")

  ; Test 5: Symbol literal
  (c++ "std::cout << \"Test 5: Symbol literal\\n\" << std::endl")
  (define test5 "#symbol")
  (tokenize-and-display test5)
  (define pattern5 (analyze-syntax test5))
  (c++ "std::cout << \"Pattern: \" << pattern5 << \"\\n\" << std::endl")

  ; Test 6: String literal
  (c++ "std::cout << \"Test 6: String literal\\n\" << std::endl")
  (define test6 "'hello world'")
  (tokenize-and-display test6)
  (define pattern6 (analyze-syntax test6))
  (c++ "std::cout << \"Pattern: \" << pattern6 << \"\\n\" << std::endl")

  (c++ "std::cout << \"\\n=== Parser Demo Complete! ===\" << std::endl")
  (c++ "std::cout << \"\\nThis demonstrates:\" << std::endl")
  (c++ "std::cout << \"  ✓ Typed string parameters (source string)\" << std::endl")
  (c++ "std::cout << \"  ✓ String operations (substring, char-at, string-length)\" << std::endl")
  (c++ "std::cout << \"  ✓ Type inference for return values\" << std::endl")
  (c++ "std::cout << \"  ✓ Smalltalk tokenization\" << std::endl")
  (c++ "std::cout << \"  ✓ Pattern recognition\" << std::endl")
  (c++ "std::cout << \"  ✓ Complete Lisp-to-C++ transpilation\" << std::endl")

  0)
