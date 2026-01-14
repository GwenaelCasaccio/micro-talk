      ; Test 14.1: Initialize symbol table
      (print-string "Test 14.1: Initialize symbol table")
      (init-symbol-table)
      (assert-true (> symbol-table 0) "Symbol table should be initialized")
      (assert-equal symbol-count 0 "Symbol count should start at 0")
      (print-string "  PASSED")

      ; Test 14.2: Intern first selector
      (print-string "Test 14.2: Intern first selector")
      ; Create string "add" manually (a=97, d=100, d=100)
      (define-var str-add (malloc 2))
      (poke str-add 3)  ; length = 3
      (define-var w-add "add")  ; String literal address
      (define-var w-add-data (peek (+ w-add 1)))  ; Read packed chars
      (poke (+ str-add 1) w-add-data)

      (define-var sel-add (intern-selector str-add))
      (assert-equal (untag-int sel-add) 1 "First selector should be ID 1")
      (assert-equal symbol-count 1 "Symbol count should be 1")
      (print-string "  Interned 'add' as selector 1")
      (print-string "  PASSED")

      ; Test 14.3: Intern second selector
      (print-string "Test 14.3: Intern second selector")
      ; Create string "sub" manually (s=115, u=117, b=98)
      (define-var str-sub (malloc 2))
      (poke str-sub 3)  ; length = 3
      (define-var w-sub "sub")  ; String literal address
      (define-var w-sub-data (peek (+ w-sub 1)))  ; Read packed chars
      (poke (+ str-sub 1) w-sub-data)

      (define-var sel-sub (intern-selector str-sub))
      (print-string "  Interned 'sub' as selector:")
      (print-int (untag-int sel-sub))
      (print-string "  Symbol count:")
      (print-int symbol-count)
      (assert-equal (untag-int sel-sub) 2 "Second selector should be ID 2")
      (assert-equal symbol-count 2 "Symbol count should be 2")
      (print-string "  Interned 'sub' as selector 2")
      (print-string "  PASSED")

      ; Test 14.4: Re-intern existing selector
      (print-string "Test 14.4: Re-intern existing selector")
      ; Create another "add" string
      (define-var str-add2 (malloc 2))
      (poke str-add2 3)
      (poke (+ str-add2 1) w-add-data)  ; Use packed data, not address

      (define-var sel-add2 (intern-selector str-add2))
      (assert-equal (untag-int sel-add2) 1 "Should return existing ID 1")
      (assert-equal symbol-count 2 "Symbol count should still be 2")
      (print-string "  Re-interned 'add' returned selector 1")
      (print-string "  PASSED")

      ; Test 14.5: Lookup selector name
      (print-string "Test 14.5: Lookup selector name")
      (define-var looked-up-add (selector-name sel-add))
      (assert-true (> looked-up-add 0) "Should return string address")
      (assert-equal (peek looked-up-add) 3 "String should have length 3")
      (print-string "  Looked up selector 1, got string 'add'")
      (print-string "  PASSED")

      ; Test 14.6: Intern common selectors for SmallInteger
      (print-string "Test 14.6: Intern standard selectors")

      ; Create selector strings
      (define-var str-negated (malloc 2))
      (poke str-negated 7)  ; "negated" = 7 chars
      ; n=110, e=101, g=103, a=97, t=116, e=101, d=100
      (define-var w-neg "negated")  ; String literal address
      (define-var w-neg-data (peek (+ w-neg 1)))  ; Read packed chars
      (poke (+ str-negated 1) w-neg-data)

      (define-var sel-negated (intern-selector str-negated))
      (print-string "  Interned 'negated' as selector:")
      (print-int (untag-int sel-negated))

      ; We can use binary operators as selectors too
      (define-var str-plus (malloc 2))
      (poke str-plus 1)  ; "+" = 1 char
      (poke (+ str-plus 1) 43)  ; + = 43
      (define-var sel-plus (intern-selector str-plus))
      (print-string "  Interned '+' as selector:")
      (print-int (untag-int sel-plus))

      (print-string "  PASSED")
      (print-string "")

      (print-string "=== Testing String Operations ===")
      (print-string "")

