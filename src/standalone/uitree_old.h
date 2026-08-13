#ifndef UITREE_H
#define UITREE_H

#include "arena_extra.h"
#include "raylib_extra.h"
#include "strview.h"

#ifndef DRAW_FUNC_ARG_TYPE
#define DRAW_FUNC_ARG_TYPE void
#endif


typedef enum {
    ContainerRoot, /* Just has one children */
    ContainerSplitH,
    ContainerSplitV,
    ContainerListH, /* Lists will render all children until one has a NULL draw_func. */
    ContainerListV,
    ContainerStack,
} ContainerType;

typedef struct wgtr_WidgetState {
    int scroll;
    int int_b;
    Rect2i rect_a;
    int __last_frame; // Used to determine if we should persist it or forget it.
} wgtr_WidgetState;

#define STRMAP__TYPE wgtr_WidgetState
#define STRMAP__NAMESPACE wgtr_Map_str_state
#include "../subprojects/woycontainer/src/strmap.h"

typedef struct Node Node;

typedef struct DrawInfo {
    int    user_draw_func_id;          // An id for the user to identify which function to call.
    Rect2i area;
    wgtr_WidgetState *state;       // Persistent state, can be used to keep container's scroll value, etc.
    bool     container_layout_request; // Only true, if this widget is marked as a Container.
    int      child_count;
    Rect2i **out_children;
    int layer;
} DrawInfo;

#define LIST__TYPE DrawInfo
#define LIST__NAMESPACE wgtr_List_DrawInfo
#include "list_simple.h"

#define DYNA__TYPE wgtr_List_DrawInfo
#define DYNA__NAMESPACE wgtr_Vec_List_DrawInfo
#include "da.h"

#define DYNA__TYPE Node
#define DYNA__NAMESPACE uitree_node_da
#define DYNA__ONLY_HEADER
#include "da.h"

typedef void (wgtr_ContainerFunc)(Rect2i area, int child_count, Rect2i *children, void *user_ctx, wgtr_WidgetState *state);

typedef struct Node {
    //strview_t identifier;    // It's OK if a widget has no identifier. It's state won't persist thru frames.
    int identifier_strpool_id;

    bool has_user_draw_func;  // User must check this, if adding an user_function_id.
    int user_draw_func_id;    // An id for the user to identify which function to call.

    wgtr_ContainerFunc *container_func;

    bool is_container;
    struct {
        void *user_ctx; // User can set this to access it later on 'container_func' call.
        ContainerType type;
        uitree_node_da children;
    } container;

    Rect2i _area; // Used to hold the area momentarily during end_tree.
} Node;

#define DYNA__TYPE Node
#define DYNA__NAMESPACE uitree_node_da
#define DYNA__ONLY_IMP
#include "da.h"

typedef struct Uitree {
    ArenaRoot arenaroot;
    Arena arena;
    strpool strpool; // For temporarily storing Nodes' identifiers.

    /*
    Each state has an "int frame;" member. If we find this title on
    the current cycle we update it. Then, at the end of frame. If we find
    any state with outdate frames then we remove it.
    */
    // Maps a title to a state. We might add or delete from this every frame.
    wgtr_Map_str_state title_to_state;
    //Map<string, State>
    //wgtr_Map_str_State

    int frame;

    Node root_node;
    Rect2i screen;

    // Only populated on "frame_end".
    // Layers -> Lists -> DrawInfo.
    //wgtr_Vec_DrawInfo out_draw_list;
    wgtr_List_DrawInfo out_draw_list;

    wgtr_WidgetState state_non_persistent; // Fallback for nodes which have no identifier.
} Uitree;

/// @Returns error.
int uitree_create(Uitree *t) {
    *t = (Uitree) { 0 };
    t->arenaroot = ArenaRoot_create(1 << 20);
    //t->draws_da = uitree_drawinfo_da_create();
    return wgtr_Map_str_state_create(&t->title_to_state);
}

void uitree_free(Uitree *t) {
    ArenaRoot_free(&t->arenaroot);
    wgtr_Map_str_state_free(&t->title_to_state);
    //uitree_drawinfo_da_free(&t->draws_da);
    *t = (Uitree) { 0 };
}

void uitree_build_start(Uitree *t, Rect2i screen) {
    t->arena = ArenaRoot_get_arena(t->arenaroot);
    t->screen = screen;
    strpool_create_with_allocator(&t->strpool, arena_allocator, &t->arena);
}

void uitree_print_tree(Uitree *t, Node *node, int level) {
    for (int i = 0; i < level; ++i) {
        printf("   ");
    }
    printf("%s", node->is_container ? "Container" : "Widget");

    if (!node->is_container) {
        printf("\n");
        return;
    }

    printf(" type:%d", node->container.type);
    strview_t identifier = strpool_get(&t->strpool, node->identifier_strpool_id);
    if (identifier.size > 0) {
        printf(" id:%"PRIstr, PRIstrarg(identifier));
    }
    printf("\n");

    for (int i = 0; i < node->container.children.size; ++i) {
        uitree_print_tree(t, &node->container.children.items[i], level+1);
    }
}

