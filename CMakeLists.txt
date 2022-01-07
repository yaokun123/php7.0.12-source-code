cmake_minimum_required(VERSION 3.14)

project(php_7_0_12)

set(CMAKE_CXX_STANDARD 14)

set(PHP_SOURCE /Users/yaokun/php-private/php-source-code/src/php-7.0.12)


set(SOURCE_FILES
        sapi/cli/php_cli.c
        sapi/cli/ps_title.c
        sapi/fpm/fpm/fpm_main.c
        sapi/fpm/fpm/fpm.c
        sapi/fpm/fpm/fpm_children.c
        sapi/fpm/fpm/fpm_conf.c
        sapi/fpm/fpm/fpm_scoreboard.c
        sapi/fpm/fpm/fpm_signals.c
        sapi/fpm/fpm/fpm_events.c
        sapi/fpm/fpm/fpm_request.c
        sapi/fpm/fpm/fpm_process_ctl.c
        sapi/fpm/fpm/fpm_sockets.c
        main/SAPI.c
        main/alloca.c
        main/fastcgi.c
        main/fopen_wrappers.c
        main/getopt.c
        main/internal_functions.c
        main/internal_functions_cli.c
        main/main.c
        main/mergesort.c
        main/network.c
        main/output.c
        main/php_content_types.c
        main/php_ini.c
        main/php_open_temporary_file.c
        main/php_scandir.c
        main/php_sprintf.c
        main/php_ticks.c
        main/php_variables.c
        main/reentrancy.c
        main/rfc1867.c
        main/snprintf.c
        main/spprintf.c
        main/strlcat.c
        main/strlcpy.c

        Zend/zend_compile.c
        Zend/zend.c
        Zend/zend_API.c
        Zend/zend_alloc.c
        Zend/zend_ast.c
        Zend/zend_builtin_functions.c
        Zend/zend_closures.c
        Zend/zend_compile.c
        Zend/zend_constants.c
        Zend/zend_default_classes.c
        Zend/zend_dtrace.c
        Zend/zend_exceptions.c
        Zend/zend_execute.c
        Zend/zend_execute_API.c
        Zend/zend_extensions.c
        Zend/zend_float.c
        Zend/zend_gc.c
        Zend/zend_generators.c
        Zend/zend_hash.c
        Zend/zend_highlight.c
        Zend/zend_inheritance.c
        Zend/zend_ini.c
        Zend/zend_ini_parser.c
        Zend/zend_ini_scanner.c
        Zend/zend_interfaces.c
        Zend/zend_iterators.c
        Zend/zend_language_parser.c
        Zend/zend_language_scanner.c
        Zend/zend_list.c
        Zend/zend_llist.c
        Zend/zend_multibyte.c
        Zend/zend_object_handlers.c
        Zend/zend_objects.c
        Zend/zend_objects_API.c
        Zend/zend_opcode.c
        Zend/zend_operators.c
        Zend/zend_ptr_stack.c
        Zend/zend_signal.c
        Zend/zend_smart_str.c
        Zend/zend_sort.c
        Zend/zend_sprintf.c
        Zend/zend_stack.c
        Zend/zend_stream.c
        Zend/zend_string.c
        Zend/zend_strtod.c
        Zend/zend_ts_hash.c
        Zend/zend_variables.c
        Zend/zend_virtual_cwd.c
        Zend/zend_vm_opcodes.c

        ext/standard/array.c
        ext/standard/assert.c
        ext/standard/base64.c
        ext/standard/basic_functions.c
        ext/standard/browscap.c
        ext/standard/crc32.c
        ext/standard/credits.c
        ext/standard/crypt.c
        ext/standard/crypt_blowfish.c
        ext/standard/crypt_freesec.c
        ext/standard/crypt_sha256.c
        ext/standard/crypt_sha512.c
        ext/standard/css.c
        ext/standard/cyr_convert.c
        ext/standard/datetime.c
        ext/standard/dir.c
        ext/standard/dl.c
        ext/standard/dns.c
        ext/standard/exec.c
        ext/standard/file.c
        ext/standard/filestat.c
        ext/standard/filters.c
        ext/standard/flock_compat.c
        ext/standard/formatted_print.c
        ext/standard/fsock.c
        ext/standard/ftok.c
        ext/standard/ftp_fopen_wrapper.c
        ext/standard/head.c
        ext/standard/html.c
        ext/standard/http.c
        ext/standard/http_fopen_wrapper.c
        ext/standard/image.c
        ext/standard/incomplete_class.c
        ext/standard/info.c
        ext/standard/iptc.c
        ext/standard/lcg.c
        ext/standard/levenshtein.c
        ext/standard/link.c
        ext/standard/mail.c
        ext/standard/math.c
        ext/standard/md5.c
        ext/standard/metaphone.c
        ext/standard/microtime.c
        ext/standard/pack.c
        ext/standard/pageinfo.c
        ext/standard/password.c
        ext/standard/php_crypt_r.c
        ext/standard/php_fopen_wrapper.c
        ext/standard/proc_open.c
        ext/standard/quot_print.c
        ext/standard/rand.c
        ext/standard/random.c
        ext/standard/scanf.c
        ext/standard/sha1.c
        ext/standard/soundex.c
        ext/standard/streamsfuncs.c
        ext/standard/string.c
        ext/standard/strnatcmp.c
        ext/standard/syslog.c
        ext/standard/type.c
        ext/standard/uniqid.c
        ext/standard/url.c
        ext/standard/url_scanner_ex.c
        ext/standard/user_filters.c
        ext/standard/uuencode.c
        ext/standard/var.c
        ext/standard/var_unserializer.c
        ext/standard/versioning.c

        ext/mysqlnd/mysqlnd_alloc.c
        ext/mysqlnd/mysqlnd_auth.c
        ext/mysqlnd/mysqlnd_block_alloc.c
        ext/mysqlnd/mysqlnd_charset.c
        ext/mysqlnd/mysqlnd_debug.c
        ext/mysqlnd/mysqlnd_driver.c
        ext/mysqlnd/mysqlnd_ext_plugin.c
        ext/mysqlnd/mysqlnd_loaddata.c
        ext/mysqlnd/mysqlnd_net.c
        ext/mysqlnd/mysqlnd_plugin.c
        ext/mysqlnd/mysqlnd_ps.c
        ext/mysqlnd/mysqlnd_ps_codec.c
        ext/mysqlnd/mysqlnd_result.c
        ext/mysqlnd/mysqlnd_result_meta.c
        ext/mysqlnd/mysqlnd_reverse_api.c
        ext/mysqlnd/mysqlnd_statistics.c
        ext/mysqlnd/mysqlnd_wireprotocol.c
        ext/mysqlnd/php_mysqlnd.c

        #[[ext/mysqli/mysqli.c
        ext/mysqli/mysqli_api.c
        ext/mysqli/mysqli_driver.c
        ext/mysqli/mysqli_embedded.c
        ext/mysqli/mysqli_exception.c
        ext/mysqli/mysqli_fe.c
        ext/mysqli/mysqli_nonapi.c
        ext/mysqli/mysqli_prop.c
        ext/mysqli/mysqli_report.c
        ext/mysqli/mysqli_result_iterator.c
        ext/mysqli/mysqli_warning.c]]

        ext/pdo/pdo.c
        ext/pdo/pdo_dbh.c
        ext/pdo/pdo_sql_parser.c
        ext/pdo/pdo_sqlstate.c
        ext/pdo/pdo_stmt.c

        #ext/pdo_mysql/mysql_driver.c
        #ext/pdo_mysql/mysql_statement.c
        #ext/pdo_mysql/pdo_mysql.c

        ext/curl/curl_file.c
        ext/curl/interface.c
        ext/curl/multi.c
        ext/curl/share.c

        )
add_executable(php ${SOURCE_FILES})

include_directories(${PHP_SOURCE}/main)
include_directories(${PHP_SOURCE}/ext)

include_directories(${PHP_SOURCE}/Zend)

include_directories(${PHP_SOURCE}/sapi)
include_directories(${PHP_SOURCE}/sapi/fpm)
include_directories(${PHP_SOURCE}/

include_directories(${PHP_SOURCE}/pear)

include_directories(${PHP_SOURCE}/TSRM)

include_directories(${PHP_SOURCE})

add_custom_target(makefile COMMAND make && make install WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})