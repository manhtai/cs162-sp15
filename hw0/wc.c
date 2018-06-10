/*
 * From The C Programming Language, 2nd Edition
 */
#include <stdio.h>

#define IN   1  /* inside a word */
#define OUT  0  /* outside a word */

/* count lines, words, and characters in input */
int main(int argc, char *argv[])
{
    int total, tnl, tnw, tnc;
    tnl = tnw = tnc = 0;
    FILE *fp;
    int count(FILE *fp, int *total, int *tnl, int *tnw, int *tnc);

    // print total when there are multple argv
    total = argc > 1;

    if (argc == 1) /* no args */
        count(stdin, &total, &tnl, &tnw, &tnc);
    else
        while (--argc > 0) {
            if ((fp = fopen(*++argv, "r")) == NULL) {
                printf("wc: can′t open %s\n", *argv);
                return 1;
            } else {
                count(fp, &total, &tnl, &tnw, &tnc);
                fclose(fp);
            }
        }

    if (total)
        printf("total %d %d %d\n", tnl, tnw, tnc);

    return 0;
}

int count(FILE *fp, int *total, int *tnl, int *tnw, int *tnc) {
    int c, nl, nw, nc, state;
    state = OUT;
    nl = nw = nc = 0;

    while ((c = getc(fp)) != EOF) {
        ++nc;
        if (c == '\n')
            ++nl;
        if (c == ' ' || c == '\n' || c =='\t')
            state = OUT;
        else if (state == OUT) {
            state = IN;
            ++nw;
        }
    }

    *tnl += nl;
    *tnw += nw;
    *tnc += nc;

    printf("%d %d %d\n", nl, nw, nc);
    return 0;
}
