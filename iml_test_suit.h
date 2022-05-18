#if !defined(IML_TEST_SUIT_H)
#define IML_TEST_SUIT_H


#include <stdarg.h>

#include "iml_types.h"
#include "iml_array.h"
#include "iml_stack.h"

#include "iml_terminal.h"


//
// @note: TS - Testing System || TS - Test Suit
//
struct __Ts_Config__;
struct __Ts_Node__;

void __ts_print_indentation__(int level);
char *__ts_format_string__(Memory_Arena *arena, const char *format, va_list va);
char *__ts_format_string__(Memory_Arena *arena, const char *format, ...);


//
// @note: Node
//
typedef void __Ts_Node_Function__(__Ts_Config__ *__ts_config__, __Ts_Node__ *__ts_current_node__);

enum __Ts_Node_Type__ {
    __TS_NODE_ROOT__    = 1,
    __TS_NODE_UNIT__    = 2,
    __TS_NODE_GROUP__   = 3,
    __TS_NODE_TEST__    = 4,
    __TS_NODE_INTERIM__ = 5,
};

struct __Ts_Node_Array__ {
    Array <__Ts_Node__ *> data;
    
    // Data that needs to be set for the next added item:
    __Ts_Node__ *current_node;
    __Ts_Node_Type__ current_type;
    const char *current_name;
};

void operator+=(__Ts_Node_Array__ &array, __Ts_Node_Function__ *func);

struct __Ts_Node__ {
    Memory_Arena *arena;
    __Ts_Node__ *parent_node;
    
    int indent_level;
    
    const char *name;
    __Ts_Node_Type__ type;
    __Ts_Node_Function__ *func;
    
    // Interim nodes
    __Ts_Node_Array__ adder_before_all;
    __Ts_Node_Array__ adder_after_all;
    __Ts_Node_Array__ adder_before_each;
    __Ts_Node_Array__ adder_after_each;
    
    // Sub groups or tests
    __Ts_Node_Array__ adder_children;
    
    // Running data
    int total_checks  = 0;
    int checks_failed = 0;
};

__Ts_Node__ *__ts_make_node__(Memory_Arena *arena, int indent_level,
                              const char *name, __Ts_Node_Type__ type, __Ts_Node_Function__ *func);
void __ts_run_node__(__Ts_Config__ *config, __Ts_Node__ *node, __Ts_Node__ *previous_node = null);


//
// @note: Config
//
struct __Ts_Config__ {
    void init(int indent_level, const char *suit_name, ...);
    Memory_Arena arena;
    
    const char *suit_name;
    int indent_level;
    __Ts_Node__ *root_node;
};

void __ts_run_config__(__Ts_Config__ *config);


//
// @note: check
//
void __ts_failed_check__(__Ts_Node__ *current_node,
                         const char *condition_string, const char *file_string, int line_number, const char *function_name,
                         const char *message_format = null, ...);


//
// @note: Macros
//
#if !BUILD_TESTS

#define _test_suit_init(...)
#define _tes_suit_deinit(...)
#define test_suit(...)
#define __TS_NODE__(...)  auto CONCAT(CONCAT(__ts_node_, __LINE__), __COUNTER__) = [](__Ts_Config__ *__ts_config__, __Ts_Node__ *__ts_current_node__) -> void
#define check(...)

#else


#define _test_suit_init(suit_name, ...)  \
__ts_config__.init(0, suit_name, __VA_ARGS__);  \
__ts_current_node__ = __ts_config__.root_node;

#define _test_suit_deinit()  __ts_run_config__(&__ts_config__);

#define test_suit(...)  \
__Ts_Config__ __ts_config__;  \
__Ts_Node__ *__ts_current_node__;  \
_test_suit_init(__VA_ARGS__);  \
defer { _test_suit_deinit(); };

#define __TS_NODE__(node_list, node_type, ...) \
__ts_current_node__->node_list.current_node = __ts_current_node__; \
__ts_current_node__->node_list.current_type = (node_type); \
__ts_current_node__->node_list.current_name = __ts_format_string__(__ts_current_node__->arena, __VA_ARGS__); \
__ts_current_node__->node_list += [](__Ts_Config__ *__ts_config__, __Ts_Node__ *__ts_current_node__) -> void

#define check(condition, ...) do { \
__ts_current_node__->total_checks += 1; \
if (!(static_cast<bool>(condition))) { \
__ts_failed_check__(__ts_current_node__, #condition, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
} \
} while (0)

#endif


