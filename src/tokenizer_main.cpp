// AUTO-GENERATED CODE - DO NOT EDIT MANUALLY
// This file was transpiled from: lisp/smalltalk_tokenizer_ffi.lisp
// To regenerate, run: make tokenizer-transpile
//
// Smalltalk Tokenizer with FFI for file I/O
// Demonstrates the Lisp-to-C++ transpiler with FFI capabilities

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Function definitions
int64_t fn_is_digit(int64_t ch) {
    int64_t var_0 = 0;
    if ((ch < 48LL)) {
        var_0 = 0LL;
    } else {
        int64_t var_1 = 0;
        if ((ch > 57LL)) {
            var_1 = 0LL;
        } else {
            var_1 = 1LL;
        }
        var_0 = var_1;
    }
    return var_0;
}

int64_t fn_is_letter(int64_t ch) {
    int64_t var_2 = 0;
    if ((ch < 65LL)) {
        var_2 = 0LL;
    } else {
        int64_t var_3 = 0;
        if ((ch <= 90LL)) {
            var_3 = 1LL;
        } else {
            int64_t var_4 = 0;
            if ((ch < 97LL)) {
                var_4 = 0LL;
            } else {
                int64_t var_5 = 0;
                if ((ch <= 122LL)) {
                    var_5 = 1LL;
                } else {
                    var_5 = 0LL;
                }
                var_4 = var_5;
            }
            var_3 = var_4;
        }
        var_2 = var_3;
    }
    return var_2;
}

int64_t fn_is_ident(int64_t ch) {
    int64_t var_6 = 0;
    if (fn_is_letter(ch) != 0) {
        var_6 = 1LL;
    } else {
        int64_t var_7 = 0;
        if (fn_is_digit(ch) != 0) {
            var_7 = 1LL;
        } else {
            var_7 = 0LL;
        }
        var_6 = var_7;
    }
    return var_6;
}

int64_t fn_is_whitespace(int64_t ch) {
    int64_t var_8 = 0;
    if ((ch == 32LL)) {
        var_8 = 1LL;
    } else {
        int64_t var_9 = 0;
        if ((ch == 9LL)) {
            var_9 = 1LL;
        } else {
            int64_t var_10 = 0;
            if ((ch == 10LL)) {
                var_10 = 1LL;
            } else {
                int64_t var_11 = 0;
                if ((ch == 13LL)) {
                    var_11 = 1LL;
                } else {
                    var_11 = 0LL;
                }
                var_10 = var_11;
            }
            var_9 = var_10;
        }
        var_8 = var_9;
    }
    return var_8;
}

