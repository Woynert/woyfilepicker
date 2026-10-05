/*
   Room for improvement:
   * Make node children be a linked list and not a vector.
*/

#ifndef UITREE_H
#define UITREE_H

#include "anytype.h"
#include "arena_extra.h"
#include "strview.h"
#include "la_extra.h"

#ifndef DRAW_FUNC_ARG_TYPE
#define DRAW_FUNC_ARG_TYPE void
#endif


typedef struct uitree_WidgetState {
    int int_a;
    int int_b;
    int int_c;
    int int_d;
    float float_a;
    float float_b;
    float float_c;
    float float_d;
    float float_e;
    float float_f;
    Rect2i rect_a;
    Rect2i rect_b;
    int __last_frame; // Used to determine if we should persist it or forget it.
} uitree_WidgetState;

#define STRMAP__TYPE uitree_WidgetState
#define STRMAP__NAMESPACE uitree_Map_str_state
#include "strmap.h"


typedef struct uitree_DrawInfo {
    int    user_draw_func_id;    // An id for the user to identify which function to call.
    Rect2i area;
    Anytype state;   // Persistent state, can be used to keep container's scroll value, etc.
    strview_t node_id;
} uitree_DrawInfo;


typedef struct {
    strview_t node_path;
    Rect2i area;
    Anytype state;
} uitree_NewDrawInfo;



#define LIST__TYPE uitree_DrawInfo
#define LIST__NAMESPACE uitree_List_DrawInfo
#include "list_simple.h"

typedef struct uitree_Node uitree_Node;

#define DYNA__TYPE uitree_Node
#define DYNA__NAMESPACE uitree__Vec_Node
#define DYNA__ONLY_HEADER
#include "da.h"

typedef struct UILayoutNode UILayoutNode;

#define DYNA__TYPE UILayoutNode
#define DYNA__NAMESPACE uitree__Vec_UILayoutNode
#define DYNA__ONLY_HEADER
#include "da.h"

//typedef void (uitree_ContainerFunc)(Rect2i area, int child_count, Rect2i *children, void *user_ctx, uitree_WidgetState *state);
typedef void (uitree_ContainerFunc)(Rect2i area, int child_count, Rect2i *children, Anytype state);

typedef struct {
    Blob blob;
    int user_type;
    void *data;
    int size;
} UILayoutNode_Data;

typedef struct UILayoutNode {
    uitree_ContainerFunc *container_func;
    void *user_ctx;              // For container_func.
    uitree__Vec_UILayoutNode children;
    ID identifier;              // ID into uitree.strpool
    Anytype state;

    int user_draw_func_id; // If <= 0 will be ignored.
    //Draw_Func_t *draw_func;

    Rect2i area; // Used to hold the area momentarily during end_tree.
} UILayoutNode;

#define DYNA__TYPE UILayoutNode
#define DYNA__NAMESPACE uitree__Vec_UILayoutNode
#define DYNA__ONLY_IMP
#include "da.h"

typedef struct uitree_Node {

    ID identifier_strpool_id; // It's OK if a widget has no identifier. It's state won't persist thru frames.

    bool has_user_draw_func;  // User must check this, if adding an user_function_id.
    int user_draw_func_id;    // An id for the user to identify which function to call.

    uitree_ContainerFunc *container_func;

    bool is_container;
    struct {
        void *user_ctx; // User can set this to access it later on 'container_func' call.
        uitree__Vec_Node children;
    } container;

    Rect2i _area; // Used to hold the area momentarily during end_tree.

    struct {
        // If not NULL then it'll be used as default.
        uitree_WidgetState *user_default_state;
    };
} uitree_Node;

#define DYNA__TYPE uitree_Node
#define DYNA__NAMESPACE uitree__Vec_Node
#define DYNA__ONLY_IMP
#include "da.h"

typedef struct Uitree {
    ArenaRoot arenaroot;
    Arena *arena;
    Strpool strpool; // For temporarily storing Nodes' identifiers.

    // Maps a title to a state. We might add or delete from this every frame.
    uitree_Map_str_state title_to_state;

    int frame;

    uitree_Node root_node;
    Rect2i screen;

    // Only populated on "frame_end".
    // Layers -> Lists -> DrawInfo.
    uitree_List_DrawInfo out_draw_list;

    uitree_WidgetState state_non_persistent; // Fallback for nodes which have no identifier.
} Uitree;

