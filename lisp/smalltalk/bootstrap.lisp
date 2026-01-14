(do
  (define-var HEAP_START 268435456)  ; 2GB offset - heap region (globals in 1GB region, heap in 2GB region)
  (define-var NULL 0)
  (define-var heap-pointer HEAP_START)
 
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

  ; AST node class (created in bootstrap after Object)
  (define-var ASTNode-class NULL)
  (define-var Array NULL)

  (define-func (tag-int v) (bit-or (bit-shl v 1) 1))
  (define-func (untag-int t) (bit-ashr t 1))
  (define-func (is-int obj) (= (bit-and obj 1) 1))
  (define-func (is-oop obj) (= (bit-and obj 1) 0))

  (define-func (slot-at object idx) (peek (+ object OBJECT_HEADER_SIZE idx)))
  (define-func (slot-at-put object idx value) (poke (+ object OBJECT_HEADER_SIZE idx) value))

  (define-func (array-at object idx) (peek (+ object OBJECT_HEADER_SIZE (peek (+ object OBJECT_HEADER_NAMED_SLOTS)) idx)))
  (define-func (array-at-put object idx value)
    (poke (+ object OBJECT_HEADER_SIZE (peek (+ object OBJECT_HEADER_NAMED_SLOTS)) idx) value))

  (define-func (malloc size)
    (do
      (define-var result heap-pointer)
      ; Round size up to multiple of 8 for proper alignment with 3-bit tagging
      ; With 3-bit tags, addresses must be 8-byte aligned (last 3 bits = 000)
      ; This ensures OOPs have xxx...xxx000 format, leaving 3 bits for tags
      (define-var remainder (% size 8))
      (define-var aligned-size (if (= remainder 0)
                                   size
                                   (+ size (- 8 remainder))))
      (set heap-pointer (+ heap-pointer aligned-size))
      result))

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

  ; ===== Hash Table Implementation =====
  ; Simple hash table with linear probing for fast lookups

  ; Hash function using DJB2 algorithm for strings
  ; Returns hash value modulo table size
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
    ; Returns object with capacity * HASH_BUCKET_SIZE indexed slots
    (do
      (define-var ht (new-instance (tag-int 992) 1 (* capacity HASH_BUCKET_SIZE)))
      (slot-at-put ht 0 (tag-int capacity))  ; Store capacity

      ; Initialize all buckets to empty
      (for (i 0 (* capacity HASH_BUCKET_SIZE))
        (array-at-put ht i NULL))

      ht))

  (define-func (hash-table-capacity ht)
    (untag-int (slot-at ht 0)))

  (define-func (hash-table-get-bucket ht bucket-idx offset)
    ; Get field from bucket
    (array-at ht (+ (* bucket-idx HASH_BUCKET_SIZE) offset)))

  (define-func (hash-table-set-bucket ht bucket-idx offset value)
    ; Set field in bucket
    (array-at-put ht (+ (* bucket-idx HASH_BUCKET_SIZE) offset) value))

  ; Note: Generic hash-table-lookup and hash-table-insert removed
  ; Each use case (symbol table, method dictionary) has specialized versions
  ; that don't require passing functions as parameters

  ; ===== Symbol Table for Selectors =====
  ; Maps selector strings to unique IDs for consistent method lookup
  ; Now using hash table for O(1) lookups instead of O(n) linear search

  (define-var SYMBOL_TABLE_CAPACITY 1000)
  (define-var SYMBOL_HASH_SIZE 509)  ; Prime number for better distribution
  (define-var symbol-table NULL)
  (define-var symbol-hash NULL)  ; Hash table for fast string lookup
  (define-var symbol-count 0)

  (define-func (init-symbol-table)
    (do
      ; Create array to hold selector strings
      ; Each entry is a string address (or NULL if unused)
      (set symbol-table (new-instance (tag-int 991) 0 SYMBOL_TABLE_CAPACITY))
      (set symbol-hash (new-hash-table SYMBOL_HASH_SIZE))
      (set symbol-count 0)
      symbol-table))

  (define-func (string-equal-addr str1-addr str2-addr)
    ; Compare two strings for equality
    (do
      (define-var len1 (peek str1-addr))
      (define-var len2 (peek str2-addr))
      ; First check: lengths must match
      (if (= len1 len2)
          (do
            (define-var equal 1)
            ; Calculate number of data words (excluding length word)
            (define-var word-count (+ (/ len1 8) 1))
            ; Compare data words starting from index 1 (skip length at index 0)
            (for (i 1 (+ word-count 1))
              (if (= (peek (+ str1-addr i)) (peek (+ str2-addr i)))
                  0
                  (set equal 0)))
            equal)
          0)))

  (define-func (symbol-hash-lookup name-str)
    ; Look up string in hash table using content comparison
    ; Returns symbol ID (tagged int) if found, 0 if not found
    (do
      (define-var capacity SYMBOL_HASH_SIZE)
      (define-var hash (hash-string name-str capacity))
      (define-var idx hash)
      (define-var found-id 0)
      (define-var probes 0)

      ; Linear probing with string content comparison
      (while (< probes capacity)
        (do
          (define-var occupied (hash-table-get-bucket symbol-hash idx HASH_OCCUPIED_OFFSET))

          (if (= occupied HASH_EMPTY)
              ; Empty slot - not found
              (set probes capacity)
              (do
                (if (= occupied HASH_OCCUPIED)
                    ; Check if string content matches
                    (do
                      (define-var stored-str (hash-table-get-bucket symbol-hash idx HASH_KEY_OFFSET))
                      (if (string-equal-addr name-str stored-str)
                          (do
                            (set found-id (hash-table-get-bucket symbol-hash idx HASH_VALUE_OFFSET))
                            (set probes capacity))
                          0))
                    0)

                ; Move to next bucket
                (set idx (% (+ idx 1) capacity))
                (set probes (+ probes 1))))))

      found-id))

  (define-func (symbol-hash-insert name-str symbol-id)
    ; Insert string into hash table
    ; Returns 1 on success, 0 if table is full
    (do
      (define-var capacity SYMBOL_HASH_SIZE)
      (define-var hash (hash-string name-str capacity))
      (define-var idx hash)
      (define-var inserted 0)
      (define-var probes 0)

      ; Linear probing to find empty or deleted slot
      (while (< probes capacity)
        (do
          (define-var occupied (hash-table-get-bucket symbol-hash idx HASH_OCCUPIED_OFFSET))

          (if (if (= occupied HASH_EMPTY) 1 (= occupied HASH_DELETED))
              ; Found slot - insert here
              (do
                (hash-table-set-bucket symbol-hash idx HASH_KEY_OFFSET name-str)
                (hash-table-set-bucket symbol-hash idx HASH_VALUE_OFFSET symbol-id)
                (hash-table-set-bucket symbol-hash idx HASH_OCCUPIED_OFFSET HASH_OCCUPIED)
                (set inserted 1)
                (set probes capacity))
              (do
                ; Move to next bucket
                (set idx (% (+ idx 1) capacity))
                (set probes (+ probes 1))))))

      inserted))

  (define-func (intern-selector name-str)
    ; Look up or create selector ID for given string
    ; name-str: address of string in memory
    ; Returns: tagged integer ID (1-based)
    (do
      (if (= symbol-table NULL)
          (init-symbol-table)
          0)

      ; Search for existing selector using hash table
      (define-var found-id (symbol-hash-lookup name-str))

      ; If found, return existing ID
      (if (> found-id 0)
          found-id
          (do
            ; Not found, create new entry
            (if (>= symbol-count SYMBOL_TABLE_CAPACITY)
                (abort "Symbol table full")
                0)

            ; Add to symbol table array
            (array-at-put symbol-table symbol-count name-str)
            (set symbol-count (+ symbol-count 1))

            ; Add to hash table for fast lookup
            (define-var new-id (tag-int symbol-count))
            (symbol-hash-insert name-str new-id)

            new-id))))

  (define-func (selector-name selector-id)
    ; Lookup string for a selector ID
    ; Returns: string address or NULL if not found
    (do
      (if (= symbol-table NULL)
          NULL
          (do
            (define-var id (untag-int selector-id))
            (if (if (> id 0) (<= id symbol-count) 0)
                (array-at symbol-table (- id 1))
                NULL)))))

  ; ===== Method Dictionary =====
  ; Now using hash table for O(1) method lookup instead of O(n) linear search

  (define-var METHOD_DICT_HASH_SIZE 127)  ; Prime number for method dictionary

  (define-func (method-hash-int key table-size)
    ; Simple hash function for integer keys (selector IDs)
    ; Using multiplicative hashing
    (do
      (define-var untagged (untag-int key))
      (% (* untagged 2654435761) table-size)))  ; Knuth's multiplicative hash constant

  (define-func (new-method-dict capacity)
    ; Create method dictionary with hash table
    ; Structure: hash table stored in indexed slots
    (do
      ; Use METHOD_DICT_HASH_SIZE for hash table
      (define-var dict (new-instance (tag-int 989) 1 (* METHOD_DICT_HASH_SIZE HASH_BUCKET_SIZE)))
      (slot-at-put dict 0 (tag-int METHOD_DICT_HASH_SIZE))  ; Store hash size

      ; Initialize all buckets to empty
      (for (i 0 (* METHOD_DICT_HASH_SIZE HASH_BUCKET_SIZE))
        (array-at-put dict i NULL))

      dict))

  (define-func (method-dict-add dict selector code-addr)
    ; Add method to dictionary using hash table
    (do
      (define-var capacity (untag-int (slot-at dict 0)))
      (define-var hash (method-hash-int selector capacity))
      (define-var idx hash)
      (define-var inserted 0)
      (define-var probes 0)

      ; Linear probing to find empty or matching slot
      (while (< probes capacity)
        (do
          (define-var bucket-offset (* idx HASH_BUCKET_SIZE))
          (define-var occupied (array-at dict (+ bucket-offset HASH_OCCUPIED_OFFSET)))

          (if (if (= occupied HASH_EMPTY) 1 (= occupied HASH_DELETED))
              ; Found empty slot - insert here
              (do
                (array-at-put dict (+ bucket-offset HASH_KEY_OFFSET) selector)
                (array-at-put dict (+ bucket-offset HASH_VALUE_OFFSET) code-addr)
                (array-at-put dict (+ bucket-offset HASH_OCCUPIED_OFFSET) HASH_OCCUPIED)
                (set inserted 1)
                (set probes capacity))
              (do
                ; Check if key already exists (update case)
                (if (= occupied HASH_OCCUPIED)
                    (if (= (array-at dict (+ bucket-offset HASH_KEY_OFFSET)) selector)
                        ; Update existing entry
                        (do
                          (array-at-put dict (+ bucket-offset HASH_VALUE_OFFSET) code-addr)
                          (set inserted 1)
                          (set probes capacity))
                        0)
                    0)

                ; Move to next bucket
                (set idx (% (+ idx 1) capacity))
                (set probes (+ probes 1))))))

      dict))

  (define-func (method-dict-lookup dict selector)
    ; Look up method in dictionary using hash table
    (do
      (if (= dict NULL)
          NULL
          (do
            (define-var capacity (untag-int (slot-at dict 0)))
            (define-var hash (method-hash-int selector capacity))
            (define-var idx hash)
            (define-var found NULL)
            (define-var probes 0)

            ; Linear probing
            (while (< probes capacity)
              (do
                (define-var bucket-offset (* idx HASH_BUCKET_SIZE))
                (define-var occupied (array-at dict (+ bucket-offset HASH_OCCUPIED_OFFSET)))

                (if (= occupied HASH_EMPTY)
                    ; Empty slot - not found
                    (set probes capacity)
                    (do
                      (if (= occupied HASH_OCCUPIED)
                          ; Check if key matches
                          (if (= (array-at dict (+ bucket-offset HASH_KEY_OFFSET)) selector)
                              (do
                                (set found (array-at dict (+ bucket-offset HASH_VALUE_OFFSET)))
                                (set probes capacity))
                              0)
                          0)

                      ; Move to next bucket
                      (set idx (% (+ idx 1) capacity))
                      (set probes (+ probes 1))))))

            found))))
  
  (define-func (new-class name superclass)
    (do
      (define-var class (new-instance (tag-int 987) 3 0))
      (slot-at-put class 0 name)
      (slot-at-put class 1 superclass)
      (slot-at-put class 2 NULL)
      class))
  
  (define-func (class-set-methods class dict)
    (slot-at-put class 2 dict))
  
  (define-var SmallInteger-class NULL)
  
  (define-func (get-class obj)
    (if (is-int obj)
        SmallInteger-class
        (peek obj)))
  
  (define-func (get-name class) (slot-at class 0))
  (define-func (get-super class) (slot-at class 1))
  (define-func (get-methods class) (slot-at class 2))
 
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
  ; Caches the last lookup result per call site to avoid expensive method dictionary searches
  ;
  ; Cache structure (per entry):
  ; - Slot 0: Cached receiver class (OOP)
  ; - Slot 1: Cached selector (tagged int)
  ; - Slot 2: Cached method address
  ;
  ; Performance impact:
  ; - Cache hit: O(1) - just 2 comparisons
  ; - Cache miss: O(1) average with hash table + O(h) for inheritance chain
  ; - Typical hit rate: 95%+ in real Smalltalk programs

  (define-var INLINE_CACHE_SIZE 64)  ; Number of cache entries
  (define-var INLINE_CACHE_ENTRY_SIZE 3)  ; Slots per entry: [class, selector, method]
  (define-var inline-cache NULL)
  (define-var inline-cache-hits 0)
  (define-var inline-cache-misses 0)

  (define-func (init-inline-cache)
    (do
      ; Create cache array: INLINE_CACHE_SIZE entries * 3 slots per entry
      (set inline-cache (new-instance (tag-int 993) 0 (* INLINE_CACHE_SIZE INLINE_CACHE_ENTRY_SIZE)))

      ; Initialize all entries to NULL
      (for (i 0 (* INLINE_CACHE_SIZE INLINE_CACHE_ENTRY_SIZE))
        (array-at-put inline-cache i NULL))

      (set inline-cache-hits 0)
      (set inline-cache-misses 0)
      inline-cache))

  (define-func (inline-cache-get-entry cache-id offset)
    ; Get field from cache entry
    ; cache-id: 0 to INLINE_CACHE_SIZE-1
    ; offset: 0=class, 1=selector, 2=method
    (array-at inline-cache (+ (* cache-id INLINE_CACHE_ENTRY_SIZE) offset)))

  (define-func (inline-cache-set-entry cache-id offset value)
    ; Set field in cache entry
    (array-at-put inline-cache (+ (* cache-id INLINE_CACHE_ENTRY_SIZE) offset) value))

  (define-func (lookup-method-cached receiver selector cache-id)
    ; Optimized method lookup with inline caching
    ; cache-id: Call site identifier (0 to INLINE_CACHE_SIZE-1)
    ; Returns: Method address or NULL if not found
    (do
      (if (= inline-cache NULL)
          (init-inline-cache)
          0)

      ; Get receiver's class
      (define-var receiver-class (get-class receiver))

      ; Check cache: compare cached class and selector with current
      (define-var cached-class (inline-cache-get-entry cache-id 0))
      (define-var cached-selector (inline-cache-get-entry cache-id 1))

      (if (if (= cached-class receiver-class) (= cached-selector selector) 0)
          ; Cache hit - fast path!
          (do
            (set inline-cache-hits (+ inline-cache-hits 1))
            (inline-cache-get-entry cache-id 2))
          ; Cache miss - slow path: full lookup
          (do
            (set inline-cache-misses (+ inline-cache-misses 1))
            (define-var method (lookup-method receiver selector))

            ; Update cache with new lookup result
            (if (> method NULL)
                (do
                  (inline-cache-set-entry cache-id 0 receiver-class)
                  (inline-cache-set-entry cache-id 1 selector)
                  (inline-cache-set-entry cache-id 2 method))
                0)

            method))))

  (define-func (inline-cache-stats)
    ; Print inline cache statistics
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

  ; Context management functions
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

  ; Mini test framework
  (define-func (assert-equal actual expected msg)
    (if (= actual expected)
        1
        (abort msg)))

  (define-func (assert-true cond msg)
    (if cond
        1
        (abort msg)))

  ; Message send: create new context and activate it
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

            ; Store arguments in context temporaries
            (define-var arg-count (untag-int (peek args)))
            (for (i 0 arg-count)
              (context-temp-at-put new-ctx i (peek (+ args 1 i))))

            ; Switch to new context
            (set current-context new-ctx)

            new-ctx))))

  ; Method return: restore sender context and return value
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

  ; ===== String Operations =====
  ; Strings are stored as: [length][packed chars (8 per word)]

  (define-func (string-length str)
    ; Return length from word 0
    (peek str))

  (define-func (string-char-at str idx)
    ; Get character at index
    ; word_idx = 1 + idx / 8
    ; byte_idx = idx % 8
    (do
      (define-var word-idx (+ 1 (/ idx 8)))
      (define-var byte-idx (% idx 8))
      (define-var word (peek (+ str word-idx)))
      (bit-and (bit-shr word (* byte-idx 8)) 255)))

  (define-func (string-equal str1 str2)
    ; Compare two strings for equality
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
            1  ; uppercase
            (if (>= char 97)
                (if (<= char 122) 1 0)  ; lowercase
                0))
        0))

  (define-func (is-whitespace char)
    ; space=32, tab=9, newline=10, return=13
    (if (= char 32) 1
    (if (= char 9) 1
    (if (= char 10) 1
    (if (= char 13) 1 0)))))

  ; ===== AST Node System =====

  ; AST Node types as constants
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
      (slot-at-put node 0 (tag-int type))     ; node type
      (slot-at-put node 1 value)              ; value (for literals/selectors)
      node))

  ; AST node accessors
  (define-func (ast-type node) (untag-int (slot-at node 0)))
  (define-func (ast-value node) (slot-at node 1))
  (define-func (ast-child node idx) (array-at node idx))
  (define-func (ast-child-put node idx child) (array-at-put node idx child))
  (define-func (ast-child-count node) (peek (+ node OBJECT_HEADER_INDEXED_SLOTS)))

  ; ===== Tokenizer =====

  ; Token types
  (define-var TOK_NUMBER 1)
  (define-var TOK_IDENTIFIER 2)
  (define-var TOK_KEYWORD 3)
  (define-var TOK_STRING 4)
  (define-var TOK_BINARY_OP 5)
  (define-var TOK_SPECIAL 6)
  (define-var TOK_EOF 99)

  ; Token class (created in bootstrap)
  (define-var Token-class NULL)

  ; Tokenizer state
  (define-var tok-source NULL)
  (define-var tok-pos 0)
  (define-var tok-length 0)

  ; Token factory: (type, value, start, end)
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
    ; Get current char without advancing
    (if (< tok-pos tok-length)
        (string-char-at tok-source tok-pos)
        0))

  (define-func (tok-advance)
    ; Move to next char
    (set tok-pos (+ tok-pos 1)))

  (define-func (tok-skip-whitespace)
    (while (is-whitespace (tok-peek))
      (tok-advance)))

  (define-func (tok-read-number)
    ; Read consecutive digits, return tagged int value
    (do
      (define-var num 0)
      (while (is-digit (tok-peek))
        (do
          (define-var digit (- (tok-peek) 48))
          (set num (+ (* num 10) digit))
          (tok-advance)))
      (tag-int num)))

  (define-func (tok-read-identifier)
    ; Read letters and digits, return start position as identifier
    (do
      (define-var start-pos tok-pos)
      (while (if (is-letter (tok-peek))
                 1
                 (is-digit (tok-peek)))
        (tok-advance))
      ; Return start position as identifier (simplified - no symbol table yet)
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
    ; Return next token
    (do
      (tok-skip-whitespace)
      (define-var value NULL)
      (define-var start tok-pos)
      (define-var c (tok-peek))

      (if (= c 0)
          ; EOF
          (new-token TOK_EOF NULL start start)
      (if (is-digit c)
          ; Number
          (do
            (set value (tok-read-number))
            (new-token TOK_NUMBER value start tok-pos))
      (if (is-letter c)
          ; Identifier or keyword
          (do
            (set value (tok-read-identifier))
            ; Check if followed by ':'
            (if (= (tok-peek) 58)  ; ':' = 58
                (do
                  (tok-advance)
                  (new-token TOK_KEYWORD value start tok-pos))
                (new-token TOK_IDENTIFIER value start tok-pos)))
      (if (is-binary-op c)
          ; Binary operator
          (do
            (tok-advance)
            (new-token TOK_BINARY_OP (tag-int c) start tok-pos))
          ; Unknown - return EOF for now
          (new-token TOK_EOF NULL start start)))))))

  (define-func (tokenize source)
    ; Tokenize entire source, return array of tokens
    (do
      (set tok-source source)
      (set tok-pos 0)
      (set tok-length (string-length source))

      ; Allocate max possible tokens (length + 1 for EOF)
      (define-var max-tokens (+ tok-length 1))
      (define-var tokens (new-instance Array 0 max-tokens))

      ; Collect tokens until EOF
      (define-var count 0)
      (define-var tok (tok-next-token))
      (while (< (token-type tok) TOK_EOF)
        (do
          (array-at-put tokens count tok)
          (set count (+ count 1))
          (set tok (tok-next-token))))

      ; Add final EOF token
      (array-at-put tokens count tok)

      tokens))

  ; ===== Parser =====

  ; Parser state
  (define-var parse-tokens NULL)
  (define-var parse-pos 0)
  (define-var parse-length 0)

  ; Parser navigation
  (define-func (parse-peek)
    ; Get current token
    (if (< parse-pos parse-length)
        (array-at parse-tokens parse-pos)
        (new-token TOK_EOF NULL 0 0)))

  (define-func (parse-advance)
    ; Move to next token
    (set parse-pos (+ parse-pos 1)))

  (define-func (parse-token-type)
    ; Get type of current token
    (token-type (parse-peek)))

  (define-func (parse-token-value)
    ; Get value of current token
    (token-value (parse-peek)))

  ; parse-primary: parse numbers and identifiers (highest precedence)
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

  ; parse-unary-message: parse unary selectors (e.g., obj method)
  (define-func (parse-unary-message)
    (do
      (define-var receiver (parse-primary))
      ; Loop while we see identifiers (unary selectors)
      (define-var check-unary (parse-token-type))
      (while (= check-unary TOK_IDENTIFIER)
        (do
          (define-var selector (parse-token-value))
          (parse-advance)

          ; Create unary message AST node
          (define-var node (new-ast-node AST_UNARY_MSG selector 1))
          (ast-child-put node 0 receiver)
          (set receiver node)

          ; Update check-unary for next iteration
          (set check-unary (parse-token-type))))
      receiver))

  ; parse-binary-message: parse binary operators (e.g., 3 + 4)
  (define-func (parse-binary-message)
    (do
      (define-var left (parse-unary-message))
      ; Check for binary operator
      (define-var check-type (parse-token-type))
      (while (= check-type TOK_BINARY_OP)
        (do
          (define-var op (parse-token-value))
          (parse-advance)
          (define-var right (parse-unary-message))

          ; Create binary message AST node
          (define-var node (new-ast-node AST_BINARY_MSG op 2))
          (ast-child-put node 0 left)
          (ast-child-put node 1 right)
          (set left node)

          ; Update check-type for next iteration
          (set check-type (parse-token-type))))
      left))

  ; parse-keyword-message: parse keyword messages (e.g., obj at: 1 put: 2)
  (define-func (parse-keyword-message)
    (do
      (define-var receiver (parse-binary-message))
      ; Check if next token is keyword
      (define-var check-keyword (parse-token-type))
      (if (= check-keyword TOK_KEYWORD)
          (do
            ; Allocate temporary storage for keyword parts and arguments
            (define-var args-temp (malloc 10))
            (define-var keywords-temp (malloc 10))
            (define-var arg-count 0)

            ; Collect keyword parts and arguments
            (while (= check-keyword TOK_KEYWORD)
              (do
                ; Save keyword token position BEFORE advancing
                (poke (+ keywords-temp arg-count) (parse-token-value))
                (parse-advance)  ; Skip the keyword token

                ; Parse argument (binary message level)
                (poke (+ args-temp arg-count) (parse-binary-message))
                (set arg-count (+ arg-count 1))

                ; Update check-keyword for next iteration
                (set check-keyword (parse-token-type))))

            ; Create keyword message AST node with:
            ; - value: number of arguments (to separate args from keyword positions)
            ; - children: [receiver, arg1, arg2, ..., keyword_pos1, keyword_pos2, ...]
            (define-var total-children (+ (+ arg-count 1) arg-count))
            (define-var node (new-ast-node AST_KEYWORD_MSG (tag-int arg-count) total-children))
            (ast-child-put node 0 receiver)
            ; Store arguments
            (for (i 0 arg-count)
              (ast-child-put node (+ i 1) (peek (+ args-temp i))))
            ; Store keyword positions
            (for (i 0 arg-count)
              (ast-child-put node (+ (+ arg-count 1) i) (peek (+ keywords-temp i))))
            node)
          receiver)))

  ; parse-expression: top-level entry point (lowest precedence)
  (define-func (parse-expression)
    (parse-keyword-message))

  ; parse: main entry point
  (define-func (parse source-string)
    ; Tokenize, then parse
    (do
      (define-var token-array (tokenize source-string))
      (set parse-tokens token-array)
      (set parse-pos 0)

      ; Count non-EOF tokens for length
      (define-var len 0)
      (while (< (token-type (array-at token-array len)) TOK_EOF)
        (set len (+ len 1)))
      (set parse-length len)

      (parse-expression)))

  ; ===== Smalltalk Bytecode Compiler (Step 3) =====

  ; VM Opcode constants
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

  ; Bytecode buffer state
  (define-var bytecode-buffer NULL)
  (define-var bytecode-pos 0)

  ; Helper function addresses (filled in during bootstrap)
  (define-var send-unary-addr NULL)
  (define-var lookup-method-addr NULL)

  ; Source string for selector extraction during compilation
  (define-var compile-source-string NULL)

  ; Temporary storage for message send compilation
  (define-var temp-receiver NULL)
  (define-var temp-method-addr NULL)

  ; Extract identifier string from source at position and intern it
  (define-func (intern-identifier-at-pos pos)
    (do
      ; Find the end of the identifier (letters and digits) or operator
      (define-var start pos)
      (define-var end pos)
      (define-var src-len (string-length compile-source-string))

      ; Check if this is an operator (single character: + - * / < > =)
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
          ; For operators, just take one character
          (set end (+ pos 1))
          ; For identifiers, scan forward while we see letters or digits
          (while (< end src-len)
            (do
              (define-var ch (string-char-at compile-source-string end))
              (if (if (is-letter ch) 1 (is-digit ch))
                  (set end (+ end 1))
                  (set end src-len)))))  ; Break loop

      ; Create a string with the extracted identifier
      (define-var id-len (- end start))
      (define-var id-str (malloc (+ (/ id-len 8) 2)))
      (poke id-str id-len)  ; Store length

      ; Copy characters into the new string
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

      ; Intern the extracted string
      (intern-selector id-str)))

  ; Build keyword selector from AST keyword message node
  ; e.g., "at:put:" from positions of "at:" and "put:"
  (define-func (build-keyword-selector ast)
    (do
      (define-var num-args (untag-int (ast-value ast)))
      (define-var total-len 0)

      ; First pass: calculate total length needed
      (for (i 0 num-args)
        (do
          (define-var kw-pos (untag-int (ast-child ast (+ (+ num-args 1) i))))
          (define-var start kw-pos)
          (define-var end kw-pos)
          (define-var src-len (string-length compile-source-string))

          ; Scan to find end of keyword (letter/digits followed by ':')
          (while (< end src-len)
            (do
              (define-var ch (string-char-at compile-source-string end))
              (if (= ch 58)  ; ':' = 58
                  (do
                    (set end (+ end 1))
                    (set end src-len))  ; Break
                  (if (if (is-letter ch) 1 (is-digit ch))
                      (set end (+ end 1))
                      (set end src-len)))))  ; Break

          (set total-len (+ total-len (- end start)))))

      ; Allocate string for concatenated selector
      (define-var sel-str (malloc (+ (/ total-len 8) 2)))
      (poke sel-str total-len)  ; Store length

      ; Second pass: copy all keyword parts into the string
      (define-var dest-pos 0)
      (for (i 0 num-args)
        (do
          (define-var kw-pos (untag-int (ast-child ast (+ (+ num-args 1) i))))
          (define-var start kw-pos)
          (define-var end kw-pos)
          (define-var src-len (string-length compile-source-string))

          ; Find end again
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

          ; Copy characters from this keyword
          (define-var kw-len (- end start))
          (for (j 0 kw-len)
            (do
              (define-var ch (string-char-at compile-source-string (+ start j)))
              ; Pack character into destination string
              (define-var word-idx (/ dest-pos 8))
              (define-var byte-idx (% dest-pos 8))
              (define-var current-word (peek (+ sel-str 1 word-idx)))
              (define-var new-word (bit-or current-word (bit-shl ch (* byte-idx 8))))
              (poke (+ sel-str 1 word-idx) new-word)
              (set dest-pos (+ dest-pos 1))))))

      ; Intern the concatenated selector
      (intern-selector sel-str)))

  ; Initialize bytecode buffer in heap
  (define-func (init-bytecode max-size)
    (do
      (set bytecode-buffer heap-pointer)
      (set bytecode-pos 0)
      (set heap-pointer (+ heap-pointer max-size))
      bytecode-buffer))

  ; Emit a word to bytecode buffer
  (define-func (emit word)
    (do
      (poke (+ bytecode-buffer bytecode-pos) word)
      (set bytecode-pos (+ bytecode-pos 1))))

  ; Get current bytecode address for jump patching
  (define-func (current-address)
    (+ bytecode-buffer bytecode-pos))

  ; Compile Smalltalk AST to VM bytecode
  (define-func (compile-st-expr ast)
    (do
      (define-var type (ast-type ast))
      (if (= type AST_NUMBER)
          ; Number: emit PUSH <value>
          (do
            (emit OP_PUSH)
            (emit (ast-value ast)))
      (if (= type AST_IDENTIFIER)
          ; Identifier: for now, just push 0 (placeholder - should load variable)
          (do
            (emit OP_PUSH)
            (emit (tag-int 0)))
      (if (= type AST_UNARY_MSG)
          ; Unary message: receiver selector
          ; Call lookup-method then FUNCALL the result
          ; Stack: [] -> [result]
          (do
            ; Get selector identifier position from AST
            (define-var selector-pos (untag-int (ast-value ast)))

            ; Extract and intern the selector from source
            (define-var selector-id (intern-identifier-at-pos selector-pos))

            ; Compile receiver expression
            (compile-st-expr (ast-child ast 0))
            ; Stack: [receiver]

            ; Duplicate receiver for both lookup and method call
            (emit OP_DUP)                          ; [receiver, receiver]

            ; Push interned selector ID as second argument to lookup-method
            (emit OP_PUSH)
            (emit selector-id)                     ; [receiver, receiver, selector_id]

            ; Push address of lookup-method function and call it
            ; The function address will be available after bootstrap compiles it
            (emit OP_PUSH)
            (emit lookup-method-addr)              ; [receiver, receiver, selector, lookup-addr]
            (emit OP_PUSH)
            (emit 2)                               ; [receiver, receiver, selector, lookup-addr, 2] (2 args)
            (emit OP_FUNCALL)                      ; [receiver, method_addr]

            ; Now call the found method with receiver as its argument
            (emit OP_PUSH)
            (emit 1)                               ; [receiver, method_addr, 1] (1 arg)
            (emit OP_FUNCALL)                      ; [result]

            0)
      (if (= type AST_KEYWORD_MSG)
          ; Keyword message: receiver selector: arg1 keyword2: arg2 ...
          ; Stack: [] -> [result]
          (do
            ; Build complete keyword selector (e.g., "at:put:")
            (define-var keyword-sel-id (build-keyword-selector ast))

            ; Get number of arguments from AST value
            (define-var num-args (untag-int (ast-value ast)))

            ; Compile all arguments first (so they're in right order on stack)
            (for (i 0 num-args)
              (compile-st-expr (ast-child ast (+ i 1))))
            ; Stack: [arg1, arg2, ..., argN]

            ; Compile receiver expression
            (compile-st-expr (ast-child ast 0))
            ; Stack: [arg1, arg2, ..., argN, receiver]

            ; Duplicate receiver for both lookup and method call
            (emit OP_DUP)
            ; Stack: [arg1, arg2, ..., argN, receiver, receiver]

            ; Push keyword selector ID for lookup
            (emit OP_PUSH)
            (emit keyword-sel-id)
            ; Stack: [arg1, ..., argN, receiver, receiver, selector_id]

            ; Push address of lookup-method function and call it
            (emit OP_PUSH)
            (emit lookup-method-addr)
            ; Stack: [arg1, ..., argN, receiver, receiver, selector_id, lookup-addr]

            (emit OP_PUSH)
            (emit 2)
            ; Stack: [arg1, ..., argN, receiver, receiver, selector_id, lookup-addr, 2]

            (emit OP_FUNCALL)
            ; Stack: [arg1, arg2, ..., argN, receiver, method_addr]

            ; Push total argument count (receiver + all keyword args)
            (emit OP_PUSH)
            (emit (+ num-args 1))
            ; Stack: [arg1, arg2, ..., argN, receiver, method_addr, N+1]

            (emit OP_FUNCALL)
            ; Stack: [result]

            0)
      (if (= type AST_BINARY_MSG)
          ; Binary message: receiver op argument
          ; Call lookup-method then FUNCALL the result with receiver and argument
          (do
            ; Get operator character from AST (stored as ASCII value, not position!)
            (define-var op-char (untag-int (ast-value ast)))

            ; Create a single-character string for the operator
            (define-var op-str (malloc 2))
            (poke op-str 1)  ; length = 1
            (poke (+ op-str 1) op-char)  ; store the operator character

            ; Intern the operator string as a selector
            (define-var op-sel-id (intern-selector op-str))

            ; Compile receiver expression
            (compile-st-expr (ast-child ast 0))
            ; Stack: [receiver]

            ; Duplicate receiver for both lookup and method call
            (emit OP_DUP)                          ; [receiver, receiver]

            ; Push interned operator selector ID
            (emit OP_PUSH)
            (emit op-sel-id)                       ; [receiver, receiver, selector_id]

            ; Push address of lookup-method function and call it
            (emit OP_PUSH)
            (emit lookup-method-addr)              ; [receiver, receiver, selector, lookup-addr]
            (emit OP_PUSH)
            (emit 2)                               ; [receiver, receiver, selector, lookup-addr, 2]
            (emit OP_FUNCALL)                      ; [receiver, method_addr]

            ; Now compile the argument
            (compile-st-expr (ast-child ast 1))    ; [receiver, method_addr, arg]

            ; Swap to get method_addr on top
            (emit OP_SWAP)                         ; [receiver, arg, method_addr]

            ; Call the method with 2 arguments (receiver + arg)
            (emit OP_PUSH)
            (emit 2)                               ; [receiver, arg, method_addr, 2]
            (emit OP_FUNCALL)                      ; [result]

            0)
          (abort "Unknown AST node type in compile"))))))))

  ; Compile Smalltalk source to bytecode, return start address
  (define-func (compile-smalltalk source-string)
    (do
      ; Store source string for selector extraction during compilation
      (set compile-source-string source-string)

      ; Initialize bytecode buffer (allocate 1000 words max)
      (init-bytecode 1000)

      ; Parse source into AST
      (define-var ast (parse source-string))

      ; Compile AST to bytecode
      (compile-st-expr ast)

      ; Emit HALT to end execution
      (emit OP_HALT)

      ; Return start address of compiled code
      bytecode-buffer))

  ; Compile a method body to bytecode
  ; For now: takes expression, compiles it, adds RET
  ; Returns bytecode address
  (define-func (compile-method source-string arg-count)
    (do
      ; Store source string for selector extraction during compilation
      (set compile-source-string source-string)

      ; Initialize bytecode buffer (allocate 1000 words max)
      (init-bytecode 1000)

      ; Parse source into AST
      (define-var ast (parse source-string))

      ; Compile AST to bytecode
      (compile-st-expr ast)

      ; Emit RET with argument count
      (emit OP_RET)
      (emit arg-count)

      ; Return start address of compiled code
      bytecode-buffer))

  ; Install a compiled method into a class
  ; class: the class to add the method to
  ; selector: tagged int identifier for the method
  ; source: method body source code
  ; arg-count: number of arguments the method takes
  (define-func (install-method class selector source arg-count)
    (do
      ; Compile the method
      (define-var method-addr (compile-method source arg-count))

      ; Get or create method dictionary for class
      (define-var methods (get-methods class))
      (if (= methods NULL)
          (do
            (define-var new-dict (new-method-dict 10))
            (class-set-methods class new-dict)
            (set methods new-dict))
          0)

      ; Add method to dictionary
      (method-dict-add methods selector method-addr)

      method-addr))

  (define-func (bootstrap-smalltalk)
    (do
      (print-string "=== Smalltalk Bootstrap ===")
      (print-string "")
      
      (print-string "Creating core classes...")
      
      (define-var ProtoObject (new-class (tag-int 1) NULL))
      (define-var proto-methods (new-method-dict 5))
      (method-dict-add proto-methods (tag-int 10) (tag-int 10000))
      (class-set-methods ProtoObject proto-methods)
      (print-string "  ProtoObject: class (sel:10)")
      
      (define-var Object (new-class (tag-int 2) ProtoObject))
      (define-var obj-methods (new-method-dict 10))
      (method-dict-add obj-methods (tag-int 20) (tag-int 20000))
      (method-dict-add obj-methods (tag-int 21) (tag-int 21000))
      (method-dict-add obj-methods (tag-int 22) (tag-int 22000))
      (class-set-methods Object obj-methods)
      (print-string "  Object: ==, ~=, yourself")

      ; Create ASTNode and Token classes for parser
      (set ASTNode-class (new-class (tag-int 100) Object))
      (set Token-class (new-class (tag-int 101) Object))

      (define-var Magnitude (new-class (tag-int 3) Object))
      (define-var mag-methods (new-method-dict 10))
      (method-dict-add mag-methods (tag-int 30) (tag-int 30000))
      (method-dict-add mag-methods (tag-int 31) (tag-int 31000))
      (class-set-methods Magnitude mag-methods)
      (print-string "  Magnitude: <, >")
      
      (define-var Number (new-class (tag-int 4) Magnitude))
      (define-var num-methods (new-method-dict 10))
      (method-dict-add num-methods (tag-int 40) (tag-int 40000))
      (method-dict-add num-methods (tag-int 41) (tag-int 41000))
      (method-dict-add num-methods (tag-int 42) (tag-int 42000))
      (method-dict-add num-methods (tag-int 43) (tag-int 43000))
      (class-set-methods Number num-methods)
      (print-string "  Number: +, -, *, /")
      
      (define-var SmallInteger (new-class (tag-int 5) Number))
      (set SmallInteger-class SmallInteger)
      (define-var int-methods (new-method-dict 15))
      (method-dict-add int-methods (tag-int 40) (tag-int 50000))
      (method-dict-add int-methods (tag-int 41) (tag-int 51000))
      (method-dict-add int-methods (tag-int 42) (tag-int 52000))
      (method-dict-add int-methods (tag-int 43) (tag-int 53000))
      (method-dict-add int-methods (tag-int 50) (tag-int 54000))
      (method-dict-add int-methods (tag-int 51) (tag-int 55000))
      (class-set-methods SmallInteger int-methods)
      (print-string "  SmallInteger: +, -, *, /, bitAnd:, bitOr:")
      
      (define-var Collection (new-class (tag-int 6) Object))
      (define-var coll-methods (new-method-dict 10))
      (method-dict-add coll-methods (tag-int 60) (tag-int 60000))
      (method-dict-add coll-methods (tag-int 61) (tag-int 61000))
      (class-set-methods Collection coll-methods)
      (print-string "  Collection: size, isEmpty")
      
      (set Array (new-class (tag-int 7) Collection))
      (define-var arr-methods (new-method-dict 10))
      (method-dict-add arr-methods (tag-int 70) (tag-int 70000))
      (method-dict-add arr-methods (tag-int 71) (tag-int 71000))
      (class-set-methods Array arr-methods)
      (print-string "  Array: at:, at:put:")
      
      (define-var Point (new-class (tag-int 8) Object))
      (define-var pt-methods (new-method-dict 10))
      (method-dict-add pt-methods (tag-int 80) (tag-int 80000))
      (method-dict-add pt-methods (tag-int 81) (tag-int 81000))
      (method-dict-add pt-methods (tag-int 82) (tag-int 82000))
      (class-set-methods Point pt-methods)
      (print-string "  Point: x, y, dist")
      
      (print-string "")
      (print-string "Class hierarchy:")
      (print-string "  ProtoObject")
      (print-string "    Object")
      (print-string "      Magnitude")
      (print-string "        Number")
      (print-string "          SmallInteger")
      (print-string "      Collection")
      (print-string "        Array")
      (print-string "      Point")
      (print-string "")
      
      (print-string "=== Testing Method Lookup ===")
      (print-string "")

