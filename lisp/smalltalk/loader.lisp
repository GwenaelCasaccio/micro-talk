; ===== Smalltalk Module Loader =====
; Provides file loading utilities and string operations for Lisp programs.
;
; PREFERRED APPROACH: Load files via command line arguments:
;   ./build/lisp_vm file1.lisp file2.lisp file3.lisp
; Each file is compiled separately, so symbols from earlier files are
; available when compiling later files.
;
; Example - Load all Smalltalk modules:
;   ./build/lisp_vm \
;     lisp/smalltalk/00-runtime.lisp \
;     lisp/smalltalk/01-symbol-table.lisp \
;     lisp/smalltalk/02-classes.lisp \
;     lisp/smalltalk/03-methods.lisp \
;     lisp/smalltalk/04-tokenizer.lisp \
;     lisp/smalltalk/05-parser.lisp \
;     lisp/smalltalk/06-compiler.lisp \
;     lisp/smalltalk/07-bootstrap.lisp \
;     lisp/smalltalk/run-bootstrap.lisp
;
; RUNTIME LOADING: This file provides load-file for runtime file loading.
; Note: Symbols defined via eval/load-file are not available at compile
; time for subsequent code in the same file. Use command line loading
; when you need to reference loaded symbols.
;
; UTILITIES PROVIDED:
;   (string-length str)          - Get string length
;   (string-byte-at str idx)     - Get byte at index
;   (string-byte-set str idx v)  - Set byte at index
;   (string-alloc len)           - Allocate new string
;   (string-concat str1 str2)    - Concatenate two strings
;   (load-file path)             - Load and evaluate file

(do
  ; ===== Memory Layout =====
  ; Code segment is protected up to 134217728 (0x8000000)
  ; Use addresses above that for our buffers
  (define-var string-heap-ptr 150000000)
  (define-var load-buffer 160000000)

  ; ===== String Operations =====
  ; Strings are packed: [length][bytes packed 8 per word, little-endian]

  ; Get string length (number of characters)
  (define-func (string-length str)
    (peek str))

  ; Get byte at index from packed string
  (define-func (string-byte-at str idx)
    (do
      (define-var word-idx (+ str 1 (/ idx 8)))
      (define-var byte-pos (% idx 8))
      (bit-and (bit-shr (peek word-idx) (* byte-pos 8)) 255)))

  ; Set byte at index in packed string
  (define-func (string-byte-set str idx val)
    (do
      (define-var word-idx (+ str 1 (/ idx 8)))
      (define-var byte-pos (% idx 8))
      (define-var shift (* byte-pos 8))
      (define-var mask (bit-xor -1 (bit-shl 255 shift)))
      (define-var old-word (peek word-idx))
      (define-var new-word (bit-or (bit-and old-word mask)
                                   (bit-shl (bit-and val 255) shift)))
      (poke word-idx new-word)))

  ; Allocate a new string of given length
  (define-func (string-alloc len)
    (do
      (define-var words-needed (+ 1 (/ (+ len 7) 8)))
      (define-var result string-heap-ptr)
      (set string-heap-ptr (+ string-heap-ptr words-needed))
      ; Initialize length
      (poke result len)
      ; Zero out data words
      (for (i 1 words-needed)
        (poke (+ result i) 0))
      result))

  ; Concatenate two strings, returns new string address
  (define-func (string-concat str1 str2)
    (do
      (define-var len1 (string-length str1))
      (define-var len2 (string-length str2))
      (define-var total-len (+ len1 len2))
      (define-var result (string-alloc total-len))
      ; Copy str1
      (for (i 0 len1)
        (string-byte-set result i (string-byte-at str1 i)))
      ; Copy str2
      (for (i 0 len2)
        (string-byte-set result (+ len1 i) (string-byte-at str2 i)))
      result))

  ; ===== File Loading =====

  ; Load and evaluate a file
  ; Returns: result of eval on success, negative on error
  ;   -1 = file open failed
  ;   -2 = fsize failed
  ;   -3 = read failed
  (define-func (load-file path)
    (do
      ; Open file (O_RDONLY = 0)
      (define-var fd (c-call 2 path 0))
      (if (< fd 0)
          -1
          (do
            ; Get file size
            (define-var size (c-call 5 fd))
            (if (< size 0)
                (do (c-call 3 fd) -2)
                (do
                  ; Read file into buffer+1 (word 20001+)
                  ; This leaves word 20000 for the length
                  (define-var bytes-read (c-call 0 fd (+ load-buffer 1) size))

                  ; Close file
                  (c-call 3 fd)

                  (if (< bytes-read 0)
                      -3
                      (do
                        ; Set packed string length at buffer[0]
                        (poke load-buffer bytes-read)

                        ; Eval the file content
                        (eval load-buffer)))))))))

  0)