/// @Returns error.
int uitree_create(Uitree *t, Arena *perm) {
    *t = (Uitree) { 0 };
    //t->arenaroot = ArenaRoot_create(1 << 20);
    t->arena = perm;
    strpool_create_with_allocator(&t->strpool, arena_allocator, perm);
    int err = uitree_Map_str_state_create_with_allocator(&t->title_to_state, arena_allocator, perm);
    return err;
}

void uitree_free(Uitree *t) {
    //ArenaRoot_free(&t->arenaroot);
    //uitree_Map_str_state_free(&t->title_to_state);
    //*t = (Uitree) { 0 };
}

void uitree_build_start(Uitree *t, Rect2i screen) {
    //t->arena = ArenaRoot_get_arena(t->arenaroot);
    //t->screen = screen;
    //strpool_create_with_allocator(&t->strpool, arena_allocator, &t->arena);
}

//void uitree__print_tree(Uitree *t, uitree_Node *node, int level) {
    //for (int i = 0; i < level; ++i) {
        //printf("   ");
    //}
    //printf("%s", node->is_container ? "Container" : "Widget");

    //strview_t identifier = strpool_get(&t->strpool, node->identifier_strpool_id);
    //if (identifier.size > 0) {
        //printf(" id:%"PRIstr, PRIstrarg(identifier));
    //}
    //if (node->has_user_draw_func) {
        //printf(" area:"Rect2i_Fmt, Rect2i_Arg(node->_area));
    //}
    //printf("\n");
    //if (!node->is_container) { return; }

    //for (int i = 0; i < node->container.children.size; ++i) {
        //uitree__print_tree(t, &node->container.children.items[i], level+1);
    //}
//}

void uitree__print_tree(Uitree *t, UILayoutNode *node, int level);
void uitree__print_tree(Uitree *t, UILayoutNode *node, int level) {
    for (int i = 0; i < level; ++i) {
        printf("   ");
    }
    //printf("%s", node->is_container ? "Container" : "Widget");
    printf("children %d", node->children.size);

    strview_t identifier = strpool_get(&t->strpool, node->identifier);
    if (identifier.size > 0) {
        printf(" id:%"PRIstr, PRIstrarg(identifier));
    }
    //if (node->has_user_draw_func) {
        printf(" area:"Rect2i_Fmt, Rect2i_Arg(node->area));
    //}
    printf("\n");
    //if (!node->is_container) { return; }

    for (int i = 0; i < node->children.size; ++i) {
        uitree__print_tree(t, &node->children.items[i], level+1);
    }
}

/// @Returns state, never NULL.
uitree_WidgetState * uitree__try_get_saved_state(Uitree *t, strview_t key, uitree_Node *node) {
    if (key.size == 0 || key.data == NULL) { // No key means no state, sorry.
        if (node->user_default_state != NULL) {
            return node->user_default_state;
        } else {
            return &t->state_non_persistent;
        }
    }
    uitree_WidgetState *state = uitree_Map_str_state_get(&t->title_to_state, key);
    bool reset_state = false;
    if (state == NULL) {
        int err = uitree_Map_str_state_upsert(&t->title_to_state, key, (uitree_WidgetState) {0});
        if (err != 0) { state = NULL; goto skip; }
        state = uitree_Map_str_state_get(&t->title_to_state, key);
        reset_state = true;
    } else if ((t->frame - state->__last_frame) > 1) { // Note: Maybe this should go in the cleanup function.
        printfd(ANSI_RED"INFO: (%"PRIstr") Resetting this old state. got %d expected %d", PRIstrarg(key), state->__last_frame, t->frame);
        *state = (uitree_WidgetState) {0};
        reset_state = true;
    } else {
        // This case means we successfully found some previously saved state.
    }
    skip:
    if (state != NULL) {
        if (reset_state && node->user_default_state != NULL) {
            *state = *node->user_default_state;
        }
        state->__last_frame = t->frame;
    } else {
        // This allows dumb containers to have default state without requiring ID.
        if (node->user_default_state != NULL) {
            state = node->user_default_state;
        } else {
            state = &t->state_non_persistent;
        }
    }
    return state;
}


