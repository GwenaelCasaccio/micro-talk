; ===== Tokenizer Module =====
; String operations, character classification, tokenizer

(do
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

  0)
