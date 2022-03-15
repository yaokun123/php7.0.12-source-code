/*
  +----------------------------------------------------------------------+
  | Yet Another Conf                                                     |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Xinchen Hui  <laruence@php.net>                              |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "main/php_scandir.h"
#include "ext/standard/info.h"
#include "php_yaconf.h"

/*
 * zend_parse_parameters参数解析，解析时除了整形、浮点型、布尔型是直接硬拷贝value外，其它解析到的变量只能是指针
 *
 * 1、整形：l、L
 * zend_long   lval;
 * zend_bool   is_null;
 * zend_parse_parameters(ZEND_NUM_ARGS(), "l", &lval)
 * 如果在标识符后加"!"，即："l!"、"L!"，则必须再提供一个zend_bool变量的地址
 * zend_parse_parameters(ZEND_NUM_ARGS(), "l!", &lval, &is_null)
 * l"与"L"的区别在于，当传参不是整形且转为整形后超过了整形的大小范围时，"L"将值调整为整形的最大或最小值，而"l"将报错
 *
 * 2、布尔：b
 * zend_bool   ok;
 * zend_parse_parameters(ZEND_NUM_ARGS(), "b", &ok, &is_null)
 * "b!"的用法与整形的完全相同，也必须再提供一个zend_bool的地址用于获取传参是否为NULL，如果为NULL，则zend_bool为0，用于获取是否NULL的zend_bool为1。
 *
 * 3、浮点型：d
 * double  dval;
 * zend_parse_parameters(ZEND_NUM_ARGS(), "d", &dval)
 * "d!"与整形、布尔型用法完全相同。
 *
 *
 * 1、字符串：s、S、p、P
 * 字符串解析有两种形式：char*、zend_string，其中"s"将参数解析到char*，且需要额外提供一个size_t类型的变量用于获取字符串长度，"S"将解析到zend_string：
 * char    *str;
 * size_t  str_len;
 * zend_parse_parameters(ZEND_NUM_ARGS(), "s", &str, &str_len)
 * 或
 * zend_string    *str;
 * zend_parse_parameters(ZEND_NUM_ARGS(), "S", &str)
 * "s!"、"S!"与整形、布尔型用法不同，字符串时不需要额外提供zend_bool的地址，如果参数为NULL，则char*、zend_string将设置为NULL。
 * 除了"s"、"S"之外还有两个类似的："p"、"P"，从解析规则来看主要用于解析路径，实际与普通字符串没什么区别
 *
 * 2、数组：a、A、h、H
 * 数组的解析也有两类，一类是解析到zval层面，另一类是解析到HashTable，其中"a"、"A"解析到的变量必须是zval，"h"、"H"解析到HashTable，这两类是等价的：
 * zval        *arr;   //必须是zval指针，不能是zval arr，因为参数保存在zend_execute_data上，arr为此空间上参数的地址
 * HashTable   *ht;
 * zend_parse_parameters(ZEND_NUM_ARGS(), "ah", &arr, &ht)
 * "a!"、"A!"、"h!"、"H!"的用法与字符串一致，也不需要额外提供别的地址，如果传参为NULL，则对应解析到的zval*、HashTable*也为NULL。
 *
 * 3、对象：o、O
 * 注意：只能解析为zval*，无法解析为zend_object*
 * zval    *obj;
 * zend_parse_parameters(ZEND_NUM_ARGS(), "o", &obj)
 * "o!"、"O!"与字符串用法相同。
 *
 * 4、资源：r
 * 如果参数为资源则可以通过"r"获取其zval的地址，但是无法直接解析到zend_resource的地址，与对象相同。
 * zval    *res;
 * zend_parse_parameters(ZEND_NUM_ARGS(), "r", &res)
 * "r!"与字符串用法相同。
 *
 * 5、类：C
 * 如果参数是一个类则可以通过"C"解析出zend_class_entry地址
 * zend_class_entry    *ce = NULL; //初始为NULL
 * zend_parse_parameters(ZEND_NUM_ARGS(), "C", &ce)
 *
 * 6、callable：f
 * callable指函数或成员方法，如果参数是函数名称字符串、array(对象/类,成员方法)，则可以通过"f"标识符解析出zend_fcall_info结构
 * zend_fcall_info         callable; //注意，这两个结构不能是指针
 * zend_fcall_info_cache   call_cache;
 * zend_parse_parameters(ZEND_NUM_ARGS(), "f", &callable, &call_cache)
 *
 * 7、任意类型：z
 * "z!"与字符串用法相同。
 *
 * notice:
 * |： 表示此后的参数为可选参数，可以不传，比如解析规则为："al|b"，则可以传2个或3个参数
 * +/*： 用于可变参数，注意这里与PHP函数...的用法不太一样，PHP中可以把函数最后一个参数前加...，表示调用时可以传多个参数，
 * 些参数都会插入...参数的数组中，"/+"也表示这个参数是可变的，但内核中只能接收一个值，即使传了多个后面那些也解析不到，""、"+"的区别在于"*"表示可以不传可变参数，而"+"表示可变参数至少有一个。
 * /




//// 定义全局变量.h
//// #define ZEND_DECLARE_MODULE_GLOBALS(module_name) zend_##module_name##_globals module_name##_globals;
// zend_yaconf_globals yaconf_globals
ZEND_DECLARE_MODULE_GLOBALS(yaconf);

//// 获取全局变量.h 该扩展没有使用SAPI定义的接口，而是自己实现的
//// #define MYTEST_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(mytest, v)
//// #define ZEND_MODULE_GLOBALS_ACCESSOR(module_name, v) (module_name##_globals.v)
//// #define MYTEST_G(v) yaconf_globals.v

static HashTable *ini_containers;
static HashTable *parsed_ini_files;
static zval active_ini_file_section;

zend_class_entry *yaconf_ce;

static void php_yaconf_zval_persistent(zval *zv, zval *rv);
static void php_yaconf_zval_dtor(zval *pzval);

typedef struct _yaconf_filenode {
	zend_string *filename;
	time_t mtime;
} yaconf_filenode;

#define PALLOC_HASHTABLE(ht) do { \
	(ht) = (HashTable*)pemalloc(sizeof(HashTable), 1); \
} while(0)

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(php_yaconf_get_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, default)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_yaconf_has_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ yaconf_module_entry:每一个扩展都需要定义一个此结构的变量，而且这个变量的名称格式必须是：{mudule_name}_module_entry
 *
 * 扩展可以在编译PHP时一起编译(静态编译)，也可以单独编译为动态库，动态库需要加入到php.ini配置中去，然后在
 * php_module_startup()阶段把这些动态库加载到PHP中
 */
