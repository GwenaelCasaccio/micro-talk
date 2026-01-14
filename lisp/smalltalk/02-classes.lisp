; ===== Classes Module =====
; Class creation and hierarchy management

(do
  ; ===== Class Structure =====
  ; Class has 3 named slots:
  ; - Slot 0: name (tagged int)
  ; - Slot 1: superclass (OOP or NULL)
  ; - Slot 2: method dictionary (OOP or NULL)

  (define-func (new-class name superclass)
    (do
      (define-var class (new-instance (tag-int 987) 3 0))
      (slot-at-put class 0 name)
      (slot-at-put class 1 superclass)
      (slot-at-put class 2 NULL)
      class))

  (define-func (class-set-methods class dict)
    (slot-at-put class 2 dict))

  (define-func (get-class obj)
    (if (is-int obj)
        SmallInteger-class
        (peek obj)))

  (define-func (get-name class) (slot-at class 0))
  (define-func (get-super class) (slot-at class 1))
  (define-func (get-methods class) (slot-at class 2))

  0)
