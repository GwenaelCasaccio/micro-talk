; ===== Bootstrap Module =====
; Class hierarchy creation for Smalltalk runtime

(do
  ; ===== Bootstrap Function =====
  ; Creates core class hierarchy

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

      0))

  0)
