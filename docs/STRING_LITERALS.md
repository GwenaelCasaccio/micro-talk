# String Literals - No More ASCII Codes!

Write readable strings with double quotes instead of painful ASCII character codes.

## New Feature: String Literals

**Before:**
```lisp
(print-string (str 72 101 108 108 111 33 0 0))  ; "Hello!"
```

**After:**
```lisp
(print-string "Hello!")
```

Much better!

## Syntax

Simply enclose text in double quotes:

```lisp
"Hello World"
"Point(x, y)"
"Class: Array"
"Method: new"
"Variable x = "
```

### Escape Sequences

Supported escape sequences:

| Escape | Result      |
|--------|-------------|
| `\n`   | Newline     |
| `\t`   | Tab         |
| `\r`   | Carriage return |
| `\\`   | Backslash   |
| `\"`   | Double quote |

**Examples:**
```lisp
"Line 1\nLine 2"
"Tab\there"
"Quote: \"Hello\""
"Path: C:\\Users\\Name"
```

## Complete Example

```lisp
(do
  (print-string "=== Smalltalk Demo ===")
  (print-string "")
  
  (print-string "Creating Object class...")
  (define Object (new-class (tag-int 100) NULL (tag-int 0)))
  
  (print-string "Creating Point class...")
  (define Point (new-class (tag-int 200) Object (tag-int 2)))
  
  (print-string "Creating Point instance...")
  (define p1 (new-instance Point 2 0 SHAPE_FIXED))
  (print-string "Point address:")
  (print-int p1)
  
  (print-string "Setting x = 10, y = 20...")
  (slot-at-put p1 0 (tag-int 10))
  (slot-at-put p1 1 (tag-int 20))
  
  (print-string "Getting x:")
  (print-int (untag-int (slot-at p1 0)))
  (print-string "Getting y:")
  (print-int (untag-int (slot-at p1 1)))
  
  (print-string "Done!")
  0)
```

**Output:**
```
DEBUG_STR: === Smalltalk Demo ===
DEBUG_STR: 
DEBUG_STR: Creating Object class...
DEBUG_STR: Creating Point class...
DEBUG_STR: Creating Point instance...
DEBUG_STR: Point address:
DEBUG: 30016
DEBUG_STR: Setting x = 10, y = 20...
DEBUG_STR: Getting x:
DEBUG: 10
DEBUG_STR: Getting y:
DEBUG: 20
DEBUG_STR: Done!
```

## Usage Patterns

### Object Inspection

```lisp
(define (inspect-point p)
  (do
    (print-string "Point:")
    (print-string "  x = ")
    (print-int (untag-int (slot-at p 0)))
    (print-string "  y = ")
    (print-int (untag-int (slot-at p 1)))
    0))
```

### Array Display

```lisp
(define (print-array arr size)
  (do
    (print-string "Array[")
    (print-int size)
    (print-string "] = {")
    
    (for (i 0 size)
      (do
        (print-int (untag-int (indexed-at arr i)))
        (if (< i (- size 1))
            (print-string ", ")
            0)))
    
    (print-string "}")
    0))
```

### Error Messages

```lisp
(define (check-bounds index limit)
  (if (>= index limit)
      (do
        (print-string "Error: Index ")
        (print-int index)
        (print-string " out of bounds (limit: ")
        (print-int limit)
        (print-string ")")
        0)
      1))
```

### Class Names

```lisp
(define (print-class-name class-id)
  (do
    (if (= class-id 100)
        (print-string "Object")
        0)
    (if (= class-id 200)
        (print-string "Point")
        0)
    (if (= class-id 300)
        (print-string "Array")
        0)
    (if (= class-id 400)
        (print-string "Person")
        0)
    0))
```

### Method Tracing

```lisp
(define (method-enter name)
  (do
    (print-string ">> Entering: ")
    (print-string name)
    0))

(define (method-exit name result)
  (do
    (print-string "<< Exiting: ")
    (print-string name)
    (print-string " -> ")
    (print-int result)
    0))

; Usage:
(define (factorial n)
  (do
    (method-enter "factorial")
    (define result ...)
    (method-exit "factorial" result)
    result))
```

### Debug Sections

```lisp
(define (test-objects)
  (do
    (print-string "========================================")
    (print-string "  Object Creation Test")
    (print-string "========================================")
    
    (print-string "Step 1: Creating classes...")
    ; ...
    
    (print-string "Step 2: Creating instances...")
    ; ...
    
    (print-string "Step 3: Setting slots...")
    ; ...
    
    (print-string "========================================")
    (print-string "  Test Complete")
    (print-string "========================================")
    0))
```

## Comparison: Before vs After

### Before (ASCII Codes)

```lisp
(define (demo)
  (do
    (print-string (str 80 111 105 110 116 40 41 0))
    (print-string (str 120 58 32 0 0 0 0 0))
    (print-int x)
    (print-string (str 121 58 32 0 0 0 0 0))
    (print-int y)
    0))
```

