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

#ifndef PHP_MYTEST_H
#define PHP_MYTEST_H

extern zend_module_entry mytest_module_entry;
#define phpext_mytest_ptr &mytest_module_entry

#define PHP_MYTEST_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_MYTEST_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_MYTEST_API __attribute__ ((visibility("default")))
#else
#	define PHP_MYTEST_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/*定义全局变量结构体*/
ZEND_BEGIN_MODULE_GLOBALS(mytest)
	zend_long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(mytest)


/* 定义一个获取全局变量的宏*/
#define MYTEST_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(mytest, v)

#if defined(ZTS) && defined(COMPILE_DL_MYTEST)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif	/* PHP_MYTEST_H */

// 函数参数的引用传递使用
ZEND_BEGIN_ARG_INFO_EX(arginfo_my_func_3, 0, 0, 1)
    ZEND_ARG_INFO(1, a) //引用
    ZEND_ARG_OBJ_INFO(0, b, Exception, 0) //注意：这里不要把字符串加""
ZEND_END_ARG_INFO()


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
