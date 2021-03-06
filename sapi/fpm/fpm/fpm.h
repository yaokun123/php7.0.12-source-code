
	/* $Id: fpm.h,v 1.13 2008/05/24 17:38:47 anight Exp $ */
	/* (c) 2007,2008 Andrei Nigmatulin */

#ifndef FPM_H
#define FPM_H 1

#include <unistd.h>

#ifdef HAVE_SYSEXITS_H
#include <sysexits.h>
#endif

#ifdef EX_OK
#define FPM_EXIT_OK EX_OK
#else
#define FPM_EXIT_OK 0
#endif

#ifdef EX_USAGE
#define FPM_EXIT_USAGE EX_USAGE
#else
#define FPM_EXIT_USAGE 64
#endif

#ifdef EX_SOFTWARE
#define FPM_EXIT_SOFTWARE EX_SOFTWARE
#else
#define FPM_EXIT_SOFTWARE 70
#endif

#ifdef EX_CONFIG
#define FPM_EXIT_CONFIG EX_CONFIG
#else
#define FPM_EXIT_CONFIG 78
#endif


int fpm_run(int *max_requests);
int fpm_init(int argc, char **argv, char *config, char *prefix, char *pid, int test_conf, int run_as_root, int force_daemon, int force_stderr);

struct fpm_globals_s {
	pid_t parent_pid;
	int argc;
	char **argv;
	char *config;
	char *prefix;				// 配置文件的前缀（一个目录）
	char *pid;
	int running_children;
	int error_log_fd;
	int log_level;
	int listening_socket; /* for this child */
	int max_requests; /* for this child */
	int is_child;
	int test_successful;
	int heartbeat;
	int run_as_root;
	int force_stderr;
	int send_config_pipe[2];
};

extern struct fpm_globals_s fpm_globals;

#endif
