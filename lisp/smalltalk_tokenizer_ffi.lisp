; Smalltalk Tokenizer with FFI for I/O
; This version is designed to be transpiled to C++ and replace tokenizer_main.cpp
(do
  ; Add cstdlib for exit()
  (c++-include "cstdlib")

  ; Token type constants
  (define TOKEN_NUMBER 1)
  (define TOKEN_IDENTIFIER 2)
  (define TOKEN_KEYWORD 3)
  (define TOKEN_STRING 4)
  (define TOKEN_SYMBOL 5)
  (define TOKEN_OPERATOR 6)

  ; Character classification helpers
  (define (is-digit ch)
    (if (< ch 48) 0 (if (> ch 57) 0 1)))

  (define (is-letter ch)
    (if (< ch 65) 0 (if (<= ch 90) 1 (if (< ch 97) 0 (if (<= ch 122) 1 0)))))

  (define (is-ident ch)
    (if (is-letter ch) 1 (if (is-digit ch) 1 0)))

  (define (is-whitespace ch)
    (if (= ch 32) 1 (if (= ch 9) 1 (if (= ch 10) 1 (if (= ch 13) 1 0)))))

  ; Read source from file or use default
  (define source (c++ "([&]() {
      if (argc > 1) {
          std::ifstream file(argv[1]);
          if (!file) {
              std::cerr << \"Error: Cannot open file '\" << argv[1] << \"'\" << std::endl;
              std::exit(1);
          }
          std::stringstream buffer;
          buffer << file.rdbuf();
          std::string content = buffer.str();
          std::cout << \"=== Smalltalk Tokenizer ===\" << std::endl;
          std::cout << \"File: \" << argv[1] << std::endl;
          std::cout << \"Size: \" << content.length() << \" characters\" << std::endl;
          std::cout << std::endl;
          return content;
      } else {
          std::string content = \"obj message: 'hello' #symbol [ :x | x + 42 ] := 100\";
          std::cout << \"=== Smalltalk Tokenizer (default input) ===\" << std::endl;
          std::cout << std::endl;
          return content;
      }
  })()"))

  ; Get source length
  (define len (string-length source))
  (define pos 0)
  (define count 0)

  ; Print header
  (c++ "std::cout << \"Tokens:\" << std::endl")

  ; Main tokenization loop
  (while (< pos len)
    (do
      ; Skip whitespace
      (while (if (< pos len) (is-whitespace (char-at source pos)) 0)
        (set pos (+ pos 1)))

      (if (< pos len)
        (do
          (define ch (char-at source pos))
          (define start pos)
          (define token-type 0)

          ; Classify and scan token
          (if (is-digit ch)
            ; NUMBER
            (do
              (while (if (< pos len) (is-digit (char-at source pos)) 0)
                (set pos (+ pos 1)))
              (set token-type TOKEN_NUMBER))

            (if (is-letter ch)
              ; IDENTIFIER or KEYWORD
              (do
                (while (if (< pos len) (is-ident (char-at source pos)) 0)
                  (set pos (+ pos 1)))
                ; Check for trailing colon (keyword)
                (if (< pos len)
                  (if (= (char-at source pos) 58)
                    (do
                      (set pos (+ pos 1))
                      (set token-type TOKEN_KEYWORD))
                    (set token-type TOKEN_IDENTIFIER))
                  (set token-type TOKEN_IDENTIFIER)))

              (if (= ch 39)
                ; STRING
                (do
                  (set pos (+ pos 1))
                  (while (if (< pos len) (if (= (char-at source pos) 39) 0 1) 0)
                    (set pos (+ pos 1)))
                  (if (< pos len) (set pos (+ pos 1)) 0)
                  (set token-type TOKEN_STRING))

                (if (= ch 35)
                  ; SYMBOL
                  (do
                    (set pos (+ pos 1))
                    (while (if (< pos len) (is-ident (char-at source pos)) 0)
                      (set pos (+ pos 1)))
                    (set token-type TOKEN_SYMBOL))

                  ; OPERATOR or other
                  (do
                    (set pos (+ pos 1))
                    ; Check for := operator
                    (if (= ch 58)
                      (if (< pos len)
                        (if (= (char-at source pos) 61)
                          (set pos (+ pos 1))
                          0)
                        0)
                      0)
                    (set token-type TOKEN_OPERATOR))))))

          ; Print the token using FFI
          ; We capture Lisp variables into a C++ lambda
          (c++ "([&]() {
              static const char* TOKEN_NAMES[] = {
                  \"UNKNOWN   \", \"NUMBER    \", \"IDENTIFIER\", \"KEYWORD   \",
                  \"STRING    \", \"SYMBOL    \", \"OPERATOR  \"
              };
              std::string token_text = source.substr(start, pos - start);
              // Escape newlines for display
              for (size_t i = 0; i < token_text.length(); i++) {
                  if (token_text[i] == '\\n') {
                      token_text.replace(i, 1, \"\\\\n\");
                      i++;
                  }
              }
              std::cout << std::setw(3) << (count + 1) << \". \"
                        << TOKEN_NAMES[token_type] << \" [\"
                        << std::setw(3) << start << \"-\"
                        << std::setw(3) << pos << \"]: '\"
                        << token_text << \"'\" << std::endl;
              return 0LL;
          })()")

          (set count (+ count 1)))
        0)))

  ; Print summary
  (c++ "std::cout << std::endl")
  (c++ "std::cout << \"Total tokens: \" << count << std::endl")

  count)
