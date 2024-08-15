#ifndef PACKL_CONTEXT_H
#define PACKL_CONTEXT_H

#include "packl-defs.h"

void packl_init_contexts(PACKL *self);
void packl_push_new_context(PACKL *self);
Context packl_get_current_context(PACKL *self);
Context_Item *packl_get_context_item_in_current_context(PACKL *self, String_View id);
Context_Item *packl_get_context_item_in_all_contexts(PACKL *self, String_View id);
void packl_push_item_in_current_context(PACKL *self, Context_Item item);
char *packl_get_context_item_type_as_cstr(Context_Item_Type type);
void packl_pop_context(PACKL *self);
void packl_remove_contexts(PACKL *self);

#endif 