**Problems:**
- Hard to read
- Easy to make mistakes
- Painful to write
- Can't see what will print

### After (String Literals)

```lisp
(define (demo)
  (do
    (print-string "Point()")
    (print-string "x: ")
    (print-int x)
    (print-string "y: ")
    (print-int y)
    0))
```

**Benefits:**
- Easy to read
- Clear intent
- Quick to write
- See exactly what will print

## Implementation Details

### Compilation

String literals are compiled into:
1. Memory allocation for string storage (in variable space)
2. Length storage (first word)
3. Character packing (8 bytes per word)
4. Address pushed to stack

### Memory Format

```
Address  | Content
---------|------------------
addr+0   | Length (in bytes)
addr+1   | Characters 0-7
addr+2   | Characters 8-15
...      | ...
```

Characters are packed little-endian (first character in low byte).

### Variable Allocation

Each string literal gets unique variable names:
- `__string_<address>__` for string base
- `__string_<address>___0`, `_1`, etc. for data words

### Bytecode

For `"Hello"` (5 bytes):
```
PUSH 5              ; Length
PUSH <addr>         ; String address
STORE               ; Store length

PUSH 0x6F6C6C6548  ; "Hello" packed
PUSH <addr+1>
STORE

PUSH <addr>         ; Return string address
```

## Performance

- **Compile time**: O(n) where n = string length
- **Runtime**: Already allocated, just address pushed
- **Memory**: 1 word per 8 characters + 1 word for length

## Advanced: String Builder

```lisp
(define (build-message name value)
  (do
    (print-string "User: ")
    (print-string name)
    (print-string ", Age: ")
    (print-int value)
    (print-string ", Status: ")
    (if (> value 18)
        (print-string "Adult")
        (print-string "Minor"))
    0))
```

## Full Smalltalk Example

```lisp
(do
  (print-string "=== Smalltalk Object Model Demo ===")
  (print-string "")
  
  (print-string "Creating Object class...")
  (define Object (new-class (tag-int 100) NULL (tag-int 0)))
  
  (print-string "Creating Point class (2 slots: x, y)...")
  (define Point (new-class (tag-int 200) Object (tag-int 2)))
  
  (print-string "Creating Point instance...")
  (define p1 (new-instance Point 2 0 SHAPE_FIXED))
  (print-string "Point address:")
  (print-int p1)
  
  (print-string "Setting x = 10, y = 20...")
  (slot-at-put p1 0 (tag-int 10))
  (slot-at-put p1 1 (tag-int 20))
  
  (print-string "Getting x:")
  (print-int (untag-int (slot-at p1 0)))
  (print-string "Getting y:")
  (print-int (untag-int (slot-at p1 1)))
  
  (print-string "")
  (print-string "Creating Array class...")
  (define Array (new-class (tag-int 300) Object (tag-int 0)))
  
  (print-string "Creating Array[5]...")
  (define arr (new-instance Array 0 5 SHAPE_INDEXABLE))
  
  (print-string "Filling array with values...")
  (indexed-at-put arr 0 (tag-int 100))
  (indexed-at-put arr 1 (tag-int 200))
  (indexed-at-put arr 2 (tag-int 300))
  
  (print-string "Array[0]:")
  (print-int (untag-int (indexed-at arr 0)))
  (print-string "Array[1]:")
  (print-int (untag-int (indexed-at arr 1)))
  (print-string "Array[2]:")
  (print-int (untag-int (indexed-at arr 2)))
  
  (print-string "")
  (print-string "=== Demo Complete! ===")
  0)
```

**Output:**
```
DEBUG_STR: === Smalltalk Object Model Demo ===
DEBUG_STR: 
DEBUG_STR: Creating Object class...
DEBUG_STR: Creating Point class (2 slots: x, y)...
DEBUG_STR: Creating Point instance...
DEBUG_STR: Point address:
DEBUG: 30016
DEBUG_STR: Setting x = 10, y = 20...
DEBUG_STR: Getting x:
DEBUG: 10
DEBUG_STR: Getting y:
DEBUG: 20
DEBUG_STR: 
DEBUG_STR: Creating Array class...
DEBUG_STR: Creating Array[5]...
DEBUG_STR: Filling array with values...
DEBUG_STR: Array[0]:
DEBUG: 100
DEBUG_STR: Array[1]:
DEBUG: 200
DEBUG_STR: Array[2]:
DEBUG: 300
DEBUG_STR: 
DEBUG_STR: === Demo Complete! ===
```

Perfect readability!

## Conclusion

String literals transform the debugging experience:

✓ **Readable code** - Know what strings say without ASCII lookup  
✓ **Quick writing** - Type what you want, not character codes  
✓ **Less errors** - No more off-by-one ASCII mistakes  
✓ **Professional output** - Clean, meaningful debug messages  
✓ **Better collaboration** - Others can understand your debug output

No more counting ASCII codes - just write `"Hello World"` and be done!
