#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/nisc.h"

const char *TOKEN_STRINGS[] = {
    [NIS_TOKEN_NONE] = "<none>",
    [NIS_TOKEN_IDENT] = "<identifer>",
    [NIS_TOKEN_CHAR] = "<character>",
    [NIS_TOKEN_INT] = "<integer>",
    [NIS_TOKEN_FLOAT] = "<real>",
    [NIS_TOKEN_DOT] = "`.`",
    [NIS_TOKEN_SINGLE_QUOTE] = "`'`",
    [NIS_TOKEN_U8PARENL] = "`u8(`",
    [NIS_TOKEN_PARENL] = "`(`",
    [NIS_TOKEN_PARENR] = "`)`",
    [NIS_TOKEN_BACKTICK] = "`\\``",
    [NIS_TOKEN_COMMA] = "`,`",
    [NIS_TOKEN_COMMA_AT] = "`,@`",
    [NIS_TOKEN_DOUBLE_QUOTE] = "`\"`",
    [NIS_TOKEN_BACKSLASH] = "`\\\\`",
    [NIS_TOKEN_SQUAREL] = "`[`",
    [NIS_TOKEN_SQUARER] = "`]`",
    [NIS_TOKEN_CURLYL] = "`{`",
    [NIS_TOKEN_CURLYR] = "`}`",
    [NIS_TOKEN_HASH] = "`#`",
    [NIS_TOKEN_INLINE] = "<identifier>",
    [NIS_TOKEN_LABEL] = "<integer>`=`",
    [NIS_TOKEN_REFERENCE] = "<integer>`#`",
    NULL,
};

enum {
    NIS_LEX_NORMAL,
    NIS_LEX_HASH,
    NIS_LEX_STRING,
};

static bool is_ident_begin(int ch) {
    switch (ch) {
    case '!': case '$': case '%': case '&': case '*':
    case '+': case '-': case '.': case '/': case ':':
    case '<': case '=': case '>': case '?': case '@':
    case '^': case '_': case '~':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y':
    case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y':
    case 'Z':
        return 1;
    default:
        return 0;
    }
}

static bool is_ident_cont(int ch) {
    switch (ch) {
    case '!': case '$': case '%': case '&': case '*':
    case '+': case '-': case '.': case '/': case ':':
    case '<': case '=': case '>': case '?': case '@':
    case '^': case '_': case '~':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y':
    case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y':
    case 'Z':
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9': case '0':
        return 1;
    default:
        return 0;
    }
}

