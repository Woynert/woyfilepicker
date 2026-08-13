#include "strbuf_extra.h"
#include "woytest.h"
#include "wstrview.h"


void wstrview_print_scape_newlines(strview_t str) {
    if (str.size < 0) { return; }
    for (int i = 0; i < str.size; ++i) {
        if (str.data[i] == '\n') {
            printf("\\n");
        } else {
            printf("%c", str.data[i]);
        }
    }
}
void wstrview_print(strview_t str) {
    printf("\"");
    wstrview_print_scape_newlines(str),
    printf("\" (size %d)", str.size);
}
#define ASSERT_STR(a,b) WOYTEST_TEMPL(a, b, strview_t, wstrview_equals, "ASSERT_STR", wstrview_print, "==")


TEST test_general(void) {
    const strview_t invalid = ((strview_t){.data=NULL,.size=0});
    ASSERT(strview_ends_with(cstr_SL("Hello there"), cstr_SL("there")));
    ASSERT(strview_ends_with(cstr_SL(""), cstr_SL("")));
    ASSERT(strview_ends_with(invalid, cstr_SL("")));
    ASSERT(strview_ends_with(cstr_SL(""), invalid));
    ASSERT(strview_ends_with(invalid, invalid));
    ASSERT_FALSE(strview_ends_with(invalid, cstr_SL("mimo")));
    ASSERT_TRUE(strview_ends_with(cstr_SL("myfilesare.here.txt"), cstr_SL(".here.txt")));
    ASSERT_TRUE(strview_ends_with(cstr_SL("0123456789"), cstr_SL("123456789")));
    ASSERT_TRUE(strview_ends_with(cstr_SL("123456789"), cstr_SL("123456789")));
    ASSERT_TRUE(strview_ends_with(cstr_SL("\n\n\n"), cstr_SL("\n")));
    TEST_PASS;
}


TEST test_wstrview_get_next_line(void) {
    strview_t data = cstr("1\n2\n3\n4");
    strview_t line;
    line = wstrview_get_next_line(&data);
    ASSERT_STR(line, cstr_SL("1"));
    ASSERT_STR(data, cstr_SL("2\n3\n4"));
    line = wstrview_get_next_line(&data);
    ASSERT_STR(line, cstr_SL("2"));
    ASSERT_STR(data, cstr_SL("3\n4"));
    line = wstrview_get_next_line(&data);
    ASSERT_STR(line, cstr_SL("3"));
    ASSERT_STR(data, cstr_SL("4"));
    line = wstrview_get_next_line(&data);
    ASSERT_STR(line, cstr_SL("4"));
    ASSERT_STR(data, cstr_SL(""));
    line = wstrview_get_next_line(&data);
    ASSERT_STR(line, cstr_SL(""));
    ASSERT_STR(data, cstr_SL(""));
    TEST_PASS;
}


int main(void) {
    TESTS_INIT();
    RUN_TEST(test_general);
    RUN_TEST(test_wstrview_get_next_line);
    TESTS_SHOW_RESULTS();
}
