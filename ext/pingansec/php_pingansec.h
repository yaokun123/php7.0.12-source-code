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

#ifndef PHP_PINGANSEC_H
#define PHP_PINGANSEC_H

extern zend_module_entry pingansec_module_entry;
#define phpext_pingansec_ptr &pingansec_module_entry


// quick operation global
#ifdef ZTS
#define PINGANSEC_G(v) TSRMG(pingansec_globals_id, zend_pingansec_globals *, v)
#else
#define PINGANSEC_G(v) (pingansec_globals.v)
#endif

#ifdef PHP_WIN32
#define PHP_PINGANSEC_API __declspec(dllexport)
#else
#define PHP_PINGANSEC_API
#endif

#define PHP_PINGANSEC_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_PINGANSEC_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PINGANSEC_API __attribute__ ((visibility("default")))
#else
#	define PHP_PINGANSEC_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif


// Declare any global variables
ZEND_BEGIN_MODULE_GLOBALS(pingansec)
    char *directory;
    int   parse_err;
#ifndef ZTS
    long   check_delay;
    time_t last_check;
    time_t directory_mtime;
#endif
ZEND_END_MODULE_GLOBALS(pingansec)

/* Always refer to the globals in your function as PINGANSEC_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
// #define PINGANSEC_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(pingansec, v)

#if defined(ZTS) && defined(COMPILE_DL_PINGANSEC)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

BEGIN_EXTERN_C()
PHP_PINGANSEC_API zval *php_pingansec_get(zend_string *name);
PHP_PINGANSEC_API int php_pingansec_has(zend_string *name);
END_EXTERN_C()

#endif	/* PHP_PINGANSEC_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
