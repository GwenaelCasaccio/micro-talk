# Loop Constructs in Lisp

The Lisp compiler now supports `while` and `for` loops for iteration.

## While Loop

Execute a body repeatedly while a condition is true.

**Syntax:** `(while condition body...)`

```lisp
(while (< x 10)
  (do
    (print x)
    (set x (+ x 1))))
```

### Features

- Evaluates condition before each iteration
- Exits when condition becomes false (0)
- Can contain multiple body expressions
- Returns 0 (while loops don't produce meaningful values)

### Examples

**Simple counter:**
```lisp
(do
  (define counter 0)
  (while (< counter 5)
    (do
      (print counter)
      (set counter (+ counter 1))))
  counter)
; Prints: 0, 1, 2, 3, 4
; Returns: 5
```

**Accumulator:**
```lisp
(do
  (define sum 0)
  (define i 1)
  (while (< i 11)
    (do
      (set sum (+ sum i))
      (set i (+ i 1))))
  sum)
; Returns: 55 (sum of 1..10)
```

**Factorial:**
```lisp
(do
  (define n 5)
  (define result 1)
  (define i 1)
  (while (< i (+ n 1))
    (do
      (set result (* result i))
      (set i (+ i 1))))
  result)
; Returns: 120 (5!)
```

**Fibonacci:**
```lisp
(do
  (define a 0)
  (define b 1)
  (define n 10)
  (define i 0)
  (while (< i n)
    (do
      (define temp b)
      (set b (+ a b))
      (set a temp)
      (set i (+ i 1))))
  a)
; Returns: 55 (10th Fibonacci number)
```

**GCD Algorithm:**
```lisp
(do
  (define a 48)
  (define b 18)
  (while (> b 0)
    (do
      (define temp b)
      (set b (% a b))
      (set a temp)))
  a)
; Returns: 6 (GCD of 48 and 18)
```

## For Loop

Iterate a variable from start to end (exclusive).

**Syntax:** `(for (variable start end) body...)`

```lisp
(for (i 0 10)
  (print i))
; Prints: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
```

### Features

- Loop variable automatically increments by 1
- Iterates from `start` (inclusive) to `end` (exclusive)
- Loop variable is scoped to the loop body
- Returns 0 (for loops don't produce meaningful values)

### Examples

**Simple iteration:**
```lisp
(for (i 0 5)
  (print i))
; Prints: 0, 1, 2, 3, 4
```

**With accumulator:**
```lisp
(do
  (define sum 0)
  (for (i 1 11)
    (set sum (+ sum i)))
  sum)
; Returns: 55 (sum of 1..10)
```

**Factorial:**
```lisp
(do
  (define result 1)
  (for (i 1 6)
    (set result (* result i)))
  result)
; Returns: 120 (5!)
```

**Nested loops:**
```lisp
(do
  (define sum 0)
  (for (i 1 4)
    (for (j 1 4)
      (set sum (+ sum 1))))
  sum)
; Returns: 9 (3 * 3 iterations)
```

**Power function:**
```lisp
(define (power base exp)
  (do
    (define result 1)
    (for (i 0 exp)
      (set result (* result base)))
    result))

(power 2 10)
; Returns: 1024 (2^10)
```

## Loops in Functions

Loops work seamlessly within function definitions:

```lisp
; Factorial using while
(define (factorial n)
  (do
    (define result 1)
    (define i 1)
    (while (< i (+ n 1))
      (do
        (set result (* result i))
        (set i (+ i 1))))
    result))

(factorial 6)  ; Returns: 720

; Sum range using for
(define (sum-range start end)
  (do
    (define total 0)
    (for (i start end)
      (set total (+ total i)))
    total))

(sum-range 1 11)  ; Returns: 55
```

## Loops with Tagging

Loops are perfect for batch processing tagged values:

```lisp
; Tag multiple integers
(do
  (define TAG_INT 1)
  (define (tag-int v) (bit-or (bit-shl v 3) TAG_INT))
  (define last 0)
  (for (i 0 5)
    (do
      (define tagged (tag-int i))
      (print tagged)  ; Prints: 1, 9, 17, 25, 33
      (set last tagged)))
  last)

; Process array of tagged integers
(do
  (define TAG_INT 1)
  (define (tag-int v) (bit-or (bit-shl v 3) TAG_INT))
  (define sum 0)
  (for (i 1 6)
    (do
      (define tagged (tag-int i))
      (define untagged (bit-ashr tagged 3))
      (set sum (+ sum untagged))))
  sum)  ; Returns: 15 (1+2+3+4+5)
```

## Loop Patterns

### While vs For

Use `while` when:
- Condition is complex
- Need to check condition mid-loop
- Increment is not always +1
- Don't know iteration count in advance

Use `for` when:
- Simple counting loop
- Know iteration count in advance
- Increment by 1 is sufficient
- Cleaner syntax for common case

### Early Exit

Neither loop supports `break`. To exit early:

**While:** Make condition false
```lisp
(do
  (define x 0)
  (define done 0)
  (while (= done 0)
    (do
      (set x (+ x 1))
      (if (> x 100)
          (set done 1)
          0))))
```

**For:** Can't exit early (will complete all iterations)

### Infinite Loops

Be careful with while loops - they can run forever:

```lisp
; This will never terminate!
(while 1
  (print 42))

; Always ensure condition eventually becomes false
(do
  (define i 0)
  (while (< i 10)  ; Good - i increases, condition becomes false
    (set i (+ i 1))))
```

## Implementation Details

### While Loop Compilation

```
loop_start:
  <evaluate condition>
  JZ loop_end          ; Jump if zero (false)
  <body>
  JMP loop_start       ; Jump back
loop_end:
  PUSH 0               ; Return value
```

### For Loop Compilation

```
<evaluate start>       ; Initialize loop variable
STORE var
<evaluate end>         ; Store end value
STORE __for_end__
loop_start:
  LOAD var
  LOAD __for_end__
  LT                   ; var < end?
  JZ loop_end
  <body>
  LOAD var
  PUSH 1
  ADD                  ; var + 1
  STORE var
  JMP loop_start
loop_end:
  PUSH 0               ; Return value
```

### Scoping

- `for` creates a new scope for the loop variable
- Loop variable is not accessible after the loop
- Variables defined in loop body are local to that iteration

## Performance

Loops compile to efficient bytecode:
- `while`: 3 instructions overhead per iteration (condition, JZ, JMP)
- `for`: 8 instructions overhead per iteration (load, compare, increment, jump)

Both are suitable for hundreds or thousands of iterations.

## Comparison with Other Features

| Feature | Use Case | Scoping |
|---------|----------|---------|
| `do` | Sequential execution | No new scope |
| `let` | Local variables | New scope |
| `while` | Conditional iteration | No new scope |
| `for` | Counted iteration | New scope for loop var |
| `function` | Reusable code | New scope for parameters |

## Testing

Run the test suite:

```bash
make loops
./test_loops
```

This tests:
- While loops (counters, accumulators, algorithms)
- For loops (simple, nested, with accumulators)
- Loops in functions
- Loops with tagging operations
- Complex algorithms (Fibonacci, GCD, power)

## Common Patterns

**Sum of range:**
```lisp
(define sum 0)
(for (i start end)
  (set sum (+ sum i)))
```

**Product of range:**
```lisp
(define product 1)
(for (i start end)
  (set product (* product i)))
```

**Count occurrences:**
```lisp
(define count 0)
(for (i 0 n)
  (if (condition i)
      (set count (+ count 1))
      0))
```

**Search:**
```lisp
(define found 0)
(define i 0)
(while (= found 0)
  (do
    (if (= (get-item i) target)
        (set found 1)
        0)
    (set i (+ i 1))))
```

## Future Enhancements

Possible additions:
- `break` statement for early exit
- `continue` statement to skip iteration
- `for-each` for iterating over collections
- `loop` with explicit `(break value)` for return
- Step parameter for `for`: `(for (i 0 10 2) ...)` 
- Downward counting: `(for (i 10 0 -1) ...)`
