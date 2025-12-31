; Test file for typed function parameters
; Demonstrates inline type annotation syntax: (define (name (param type)) return-type body)

(do
  ; ============================================================================
  ; Test 1: Simple string parameter function
  ; ============================================================================
  (define (greet (name string)) string
    (string-concat "Hello, " name))

  ; ============================================================================
  ; Test 2: Multiple string parameters
  ; ============================================================================
  (define (full-name (first string) (last string)) string
    (string-concat (string-concat first " ") last))

  ; ============================================================================
  ; Test 3: Mixed parameter types (int and string)
  ; ============================================================================
  (define (repeat-char (ch string) (count int)) string
    (do
      (define result "")
      (define i 0)
      (while (< i count)
        (do
          (set result (string-concat result ch))
          (set i (+ i 1))))
      result))

  ; ============================================================================
  ; Test 4: String parameter with int return type
  ; ============================================================================
  (define (string-is-empty (s string)) int
    (if (= (string-length s) 0) 1 0))

  ; ============================================================================
  ; Test 5: Backward compatibility - untyped function
  ; ============================================================================
  (define (add a b)
    (+ a b))

  ; ============================================================================
  ; Test 6: Function with only parameter types (inferred return)
  ; ============================================================================
  (define (make-greeting (prefix string) (name string))
    (string-concat prefix name))

  ; ============================================================================
  ; MAIN - Test all functions
  ; ============================================================================
  (c++ "std::cout << \"=== Typed Function Tests ===\\n\" << std::endl")

  ; Test 1: greet
  (define greeting (greet "World"))
  (c++ "std::cout << \"Test 1 - greet: \" << greeting << std::endl")

  ; Test 2: full-name
  (define name (full-name "John" "Doe"))
  (c++ "std::cout << \"Test 2 - full-name: \" << name << std::endl")

  ; Test 3: repeat-char
  (define stars (repeat-char "*" 5))
  (c++ "std::cout << \"Test 3 - repeat-char: \" << stars << std::endl")

  ; Test 4: string-is-empty
  (define empty-check1 (string-is-empty ""))
  (define empty-check2 (string-is-empty "hello"))
  (c++ "std::cout << \"Test 4 - string-is-empty empty string: \" << empty_check1 << std::endl")
  (c++ "std::cout << \"Test 4 - string-is-empty 'hello': \" << empty_check2 << std::endl")

  ; Test 5: add (backward compatible)
  (define sum (add 10 20))
  (c++ "std::cout << \"Test 5 - add (untyped): \" << sum << std::endl")

  ; Test 6: make-greeting
  (define hi (make-greeting "Hi, " "Alice"))
  (c++ "std::cout << \"Test 6 - make-greeting: \" << hi << std::endl")

  (c++ "std::cout << \"\\n=== All tests complete! ===\" << std::endl")

  0)