Rect2i uitree_calculate_area_for_child_n(ContainerType type, Rect2i source, int children_count, int i) {
    Rect2i area = source;
    switch (type) {
        case ContainerRoot: break;
        case ContainerListV:
        {
            //area.x = source.x;
            //area.width = source.width;
            area.height = (source.height / children_count);
            area.y = source.y + area.height * i;
            if (i == (children_count-1)) { // Fixes missing pixels.
                area.height = source.height - area.height * (children_count -1);
            }
        } break;
        case ContainerListH:
        {
            //area.y = source.y;
            //area.height = source.height;
            area.width = (source.width / children_count);
            area.x = source.x + area.width * i;
            if (i == (children_count-1)) { // Fixes missing pixels.
                area.width = source.width - area.width * (children_count -1);
            }
        } break;
        default: break;
    }
    return area;
}


/// @Returns state or NULL.
wgtr_WidgetState * uitree__try_get_saved_state(Uitree *t, strview_t key) {
    if (key.size == 0) {
        // No key means no state, sorry.
        return NULL;
    }
    wgtr_WidgetState *state = wgtr_Map_str_state_get(&t->title_to_state, key);
    if (state == NULL) {
        int err = wgtr_Map_str_state_upsert(&t->title_to_state, key, (wgtr_WidgetState) {0});
        if (err != 0) { return NULL; }
        state = wgtr_Map_str_state_get(&t->title_to_state, key);
    } else if ((t->frame - state->__last_frame) > 1) { // Note: Maybe this should go in the cleanup function.
        printfd(ANSI_RED"INFO: (%"PRIstr") Resetting this old state. got %d expected %d", PRIstrarg(key), state->__last_frame, t->frame);
        *state = (wgtr_WidgetState) {0};
    } else {
        //printfd(ANSI_GRE"INFO: (%"PRIstr") State. got %d expected %d", PRIstrarg(key), state->__last_frame, t->frame);
    }
    state->__last_frame = t->frame;
    return state;
}

#define DYNA__TYPE int
#define DYNA__NAMESPACE Vec_oldkeys
#include "da.h"

void uitree__cleanup_saved_state(Uitree *t) {
    // Collect keys to purge.

    Arena arena = t->arena;
    strpool old_keys;
    Vec_oldkeys old_keys_ids = Vec_oldkeys_create_with_allocator(arena_allocator, &arena);
    strpool_create_with_allocator(&old_keys, arena_allocator, &arena);

    wgtr_Map_str_state_It it = { 0 };
    while (wgtr_Map_str_state_it_next(&t->title_to_state, &it)) {
        if ((t->frame - it.value->__last_frame) > 1) {
            int str_key_id = strpool_append(&old_keys, it.key);
            Vec_oldkeys_append(&old_keys_ids, str_key_id);
        }
    }

    // Remove all.
    for (dyna_foreach(int, iter, old_keys_ids)) {
        strview_t key = strpool_get(&old_keys, iter.index);
        wgtr_Map_str_state_remove(&t->title_to_state, key);
        printfd(ANSI_RED"DEBUG: Cleaning up this state %"PRIstr, PRIstrarg(key));
    }
}


typedef struct {
    Node *node;
    int child_idx;
    int depth;
} StackItem;
#define DYNA__TYPE StackItem
#define DYNA__NAMESPACE uitree_stack_da
#include "da.h"