int main(int argc, char** argv) {
    0LL;
    int64_t token_number = 1LL;
    0LL;
    int64_t token_identifier = 2LL;
    0LL;
    int64_t token_keyword = 3LL;
    0LL;
    int64_t token_string = 4LL;
    0LL;
    int64_t token_symbol = 5LL;
    0LL;
    int64_t token_operator = 6LL;
    0LL;
    0LL;
    0LL;
    0LL;
    0LL;
    std::string source = (([&]() {
        if (argc > 1) {
            std::ifstream file(argv[1]);
            if (!file) {
                std::cerr << "Error: Cannot open file '" << argv[1] << "'" << '\n';
                std::exit(1);
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            std::cout << "=== Smalltalk Tokenizer ===" << '\n';
            std::cout << "File: " << argv[1] << '\n';
            std::cout << "Size: " << content.length() << " characters" << '\n';
            std::cout << '\n';
            return content;
        } else {
            std::string content = "obj message: 'hello' #symbol [ :x | x + 42 ] := 100";
            std::cout << "=== Smalltalk Tokenizer (default input) ===" << '\n';
            std::cout << '\n';
            return content;
        }
    })());
    0LL;
    auto len = (int64_t)source.length();
    0LL;
    int64_t pos = 0LL;
    0LL;
    int64_t count = 0LL;
    0LL;
    (std::cout << "Tokens:" << '\n');
label_0:
    if (!((pos < len)))
        goto label_0_end;
label_1:
    int64_t var_12 = 0;
    if ((pos < len)) {
        var_12 = fn_is_whitespace((int64_t)source[pos]);
    } else {
        var_12 = 0LL;
    }
    if ((var_12) == 0)
        goto label_1_end;
    pos = (pos + 1LL);
    pos;
    goto label_1;
label_1_end:
    0LL;
    int64_t var_13 = 0;
    if ((pos < len)) {
        auto ch = (int64_t)source[pos];
        0LL;
        int64_t start = pos;
        0LL;
        int64_t token_type = 0LL;
        0LL;
        int64_t var_14 = 0;
        if (fn_is_digit(ch) != 0) {
        label_2:
            int64_t var_15 = 0;
            if ((pos < len)) {
                var_15 = fn_is_digit((int64_t)source[pos]);
            } else {
                var_15 = 0LL;
            }
            if ((var_15) == 0)
                goto label_2_end;
            pos = (pos + 1LL);
            pos;
            goto label_2;
        label_2_end:
            0LL;
            token_type = token_number;
            var_14 = token_type;
        } else {
            int64_t var_16 = 0;
            if (fn_is_letter(ch) != 0) {
            label_3:
                int64_t var_17 = 0;
                if ((pos < len)) {
                    var_17 = fn_is_ident((int64_t)source[pos]);
                } else {
                    var_17 = 0LL;
                }
                if ((var_17) == 0)
                    goto label_3_end;
                pos = (pos + 1LL);
                pos;
                goto label_3;
            label_3_end:
                0LL;
                int64_t var_18 = 0;
                if ((pos < len)) {
                    int64_t var_19 = 0;
                    if (((int64_t)source[pos] == 58LL)) {
                        pos = (pos + 1LL);
                        pos;
                        token_type = token_keyword;
                        var_19 = token_type;
                    } else {
                        token_type = token_identifier;
                        var_19 = token_type;
                    }
                    var_18 = var_19;
                } else {
                    token_type = token_identifier;
                    var_18 = token_type;
                }
                var_16 = var_18;
            } else {
                int64_t var_20 = 0;
                if ((ch == 39LL)) {
                    pos = (pos + 1LL);
                    pos;
                label_4:
                    int64_t var_21 = 0;
                    if ((pos < len)) {
                        int64_t var_22 = 0;
                        if (((int64_t)source[pos] == 39LL)) {
                            var_22 = 0LL;
                        } else {
                            var_22 = 1LL;
                        }
                        var_21 = var_22;
                    } else {
                        var_21 = 0LL;
                    }
                    if ((var_21) == 0)
                        goto label_4_end;
                    pos = (pos + 1LL);
                    pos;
                    goto label_4;
                label_4_end:
                    0LL;
                    int64_t var_23 = 0;
                    if ((pos < len)) {
                        pos = (pos + 1LL);
                        var_23 = pos;
                    } else {
                        var_23 = 0LL;
                    }
                    var_23;
                    token_type = token_string;
                    var_20 = token_type;
                } else {
                    int64_t var_24 = 0;
                    if ((ch == 35LL)) {
                        pos = (pos + 1LL);
                        pos;
                    label_5:
                        int64_t var_25 = 0;
                        if ((pos < len)) {
                            var_25 = fn_is_ident((int64_t)source[pos]);
                        } else {
                            var_25 = 0LL;
                        }
                        if ((var_25) == 0)
                            goto label_5_end;
                        pos = (pos + 1LL);
                        pos;
                        goto label_5;
                    label_5_end:
                        0LL;
                        token_type = token_symbol;
                        var_24 = token_type;
                    } else {
                        pos = (pos + 1LL);
                        pos;
                        int64_t var_26 = 0;
                        if ((ch == 58LL)) {
                            int64_t var_27 = 0;
                            if ((pos < len)) {
                                int64_t var_28 = 0;
                                if (((int64_t)source[pos] == 61LL)) {
                                    pos = (pos + 1LL);
                                    var_28 = pos;
                                } else {
                                    var_28 = 0LL;
                                }
                                var_27 = var_28;
                            } else {
                                var_27 = 0LL;
                            }
                            var_26 = var_27;
                        } else {
                            var_26 = 0LL;
                        }
                        var_26;
                        token_type = token_operator;
                        var_24 = token_type;
                    }
                    var_20 = var_24;
                }
                var_16 = var_20;
            }
            var_14 = var_16;
        }
        var_14;
        (([&]() {
            static const char* token_names[] = {"UNKNOWN   ", "NUMBER    ", "IDENTIFIER",
                                                "KEYWORD   ", "STRING    ", "SYMBOL    ",
                                                "OPERATOR  "};
            std::string token_text = source.substr(start, pos - start);
            // Escape newlines for display
            for (size_t i = 0; i < token_text.length(); i++) {
                if (token_text[i] == '\n') {
                    token_text.replace(i, 1, "\\n");
                    i++;
                }
            }
            std::cout << std::setw(3) << (count + 1) << ". " << token_names[token_type] << " ["
                      << std::setw(3) << start << "-" << std::setw(3) << pos << "]: '" << token_text
                      << "'" << '\n';
            return 0LL;
        })());
        count = (count + 1LL);
        var_13 = count;
    } else {
        var_13 = 0LL;
    }
    var_13;
    goto label_0;
label_0_end:
    0LL;
    (std::cout << '\n');
    (std::cout << "Total tokens: " << count << '\n');
    int64_t result = count;
    std::cout << result << '\n';
    return 0;
}
