;; Simple tagging/untagging examples using only basic Lisp operations
;; No define, no lambda - just arithmetic and bitwise operations

;; Tag scheme:
;; TAG_MASK = 7 (0b111)
;; TAG_INT = 1 (0b001)
;; TAG_OOP = 0 (0b000)

;; Example 1: Tag integer 42
;; Result: (42 << 3) | 1 = 336 | 1 = 337
(bit-or (bit-shl 42 3) 1)

;; Example 2: Untag integer 337
;; Result: 337 >> 3 = 42
(bit-ashr 337 3)

;; Example 3: Complete round-trip for integer 99
(do
  (print (bit-or (bit-shl 99 3) 1))
  (bit-ashr (bit-or (bit-shl 99 3) 1) 3))

;; Example 4: Tag OOP address 16384 (heap start)
;; Result: 16384 | 0 = 16384 (no change, tag is 0)
(bit-or 16384 0)

;; Example 5: Untag OOP
;; Clear lower 3 bits: addr & ~7
(bit-and 16384 (bit-xor -1 7))

;; Example 6: Check if value is tagged integer
;; Extract tag and compare with TAG_INT (1)
(do
  (print (bit-and 337 7))
  (= (bit-and 337 7) 1))

;; Example 7: Check if value is OOP
;; Extract tag and compare with TAG_OOP (0)
(do
  (print (bit-and 16384 7))
  (= (bit-and 16384 7) 0))

;; Example 8: Tag negative integer -42
;; Result: (-42 << 3) | 1 = -336 | 1 = -335
(bit-or (bit-shl -42 3) 1)

;; Example 9: Untag negative integer -335
;; Result: -335 >> 3 = -42 (arithmetic shift preserves sign)
(bit-ashr -335 3)

;; Example 10: Complete tagging demo
(do
  (print 1000)
  (print (bit-or (bit-shl 42 3) 1))
  (print (bit-ashr 337 3))
  (print (= (bit-and 337 7) 1))
  (print (bit-or 32768 0))
  (print (bit-and 32768 (bit-xor -1 7)))
  (print (= (bit-and 32768 7) 0))
  9999)
