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
         _                       _____           _
 _ __ (_)_ __   __ _  __ _ _ _|_   _|__   ___ | |___
| '_ \| | '_ \ / _` |/ _` | '_ \| |/ _ \ / _ \| / __|
| |_) | | | | | (_| | (_| | | | | | (_) | (_) | \__ \
| .__/|_|_| |_|\__, |\__,_|_| |_|_|\___/ \___/|_|___/
|_|            |___/
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_pingan_tools.h"

/* If you declare any globals in php_pingan_tools.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(pingan_tools)
*/

/* True global resources - no need for thread safety here */
static int le_pingan_tools;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("pingan_tools.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_pingan_tools_globals, pingan_tools_globals)
    STD_PHP_INI_ENTRY("pingan_tools.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_pingan_tools_globals, pingan_tools_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_pingan_tools_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_pingan_tools_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "pingan_tools", arg);

	RETURN_STR(strg);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_pingan_tools_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_pingan_tools_init_globals(zend_pingan_tools_globals *pingan_tools_globals)
{
	pingan_tools_globals->global_value = 0;
	pingan_tools_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(pingan_tools)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(pingan_tools)
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
PHP_RINIT_FUNCTION(pingan_tools)
{
#if defined(COMPILE_DL_PINGAN_TOOLS) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(pingan_tools)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(pingan_tools)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "pingan_tools support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ pingan_tools_functions[]
 *
 * Every user visible function must have an entry in pingan_tools_functions[].
 */
const zend_function_entry pingan_tools_functions[] = {
	PHP_FE(confirm_pingan_tools_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in pingan_tools_functions[] */
};
/* }}} */

/* {{{ pingan_tools_module_entry
 */
zend_module_entry pingan_tools_module_entry = {
	STANDARD_MODULE_HEADER,
	"pingan_tools",
	pingan_tools_functions,
	PHP_MINIT(pingan_tools),
	PHP_MSHUTDOWN(pingan_tools),
	PHP_RINIT(pingan_tools),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(pingan_tools),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(pingan_tools),
	PHP_PINGAN_TOOLS_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PINGAN_TOOLS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(pingan_tools)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
