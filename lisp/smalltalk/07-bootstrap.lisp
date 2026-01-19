; ===== Bootstrap Module =====
; Minimal Smalltalk class hierarchy creation
;
; Class Hierarchy:
;   ProtoObject
;     Object
;       Behavior
;         ClassDescription
;           Class
;           Metaclass
;       Magnitude
;         Number
;           SmallInteger
;       Collection
;         SequenceableCollection
;           ArrayedCollection
;             Array
;             String
;             Symbol
;           OrderedCollection
;         HashedCollection
;           Set
;           Dictionary
;             MethodDictionary
;       CompiledMethod

(do
  ; ===== Global Class Variables =====
  ; These are set during bootstrap and accessible throughout the system
  (define-var ProtoObject-class NULL)
  (define-var Object-class NULL)
  (define-var Behavior-class NULL)
  (define-var ClassDescription-class NULL)
  (define-var Class-class NULL)
  (define-var Metaclass-class NULL)
  (define-var Magnitude-class NULL)
  (define-var Number-class NULL)
  ; SmallInteger-class already defined in 00-runtime.lisp
  (define-var Collection-class NULL)
  (define-var SequenceableCollection-class NULL)
  (define-var ArrayedCollection-class NULL)
  ; Array already defined in 00-runtime.lisp
  (define-var String-class NULL)
  (define-var Symbol-class NULL)
  (define-var OrderedCollection-class NULL)
  (define-var HashedCollection-class NULL)
  (define-var Set-class NULL)
  (define-var Dictionary-class NULL)
  (define-var MethodDictionary-class NULL)
  (define-var CompiledMethod-class NULL)

  ; ===== Class Name IDs =====
  ; Used for debugging and reflection
  (define-var NAME_PROTO_OBJECT 1)
  (define-var NAME_OBJECT 2)
  (define-var NAME_BEHAVIOR 3)
  (define-var NAME_CLASS_DESC 4)
  (define-var NAME_CLASS 5)
  (define-var NAME_METACLASS 6)
  (define-var NAME_MAGNITUDE 10)
  (define-var NAME_NUMBER 11)
  (define-var NAME_SMALL_INTEGER 12)
  (define-var NAME_COLLECTION 20)
  (define-var NAME_SEQ_COLLECTION 21)
  (define-var NAME_ARRAYED_COLL 22)
  (define-var NAME_ARRAY 23)
  (define-var NAME_STRING 24)
  (define-var NAME_SYMBOL 25)
  (define-var NAME_ORDERED_COLL 26)
  (define-var NAME_HASHED_COLL 30)
  (define-var NAME_SET 31)
  (define-var NAME_DICTIONARY 32)
  (define-var NAME_METHOD_DICT 33)
  (define-var NAME_COMPILED_METHOD 40)
  (define-var NAME_AST_NODE 100)
  (define-var NAME_TOKEN 101)

  ; ===== Selector IDs =====
  ; Core selectors used by the system
  (define-var SEL_CLASS 10)           ; class
  (define-var SEL_IDENTITY_EQ 20)     ; ==
  (define-var SEL_IDENTITY_NEQ 21)    ; ~~
  (define-var SEL_YOURSELF 22)        ; yourself
  (define-var SEL_EQUAL 23)           ; =
  (define-var SEL_NOT_EQUAL 24)       ; ~=
  (define-var SEL_HASH 25)            ; hash
  (define-var SEL_IS_NIL 26)          ; isNil
  (define-var SEL_NOT_NIL 27)         ; notNil
  (define-var SEL_SUPERCLASS 30)      ; superclass
  (define-var SEL_METHOD_DICT 31)     ; methodDict
  (define-var SEL_NEW 32)             ; new
  (define-var SEL_NEW_SIZE 33)        ; new:
  (define-var SEL_BASIC_NEW 34)       ; basicNew
  (define-var SEL_BASIC_NEW_SIZE 35)  ; basicNew:
  (define-var SEL_LT 40)              ; <
  (define-var SEL_GT 41)              ; >
  (define-var SEL_LTE 42)             ; <=
  (define-var SEL_GTE 43)             ; >=
  (define-var SEL_PLUS 50)            ; +
  (define-var SEL_MINUS 51)           ; -
  (define-var SEL_TIMES 52)           ; *
  (define-var SEL_DIVIDE 53)          ; /
  (define-var SEL_MOD 54)             ; \\
  (define-var SEL_BIT_AND 55)         ; bitAnd:
  (define-var SEL_BIT_OR 56)          ; bitOr:
  (define-var SEL_BIT_XOR 57)         ; bitXor:
  (define-var SEL_BIT_SHIFT 58)       ; bitShift:
  (define-var SEL_NEGATED 59)         ; negated
  (define-var SEL_SIZE 60)            ; size
  (define-var SEL_IS_EMPTY 61)        ; isEmpty
  (define-var SEL_NOT_EMPTY 62)       ; notEmpty
  (define-var SEL_DO 63)              ; do:
  (define-var SEL_INCLUDES 64)        ; includes:
  (define-var SEL_AT 70)              ; at:
  (define-var SEL_AT_PUT 71)          ; at:put:
  (define-var SEL_FIRST 72)           ; first
  (define-var SEL_LAST 73)            ; last
  (define-var SEL_ADD 80)             ; add:
  (define-var SEL_REMOVE 81)          ; remove:
  (define-var SEL_ADD_FIRST 82)       ; addFirst:
  (define-var SEL_ADD_LAST 83)        ; addLast:
  (define-var SEL_REMOVE_FIRST 84)    ; removeFirst
  (define-var SEL_REMOVE_LAST 85)     ; removeLast
  (define-var SEL_AT_IFABSENT 90)     ; at:ifAbsent:
  (define-var SEL_AT_PUT_IFABSENT 91) ; at:put:ifAbsent:
  (define-var SEL_KEYS 92)            ; keys
  (define-var SEL_VALUES 93)          ; values
  (define-var SEL_SELECTOR 100)       ; selector
  (define-var SEL_BYTECODES 101)      ; bytecodes
  (define-var SEL_LITERALS 102)       ; literals
  (define-var SEL_NUM_ARGS 103)       ; numArgs
  (define-var SEL_NUM_TEMPS 104)      ; numTemps
  (define-var SEL_COPY_FROM_TO 110)   ; copyFrom:to:
  (define-var SEL_AS_STRING 111)      ; asString
  (define-var SEL_AS_SYMBOL 112)      ; asSymbol
  (define-var SEL_AS_ARRAY 113)       ; asArray
  (define-var SEL_CONCAT 114)         ; ,

  ; ===== Bootstrap Function =====
  ; Creates the complete minimal class hierarchy

  (define-func (bootstrap-smalltalk)
    (do
      (print-string "=== Smalltalk Bootstrap ===")
      (print-string "")

      ; ===== Core Root Classes =====
      (print-string "Creating root classes...")

      ; ProtoObject - the absolute root, minimal protocol
      (set ProtoObject-class (new-class (tag-int NAME_PROTO_OBJECT) NULL))
      (define-var proto-methods (new-method-dict 5))
      (method-dict-add proto-methods (tag-int SEL_CLASS) (tag-int 10000))
      (class-set-methods ProtoObject-class proto-methods)
      (print-string "  ProtoObject")

      ; Object - standard root with full protocol
      (set Object-class (new-class (tag-int NAME_OBJECT) ProtoObject-class))
      (define-var obj-methods (new-method-dict 15))
      (method-dict-add obj-methods (tag-int SEL_IDENTITY_EQ) (tag-int 20000))
      (method-dict-add obj-methods (tag-int SEL_IDENTITY_NEQ) (tag-int 20001))
      (method-dict-add obj-methods (tag-int SEL_YOURSELF) (tag-int 20002))
      (method-dict-add obj-methods (tag-int SEL_EQUAL) (tag-int 20003))
      (method-dict-add obj-methods (tag-int SEL_NOT_EQUAL) (tag-int 20004))
      (method-dict-add obj-methods (tag-int SEL_HASH) (tag-int 20005))
      (method-dict-add obj-methods (tag-int SEL_IS_NIL) (tag-int 20006))
      (method-dict-add obj-methods (tag-int SEL_NOT_NIL) (tag-int 20007))
      (class-set-methods Object-class obj-methods)
      (print-string "  Object")

      ; ===== Behavior Hierarchy =====
      (print-string "Creating behavior classes...")

      ; Behavior - defines the minimal protocol for all class-like objects
      (set Behavior-class (new-class (tag-int NAME_BEHAVIOR) Object-class))
      (define-var beh-methods (new-method-dict 10))
      (method-dict-add beh-methods (tag-int SEL_SUPERCLASS) (tag-int 30000))
      (method-dict-add beh-methods (tag-int SEL_METHOD_DICT) (tag-int 30001))
      (method-dict-add beh-methods (tag-int SEL_NEW) (tag-int 30002))
      (method-dict-add beh-methods (tag-int SEL_NEW_SIZE) (tag-int 30003))
      (method-dict-add beh-methods (tag-int SEL_BASIC_NEW) (tag-int 30004))
      (method-dict-add beh-methods (tag-int SEL_BASIC_NEW_SIZE) (tag-int 30005))
      (class-set-methods Behavior-class beh-methods)
      (print-string "  Behavior")

      ; ClassDescription - common protocol for Class and Metaclass
      (set ClassDescription-class (new-class (tag-int NAME_CLASS_DESC) Behavior-class))
      (print-string "  ClassDescription")

      ; Class - describes a regular class
      (set Class-class (new-class (tag-int NAME_CLASS) ClassDescription-class))
      (print-string "  Class")

      ; Metaclass - describes the class of a class
      (set Metaclass-class (new-class (tag-int NAME_METACLASS) ClassDescription-class))
      (print-string "  Metaclass")

      ; ===== Magnitude Hierarchy =====
      (print-string "Creating magnitude classes...")

      ; Magnitude - objects that can be compared
      (set Magnitude-class (new-class (tag-int NAME_MAGNITUDE) Object-class))
      (define-var mag-methods (new-method-dict 10))
      (method-dict-add mag-methods (tag-int SEL_LT) (tag-int 40000))
      (method-dict-add mag-methods (tag-int SEL_GT) (tag-int 40001))
      (method-dict-add mag-methods (tag-int SEL_LTE) (tag-int 40002))
      (method-dict-add mag-methods (tag-int SEL_GTE) (tag-int 40003))
      (class-set-methods Magnitude-class mag-methods)
      (print-string "  Magnitude")

      ; Number - numeric magnitudes
      (set Number-class (new-class (tag-int NAME_NUMBER) Magnitude-class))
      (define-var num-methods (new-method-dict 15))
      (method-dict-add num-methods (tag-int SEL_PLUS) (tag-int 50000))
      (method-dict-add num-methods (tag-int SEL_MINUS) (tag-int 50001))
      (method-dict-add num-methods (tag-int SEL_TIMES) (tag-int 50002))
      (method-dict-add num-methods (tag-int SEL_DIVIDE) (tag-int 50003))
      (method-dict-add num-methods (tag-int SEL_MOD) (tag-int 50004))
      (method-dict-add num-methods (tag-int SEL_NEGATED) (tag-int 50005))
      (class-set-methods Number-class num-methods)
      (print-string "  Number")

      ; SmallInteger - tagged immediate integers
      (define-var SmallInteger (new-class (tag-int NAME_SMALL_INTEGER) Number-class))
      (set SmallInteger-class SmallInteger)
      (define-var int-methods (new-method-dict 15))
      (method-dict-add int-methods (tag-int SEL_PLUS) (tag-int 51000))
      (method-dict-add int-methods (tag-int SEL_MINUS) (tag-int 51001))
      (method-dict-add int-methods (tag-int SEL_TIMES) (tag-int 51002))
      (method-dict-add int-methods (tag-int SEL_DIVIDE) (tag-int 51003))
      (method-dict-add int-methods (tag-int SEL_MOD) (tag-int 51004))
      (method-dict-add int-methods (tag-int SEL_BIT_AND) (tag-int 51005))
      (method-dict-add int-methods (tag-int SEL_BIT_OR) (tag-int 51006))
      (method-dict-add int-methods (tag-int SEL_BIT_XOR) (tag-int 51007))
      (method-dict-add int-methods (tag-int SEL_BIT_SHIFT) (tag-int 51008))
      (class-set-methods SmallInteger int-methods)
      (print-string "  SmallInteger")

      ; ===== Collection Hierarchy =====
      (print-string "Creating collection classes...")

      ; Collection - abstract collection protocol
      (set Collection-class (new-class (tag-int NAME_COLLECTION) Object-class))
      (define-var coll-methods (new-method-dict 10))
      (method-dict-add coll-methods (tag-int SEL_SIZE) (tag-int 60000))
      (method-dict-add coll-methods (tag-int SEL_IS_EMPTY) (tag-int 60001))
      (method-dict-add coll-methods (tag-int SEL_NOT_EMPTY) (tag-int 60002))
      (method-dict-add coll-methods (tag-int SEL_DO) (tag-int 60003))
      (method-dict-add coll-methods (tag-int SEL_INCLUDES) (tag-int 60004))
      (class-set-methods Collection-class coll-methods)
      (print-string "  Collection")

      ; SequenceableCollection - collections with ordered elements
      (set SequenceableCollection-class (new-class (tag-int NAME_SEQ_COLLECTION) Collection-class))
      (define-var seq-methods (new-method-dict 10))
      (method-dict-add seq-methods (tag-int SEL_AT) (tag-int 61000))
      (method-dict-add seq-methods (tag-int SEL_AT_PUT) (tag-int 61001))
      (method-dict-add seq-methods (tag-int SEL_FIRST) (tag-int 61002))
      (method-dict-add seq-methods (tag-int SEL_LAST) (tag-int 61003))
      (method-dict-add seq-methods (tag-int SEL_COPY_FROM_TO) (tag-int 61004))
      (class-set-methods SequenceableCollection-class seq-methods)
      (print-string "  SequenceableCollection")

      ; ArrayedCollection - fixed-size indexed collections
      (set ArrayedCollection-class (new-class (tag-int NAME_ARRAYED_COLL) SequenceableCollection-class))
      (print-string "  ArrayedCollection")

      ; Array - general purpose array
      (set Array (new-class (tag-int NAME_ARRAY) ArrayedCollection-class))
      (define-var arr-methods (new-method-dict 10))
      (method-dict-add arr-methods (tag-int SEL_AT) (tag-int 62000))
      (method-dict-add arr-methods (tag-int SEL_AT_PUT) (tag-int 62001))
      (method-dict-add arr-methods (tag-int SEL_SIZE) (tag-int 62002))
      (method-dict-add arr-methods (tag-int SEL_AS_ARRAY) (tag-int 62003))
      (class-set-methods Array arr-methods)
      (print-string "  Array")

      ; String - byte-indexed character sequence
      (set String-class (new-class (tag-int NAME_STRING) ArrayedCollection-class))
      (define-var str-methods (new-method-dict 15))
      (method-dict-add str-methods (tag-int SEL_AT) (tag-int 63000))
      (method-dict-add str-methods (tag-int SEL_AT_PUT) (tag-int 63001))
      (method-dict-add str-methods (tag-int SEL_SIZE) (tag-int 63002))
      (method-dict-add str-methods (tag-int SEL_EQUAL) (tag-int 63003))
      (method-dict-add str-methods (tag-int SEL_HASH) (tag-int 63004))
      (method-dict-add str-methods (tag-int SEL_AS_STRING) (tag-int 63005))
      (method-dict-add str-methods (tag-int SEL_AS_SYMBOL) (tag-int 63006))
      (method-dict-add str-methods (tag-int SEL_CONCAT) (tag-int 63007))
      (class-set-methods String-class str-methods)
      (print-string "  String")

      ; Symbol - unique interned string (identity comparison)
      (set Symbol-class (new-class (tag-int NAME_SYMBOL) ArrayedCollection-class))
      (define-var sym-methods (new-method-dict 10))
      (method-dict-add sym-methods (tag-int SEL_AT) (tag-int 64000))
      (method-dict-add sym-methods (tag-int SEL_SIZE) (tag-int 64001))
      (method-dict-add sym-methods (tag-int SEL_AS_STRING) (tag-int 64002))
      (method-dict-add sym-methods (tag-int SEL_AS_SYMBOL) (tag-int 64003))
      (class-set-methods Symbol-class sym-methods)
      (print-string "  Symbol")

      ; OrderedCollection - growable indexed collection
      (set OrderedCollection-class (new-class (tag-int NAME_ORDERED_COLL) SequenceableCollection-class))
      (define-var oc-methods (new-method-dict 15))
      (method-dict-add oc-methods (tag-int SEL_AT) (tag-int 65000))
      (method-dict-add oc-methods (tag-int SEL_AT_PUT) (tag-int 65001))
      (method-dict-add oc-methods (tag-int SEL_SIZE) (tag-int 65002))
      (method-dict-add oc-methods (tag-int SEL_ADD) (tag-int 65003))
      (method-dict-add oc-methods (tag-int SEL_ADD_FIRST) (tag-int 65004))
      (method-dict-add oc-methods (tag-int SEL_ADD_LAST) (tag-int 65005))
      (method-dict-add oc-methods (tag-int SEL_REMOVE_FIRST) (tag-int 65006))
      (method-dict-add oc-methods (tag-int SEL_REMOVE_LAST) (tag-int 65007))
      (class-set-methods OrderedCollection-class oc-methods)
      (print-string "  OrderedCollection")

      ; ===== Hashed Collection Hierarchy =====
      (print-string "Creating hashed collection classes...")

      ; HashedCollection - collections using hash for lookup
      (set HashedCollection-class (new-class (tag-int NAME_HASHED_COLL) Collection-class))
      (define-var hc-methods (new-method-dict 10))
      (method-dict-add hc-methods (tag-int SEL_SIZE) (tag-int 66000))
      (method-dict-add hc-methods (tag-int SEL_INCLUDES) (tag-int 66001))
      (method-dict-add hc-methods (tag-int SEL_ADD) (tag-int 66002))
      (method-dict-add hc-methods (tag-int SEL_REMOVE) (tag-int 66003))
      (class-set-methods HashedCollection-class hc-methods)
      (print-string "  HashedCollection")

      ; Set - unordered collection with unique elements
      (set Set-class (new-class (tag-int NAME_SET) HashedCollection-class))
      (define-var set-methods (new-method-dict 10))
      (method-dict-add set-methods (tag-int SEL_ADD) (tag-int 67000))
      (method-dict-add set-methods (tag-int SEL_REMOVE) (tag-int 67001))
      (method-dict-add set-methods (tag-int SEL_INCLUDES) (tag-int 67002))
      (class-set-methods Set-class set-methods)
      (print-string "  Set")

      ; Dictionary - key-value mapping
      (set Dictionary-class (new-class (tag-int NAME_DICTIONARY) HashedCollection-class))
      (define-var dict-methods (new-method-dict 15))
      (method-dict-add dict-methods (tag-int SEL_AT) (tag-int 68000))
      (method-dict-add dict-methods (tag-int SEL_AT_PUT) (tag-int 68001))
      (method-dict-add dict-methods (tag-int SEL_AT_IFABSENT) (tag-int 68002))
      (method-dict-add dict-methods (tag-int SEL_INCLUDES) (tag-int 68003))
      (method-dict-add dict-methods (tag-int SEL_KEYS) (tag-int 68004))
      (method-dict-add dict-methods (tag-int SEL_VALUES) (tag-int 68005))
      (method-dict-add dict-methods (tag-int SEL_REMOVE) (tag-int 68006))
      (class-set-methods Dictionary-class dict-methods)
      (print-string "  Dictionary")

      ; MethodDictionary - specialized dictionary for method lookup
      (set MethodDictionary-class (new-class (tag-int NAME_METHOD_DICT) Dictionary-class))
      (define-var md-methods (new-method-dict 10))
      (method-dict-add md-methods (tag-int SEL_AT) (tag-int 69000))
      (method-dict-add md-methods (tag-int SEL_AT_PUT) (tag-int 69001))
      (class-set-methods MethodDictionary-class md-methods)
      (print-string "  MethodDictionary")

      ; ===== CompiledMethod =====
      (print-string "Creating method classes...")

      ; CompiledMethod - compiled bytecode with metadata
      (set CompiledMethod-class (new-class (tag-int NAME_COMPILED_METHOD) Object-class))
      (define-var cm-methods (new-method-dict 10))
      (method-dict-add cm-methods (tag-int SEL_SELECTOR) (tag-int 70000))
      (method-dict-add cm-methods (tag-int SEL_BYTECODES) (tag-int 70001))
      (method-dict-add cm-methods (tag-int SEL_LITERALS) (tag-int 70002))
      (method-dict-add cm-methods (tag-int SEL_NUM_ARGS) (tag-int 70003))
      (method-dict-add cm-methods (tag-int SEL_NUM_TEMPS) (tag-int 70004))
      (class-set-methods CompiledMethod-class cm-methods)
      (print-string "  CompiledMethod")

      ; ===== Parser Support Classes =====
      (print-string "Creating parser support classes...")

      ; ASTNode and Token classes for the parser
      (set ASTNode-class (new-class (tag-int NAME_AST_NODE) Object-class))
      (set Token-class (new-class (tag-int NAME_TOKEN) Object-class))
      (print-string "  ASTNode")
      (print-string "  Token")

      ; ===== Print Class Hierarchy =====
      (print-string "")
      (print-string "Class hierarchy:")
      (print-string "  ProtoObject")
      (print-string "    Object")
      (print-string "      Behavior")
      (print-string "        ClassDescription")
      (print-string "          Class")
      (print-string "          Metaclass")
      (print-string "      Magnitude")
      (print-string "        Number")
      (print-string "          SmallInteger")
      (print-string "      Collection")
      (print-string "        SequenceableCollection")
      (print-string "          ArrayedCollection")
      (print-string "            Array")
      (print-string "            String")
      (print-string "            Symbol")
      (print-string "          OrderedCollection")
      (print-string "        HashedCollection")
      (print-string "          Set")
      (print-string "          Dictionary")
      (print-string "            MethodDictionary")
      (print-string "      CompiledMethod")
      (print-string "      ASTNode")
      (print-string "      Token")
      (print-string "")

      (print-string "=== Bootstrap Complete ===")
      (print-string "")

      ; Return Object-class as the root for normal programming
      Object-class))

  ; ===== Class Accessor Functions =====
  ; These provide access to the bootstrapped classes

  (define-func (st-object-class) Object-class)
  (define-func (st-behavior-class) Behavior-class)
  (define-func (st-class-class) Class-class)
  (define-func (st-metaclass-class) Metaclass-class)
  (define-func (st-small-integer-class) SmallInteger-class)
  (define-func (st-array-class) Array)
  (define-func (st-string-class) String-class)
  (define-func (st-symbol-class) Symbol-class)
  (define-func (st-ordered-collection-class) OrderedCollection-class)
  (define-func (st-set-class) Set-class)
  (define-func (st-dictionary-class) Dictionary-class)
  (define-func (st-method-dictionary-class) MethodDictionary-class)
  (define-func (st-compiled-method-class) CompiledMethod-class)

  0)
