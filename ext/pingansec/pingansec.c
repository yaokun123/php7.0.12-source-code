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
*/
ZEND_DECLARE_MODULE_GLOBALS(pingansec)

/* True global resources - no need for thread safety here */
static int le_pingansec;

static HashTable *ini_containers;
zend_class_entry *pingansec_ce;

static void php_pingansec_zval_persistent(zval *zv, zval *rv);
static void php_pingansec_zval_dtor(zval *pzval);


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

ZEND_BEGIN_ARG_INFO_EX(php_pingansec_set_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
/* }}} */



static void php_pingansec_hash_destroy(HashTable *ht) /* {{{ */ {
    zend_string *key;
    zval *element;

#if PHP_VERSION_ID < 70400
    if (((ht)->u.flags & HASH_FLAG_INITIALIZED)) {
#else
    if (HT_IS_INITIALIZED(ht)) {
#endif
        ZEND_HASH_FOREACH_STR_KEY_VAL(ht, key, element) {
            if (key) {
                free(key);
            }
            php_pingansec_zval_dtor(element);
        } ZEND_HASH_FOREACH_END();
        free(HT_GET_DATA_ADDR(ht));
    }
    free(ht);
}
/* }}} */


static void php_pingansec_zval_dtor(zval *pzval) /* {{{ */ {
    switch (Z_TYPE_P(pzval)) {
        case IS_ARRAY:
            // php_pingansec_hash_destroy(Z_ARRVAL_P(pzval));
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

static zend_string* php_pingansec_str_persistent(char *str, size_t len) /* {{{ */ {
    zend_string *key = zend_string_init(str, len, 1);
    if (key == NULL) {
        zend_error(E_ERROR, "fail to allocate memory for string, no enough memory?");
    }
    key->h = zend_string_hash_val(key);
#if PHP_VERSION_ID < 70300
    GC_FLAGS(key) |= (IS_STR_INTERNED | IS_STR_PERMANENT);
#else
    GC_ADD_FLAGS(key, IS_STR_INTERNED | IS_STR_PERMANENT);
#endif
    return key;
}
/* }}} */

static void php_pingansec_zval_persistent(zval *zv, zval *rv) /* {{{ */ {
    switch (Z_TYPE_P(zv)) {
#if PHP_VERSION_ID < 70300
        case IS_CONSTANT:
#endif
        case IS_STRING:
            ZVAL_INTERNED_STR(rv, php_pingansec_str_persistent(Z_STRVAL_P(zv), Z_STRLEN_P(zv)));
            break;
        case IS_ARRAY:
        case IS_RESOURCE:
        case IS_OBJECT:
        case _IS_BOOL:
        case IS_LONG:
        case IS_NULL:
            ZEND_ASSERT(0);
            break;
    }
} /* }}} */

static zval* php_pingansec_symtable_update(HashTable *ht, char *k_v, size_t k_l, zval *value) /* {{{ */ {
    zval *element;

    // zend_ulong idx;
    /*if (ZEND_HANDLE_NUMERIC_STR(key, len, idx)) {
        if ((element = zend_hash_index_find(ht, idx))) {
            php_pingansec_zval_dtor(element);
            ZVAL_COPY_VALUE(element, zv);
        } else {
            element = zend_hash_index_add(ht, idx, zv);
        }
    } else {
        if ((element = zend_hash_str_find(ht, key, len))) {
            php_pingansec_zval_dtor(element);
            ZVAL_COPY_VALUE(element, zv);
        } else {
            element = zend_hash_add(ht, php_pingansec_str_persistent(key, len), zv);
        }
    }*/
    //element = zend_hash_add(ht, php_pingansec_str_persistent(k_v, k_l), zv);

    //zval *rv = (zval *)malloc(sizeof(zval));
    //ZVAL_STRINGL(rv, v_v, v_l);

    zval rv;
    php_pingansec_zval_persistent(value, &rv);
    element = zend_hash_update(ht, php_pingansec_str_persistent(k_v, k_l), &rv);

    return element;
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
    return NULL;
}
/* }}} */

PHP_PINGANSEC_API int php_pingansec_has(zend_string *name) /* {{{ */ {
    return php_pingansec_get(name) != NULL;
}
/* }}} */

PHP_PINGANSEC_API int php_pingansec_set(zend_string *name, zval *value) /* {{{ */ {
    if (ini_containers) {
        HashTable *target = ini_containers;
        zval *res = NULL;

        char *k_v,*v_v;
        size_t k_l,v_l;

        k_v = ZSTR_VAL(name);   // (zstr)->val
        k_l = ZSTR_LEN(name);   // (zstr)->len

        //v_v = ZSTR_VAL(value);
        //v_l = ZSTR_LEN(value);

        //res = php_pingansec_symtable_update(target, k_v, k_l, v_v, v_l);
        res = php_pingansec_symtable_update(target, k_v, k_l, value);
        if (res){
            return 1;
        }
    }
    return 0;
}
/* }}} */

/* Remove comments and fill if you need to have entries in php.ini
*/
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("pingansec.directory", "", PHP_INI_SYSTEM, OnUpdateString, directory, zend_pingansec_globals, pingansec_globals)
#ifndef ZTS
    STD_PHP_INI_ENTRY("pingansec.check_delay", "1000", PHP_INI_SYSTEM, OnUpdateLong, check_delay, zend_pingansec_globals, pingansec_globals)
#endif
PHP_INI_END()
/* }}} */


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
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &name) == FAILURE) {
        return;
    }

    val = php_pingansec_get(name);

    if (val) {
        Z_TRY_ADDREF_P(val);
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
        RETURN_FALSE;
    }

    RETURN_BOOL(php_pingansec_has(name));
}
/* }}} */


/** {{{ proto public Pingansec::set(string $name)
*/
PHP_METHOD(pingansec, set) {
    zend_string *name;
    // zend_string *value;
    zval *value;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &name, &value) == FAILURE) {
        RETURN_FALSE;
    }

    RETURN_BOOL(php_pingansec_set(name, value));
}
/* }}} */


