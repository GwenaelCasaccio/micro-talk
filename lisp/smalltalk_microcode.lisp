; Smalltalk Runtime as Microcode
; These primitives extend the VM with Smalltalk-specific operations
; using opcodes 100-255

(do
  ; Constants
  (define NULL 0)
  (define TAG_INT 1)
  (define TAG_MASK 7)

  ; ============================================================================
  ; TAGGED INTEGER OPERATIONS (Microcode)
  ; ============================================================================

  (defmicro st-tag-int (value)
    (bit-or (bit-shl value 3) TAG_INT))

  (defmicro st-untag-int (tagged)
    (bit-ashr tagged 3))

  (defmicro st-is-int (obj)
    (if (= (bit-and obj TAG_MASK) TAG_INT) 1 0))

  (defmicro st-is-oop (obj)
    (if (= (bit-and obj TAG_MASK) 0) 1 0))

  ; ============================================================================
  ; OBJECT CREATION MICROCODE
  ; ============================================================================

  ; Create a new class object
  ; Class structure: [name, superclass, methods]
  (defmicro st-new-class (name superclass)
    (do
      (define class (malloc 3))
      (poke class name)
      (poke (+ class 1) superclass)
      (poke (+ class 2) NULL)
      class))

  ; Create a new instance
  ; Instance structure: [class, ...]
  (defmicro st-new-instance (class)
    (do
      (define obj (malloc 3))
      (poke obj class)
      obj))

  ; ============================================================================
  ; CLASS/INSTANCE ACCESS MICROCODE
  ; ============================================================================

  (defmicro st-get-class (obj)
    ; For integers, return NULL (will be SmallInteger-class later)
    (if (= (bit-and obj TAG_MASK) TAG_INT)
        NULL
        (peek obj)))

  (defmicro st-get-super (class)
    (peek (+ class 1)))

  (defmicro st-get-methods (class)
    (peek (+ class 2)))

  (defmicro st-set-methods (class methods)
    (do
      (poke (+ class 2) methods)
      methods))

  ; ============================================================================
  ; METHOD DICTIONARY MICROCODE
  ; ============================================================================

  ; Create a method dictionary
  ; Dictionary structure: [size, selector0, code0, selector1, code1, ...]
  (defmicro st-new-method-dict (capacity)
    (do
      (define dict (malloc (+ 1 (* capacity 2))))
      (poke dict (st-tag-int 0))
      dict))

  ; Add a method to the dictionary
  (defmicro st-method-dict-add (dict selector code-addr)
    (do
      (define size (st-untag-int (peek dict)))
      (define entry (+ dict 1 (* size 2)))
      (poke entry selector)
      (poke (+ entry 1) code-addr)
      (poke dict (st-tag-int (+ size 1)))
      dict))

  ; Lookup a method in a single dictionary (no inheritance)
  (defmicro st-method-dict-lookup (dict selector)
    (do
      (if (= dict NULL)
          NULL
          (do
            (define size (st-untag-int (peek dict)))
            (define found NULL)
            (for (i 0 size)
              (do
                (define entry (+ dict 1 (* i 2)))
                (if (= (peek entry) selector)
                    (set found (peek (+ entry 1)))
                    0)))
            found))))

  ; ============================================================================
  ; METHOD LOOKUP WITH INHERITANCE (Microcode)
  ; ============================================================================

  ; Lookup a method starting from receiver's class, walking up inheritance chain
  (defmicro st-lookup-method (receiver selector)
    (do
      (define current-class (st-get-class receiver))
      (define found NULL)

      (while (> current-class NULL)
        (do
          (if (= found NULL)
              (do
                (define methods (st-get-methods current-class))
                (if (> methods NULL)
                    (set found (st-method-dict-lookup methods selector))
                    0)
                (if (= found NULL)
                    (set current-class (st-get-super current-class))
                    (set current-class NULL)))
              (set current-class NULL))))

      found))

  ; ============================================================================
  ; SLOT ACCESS MICROCODE
  ; ============================================================================

  ; Get slot value from instance
  ; Slot 0 is class, so user slots start at offset 1
  (defmicro st-slot-at (obj index)
    (peek (+ obj 1 index)))

  ; Set slot value in instance
  (defmicro st-slot-at-put (obj index value)
    (do
      (poke (+ obj 1 index) value)
      value))

  ; ============================================================================
  ; MEMORY ALLOCATION MICROCODE
  ; ============================================================================

  (defmicro st-malloc (size)
    (malloc size))

  ; Success message
  (print-string "Smalltalk microcode definitions loaded"))
