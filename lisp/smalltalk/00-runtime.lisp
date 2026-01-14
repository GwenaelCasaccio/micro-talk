; ===== Smalltalk Runtime Module =====
; Tagging, memory allocation, object creation

(do
  ; Memory layout constants
  (define-var HEAP_START 268435456)  ; 2GB offset - heap region
  (define-var NULL 0)
  (define-var heap-pointer HEAP_START)

  ; Object header structure
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

  ; Forward declarations for classes (set during bootstrap)
  (define-var ASTNode-class NULL)
  (define-var Array NULL)
  (define-var SmallInteger-class NULL)

  ; ===== Tagged Pointer Operations =====
  ; Lower bit encodes type: 1 = SmallInteger, 0 = OOP

  (define-func (tag-int v) (bit-or (bit-shl v 1) 1))
  (define-func (untag-int t) (bit-ashr t 1))
  (define-func (is-int obj) (= (bit-and obj 1) 1))
  (define-func (is-oop obj) (= (bit-and obj 1) 0))

  ; ===== Object Slot Access =====

  (define-func (slot-at object idx) (peek (+ object OBJECT_HEADER_SIZE idx)))
  (define-func (slot-at-put object idx value) (poke (+ object OBJECT_HEADER_SIZE idx) value))

  (define-func (array-at object idx) (peek (+ object OBJECT_HEADER_SIZE (peek (+ object OBJECT_HEADER_NAMED_SLOTS)) idx)))
  (define-func (array-at-put object idx value)
    (poke (+ object OBJECT_HEADER_SIZE (peek (+ object OBJECT_HEADER_NAMED_SLOTS)) idx) value))

  ; ===== Memory Allocation =====

  (define-func (malloc size)
    (do
      (define-var result heap-pointer)
      ; Round size up to multiple of 8 for proper alignment with tagging
      (define-var remainder (% size 8))
      (define-var aligned-size (if (= remainder 0)
                                   size
                                   (+ size (- 8 remainder))))
      (set heap-pointer (+ heap-pointer aligned-size))
      result))

  ; ===== Object Creation =====

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

  0)
