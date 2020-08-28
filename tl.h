#ifndef NEWBIETL_INCLUDED
#define NEWBIETL_INCLUDED


struct tl_list {
    const char     *name;
    const char     *data;
    struct tl_list *list;
    struct tl_list *next;
};

struct tl_list *tl_new(const char     *name,
                       const char     *data,
                       struct tl_list *list,
                       struct tl_list *next);

struct tl_list *tl_plain_list(struct tl_list *list, struct tl_list *next);

struct tl_list *tl_named_list(const char     *name,
                              struct tl_list *list,
                              struct tl_list *next);

struct tl_list *tl_named_data(const char     *name,
                              const char     *data,
                              struct tl_list *next);


void tl_translate(struct tl_list *l);


#endif