zend_module_entry yaconf_module_entry = {
	STANDARD_MODULE_HEADER,
	"yaconf",
	NULL,
	PHP_MINIT(yaconf),
	PHP_MSHUTDOWN(yaconf),
#ifndef ZTS
	PHP_RINIT(yaconf),
#else
	NULL,
#endif
	NULL,
	PHP_MINFO(yaconf),
	PHP_YACONF_VERSION,
	PHP_MODULE_GLOBALS(yaconf),
	PHP_GINIT(yaconf),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */


//定义了一个get_module()函数，返回扩展zend_module_entry结构的地址
//这就是为什么这个结构的变量名必须是 扩展名称_module_entry 这种格式的原因。
#ifdef COMPILE_DL_YACONF
ZEND_GET_MODULE(yaconf)
#endif

static void php_yaconf_hash_init(zval *zv, size_t size) /* {{{ */ {
	HashTable *ht;
	PALLOC_HASHTABLE(ht);
	/* ZVAL_PTR_DTOR is necessary in case that this array be cloned */
	zend_hash_init(ht, size, NULL, ZVAL_PTR_DTOR, 1);
#if PHP_VERSION_ID < 70300
	GC_FLAGS(ht) |= (IS_ARRAY_IMMUTABLE | HASH_FLAG_STATIC_KEYS);
#else
	HT_FLAGS(ht) |= (IS_ARRAY_IMMUTABLE | HASH_FLAG_STATIC_KEYS);
#endif
#if PHP_VERSION_ID >= 70400
	zend_hash_real_init(ht, 0);
#endif
#if PHP_VERSION_ID >= 70200
	HT_ALLOW_COW_VIOLATION(ht);
#endif
#if PHP_VERSION_ID < 70300
	GC_FLAGS(ht) &= ~HASH_FLAG_APPLY_PROTECTION;
#endif

#if PHP_VERSION_ID < 70300
	GC_REFCOUNT(ht) = 2;
#else
	GC_SET_REFCOUNT(ht, 2);
#endif

	ZVAL_ARR(zv, ht);
#if PHP_VERSION_ID < 70200
	Z_TYPE_FLAGS_P(zv) = IS_TYPE_IMMUTABLE;
#elif PHP_VERSION_ID < 70300
	Z_TYPE_FLAGS_P(zv) = IS_TYPE_COPYABLE;
#else
	Z_TYPE_FLAGS_P(zv) = 0;
#endif
} 
/* }}} */

static void php_yaconf_hash_destroy(HashTable *ht) /* {{{ */ {
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
			php_yaconf_zval_dtor(element);
		} ZEND_HASH_FOREACH_END();
		free(HT_GET_DATA_ADDR(ht));
	}
	free(ht);
} /* }}} */

