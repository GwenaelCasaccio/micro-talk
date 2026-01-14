; ===== Compiler Module =====
; Smalltalk to VM bytecode compiler

(do
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
