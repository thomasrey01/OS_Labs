#ifndef AST_H
#define AST_H

struct ast;

enum type {
    INPUTLINE,
    PIPELINE,
    CHAIN,
    REDIRECT,
};

enum commOp {
    TREE_AND,
    TREE_OR,
    TREE_BACK,
    SEMI,
    NONE,
};

enum redirType {
    LEFT,
    RIGHT,
};

enum chainType {
    STATUS,
    EXIT,
    CD,
    COMMAND,
};

struct inputline {
    enum commOp op;
    struct ast *left;
    struct ast *right;
};

struct redirect {
    int size;
    int ptr;
    char **command;
    int num;
    enum redirType r1;
    enum redirType r2;
};

struct chain {
    int size;
    int ptr;
    char **command;
    enum chainType t;
};

struct ast {
    enum type t;
    union {
        struct inputline *i;
        struct chain *c;
        struct redirect *r;
    };
};

struct ast *createNode(enum type t);
void addCommand(char *s, struct chain *tree);
void addNull(struct chain *tree);
void freeSyntaxTree(struct ast *tree);
void printSyntaxTree(struct ast *tree);

#endif