static void php_yaconf_zval_dtor(zval *pzval) /* {{{ */ {
	switch (Z_TYPE_P(pzval)) {
		case IS_ARRAY:
			php_yaconf_hash_destroy(Z_ARRVAL_P(pzval));
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

static zend_string* php_yaconf_str_persistent(char *str, size_t len) /* {{{ */ {
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

static zval* php_yaconf_symtable_update(HashTable *ht, char *key, size_t len, zval *zv) /* {{{ */ {
	zend_ulong idx;
	zval *element;
	
	if (ZEND_HANDLE_NUMERIC_STR(key, len, idx)) {
		if ((element = zend_hash_index_find(ht, idx))) {
			php_yaconf_zval_dtor(element);
			ZVAL_COPY_VALUE(element, zv);
		} else {
			element = zend_hash_index_add(ht, idx, zv);
		}
	} else {
		if ((element = zend_hash_str_find(ht, key, len))) {
			php_yaconf_zval_dtor(element);
			ZVAL_COPY_VALUE(element, zv);
		} else {
			element = zend_hash_add(ht, php_yaconf_str_persistent(key, len), zv);
		}
	}

	return element;
}
/* }}} */
	
static void php_yaconf_hash_copy(HashTable *target, HashTable *source) /* {{{ */ {
	zend_string *key;
	zend_long idx;
	zval *element, rv;

	ZEND_HASH_FOREACH_KEY_VAL(source, idx, key, element) {
		php_yaconf_zval_persistent(element, &rv);
		if (key) {
			zend_hash_update(target, php_yaconf_str_persistent(ZSTR_VAL(key), ZSTR_LEN(key)), &rv);
		} else {
			zend_hash_index_update(target, idx, &rv);
		}
	} ZEND_HASH_FOREACH_END();
} /* }}} */

static void php_yaconf_zval_persistent(zval *zv, zval *rv) /* {{{ */ {
	switch (Z_TYPE_P(zv)) {
#if PHP_VERSION_ID < 70300
		case IS_CONSTANT:
#endif
		case IS_STRING:
			ZVAL_INTERNED_STR(rv, php_yaconf_str_persistent(Z_STRVAL_P(zv), Z_STRLEN_P(zv)));
			break;
		case IS_ARRAY:
			{
				php_yaconf_hash_init(rv, zend_hash_num_elements(Z_ARRVAL_P(zv)));
				php_yaconf_hash_copy(Z_ARRVAL_P(rv), Z_ARRVAL_P(zv));
			}
			break;
		case IS_RESOURCE:
		case IS_OBJECT:
		case _IS_BOOL:
		case IS_LONG:
		case IS_NULL:
			ZEND_ASSERT(0);
			break;
	}
} /* }}} */

static inline void php_yaconf_trim_key(char **key, size_t *len) /* {{{ */ {
	/* handle foo : bar :: test */
	while (*len && (**key == ' ' || **key == ':')) {
		(*key)++;
		(*len)--;
	}

	while ((*len) && *((*key) + (*len) - 1) == ' ') {
		(*len)--;
	}
}
/* }}} */

static zval* php_yaconf_parse_nesting_key(HashTable *target, char **key, size_t *key_len, char *delim) /* {{{ */ {
	zval *pzval;
	char *seg = *key;
	size_t len = *key_len;
	int nesting = 0;

	do {
		if (++nesting > 64) {
			YACONF_G(parse_err) = 1;
			php_error(E_WARNING, "Nesting too deep? key name contains more than 64 '.'");
			return NULL;
		}
		if (!(pzval = zend_symtable_str_find(target, seg, delim - seg))) {
			zval rv;
			ZVAL_UNDEF(&rv);
			pzval = php_yaconf_symtable_update(target, seg, delim - seg, &rv);
		}

		len -= (delim - seg) + 1;
		seg = delim + 1;
		if ((delim = memchr(seg, '.', len))) {
			if (Z_TYPE_P(pzval) != IS_ARRAY) {
				php_yaconf_zval_dtor(pzval);
				php_yaconf_hash_init(pzval, 8);
			}
		} else {
			*key = seg;
			*key_len = len;
			return pzval;
		}
		target = Z_ARRVAL_P(pzval);
	} while (1);
}
/* }}} */

static void php_yaconf_simple_parser_cb(zval *key, zval *value, zval *index, int callback_type, void *arg) /* {{{ */ {
	zval *pzval, rv;
	HashTable *target = Z_ARRVAL_P((zval *)arg);
	
	if (callback_type == ZEND_INI_PARSER_ENTRY) {
		char *delim;

		if (UNEXPECTED(delim = memchr(Z_STRVAL_P(key), '.', Z_STRLEN_P(key)))) {
			char *seg = Z_STRVAL_P(key);
			size_t len = Z_STRLEN_P(key);

			pzval = php_yaconf_parse_nesting_key(target, &seg, &len, delim);
			if (pzval == NULL) {
				return;
			}

			if (Z_TYPE_P(pzval) != IS_ARRAY) {
				php_yaconf_hash_init(pzval, 8);
			}

			php_yaconf_zval_persistent(value, &rv);
			php_yaconf_symtable_update(Z_ARRVAL_P(pzval), seg, len, &rv);
		} else {
			php_yaconf_zval_persistent(value, &rv);
			if ((pzval = zend_symtable_find(target, Z_STR_P(key)))) {
				php_yaconf_zval_dtor(pzval);
				ZVAL_COPY_VALUE(pzval, &rv);
			} else {
				php_yaconf_symtable_update(target, Z_STRVAL_P(key), Z_STRLEN_P(key), &rv);
			}
		}
	} else if (callback_type == ZEND_INI_PARSER_POP_ENTRY) {
		zend_ulong idx;
		
		if (ZEND_HANDLE_NUMERIC(Z_STR_P(key), idx)) {
			if ((pzval = zend_hash_index_find(target, idx)) == NULL) {
				php_yaconf_hash_init(&rv, 8);
				pzval = zend_hash_index_update(target, idx, &rv);
			} else if (Z_TYPE_P(pzval) != IS_ARRAY) {
				php_yaconf_zval_dtor(pzval);
				php_yaconf_hash_init(pzval, 8);
			}
		} else {
			char *delim;

			if (UNEXPECTED(delim = memchr(Z_STRVAL_P(key), '.', Z_STRLEN_P(key)))) {
				zval *parent;
				char *seg = Z_STRVAL_P(key);
				size_t len = Z_STRLEN_P(key);

				parent = php_yaconf_parse_nesting_key(target, &seg, &len, delim);
				if (parent == NULL) {
					return;
				}

				if (Z_TYPE_P(parent) != IS_ARRAY) {
					php_yaconf_hash_init(parent, 8);
					php_yaconf_hash_init(&rv, 8);
					pzval = php_yaconf_symtable_update(Z_ARRVAL_P(parent), seg, len, &rv);
				} else {
					if ((pzval = zend_symtable_str_find(Z_ARRVAL_P(parent), seg, len))) {
						if (Z_TYPE_P(pzval) != IS_ARRAY) {
							php_yaconf_hash_init(&rv, 8);
							pzval = php_yaconf_symtable_update(Z_ARRVAL_P(parent), seg, len, &rv);
						}
					} else {
						php_yaconf_hash_init(&rv, 8);
						pzval = php_yaconf_symtable_update(Z_ARRVAL_P(parent), seg, len, &rv);
					}
				}
			} else {
				if ((pzval = zend_symtable_str_find(target, Z_STRVAL_P(key), Z_STRLEN_P(key)))) {
					if (Z_TYPE_P(pzval) != IS_ARRAY) {
						php_yaconf_zval_dtor(pzval);
						php_yaconf_hash_init(pzval, 8);
					}	
				} else {
					php_yaconf_hash_init(&rv, 8);
					pzval = php_yaconf_symtable_update(target, Z_STRVAL_P(key), Z_STRLEN_P(key), &rv);
				}
			}
		}

		ZEND_ASSERT(Z_TYPE_P(pzval) == IS_ARRAY);
		php_yaconf_zval_persistent(value, &rv);
		if (index && Z_STRLEN_P(index)) {
			php_yaconf_symtable_update(Z_ARRVAL_P(pzval), Z_STRVAL_P(index), Z_STRLEN_P(index), &rv);
		} else {
			zend_hash_next_index_insert(Z_ARRVAL_P(pzval), &rv);
		}
	} else if (callback_type == ZEND_INI_PARSER_SECTION) {
	}
}
/* }}} */

static void php_yaconf_ini_parser_cb(zval *key, zval *value, zval *index, int callback_type, void *arg) /* {{{ */ {
	zval *target = (zval *)arg;

	if (YACONF_G(parse_err)) {
		return;
	}

	if (callback_type == ZEND_INI_PARSER_SECTION) {
		zval *parent;
		char *section, *delim;
		size_t sec_len;
		int nesting = 0;

		php_yaconf_hash_init(&active_ini_file_section, 128);

		section = Z_STRVAL_P(key);
		sec_len = Z_STRLEN_P(key);

		while ((delim = (char *)zend_memrchr(section, ':', sec_len))) {
			section = delim + 1;
			sec_len = sec_len - (delim - Z_STRVAL_P(key) + 1);

			if (++nesting > 16) {
				php_error(E_WARNING, "Nesting too deep? Only less than 16 level inheritance is allowed");
				YACONF_G(parse_err) = 1;
				return;
			}

			php_yaconf_trim_key(&section, &sec_len);
			if ((parent = zend_hash_str_find(Z_ARRVAL_P(target), section, sec_len))) {
				if (Z_TYPE_P(parent) == IS_ARRAY) {
					php_yaconf_hash_copy(Z_ARRVAL(active_ini_file_section), Z_ARRVAL_P(parent));
				} else {
					/* May copy the single value into current section? */
				}
			}
			section = Z_STRVAL_P(key);
			sec_len = delim - section;
		} 
		if (sec_len == 0) {
			php_yaconf_hash_destroy(Z_ARRVAL(active_ini_file_section));
			ZVAL_UNDEF(&active_ini_file_section);
			return;
		}
		php_yaconf_trim_key(&section, &sec_len);
		php_yaconf_symtable_update(Z_ARRVAL_P(target), section, sec_len, &active_ini_file_section);
	} else if (value) {
		if (!Z_ISUNDEF(active_ini_file_section)) {
			target = &active_ini_file_section;
		}
		php_yaconf_simple_parser_cb(key, value, index, callback_type, target);
	}
}
/* }}} */

PHP_YACONF_API zval *php_yaconf_get(zend_string *name) /* {{{ */ {
	if (ini_containers) {
		zval *pzval;
		char *seg, *delim;
		size_t len;
		HashTable *target = ini_containers;

		if (UNEXPECTED(delim = memchr(ZSTR_VAL(name), '.', ZSTR_LEN(name)))) {
			seg = ZSTR_VAL(name);
			len = ZSTR_LEN(name);
			do {
				if (!(pzval = zend_symtable_str_find(target, seg, delim - seg)) || Z_TYPE_P(pzval) != IS_ARRAY) {
					return pzval;
				}
				target = Z_ARRVAL_P(pzval);
				len -= (delim - seg) + 1;
				seg = delim + 1;
				if (!(delim = memchr(seg, '.', len))) {
					return zend_symtable_str_find(target, seg, len);
				}
			} while (1);
		} else {
			return zend_symtable_find(target, name);
		}
	}
	return NULL;
}
/* }}} */

PHP_YACONF_API int php_yaconf_has(zend_string *name) /* {{{ */ {
	return php_yaconf_get(name) != NULL;
}
/* }}} */

//// PHP_FUNCTION(function_name)
//// 内部函数注册
/*
 * #define PHP_FUNCTION			ZEND_FUNCTION
 * #define ZEND_FUNCTION(name)				ZEND_NAMED_FUNCTION(ZEND_FN(name))
 * #define ZEND_NAMED_FUNCTION(name)		void name(INTERNAL_FUNCTION_PARAMETERS)
 * #define ZEND_FN(name) zif_##name
 * #define INTERNAL_FUNCTION_PARAMETERS zend_execute_data *execute_data, zval *return_value
 *
 * void zif_function_name(zend_execute_data *execute_data, zval *return_value)
 */


/** {{{ proto public Yaconf::get(string $name, $default = NULL)
*/
PHP_METHOD(yaconf, get) {
	zend_string *name;
	zval *val, *defv = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|z", &name, &defv) == FAILURE) {
		return;
	} 

	val = php_yaconf_get(name);
	if (val) {
		RETURN_ZVAL(val, 0, 0);
	} else if (defv) {
		RETURN_ZVAL(defv, 1, 0);
	}

	RETURN_NULL();
}
/* }}} */

/** {{{ proto public Yaconf::has(string $name)
*/
PHP_METHOD(yaconf, has) {
	zend_string *name;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &name) == FAILURE) {
		return;
	} 

	RETURN_BOOL(php_yaconf_has(name));
}
/* }}} */

