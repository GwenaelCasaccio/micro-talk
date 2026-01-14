#!/bin/bash
# Build combined bootstrap.lisp from modular sources
# This generates the bootstrap.lisp file used by tests

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
OUTPUT="$SCRIPT_DIR/bootstrap.lisp"

MODULES=(
    "00-runtime.lisp"
    "01-symbol-table.lisp"
    "02-classes.lisp"
    "03-methods.lisp"
    "04-tokenizer.lisp"
    "05-parser.lisp"
    "06-compiler.lisp"
    "07-bootstrap.lisp"
)

# Header
cat > "$OUTPUT" << 'HEADER'
; ===== Smalltalk Bootstrap =====
; Generated from modular sources - DO NOT EDIT directly
;
; Source modules:
;   00-runtime.lisp      - Tagging, malloc, object creation
;   01-symbol-table.lisp - Hash table and symbol table
;   02-classes.lisp      - Class hierarchy
;   03-methods.lisp      - Method dictionaries, cache, context
;   04-tokenizer.lisp    - String ops, tokenizer
;   05-parser.lisp       - AST and parser
;   06-compiler.lisp     - Bytecode compiler
;   07-bootstrap.lisp    - Bootstrap function (class creation)
;
; Rebuild with: make smalltalk-build

(do
HEADER

# Extract content from each module (strip outer (do ... 0) wrapper)
for module in "${MODULES[@]}"; do
    echo "" >> "$OUTPUT"
    echo "  ; ============ $module ============" >> "$OUTPUT"
    
    # Strip: leading comments, (do line, final 0) line
    sed '
        /^; /d
        /^$/d
        /^(do$/d
        /^  0)$/d
    ' "$SCRIPT_DIR/$module" >> "$OUTPUT"
done

echo "Generated: $OUTPUT"
