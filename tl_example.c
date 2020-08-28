#include <tl.h>


int main()
{
    struct tl_list *l =
      tl_named_data("username", "aversey",
      tl_named_list("articles",
        tl_plain_list(
          tl_named_data("title", "Substitution Processes",
          tl_named_data("author", "Edsger Dijkstra", 0)),
        tl_plain_list(
          tl_named_data("title", "NewbieTL",
          tl_named_data("author", "Aleksey Veresov", 0)),
        tl_plain_list(
          tl_named_data("title", "Bullshit",
          tl_named_data("author", "Ongo Gablogian", 0)), 0))), 0));
    tl_translate(l);
    return 0;
}
