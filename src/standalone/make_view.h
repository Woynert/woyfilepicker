/*
    Usage:

    #define MAKEVIEW__TYPE <type>
    #include "make_view.h"

    Example:

    #define MAKEVIEW__TYPE Entity
    #include "make_view.h"
    ...
    const Entity_view view = { .items = entities, .size = entity_count };
*/

#ifndef MAKEVIEW__TYPE
#define MAKEVIEW__TYPE int
#endif

/* Token concatenation. */
#define MAKEVIEW__TOKCAT_(a, b) a ## b
#define MAKEVIEW__TOKCAT(a, b) MAKEVIEW__TOKCAT_(a, b)

typedef struct MAKEVIEW__TOKCAT(MAKEVIEW__TYPE, _view) {
	const MAKEVIEW__TYPE *items;
	const int size;
} MAKEVIEW__TOKCAT(MAKEVIEW__TYPE, _view);

#undef MAKEVIEW__TYPE
#undef MAKEVIEW__TOKCAT
#undef MAKEVIEW__TOKCAT_

#ifndef MAKEVIEW__MACROS
#define MAKEVIEW__MACROS
#define view_foreach(type, iter, array) \
	struct { int index; const type *ref; } iter = { .index = 0, .ref = array.items }; iter.index < array.size; ++iter.index, ++iter.ref
#endif