/** {{{ proto public Yaconf::__debug_info(string $name)
 */
PHP_METHOD(yaconf, __debug_info) {
	zend_string *name;
	zval *val;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &name) == FAILURE) {
		return;
	} 

	val = php_yaconf_get(name);
	if (val) {
		zval zv;
		char *address;
		size_t len;
		array_init(return_value);
		ZVAL_STR(&zv, name);
		zend_hash_str_add_new(Z_ARRVAL_P(return_value), "key", sizeof("key") - 1, &zv);
		Z_TRY_ADDREF(zv);
		len = spprintf(&address, 0, "%p", val); /* can not use zend_strpprintf as it only exported after PHP-7.2 */
		ZVAL_STR(&zv, zend_string_init(address, len, 0)); 
		efree(address);
		zend_hash_str_add_new(Z_ARRVAL_P(return_value), "address", sizeof("address") - 1, &zv);
		zend_hash_str_add_new(Z_ARRVAL_P(return_value), "val", sizeof("val") - 1, val);
		Z_TRY_ADDREF_P(val);
		return;
	}

	RETURN_NULL();
}
/* }}} */

/* {{{  yaconf_methods */
zend_function_entry yaconf_methods[] = {
	PHP_ME(yaconf, get, php_yaconf_get_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(yaconf, has, php_yaconf_has_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(yaconf, __debug_info, php_yaconf_has_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_INI
 *
 * #define PHP_INI_BEGIN		ZEND_INI_BEGIN
 * #define PHP_INI_END			ZEND_INI_END
 *
 * #define ZEND_INI_BEGIN()		static const zend_ini_entry_def ini_entries[] = {
 * #define ZEND_INI_END()		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0} };
 *
 * static const zend_ini_entry_def ini_entries[] = {
 *          { NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0}
 * }
 *
 * #define STD_PHP_INI_ENTRY		STD_ZEND_INI_ENTRY
 * #define STD_ZEND_INI_ENTRY_EX(name, default_value, modifiable, on_modify, property_name, struct_type, struct_ptr, displayer)
 * name: php.ini中的配置标识符
 * default_value: 默认值，注意不管转化后是什么类型，这里必须设置为字符串
 * modifiable: 可修改等级，ZEND_INI_USER为可以在php脚本中修改，ZEND_INI_SYSTEM为可以在php.ini中修改，还有一个ZEND_INI_PERDIR，ZEND_INI_ALL表示三种都可以，通常情况下设置为ZEND_INI_ALL、ZEND_INI_SYSTEM即可
 * on_modify: 函数指针，用于指定发现这个配置后赋值处理的函数，默认提供了5个：OnUpdateBool、OnUpdateLong、OnUpdateLongGEZero、OnUpdateReal、OnUpdateString、OnUpdateStringUnempty，支持可以自定义
 * property_name: 要映射到的结构struct_type中的成员
 * struct_type: 映射结构的类型
 * struct_ptr: 映射结构的变量地址，发现配置后会
 *
 * 除了STD_PHP_INI_ENTRY()这个宏还有一个类似的宏STD_PHP_INI_BOOLEAN()，用法一致，差别在于后者会自动把配置添加到phpinfo()输出中
 */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("yaconf.directory", "", PHP_INI_SYSTEM, OnUpdateString, directory, zend_yaconf_globals, yaconf_globals)
#ifndef ZTS
	STD_PHP_INI_ENTRY("yaconf.check_delay", "300", PHP_INI_SYSTEM, OnUpdateLong, check_delay, zend_yaconf_globals, yaconf_globals)
#endif
PHP_INI_END()
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
 *
 * #define PHP_GINIT_FUNCTION		ZEND_GINIT_FUNCTION
 * #define ZEND_GINIT_FUNCTION			ZEND_MODULE_GLOBALS_CTOR_D
 * #define ZEND_MODULE_GLOBALS_CTOR_D(module)  void ZEND_MODULE_GLOBALS_CTOR_N(module)(zend_##module##_globals *module##_globals)
 * #define ZEND_MODULE_GLOBALS_CTOR_N(module)  zm_globals_ctor_##module
 *
 * zm_globals_ctor_yaconf(zend_yaconf_globals *yaconf_globals)
 *
 *
 * PHP_GINIT
 * #define PHP_GINIT		ZEND_GINIT
 * #define ZEND_GINIT(module)		((void (*)(void*))(ZEND_MODULE_GLOBALS_CTOR_N(module)))
 * #define ZEND_MODULE_GLOBALS_CTOR_N(module)  zm_globals_ctor_##module
 * zm_globals_ctor_yaconf
*/
PHP_GINIT_FUNCTION(yaconf)
{
	yaconf_globals->directory = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 * #define PHP_MINIT_FUNCTION		ZEND_MODULE_STARTUP_D
 * #define ZEND_MODULE_STARTUP_D(module)		int ZEND_MODULE_STARTUP_N(module)(INIT_FUNC_ARGS)
 * #define ZEND_MODULE_STARTUP_N(module)       zm_startup_##module
 * #define INIT_FUNC_ARGS		int type, int module_number
 *
 * zm_startup_yaconf(int type, int module_number)
 *
 *
 * PHP_MINIT
 * #define PHP_MINIT		ZEND_MODULE_STARTUP_N
 * #define ZEND_MODULE_STARTUP_N(module)       zm_startup_##module
 *
 * zm_startup_yaconf
 */
PHP_MINIT_FUNCTION(yaconf)
{
	const char *dirname;
	size_t dirlen;
	zend_class_entry ce;
	zend_stat_t dir_sb = {0};

	REGISTER_INI_ENTRIES();         // 将php.ini解析到指定结构体

	INIT_CLASS_ENTRY(ce, "Yaconf", yaconf_methods);     // 类方法

	yaconf_ce = zend_register_internal_class_ex(&ce, NULL); // 内部类注册

	if ((dirname = YACONF_G(directory)) && (dirlen = strlen(dirname)) 
#ifndef ZTS
			&& !VCWD_STAT(dirname, &dir_sb) && S_ISDIR(dir_sb.st_mode)
#endif
			) {// 设置了配置文件的目录之后才会执行
		zval result;
		int ndir;
		struct dirent **namelist;
		char *p, ini_file[MAXPATHLEN];//ini_file长度最多1024个字节

#ifndef ZTS
		YACONF_G(directory_mtime) = dir_sb.st_mtime;//目录的最后一次修改时间
#endif

		if ((ndir = php_scandir(dirname, &namelist, 0, php_alphasort)) > 0) {//返回目录下包含的文件和目录总个数（包含.和..）
			uint32_t i;
			zend_stat_t sb;
			zend_file_handle fh = {{0}, 0};

			PALLOC_HASHTABLE(ini_containers);//hashTable初始化（持久化）
			zend_hash_init(ini_containers, ndir, NULL, NULL, 1);

			PALLOC_HASHTABLE(parsed_ini_files);//hashTable初始化（持久化）
			zend_hash_init(parsed_ini_files, ndir, NULL, NULL, 1);

			for (i = 0; i < ndir; i++) {//遍历目录下包含的文件和目录
				if (!(p = strrchr(namelist[i]->d_name, '.')) || strcmp(p, ".ini")) {
					free(namelist[i]);
					continue;   // strrchr返回目标字符串最后出现的位置-之后的字符串
					            // 这就过滤掉所有不以.ini结尾的文件和目录
				}

				snprintf(ini_file, MAXPATHLEN, "%s%c%s", dirname, DEFAULT_SLASH, namelist[i]->d_name);//ini_file文件为全路径的.ini文件

				if (VCWD_STAT(ini_file, &sb) == 0) {//获取ini_file的状态
					if (S_ISREG(sb.st_mode)) {         //regular file=普通文件
						yaconf_filenode node;
						if ((fh.handle.fp = VCWD_FOPEN(ini_file, "r"))) {//打开ini文件
							fh.filename = ini_file;
							fh.type = ZEND_HANDLE_FP;
				            ZVAL_UNDEF(&active_ini_file_section);   //表示zval被销毁
							YACONF_G(parse_err) = 0;
							php_yaconf_hash_init(&result, 128);     //result变量赋值为一个hashTable，大小128
							if (zend_parse_ini_file(&fh, 1, 0 /* 解析init文件到result中 */,
									php_yaconf_ini_parser_cb, (void *)&result) == FAILURE || YACONF_G(parse_err)) {
								YACONF_G(parse_err) = 0;
								php_yaconf_hash_destroy(Z_ARRVAL(result));
								free(namelist[i]);
								continue;
							}
						}
						
						php_yaconf_symtable_update(ini_containers, namelist[i]->d_name, p - namelist[i]->d_name, &result);

						node.filename = zend_string_init(namelist[i]->d_name, strlen(namelist[i]->d_name), 1);
						node.mtime = sb.st_mtime;
						zend_hash_update_mem(parsed_ini_files, node.filename, &node, sizeof(yaconf_filenode));
					}
				} else {
					php_error(E_ERROR, "Could not stat '%s'", ini_file);
				}
				free(namelist[i]);
			}
#ifndef ZTS
			YACONF_G(last_check) = time(NULL);
#endif
			free(namelist);
		} else {
			php_error(E_ERROR, "Couldn't opendir '%s'", dirname);
		}
	}

	return SUCCESS;
}
/* }}} */

#ifndef ZTS
/* {{{ PHP_RINIT_FUNCTION(yaconf)
 * #define PHP_RINIT_FUNCTION		ZEND_MODULE_ACTIVATE_D
 * #define ZEND_MODULE_ACTIVATE_D(module)		int ZEND_MODULE_ACTIVATE_N(module)(INIT_FUNC_ARGS)
 * #define ZEND_MODULE_ACTIVATE_N(module)		zm_activate_##module
 * #define INIT_FUNC_ARGS		int type, int module_number
 *
 * zm_activate_yaconf(int type, int module_number)
 *
 *
 * PHP_RINIT
 * #define PHP_RINIT		ZEND_MODULE_ACTIVATE_N
 * #define ZEND_MODULE_ACTIVATE_N(module)		zm_activate_##module
 * zm_activate_yaconf
*/
PHP_RINIT_FUNCTION(yaconf)
{
	if (YACONF_G(check_delay) && (time(NULL) - YACONF_G(last_check) < YACONF_G(check_delay))) {
		YACONF_DEBUG("config check delay doesn't execeed, ignore");
		return SUCCESS;
	} else {//每次请求的时候都检查文件是否更新
		char *dirname;
		zend_stat_t dir_sb = {0};

		YACONF_G(last_check) = time(NULL);//更新一下最后检查时间

		if ((dirname = YACONF_G(directory)) && !VCWD_STAT(dirname, &dir_sb) && S_ISDIR(dir_sb.st_mode)) {
			if (dir_sb.st_mtime == YACONF_G(directory_mtime)) {
				YACONF_DEBUG("config directory is not modefied");
				return SUCCESS;
			} else {
				zval result;
				int i, ndir;
				struct dirent **namelist;
				char *p, ini_file[MAXPATHLEN];

				YACONF_G(directory_mtime) = dir_sb.st_mtime;

				if ((ndir = php_scandir(dirname, &namelist, 0, php_alphasort)) > 0) {
					zend_stat_t sb;
					zend_file_handle fh = {{0}, 0};
					yaconf_filenode *node = NULL;

					for (i = 0; i < ndir; i++) {
						zval *orig_ht = NULL;
						if (!(p = strrchr(namelist[i]->d_name, '.')) || strcmp(p, ".ini")) {
							free(namelist[i]);
							continue;
						}

						snprintf(ini_file, MAXPATHLEN, "%s%c%s", dirname, DEFAULT_SLASH, namelist[i]->d_name);
						if (VCWD_STAT(ini_file, &sb) || !S_ISREG(sb.st_mode)) {//检查是否为普通文件
							free(namelist[i]);
							continue;
						}

						if ((node = (yaconf_filenode*)zend_hash_str_find_ptr(parsed_ini_files, namelist[i]->d_name, strlen(namelist[i]->d_name))) == NULL) {
							YACONF_DEBUG("new configure file found");
						} else if (node->mtime == sb.st_mtime) {//在静态hashTable上查找文件属性（最后修改时间），跟当前的文件属性对比
							free(namelist[i]);//时间相同，证明文件未修改，继续下个文件
							continue;
						}

						if ((fh.handle.fp = VCWD_FOPEN(ini_file, "r"))) {//文件最后时间发生变化，重新解析文件内容到result中
							fh.filename = ini_file;
							fh.type = ZEND_HANDLE_FP;
							ZVAL_UNDEF(&active_ini_file_section);
							YACONF_G(parse_err) = 0;
							php_yaconf_hash_init(&result, 128);
							if (zend_parse_ini_file(&fh, 1, 0 /* ZEND_INI_SCANNER_NORMAL */,
									php_yaconf_ini_parser_cb, (void *)&result) == FAILURE || YACONF_G(parse_err)) {
								YACONF_G(parse_err) = 0;
								php_yaconf_hash_destroy(Z_ARRVAL(result));
								free(namelist[i]);
								continue;
							}
						}

                        //在ini_containers这个hashTable中查找文件名（去掉.ini）
						if ((orig_ht = zend_symtable_str_find(ini_containers, namelist[i]->d_name, p - namelist[i]->d_name)) != NULL) {
							php_yaconf_hash_destroy(Z_ARRVAL_P(orig_ht));       //找到了，将原来的配置文件释放
							ZVAL_COPY_VALUE(orig_ht, &result);              //指向新的解析配置
						} else {//没有找到说明是新添加的文件，直接添加到hashTable上即可
							php_yaconf_symtable_update(ini_containers, namelist[i]->d_name, p - namelist[i]->d_name, &result);
						}

						if (node) {//如果node存在（说明不是新加的文件），更新一下属性（文件的最后修改时间）
							node->mtime = sb.st_mtime;
						} else {//node不存在说明是新加的文件，将文件属性添加到parsed_ini_files这个hashTable中
							yaconf_filenode n = {0};
							n.filename = zend_string_init(namelist[i]->d_name, strlen(namelist[i]->d_name), 1);
							n.mtime = sb.st_mtime;
							zend_hash_update_mem(parsed_ini_files, n.filename, &n, sizeof(yaconf_filenode));
						}
						free(namelist[i]);
					}
					free(namelist);
				}
				return SUCCESS;
			}
		} 
		YACONF_DEBUG("stat config directory failed");
	}

	return SUCCESS;
}
/* }}} */
#endif

/* {{{ PHP_MSHUTDOWN_FUNCTION
 *
 * #define PHP_MSHUTDOWN_FUNCTION	ZEND_MODULE_SHUTDOWN_D
 * #define ZEND_MODULE_SHUTDOWN_D(module)		int ZEND_MODULE_SHUTDOWN_N(module)(SHUTDOWN_FUNC_ARGS)
 * #define ZEND_MODULE_SHUTDOWN_N(module)		zm_shutdown_##module
 * #define SHUTDOWN_FUNC_ARGS	int type, int module_number
 *
 * zm_shutdown_yaconf(int type, int module_number)
 *
 * PHP_MSHUTDOWN
 * #define PHP_MSHUTDOWN	ZEND_MODULE_SHUTDOWN_N
 * #define ZEND_MODULE_SHUTDOWN_N(module)		zm_shutdown_##module
 *
 * zm_shutdown_yaconf
 */
PHP_MSHUTDOWN_FUNCTION(yaconf)
{
	UNREGISTER_INI_ENTRIES();

	if (parsed_ini_files) {//模块的shutdown释放才parsed_ini_files和ini_containers
		php_yaconf_hash_destroy(parsed_ini_files);
	}

	if (ini_containers) {
		php_yaconf_hash_destroy(ini_containers);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(yaconf)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "yaconf support", "enabled");
	php_info_print_table_row(2, "version", PHP_YACONF_VERSION);
#ifndef ZTS
	php_info_print_table_row(2, "yaconf config last check time",  ctime(&(YACONF_G(last_check))));
#else
	php_info_print_table_row(2, "yaconf config last check time",  "-");
#endif
	php_info_print_table_end();

	php_info_print_table_start();
	php_info_print_table_header(2, "parsed filename", "mtime");
	if (parsed_ini_files && zend_hash_num_elements(parsed_ini_files)) {
		yaconf_filenode *node;
		ZEND_HASH_FOREACH_PTR(parsed_ini_files, node) {
			php_info_print_table_row(2, ZSTR_VAL(node->filename),  ctime(&node->mtime));
		} ZEND_HASH_FOREACH_END();
	}
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
