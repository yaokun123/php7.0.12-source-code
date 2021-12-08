/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2016 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_GLOBALS_H
#define ZEND_GLOBALS_H


#include <setjmp.h>

#include "zend_globals_macros.h"

#include "zend_stack.h"
#include "zend_ptr_stack.h"
#include "zend_hash.h"
#include "zend_llist.h"
#include "zend_objects.h"
#include "zend_objects_API.h"
#include "zend_modules.h"
#include "zend_float.h"
#include "zend_multibyte.h"
#include "zend_multiply.h"
#include "zend_arena.h"

/* Define ZTS if you want a thread-safe Zend */
/*#undef ZTS*/

#ifdef ZTS

BEGIN_EXTERN_C()
ZEND_API extern int compiler_globals_id;
ZEND_API extern int executor_globals_id;
END_EXTERN_C()

#endif

#define SYMTABLE_CACHE_SIZE 32


#include "zend_compile.h"

/* excpt.h on Digital Unix 4.0 defines function_table */
#undef function_table

#define ZEND_EARLY_BINDING_COMPILE_TIME 0
#define ZEND_EARLY_BINDING_DELAYED      1
#define ZEND_EARLY_BINDING_DELAYED_ALL  2

typedef struct _zend_vm_stack *zend_vm_stack;
typedef struct _zend_ini_entry zend_ini_entry;


struct _zend_compiler_globals {
	zend_stack loop_var_stack;

	zend_class_entry *active_class_entry;

	zend_string *compiled_filename;

	int zend_lineno;

	zend_op_array *active_op_array;

	HashTable *function_table;	/* function symbol table */
	HashTable *class_table;		/* class table */

	HashTable filenames_table;

	HashTable *auto_globals;

	zend_bool parse_error;
	zend_bool in_compilation;
	zend_bool short_tags;

	zend_bool unclean_shutdown;

	zend_bool ini_parser_unbuffered_errors;

	zend_llist open_files;

	struct _zend_ini_parser_param *ini_parser_param;

	uint32_t start_lineno;
	zend_bool increment_lineno;

	zend_string *doc_comment;

	uint32_t compiler_options; /* set of ZEND_COMPILE_* constants */

	HashTable const_filenames;

	zend_oparray_context context;
	zend_file_context file_context;

	zend_arena *arena;

	zend_string *empty_string;
	zend_string *one_char_string[256];

	HashTable interned_strings;

	const zend_encoding **script_encoding_list;
	size_t script_encoding_list_size;
	zend_bool multibyte;
	zend_bool detect_unicode;
	zend_bool encoding_declared;

	zend_ast *ast;
	zend_arena *ast_arena;

	zend_stack delayed_oplines_stack;

#ifdef ZTS
	zval **static_members_table;
	int last_static_member;
#endif
};


struct _zend_executor_globals {//全局变量，在main()执行前分配
	zval uninitialized_zval;
	zval error_zval;

	/* symbol table cache */
	zend_array *symtable_cache[SYMTABLE_CACHE_SIZE];
	zend_array **symtable_cache_limit;
	zend_array **symtable_cache_ptr;

	zend_array symbol_table;		//// PHP全局变量hash表 函数外、类外定义的变量（包括$_GET/$_POST等）
                                    //// 销毁(php_request_shutdown->zend_call_destructors->shutdown_destructors)

	HashTable included_files;	    //// 已经include的脚本

	JMP_BUF *bailout;               //try-catch保存的catch跳转位置

	int error_reporting;
	int exit_status;

	HashTable *function_table;	    //全部已编译的function哈希表，包括内部函数、用户自定义函数，函数调用将从这里查找
	HashTable *class_table;		    //全部已编译的class哈希表，new class时从此处查找
	HashTable *zend_constants;	    //// 常量符号表define()函数定义，分为持久化常量和非持久化常量
	                                //// 非持久化常量销毁：php_request_shutdown->zend_deactivate->shutdown_executor->clean_non_persistent_constants
	                                //// 持久化常量销毁：php_module_shutdown

	zval          *vm_stack_top;
	zval          *vm_stack_end;
	zend_vm_stack  vm_stack;        //运行栈内存池：就是一块空白的内存，用于分配php执行期间的一些结构，(zend_execute_data)局部变量就是分配在这上面

