/* A simple Lisp-like, dynamic programming language implementation.
 *
 * The language has but six types:
 *    - nil
 *          Represents a value with nothing in it; A `nil' value.
 *    - number
 *          Represents a 64 bit floating point number.
 *    - boolean
 *          Represents a true or false value.
 *    - string
 *          Represents a string of text. Strings are *not* null-terminated,
 *          meaning storing their length is required. Because of this, strings
 *          can be used to represent generic binary data, as well as just text.
 *    - function
 *          Represents a user-defined function.
 *    - pointer (C user data)
 *          Represents a pointer to a block of memory managed by the host
 *          application.
 *
 * = PROGRAMMING GUIDE =
 *    = The Basics =
 *      All expressions must be parenthesised. Maths is in Polish notation,
 *      meaning the operator comes first, followed by the arguments, like so:
 *
 *          (+ 10 30)
 *
 *    = Literals =
 *      | Type      | Example(s)                 |
 *      ------------------------------------------
 *      | nil       | nil                        |
 *      | string    | "Hello, world!"            |
 *      | number    | 85      44.3               |
 *      | boolean   | true    false              |
 *      | function  | (fun (x y) ((+ x y)))      |
 *
 *    = Operators =
 *      | Symbol | Params | Description                                     | Return type |
 *      -----------------------------------------------------------------------------------
 *      | +      | 2      | Adds two numbers                                | number      |
 *      | -      | 2      | Subtracts the second number from the first      | number      |
 *      | *      | 2      | Multiplies two numbers                          | number      |
 *      | /      | 2      | Divides the first number by the second          | number      |
 *      | &      | 2      | True if both inputs are true                    | boolean     |
 *      | |      | 2      | True if one of the inputs are true              | boolean     |
 *      | !      | 1      | The reverse of the input                        | boolean     |
 *      | =      | 2      | True if the inputs equal each other             | boolean     |
 *      | <      | 2      | True if the first input is less than the second | boolean     |
 *      | <=     | 2      | True if the first input is less than or equal   | boolean     |
 *      |        |        | to the second                                   |             |
 *      | >      | 2      | True if the first input is greater than the     | boolean     |
 *      |        |        | second                                          |             |
 *      | >=     | 2      | True if the first input is greater than or equal| boolean     |
 *      |        |        | to the second                                   |             |
 *      | #      | 1      | Get the length of an array or string.           | number      |
 *
 *    = Built-in Functions =
 *      | Keyword | Description                             | Example                     |
 *      | print   | Prints a value, followed by a new line  | (print "Hello, world!")     |
 *      | put     | Prints a value                          | (put "Hello, world!")       |
 *      | set     | Sets the value of or declares a variable| (set x 200)                 |
 *      | cat     | Concatenate two strings                 | (cat "Hello, " "world!")    |
 *      | ret     | Halt a function and return a value      | (ret 25)                    |
 *      | array   | Create a new array                      | (array (10 50 2)            |
 *      | at      | Get the value at an index in an array   | (at some_arr 2)             |
 *      | seta    | Set the value in an array, resize.      | (seta some_arr 4 "Hi!")     |
 *      | rm      | Remove a value from an array by index   | (rm some_arr 3)             |
 *
 *    = If Statements = 
 *      If statements are declared like so:
 *          (if true (
 *              (print "True")
 *          ) (
 *              (print "False")
 *          ))
 *      Every if statement must also have an else clause. If an else clause isn't required,
 *      you may simply place `(nil)' in the place of one, to do nothing.
 *
 *    = While Statements =
 *      This language supports a single kind of loop: The while loop. It works how would
 *      expect a while loop to work in C:
 *          (set i 0)
 *          (while (< i 10) (
 *              (print i)
 *              (set i (+ i 1))
 *          ))
 *
 *    = Functions =
 *      Functions can be declared like so:
 *          (set add_numbers (fun (a b) (
 *              (ret (+ a b))
 *          ))
 *       ...And called like this:
 *          (add_numbers 29 3)
 *
 *    = Comments =
 *      Anything starting with a semi-colon (;) is treated as a comment. Comments go until
 *      the end of the line.
 *
 *    = Native functions =
 *      Native functions are a way for the scripts to communicate with the host application.
 *      They take the form of C functions that are registered to the script engine and called
 *      from the script in the same way that script-defined functions would be called.
 *
 *           static struct lsp_val native(struct lsp_state* ctx, u32 argc, struct lsp_val* args) {
 *               printf("Hello, from a native function!\n");
 *               return lsp_make_nil();
 *           }
 *
 *           lsp_register(state, "native", 0, native);
 *
 *    = Pointers =
 *      Pointers are a way for scripts to manage data allocated by the host application. This
 *      is done through an allocator and deallocator function provided by the host.
 *
 *          static void ptr_create(struct lsp_state* ctx, void** ptr) {
 *              *ptr = core_alloc(sizeof(int));
 *              **(int**)ptr = 10;
 *          }
 *
 *          static void ptr_destroy(struct lsp_state* ctx, void** ptr) {
 *              core_free(*ptr);
 *          }
 *
 *          lsp_register_ptr(ctx, "TestPointer", ptr_create, ptr_destroy);
 *
 *      Pointers can be allocated from a script using the `new' keyword, like so:
 *          (set test_ptr (new TestPointer))
 *      This will call the `ptr_create' function. When the garbage collector runs and
 *      decides that the object is ready to be destroyed, the `ptr_destroy' function
 *      will be called.
 *
 *    = Garbage Collection =
 *      The garbage collector runs whenever it is out of space for objects. The default amount of
 *      objects is 1024. The garbage collector can also be run using the `collect_garbage' standard
 *      library function.
 *
 *    = Standard Library =
 *      The function `lsp_register_std' can be called from C to register the standard library to a
 *      script engine. This is an optional step, and the language can operate without the standard
 *      library.
 *
 *      Standard Functions:
 *          | Name           | Description                                    | Return type      |
 *          --------------------------------------------------------------------------------------
 *          | memory_usage   | Get the current application memory usage       | number           |
 *          | stack_count    | Get the current size of the VM's stack         | number           |
 *          | bit_and        | Do a bitwise and operation on two numbers      | number           |
 *          | bit_or         | Do a bitwise or operation on two numbers       | number           |
 *          | shift_left     | Shift the first input left by the second       | number           |
 *          | shift_right    | Shift the first input right by the second      | number           |
 *          | mod            | Returns the remainder from a division          | number           |
 *          | collect_garbage| Run the garbage collector                      | nil              |
 *          | type           | Get the type of a value                        | string           |
 *          | except         | Throw a runtime error                          | nil              |
 */