/* {{{  pingansec_methods */
zend_function_entry pingansec_methods[] = {
        PHP_ME(pingansec, get, php_pingansec_get_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(pingansec, has, php_pingansec_has_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(pingansec, set, php_pingansec_set_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        {NULL, NULL, NULL}
};
/* }}} */


// module_startup_func
PHP_MINIT_FUNCTION(pingansec)
{
    zend_class_entry ce;
    REGISTER_INI_ENTRIES();
    INIT_CLASS_ENTRY(ce, "Pingansec", pingansec_methods);
    pingansec_ce = zend_register_internal_class_ex(&ce, NULL);


    PALLOC_HASHTABLE(ini_containers);
    zend_hash_init(ini_containers, 8, NULL, NULL, 1);


    const char *dirname;
    size_t dirlen;
    zend_stat_t dir_sb = {0};
    if ((dirname = PINGANSEC_G(directory)) && (dirlen = strlen(dirname))
#ifndef ZTS
			&& !VCWD_STAT(dirname, &dir_sb) && S_ISDIR(dir_sb.st_mode)
#endif
			) {
#ifndef ZTS
        PINGANSEC_G(directory_mtime) = dir_sb.st_mtime;
        PINGANSEC_G(last_check) = time(NULL);
#endif
    }
    return SUCCESS;
}


/* {{{ PHP_MSHUTDOWN_FUNCTION
 *
 * module_shutdown_func
 */
PHP_MSHUTDOWN_FUNCTION(pingansec)
{
    if (ini_containers) {
        php_pingansec_hash_destroy(ini_containers);
    }
    return SUCCESS;
}
/* }}} */



PHP_RINIT_FUNCTION(pingansec)
{
    if(PINGANSEC_G(check_delay) && (time(NULL) - PINGANSEC_G(last_check) < PINGANSEC_G(check_delay))){
        PINGANSEC_DEBUG("check delay doesn't execeed, ignore");
    }else{  // check for clean store
        char *dirname;
        zend_stat_t dir_sb = {0};

        PINGANSEC_G(last_check) = time(NULL);

        // must set directory
        if ((dirname = PINGANSEC_G(directory)) && !VCWD_STAT(dirname, &dir_sb) && S_ISDIR(dir_sb.st_mode)) {
            if (dir_sb.st_mtime == PINGANSEC_G(directory_mtime)) {
                PINGANSEC_DEBUG("directory is not modefied");
                return SUCCESS;
            }else{

                // clean
                PINGANSEC_G(directory_mtime) = dir_sb.st_mtime;
                php_pingansec_hash_destroy(ini_containers);

                // ini_containers = NULL;
                PALLOC_HASHTABLE(ini_containers);
                zend_hash_init(ini_containers, 8, NULL, NULL, 1);
            }
        }
        PINGANSEC_DEBUG("directory need setting in .ini");
    }
    return SUCCESS;
}


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

    DISPLAY_INI_ENTRIES();
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
