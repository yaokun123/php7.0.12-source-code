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
   | Author:  Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef SAPI_H
#define SAPI_H

#include "php.h"
#include "zend.h"
#include "zend_API.h"
#include "zend_llist.h"
#include "zend_operators.h"
#ifdef PHP_WIN32
#include "win95nt.h"
#include "win32/php_stdint.h"
#endif
#include <sys/stat.h>
#include "php.h"

#define SAPI_OPTION_NO_CHDIR 1
#define SAPI_POST_BLOCK_SIZE 0x4000

#ifdef PHP_WIN32
#	ifdef SAPI_EXPORTS
#		define SAPI_API __declspec(dllexport)
#	else
#		define SAPI_API __declspec(dllimport)
#	endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define SAPI_API __attribute__ ((visibility("default")))
#else
#	define SAPI_API
#endif

#undef shutdown

typedef struct {
	char *header;
	size_t header_len;
} sapi_header_struct;


typedef struct {
	zend_llist headers;
	int http_response_code;
	unsigned char send_default_content_type;
	char *mimetype;
	char *http_status_line;
} sapi_headers_struct;


typedef struct _sapi_post_entry sapi_post_entry;
typedef struct _sapi_module_struct sapi_module_struct;

BEGIN_EXTERN_C()
extern SAPI_API sapi_module_struct sapi_module;  /* true global */
END_EXTERN_C()

/* Some values in this structure needs to be filled in before
 * calling sapi_activate(). We WILL change the `char *' entries,
 * so make sure that you allocate a separate buffer for them
 * and that you free them after sapi_deactivate().
 */

typedef struct {
	const char *request_method;
	char *query_string;
	char *cookie_data;
	zend_long content_length;

	char *path_translated;
	char *request_uri;

	/* Do not use request_body directly, but the php://input stream wrapper instead */
	struct _php_stream *request_body;

	const char *content_type;

	zend_bool headers_only;
	zend_bool no_headers;
	zend_bool headers_read;

	sapi_post_entry *post_entry;

	char *content_type_dup;

	/* for HTTP authentication */
	char *auth_user;
	char *auth_password;
	char *auth_digest;

	/* this is necessary for the CGI SAPI module */
	char *argv0;

	char *current_user;
	int current_user_length;

	/* this is necessary for CLI module */
	int argc;
	char **argv;
	int proto_num;
} sapi_request_info;


typedef struct _sapi_globals_struct {
	void *server_context;
	sapi_request_info request_info;
	sapi_headers_struct sapi_headers;
	int64_t read_post_bytes;
	unsigned char post_read;
	unsigned char headers_sent;
	zend_stat_t global_stat;
	char *default_mimetype;
	char *default_charset;
	HashTable *rfc1867_uploaded_files;
	zend_long post_max_size;
	int options;
	zend_bool sapi_started;
	double global_request_time;
	HashTable known_post_content_types;
	zval callback_func;
	zend_fcall_info_cache fci_cache;
} sapi_globals_struct;


BEGIN_EXTERN_C()
#ifdef ZTS
# define SG(v) ZEND_TSRMG(sapi_globals_id, sapi_globals_struct *, v)
SAPI_API extern int sapi_globals_id;
#else
# define SG(v) (sapi_globals.v)
extern SAPI_API sapi_globals_struct sapi_globals;
#endif

SAPI_API void sapi_startup(sapi_module_struct *sf);
SAPI_API void sapi_shutdown(void);
SAPI_API void sapi_activate(void);
SAPI_API void sapi_deactivate(void);
SAPI_API void sapi_initialize_empty_request(void);
END_EXTERN_C()

/*
 * This is the preferred and maintained API for
 * operating on HTTP headers.
 */

/*
 * Always specify a sapi_header_line this way:
 *
 *     sapi_header_line ctr = {0};
 */

