; ===== Methods Module =====
; Method dictionaries, lookup, inline cache, context management

(do
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

  0)
