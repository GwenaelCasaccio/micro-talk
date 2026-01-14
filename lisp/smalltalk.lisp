; ===== Combined Smalltalk Implementation =====
; Generated from modular sources in lisp/smalltalk/
; DO NOT EDIT - edit the source modules instead
;
; Modules:
;   00-runtime.lisp      - Tagging, malloc, object creation
;   01-symbol-table.lisp - Hash table and symbol table
;   02-classes.lisp      - Class hierarchy
;   03-methods.lisp      - Method dictionaries, cache, context
;   04-tokenizer.lisp    - String ops, tokenizer
;   05-parser.lisp       - AST and parser
;   06-compiler.lisp     - Bytecode compiler

(do

  ; ============ 00-runtime.lisp ============
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

  ; ============ 01-symbol-table.lisp ============
  ; ===== Hash Table Implementation =====
  ; Simple hash table with linear probing for fast lookups
  ; Hash function using DJB2 algorithm for strings
  (define-func (hash-string str-addr table-size)
    (do
      (if (= str-addr NULL)
          0
          (do
            (define-var str-len (peek str-addr))
            (define-var hash 5381)  ; DJB2 initial value
            ; Hash each character
            (for (i 0 str-len)
              (do
                ; Get word containing this character
                (define-var word-idx (/ i 8))
                (define-var char-in-word (% i 8))
                (define-var word (peek (+ str-addr 1 word-idx)))
                ; Extract byte at position
                (define-var shift (* char-in-word 8))
                (define-var char-val (bit-and (bit-shr word shift) 255))
                ; hash = hash * 33 + char
                (set hash (+ (* hash 33) char-val))))
            ; Return hash modulo table size
            (% hash table-size)))))
  ; Hash table structure:
  ; Each bucket has 3 slots: [key, value, occupied_flag]
  ; occupied_flag: 0 = empty, 1 = occupied, 2 = deleted
  (define-var HASH_BUCKET_SIZE 3)
  (define-var HASH_KEY_OFFSET 0)
  (define-var HASH_VALUE_OFFSET 1)
  (define-var HASH_OCCUPIED_OFFSET 2)
  (define-var HASH_EMPTY 0)
  (define-var HASH_OCCUPIED 1)
  (define-var HASH_DELETED 2)
  (define-func (new-hash-table capacity)
    ; Create hash table with given capacity
    (do
      (define-var ht (new-instance (tag-int 992) 1 (* capacity HASH_BUCKET_SIZE)))
      (slot-at-put ht 0 (tag-int capacity))
      ; Initialize all buckets to empty
      (for (i 0 (* capacity HASH_BUCKET_SIZE))
        (array-at-put ht i NULL))
      ht))
  (define-func (hash-table-capacity ht)
    (untag-int (slot-at ht 0)))
  (define-func (hash-table-get-bucket ht bucket-idx offset)
    (array-at ht (+ (* bucket-idx HASH_BUCKET_SIZE) offset)))
  (define-func (hash-table-set-bucket ht bucket-idx offset value)
    (array-at-put ht (+ (* bucket-idx HASH_BUCKET_SIZE) offset) value))
  ; ===== Symbol Table for Selectors =====
  ; Maps selector strings to unique IDs for consistent method lookup
  (define-var SYMBOL_TABLE_CAPACITY 1000)
  (define-var SYMBOL_HASH_SIZE 509)  ; Prime number for better distribution
  (define-var symbol-table NULL)
  (define-var symbol-hash NULL)
  (define-var symbol-count 0)
  (define-func (init-symbol-table)
    (do
      (set symbol-table (new-instance (tag-int 991) 0 SYMBOL_TABLE_CAPACITY))
      (set symbol-hash (new-hash-table SYMBOL_HASH_SIZE))
      (set symbol-count 0)
      symbol-table))
  (define-func (string-equal-addr str1-addr str2-addr)
    ; Compare two strings for equality
    (do
      (define-var len1 (peek str1-addr))
      (define-var len2 (peek str2-addr))
      (if (= len1 len2)
          (do
            (define-var equal 1)
            (define-var word-count (+ (/ len1 8) 1))
            (for (i 1 (+ word-count 1))
              (if (= (peek (+ str1-addr i)) (peek (+ str2-addr i)))
                  0
                  (set equal 0)))
            equal)
          0)))
  (define-func (symbol-hash-lookup name-str)
    ; Look up string in hash table
    ; Returns symbol ID (tagged int) if found, 0 if not found
    (do
      (define-var capacity SYMBOL_HASH_SIZE)
      (define-var hash (hash-string name-str capacity))
      (define-var idx hash)
      (define-var found-id 0)
      (define-var probes 0)
      (while (< probes capacity)
        (do
          (define-var occupied (hash-table-get-bucket symbol-hash idx HASH_OCCUPIED_OFFSET))
          (if (= occupied HASH_EMPTY)
              (set probes capacity)
              (do
                (if (= occupied HASH_OCCUPIED)
                    (do
                      (define-var stored-str (hash-table-get-bucket symbol-hash idx HASH_KEY_OFFSET))
                      (if (string-equal-addr name-str stored-str)
                          (do
                            (set found-id (hash-table-get-bucket symbol-hash idx HASH_VALUE_OFFSET))
                            (set probes capacity))
                          0))
                    0)
                (set idx (% (+ idx 1) capacity))
                (set probes (+ probes 1))))))
      found-id))
  (define-func (symbol-hash-insert name-str symbol-id)
    ; Insert string into hash table
    (do
      (define-var capacity SYMBOL_HASH_SIZE)
      (define-var hash (hash-string name-str capacity))
      (define-var idx hash)
      (define-var inserted 0)
      (define-var probes 0)
      (while (< probes capacity)
        (do
          (define-var occupied (hash-table-get-bucket symbol-hash idx HASH_OCCUPIED_OFFSET))
          (if (if (= occupied HASH_EMPTY) 1 (= occupied HASH_DELETED))
              (do
                (hash-table-set-bucket symbol-hash idx HASH_KEY_OFFSET name-str)
                (hash-table-set-bucket symbol-hash idx HASH_VALUE_OFFSET symbol-id)
                (hash-table-set-bucket symbol-hash idx HASH_OCCUPIED_OFFSET HASH_OCCUPIED)
                (set inserted 1)
                (set probes capacity))
              (do
                (set idx (% (+ idx 1) capacity))
                (set probes (+ probes 1))))))
      inserted))
  (define-func (intern-selector name-str)
    ; Look up or create selector ID for given string
    (do
      (if (= symbol-table NULL)
          (init-symbol-table)
          0)
      (define-var found-id (symbol-hash-lookup name-str))
      (if (> found-id 0)
          found-id
          (do
            (if (>= symbol-count SYMBOL_TABLE_CAPACITY)
                (abort "Symbol table full")
                0)
            (array-at-put symbol-table symbol-count name-str)
            (set symbol-count (+ symbol-count 1))
            (define-var new-id (tag-int symbol-count))
            (symbol-hash-insert name-str new-id)
            new-id))))
  (define-func (selector-name selector-id)
    ; Lookup string for a selector ID
    (do
      (if (= symbol-table NULL)
          NULL
          (do
            (define-var id (untag-int selector-id))
            (if (if (> id 0) (<= id symbol-count) 0)
                (array-at symbol-table (- id 1))
                NULL)))))

  ; ============ 02-classes.lisp ============
  ; ===== Class Structure =====
  ; Class has 3 named slots:
  ; - Slot 0: name (tagged int)
  ; - Slot 1: superclass (OOP or NULL)
  ; - Slot 2: method dictionary (OOP or NULL)
  (define-func (new-class name superclass)
    (do
      (define-var class (new-instance (tag-int 987) 3 0))
      (slot-at-put class 0 name)
      (slot-at-put class 1 superclass)
      (slot-at-put class 2 NULL)
      class))
  (define-func (class-set-methods class dict)
    (slot-at-put class 2 dict))
  (define-func (get-class obj)
    (if (is-int obj)
        SmallInteger-class
        (peek obj)))
  (define-func (get-name class) (slot-at class 0))
  (define-func (get-super class) (slot-at class 1))
  (define-func (get-methods class) (slot-at class 2))

  ; ============ 03-methods.lisp ============
  ; ===== Method Dictionary =====
  ; Hash table for O(1) method lookup
  (define-var METHOD_DICT_HASH_SIZE 127)
  (define-func (method-hash-int key table-size)
    ; Hash function for integer keys (selector IDs)
    (do
      (define-var untagged (untag-int key))
      (% (* untagged 2654435761) table-size)))
  (define-func (new-method-dict capacity)
    (do
      (define-var dict (new-instance (tag-int 989) 1 (* METHOD_DICT_HASH_SIZE HASH_BUCKET_SIZE)))
      (slot-at-put dict 0 (tag-int METHOD_DICT_HASH_SIZE))
      (for (i 0 (* METHOD_DICT_HASH_SIZE HASH_BUCKET_SIZE))
        (array-at-put dict i NULL))
      dict))
  (define-func (method-dict-add dict selector code-addr)
    (do
      (define-var capacity (untag-int (slot-at dict 0)))
      (define-var hash (method-hash-int selector capacity))
      (define-var idx hash)
      (define-var inserted 0)
      (define-var probes 0)
      (while (< probes capacity)
        (do
          (define-var bucket-offset (* idx HASH_BUCKET_SIZE))
          (define-var occupied (array-at dict (+ bucket-offset HASH_OCCUPIED_OFFSET)))
          (if (if (= occupied HASH_EMPTY) 1 (= occupied HASH_DELETED))
              (do
                (array-at-put dict (+ bucket-offset HASH_KEY_OFFSET) selector)
                (array-at-put dict (+ bucket-offset HASH_VALUE_OFFSET) code-addr)
                (array-at-put dict (+ bucket-offset HASH_OCCUPIED_OFFSET) HASH_OCCUPIED)
                (set inserted 1)
                (set probes capacity))
              (do
                (if (= occupied HASH_OCCUPIED)
                    (if (= (array-at dict (+ bucket-offset HASH_KEY_OFFSET)) selector)
                        (do
                          (array-at-put dict (+ bucket-offset HASH_VALUE_OFFSET) code-addr)
                          (set inserted 1)
                          (set probes capacity))
                        0)
                    0)
                (set idx (% (+ idx 1) capacity))
                (set probes (+ probes 1))))))
      dict))
  (define-func (method-dict-lookup dict selector)
    (do
      (if (= dict NULL)
          NULL
          (do
            (define-var capacity (untag-int (slot-at dict 0)))
            (define-var hash (method-hash-int selector capacity))
            (define-var idx hash)
            (define-var found NULL)
            (define-var probes 0)
            (while (< probes capacity)
              (do
                (define-var bucket-offset (* idx HASH_BUCKET_SIZE))
                (define-var occupied (array-at dict (+ bucket-offset HASH_OCCUPIED_OFFSET)))
                (if (= occupied HASH_EMPTY)
                    (set probes capacity)
                    (do
                      (if (= occupied HASH_OCCUPIED)
                          (if (= (array-at dict (+ bucket-offset HASH_KEY_OFFSET)) selector)
                              (do
                                (set found (array-at dict (+ bucket-offset HASH_VALUE_OFFSET)))
                                (set probes capacity))
                              0)
                          0)
                      (set idx (% (+ idx 1) capacity))
                      (set probes (+ probes 1))))))
            found))))
  ; ===== Method Lookup =====
  (define-func (lookup-method receiver selector)
    (do
      (define-var current-class (get-class receiver))
      (define-var found NULL)
      (while (> current-class NULL)
        (do
          (if (= found NULL)
              (do
                (define-var methods (get-methods current-class))
                (if (> methods NULL)
                    (set found (method-dict-lookup methods selector))
                    0)
                (if (= found NULL)
                    (set current-class (get-super current-class))
                    (set current-class NULL)))
              (set current-class NULL))))
      found))
  ; ===== Inline Method Cache =====
  ; Monomorphic inline cache for fast method lookup
  (define-var INLINE_CACHE_SIZE 64)
  (define-var INLINE_CACHE_ENTRY_SIZE 3)
  (define-var inline-cache NULL)
  (define-var inline-cache-hits 0)
  (define-var inline-cache-misses 0)
  (define-func (init-inline-cache)
    (do
      (set inline-cache (new-instance (tag-int 993) 0 (* INLINE_CACHE_SIZE INLINE_CACHE_ENTRY_SIZE)))
      (for (i 0 (* INLINE_CACHE_SIZE INLINE_CACHE_ENTRY_SIZE))
        (array-at-put inline-cache i NULL))
      (set inline-cache-hits 0)
      (set inline-cache-misses 0)
      inline-cache))
  (define-func (inline-cache-get-entry cache-id offset)
    (array-at inline-cache (+ (* cache-id INLINE_CACHE_ENTRY_SIZE) offset)))
  (define-func (inline-cache-set-entry cache-id offset value)
    (array-at-put inline-cache (+ (* cache-id INLINE_CACHE_ENTRY_SIZE) offset) value))
  (define-func (lookup-method-cached receiver selector cache-id)
    ; Optimized method lookup with inline caching
    (do
      (if (= inline-cache NULL)
          (init-inline-cache)
          0)
      (define-var receiver-class (get-class receiver))
      (define-var cached-class (inline-cache-get-entry cache-id 0))
      (define-var cached-selector (inline-cache-get-entry cache-id 1))
      (if (if (= cached-class receiver-class) (= cached-selector selector) 0)
          ; Cache hit
          (do
            (set inline-cache-hits (+ inline-cache-hits 1))
            (inline-cache-get-entry cache-id 2))
          ; Cache miss
          (do
            (set inline-cache-misses (+ inline-cache-misses 1))
            (define-var method (lookup-method receiver selector))
            (if (> method NULL)
                (do
                  (inline-cache-set-entry cache-id 0 receiver-class)
                  (inline-cache-set-entry cache-id 1 selector)
                  (inline-cache-set-entry cache-id 2 method))
                0)
            method))))
  (define-func (inline-cache-stats)
    (do
      (print-string "=== Inline Cache Statistics ===")
      (print-string "  Hits:")
      (print-int inline-cache-hits)
      (print-string "  Misses:")
      (print-int inline-cache-misses)
      (if (> inline-cache-hits 0)
          (do
            (define-var total (+ inline-cache-hits inline-cache-misses))
            (define-var hit-rate (/ (* inline-cache-hits 100) total))
            (print-string "  Hit rate:")
            (print-int hit-rate)
            (print-string "%"))
          0)))
  ; ===== Context Management =====
  (define-func (new-context sender receiver method temp-count)
    (do
      (define-var ctx (new-instance (tag-int 999) CONTEXT_NAMED_SLOTS temp-count))
      (slot-at-put ctx CONTEXT_SENDER sender)
      (slot-at-put ctx CONTEXT_RECEIVER receiver)
      (slot-at-put ctx CONTEXT_METHOD method)
      (slot-at-put ctx CONTEXT_PC (tag-int 0))
      (slot-at-put ctx CONTEXT_SP (tag-int 0))
      ctx))
  (define-func (context-get-sender ctx) (slot-at ctx CONTEXT_SENDER))
  (define-func (context-get-receiver ctx) (slot-at ctx CONTEXT_RECEIVER))
  (define-func (context-get-method ctx) (slot-at ctx CONTEXT_METHOD))
  (define-func (context-get-pc ctx) (untag-int (slot-at ctx CONTEXT_PC)))
  (define-func (context-get-sp ctx) (untag-int (slot-at ctx CONTEXT_SP)))
  (define-func (context-set-pc ctx value) (slot-at-put ctx CONTEXT_PC (tag-int value)))
  (define-func (context-set-sp ctx value) (slot-at-put ctx CONTEXT_SP (tag-int value)))
  (define-func (context-temp-at ctx idx) (array-at ctx idx))
  (define-func (context-temp-at-put ctx idx value) (array-at-put ctx idx value))
  ; ===== Test Framework =====
  (define-func (assert-equal actual expected msg)
    (if (= actual expected)
        1
        (abort msg)))
  (define-func (assert-true cond msg)
    (if cond
        1
        (abort msg)))
  ; ===== Message Send =====
  (define-func (message-send receiver selector args temp-count)
    (do
      (define-var method (lookup-method receiver selector))
      (if (= method NULL)
          (do
            (print-string "ERROR: Method not found")
            (print-int (untag-int selector))
            NULL)
          (do
            (define-var new-ctx (new-context current-context receiver method temp-count))
            (define-var arg-count (untag-int (peek args)))
            (for (i 0 arg-count)
              (context-temp-at-put new-ctx i (peek (+ args 1 i))))
            (set current-context new-ctx)
            new-ctx))))
  (define-func (method-return return-value)
    (do
      (if (= current-context NULL)
          (do
            (print-string "ERROR: Cannot return, no active context")
            NULL)
          (do
            (define-var sender (context-get-sender current-context))
            (set current-context sender)
            return-value))))

  ; ============ 04-tokenizer.lisp ============
  ; ===== String Operations =====
  ; Strings are stored as: [length][packed chars (8 per word)]
  (define-func (string-length str)
    (peek str))
  (define-func (string-char-at str idx)
    (do
      (define-var word-idx (+ 1 (/ idx 8)))
      (define-var byte-idx (% idx 8))
      (define-var word (peek (+ str word-idx)))
      (bit-and (bit-shr word (* byte-idx 8)) 255)))
  (define-func (string-equal str1 str2)
    (do
      (define-var len1 (string-length str1))
      (define-var len2 (string-length str2))
      (if (= len1 len2)
          (do
            (define-var equal 1)
            (for (i 0 len1)
              (if (= (string-char-at str1 i) (string-char-at str2 i))
                  0
                  (set equal 0)))
            equal)
          0)))
  ; ===== Character Classification =====
  (define-func (is-digit char)
    ; Check if char is '0'-'9' (48-57)
    (if (>= char 48)
        (if (<= char 57) 1 0)
        0))
  (define-func (is-letter char)
    ; Check if char is a-z (97-122) or A-Z (65-90)
    (if (>= char 65)
        (if (<= char 90)
            1
            (if (>= char 97)
                (if (<= char 122) 1 0)
                0))
        0))
  (define-func (is-whitespace char)
    ; space=32, tab=9, newline=10, return=13
    (if (= char 32) 1
    (if (= char 9) 1
    (if (= char 10) 1
    (if (= char 13) 1 0)))))
  ; ===== Token Types =====
  (define-var TOK_NUMBER 1)
  (define-var TOK_IDENTIFIER 2)
  (define-var TOK_KEYWORD 3)
  (define-var TOK_STRING 4)
  (define-var TOK_BINARY_OP 5)
  (define-var TOK_SPECIAL 6)
  (define-var TOK_EOF 99)
  ; Token class (set during bootstrap)
  (define-var Token-class NULL)
  ; Tokenizer state
  (define-var tok-source NULL)
  (define-var tok-pos 0)
  (define-var tok-length 0)
  ; Token factory
  (define-func (new-token type value start end)
    (do
      (define-var tok (new-instance Token-class 4 0))
      (slot-at-put tok 0 (tag-int type))
      (slot-at-put tok 1 value)
      (slot-at-put tok 2 (tag-int start))
      (slot-at-put tok 3 (tag-int end))
      tok))
  ; Token accessors
  (define-func (token-type tok) (untag-int (slot-at tok 0)))
  (define-func (token-value tok) (slot-at tok 1))
  (define-func (token-start tok) (untag-int (slot-at tok 2)))
  (define-func (token-end tok) (untag-int (slot-at tok 3)))
  ; Tokenizer helpers
  (define-func (tok-peek)
    (if (< tok-pos tok-length)
        (string-char-at tok-source tok-pos)
        0))
  (define-func (tok-advance)
    (set tok-pos (+ tok-pos 1)))
  (define-func (tok-skip-whitespace)
    (while (is-whitespace (tok-peek))
      (tok-advance)))
  (define-func (tok-read-number)
    (do
      (define-var num 0)
      (while (is-digit (tok-peek))
        (do
          (define-var digit (- (tok-peek) 48))
          (set num (+ (* num 10) digit))
          (tok-advance)))
      (tag-int num)))
  (define-func (tok-read-identifier)
    (do
      (define-var start-pos tok-pos)
      (while (if (is-letter (tok-peek))
                 1
                 (is-digit (tok-peek)))
        (tok-advance))
      (tag-int start-pos)))
  (define-func (is-binary-op char)
    ; Check if char is a binary operator: + - * / < > =
    (if (= char 43) 1  ; +
    (if (= char 45) 1  ; -
    (if (= char 42) 1  ; *
    (if (= char 47) 1  ; /
    (if (= char 60) 1  ; <
    (if (= char 62) 1  ; >
    (if (= char 61) 1  ; =
        0))))))))
  (define-func (tok-next-token)
    (do
      (tok-skip-whitespace)
      (define-var value NULL)
      (define-var start tok-pos)
      (define-var c (tok-peek))
      (if (= c 0)
          (new-token TOK_EOF NULL start start)
      (if (is-digit c)
          (do
            (set value (tok-read-number))
            (new-token TOK_NUMBER value start tok-pos))
      (if (is-letter c)
          (do
            (set value (tok-read-identifier))
            (if (= (tok-peek) 58)  ; ':'
                (do
                  (tok-advance)
                  (new-token TOK_KEYWORD value start tok-pos))
                (new-token TOK_IDENTIFIER value start tok-pos)))
      (if (is-binary-op c)
          (do
            (tok-advance)
            (new-token TOK_BINARY_OP (tag-int c) start tok-pos))
          (new-token TOK_EOF NULL start start)))))))
  (define-func (tokenize source)
    (do
      (set tok-source source)
      (set tok-pos 0)
      (set tok-length (string-length source))
      (define-var max-tokens (+ tok-length 1))
      (define-var tokens (new-instance Array 0 max-tokens))
      (define-var count 0)
      (define-var tok (tok-next-token))
      (while (< (token-type tok) TOK_EOF)
        (do
          (array-at-put tokens count tok)
          (set count (+ count 1))
          (set tok (tok-next-token))))
      (array-at-put tokens count tok)
      tokens))

  ; ============ 05-parser.lisp ============
  ; ===== AST Node Types =====
  (define-var AST_NUMBER 1)
  (define-var AST_IDENTIFIER 2)
  (define-var AST_STRING 3)
  (define-var AST_UNARY_MSG 4)
  (define-var AST_BINARY_MSG 5)
  (define-var AST_KEYWORD_MSG 6)
  (define-var AST_BLOCK 7)
  (define-var AST_ASSIGNMENT 8)
  (define-var AST_CASCADE 9)
  (define-var AST_RETURN 10)
  ; Factory function to create AST nodes
  (define-func (new-ast-node type value child-count)
    (do
      (define-var node (new-instance ASTNode-class 2 child-count))
      (slot-at-put node 0 (tag-int type))
      (slot-at-put node 1 value)
      node))
  ; AST node accessors
  (define-func (ast-type node) (untag-int (slot-at node 0)))
  (define-func (ast-value node) (slot-at node 1))
  (define-func (ast-child node idx) (array-at node idx))
  (define-func (ast-child-put node idx child) (array-at-put node idx child))
  (define-func (ast-child-count node) (peek (+ node OBJECT_HEADER_INDEXED_SLOTS)))
  ; ===== Parser State =====
  (define-var parse-tokens NULL)
  (define-var parse-pos 0)
  (define-var parse-length 0)
  ; Parser navigation
  (define-func (parse-peek)
    (if (< parse-pos parse-length)
        (array-at parse-tokens parse-pos)
        (new-token TOK_EOF NULL 0 0)))
  (define-func (parse-advance)
    (set parse-pos (+ parse-pos 1)))
  (define-func (parse-token-type)
    (token-type (parse-peek)))
  (define-func (parse-token-value)
    (token-value (parse-peek)))
  ; ===== Parser Rules =====
  ; parse-primary: numbers and identifiers (highest precedence)
  (define-func (parse-primary)
    (do
      (define-var type (parse-token-type))
      (define-var value NULL)
      (if (= type TOK_NUMBER)
          (do
            (set value (parse-token-value))
            (parse-advance)
            (new-ast-node AST_NUMBER value 0))
      (if (= type TOK_IDENTIFIER)
          (do
            (set value (parse-token-value))
            (parse-advance)
            (new-ast-node AST_IDENTIFIER value 0))
          (abort "Unexpected token in parse-primary")))))
  ; parse-unary-message: unary selectors (e.g., obj method)
  (define-func (parse-unary-message)
    (do
      (define-var receiver (parse-primary))
      (define-var check-unary (parse-token-type))
      (while (= check-unary TOK_IDENTIFIER)
        (do
          (define-var selector (parse-token-value))
          (parse-advance)
          (define-var node (new-ast-node AST_UNARY_MSG selector 1))
          (ast-child-put node 0 receiver)
          (set receiver node)
          (set check-unary (parse-token-type))))
      receiver))
  ; parse-binary-message: binary operators (e.g., 3 + 4)
  (define-func (parse-binary-message)
    (do
      (define-var left (parse-unary-message))
      (define-var check-type (parse-token-type))
      (while (= check-type TOK_BINARY_OP)
        (do
          (define-var op (parse-token-value))
          (parse-advance)
          (define-var right (parse-unary-message))
          (define-var node (new-ast-node AST_BINARY_MSG op 2))
          (ast-child-put node 0 left)
          (ast-child-put node 1 right)
          (set left node)
          (set check-type (parse-token-type))))
      left))
  ; parse-keyword-message: keyword messages (e.g., obj at: 1 put: 2)
  (define-func (parse-keyword-message)
    (do
      (define-var receiver (parse-binary-message))
      (define-var check-keyword (parse-token-type))
      (if (= check-keyword TOK_KEYWORD)
          (do
            (define-var args-temp (malloc 10))
            (define-var keywords-temp (malloc 10))
            (define-var arg-count 0)
            (while (= check-keyword TOK_KEYWORD)
              (do
                (poke (+ keywords-temp arg-count) (parse-token-value))
                (parse-advance)
                (poke (+ args-temp arg-count) (parse-binary-message))
                (set arg-count (+ arg-count 1))
                (set check-keyword (parse-token-type))))
            (define-var total-children (+ (+ arg-count 1) arg-count))
            (define-var node (new-ast-node AST_KEYWORD_MSG (tag-int arg-count) total-children))
            (ast-child-put node 0 receiver)
            (for (i 0 arg-count)
              (ast-child-put node (+ i 1) (peek (+ args-temp i))))
            (for (i 0 arg-count)
              (ast-child-put node (+ (+ arg-count 1) i) (peek (+ keywords-temp i))))
            node)
          receiver)))
  ; parse-expression: top-level entry point
  (define-func (parse-expression)
    (parse-keyword-message))
  ; parse: main entry point
  (define-func (parse source-string)
    (do
      (define-var token-array (tokenize source-string))
      (set parse-tokens token-array)
      (set parse-pos 0)
      (define-var len 0)
      (while (< (token-type (array-at token-array len)) TOK_EOF)
        (set len (+ len 1)))
      (set parse-length len)
      (parse-expression)))

  ; ============ 06-compiler.lisp ============
  ; ===== VM Opcode Constants =====
  (define-var OP_HALT 0)
  (define-var OP_PUSH 1)
  (define-var OP_POP 2)
  (define-var OP_DUP 3)
  (define-var OP_SWAP 4)
  (define-var OP_ADD 5)
  (define-var OP_SUB 6)
  (define-var OP_MUL 7)
  (define-var OP_DIV 8)
  (define-var OP_MOD 9)
  (define-var OP_EQ 10)
  (define-var OP_LT 11)
  (define-var OP_GT 12)
  (define-var OP_LTE 13)
  (define-var OP_GTE 14)
  (define-var OP_JMP 15)
  (define-var OP_JZ 16)
  (define-var OP_ENTER 17)
  (define-var OP_LEAVE 18)
  (define-var OP_CALL 19)
  (define-var OP_RET 20)
  (define-var OP_IRET 21)
  (define-var OP_LOAD 22)
  (define-var OP_STORE 23)
  (define-var OP_BP_LOAD 24)
  (define-var OP_BP_STORE 25)
  (define-var OP_PRINT 26)
  (define-var OP_PRINT_STR 27)
  (define-var OP_AND 28)
  (define-var OP_OR 29)
  (define-var OP_XOR 30)
  (define-var OP_SHL 31)
  (define-var OP_SHR 32)
  (define-var OP_ASHR 33)
  (define-var OP_CLI 34)
  (define-var OP_STI 35)
  (define-var OP_SIGNAL_REG 36)
  (define-var OP_ABORT 37)
  (define-var OP_FUNCALL 38)
  ; ===== Bytecode Buffer =====
  (define-var bytecode-buffer NULL)
  (define-var bytecode-pos 0)
  ; Helper function addresses (filled in during bootstrap)
  (define-var send-unary-addr NULL)
  (define-var lookup-method-addr NULL)
  ; Source string for selector extraction
  (define-var compile-source-string NULL)
  ; ===== Selector Interning =====
  (define-func (intern-identifier-at-pos pos)
    ; Extract identifier string from source at position and intern it
    (do
      (define-var start pos)
      (define-var end pos)
      (define-var src-len (string-length compile-source-string))
      ; Check if this is an operator
      (define-var first-char (string-char-at compile-source-string pos))
      (define-var is-operator (if (= first-char 43) 1   ; +
                               (if (= first-char 45) 1   ; -
                               (if (= first-char 42) 1   ; *
                               (if (= first-char 47) 1   ; /
                               (if (= first-char 60) 1   ; <
                               (if (= first-char 62) 1   ; >
                               (if (= first-char 61) 1   ; =
                                   0))))))))
      (if is-operator
          (set end (+ pos 1))
          (while (< end src-len)
            (do
              (define-var ch (string-char-at compile-source-string end))
              (if (if (is-letter ch) 1 (is-digit ch))
                  (set end (+ end 1))
                  (set end src-len)))))
      ; Create string with extracted identifier
      (define-var id-len (- end start))
      (define-var id-str (malloc (+ (/ id-len 8) 2)))
      (poke id-str id-len)
      (define-var word-count (+ (/ id-len 8) 1))
      (for (word-idx 0 word-count)
        (do
          (define-var word 0)
          (for (byte-idx 0 8)
            (do
              (define-var char-idx (+ (* word-idx 8) byte-idx))
              (if (< char-idx id-len)
                  (do
                    (define-var ch (string-char-at compile-source-string (+ start char-idx)))
                    (set word (bit-or word (bit-shl ch (* byte-idx 8)))))
                  0)))
          (poke (+ id-str 1 word-idx) word)))
      (intern-selector id-str)))
  (define-func (build-keyword-selector ast)
    ; Build keyword selector from AST (e.g., "at:put:")
    (do
      (define-var num-args (untag-int (ast-value ast)))
      (define-var total-len 0)
      ; First pass: calculate total length
      (for (i 0 num-args)
        (do
          (define-var kw-pos (untag-int (ast-child ast (+ (+ num-args 1) i))))
          (define-var start kw-pos)
          (define-var end kw-pos)
          (define-var src-len (string-length compile-source-string))
          (while (< end src-len)
            (do
              (define-var ch (string-char-at compile-source-string end))
              (if (= ch 58)  ; ':'
                  (do
                    (set end (+ end 1))
                    (set end src-len))
                  (if (if (is-letter ch) 1 (is-digit ch))
                      (set end (+ end 1))
                      (set end src-len)))))
          (set total-len (+ total-len (- end start)))))
      ; Allocate string
      (define-var sel-str (malloc (+ (/ total-len 8) 2)))
      (poke sel-str total-len)
      ; Second pass: copy keyword parts
      (define-var dest-pos 0)
      (for (i 0 num-args)
        (do
          (define-var kw-pos (untag-int (ast-child ast (+ (+ num-args 1) i))))
          (define-var start kw-pos)
          (define-var end kw-pos)
          (define-var src-len (string-length compile-source-string))
          (while (< end src-len)
            (do
              (define-var ch (string-char-at compile-source-string end))
              (if (= ch 58)
                  (do
                    (set end (+ end 1))
                    (set end src-len))
                  (if (if (is-letter ch) 1 (is-digit ch))
                      (set end (+ end 1))
                      (set end src-len)))))
          (define-var kw-len (- end start))
          (for (j 0 kw-len)
            (do
              (define-var ch (string-char-at compile-source-string (+ start j)))
              (define-var word-idx (/ dest-pos 8))
              (define-var byte-idx (% dest-pos 8))
              (define-var current-word (peek (+ sel-str 1 word-idx)))
              (define-var new-word (bit-or current-word (bit-shl ch (* byte-idx 8))))
              (poke (+ sel-str 1 word-idx) new-word)
              (set dest-pos (+ dest-pos 1))))))
      (intern-selector sel-str)))
  ; ===== Bytecode Emission =====
  (define-func (init-bytecode max-size)
    (do
      (set bytecode-buffer heap-pointer)
      (set bytecode-pos 0)
      (set heap-pointer (+ heap-pointer max-size))
      bytecode-buffer))
  (define-func (emit word)
    (do
      (poke (+ bytecode-buffer bytecode-pos) word)
      (set bytecode-pos (+ bytecode-pos 1))))
  (define-func (current-address)
    (+ bytecode-buffer bytecode-pos))
  ; ===== AST Compilation =====
  (define-func (compile-st-expr ast)
    (do
      (define-var type (ast-type ast))
      (if (= type AST_NUMBER)
          (do
            (emit OP_PUSH)
            (emit (ast-value ast)))
      (if (= type AST_IDENTIFIER)
          (do
            (emit OP_PUSH)
            (emit (tag-int 0)))
      (if (= type AST_UNARY_MSG)
          ; Unary message: receiver selector
          (do
            (define-var selector-pos (untag-int (ast-value ast)))
            (define-var selector-id (intern-identifier-at-pos selector-pos))
            (compile-st-expr (ast-child ast 0))
            (emit OP_DUP)
            (emit OP_PUSH)
            (emit selector-id)
            (emit OP_PUSH)
            (emit lookup-method-addr)
            (emit OP_PUSH)
            (emit 2)
            (emit OP_FUNCALL)
            (emit OP_PUSH)
            (emit 1)
            (emit OP_FUNCALL)
            0)
      (if (= type AST_KEYWORD_MSG)
          ; Keyword message: receiver selector: arg1 keyword2: arg2 ...
          (do
            (define-var keyword-sel-id (build-keyword-selector ast))
            (define-var num-args (untag-int (ast-value ast)))
            (for (i 0 num-args)
              (compile-st-expr (ast-child ast (+ i 1))))
            (compile-st-expr (ast-child ast 0))
            (emit OP_DUP)
            (emit OP_PUSH)
            (emit keyword-sel-id)
            (emit OP_PUSH)
            (emit lookup-method-addr)
            (emit OP_PUSH)
            (emit 2)
            (emit OP_FUNCALL)
            (emit OP_PUSH)
            (emit (+ num-args 1))
            (emit OP_FUNCALL)
            0)
      (if (= type AST_BINARY_MSG)
          ; Binary message: receiver op argument
          (do
            (define-var op-char (untag-int (ast-value ast)))
            (define-var op-str (malloc 2))
            (poke op-str 1)
            (poke (+ op-str 1) op-char)
            (define-var op-sel-id (intern-selector op-str))
            (compile-st-expr (ast-child ast 0))
            (emit OP_DUP)
            (emit OP_PUSH)
            (emit op-sel-id)
            (emit OP_PUSH)
            (emit lookup-method-addr)
            (emit OP_PUSH)
            (emit 2)
            (emit OP_FUNCALL)
            (compile-st-expr (ast-child ast 1))
            (emit OP_SWAP)
            (emit OP_PUSH)
            (emit 2)
            (emit OP_FUNCALL)
            0)
          (abort "Unknown AST node type in compile"))))))))
  ; ===== Public Compiler API =====
  (define-func (compile-smalltalk source-string)
    ; Compile Smalltalk source to bytecode, return start address
    (do
      (set compile-source-string source-string)
      (init-bytecode 1000)
      (define-var ast (parse source-string))
      (compile-st-expr ast)
      (emit OP_HALT)
      bytecode-buffer))
  (define-func (compile-method source-string arg-count)
    ; Compile a method body (ends with RET instead of HALT)
    (do
      (set compile-source-string source-string)
      (init-bytecode 1000)
      (define-var ast (parse source-string))
      (compile-st-expr ast)
      (emit OP_RET)
      (emit arg-count)
      bytecode-buffer))
  (define-func (install-method class selector source arg-count)
    ; Install a compiled method into a class
    (do
      (define-var method-addr (compile-method source arg-count))
      (define-var methods (get-methods class))
      (if (= methods NULL)
          (do
            (define-var new-dict (new-method-dict 10))
            (class-set-methods class new-dict)
            (set methods new-dict))
          0)
      (method-dict-add methods selector method-addr)
      method-addr))

  0)
