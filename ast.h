#ifndef AST_H
#define AST_H

struct ast;

enum type {
    INPUTLINE,
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

enum chainType {
    STATUS,
    EXIT,
    COMMAND,
};

struct inputline {
    enum commOp op;
    struct ast *left;
    struct ast *right;
};

struct chain {
    enum chainType t;
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
void addCommand(char *s, struct chain *tree);
void addNull(struct chain *tree);
void freeSyntaxTree(struct ast *tree);

#endif
