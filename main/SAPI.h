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
	char *name;                                                 //????????????cli???fpm???
	char *pretty_name;                                          //????????????????????????

	int (*startup)(struct _sapi_module_struct *sapi_module);    //??????????????????????????????
	int (*shutdown)(struct _sapi_module_struct *sapi_module);   //??????????????????????????????

	int (*activate)(void);                                      //??????request????????????????????????
	int (*deactivate)(void);                                    //?????????request??????????????????

	size_t (*ub_write)(const char *str, size_t str_length);     //??????????????????
	void (*flush)(void *server_context);                        //????????????
	zend_stat_t *(*get_stat)(void);                             //?????????????????????????????????????????????
	char *(*getenv)(char *name, size_t name_len);               //?????????????????????????????????

	void (*sapi_error)(int type, const char *error_msg, ...);   //????????????????????????

	int (*header_handler)(sapi_header_struct *sapi_header, sapi_header_op_enum op, sapi_headers_struct *sapi_headers);  //??????header()?????????????????????
	int (*send_headers)(sapi_headers_struct *sapi_headers);     //????????????header???????????????
	void (*send_header)(sapi_header_struct *sapi_header, void *server_context);     //???????????????header???????????????

	size_t (*read_post)(char *buffer, size_t count_bytes);      //??????HTTP POST????????????????????????
	char *(*read_cookies)(void);                                //??????COOKIE

	void (*register_server_variables)(zval *track_vars_array);  //???$_SERVER??????????????????????????????
	void (*log_message)(char *message);                         //??????????????????????????????
	double (*get_request_time)(void);                           //?????????????????????????????????
	void (*terminate_process)(void);                            //??????exit????????????????????????

	char *php_ini_path_override;                                //PHP???ini????????????????????????

	void (*block_interruptions)(void);
	void (*unblock_interruptions)(void);

	void (*default_post_reader)(void);                          //????????????POST??????
	void (*treat_data)(int arg, char *str, zval *destArray);    //?????????????????????
	char *executable_location;                                  //?????????????????????

	int php_ini_ignore;                                         //?????????????????????ini????????????
	int php_ini_ignore_cwd;                                     //?????????????????????php.ini

	int (*get_fd)(int *fd);                                     //?????????????????????fd???????????????

	int (*force_http_10)(void);                                 //????????????http1.0?????????????????????

	int (*get_target_uid)(uid_t *);                             //?????????????????????uid????????????
	int (*get_target_gid)(gid_t *);                             //?????????????????????gid????????????

	unsigned int (*input_filter)(int arg, char *var, char **val, size_t val_len, size_t *new_val_len);  //???????????????????????????????????????????????????????????????????????????????????????$_GET???$_POST???$_COOKIE???

	void (*ini_defaults)(HashTable *configuration_hash);
	int phpinfo_as_text;                                        //????????????phpinfo?????? //?????????ini???????????????????????????ini???????????????HashTable???

	char *ini_entries;                                          //??????????????????ini??????????????????php -d??????
	const zend_function_entry *additional_functions;            //??????SAPI??????????????????????????????????????????cli???cli_get_process_title
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