#define DYNA__TYPE ID
#define DYNA__NAMESPACE Vec_oldkeys
#include "da.h"

void uitree__cleanup_saved_state(Uitree *t) {
    // Collect keys to purge.

    Arena arena = *t->arena;
    Strpool old_keys;
    Vec_oldkeys old_keys_ids = Vec_oldkeys_create_with_allocator(arena_allocator, &arena);
    strpool_create_with_allocator(&old_keys, arena_allocator, &arena);

    uitree_Map_str_state_It it = { 0 };
    while (uitree_Map_str_state_it_next(&t->title_to_state, &it)) {
        if ((t->frame - it.value->__last_frame) > 1) {
            ID str_key_id = strpool_append(&old_keys, it.key);
            Vec_oldkeys_append(&old_keys_ids, str_key_id);
        }
    }

    // Remove all.
    for (dyna_foreach(ID, iter, old_keys_ids)) {
        strview_t key = strpool_get(&old_keys, *iter.ref); // WARNING: Before it was: "iter.index" (???)
        uitree_Map_str_state_remove(&t->title_to_state, key);
        printfd(ANSI_RED"DEBUG: Cleaning up this state %"PRIstr, PRIstrarg(key));
    }
}


void uitree__container_calculate_children_area(Uitree *t, Arena *scratch, UILayoutNode *node) {
    //if (node->container_func == NULL) { printfd(ANSI_RED"ERROR: Container has no 'container_function'."); return; }
    if (node->container_func == NULL) {
        for (int i = 0; i < node->children.size; ++i) {
            node->children.items[i].area = node->area;
        }
        return;
    }
    Rect2i *children_areas = arena_new(scratch, Rect2i, node->children.size);
    //strview_t identifier = strpool_get(&t->strpool, node->identifier_strpool_id);
    //uitree_WidgetState *state = uitree__try_get_saved_state(t, identifier, node);
    //wassert(state);
    node->container_func(node->area, node->children.size, children_areas, node->state);
    for (int i = 0; i < node->children.size; ++i) {
        node->children.items[i].area = children_areas[i];
    }
}


typedef struct {
    UILayoutNode *node;
    int child_idx;
    int depth;
} StackItem;

#define LIST__TYPE StackItem
#define LIST__NAMESPACE uitree__List_Stack
#include "list_simple.h"

void uitree__DELME_print_dyna(uitree__List_Stack *list, int depth) {
    for (int i = 0; i < depth; ++i) { printf("    "); }
    printfd(ANSI_BLU"↓↓↓");
    int k = -1;
    uitree__List_Stack_It it = { 0 };
    while (uitree__List_Stack_it_next(list, &it)) {
        ++k;
        StackItem *item = it.item;
        for (int i = 0; i < depth; ++i) { printf("    "); }
        printfd(ANSI_BLU"%d %p: %p %d %d", k, (void*)item, (void*)item->node, item->child_idx, item->depth);
    }
}


