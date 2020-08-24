#include <stdlib.h>
#include <stdio.h>
#include <string.h>


struct reus_list {
    const char       *name;
    const char       *data;
    struct reus_list *list;
    struct reus_list *next;
};

struct reus {
    struct reus_list *variables;
    char             *name;
    char             *body;
    int               opened;
};

typedef void *(*reus_state)(struct reus *r, char c);


void reus_string_add(char **str, char c)
{
    int *header;
    if (!*str) {
        header    = malloc(9);
        *str      = (char *) header + 8;
        header[0] = 1;
        header[1] = 0;
        (*str)[0] = 0;
    } else header = (int *)(*str - 8);
    if (header[0] == ++header[1]) {
        header = realloc(header, 8 + (header[0] <<= 1));
        *str   = (char *) header + 8;
    }
    (*str)[header[1] - 1] = c;
    (*str)[header[1]]     = 0;
}


struct reus_list *reus_list_new(const char       *name,
                                const char       *data,
                                struct reus_list *list,
                                struct reus_list *next)
{
    struct reus_list *res = malloc(sizeof(*res));
    res->name             = name;
    res->data             = data;
    res->list             = list;
    res->next             = next;
    return res;
}

struct reus_list *reus_list_union(struct reus_list *a, struct reus_list *b)
{
    if (!a) return b ? reus_list_union(b, 0) : 0;
    return reus_list_new(a->name, a->data,
                         reus_list_union(a->list, 0),
                         reus_list_union(a->next, b));
}

struct reus_list *reus_list_get(struct reus_list *l, char *name)
{
    if (!l) return 0;
    if (!strcmp(l->name, name)) return l;
    return reus_list_get(l->next, name);
}

void reus_list_free(struct reus_list *l)
{
    if (l->list) reus_list_free(l->list);
    if (l->next) reus_list_free(l->next);
    free(l);
}


void reus_init(struct reus *r)
{
    r->variables = 0;
    r->name      = 0;
    r->body      = 0;
    r->opened    = 0;
}

void reus_free(struct reus *r)
{
    if (r->variables) reus_list_free(r->variables);
    if (r->name)      free(r->name - 8);
    if (r->body)      free(r->body - 8);
}


void *reus_state_at(struct reus *r, char c);
void *reus_state_subs(struct reus *r, char c);
void *reus_state_text(struct reus *r, char c)
{
    if (c == '@') return reus_state_at;
    if (c == '{') return reus_state_subs;
    putchar(c);
    return reus_state_text;
}

void *reus_state_subs(struct reus *r, char c)
{
    if (c == '}') {
        struct reus_list *var = reus_list_get(r->variables, r->name);
        if (var && var->data) {
            printf("%s", var->data);
        }
        free(r->name - 8);
        r->name = 0;
        return reus_state_text;
    }
    reus_string_add(&r->name, c);
    return reus_state_subs;
}

void reus_at(struct reus *r)
{
    struct reus_list *var = reus_list_get(r->variables, r->name);
    for (var = var->list; var; var = var->next) {
        char *c;
        struct reus sr;
        reus_state  s = reus_state_text;
        reus_init(&sr);
        sr.variables = reus_list_union(var->list, r->variables);
        for (c = r->body; *c; ++c) {
            s = s(&sr, *c);
        }
        reus_free(&sr);
    }
}
void *reus_state_body(struct reus *r, char c);
void *reus_state_at(struct reus *r, char c)
{
    if (c == '{') return reus_state_body;
    reus_string_add(&r->name, c);
    return reus_state_at;
}

void *reus_state_body(struct reus *r, char c)
{
    if (c == '{') ++r->opened;
    if (c == '}' && !r->opened--) {
        reus_at(r);
        r->opened = 0;
        free(r->name - 8);
        r->name = 0;
        free(r->body - 8);
        r->body = 0;
        return reus_state_text;
    }
    reus_string_add(&r->body, c);
    return reus_state_body;
}


int main()
{
    int         c;
    struct reus r;
    reus_state  s = reus_state_text;
    reus_init(&r);
    r.variables =
    reus_list_new("username", "aversey", 0,
    reus_list_new("articles", 0,
        reus_list_new(0, 0,
            reus_list_new("title", "Smooth Sort", 0,
            reus_list_new("author", "Edsger Dijkstra", 0, 0)),
        reus_list_new(0, 0,
            reus_list_new("title", "Your Own Site", 0,
            reus_list_new("author", "Aleksey Veresov", 0, 0)),
        reus_list_new(0, 0,
            reus_list_new("title", "Bullshit", 0,
            reus_list_new("author", "Ongo Gablogian", 0, 0)), 0))), 0));
    while ((c = getchar()) != EOF) {
        s = s(&r, c);
    }
    reus_free(&r);
    return 0;
}