typedef struct {
	char *line; /* If you allocated this, you need to free it yourself */
	size_t line_len;
	zend_long response_code; /* long due to zend_parse_parameters compatibility */
} sapi_header_line;

typedef enum {					/* Parameter: 			*/
	SAPI_HEADER_REPLACE,		/* sapi_header_line* 	*/
	SAPI_HEADER_ADD,			/* sapi_header_line* 	*/
	SAPI_HEADER_DELETE,			/* sapi_header_line* 	*/
	SAPI_HEADER_DELETE_ALL,		/* void					*/
	SAPI_HEADER_SET_STATUS		/* int 					*/
} sapi_header_op_enum;

BEGIN_EXTERN_C()
SAPI_API int sapi_header_op(sapi_header_op_enum op, void *arg);

/* Deprecated functions. Use sapi_header_op instead. */
SAPI_API int sapi_add_header_ex(char *header_line, size_t header_line_len, zend_bool duplicate, zend_bool replace);
#define sapi_add_header(a, b, c) sapi_add_header_ex((a),(b),(c),1)


SAPI_API int sapi_send_headers(void);
SAPI_API void sapi_free_header(sapi_header_struct *sapi_header);
SAPI_API void sapi_handle_post(void *arg);
SAPI_API size_t sapi_read_post_block(char *buffer, size_t buflen);
SAPI_API int sapi_register_post_entries(sapi_post_entry *post_entry);
SAPI_API int sapi_register_post_entry(sapi_post_entry *post_entry);
SAPI_API void sapi_unregister_post_entry(sapi_post_entry *post_entry);
SAPI_API int sapi_register_default_post_reader(void (*default_post_reader)(void));
SAPI_API int sapi_register_treat_data(void (*treat_data)(int arg, char *str, zval *destArray));
SAPI_API int sapi_register_input_filter(unsigned int (*input_filter)(int arg, char *var, char **val, size_t val_len, size_t *new_val_len), unsigned int (*input_filter_init)(void));

SAPI_API int sapi_flush(void);
SAPI_API zend_stat_t *sapi_get_stat(void);
SAPI_API char *sapi_getenv(char *name, size_t name_len);

SAPI_API char *sapi_get_default_content_type(void);
SAPI_API void sapi_get_default_content_type_header(sapi_header_struct *default_header);
SAPI_API size_t sapi_apply_default_charset(char **mimetype, size_t len);
SAPI_API void sapi_activate_headers_only(void);

SAPI_API int sapi_get_fd(int *fd);
SAPI_API int sapi_force_http_10(void);

SAPI_API int sapi_get_target_uid(uid_t *);
SAPI_API int sapi_get_target_gid(gid_t *);
SAPI_API double sapi_get_request_time(void);
SAPI_API void sapi_terminate_process(void);
END_EXTERN_C()

struct _sapi_module_struct {
	char *name;                                                 //名字，如cli、fpm等
	char *pretty_name;                                          //更容易理解的名字

	int (*startup)(struct _sapi_module_struct *sapi_module);    //模块启动时调用的函数
	int (*shutdown)(struct _sapi_module_struct *sapi_module);   //模块结束时调用的函数

	int (*activate)(void);                                      //处理request时需要调用的函数
	int (*deactivate)(void);                                    //处理完request要调用的函数

	size_t (*ub_write)(const char *str, size_t str_length);     //用于输出数据
	void (*flush)(void *server_context);                        //刷新缓存
	zend_stat_t *(*get_stat)(void);                             //判断对执行的文件是否有执行权限
	char *(*getenv)(char *name, size_t name_len);               //获取函数变量的函数指针

	void (*sapi_error)(int type, const char *error_msg, ...);   //错误处理函数指针

	int (*header_handler)(sapi_header_struct *sapi_header, sapi_header_op_enum op, sapi_headers_struct *sapi_headers);  //调用header()时被调用的函数
	int (*send_headers)(sapi_headers_struct *sapi_headers);     //发送全部header的函数指针
	void (*send_header)(sapi_header_struct *sapi_header, void *server_context);     //发送某一个header的函数指针

