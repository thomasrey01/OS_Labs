#ifndef AST_H
#define AST_H

struct ast;
struct pipeline;

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

struct command {
    int size;
    int ptr;
    char **command;
};

struct inputline {
    enum commOp op;
    struct ast *left;
    struct ast *right;
};

struct pipeline {
    struct command *comm;
    struct pipeline *next;
};

struct redirect {
    struct command *comm;
    int num;
    enum redirType r1;
    enum redirType r2;
};

struct chain {
    enum chainType t;
    union {
        struct pipeline *pipel;
        char *dir;
    };
    struct redirect *red;
};

struct ast {
    enum type t;
    union {
        struct inputline *i;
        struct chain *c;
    };
};

struct ast *createNode(enum type t);
struct pipeline *createPipeline();
void addCommand(char *s, struct command *tree);
void addCD(char *s, struct chain *c);
void addNull(struct command *tree);
void freeSyntaxTree(struct ast *tree);
void printSyntaxTree(struct ast *tree);

#endif
