#include "tl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * public list functions
 */
struct tl_list *tl_new(const char     *name,
                       const char     *data,
                       struct tl_list *list,
                       struct tl_list *next)
{
    struct tl_list *res = malloc(sizeof(*res));
    res->name           = name;
    res->data           = data;
    res->list           = list;
    res->next           = next;
    return res;
}

struct tl_list *tl_plain_list(struct tl_list *list, struct tl_list *next)
{
    return tl_new(0, 0, list, next);
}

struct tl_list *tl_named_list(const char     *name,
                              struct tl_list *list,
                              struct tl_list *next)
{
    return tl_new(name, 0, list, next);
}

struct tl_list *tl_named_data(const char     *name,
                              const char     *data,
                              struct tl_list *next)
{
    return tl_new(name, data, 0, next);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * internal list functions
 */
static struct tl_list *tl_list_union(struct tl_list *a, struct tl_list *b)
{
    if (!a) return b ? tl_list_union(b, 0) : 0;
    return tl_new(a->name, a->data,
                  tl_list_union(a->list, 0),
                  tl_list_union(a->next, b));
}

static struct tl_list *tl_list_get(struct tl_list *l, char *name)
{
    if (!l) return 0;
    if (!strcmp(l->name, name)) return l;
    return tl_list_get(l->next, name);
}

static void tl_list_free(struct tl_list *l)
{
    if (l->list) tl_list_free(l->list);
    if (l->next) tl_list_free(l->next);
    free(l);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * string
 */
static int *tl_string_size(char *str)  { return ((int *) str) - 2; }
static int *tl_string_len(char *str)   { return ((int *) str) - 1; }

static void tl_string_add(char **str, char c)
{
    static const int shift = 2 * sizeof(int);
    if (!*str) {
        *str  = malloc(shift + 1);
        *str += shift;
        *tl_string_size(*str) = 1;
        *tl_string_len(*str)  = 0;
        (*str)[0] = 0;
    }
    ++*tl_string_len(*str);
    if (*tl_string_size(*str) == *tl_string_len(*str)) {
        *tl_string_size(*str) *= 2;
        *str  = realloc(*str - shift, shift + *tl_string_size(*str));
        *str += shift;
    }
    (*str)[*tl_string_len(*str) - 1] = c;
    (*str)[*tl_string_len(*str)]     = 0;
}

static void tl_string_free(char *str)
{
    if (str) free(str - 2 * sizeof(int));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * template language translator as automata
 */
struct tl {
    struct tl_list *variables;
    char           *name;
    char           *body;
    int             opened;
};
typedef void *(*tl_state)(struct tl *t, char c);

static void tl_free(struct tl *t)
{
    if (t->variables) tl_list_free(t->variables);
    if (t->name)      tl_string_free(t->name);
    if (t->body)      tl_string_free(t->body);
}

static void *tl_state_text(struct tl *t, char c);
void tl_translate(struct tl_list *l)
{
    int c;
    tl_state  state    = tl_state_text;
    struct tl automata = { l, 0, 0, 0 };
    while ((c = getchar()) != EOF) {
        state = state(&automata, c);
    }
    tl_free(&automata);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * automata states
 */
static void *tl_state_backslash(struct tl *t, char c);
static void *tl_state_at(struct tl *t, char c);
static void *tl_state_subs(struct tl *t, char c);
static void *tl_state_text(struct tl *t, char c)
{
    if (c == '\\') return tl_state_backslash;
    if (c == '@') return tl_state_at;
    if (c == '{') return tl_state_subs;
    putchar(c);
    return tl_state_text;
}

static void *tl_state_subs(struct tl *t, char c)
{
    if (c == '}') {
        struct tl_list *var = tl_list_get(t->variables, t->name);
        if (var && var->data) {
            printf("%s", var->data);
        }
        tl_string_free(t->name);
        t->name = 0;
        return tl_state_text;
    }
    tl_string_add(&t->name, c);
    return tl_state_subs;
}

static void tl_at(struct tl *t)
{
    struct tl_list *var = tl_list_get(t->variables, t->name);
    for (var = var->list; var; var = var->next) {
        char *c;
        tl_state  state    = tl_state_text;
        struct tl automata = { 0, 0, 0, 0 };
        automata.variables = tl_list_union(var->list, t->variables);
        for (c = t->body; *c; ++c) {
            state = state(&automata, *c);
        }
        tl_free(&automata);
    }
}
static void *tl_state_body(struct tl *t, char c);
static void *tl_state_at(struct tl *t, char c)
{
    if (c == '{') return tl_state_body;
    tl_string_add(&t->name, c);
    return tl_state_at;
}

static void *tl_state_body(struct tl *t, char c)
{
    if (c == '{') ++t->opened;
    if (c == '}' && !t->opened--) {
        tl_at(t);
        t->opened = 0;
        free(t->name - 8);
        t->name = 0;
        free(t->body - 8);
        t->body = 0;
        return tl_state_text;
    }
    tl_string_add(&t->body, c);
    return tl_state_body;
}

static void *tl_state_backslash(struct tl *t, char c)
{
    if (c == '\n') return tl_state_text;
    if (c == 'n') {
        putchar('\n');
        return tl_state_text;
    }
    putchar(c);
    return tl_state_text;
}