	size_t (*read_post)(char *buffer, size_t count_bytes);      //获取HTTP POST中数据的函数指针
	char *(*read_cookies)(void);                                //获取COOKIE

	void (*register_server_variables)(zval *track_vars_array);  //从$_SERVER中获取变量的函数指针
	void (*log_message)(char *message);                         //输出错误信息函数指针
	double (*get_request_time)(void);                           //获取请求时间的函数指针
	void (*terminate_process)(void);                            //调用exit退出时的函数指针

	char *php_ini_path_override;                                //PHP的ini文件被复写的地址

	void (*block_interruptions)(void);
	void (*unblock_interruptions)(void);

	void (*default_post_reader)(void);                          //负责解析POST数据
	void (*treat_data)(int arg, char *str, zval *destArray);    //对数据进行处理
	char *executable_location;                                  //执行的地理位置

	int php_ini_ignore;                                         //是否不使用任何ini配置文件
	int php_ini_ignore_cwd;                                     //忽略当前路径的php.ini

	int (*get_fd)(int *fd);                                     //获取执行文件的fd的函数指针

	int (*force_http_10)(void);                                 //强制使用http1.0版本的函数指针

	int (*get_target_uid)(uid_t *);                             //获取执行程序的uid函数指针
	int (*get_target_gid)(gid_t *);                             //获取执行程序的gid函数指针

	unsigned int (*input_filter)(int arg, char *var, char **val, size_t val_len, size_t *new_val_len);  //对输入进行过滤的函数指针，比如将输入参数填充到自动全局变量$_GET、$_POST、$_COOKIE中

	void (*ini_defaults)(HashTable *configuration_hash);
	int phpinfo_as_text;                                        //是否输出phpinfo信息 //默认的ini配置的函数指针，把ini配置信息在HashTable中

	char *ini_entries;                                          //执行时附带的ini配置，可以使php -d设置
	const zend_function_entry *additional_functions;            //每个SAPI模块特有的一些函数注册，比如cli的cli_get_process_title
	unsigned int (*input_filter_init)(void);
};


struct _sapi_post_entry {
	char *content_type;
	uint content_type_len;
	void (*post_reader)(void);
	void (*post_handler)(char *content_type_dup, void *arg);
};

/* header_handler() constants */
#define SAPI_HEADER_ADD			(1<<0)


#define SAPI_HEADER_SENT_SUCCESSFULLY	1
#define SAPI_HEADER_DO_SEND				2
#define SAPI_HEADER_SEND_FAILED			3

#define SAPI_DEFAULT_MIMETYPE		"text/html"
#define SAPI_DEFAULT_CHARSET		PHP_DEFAULT_CHARSET
#define SAPI_PHP_VERSION_HEADER		"X-Powered-By: PHP/" PHP_VERSION

#define SAPI_POST_READER_FUNC(post_reader) void post_reader(void)
#define SAPI_POST_HANDLER_FUNC(post_handler) void post_handler(char *content_type_dup, void *arg)

#define SAPI_TREAT_DATA_FUNC(treat_data) void treat_data(int arg, char *str, zval* destArray)
#define SAPI_INPUT_FILTER_FUNC(input_filter) unsigned int input_filter(int arg, char *var, char **val, size_t val_len, size_t *new_val_len)

BEGIN_EXTERN_C()
SAPI_API SAPI_POST_READER_FUNC(sapi_read_standard_form_data);
SAPI_API SAPI_POST_READER_FUNC(php_default_post_reader);
SAPI_API SAPI_TREAT_DATA_FUNC(php_default_treat_data);
SAPI_API SAPI_INPUT_FILTER_FUNC(php_default_input_filter);
END_EXTERN_C()

#define STANDARD_SAPI_MODULE_PROPERTIES

#endif /* SAPI_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
