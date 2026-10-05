#ifndef ANYTYPE_USER_TYPES_H
#define ANYTYPE_USER_TYPES_H


/// Add your types here:
/// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓


#define ANYTYPE_LIST \
    ANYTYPE_ITEM_X(Widget_2split_State) \
    ANYTYPE_ITEM_X(Simple_Scroll_t)     \
    ANYTYPE_ITEM_X(Complex_Scroll_t)    \


/// ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑


enum ANYTYPE {
    ANYTYPE_INVALID,
    #define ANYTYPE_ITEM_X(a) ANYTYPE_##a,
    ANYTYPE_LIST
    #undef ANYTYPE_ITEM_X
};

#endif