#define unit(...)      __TS_NODE__(adder_children,    __TS_NODE_UNIT__,    __VA_ARGS__)
#define group(...)     __TS_NODE__(adder_children,    __TS_NODE_GROUP__,   __VA_ARGS__)
#define test(...)      __TS_NODE__(adder_children,    __TS_NODE_TEST__,    __VA_ARGS__)

#define before_all()   __TS_NODE__(adder_before_all,  __TS_NODE_INTERIM__, "before_all")
#define after_all()    __TS_NODE__(adder_after_all,   __TS_NODE_INTERIM__, "after_all")

#define before_each()  __TS_NODE__(adder_before_each, __TS_NODE_INTERIM__, "before_each")
#define after_each()   __TS_NODE__(adder_after_each,  __TS_NODE_INTERIM__, "after_each")


//
// @note: Global Test_Suit
//
#if TS_RUN_GLOBAL_TESTS && BUILD_TESTS

typedef void __Ts_Global_Test_Function__(__Ts_Config__ *__ts_config__, __Ts_Node__ *__ts_current_node__);

struct __Ts_Global_Test_Suit__ {
    __Ts_Config__ __ts_config__;
    __Ts_Node__ *__ts_current_node__;
    
    __Ts_Global_Test_Suit__();
    ~__Ts_Global_Test_Suit__();
};
global __Ts_Global_Test_Suit__ __ts_global_test_suit__;

struct __Ts_Global_Test_Function_Adder__ {
    char *name;
    
    __Ts_Global_Test_Function_Adder__(const char *name, ...);
    int operator +(__Ts_Global_Test_Function__ *func);
};

#define global_unit(name, ...)  static int CONCAT(CONCAT(__ts_global_unit_, __LINE__), CONCAT(_, __COUNTER__)) = __Ts_Global_Test_Function_Adder__(name, ##__VA_ARGS__) + (__Ts_Global_Test_Function__ *) [](__Ts_Config__ *__ts_config__, __Ts_Node__ *__ts_current_node__) -> void

#else

#define global_unit(...)  void CONCAT(CONCAT(__ts_global_unit_, __LINE__), __COUNTER__) (__Ts_Config__ *__ts_config__, __Ts_Node__ *__ts_current_node__)

#endif


//~
//~
#endif // IML_TEST_SUIT_H
//
//~
//~
// @note: Implementation
//
//~
//~
#if defined(IML_TEST_SUIT_IMPLEMENTATION)
#if !defined(IML_TEST_SUIT_CPP)
#define IML_TEST_SUIT_CPP


void __ts_print_indentation__(int level) {
    for (int i = 0; i < level; ++i) {
        printf("    ");
    }
}

char *__ts_format_string__(Memory_Arena *arena, const char *format, va_list va) {
    int size = vsnprintf(0, 0, format, va) + 1;
    char *result = push_array(arena, char, size);
    vsnprintf(result, size, format, va);
    return result;
}

char *__ts_format_string__(Memory_Arena *arena, const char *format, ...) {
    va_list va;
    va_start(va, format);
    char *result = __ts_format_string__(arena, format, va);
    va_end(va);
    return result;
}


//
// @note: Node
//
void operator+=(__Ts_Node_Array__ &array, __Ts_Node_Function__ *func) {
    assert(array.current_node);
    array.data.add(__ts_make_node__(array.current_node->arena, array.current_node->indent_level + 1,
                                    array.current_name, array.current_type, func));
}

__Ts_Node__ *__ts_make_node__(Memory_Arena *arena, int indent_level,
                              const char *name, __Ts_Node_Type__ type, __Ts_Node_Function__ *func) {
    auto node = push_struct(arena, __Ts_Node__);
    assert(node);
    *node = {};
    
    node->arena = arena;
    
    node->indent_level = indent_level;
    node->name = name;
    node->type = type;
    node->func = func;
    
    return node;
}

