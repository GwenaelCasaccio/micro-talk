# Smalltalk Method Lookup

Complete implementation of Smalltalk method lookup with inheritance chain traversal, method override, and proper failure handling.

## Features

✓ **Inheritance chain traversal** - Walk superclass chain  
✓ **Method override** - Subclass methods take precedence  
✓ **Dictionary lookup** - Fast hash-like method dictionary  
✓ **NULL on failure** - Return NULL when method not found  
✓ **Tagged selectors** - Use integers as method selectors

## Core Algorithm

```lisp
(define (lookup-method receiver selector)
  (do
    (define current-class (get-class receiver))
    (define found NULL)
    
    (while (> current-class NULL)
      (do
        (if (= found NULL)
            (do
              (define methods (get-methods current-class))
              (if (> methods NULL)
                  (set found (method-dict-lookup methods selector))
                  0)
              (if (= found NULL)
                  (set current-class (get-super current-class))
                  (set current-class NULL)))
            (set current-class NULL))))
    
    found))
```

## How It Works

1. **Get receiver's class** - Extract class from object header
2. **Start at receiver's class** - Begin lookup in most specific class
3. **Check method dictionary** - Look for selector in current class
4. **If found** - Return method address immediately
5. **If not found** - Move to superclass and repeat
6. **If no superclass** - Return NULL (method not found)

## Method Dictionary Structure

```
Dictionary:
[SIZE][sel0][code0][sel1][code1][sel2][code2]...

Example:
30000: 2        (size = 2 methods)
30001: 1        (selector: printString)
30002: 1000     (code address)
30003: 2        (selector: class)
30004: 2000     (code address)
```

## Class Structure

```
Class:
[NAME][SUPERCLASS][METHODS]

Example Object class:
30000: 100      (name)
30001: 0        (superclass = NULL)
30002: 30008    (methods dictionary)
```

## Test Results

```
=== Method Lookup Demo ===

Creating Object class...
  printString (sel:1) -> 1000
  class (sel:2) -> 2000

Creating Point class...
  x (sel:3) -> 3000
  y (sel:4) -> 4000
  printString (sel:1) -> 3100 (override)

Test 1: p>>x (sel:3)
  Result: 3000      ✓ Found in Point
  Expected: 3000

Test 2: p>>printString (sel:1)
  (override in Point)
  Result: 3100      ✓ Override works
  Expected: 3100

Test 3: p>>class (sel:2)
  (inherited from Object)
  Result: 2000      ✓ Inheritance works
  Expected: 2000

Test 4: p>>unknown (sel:99)
  Result: 0         ✓ Returns NULL
  Expected: 0

=== All Tests Passed! ===
```

## API Reference

### lookup-method

**Syntax:** `(lookup-method receiver selector)`

**Parameters:**
- `receiver` - Object receiving the message
- `selector` - Tagged integer selector

**Returns:**
- Method code address (tagged integer) if found
- `NULL` (0) if not found

**Example:**
```lisp
(define method (lookup-method my-point (tag-int 3)))
(if (> method NULL)
    (print-string "Method found!")
    (print-string "Method not found"))
```

### method-dict-lookup

**Syntax:** `(method-dict-lookup dict selector)`

Searches a single method dictionary for a selector.

**Parameters:**
- `dict` - Method dictionary address
- `selector` - Tagged integer selector

**Returns:**
- Method code address if found
- `NULL` if not found

### Helper Functions

```lisp
(get-class obj)         ; Get object's class
(get-super class)       ; Get class's superclass
(get-methods class)     ; Get class's method dictionary
```

## Creating Classes with Methods

```lisp
; Create base class
(define Object (new-class (tag-int 100) NULL))
(define obj-methods (new-method-dict 5))
(method-dict-add obj-methods (tag-int 1) (tag-int 1000))
(method-dict-add obj-methods (tag-int 2) (tag-int 2000))
(class-set-methods Object obj-methods)

; Create subclass
(define Point (new-class (tag-int 200) Object))
(define pt-methods (new-method-dict 5))
(method-dict-add pt-methods (tag-int 3) (tag-int 3000))
(method-dict-add pt-methods (tag-int 1) (tag-int 3100))  ; Override
(class-set-methods Point pt-methods)
```

## Inheritance Chain Example

```
Object (sel:1 -> 1000, sel:2 -> 2000)
   ^
   |
Point (sel:3 -> 3000, sel:1 -> 3100)
   ^
   |
instance

Lookup p>>x (sel:3):
  1. Check Point methods: Found 3000 ✓

Lookup p>>printString (sel:1):
  1. Check Point methods: Found 3100 ✓ (override)

Lookup p>>class (sel:2):
  1. Check Point methods: Not found
  2. Check Object methods: Found 2000 ✓

Lookup p>>unknown (sel:99):
  1. Check Point methods: Not found
  2. Check Object methods: Not found
  3. Check NULL superclass: Stop
  4. Return NULL
```

## Performance

- **Best case**: O(1) - Method in own class
- **Average case**: O(d) - d = depth in hierarchy
- **Worst case**: O(d*m) - d = depth, m = methods per class

With typical Smalltalk hierarchies (depth 3-5), lookup is very fast.

## Advanced: Method Caching

For production use, add a method cache:

```lisp
(define method-cache (new-array 256))

(define (cached-lookup receiver selector)
  (do
    (define hash (% selector 256))
    (define cached (array-at method-cache hash))
    
    (if (= cached NULL)
        (do
          (define method (lookup-method receiver selector))
          (array-at-put method-cache hash method)
          method)
        cached)))
```

## Integration with Message Sending

```lisp
(define (send receiver selector arg)
  (do
    (define method (lookup-method receiver selector))
    
    (if (> method NULL)
        (execute-method method receiver arg)
        (send receiver (tag-int 999) selector))))  ; doesNotUnderstand
```

## Error Handling

```lisp
(define (safe-send receiver selector)
  (do
    (define method (lookup-method receiver selector))
    
    (if (= method NULL)
        (do
          (print-string "Error: Method not found")
          (print-string "  Selector:")
          (print-int selector)
          (print-string "  Receiver class:")
          (print-int (get-class receiver))
          NULL)
        method)))
```

## Testing

```bash
g++ -std=c++17 -O2 -o test_fixed test_fixed.cpp
./test_fixed
```

## Conclusion

The method lookup system provides:

✓ **Complete inheritance** - Full superclass chain traversal  
✓ **Method override** - Subclasses can override superclass methods  
✓ **Fast lookup** - O(depth) average case  
✓ **Proper failure** - Returns NULL when method not found  
✓ **Production-ready** - Ready for full Smalltalk implementation

This is the foundation for a complete Smalltalk message sending system!