/*
void uitree_build_end(Uitree *t) {
    ++t->frame;
    uitree__cleanup_saved_state(t);

    if ((0)) {
        printfd("PRINTING TREE:");
        uitree__print_tree(t, &t->root_node, 0);
    }

    // Iterate over tree to calculate children area.

    {
        Arena arena = *t->arena;
        uitree__List_Stack stack = uitree__List_Stack_create_with_allocator(arena_allocator, &arena);
        StackItem *item;

        t->root_node._area = t->screen;
        uitree__container_calculate_children_area(t, &arena, &t->root_node);
        uitree__List_Stack_append(&stack, (StackItem) { .node = &t->root_node, });

        while (stack.size > 0) {
            STACK_AREA_CONTINUE:
            item = uitree__List_Stack_get_tail(&stack);
            if (!item) { break; }
            while (item->child_idx < item->node->container.children.size) {
                uitree_Node *child = &item->node->container.children.items[item->child_idx];
                ++item->child_idx;
                if (child->is_container && child->container.children.size > 0) {
                    uitree__container_calculate_children_area(t, &arena, child);
                    uitree__List_Stack_append(&stack, (StackItem){ .node = child, });
                    goto STACK_AREA_CONTINUE;
                }
            }
            uitree__List_Stack_remove_tail(&stack);
        }
    }

    // Calculate drawing order.

    t->out_draw_list = uitree_List_DrawInfo_create_with_allocator(arena_allocator, t->arena);

    {
        uitree__List_Stack stack = uitree__List_Stack_create_with_allocator(arena_allocator, t->arena);
        StackItem *item;

        uitree__List_Stack_append(&stack, (StackItem) { .node = &t->root_node, });
        item = uitree__List_Stack_get_tail(&stack); // DELME

        while (stack.size) {
            STACK_ORDER_CONTINUE:
            item = uitree__List_Stack_get_tail(&stack);
            if (!item) { break; }
            while (item->child_idx < item->node->container.children.size) {
                uitree_Node *child = &item->node->container.children.items[item->node->container.children.size -1 -item->child_idx];
                ++item->child_idx;
                if (child->has_user_draw_func) {
                    strview_t identifier = strpool_get(&t->strpool, child->identifier_strpool_id);
                    uitree_WidgetState *state = uitree__try_get_saved_state(t, identifier, child);
                    uitree_DrawInfo draw_info = { .user_draw_func_id = child->user_draw_func_id, .area = child->_area, .state = state, };
                    uitree_List_DrawInfo_append(&t->out_draw_list, draw_info);
                }
                if (child->is_container && child->container.children.size > 0) {
                    uitree__List_Stack_append(&stack, (StackItem){ .node = child, .depth = item->depth +1, });
                    goto STACK_ORDER_CONTINUE;
                }
            }
            int err = uitree__List_Stack_remove_tail(&stack); // Done with this frame.
            wassert(err == 0);
        }
    }

    t->state_non_persistent = (uitree_WidgetState) { 0 };
    return;
}
*/


void uitree_build_end_new(Uitree *t, UILayoutNode root) {
    //uitree__print_tree(t, &root, 0);

    // Iterate over tree to calculate children area.
    {
        Arena scratch = *t->arena;
        uitree__List_Stack stack = uitree__List_Stack_create_with_allocator(arena_allocator, &scratch);
        StackItem *item;

        t->root_node._area = t->screen;
        uitree__container_calculate_children_area(t, &scratch, &root);
        uitree__List_Stack_append(&stack, (StackItem) { .node = &root, });

        while (stack.size > 0) {
            STACK_AREA_CONTINUE:
            item = uitree__List_Stack_get_tail(&stack);
            if (!item) { break; }
            while (item->child_idx < item->node->children.size) {
                UILayoutNode *child = &item->node->children.items[item->child_idx];
                ++item->child_idx;
                if (child->children.size > 0) {
                    uitree__container_calculate_children_area(t, &scratch, child);
                    uitree__List_Stack_append(&stack, (StackItem){ .node = child, });
                    goto STACK_AREA_CONTINUE;
                }
            }
            uitree__List_Stack_remove_tail(&stack);
        }
    }

    //uitree__print_tree(t, &root, 0);

    // Calculate drawing order.

    t->out_draw_list = uitree_List_DrawInfo_create_with_allocator(arena_allocator, t->arena);

    {
        uitree__List_Stack stack = uitree__List_Stack_create_with_allocator(arena_allocator, t->arena);
        StackItem *item;

        uitree__List_Stack_append(&stack, (StackItem) { .node = &root, });
        item = uitree__List_Stack_get_tail(&stack); // DELME

        while (stack.size) {
            STACK_ORDER_CONTINUE:
            item = uitree__List_Stack_get_tail(&stack);
            if (!item) { break; }
            while (item->child_idx < item->node->children.size) {
                UILayoutNode *child = &item->node->children.items[item->node->children.size -1 -item->child_idx];
                ++item->child_idx;
                if (child->user_draw_func_id > 0) {
                    uitree_DrawInfo draw_info = {
                        .user_draw_func_id = child->user_draw_func_id, .area = child->area, .state = child->state,
                        .node_id = strpool_get(&t->strpool, child->identifier)
                    };
                    uitree_List_DrawInfo_append(&t->out_draw_list, draw_info);
                }
                if (child->children.size > 0) {
                    uitree__List_Stack_append(&stack, (StackItem){ .node = child, .depth = item->depth +1, });
                    goto STACK_ORDER_CONTINUE;
                }
            }
            int err = uitree__List_Stack_remove_tail(&stack); // Done with this frame.
            wassert(err == 0);
        }
    }

    t->state_non_persistent = (uitree_WidgetState) { 0 };
    return;
}