void __ts_run_node__(__Ts_Config__ *config, __Ts_Node__ *node, __Ts_Node__ *previous_node) {
    bool output_after_execution = true;
    
    //~ @note: Print node name.
    if (!output_after_execution) {
        if (node->type != __TS_NODE_INTERIM__ && node->type != __TS_NODE_ROOT__) {
            __ts_print_indentation__(node->indent_level);
            printf("%s\n", node->name);
        }
    }
    else {
        if (node->type == __TS_NODE_UNIT__ || node->type == __TS_NODE_GROUP__) {
            __ts_print_indentation__(node->indent_level);
            printf("Running %s:\n", node->name);
        }
    }
    
    //~
#define _run_node(it, ...) do { \
if (0) { printf("run_node(\"%s\"; @line=%d\n", it->name, __LINE__); } \
__ts_run_node__(config, it  , ## __VA_ARGS__); \
node->total_checks += it->total_checks; \
it->total_checks = 0; \
node->checks_failed += it->checks_failed; \
it->checks_failed = 0; \
} while (0)
    
    
    //~ @note: Run current node scope.
    assert(node);
    {
        if (previous_node) {  // :previous_node So that we can associate before_each/after_each with the actual running node.
            For (previous_node->adder_before_each.data)  _run_node(it);
        }
        if (node->func)  node->func(config, node);
        if (previous_node) {  // :previous_node
            For (previous_node->adder_after_each.data)  _run_node(it);
        }
    }
    
    //~ @note: Run sub nodes.
    if (node->type == __TS_NODE_INTERIM__) {
        assert(!node->adder_before_all.data.count, "Interim nodes can not have sub nodes!");
        assert(!node->adder_after_all.data.count, "Interim nodes can not have sub nodes!");
        assert(!node->adder_before_each.data.count, "Interim nodes can not have sub nodes!");
        assert(!node->adder_after_each.data.count, "Interim nodes can not have sub nodes!");
        assert(!node->adder_children.data.count, "Interim nodes can not have sub nodes!");
    }
    else if (node->type == __TS_NODE_GROUP__) {
        if (previous_node) {
            For (previous_node->adder_before_all.data)   node->adder_before_all.data.insert_at(0, it);
            For (previous_node->adder_after_all.data)    node->adder_after_all.data.add(it);
            For (previous_node->adder_before_each.data)  node->adder_before_each.data.insert_at(0, it);
            For (previous_node->adder_after_each.data)   node->adder_after_each.data.add(it);
        }
        
        For (node->adder_children.data)  _run_node(it, node);
    }
    else if (node->type == __TS_NODE_ROOT__ || node->type == __TS_NODE_UNIT__ || node->type == __TS_NODE_TEST__) {
        For (node->adder_before_all.data)  _run_node(it);
        For (node->adder_children.data)    _run_node(it, node);  // :previous_node Passing the previous node only to the children, not interim nodes.
        For (node->adder_after_all.data)   _run_node(it);
    }
    else {
        invalid_path;
    }
    
#undef _run_node
    
    //~ @note: Print results
    auto colors = get_terminal_colors();  // @todo Store in config, witch optional option to disable it.
    
    if (output_after_execution && (node->type != __TS_NODE_INTERIM__ && node->type != __TS_NODE_ROOT__)) {
        const char *status;
        if (node->total_checks <= 0) {
            status = tprint("%sNO CHECKS%s", colors.magenta, colors.reset);
        }
        else if (node->checks_failed) {
            status = tprint("%sERROR%s", colors.red, colors.reset);
        }
        else {
            status = tprint("%sOK%s", colors.green, colors.reset);
        }
        __ts_print_indentation__(node->indent_level);
        printf("%s  .... %s\n", node->name, status);
    }
    
    if (node->type == __TS_NODE_UNIT__ || node->type == __TS_NODE_GROUP__) {
        if (node != *previous_node->adder_children.data.peek()) {
            printf("\n");
        }
    }
    else if (node->type == __TS_NODE_ROOT__) {
        if (node->adder_children.data.count > 0 &&
            ((node->adder_children.data[node->adder_children.data.count - 1]->type != __TS_NODE_UNIT__) &&
             (node->adder_children.data[node->adder_children.data.count - 1]->type != __TS_NODE_GROUP__))) {
            printf("\n");
        }
        
        //~ @note Print results
        __ts_print_indentation__(node->indent_level);
        printf("\n------------------------------------------\n");
        int checks_failed = node->checks_failed;
        if (checks_failed > 0) {
            __ts_print_indentation__(node->indent_level);
            printf("- %sERROR:%s %d check%s failed!\n", colors.red, colors.reset,
                   checks_failed, (checks_failed == 1) ? "" : "s");
        }
        else {
            __ts_print_indentation__(node->indent_level);
            printf("- %sPASSED%s\n", colors.green, colors.reset);
        }
    }
    
    // @note:
    // Output:
    // Lexer tests:  .... [2/2] OK
    //   Integer tests:  .... OK
    //     Equal:  .... OK
    //   Float tests:  .... [1/2] ERROR
    //     Equal: 1 == 2  .... ERROR
    //     Not Equal:  .... OK
    // PASSED
}