	struct _zend_execute_data *current_execute_data;//指向当前正在执行的"运行栈"(zend_execute_data)，函数调用就是分配一个新的zend_execute_data
	                                                //然后将EG(current_execute_data)指向新的结构体继续执行，调用完毕再还原回去。类似汇编call,ret指令
	zend_class_entry *scope;

	zend_long precision;

	int ticks_count;

	HashTable *in_autoload;                         //在类自动加载过程中会用到
	zend_function *autoload_func;                   //自动加载回调函数__autoload()
	zend_bool full_tables_cleanup;

	/* for extended information support */
	zend_bool no_extensions;

#ifdef ZEND_WIN32
	zend_bool timed_out;
	OSVERSIONINFOEX windows_version_info;
#endif

	HashTable regular_list;
	HashTable persistent_list;                      //// 持久化符号表，request请求结束后不释放，可以跨request共享，在php_module_shutdown阶段清理
	                                                //// 网络通信应用中的长链接的实现就可以将连接存到这里维持共享，redis->pconnect,pdo-mysql长链接等

	int user_error_handler_error_reporting;
	zval user_error_handler;
	zval user_exception_handler;
	zend_stack user_error_handlers_error_reporting;
	zend_stack user_error_handlers;
	zend_stack user_exception_handlers;

	zend_error_handling_t  error_handling;
	zend_class_entry      *exception_class;

	/* timeout support */
	zend_long timeout_seconds;

	int lambda_count;

	HashTable *ini_directives;                  //php.ini配置项
	HashTable *modified_ini_directives;
	zend_ini_entry *error_reporting_ini_entry;

	zend_objects_store objects_store;           //对象的存储（object_buckets）
	                                            //将所有的对象保存在EG(objects_store).object_buckets中的目的是用于垃圾回收(不确定是不是还有其它的作用)，
	                                            // 防止出现循环引用而导致内存泄漏的问题
	zend_object *exception, *prev_exception;
	const zend_op *opline_before_exception;
	zend_op exception_op[3];

	struct _zend_module_entry *current_module;

	zend_bool active;
	zend_bool valid_symbol_table;

	zend_long assertions;

	uint32_t           ht_iterators_count;     /* number of allocatd slots */
	uint32_t           ht_iterators_used;      /* number of used slots */
	HashTableIterator *ht_iterators;
	HashTableIterator  ht_iterators_slots[16];

	void *saved_fpu_cw_ptr;
#if XPFPA_HAVE_CW
	XPFPA_CW_DATATYPE saved_fpu_cw;
#endif

	zend_function trampoline;
	zend_op       call_trampoline_op;

	void *reserved[ZEND_MAX_RESERVED_RESOURCES];
};

struct _zend_ini_scanner_globals {
	zend_file_handle *yy_in;
	zend_file_handle *yy_out;

	unsigned int yy_leng;
	unsigned char *yy_start;
	unsigned char *yy_text;
	unsigned char *yy_cursor;
	unsigned char *yy_marker;
	unsigned char *yy_limit;
	int yy_state;
	zend_stack state_stack;

	char *filename;
	int lineno;

	/* Modes are: ZEND_INI_SCANNER_NORMAL, ZEND_INI_SCANNER_RAW, ZEND_INI_SCANNER_TYPED */
	int scanner_mode;
};

typedef enum {
	ON_TOKEN,
	ON_FEEDBACK,
	ON_STOP
} zend_php_scanner_event;

struct _zend_php_scanner_globals {
	zend_file_handle *yy_in;
	zend_file_handle *yy_out;

	unsigned int yy_leng;
	unsigned char *yy_start;
	unsigned char *yy_text;
	unsigned char *yy_cursor;
	unsigned char *yy_marker;
	unsigned char *yy_limit;
	int yy_state;
	zend_stack state_stack;
	zend_ptr_stack heredoc_label_stack;

	/* original (unfiltered) script */
	unsigned char *script_org;
	size_t script_org_size;

	/* filtered script */
	unsigned char *script_filtered;
	size_t script_filtered_size;

	/* input/output filters */
	zend_encoding_filter input_filter;
	zend_encoding_filter output_filter;
	const zend_encoding *script_encoding;

	/* initial string length after scanning to first variable */
	int scanned_string_len;

	/* hooks */
	void (* on_event)(zend_php_scanner_event event, int token, int line);
};

#endif /* ZEND_GLOBALS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