uitree_Node uitree_container_dumb(uitree_ContainerFunc *cont_func) {
    return (uitree_Node) {
        .identifier_strpool_id = ID_INVALID,
        .is_container = true,
        .container_func = cont_func,
    };
}

uitree_Node uitree_container_dumb_id(Uitree *t, strview_t id, uitree_ContainerFunc *cont_func) {
    ID str_id = ID_INVALID;
    if (id.size > 0 && id.data != NULL) {
        str_id = strpool_append(&t->strpool, id);
    }
    return (uitree_Node) {
        .identifier_strpool_id = str_id,
        .is_container = true,
        .container_func = cont_func,
    };
}

uitree_Node uitree_container(Uitree *t, strview_t id, uitree_ContainerFunc *cont_func, int user_draw_func_id) {
    ID str_id = ID_INVALID;
    if (id.size > 0 && id.data != NULL) {
        str_id = strpool_append(&t->strpool, id);
    }
    return (uitree_Node) {
        .identifier_strpool_id = str_id,
        .is_container = true,
        .container_func = cont_func,
        .has_user_draw_func = true,
        .user_draw_func_id = user_draw_func_id,
    };
}

uitree_Node uitree_widget(int user_draw_func_id) {
    return (uitree_Node) {
        .identifier_strpool_id = ID_INVALID,
        .has_user_draw_func = true,
        .user_draw_func_id = user_draw_func_id,
    };
}

uitree_Node uitree_widget_id(Uitree *t, int user_draw_func_id, strview_t id) {
    ID str_id = ID_INVALID;
    if (id.size > 0 && id.data != NULL) {
        str_id = strpool_append(&t->strpool, id);
    }
    return (uitree_Node) {
        .identifier_strpool_id = str_id,
        .has_user_draw_func = true,
        .user_draw_func_id = user_draw_func_id,
    };
}


void uitree_container_add_child(Uitree *t, uitree_Node *parent_node, uitree_Node child_node) {
    if (!parent_node->is_container) {
        printfd("WAR: Refusing to add child to non-container node.");
        return;
    }

    if (parent_node->container.children.items == NULL) {
        parent_node->container.children = uitree__Vec_Node_create_with_allocator(arena_allocator, t->arena);
    }

    uitree__Vec_Node_append(&parent_node->container.children, child_node);
}




void widget_add_child(Uitree *t, UILayoutNode *parent_node, UILayoutNode child_node) {
    if (parent_node->children.items == NULL) {
        parent_node->children = uitree__Vec_UILayoutNode_create_with_allocator(arena_allocator, t->arena);
    }
    uitree__Vec_UILayoutNode_append(&parent_node->children, child_node);
}

UILayoutNode make_widget(void) { return (UILayoutNode) {0}; }

void widget_set_id(Uitree *t, UILayoutNode *node, strview_t id) {
    node->identifier = strpool_append(&t->strpool, id);
}

void widget_set_container_function(UILayoutNode *node, uitree_ContainerFunc cont_func) {
    node->container_func = cont_func;
}

void widget_set_state(UILayoutNode *node, Anytype state) {
    node->state = state;
}

void widget_set_drawing_func(UILayoutNode *node, int drawing_func_user_id) {
    node->user_draw_func_id = drawing_func_user_id;
}

#endif
