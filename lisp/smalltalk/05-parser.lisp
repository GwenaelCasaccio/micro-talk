; ===== Parser Module =====
; AST node system and Smalltalk parser

(do
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

  0)