#pragma once

#include "common.h"

struct lsp_state;
struct lsp_chunk;

enum {
	lsp_val_nil = 0,
	lsp_val_num,
	lsp_val_bool,
	lsp_val_obj
};

enum {
	lsp_obj_str = 0,
	lsp_obj_fun,
	lsp_obj_ptr,
	lsp_obj_arr
};

typedef struct lsp_val (*lsp_nat_fun_t)(struct lsp_state*, u32, struct lsp_val*);

typedef void (*lsp_ptr_create_fun)(struct lsp_state*, void** ptr);
typedef void (*lsp_ptr_destroy_fun)(struct lsp_state*, void** ptr);

struct lsp_obj {
	u8 type;
	u8 mark;
	bool is_const;
	bool recyclable;

	union {
		struct { char* chars; u32 len; }                          str;
		struct { char* name; struct lsp_chunk* chunk; u32 argc; } fun;
		struct { u8 type; void* ptr; }                            ptr;
		struct { u32 count; u32 cap; struct lsp_val* vals; }      arr;
	} as;
};

struct lsp_val {
	u8 type;
	union {
		f64 num;
		bool boolean;
		struct lsp_obj* obj;
	} as;
};

#define lsp_make_nil()           ((struct lsp_val) { .type = lsp_val_nil })
#define lsp_make_num(n_)         ((struct lsp_val) { .type = lsp_val_num,  .as.num = n_ })
#define lsp_make_bool(b_)        ((struct lsp_val) { .type = lsp_val_bool, .as.boolean = b_ })

