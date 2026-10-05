#ifndef FOCUSER_H
#define FOCUSER_H

#include "strbuf.h"
#include "strbuf_extra.h"
#include "strview.h"

enum {
    FOCUSER_EVENT_NONE,
    FOCUSER_EVENT_NEXT,
    FOCUSER_EVENT_PREV,
};

typedef struct {
    bool has_focus;
    bool _has_custom_event_handler;
} Focusable;


#define WMAPSTR__TYPE Focusable
#define WMAPSTR__NAMESPACE focuser_Map_str_Focusable
#include "wmapstr.h"

typedef struct {
    bool has_focused_id;
    strbuf_t *focused_id;
    //focuser_Map_str_Focusable map;
    //focuser_Map_str_Focusable *map_curr;
    //focuser_Map_str_Focusable *map_prev;
    focuser_Map_str_Focusable widgets;
    //focuser_Map_str_Focusable map2;
    //Strpool strpool;                   // Cleared every cycle.
} Focuser;

void focuser__swap(Focuser *f);

void Focuser_create(Focuser *f) {
    *f = (Focuser) { 0 };
    f->focused_id = strbuf_create_empty(0, 0);
    focuser_Map_str_Focusable_create(&f->widgets);
    //focuser_Map_str_Focusable_create(&f->map2);
    //f->map_curr = &f->widgets;
    //focuser__swap(f);
}

void Focuser_free(Focuser *f) {
    focuser_Map_str_Focusable_free(&f->widgets);
    //focuser_Map_str_Focusable_free(&f->map2);
    strbuf_destroy(&f->focused_id);
    *f = (Focuser) { 0 };
}

void focuser__swap(Focuser *f) {
    //if (f->map_curr == &f->map1) {
        //f->map_curr = &f->map2;
        //f->map_prev = &f->map1;
    //} else {
        //f->map_curr = &f->map1;
        //f->map_prev = &f->map2;
    //}
    // Should we clear the current one... ??
    focuser_Map_str_Focusable_clear(&f->widgets);
}

Focusable focuser_make_focusable(Focuser *f, strview_t id) {
    int err;
    Focusable item = {
        .has_focus = f->has_focused_id && strview_equal(id, strview(f->focused_id)),
    };
    if (!strview_is_valid(id)) { return item; }
    //Focusable *item_prev = focuser_Map_str_Focusable_get(f->map_prev, id);
    //if (!strview_is_valid(id)) { return item; }
    err = focuser_Map_str_Focusable_upsert(&f->widgets, id, item);
    //if (err) { return item; }
    //Focusable *item_prev = focuser_Map_str_Focusable_get(f->map_prev, id);
    //if (err || !item_prev) { return item; }
    return item;
}

void focuser_take_focus(Focuser *f, strview_t id) {
    strbuf_assign(&f->focused_id, id);
    f->has_focused_id = true;
    printfd("focus for ["PRIstrw"] ["PRIstrw"]", PRIstrarg(id), PRIstrargbuf(f->focused_id));
}

//void focuser_skip_item_focus(Focuser *f, bool next_or_prev)

void focuser_advance_focus(Focuser *f, bool next_or_prev) {
    //return;
    if (!f->has_focused_id) { return; }
    if (!focuser_Map_str_Focusable_pair_count(&f->widgets)) { return; }
    focuser_Map_str_Focusable_It it = { 0 };
    int err = focuser_Map_str_Focusable_get_it_for(&f->widgets, strview(f->focused_id), &it, next_or_prev);
    if (err) { printfd("W: Uhh didn't find thee uhh selected uh widget"); return; }
    if (next_or_prev) {
        {
            focuser_Map_str_Focusable_It it2 = it;
            printfd(ANSI_BLU"[init] forwards keys -> "PRIstrw, PRIstrarg(it2.key));
            while(focuser_Map_str_Focusable_it_next(&f->widgets, &it2)){
                printfd(ANSI_BLU"forwards keys -> "PRIstrw, PRIstrarg(it2.key));
            }
        }
        bool wrap_around = !focuser_Map_str_Focusable_it_next(&f->widgets, &it);
        if (wrap_around) {
            it = focuser_Map_str_Focusable_make_it(&f->widgets);
            focuser_Map_str_Focusable_it_next(&f->widgets, &it);
        }
    } else {
        {
            focuser_Map_str_Focusable_It it2 = it;
            printfd(ANSI_BLU"[init] backwards keys -> "PRIstrw, PRIstrarg(it2.key));
            while(focuser_Map_str_Focusable_it_prev(&f->widgets, &it2)){
                printfd(ANSI_BLU"backwards keys -> "PRIstrw, PRIstrarg(it2.key));
            }
        }
        //printfd(ANSI_BLU"PREV1 -> "PRIstrw, PRIstrarg(it.key));
        bool wrap_around = !focuser_Map_str_Focusable_it_prev(&f->widgets, &it);
        //printfd(ANSI_BLU"PREV2 -> "PRIstrw, PRIstrarg(it.key));
        if (wrap_around) {
            printfd("PREV WRAP AROUND");
            it = focuser_Map_str_Focusable_make_it_end(&f->widgets);
            focuser_Map_str_Focusable_it_prev(&f->widgets, &it);
        }
    }
    printfd("BEHOLD YOUR PREV FOCUSED ID ->["PRIstrw"]", PRIstrargbuf(f->focused_id));
    strbuf_assign(&f->focused_id, it.key);
    printfd("BEHOLD YOUR NEW FOCUSED ID ->["PRIstrw"]", PRIstrargbuf(f->focused_id));
}

void focuser__default_event_handler(Focuser *f, Focusable focus, int event) {
    if (!event) { return; }
    else if (event == FOCUSER_EVENT_NEXT) {
        focuser_advance_focus(f, true);
    }
    else if (event == FOCUSER_EVENT_PREV) {
        focuser_advance_focus(f, false);
    }
}

void focuser_focus_next(Focuser *f) {
    if (!focuser_Map_str_Focusable_pair_count(&f->widgets)) { return; }
    Focusable *item = NULL;
    if (f->has_focused_id) {
        //item = focuser_Map_str_Focusable_get(f->map_curr, strview(f->focused_id));
        focuser_Map_str_Focusable_It it;
        //int err = focuser_Map_str_Focusable_get_it_for(f->map_curr, strview(f->focused_id), &it);
        //if (!err) {

        //}
    }
    if (!item) {
        printferr("Didn't find it oh no! ["PRIstrw"]", PRIstrargbuf(f->focused_id));
        focuser_Map_str_Focusable_It it = focuser_Map_str_Focusable_make_it(&f->widgets);
        focuser_Map_str_Focusable_it_next(&f->widgets, &it);
        item = it.value;
        strbuf_assign(&f->focused_id, it.key);
        printferr("So instead we found ["PRIstrw"]", PRIstrarg(it.key));
        f->has_focused_id = true;
    }
    if (!item) { return; }
}

void focuser_print_debug(Focuser *f) {
    printfd("---");
    printfd("has_focused_id? %d ->["PRIstrw"]", f->has_focused_id, PRIstrarg(strview(f->focused_id)));
    printfd("Printing focusable widgets::");
    focuser_Map_str_Focusable_It it = focuser_Map_str_Focusable_make_it(&f->widgets);
    while(focuser_Map_str_Focusable_it_next(&f->widgets, &it)) {
        printfd(PRIstrw" (has_focus? %d)", PRIstrarg(it.key), f->has_focused_id && wstrview_equals(it.key, strview(f->focused_id)));
    }
    printfd("---");

}



//int focuser_enable_keyboard_events(

#endif
