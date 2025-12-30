# String Printing for Better Debugging

Enhanced debugging support with `print-int` (renamed from `print`) and new `print-string` for human-readable output.

## New Features

### print-int (alias: print)

Print a 64-bit integer value (previously just `print`).

**Syntax:** `(print-int value)` or `(print value)`

```lisp
(print-int 42)           ; DEBUG: 42
(print-int 0xFF)         ; DEBUG: 255
(print-int heap-pointer) ; DEBUG: 30024
```

### print-string

Print a null-terminated or length-prefixed string from memory.

**Syntax:** `(print-string address)`

```lisp
(define msg (make-string))
(print-string msg)  ; DEBUG_STR: Hello World
```

**String format in memory:**
```
Address   | Content
----------|------------------
addr+0    | Length (bytes)
addr+1    | Characters 0-7 (packed)
addr+2    | Characters 8-15 (packed)
...       | ...
```

## String Helper Functions

### Simple 8-character strings

```lisp
(define (str s0 s1 s2 s3 s4 s5 s6 s7)
  (do
    (define s (malloc 2))
    (poke s 8)
    (poke (+ s 1) (+ s0
                     (bit-shl s1 8)
                     (bit-shl s2 16)
                     (bit-shl s3 24)
                     (bit-shl s4 32)
                     (bit-shl s5 40)
                     (bit-shl s6 48)
                     (bit-shl s7 56)))
    s))
```

**Usage:**
```lisp
(print-string (str 72 101 108 108 111 33 0 0))  ; "Hello!"
(print-string (str 87 111 114 108 100 33 0 0))  ; "World!"
```

### 16-character strings

```lisp
(define (str16 len s0 s1 s2 s3 s4 s5 s6 s7 s8 s9 s10 s11 s12 s13 s14 s15)
  (do
    (define s (malloc 3))
    (poke s len)
    (poke (+ s 1) (+ s0 (bit-shl s1 8) ... ))
    (poke (+ s 2) (+ s8 (bit-shl s9 8) ... ))
    s))
```

**Usage:**
```lisp
(print-string (str16 15 83 109 97 108 108 116 97 108
                         107 32 79 98 106 101 99 116))
; "Smalltalk Objec"
```

## Complete Example

```lisp
(do
  (define HEAP_START 30000)
  (define heap-pointer HEAP_START)
  
  (define (malloc size)
    (do
      (define result heap-pointer)
      (set heap-pointer (+ heap-pointer size))
      result))
  
  ; Helper to create 8-char strings
  (define (str s0 s1 s2 s3 s4 s5 s6 s7)
    (do
      (define s (malloc 2))
      (poke s 8)
      (poke (+ s 1) (+ s0
                       (bit-shl s1 8)
                       (bit-shl s2 16)
                       (bit-shl s3 24)
                       (bit-shl s4 32)
                       (bit-shl s5 40)
                       (bit-shl s6 48)
                       (bit-shl s7 56)))
      s))
  
  ; Create and print strings
  (print-string (str 72 101 108 108 111 33 0 0))    ; "Hello!"
  (print-string (str 87 111 114 108 100 33 0 0))    ; "World!"
  (print-string (str 80 111 105 110 116 40 41 0))   ; "Point()"
  (print-string (str 79 98 106 101 99 116 0 0))     ; "Object"
  (print-string (str 67 108 97 115 115 58 32 0))    ; "Class: "
  
  0)
```

**Output:**
```
DEBUG_STR: Hello!
DEBUG_STR: World!
DEBUG_STR: Point()
DEBUG_STR: Object
DEBUG_STR: Class: 
```

## ASCII Character Reference

Common characters for strings:

```
Char | ASCII | Char | ASCII | Char | ASCII
-----|-------|------|-------|------|------
' '  | 32    | '0'  | 48    | '@'  | 64
'!'  | 33    | '1'  | 49    | 'A'  | 65
'"'  | 34    | '2'  | 50    | 'B'  | 66
'('  | 40    | '9'  | 57    | 'Z'  | 90
')'  | 41    | ':'  | 58    | 'a'  | 97
','  | 44    | ';'  | 59    | 'b'  | 98
'.'  | 46    | '='  | 61    | 'z'  | 122
'/'  | 47    | '?'  | 63    | NUL  | 0
```

## Using with Smalltalk Objects

### Class Names

```lisp
(define (print-class-name class-id)
  (do
    (if (= class-id 100)
        (print-string (str 79 98 106 101 99 116 0 0))  ; "Object"
        0)
    (if (= class-id 200)
        (print-string (str 80 111 105 110 116 0 0 0))  ; "Point"
        0)
    (if (= class-id 300)
        (print-string (str 65 114 114 97 121 0 0 0))   ; "Array"
        0)
    0))
```

