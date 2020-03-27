#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32

static char buffer[2048];

/* Fake readline function */
char *readline(char *prompt)
{
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char *cpy = malloc(strlen(buffer) + 1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy) - 1] = '\0';
    return cpy;
}

/* Fake add_history function */
void add_history(char *unused) {}

/* Otherwise include the editline headers */
#else
#include <editline/readline.h>
#endif
long eval_op(long x, char *op, long y)
{
    if (strcmp(op, "+") == 0)
    {
        return x + y;
    }
    if (strcmp(op, "-") == 0)
    {
        return x - y;
    }
    if (strcmp(op, "*") == 0)
    {
        return x * y;
    }
    if (strcmp(op, "/") == 0)
    {
        return x / y;
    }
    if (strcmp(op, "^") == 0)
    {
        long a = 1;
        for (int i = 0; i < y; i++)
        {
            a *= x;
        }
        return a;
    }
    printf("Operator not found\n");
    return 0;
}
long eval(mpc_ast_t *t)
{

    /* If tagged as number return it directly. */
    if (strstr(t->tag, "number"))
    {
        return atoi(t->contents);
    }

    /* The operator is always second child. */
    char *op = t->children[1]->contents;

    /* We store the third child in `x` */
    long x = eval(t->children[2]);

    /* Iterate the remaining children and combining. */
    int i = 3;
    while (strstr(t->children[i]->tag, "expr"))
    {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}
int main(int argc, char **argv)
{
    /* Create Some Parsers */
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                     \
    number   : /-?[0-9]+/ ;                             \
    operator : '+' | '-' | '*' | '/' | '^' ;                  \
    expr     : <number> | '(' <operator> <expr>+ ')' ;  \
    lispy    : /^/ <operator> <expr>+ /$/ ;             \
  ",
              Number, Operator, Expr, Lispy);
    puts("Lispy Version 0.0.0.0.2");
    puts("Press Ctrl+c to Exit\n");

    while (1)
    {
        char *input = readline("lispy: ");

        add_history(input);
        if (strcmp(input, "quit") == 0)
        {
            printf("quiting program\n");
            break;
        }

        /* Attempt to Parse the user Input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r))
        {
            printf("%li\n", eval(r.output));
            mpc_ast_delete(r.output);
        }
        else
        {
            /* Otherwise Print the Error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}