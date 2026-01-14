; ===== Symbol Table Module =====
; Hash table implementation and symbol table for selectors

(do
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

  0)
