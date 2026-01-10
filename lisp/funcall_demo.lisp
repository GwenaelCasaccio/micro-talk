; Demonstration of funcall primitive for dynamic method dispatch
; This shows how funcall enables Smalltalk-style message sending

(do
  ; ============================================================================
  ; Example 1: Basic funcall usage
  ; ============================================================================
  (print-string "=== Example 1: Basic funcall ===")
  (print-string "")

  (define-func (greet name)
    (do
      (print-string "Hello, ")
      (print-string name)
      42))

  (print-string "Direct call: (greet \"World\")")
  (greet "World")

  (print-string "")
  (print-string "Dynamic call: (funcall (function-address greet) \"World\")")
  (funcall (function-address greet) "World")

  (print-string "")
  (print-string "✓ Both produce the same result!")
  (print-string "")

  ; ============================================================================
  ; Example 2: Method lookup simulation
  ; ============================================================================
  (print-string "=== Example 2: Simulated method lookup ===")
  (print-string "")

  ; Define some "methods"
  (define-func (method-add a b)
    (+ a b))

  (define-func (method-mul a b)
    (* a b))

  (define-func (method-sub a b)
    (- a b))

  ; Simulate method dictionary (selector ID -> method address)
  ; In real Smalltalk, this would be a hash table
  ; For demo, we use if-else chain
  (define-func (lookup-method selector-id)
    (if (= selector-id 1)
        (function-address method-add)
        (if (= selector-id 2)
            (function-address method-mul)
            (if (= selector-id 3)
                (function-address method-sub)
                0))))

  ; Simulate message send: receiver selector: arg
  (define-func (send-binary-message receiver selector arg)
    (do
      (define-var method-addr (lookup-method selector))
      (funcall method-addr receiver arg)))

  (print-string "Message sends:")
  (print-string "  15 + 8 = ")
  (print (send-binary-message 15 1 8))  ; selector 1 = add

  (print-string "  15 * 8 = ")
  (print (send-binary-message 15 2 8))  ; selector 2 = mul

  (print-string "  15 - 8 = ")
  (print (send-binary-message 15 3 8))  ; selector 3 = sub

  (print-string "")
  (print-string "✓ Dynamic method dispatch working!")
  (print-string "")

  ; ============================================================================
  ; Example 3: Higher-order functions
  ; ============================================================================
  (print-string "=== Example 3: Higher-order functions ===")
  (print-string "")

  (define-func (apply-twice func arg)
    (funcall func (funcall func arg)))

  (define-func (increment x)
    (+ x 1))

  (define-func (double x)
    (* x 2))

  (print-string "Apply increment twice to 5:")
  (print (apply-twice (function-address increment) 5))

  (print-string "Apply double twice to 3:")
  (print (apply-twice (function-address double) 3))

  (print-string "")
  (print-string "✓ Higher-order functions enabled!")
  (print-string "")

  ; ============================================================================
  ; Example 4: Function table / Jump table pattern
  ; ============================================================================
  (print-string "=== Example 4: Jump table pattern ===")
  (print-string "")

  (define-func (operation-0)
    (print-string "Operation 0: Initialize"))

  (define-func (operation-1)
    (print-string "Operation 1: Process"))

  (define-func (operation-2)
    (print-string "Operation 2: Finalize"))

  ; Dispatch based on opcode
  (define-func (dispatch opcode)
    (if (= opcode 0)
        (funcall (function-address operation-0))
        (if (= opcode 1)
            (funcall (function-address operation-1))
            (if (= opcode 2)
                (funcall (function-address operation-2))
                0))))

  (print-string "Dispatching opcodes:")
  (dispatch 0)
  (dispatch 1)
  (dispatch 2)

  (print-string "")
  (print-string "✓ Jump table pattern working!")
  (print-string "")

  ; ============================================================================
  ; Summary
  ; ============================================================================
  (print-string "=== Summary ===")
  (print-string "")
  (print-string "The funcall primitive enables:")
  (print-string "  ✓ Dynamic method dispatch")
  (print-string "  ✓ Message sending (Smalltalk-style)")
  (print-string "  ✓ Higher-order functions")
  (print-string "  ✓ Jump tables / function dispatch")
  (print-string "  ✓ Polymorphism and dynamic binding")
  (print-string "")
  (print-string "This is the foundation for full Smalltalk message sends!")

  0)
