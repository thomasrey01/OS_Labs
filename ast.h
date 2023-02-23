#ifndef AST_H
#define AST_H

struct ast;

enum type {
    INPUTLINE,
    COMMAND,
    PIPELINE,
    CHAIN,
};

enum commOp {
    TREE_AND,
    TREE_OR,
    TREE_BACK,
    SEMI,
    NONE,
};

struct inputline {
    enum commOp op;
    struct ast *left;
    struct ast *right;
};

struct chain {
    char **command; // piping is not yet needed for this lab
    int size;
    int ptr;
};

struct ast {
    enum type t;
    union {
        struct inputline *i;
        struct chain *c;
    };
};

struct ast *createNode(enum type t);

#endif