### Object Inspection

```lisp
(define (inspect-object obj)
  (do
    (print-string (str 79 98 106 101 99 116 58 32))    ; "Object: "
    (print-int obj)
    
    (print-string (str 67 108 97 115 115 58 32 0))     ; "Class: "
    (print-int (get-class obj))
    
    (print-string (str 83 104 97 112 101 58 32 0))     ; "Shape: "
    (print-int (get-shape obj))
    
    0))
```

**Output:**
```
DEBUG_STR: Object: 
DEBUG: 30022
DEBUG_STR: Class: 
DEBUG: 30012
DEBUG_STR: Shape: 
DEBUG: 0
```

### Slot Names

```lisp
(define (print-point-slot obj index)
  (do
    (if (= index 0)
        (print-string (str 120 58 32 0 0 0 0 0))       ; "x: "
        (print-string (str 121 58 32 0 0 0 0 0)))      ; "y: "
    (print-int (untag-int (slot-at obj index)))
    0))

; Usage:
(print-point-slot my-point 0)  ; "x: " then value
(print-point-slot my-point 1)  ; "y: " then value
```

### Array Display

```lisp
(define (print-array-element arr index)
  (do
    (print-string (str 91 0 0 0 0 0 0 0))              ; "["
    (print-int index)
    (print-string (str 93 61 32 0 0 0 0 0))            ; "]= "
    (print-int (untag-int (indexed-at arr index)))
    0))

; Usage:
(print-array-element my-array 0)
; Output:
; DEBUG_STR: [
; DEBUG: 0
; DEBUG_STR: ]= 
; DEBUG: 42
```

## Advanced: Message Names

For Smalltalk message sending:

```lisp
(define MSG_NEW 1)
(define MSG_ADD 2)
(define MSG_PRINT 3)

(define (print-message-name msg-id)
  (do
    (if (= msg-id MSG_NEW)
        (print-string (str 110 101 119 0 0 0 0 0))     ; "new"
        0)
    (if (= msg-id MSG_ADD)
        (print-string (str 97 100 100 58 0 0 0 0))     ; "add:"
        0)
    (if (= msg-id MSG_PRINT)
        (print-string (str 112 114 105 110 116 0 0 0)) ; "print"
        0)
    0))
```

## Performance

- `print-int`: O(1) - Just prints integer
- `print-string`: O(n) - n = string length
- Memory: 1 word per 8 characters + 1 word for length

## Benefits for Debugging

**Before (print only):**
```
DEBUG: 30022
DEBUG: 30012
DEBUG: 0
DEBUG: 2
```

**After (with print-string):**
```
DEBUG_STR: Point instance
DEBUG_STR: Class: Point
DEBUG_STR: Shape: fixed
DEBUG_STR: Slots: 2
```

Much easier to understand what's happening!

## Integration Example

```lisp
(define (new-point x y)
  (do
    (print-string (str 78 101 119 32 80 111 105 110))  ; "New Poin"
    (print-string (str 116 40 0 0 0 0 0 0))            ; "t("
    (print-int x)
    (print-string (str 44 32 0 0 0 0 0 0))             ; ", "
    (print-int y)
    (print-string (str 41 0 0 0 0 0 0 0))              ; ")"
    
    (define p (new-instance Point 2 0 SHAPE_FIXED))
    (slot-at-put p 0 (tag-int x))
    (slot-at-put p 1 (tag-int y))
    
    (print-string (str 67 114 101 97 116 101 100))     ; "Created"
    (print-int p)
    
    p))
```

**Output:**
```
DEBUG_STR: New Poin
DEBUG_STR: t(
DEBUG: 10
DEBUG_STR: , 
DEBUG: 20
DEBUG_STR: )
DEBUG_STR: Created
DEBUG: 30022
```

## Future Enhancements

Possible additions:
- `print-hex` - Print values in hexadecimal
- `print-bin` - Print values in binary
- `print-char` - Print single character
- `sprintf` - Format strings in memory
- String comparison functions
- String length function
- String concatenation

## Conclusion

With `print-int` and `print-string`, debugging becomes much more pleasant:

✓ **Clear output** - Know what values mean  
✓ **Better tracing** - Follow execution flow  
✓ **Object inspection** - Readable object dumps  
✓ **Message tracking** - See what messages are sent  
✓ **Easy diagnostics** - Quick problem identification

These primitives transform the debugging experience from cryptic numbers to readable, meaningful output!