int nis_lex(struct NisTokens *dest, const char *src, size_t len) {
    size_t cap = 16;
    dest->list = malloc(cap * sizeof(NisToken));
    dest->len = 0;

    int state = NIS_LEX_NORMAL;

    const char *ptr = src;
    for (size_t offset = 0; offset < len; ) {
        if (dest->len == cap) {
            cap *= 2;
            dest->list = realloc(dest->list, cap * sizeof(NisToken));
        }
        switch (state) {
        case NIS_LEX_NORMAL: {
            ptr = src + offset;
            char ch = src[offset++];
            switch (ch) {
            case '(': {
                dest->list[dest->len].kind = NIS_TOKEN_PARENL;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case ')': {
                dest->list[dest->len].kind = NIS_TOKEN_PARENR;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case '\'': {
                dest->list[dest->len].kind = NIS_TOKEN_SINGLE_QUOTE;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case '`': {
                dest->list[dest->len].kind = NIS_TOKEN_BACKTICK;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case ',': {
                if (src[offset] == '@') {
                    offset++;
                    dest->list[dest->len].kind = NIS_TOKEN_COMMA_AT;
                    dest->list[dest->len].span.ptr = ptr;
                    dest->list[dest->len].span.len = 2;
                } else {
                    dest->list[dest->len].kind = NIS_TOKEN_COMMA;
                    dest->list[dest->len].span.ptr = ptr;
                    dest->list[dest->len].span.len = 1;
                }
                dest->len++;
            } break;
            case '[': {
                dest->list[dest->len].kind = NIS_TOKEN_SQUAREL;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case ']': {
                dest->list[dest->len].kind = NIS_TOKEN_SQUARER;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case '{': {
                dest->list[dest->len].kind = NIS_TOKEN_CURLYL;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case '}': {
                dest->list[dest->len].kind = NIS_TOKEN_CURLYR;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case '\t': case '\n': case ' ': {
                // nothing
            } break;
            case '"': {
                // TODO
            } break;
            case '\\': {
                // TODO
            } break;
            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
            case '0': {
                size_t len = 1;
                long num = ch - '0';
                for (ch = src[offset]; ch >= '0' && ch <= '9'; ch = src[offset++]) {
                    num *= 10;
                    num += ch;
                    ++len;
                }
                dest->list[dest->len].kind = NIS_TOKEN_INT;
                dest->list[dest->len].vint = num;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = len;
                dest->len++;
            } break;
            default: {
                if (is_ident_begin(ch)) {
                    size_t len = 1;
                    size_t cap = 0;
                    char vinline[8];
                    size_t idx = 0;
                    char *vatom = NULL;
                    vinline[idx++] = ch;
                    for (ch = src[offset]; is_ident_cont(ch); ch = src[offset++]) {
                        if (idx == 7) {
                            cap = 16;
                            vatom = malloc(cap * sizeof(char));
                            memcpy(vatom, vinline, 7);
                        }
                        if (idx >= 7) {
                            vatom[idx++] = ch;
                        } else {
                            vinline[idx++] = ch;
                        }
                        ++len;
                    }
                    
                    if (vatom) {
                        dest->list[dest->len].kind = NIS_TOKEN_IDENT;
                        dest->list[dest->len].subkind = NIS_TOKEN_NONE;
                        dest->list[dest->len].vatom = vatom;
                    } else {
                        vinline[len] = '\0';
                        if (strcmp(vinline, ".") == 0) {
                            dest->list[dest->len].kind = NIS_TOKEN_DOT;
                        } else {
                            dest->list[dest->len].kind = NIS_TOKEN_IDENT;
                            dest->list[dest->len].subkind = NIS_TOKEN_INLINE;
                            strncpy(dest->list[dest->len].vinline, vinline, 8);
                        }
                    }
                    dest->list[dest->len].span.ptr = ptr;
                    dest->list[dest->len].span.len = len;
                    dest->len++;
                } else {
                    fprintf(stderr, "nisc:%s:%d: error: `%c` is not a valid character\n", __FILE__, __LINE__, ch);
                    return 1;
                }
            } break;
            }
        } break;
        case NIS_LEX_HASH: {
        } break;
        case NIS_LEX_STRING: {
        } break;
        }
    }
    return 0;
}

void nis_del_tokens(struct NisTokens *tokens) {
    for (size_t i = 0; i < tokens->len; i++) {
        NisToken *token = tokens->list + i;
        if (token->kind == NIS_TOKEN_IDENT
            && token->subkind == NIS_TOKEN_NONE) {
            free(token->vatom);
        }
    }
    free(tokens->list);
}

static int nis_parse_one(NisValue *dest, NisGc *gc, struct NisTokens *tokens, size_t *idx) {
    NisToken *token = &tokens->list[*idx];
    ++*idx;
    switch (token->kind) {
    case NIS_TOKEN_IDENT: {
        const char *atom;
        if (token->subkind == NIS_TOKEN_INLINE) {
            atom = token->vinline;
        } else {
            atom = token->vatom;
        }
        if (strcmp(atom, "lambda") == 0) {
            nis_special(dest, gc, NIS_VALUE_LAMBDA);
        } else if (strcmp(atom, "if") == 0) {
            nis_special(dest, gc, NIS_VALUE_IF);
        } else if (strcmp(atom, "set!") == 0) {
            nis_special(dest, gc, NIS_VALUE_SET);
        } else if (strcmp(atom, "include") == 0) {
            nis_special(dest, gc, NIS_VALUE_INCLUDE);
        } else if (strcmp(atom, "include-ci") == 0) {
            nis_special(dest, gc, NIS_VALUE_INCLUDE_CI);
        } else if (strcmp(atom, "cond") == 0) {
            nis_special(dest, gc, NIS_VALUE_COND);
        } else if (strcmp(atom, "case") == 0) {
            nis_special(dest, gc, NIS_VALUE_CASE);
        } else if (strcmp(atom, "else") == 0) {
            nis_special(dest, gc, NIS_VALUE_ELSE);
        } else if (strcmp(atom, "and") == 0) {
            nis_special(dest, gc, NIS_VALUE_AND);
        } else if (strcmp(atom, "or") == 0) {
            nis_special(dest, gc, NIS_VALUE_OR);
        } else if (strcmp(atom, "unless") == 0) {
            nis_special(dest, gc, NIS_VALUE_UNLESS);
        } else if (strcmp(atom, "cond-expand") == 0) {
            nis_special(dest, gc, NIS_VALUE_COND_EXPAND);
        } else if (strcmp(atom, "let") == 0) {
            nis_special(dest, gc, NIS_VALUE_LET);
        } else if (strcmp(atom, "let*") == 0) {
            nis_special(dest, gc, NIS_VALUE_LET_STAR);
        } else if (strcmp(atom, "letrec") == 0) {
            nis_special(dest, gc, NIS_VALUE_LETREC);
        } else if (strcmp(atom, "letrec*") == 0) {
            nis_special(dest, gc, NIS_VALUE_LETREC_STAR);
        } else if (strcmp(atom, "let-values") == 0) {
            nis_special(dest, gc, NIS_VALUE_LET_VALUES);
        } else if (strcmp(atom, "let*-values") == 0) {
            nis_special(dest, gc, NIS_VALUE_LET_VALUES_STAR);
        } else if (strcmp(atom, "begin") == 0) {
            nis_special(dest, gc, NIS_VALUE_BEGIN);
        } else if (strcmp(atom, "do") == 0) {
            nis_special(dest, gc, NIS_VALUE_DO);
        } else if (strcmp(atom, "delay") == 0) {
            nis_special(dest, gc, NIS_VALUE_DELAY);
        } else if (strcmp(atom, "delay-force") == 0) {
            nis_special(dest, gc, NIS_VALUE_DELAY_FORCE);
        } else if (strcmp(atom, "force") == 0) {
            nis_special(dest, gc, NIS_VALUE_FORCE);
        } else if (strcmp(atom, "make-promise") == 0) {
            nis_special(dest, gc, NIS_VALUE_MAKE_PROMISE);
        } else if (strcmp(atom, "make-parameter") == 0) {
            nis_special(dest, gc, NIS_VALUE_MAKE_PARAMETER);
        } else if (strcmp(atom, "parameterize") == 0) {
            nis_special(dest, gc, NIS_VALUE_PARAMETERIZE);
        } else if (strcmp(atom, "guard") == 0) {
            nis_special(dest, gc, NIS_VALUE_GUARD);
        } else if (strcmp(atom, "quasiquote") == 0) {
            nis_special(dest, gc, NIS_VALUE_QUASIQUOTE);
        } else if (strcmp(atom, "unquote") == 0) {
            nis_special(dest, gc, NIS_VALUE_UNQUOTE);
        } else if (strcmp(atom, "unquote-splicing") == 0) {
            nis_special(dest, gc, NIS_VALUE_UNQUOTE_SPLICING);
        } else if (strcmp(atom, "case-lambda") == 0) {
            nis_special(dest, gc, NIS_VALUE_CASE_LAMBDA);
        } else if (strcmp(atom, "let-syntax") == 0) {
            nis_special(dest, gc, NIS_VALUE_LET_SYNTAX);
        } else if (strcmp(atom, "letrec-syntax") == 0) {
            nis_special(dest, gc, NIS_VALUE_LETREC_SYNTAX);
        } else if (strcmp(atom, "syntax-rules") == 0) {
            nis_special(dest, gc, NIS_VALUE_SYNTAX_RULES);
        } else if (strcmp(atom, "syntax-error") == 0) {
            nis_special(dest, gc, NIS_VALUE_SYNTAX_ERROR);
        } else {
            nis_atom(dest, gc, atom);
            dest->vtree->span = token->span;
        }
        return 0;
    }
    case NIS_TOKEN_CHAR: {
        nis_int(dest, gc, token->vchar);
        return 0;
    }
    case NIS_TOKEN_INT: {
        nis_int(dest, gc, token->vint);
        return 0;
    }
    case NIS_TOKEN_FLOAT: {
        nis_float(dest, gc, token->vfloat);
        return 0;
    }
    case NIS_TOKEN_PARENL: {
        size_t len = token->span.len;

        NisToken *token2 = &tokens->list[*idx];
        
        if (token2->kind == NIS_TOKEN_PARENR) {
            ++*idx;
            nis_nil(dest, gc);
            return 0;
        }

        NisValue fst;
        nis_parse_one(&fst, gc, tokens, idx);
        
        token2 = &tokens->list[*idx];

        if (token2->kind == NIS_TOKEN_PARENR) {
            ++*idx;
            NisValue nil;
            nis_nil(&nil, gc);
            nis_pair(dest, gc, &fst, &nil);
            
            len += (token2->span.ptr - (token->span.ptr + len)) + token2->span.len;
            dest->vtree->span.ptr = token->span.ptr;
            dest->vtree->span.len = len;

            return 0;
        } else if (token2->kind == NIS_TOKEN_DOT) {
            ++*idx;
            NisValue snd;
            nis_parse_one(&snd, gc, tokens, idx);
            nis_pair(dest, gc, &fst, &snd);
            
            token2 = &tokens->list[*idx];

            if (token2->kind != NIS_TOKEN_PARENR) {
                fprintf(stderr,
                        "nisc:%s:%d: error: unexpected token: %s\n",
                        __FILE__,
                        __LINE__,
                        TOKEN_STRINGS[token2->kind]);
                return 1;
            }
            
            len += (token2->span.ptr - (token->span.ptr + len)) + token2->span.len;
            dest->vtree->span.ptr = token->span.ptr;
            dest->vtree->span.len = len;
            
            return 0;
        }

        NisValue nil;
        nis_nil(&nil, gc);
        nis_pair(dest, gc, &fst, &nil);

        NisStree *list = dest->vtree;
        
        token2 = &tokens->list[*idx];

        while (token2->kind != NIS_TOKEN_PARENR) {
            if (token2->kind == NIS_TOKEN_DOT) {
                ++*idx;
                NisValue last;
                nis_parse_one(&last, gc, tokens, idx);
                list->vpair.cdr = nis_value_to_stree(gc, &last);
                break;
            } else {
                NisValue next;
                nis_parse_one(&next, gc, tokens, idx);
                NisValue newlist;
                nis_pair(&newlist, gc, &next, &nil);
                list->vpair.cdr = newlist.vtree;
                list = list->vpair.cdr;
            }
            token2 = &tokens->list[*idx];
        }
        
        token2 = &tokens->list[*idx];

        if (token2->kind != NIS_TOKEN_PARENR) {
            fprintf(stderr,
                    "nisc:%s:%d: error: unexpected token: %s\n",
                    __FILE__,
                    __LINE__,
                    TOKEN_STRINGS[token2->kind]);
            return 1;
        }

        ++*idx;
        
        len += (token2->span.ptr - (token->span.ptr + len)) + token2->span.len;
        dest->vtree->span.ptr = token->span.ptr;
        dest->vtree->span.len = len;

        return 0;
    }
    case NIS_TOKEN_SINGLE_QUOTE: {
        NisValue expr;
        nis_parse_one(&expr, gc, tokens, idx);

        NisValue nil;
        nis_nil(&nil, gc);
        
        NisValue list;
        nis_pair(&list, gc, &expr, &nil);

        NisValue quote;
        nis_special(&quote, gc, NIS_VALUE_QUOTE);

        nis_pair(dest, gc, &quote, &list);
        return 0;
    }
    case NIS_TOKEN_BACKTICK: {
        NisValue expr;
        nis_parse_one(&expr, gc, tokens, idx);

        NisValue nil;
        nis_nil(&nil, gc);
        
        NisValue list;
        nis_pair(&list, gc, &expr, &nil);

        NisValue quote;
        nis_special(&quote, gc, NIS_VALUE_QUASIQUOTE);

        nis_pair(dest, gc, &quote, &list);
        return 0;
    }
    case NIS_TOKEN_COMMA: {
        NisValue expr;
        nis_parse_one(&expr, gc, tokens, idx);

        NisValue nil;
        nis_nil(&nil, gc);
        
        NisValue list;
        nis_pair(&list, gc, &expr, &nil);

        NisValue unquote;
        nis_special(&unquote, gc, NIS_VALUE_UNQUOTE);

        nis_pair(dest, gc, &unquote, &list);
        return 0;
    }
    case NIS_TOKEN_COMMA_AT: {
        NisValue expr;
        nis_parse_one(&expr, gc, tokens, idx);

        NisValue nil;
        nis_nil(&nil, gc);
        
        NisValue list;
        nis_pair(&list, gc, &expr, &nil);

        NisValue unquote;
        nis_special(&unquote, gc, NIS_VALUE_UNQUOTE_SPLICING);

        nis_pair(dest, gc, &unquote, &list);
        return 0;
    }
    default:
        fprintf(stderr,
                "nisc:%s:%d: error: unexpected token: %s\n",
                __FILE__,
                __LINE__,
                TOKEN_STRINGS[token->kind]);
        return 1;
    }
}

int nis_parse(NisValue **dest, size_t *len, NisGc *gc, struct NisTokens *tokens) {
    int status = 0;
    size_t offset = 0;
    size_t capacity = 64;
    *dest = malloc(capacity * sizeof(NisValue));
    *len = 0;
    size_t i = 0;
    while (i < tokens->len) {
        if (offset == capacity) {
            capacity *= 2;
            *dest = realloc(*dest, capacity * sizeof(NisValue));
        }
        int s = nis_parse_one(*dest + offset++, gc, tokens, &i);
        if (!s) {
            ++*len;
        }
        status |= s;
    }
    return status;
}