//
// @note: Config
//
void __Ts_Config__::init(int indent_level, const char *format, ...) {
    this->arena = os_make_arena();
    
    va_list va;
    va_start(va, format);
    const char *suit_name = __ts_format_string__(&this->arena, format, va);
    va_end(va);
    
    this->suit_name = suit_name;
    this->indent_level = indent_level;
    this->root_node = __ts_make_node__(&arena, indent_level, suit_name, __TS_NODE_ROOT__, 0);
}

void __ts_run_config__(__Ts_Config__ *config) {
    printf("\n");
    __ts_print_indentation__(config->indent_level);
    printf("------------------------------------------\n");
    __ts_print_indentation__(config->indent_level);
    printf("- Running test suit '%s':\n", config->suit_name);
    __ts_print_indentation__(config->indent_level);
    printf("------------------------------------------\n");
    printf("\n");
    
    //~ @note: Run root node
    __ts_run_node__(config, config->root_node);
    
    __ts_print_indentation__(config->indent_level);
    printf("------------------------------------------\n");
}


//
// @note: check
//
void __ts_failed_check__(__Ts_Node__ *current_node,
                         const char *condition_string, const char *file_string, int line_number, const char *function_name,
                         const char *message_format, ...) {
    Memory_Arena *arena = current_node->arena;
    auto colors = get_terminal_colors();  // @todo Store in config, witch optional option to disable it.
    
    current_node->checks_failed += 1;
    
    const char *separator = "";
    const char *message = "";
    if (message_format) {
        separator = ", ";
        va_list va;
        va_start(va, message_format);
        message = __ts_format_string__(arena, message_format, va);
        va_end(va);
    }
    
    __ts_print_indentation__(current_node->indent_level + 1);
    printf("%sCheck failed:%s '%s'%s%s\n", colors.red, colors.reset, condition_string, separator, message);
    __ts_print_indentation__(current_node->indent_level + 1);
    printf("    %sOn file:%s %s\n",      colors.green, colors.reset, file_string);
    __ts_print_indentation__(current_node->indent_level + 1);
    printf("    %sOn line:%s %d\n",      colors.green, colors.reset, line_number);
}


//
// @note: Global Test_Suit
//
#if TS_RUN_GLOBAL_TESTS && BUILD_TESTS
__Ts_Global_Test_Suit__::__Ts_Global_Test_Suit__() {
    _test_suit_init("Global Tests");
}

__Ts_Global_Test_Suit__::~__Ts_Global_Test_Suit__() {
    {
#if defined(TS_OUTPUT_GLOBAL_TESTS_TO_LOGFILE_PATH)
#if !BUILD_WITH_NO_CRT
#error "Can't output to logfile when compiling with CRT!"
#endif
        auto _stdout = stdout;
        auto _stderr = stderr;
        File_Handle test_output_file = os_open_file(TS_OUTPUT_GLOBAL_TESTS_TO_LOGFILE_PATH, OPEN_FILE_write);
        stdout = &test_output_file;
        stderr = &test_output_file;
        defer {
            stdout = _stdout;
            stderr = _stderr;
        };
#endif
        
        _test_suit_deinit();
    }
    
#if 0
    //assert(__ts_config__.root_node->checks_failed == 0);
#else
    if (__ts_config__.root_node->checks_failed > 0) {
        auto colors = get_terminal_colors();
        printf("%sERROR:%s Global_Test_Suit %d check(s) failed!\n", colors.red, colors.reset, __ts_config__.root_node->checks_failed);
    }
#endif
}

__Ts_Global_Test_Function_Adder__::__Ts_Global_Test_Function_Adder__(const char *name, ...) {
    va_list va;
    va_start(va, name);
    this->name = __ts_format_string__(__ts_global_test_suit__.__ts_current_node__->arena, name, va);
    va_end(va);
}

int __Ts_Global_Test_Function_Adder__::operator +(__Ts_Global_Test_Function__ *func) {
    __Ts_Node__ *__ts_current_node__ = __ts_global_test_suit__.__ts_current_node__;
    __ts_current_node__->adder_children.data.add(__ts_make_node__(__ts_current_node__->arena, __ts_current_node__->indent_level + 1, name, __TS_NODE_UNIT__, func));
    return 0;
}
#endif


#endif // IML_TEST_SUIT_CPP
#endif // IML_TEST_SUIT_IMPLEMENTATION

