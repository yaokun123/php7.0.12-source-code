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
#include "php_pingansec.h"

/* If you declare any globals in php_pingansec.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(pingansec)
*/

/* True global resources - no need for thread safety here */
static int le_pingansec;

static HashTable *ini_containers;
zend_class_entry *pingansec_ce;

// malloc hashtable size without api
#define PALLOC_HASHTABLE(ht) do { \
	(ht) = (HashTable*)pemalloc(sizeof(HashTable), 1); \
} while(0)


/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(php_pingansec_get_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_pingansec_has_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()
/* }}} */


// hash table destory
static void php_pingan_hash_destroy(HashTable *ht) /* {{{ */ {
    zend_string *key;
    zval *element;

#if PHP_VERSION_ID < 70400
    if (((ht)->u.flags & HASH_FLAG_INITIALIZED)) {
#else
    if (HT_IS_INITIALIZED(ht)) {
#endif
        ZEND_HASH_FOREACH_STR_KEY_VAL(ht, key, element) {
            if (key) {
                free(key);  // free key
            }
            php_pingan_zval_dtor(element);  // free value
        } ZEND_HASH_FOREACH_END();
        free(HT_GET_DATA_ADDR(ht));         // free hash
    }
    free(ht);               // destory hashtable variable
} /* }}} */

// zval free
static void php_pingan_zval_dtor(zval *pzval) /* {{{ */ {
    switch (Z_TYPE_P(pzval)) {
        case IS_ARRAY:
            php_pingan_hash_destroy(Z_ARRVAL_P(pzval));
            break;
        case IS_PTR:
        case IS_STRING:
            free(Z_PTR_P(pzval));
            break;
        default:
            break;
    }
}
/* }}} */

PHP_PINGANSEC_API zval *php_pingansec_get(zend_string *name) /* {{{ */ {
    if (ini_containers) {
        zval *pzval;
        char *seg;
        size_t len;
        HashTable *target = ini_containers;


        seg = ZSTR_VAL(name);
        len = ZSTR_LEN(name);
        return zend_symtable_str_find(target, seg, len);
    }
    return NUll;
}
/* }}} */

PHP_PINGANSEC_API int php_pingansec_has(zend_string *name) /* {{{ */ {
    return php_pingansec_get(name) != NULL;
}
/* }}} */

/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("pingansec.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_pingansec_globals, pingansec_globals)
    STD_PHP_INI_ENTRY("pingansec.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_pingansec_globals, pingansec_globals)
PHP_INI_END()
*/


/*PHP_FUNCTION(confirm_pingansec_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "pingansec", arg);

	RETURN_STR(strg);
}*/

/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/



/* Uncomment this function if you have INI entries
static void php_pingansec_init_globals(zend_pingansec_globals *pingansec_globals)
{
	pingansec_globals->global_value = 0;
	pingansec_globals->global_string = NULL;
}
*/

/** {{{ proto public Pingansec::get(string $name)
*/
PHP_METHOD(pingansec, get) {
    zend_string *name;
    zval *val = NULL;

    // S    ->  zend_string
    // z    ->
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &name) == FAILURE) {
        return;
    }

    val = php_pingansec_get(name);
    if (val) {
        RETURN_ZVAL(val, 0, 0);
    }
    RETURN_NULL();
}
/* }}} */

/** {{{ proto public Pingansec::has(string $name)
*/
PHP_METHOD(pingansec, has) {
    zend_string *name;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &name) == FAILURE) {
        return;
    }

    RETURN_BOOL(php_pingansec_has(name));
}
/* }}} */


zend_function_entry pingansec_methods[] = {
        PHP_ME(pingansec, get, php_pingansec_get_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(pingansec, has, php_pingansec_has_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        {NULL, NULL, NULL}
};


// module_startup_func
PHP_MINIT_FUNCTION(pingansec)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Pingansec", pingansec_methods);
    pingansec_ce = zend_register_internal_class_ex(&ce, NULL);


    PALLOC_HASHTABLE(ini_containers);
    zend_hash_init(ini_containers, 8, NULL, NULL, 1);
	return SUCCESS;
}


// module_shutdown_func
PHP_MSHUTDOWN_FUNCTION(pingansec)
{
    if (ini_containers) {
        php_pingan_hash_destroy(ini_containers);
    }
	return SUCCESS;
}


// request_startup_func
PHP_RINIT_FUNCTION(pingansec)
{
#if defined(COMPILE_DL_PINGANSEC) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}

// request_shutdown_func
PHP_RSHUTDOWN_FUNCTION(pingansec)
{
	return SUCCESS;
}


PHP_MINFO_FUNCTION(pingansec)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "pingansec support", "enabled");
    php_info_print_table_row(2, "version", PHP_PINGANSEC_VERSION);
    php_info_print_table_row(2, "author", "yaok");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}


/*const zend_function_entry pingansec_functions[] = {
	PHP_FE(confirm_pingansec_compiled,	NULL)		*//* For testing, remove later. *//*
	PHP_FE_END	*//* Must be the last line in pingansec_functions[] *//*
};*/


// pingansec_module_entry
zend_module_entry pingansec_module_entry = {
	STANDARD_MODULE_HEADER,
	"pingansec",
	NULL,               // unused function, only used method
	PHP_MINIT(pingansec),
	PHP_MSHUTDOWN(pingansec),
	PHP_RINIT(pingansec),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(pingansec),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(pingansec),
	PHP_PINGANSEC_VERSION,
	STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_PINGANSEC
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(pingansec)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
