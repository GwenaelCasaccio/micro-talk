; ===== Smalltalk Runtime Module =====
; Tagging, memory allocation, object creation

(do
  ; Memory layout constants
  (define-var HEAP_START 268435456)  ; 2GB offset - heap region
  (define-var NULL 0)
  (define-var heap-pointer HEAP_START)

  ; ===== Object Header Structure =====
  ; Every object has a 5-word header:
  ;   [0] behavior      - class/behavior pointer
  ;   [1] identity      - unique identity for hash computation
  ;   [2] shape         - indexed slot element type (see SHAPE_* constants)
  ;   [3] named-slots   - count of named instance variables
  ;   [4] indexed-slots - count of indexed slots (arrays, strings, etc.)

  (define-var OBJECT_HEADER_SIZE 5)
  (define-var OBJECT_HEADER_BEHAVIOR 0)
  (define-var OBJECT_HEADER_IDENTITY 1)
  (define-var OBJECT_HEADER_SHAPE 2)
  (define-var OBJECT_HEADER_NAMED_SLOTS 3)
  (define-var OBJECT_HEADER_INDEXED_SLOTS 4)

  ; ===== Shape Constants =====
  ; Defines the element type for indexed slots
  ; Used for arrays, strings, and other variable-size objects

  (define-var SHAPE_NONE 0)    ; No indexed slots (regular object)
  (define-var SHAPE_INT8 1)    ; Signed 8-bit integers (ByteArray signed)
  (define-var SHAPE_UINT8 2)   ; Unsigned 8-bit integers (ByteArray, String)
  (define-var SHAPE_INT16 3)   ; Signed 16-bit integers (ShortArray signed)
  (define-var SHAPE_UINT16 4)  ; Unsigned 16-bit integers (ShortArray)
  (define-var SHAPE_INT32 5)   ; Signed 32-bit integers (IntegerArray signed)
  (define-var SHAPE_UINT32 6)  ; Unsigned 32-bit integers (IntegerArray)
  (define-var SHAPE_OBJECT 7)  ; Object pointers (Array, OrderedCollection)

  ; Global identity counter for generating unique object identities
  (define-var next-identity 1)

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

  ; ===== Object Header Access =====

  (define-func (object-behavior obj) (peek (+ obj OBJECT_HEADER_BEHAVIOR)))
  (define-func (object-behavior-put obj val) (poke (+ obj OBJECT_HEADER_BEHAVIOR) val))

  (define-func (object-identity obj) (peek (+ obj OBJECT_HEADER_IDENTITY)))
  (define-func (object-identity-put obj val) (poke (+ obj OBJECT_HEADER_IDENTITY) val))

  (define-func (object-shape obj) (peek (+ obj OBJECT_HEADER_SHAPE)))
  (define-func (object-shape-put obj val) (poke (+ obj OBJECT_HEADER_SHAPE) val))

  (define-func (object-named-slot-count obj) (peek (+ obj OBJECT_HEADER_NAMED_SLOTS)))
  (define-func (object-indexed-slot-count obj) (peek (+ obj OBJECT_HEADER_INDEXED_SLOTS)))

  ; Identity hash - uses the object's unique identity
  (define-func (identity-hash obj)
    (if (is-int obj)
        (untag-int obj)  ; SmallIntegers hash to themselves
        (object-identity obj)))

  ; ===== Object Slot Access =====

  (define-func (slot-at object idx) (peek (+ object OBJECT_HEADER_SIZE idx)))
  (define-func (slot-at-put object idx value) (poke (+ object OBJECT_HEADER_SIZE idx) value))

  (define-func (array-at object idx) (peek (+ object OBJECT_HEADER_SIZE (peek (+ object OBJECT_HEADER_NAMED_SLOTS)) idx)))
  (define-func (array-at-put object idx value)
    (poke (+ object OBJECT_HEADER_SIZE (peek (+ object OBJECT_HEADER_NAMED_SLOTS)) idx) value))

  ; ===== Byte/Word Access for Different Shapes =====
  ; These functions handle element access based on object shape

  (define-func (indexed-byte-at object idx)
    ; Access a single byte from indexed storage
    ; Used for SHAPE_INT8, SHAPE_UINT8 (strings, byte arrays)
    (do
      (define-var base (+ object OBJECT_HEADER_SIZE (object-named-slot-count object)))
      (define-var word-idx (/ idx 8))
      (define-var byte-idx (% idx 8))
      (define-var word (peek (+ base word-idx)))
      (bit-and (bit-shr word (* byte-idx 8)) 255)))

  (define-func (indexed-byte-at-put object idx value)
    ; Store a single byte in indexed storage
    (do
      (define-var base (+ object OBJECT_HEADER_SIZE (object-named-slot-count object)))
      (define-var word-idx (/ idx 8))
      (define-var byte-idx (% idx 8))
      (define-var word-addr (+ base word-idx))
      (define-var word (peek word-addr))
      (define-var shift (* byte-idx 8))
      (define-var mask (bit-xor -1 (bit-shl 255 shift)))
      (define-var new-word (bit-or (bit-and word mask) (bit-shl (bit-and value 255) shift)))
      (poke word-addr new-word)))

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

  ; Generate a new unique identity
  (define-func (allocate-identity)
    (do
      (define-var id next-identity)
      (set next-identity (+ next-identity 1))
      id))

  ; ===== Object Creation =====

  ; Create a new instance with default shape (SHAPE_NONE for no indexed, SHAPE_OBJECT for indexed)
  (define-func (new-instance behavior named indexed)
    (do
      (define-var shape (if (> indexed 0) SHAPE_OBJECT SHAPE_NONE))
      (new-instance-with-shape behavior named indexed shape)))

  ; Create a new instance with explicit shape
  (define-func (new-instance-with-shape behavior named indexed shape)
    (do
      ; Calculate storage size based on shape
      (define-var indexed-words
        (if (= shape SHAPE_NONE)
            0
        (if (= shape SHAPE_OBJECT)
            indexed  ; One word per object pointer
        (if (< shape SHAPE_INT16)
            ; SHAPE_INT8, SHAPE_UINT8: 8 bytes per word
            (+ (/ indexed 8) (if (> (% indexed 8) 0) 1 0))
        (if (< shape SHAPE_INT32)
            ; SHAPE_INT16, SHAPE_UINT16: 4 elements per word
            (+ (/ indexed 4) (if (> (% indexed 4) 0) 1 0))
            ; SHAPE_INT32, SHAPE_UINT32: 2 elements per word
            (+ (/ indexed 2) (if (> (% indexed 2) 0) 1 0)))))))
      (define-var object-size (+ OBJECT_HEADER_SIZE named indexed-words))
      (define-var object (malloc object-size))
      ; Initialize all slots to NULL/zero
      (for (i 0 object-size)
        (poke (+ object i) NULL))
      ; Set header fields
      (poke (+ object OBJECT_HEADER_BEHAVIOR) behavior)
      (poke (+ object OBJECT_HEADER_IDENTITY) (allocate-identity))
      (poke (+ object OBJECT_HEADER_SHAPE) shape)
      (poke (+ object OBJECT_HEADER_NAMED_SLOTS) named)
      (poke (+ object OBJECT_HEADER_INDEXED_SLOTS) indexed)
      object))

  ; ===== Convenience Constructors =====

  ; Create a byte array (String, ByteArray)
  (define-func (new-byte-array behavior size)
    (new-instance-with-shape behavior 0 size SHAPE_UINT8))

  ; Create an object array (Array, OrderedCollection storage)
  (define-func (new-object-array behavior size)
    (new-instance-with-shape behavior 0 size SHAPE_OBJECT))

  0)
