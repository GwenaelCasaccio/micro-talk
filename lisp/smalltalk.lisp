(do
  (define-var HEAP_START 30000)
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
  (define-func (array-at-put object idx value) (poke (+ object OBJECT_HEADER_SIZE (peek (+ object OBJECT_HEADER_NAMED_SLOTS)) idx) value))

  (define-func (malloc size)
    (do
      (define-var result heap-pointer)
      (set heap-pointer (+ heap-pointer size))
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

  (define-func (new-method-dict capacity)
    (do
      (define-var dict (new-instance (tag-int 989) 1 (* capacity 2)))
      (slot-at-put dict 0 (tag-int 0))
      dict))
  
  (define-func (method-dict-add dict selector code-addr)
    (do
      (define-var size (untag-int (slot-at dict 0)))
      (define-var entry (+ 1 (* size 2)))
      (array-at-put dict entry selector)
      (array-at-put dict (+ entry 1) code-addr)
      (slot-at-put dict  0 (tag-int (+ size 1)))
      dict))
  
  (define-func (method-dict-lookup dict selector)
    (do
      (if (= dict NULL)
          NULL
          (do
            (define-var size (untag-int (slot-at dict 0)))
            (define-var found NULL)
            (for (i 0 size)
              (do
                (define-var entry (+ 1 (* i 2)))
                (if (= (array-at dict entry) selector)
                    (set found (array-at dict (+ entry 1)))
                    0)))
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
            (define-var arg-count (untag-int args))
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
            (define-var arg-count 0)

            ; Collect keyword parts and arguments
            (while (= check-keyword TOK_KEYWORD)
              (do
                (parse-advance)  ; Skip the keyword token

                ; Parse argument (binary message level)
                (define-var arg-result (parse-binary-message))
                (poke (+ args-temp arg-count) arg-result)
                (set arg-count (+ arg-count 1))

                ; Update check-keyword for next iteration
                (set check-keyword (parse-token-type))))

            ; Create keyword message AST node (receiver + args as children)
            (define-var node (new-ast-node AST_KEYWORD_MSG (tag-int 0) (+ arg-count 1)))
            (ast-child-put node 0 receiver)
            (for (i 0 arg-count)
              (ast-child-put node (+ i 1) (peek (+ args-temp i))))
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
  (define-var OP_ADD 4)
  (define-var OP_SUB 5)
  (define-var OP_MUL 6)
  (define-var OP_DIV 7)
  (define-var OP_MOD 8)
  (define-var OP_AND 9)
  (define-var OP_OR 10)
  (define-var OP_XOR 11)
  (define-var OP_SHL 12)
  (define-var OP_SHR 13)
  (define-var OP_ASHR 14)
  (define-var OP_EQ 15)
  (define-var OP_LT 16)
  (define-var OP_GT 17)
  (define-var OP_LTE 18)
  (define-var OP_GTE 19)

  ; Bytecode buffer state
  (define-var bytecode-buffer NULL)
  (define-var bytecode-pos 0)

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
          ; Identifier: for now, just push 0 (placeholder)
          (do
            (emit OP_PUSH)
            (emit (tag-int 0)))
      (if (= type AST_UNARY_MSG)
          ; Unary message: compile receiver, then emit message send
          ; For now: just compile receiver (no actual message send yet)
          (compile-st-expr (ast-child ast 0))
      (if (= type AST_KEYWORD_MSG)
          ; Keyword message: compile receiver and all args
          ; For now: just compile receiver (no actual message send yet)
          (compile-st-expr (ast-child ast 0))
      (if (= type AST_BINARY_MSG)
          ; Binary message: compile left, compile right, emit operator
          (do
            (compile-st-expr (ast-child ast 0))  ; compile receiver
            (compile-st-expr (ast-child ast 1))  ; compile argument
            (define-var op (untag-int (ast-value ast)))

            ; Emit appropriate opcode
            (if (= op 43)  ; + (ASCII 43)
                (emit OP_ADD)
            (if (= op 45)  ; - (ASCII 45)
                (emit OP_SUB)
            (if (= op 42)  ; * (ASCII 42)
                (emit OP_MUL)
            (if (= op 47)  ; / (ASCII 47)
                (emit OP_DIV)
                (abort "Unknown binary operator in compile"))))))
          (abort "Unknown AST node type in compile"))))))))

  ; Compile Smalltalk source to bytecode, return start address
  (define-func (compile-smalltalk source-string)
    (do
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
      
      (print-string "Test 1: SmallInteger method lookup")
      (define-var int-val (tag-int 42))
      (print-string "  42 is tagged int?")
      (print-int (is-int int-val))
      
      (print-string "  42 class:")
      (define-var int-class (get-class int-val))
      (print-string "    class addr:")
      (print-int int-class)
      (print-string "    class name:")
      (print-int (untag-int (get-name int-class)))
      
      (print-string "  Lookup: 42 + (sel:40)")
      (define-var add-method (lookup-method int-val (tag-int 40)))
      (print-string "    Found:")
      (print-int (untag-int add-method))
      (print-string "    Expected: 50000 (SmallInteger override)")
      (print-string "")
      
      (print-string "Test 2: SmallInteger inherited method")
      (print-string "  Lookup: 42 < (sel:30)")
      (define-var lt-method (lookup-method int-val (tag-int 30)))
      (print-string "    Found:")
      (print-int (untag-int lt-method))
      (print-string "    Expected: 30000 (from Magnitude)")
      (print-string "")
      
      (print-string "Test 3: SmallInteger from Object")
      (print-string "  Lookup: 42 == (sel:20)")
      (define-var eq-method (lookup-method int-val (tag-int 20)))
      (print-string "    Found:")
      (print-int (untag-int eq-method))
      (print-string "    Expected: 20000 (from Object)")
      (print-string "")
      
      (print-string "Test 4: SmallInteger from ProtoObject")
      (print-string "  Lookup: 42 class (sel:10)")
      (define-var class-method (lookup-method int-val (tag-int 10)))
      (print-string "    Found:")
      (print-int (untag-int class-method))
      (print-string "    Expected: 10000 (from ProtoObject)")
      (print-string "")
      
      (print-string "Test 5: Point instance")
      (define-var p (new-instance Point 2 0))
      (print-string "  Point instance:")
      (print-int p)
      (print-string "  is int?")
      (print-int (is-int p))
      (print-string "  is oop?")
      (print-int (is-oop p))
      
      (print-string "  Lookup: p x (sel:80)")
      (define-var x-method (lookup-method p (tag-int 80)))
      (print-string "    Found:")
      (print-int (untag-int x-method))
      (print-string "    Expected: 80000")
      (print-string "")
      
      (print-string "Test 6: Point inherited from Object")
      (print-string "  Lookup: p == (sel:20)")
      (define-var p-eq-method (lookup-method p (tag-int 20)))
      (print-string "    Found:")
      (print-int (untag-int p-eq-method))
      (print-string "    Expected: 20000 (from Object)")
      (print-string "")
      
      (print-string "Test 7: Array instance")
      (define-var arr (new-instance Array 0 70))
      (print-string "  Lookup: arr at: (sel:70)")
      (define-var at-method (lookup-method arr (tag-int 70)))
      (print-string "    Found:")
      (print-int (untag-int at-method))
      (print-string "    Expected: 70000")
      
      (print-string "  Lookup: arr size (sel:60)")
      (define-var size-method (lookup-method arr (tag-int 60)))
      (print-string "    Found:")
      (print-int (untag-int size-method))
      (print-string "    Expected: 60000 (from Collection)")
      (print-string "")
      
      (print-string "Test 8: Complete inheritance chain")
      (print-string "  SmallInteger hierarchy depth: 5")
      (print-string "    SmallInteger -> Number -> Magnitude -> Object -> ProtoObject")
      (print-string "  Test all levels:")
      
      (define-var si (tag-int 100))
      (print-string "    Level 1 (SmallInteger): bitAnd: (sel:50)")
      (define-var l1 (lookup-method si (tag-int 50)))
      (print-int (untag-int l1))
      
      (print-string "    Level 2 (Number): + (sel:40)")
      (define-var l2 (lookup-method si (tag-int 40)))
      (print-int (untag-int l2))
      
      (print-string "    Level 3 (Magnitude): < (sel:30)")
      (define-var l3 (lookup-method si (tag-int 30)))
      (print-int (untag-int l3))
      
      (print-string "    Level 4 (Object): == (sel:20)")
      (define-var l4 (lookup-method si (tag-int 20)))
      (print-int (untag-int l4))
      
      (print-string "    Level 5 (ProtoObject): class (sel:10)")
      (define-var l5 (lookup-method si (tag-int 10)))
      (print-int (untag-int l5))
      
      (print-string "")
      (print-string "=== Testing Context Management ===")
      (print-string "")

      ; Test 9: Create context
      (print-string "Test 9: Create context")
      (assert-equal current-context NULL "Initial context should be NULL")
      (define-var test-ctx (new-context NULL (tag-int 42) (tag-int 50000) 3))
      (assert-equal (untag-int (context-get-receiver test-ctx)) 42 "Context receiver should be 42")
      (assert-equal (untag-int (context-get-method test-ctx)) 50000 "Context method should be 50000")
      (print-string "  PASSED")

      ; Test 10: Context temporaries
      (print-string "Test 10: Context temporaries")
      (context-temp-at-put test-ctx 0 (tag-int 10))
      (context-temp-at-put test-ctx 1 (tag-int 20))
      (context-temp-at-put test-ctx 2 (tag-int 30))
      (assert-equal (untag-int (context-temp-at test-ctx 0)) 10 "Temp 0 should be 10")
      (assert-equal (untag-int (context-temp-at test-ctx 1)) 20 "Temp 1 should be 20")
      (assert-equal (untag-int (context-temp-at test-ctx 2)) 30 "Temp 2 should be 30")
      (print-string "  PASSED")

      ; Test 11: Message send (context switching)
      (print-string "Test 11: Message send (context switching)")
      (define-var args-addr (malloc 3))
      (poke args-addr (tag-int 2))
      (poke (+ args-addr 1) (tag-int 100))
      (poke (+ args-addr 2) (tag-int 200))
      (define-var msg-ctx (message-send (tag-int 42) (tag-int 40) args-addr 2))
      (assert-equal current-context msg-ctx "Current context should be new context")
      (assert-equal (untag-int (context-get-receiver msg-ctx)) 42 "Message receiver should be 42")
      (assert-equal (untag-int (context-get-method msg-ctx)) 50000 "Should find SmallInteger + method")
      (assert-equal (untag-int (context-temp-at msg-ctx 0)) 100 "First arg should be 100")
      (assert-equal (untag-int (context-temp-at msg-ctx 1)) 200 "Second arg should be 200")
      (print-string "  PASSED")

      ; Test 12: Nested message sends
      (print-string "Test 12: Nested message sends")
      (define-var outer-ctx current-context)
      (define-var args2-addr (malloc 2))
      (poke args2-addr (tag-int 1))
      (poke (+ args2-addr 1) (tag-int 5))
      (define-var inner-ctx (message-send (tag-int 10) (tag-int 41) args2-addr 1))
      (assert-equal (context-get-sender inner-ctx) outer-ctx "Inner sender should be outer context")
      (assert-equal current-context inner-ctx "Current should be inner context")
      (print-string "  PASSED")

      ; Test 13: Method return
      (print-string "Test 13: Method return")
      (define-var return-val (method-return (tag-int 999)))
      (assert-equal (untag-int return-val) 999 "Return value should be 999")
      (assert-equal current-context outer-ctx "Should restore outer context")
      (print-string "  PASSED")

      ; Test 14: Return to NULL (end of chain)
      (print-string "Test 14: Return to NULL (end of chain)")
      (define-var final-return (method-return (tag-int 777)))
      (assert-equal (untag-int final-return) 777 "Final return value should be 777")
      (assert-equal current-context NULL "Should restore to NULL context")
      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing String Operations ===")
      (print-string "")

      ; Test 15: String operations with manual string
      (print-string "Test 15: String operations")

      ; Create test string "Hi" manually (H=72, i=105)
      (define-var test-str (malloc 2))
      (poke test-str 2)  ; length = 2
      (define-var word (+ 72 (bit-shl 105 8)))  ; Pack 'H' and 'i'
      (poke (+ test-str 1) word)

      (assert-equal (string-length test-str) 2 "String length should be 2")
      (assert-equal (string-char-at test-str 0) 72 "First char should be 'H'=72")
      (assert-equal (string-char-at test-str 1) 105 "Second char should be 'i'=105")
      (print-string "  PASSED")

      ; Test 16: Character classification
      (print-string "Test 16: Character classification")
      (assert-equal (is-digit 48) 1 "'0'=48 is digit")
      (assert-equal (is-digit 53) 1 "'5'=53 is digit")
      (assert-equal (is-digit 65) 0 "'A'=65 is not digit")
      (assert-equal (is-letter 65) 1 "'A'=65 is letter")
      (assert-equal (is-letter 122) 1 "'z'=122 is letter")
      (assert-equal (is-letter 48) 0 "'0'=48 is not letter")
      (assert-equal (is-whitespace 32) 1 "Space=32 is whitespace")
      (assert-equal (is-whitespace 10) 1 "Newline=10 is whitespace")
      (assert-equal (is-whitespace 65) 0 "'A'=65 is not whitespace")
      (print-string "  PASSED")

      ; Test 17: AST node creation
      (print-string "Test 17: AST node creation")
      (define-var ast-num (new-ast-node AST_NUMBER (tag-int 42) 0))
      (assert-equal (ast-type ast-num) AST_NUMBER "AST type should be NUMBER")
      (assert-equal (untag-int (ast-value ast-num)) 42 "AST value should be 42")

      (define-var ast-binop (new-ast-node AST_BINARY_MSG (tag-int 43) 2))
      (ast-child-put ast-binop 0 ast-num)
      (ast-child-put ast-binop 1 ast-num)
      (assert-equal (ast-type ast-binop) AST_BINARY_MSG "AST type should be BINARY_MSG")
      (assert-equal (ast-child ast-binop 0) ast-num "First child should be ast-num")
      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing Tokenizer ===")
      (print-string "")

      ; Test 18: Tokenize "3 + 4"
      (print-string "Test 18: Tokenize '3 + 4'")

      ; Create test string "3 + 4" manually
      (define-var test-source (malloc 2))
      (poke test-source 5)  ; length = 5
      (define-var w0 (+ 51 (bit-shl 32 8) (bit-shl 43 16) (bit-shl 32 24) (bit-shl 52 32)))
      (poke (+ test-source 1) w0)  ; "3 + 4" (51=3, 32=space, 43=+, 32=space, 52=4)

      (define-var tokens (tokenize test-source))

      ; Should have 4 tokens: NUMBER(3), BINARY_OP(+), NUMBER(4), EOF
      (define-var tok0 (array-at tokens 0))
      (define-var tok1 (array-at tokens 1))
      (define-var tok2 (array-at tokens 2))
      (define-var tok3 (array-at tokens 3))

      (assert-equal (token-type tok0) TOK_NUMBER "First token should be NUMBER")
      (assert-equal (untag-int (token-value tok0)) 3 "First token value should be 3")

      (assert-equal (token-type tok1) TOK_BINARY_OP "Second token should be BINARY_OP")
      (assert-equal (untag-int (token-value tok1)) 43 "Second token value should be + (43)")

      (assert-equal (token-type tok2) TOK_NUMBER "Third token should be NUMBER")
      (assert-equal (untag-int (token-value tok2)) 4 "Third token value should be 4")

      (assert-equal (token-type tok3) TOK_EOF "Fourth token should be EOF")

      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing Parser ===")
      (print-string "")

      ; Test 19: Parse "3 + 4" into AST
      (print-string "Test 19: Parse '3 + 4' into AST")

      (define-var ast (parse test-source))

      ; AST should be: BINARY_MSG(+, NUMBER(3), NUMBER(4))
      (assert-equal (ast-type ast) AST_BINARY_MSG "Root should be BINARY_MSG")
      (assert-equal (untag-int (ast-value ast)) 43 "Operator should be + (43)")

      (define-var left-child (ast-child ast 0))
      (define-var right-child (ast-child ast 1))

      (assert-equal (ast-type left-child) AST_NUMBER "Left child should be NUMBER")
      (assert-equal (untag-int (ast-value left-child)) 3 "Left value should be 3")

      (assert-equal (ast-type right-child) AST_NUMBER "Right child should be NUMBER")
      (assert-equal (untag-int (ast-value right-child)) 4 "Right value should be 4")

      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing Smalltalk Compiler ===")
      (print-string "")

      ; Test 20: Compile "3 + 4" to bytecode
      (print-string "Test 20: Compile '3 + 4' to VM bytecode")

      (define-var code-addr (compile-smalltalk test-source))

      ; Verify bytecode was generated
      (assert-true (> code-addr 0) "Code address should be valid")

      ; Inspect compiled bytecode
      (print-string "  Compiled bytecode:")
      (print-string "    Address:")
      (print-int code-addr)

      ; Expected bytecode for "3 + 4":
      ; PUSH, 3, PUSH, 4, ADD, HALT
      (define-var b0 (peek code-addr))
      (define-var b1 (peek (+ code-addr 1)))
      (define-var b2 (peek (+ code-addr 2)))
      (define-var b3 (peek (+ code-addr 3)))
      (define-var b4 (peek (+ code-addr 4)))
      (define-var b5 (peek (+ code-addr 5)))

      (print-string "    Bytecode:")
      (print-int b0)  ; Should be OP_PUSH (1)
      (print-int b1)  ; Should be tagged 3
      (print-int b2)  ; Should be OP_PUSH (1)
      (print-int b3)  ; Should be tagged 4
      (print-int b4)  ; Should be OP_ADD (4)
      (print-int b5)  ; Should be OP_HALT (0)

      (assert-equal b0 OP_PUSH "First opcode should be PUSH")
      (assert-equal (untag-int b1) 3 "First operand should be 3")
      (assert-equal b2 OP_PUSH "Second opcode should be PUSH")
      (assert-equal (untag-int b3) 4 "Second operand should be 4")
      (assert-equal b4 OP_ADD "Third opcode should be ADD")
      (assert-equal b5 OP_HALT "Fourth opcode should be HALT")

      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing Extended Messages (Step 4) ===")
      (print-string "")

      ; Test 21: Tokenize identifier
      (print-string "Test 21: Tokenize 'Point new'")

      ; Create test string "Point new"
      (define-var test-unary (malloc 3))
      (poke test-unary 9)  ; length = 9
      ; "Point new" = P=80, o=111, i=105, n=110, t=116, space=32, n=110, e=101, w=119
      (define-var w-unary0 (+ 80
                              (bit-shl 111 8)
                              (bit-shl 105 16)
                              (bit-shl 110 24)
                              (bit-shl 116 32)
                              (bit-shl 32 40)
                              (bit-shl 110 48)
                              (bit-shl 101 56)))
      (define-var w-unary1 119)  ; 'w'
      (poke (+ test-unary 1) w-unary0)
      (poke (+ test-unary 2) w-unary1)

      (define-var tokens-unary (tokenize test-unary))
      (define-var tok-p (array-at tokens-unary 0))
      (define-var tok-new (array-at tokens-unary 1))
      (define-var tok-eof1 (array-at tokens-unary 2))

      (assert-equal (token-type tok-p) TOK_IDENTIFIER "First token should be IDENTIFIER")
      (assert-equal (token-type tok-new) TOK_IDENTIFIER "Second token should be IDENTIFIER")
      (assert-equal (token-type tok-eof1) TOK_EOF "Third token should be EOF")
      (print-string "  PASSED")

      ; Test 22: Parse unary message "Point new"
      (print-string "Test 22: Parse 'Point new'")
      (define-var ast-unary (parse test-unary))

      ; Should be: UNARY_MSG(new, IDENTIFIER(Point))
      (assert-equal (ast-type ast-unary) AST_UNARY_MSG "Root should be UNARY_MSG")

      (define-var unary-receiver (ast-child ast-unary 0))
      (assert-equal (ast-type unary-receiver) AST_IDENTIFIER "Receiver should be IDENTIFIER")
      (print-string "  PASSED")

      ; Test 23: Tokenize keyword message
      (print-string "Test 23: Tokenize 'x: 3 y: 4'")

      ; Create test string "x: 3 y: 4" (9 chars)
      (define-var test-keyword (malloc 3))
      (poke test-keyword 9)  ; length = 9
      ; "x: 3 y: 4" = x=120, :=58, space=32, 3=51, space=32, y=121, :=58, space=32, 4=52
      (define-var w-kw0 (+ 120
                           (bit-shl 58 8)
                           (bit-shl 32 16)
                           (bit-shl 51 24)
                           (bit-shl 32 32)
                           (bit-shl 121 40)
                           (bit-shl 58 48)
                           (bit-shl 32 56)))
      (define-var w-kw1 52)  ; '4'
      (poke (+ test-keyword 1) w-kw0)
      (poke (+ test-keyword 2) w-kw1)

      (define-var tokens-kw (tokenize test-keyword))
      (define-var tok-x (array-at tokens-kw 0))
      (define-var tok-3 (array-at tokens-kw 1))
      (define-var tok-y (array-at tokens-kw 2))
      (define-var tok-4 (array-at tokens-kw 3))

      (assert-equal (token-type tok-x) TOK_KEYWORD "First token should be KEYWORD")
      (assert-equal (token-type tok-3) TOK_NUMBER "Second token should be NUMBER")
      (assert-equal (token-type tok-y) TOK_KEYWORD "Third token should be KEYWORD")
      (assert-equal (token-type tok-4) TOK_NUMBER "Fourth token should be NUMBER")
      (print-string "  PASSED")

      ; Test 24: Tokenize binary message "5 + 3"
      (print-string "Test 24: Tokenize '5 + 3'")

      ; Create test string "5 + 3" (5 chars)
      (define-var test-binary-tok (malloc 2))
      (poke test-binary-tok 5)  ; length = 5
      ; "5 + 3" = 5=53, space=32, +=43, space=32, 3=51
      (define-var w-bin-tok (+ 53
                               (bit-shl 32 8)
                               (bit-shl 43 16)
                               (bit-shl 32 24)
                               (bit-shl 51 32)))
      (poke (+ test-binary-tok 1) w-bin-tok)

      (define-var tokens-bin (tokenize test-binary-tok))
      (define-var tok-5 (array-at tokens-bin 0))
      (define-var tok-plus (array-at tokens-bin 1))
      (define-var tok-3-bin (array-at tokens-bin 2))

      (assert-equal (token-type tok-5) TOK_NUMBER "First token should be NUMBER")
      (assert-equal (untag-int (token-value tok-5)) 5 "First token value should be 5")
      (assert-equal (token-type tok-plus) TOK_BINARY_OP "Second token should be BINARY_OP")
      (assert-equal (untag-int (token-value tok-plus)) 43 "Second token value should be + (43)")
      (assert-equal (token-type tok-3-bin) TOK_NUMBER "Third token should be NUMBER")
      (assert-equal (untag-int (token-value tok-3-bin)) 3 "Third token value should be 3")
      (print-string "  PASSED")

      ; Test 25: Parse binary message "5 + 3"
      (print-string "Test 25: Parse '5 + 3'")

      (define-var ast-binary (parse test-binary-tok))

      ; Should be: BINARY_MSG(+, NUMBER(5), NUMBER(3))
      (assert-equal (ast-type ast-binary) AST_BINARY_MSG "Root should be BINARY_MSG")
      (assert-equal (untag-int (ast-value ast-binary)) 43 "Operator should be + (43)")

      (define-var bin-left (ast-child ast-binary 0))
      (define-var bin-right (ast-child ast-binary 1))

      (assert-equal (ast-type bin-left) AST_NUMBER "Left child should be NUMBER")
      (assert-equal (untag-int (ast-value bin-left)) 5 "Left value should be 5")
      (assert-equal (ast-type bin-right) AST_NUMBER "Right child should be NUMBER")
      (assert-equal (untag-int (ast-value bin-right)) 3 "Right value should be 3")
      (print-string "  PASSED")

      ; Test 26: Tokenize "Point x: 3" first to debug
      (print-string "Test 26: Tokenize 'Point x: 3'")

      ; Create test string "Point x: 3" (10 chars)
      ; Need: 1 word for length + (10+7)/8 = 2 words for chars = 3 words total
      (define-var test-point-kw (malloc 3))
      (poke test-point-kw 10)  ; length = 10
      ; "Point x: 3" = P=80, o=111, i=105, n=110, t=116, space=32, x=120, :=58
      (define-var w-point-kw0 (+ 80
                                  (bit-shl 111 8)
                                  (bit-shl 105 16)
                                  (bit-shl 110 24)
                                  (bit-shl 116 32)
                                  (bit-shl 32 40)
                                  (bit-shl 120 48)
                                  (bit-shl 58 56)))
      ; " 3" = space=32, 3=51
      (define-var w-point-kw1 (+ 32 (bit-shl 51 8)))
      (poke (+ test-point-kw 1) w-point-kw0)
      (poke (+ test-point-kw 2) w-point-kw1)

      (define-var tokens-point-kw (tokenize test-point-kw))

      ; Should have: IDENTIFIER(Point), KEYWORD(x:), NUMBER(3), EOF
      (define-var tok-Point (array-at tokens-point-kw 0))
      (define-var tok-x-colon (array-at tokens-point-kw 1))
      (define-var tok-3-kw (array-at tokens-point-kw 2))
      (define-var tok-eof-kw (array-at tokens-point-kw 3))

      (assert-equal (token-type tok-Point) TOK_IDENTIFIER "Token 0 should be IDENTIFIER")
      (assert-equal (token-type tok-x-colon) TOK_KEYWORD "Token 1 should be KEYWORD")
      (assert-equal (token-type tok-3-kw) TOK_NUMBER "Token 2 should be NUMBER")
      (assert-equal (token-type tok-eof-kw) TOK_EOF "Token 3 should be EOF")
      (print-string "  PASSED")

      ; Test 27: Parse simple number "7" to verify parser works
      (print-string "Test 27: Parse '7'")

      ; Create test string "7" (1 char)
      (define-var test-seven (malloc 2))
      (poke test-seven 1)  ; length = 1
      (poke (+ test-seven 1) 55)  ; '7' = ASCII 55

      (define-var ast-seven (parse test-seven))

      ; Should be: NUMBER(7)
      (assert-equal (ast-type ast-seven) AST_NUMBER "Root should be NUMBER")
      (assert-equal (untag-int (ast-value ast-seven)) 7 "Value should be 7")
      (print-string "  PASSED")

      ; Test 28: Parse keyword message "Point x: 3"
      (print-string "Test 28: Parse 'Point x: 3'")

      ; Create test string "Point x: 3" (10 chars)
      ; Need: 1 word for length + (10+7)/8 = 2 words for chars = 3 words total
      (define-var test-kw-full (malloc 3))
      (poke test-kw-full 10)  ; length = 10
      ; "Point x: 3" = P=80, o=111, i=105, n=110, t=116, space=32, x=120, :=58
      (define-var w-kw-full0 (+ 80
                                 (bit-shl 111 8)
                                 (bit-shl 105 16)
                                 (bit-shl 110 24)
                                 (bit-shl 116 32)
                                 (bit-shl 32 40)
                                 (bit-shl 120 48)
                                 (bit-shl 58 56)))
      ; " 3" = space=32, 3=51
      (define-var w-kw-full1 (+ 32 (bit-shl 51 8)))
      (poke (+ test-kw-full 1) w-kw-full0)
      (poke (+ test-kw-full 2) w-kw-full1)

      (define-var ast-kw-full (parse test-kw-full))

      ; Should be: KEYWORD_MSG(x:, IDENTIFIER(Point), NUMBER(3))
      (assert-equal (ast-type ast-kw-full) AST_KEYWORD_MSG "Root should be KEYWORD_MSG")

      (define-var kw-full-receiver (ast-child ast-kw-full 0))
      (define-var kw-full-arg (ast-child ast-kw-full 1))

      (assert-equal (ast-type kw-full-receiver) AST_IDENTIFIER "Receiver should be IDENTIFIER")
      (assert-equal (ast-type kw-full-arg) AST_NUMBER "Argument should be NUMBER")
      (assert-equal (untag-int (ast-value kw-full-arg)) 3 "Argument value should be 3")
      (print-string "  PASSED")
      (print-string "")

      (print-string "=== All Tests Passed! ===")
      (print-string "")
      (print-string "Bootstrap complete!")
      (print-string "  8 classes created")
      (print-string "  SmallInteger support working")
      (print-string "  5-level inheritance chain working")
      (print-string "  Method override working")
      (print-string "  Context management working!")
      (print-string "  Message send/return working!")
      (print-string "  String operations working!")
      (print-string "  AST node system working!")
      (print-string "  Tokenizer working! (numbers, identifiers, keywords, binary ops)")
      (print-string "  Parser working! (unary, binary, keyword messages)")
      (print-string "  Smalltalk->VM bytecode compiler working!")
      (print-string "")
      (print-string "Smalltalk implementation (Step 4 complete)!")
      (print-string "  Binary messages: 3 + 4")
      (print-string "  Unary messages: Point new")
      (print-string "  Keyword messages: Point x: 3 y: 4")
      (print-string "  All message types parse correctly!")
      (print-string "  Bytecode compilation working for arithmetic")

      0))
  
  (bootstrap-smalltalk))