void uitree_build_end(Uitree *t) {
    ++t->frame;
    uitree__cleanup_saved_state(t);

    if ((1)) {
        printfd("PRINTING TREE:");
        uitree_print_tree(t, &t->root_node, 0);
    }

    wgtr_Vec_List_DrawInfo layers = wgtr_Vec_List_DrawInfo_create_with_allocator(arena_allocator, &t->arena);

    printfd("Starting size calculation.");
    {
        // Saving only containers in this stack.

        uitree_stack_da stack = uitree_stack_da_create_with_allocator(arena_allocator, &t->arena);

        t->root_node._area = t->screen;
        StackItem new_item = {
            .node = &t->root_node,
            .child_idx = 0,
            .depth = 0,
        };
        goto NEW_ITEM;

        NEW_ITEM:
        {
            if (new_item.node->container.children.size > 0) {
                // Calculate children areas.
                // Purposefully avoiding VLA here.
                Arena arena_tmp = t->arena;
                Rect2i *children_areas = arena_new(&arena_tmp, Rect2i, new_item.node->container.children.size);
                if (new_item.node->container_func != NULL) {
                    strview_t identifier = strpool_get(&t->strpool, new_item.node->identifier_strpool_id);
                    wgtr_WidgetState *state = uitree__try_get_saved_state(t, identifier);
                    state = state ? state : &t->state_non_persistent;
                    new_item.node->container_func(new_item.node->_area, new_item.node->container.children.size, children_areas, new_item.node->container.user_ctx, state);
                    for (int i = 0; i < new_item.node->container.children.size; ++i) {
                        new_item.node->container.children.items[i]._area = children_areas[i];
                    }
                } else {
                    printfd("ERROR: Container has no 'container_function'.");
                }
            }
            uitree_stack_da_append(&stack, new_item);
        }

        CONTINUE:
        {
            StackItem *curr_item = uitree_stack_da_get_safe(&stack, stack.size-1);
            Node *parent = curr_item->node;

            for (; curr_item->child_idx < parent->container.children.size; ++curr_item->child_idx) {

                Node *node = &parent->container.children.items[curr_item->child_idx];

                if (node->has_user_draw_func) {

                    // Add to collection of DrawInfos.

                    int depth = curr_item->depth;
                    while(depth >= layers.size) {
                        wgtr_List_DrawInfo new_layer = wgtr_List_DrawInfo_create_with_allocator(arena_allocator, &t->arena);
                        wgtr_Vec_List_DrawInfo_append(&layers, new_layer);
                    }
                    wgtr_List_DrawInfo *draw_layer = &layers.items[depth];
                    strview_t identifier = strpool_get(&t->strpool, node->identifier_strpool_id);
                    printfd("(i %d) Widget right here! :) id (%d) (%"PRIstr") "Rect2i_Fmt, curr_item->child_idx, node->identifier_strpool_id, PRIstrarg(identifier), Rect2i_Arg(node->_area));

                    // Check if we have state for this one.

                    wgtr_WidgetState *state = uitree__try_get_saved_state(t, identifier);
                    state = state ? state : &t->state_non_persistent;
                    DrawInfo draw_info = {
                        .user_draw_func_id = node->user_draw_func_id,
                        .area = node->_area,
                        .state = state,
                        .layer = depth,
                    };

                    wgtr_List_DrawInfo_append(draw_layer, draw_info);
                }

                if (node->is_container) {
                    ++curr_item->child_idx;
                    new_item = (StackItem) {
                        .node = node,
                        .child_idx = 0,
                        //.area = node->_area,
                        .depth = curr_item->depth +1,
                    };

                    goto NEW_ITEM;
                }
            }
            // Finished with the children. Let's pop and go up.
            uitree_stack_da_remove_at(&stack, stack.size-1);
            if (stack.size > 0) {
                goto CONTINUE;
            }
        }
    }
    printfd("TREE HAS BEEN UHH PROCESSED");
    printfd("Printing layered things");

    // Populate final list.
    // aka. Flatenning the list so the user can just iterate through it.

    t->out_draw_list = wgtr_List_DrawInfo_create_with_allocator(arena_allocator, &t->arena);

    for (int i = layers.size -1; i > -1; i += -1) {
        wgtr_List_DrawInfo_It it = { 0 };
        while(wgtr_List_DrawInfo_it_next(&layers.items[i], &it)) {
            wgtr_List_DrawInfo_append(&t->out_draw_list, *it.item);
            printfd(ANSI_CYA"(depth %d) Widget right here! :) "Rect2i_Fmt, i, Rect2i_Arg(it.item->area));
        }
    }

    t->state_non_persistent = (wgtr_WidgetState) { 0 };

    printfd("This is the end");

}

Node uitree_dumb_container(Uitree *t, strview_t id, wgtr_ContainerFunc *cont_func) {
    int str_id = -1;
    if (id.size > 0 && id.data != NULL) {
        str_id = strpool_append(&t->strpool, id);
    }
    return (Node) {
        .identifier_strpool_id = str_id,
        .is_container = true,
        .container_func = cont_func,
    };
}

Node uitree_container(Uitree *t, strview_t id, wgtr_ContainerFunc *cont_func, int user_draw_func_id) {
    int str_id = -1;
    if (id.size > 0 && id.data != NULL) {
        str_id = strpool_append(&t->strpool, id);
    }
    return (Node) {
        .identifier_strpool_id = str_id,
        .is_container = true,
        .container_func = cont_func,
        .has_user_draw_func = true,
        .user_draw_func_id = user_draw_func_id,
    };
}

Node uitree_widget(int user_draw_func_id) {
    return (Node) {
        .identifier_strpool_id = -1,
        .has_user_draw_func = true,
        .user_draw_func_id = user_draw_func_id,
    };
}


void uitree_container_add_child(Uitree *t, Node *parent_node, Node child_node) {
    if (!parent_node->is_container) {
        printfd("WAR: Refusing to add child to non-container node.");
        return;
    }

    if (parent_node->container.children.items == NULL) {
        parent_node->container.children = uitree_node_da_create_with_allocator(arena_allocator, &t->arena);
    }

    uitree_node_da_append(&parent_node->container.children, child_node);
}

#endif
