/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_mytest.h"

/* 声明全局变量:*/
ZEND_DECLARE_MODULE_GLOBALS(mytest)


/* True global resources - no need for thread safety here */
static int le_mytest;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini*/
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("mytest.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_mytest_globals, mytest_globals)
    STD_PHP_INI_ENTRY("mytest.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_mytest_globals, mytest_globals)
PHP_INI_END()

/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_mytest_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_mytest_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "mytest", arg);

	RETURN_STR(strg);
}
//自定义内部函数
PHP_FUNCTION(my_func_1)
{
    printf("Hello, I'm my_func_1\n");
}
PHP_FUNCTION(my_func_2)
{
    printf("Hello, I'm my_func_2\n");
}
//自定义内部函数，带参数解析的
PHP_FUNCTION(my_func_3)
{
    // 扩展中定义的函数直接按照顺序从zend_execute_data上读取对应的值即可，
    // PHP中通过zend_parse_parameters()这个函数解析zend_execute_data上保存的参数
    // zend_parse_parameters(int num_args, const char *type_spec, ...);
    // num_args:为实际传参数，通过ZEND_NUM_ARGS()获取
    // type_spec是一个字符串，用来标识解析参数的类型，比如:"la"表示第一个参数为整形，第二个为数组，将按照这个解析到指定变量；
    // 后面是一个可变参数，用来指定解析到的变量，这个值与type_spec配合使用，即type_spec用来指定解析的变量类型，可变参数用来指定要解析到的变量，这个值必须是指针。

    // 注意：解析时除了整形、浮点型、布尔型是直接硬拷贝value外，其它解析到的变量只能是指针
    // 整形：l、L   ->  解析到的变量类型必须是zend_long，如果在标识符后加"!"，即："l!"、"L!"，则必须再提供一个zend_bool变量的地址
    // 布尔型：b    ->  解析到的变量必须是zend_bool
    // 浮点型：d    ->  解析的变量类型必须为double
    // 字符串：s、S、p、P  ->  解析有两种形式：char*、zend_string，其中"s"将参数解析到char*，且需要额外提供一个size_t类型的变量用于获取字符串长度，"S"将解析到zend_string
    // 数组：a、A、h、H   ->  数组的解析也有两类，一类是解析到zval层面，另一类是解析到HashTable，其中"a"、"A"解析到的变量必须是zval，"h"、"H"解析到HashTable
    // 对象：o、O   ->  如果参数是一个对象则可以通过"o"、"O"将其解析到目标变量，注意：只能解析为zval*，无法解析为zend_object*
    // 资源：r     ->  如果参数为资源则可以通过"r"获取其zval的地址，但是无法直接解析到zend_resource的地址，与对象相同
    // 类：C      ->  如果参数是一个类则可以通过"C"解析出zend_class_entry地址
    // callable：f   ->  callable指函数或成员方法，如果参数是函数名称字符串、array(对象/类,成员方法)，则可以通过"f"标识符解析出zend_fcall_info结构，这个结构是调用函数、成员方法时的唯一输入
    // 任意类型：z   ->  "z"表示按参数实际类型解析，比如参数为字符串就解析为字符串，参数为数组就解析为数组，这种实际就是将zend_execute_data上的参数地址拷贝到目的变量了，没有做任何转化


    // 引用传参情况比较特殊
    // 引用参数通过zend_parse_parameters()解析时只能使用"z"解析，不能再直接解析为zend_value了，否则引用将失效
    zval    *lval; //必须为zval，定义为zend_long也能解析出，但不是引用
    zval    *obj;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), "zo", &lval, &obj) == FAILURE){
        RETURN_FALSE;
    }

    //lval的类型为IS_REFERENCE
    zval *real_val = Z_REFVAL_P(lval); //获取实际引用的zval地址：&(lval.value->ref.val)
    Z_LVAL_P(real_val) = 100; //设置实际引用的类型
}

//自定义函数参数返回值
PHP_FUNCTION(my_fuunc_4)
{
    //调用内部函数时其返回值指针作为参数传入，这个参数为zval *return_value，
    // 如果函数有返回值直接设置此指针即可，需要特别注意的是设置返回值时需要增加其引用计数

    zval    *arr;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), "a", &arr) == FAILURE){
        RETURN_FALSE;
    }

    //增加引用计数
    Z_ADDREF_P(arr);

    //设置返回值为数组：此函数接收一个数组，然后直接返回该数组
    ZVAL_ARR(return_value, Z_ARR_P(arr));
}
// 函数定义完了就需要向PHP注册了，这里并不需要扩展自己注册，PHP提供了一个内部函数注册结构：zend_function_entry，
// 扩展只需要为每个内部函数生成这样一个结构，然后把它们保存到扩展zend_module_entry.functions即可，
// 在加载扩展中会自动向EG(function_table)注册。
// zend_function_entry结构可以通过PHP_FE()或ZEND_FE()定义：
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_mytest_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_mytest_init_globals(zend_mytest_globals *mytest_globals)
{
	mytest_globals->global_value = 0;
	mytest_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(mytest)
{
	/* 将php.ini解析到指定结构体 */
	REGISTER_INI_ENTRIES();

	//测试解析php.ini到全局变量中
    printf("mytest.global_value %d\n", MYTEST_G(global_value));
    printf("mytest.global_string %s\n", MYTEST_G(global_string));

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(mytest)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(mytest)
{
#if defined(COMPILE_DL_MYTEST) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(mytest)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mytest)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "mytest support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ mytest_functions[]
 *
 * Every user visible function must have an entry in mytest_functions[].
 */
const zend_function_entry mytest_functions[] = {
	PHP_FE(confirm_mytest_compiled,	NULL)		/* For testing, remove later. */

	PHP_FE(my_func_1, NULL)
	PHP_FE(my_func_2, NULL)
	PHP_FE(my_func_3, arginfo_my_func_3)
	PHP_FE(my_fuunc_4, NULL)


	PHP_FE_END	/* 末尾必须加这个 */
	//最后将zend_module_entry->functions设置为mytest_functions即可：
};
/* }}} */

/* {{{ mytest_module_entry
 */
zend_module_entry mytest_module_entry = {
	STANDARD_MODULE_HEADER,
	"mytest",
	mytest_functions,       // 注册内部函数
	PHP_MINIT(mytest),
	PHP_MSHUTDOWN(mytest),
	PHP_RINIT(mytest),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(mytest),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(mytest),
	PHP_MYTEST_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MYTEST
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(mytest)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