#define lsp_is_nil(v_)  ((v_).type == lsp_val_nil)
#define lsp_is_obj(v_)  ((v_).type == lsp_val_obj)
#define lsp_is_num(v_)  ((v_).type == lsp_val_num)
#define lsp_is_bool(v_) ((v_).type == lsp_val_bool)
#define lsp_is_str(v_)  ((v_).type == lsp_val_obj && (v_).as.obj->type == lsp_obj_str)
#define lsp_is_arr(v_)  ((v_).type == lsp_val_obj && (v_).as.obj->type == lsp_obj_arr)
#define lsp_is_ptr(v_)  ((v_).type == lsp_val_obj && (v_).as.obj->type == lsp_obj_ptr)
#define lsp_is_fun(v_)  ((v_).type == lsp_val_obj && (v_).as.obj->type == lsp_obj_fun)

#define lsp_as_num(v_)  ((v_).as.num)
#define lsp_as_bool(v_) ((v_).as.boolean)
#define lsp_as_str(v_)  ((v_).as.obj->as.str)
#define lsp_as_arr(v_)  ((v_).as.obj->as.arr)
#define lsp_as_fun(v_)  ((v_).as.obj->as.fun)
#define lsp_as_ptr(v_)  ((v_).as.obj->as.ptr)

#define lsp_arg_assert(ctx_, v_, t_, m_, ...) \
	do { \
		if ((v_).type != t_) { \
			lsp_exception(ctx_, m_, ##__VA_ARGS__); \
			return lsp_make_nil(); \
		} \
	} while (0)

#define lsp_arg_obj_assert(ctx_, v_, t_, m_, ...) \
	do { \
		if ((v_).type != lsp_val_obj || (v_).as.obj->type != t_) { \
			lsp_exception(ctx_, m_, ##__VA_ARGS__); \
			return lsp_make_nil(); \
		} \
	} while (0)

#define lsp_arg_ptr_assert(ctx_, v_, t_, m_, ...) \
	do { \
		if ((v_).type != lsp_val_obj || (v_).as.obj->type != lsp_obj_ptr || (v_).as.obj->as.ptr.type != lsp_get_ptr_type(ctx_, t_)) { \
			lsp_exception(ctx_, m_, ##__VA_ARGS__); \
			return lsp_make_nil(); \
		} \
	} while (0)

API struct lsp_val lsp_make_str(struct lsp_state* ctx, const char* start, u32 len);
API struct lsp_val lsp_make_arr(struct lsp_state* ctx, struct lsp_val* vals, u32 len);
API struct lsp_val lsp_make_fun(struct lsp_state* ctx, const char* name_start, u32 name_len, struct lsp_chunk* chunk, u32 argc);
API struct lsp_val lsp_make_ptr(struct lsp_state* ctx, u8 idx);

API bool lsp_vals_eq(struct lsp_state* ctx, struct lsp_val a, struct lsp_val b);

API struct lsp_state* new_lsp_state(void* error, void* info);
API void free_lsp_state(struct lsp_state* state);

API void lsp_set_simple_errors(struct lsp_state* state, bool simple);
API void lsp_set_warnings(struct lsp_state* state, bool warnings);

API void           lsp_push(struct lsp_state* ctx, struct lsp_val val);
API struct lsp_val lsp_pop(struct lsp_state* ctx);
API struct lsp_val lsp_peek(struct lsp_state* ctx);
API u32 lsp_get_stack_count(struct lsp_state* ctx);

API void lsp_free_obj(struct lsp_state* ctx, struct lsp_obj* obj);

API struct lsp_val lsp_do_string(struct lsp_state* ctx, const char* name, const char* str);
API struct lsp_val lsp_do_file(struct lsp_state* ctx, const char* file_path);

API void lsp_register(struct lsp_state* ctx, const char* name, u32 argc, lsp_nat_fun_t fun);
API void lsp_register_ptr(struct lsp_state* ctx, const char* name, lsp_ptr_create_fun on_create, lsp_ptr_destroy_fun on_destroy);
API u8 lsp_get_ptr_type(struct lsp_state* ctx, const char* name);
API void lsp_register_std(struct lsp_state* ctx);

API void lsp_exception(struct lsp_state* ctx, const char* message, ...);

API void lsp_collect_garbage(struct lsp_state* ctx);
