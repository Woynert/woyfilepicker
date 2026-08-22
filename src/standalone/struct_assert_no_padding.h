/*
   Usage:

    typedef struct {
        #define STRUCT_MEMBERS      \
        X( int        , img       ) \
        X( Rect2i     , source    ) \
        X( Rect2i     , dest      ) \
        X( V2i        , origin    ) \
        X( float      , rotation  ) \
        X( Color      , tint      )
        #define X(type, name) type name;
        STRUCT_MEMBERS
        #undef X
    } dbuf_draw_texture_args_t;

    #define STRUCT_NAME dbuf_draw_texture_args_t
    #include "struct_assert_no_padding.h"

   */
#ifndef STRUCT_MEMBERS
    #error "Must define STRUCT_MEMBERS X-macro list."
#endif
#ifndef STRUCT_NAME
    #error "Must define STRUCT_NAME."
#endif

_Static_assert(
#define X(type, name, ...) sizeof(type) +
STRUCT_MEMBERS
#undef X
0 == sizeof(STRUCT_NAME), "ERROR: Struct contains padding.");

#undef STRUCT_NAME
#undef STRUCT_MEMBERS
