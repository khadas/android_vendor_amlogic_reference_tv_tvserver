/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#include "common.h"
#include "av/av_fuse.h"
#include "vbi/vbi_fuse.h"
#include "vbe/vbe_fuse.h"
#include "pq/pq_fuse.h"

int isRunning = 1;

extern struct cuse_obj av_cuse;
extern struct cuse_obj vbi_cuse;
extern struct cuse_obj vbe_cuse;
extern struct cuse_obj hdmirx_cuse;
extern struct cuse_obj pq_cuse;

struct cuse_obj *tvin_cuse[] = {
	&av_cuse,
	&vbi_cuse,
	&vbe_cuse,
	&hdmirx_cuse,
	//&vt_cuse,
	&pq_cuse,
};

static void sig_handler(int signo)
{
	isRunning = 0;
}

static void *tvin_cuse_thread(void *arg)
{
	int ret;
	struct cuse_obj *ptvin_obj = (struct cuse_obj *)arg;

	if (ptvin_obj == NULL) {
		printf("Null pointer\n");
		return NULL;
	}
	printf("Enter,%s,dev_name:%s\n", __func__, ptvin_obj->dev_name);

	ret = cuse_lowlevel_main(ptvin_obj->args.argc, ptvin_obj->args.argv,
		&ptvin_obj->ci, ptvin_obj->p_cuse_ops, arg);

	printf("pthread_exit,%s,dev_name:%s\n", __func__, ptvin_obj->dev_name);

	pthread_exit(NULL);
	return NULL;
}

#define CUSEXMP_OPT(t, p) { t, offsetof(struct cusexmp_param, p), 1 }

static const struct fuse_opt cusexmp_opts[] = {
	CUSEXMP_OPT("-M %u",		major),
	CUSEXMP_OPT("--maj=%u",		major),
	CUSEXMP_OPT("-m %u",		minor),
	CUSEXMP_OPT("--min=%u",		minor),
	CUSEXMP_OPT("-n %s",		dev_name),
	CUSEXMP_OPT("--name=%s",	dev_name),
	FUSE_OPT_KEY("-h",		0),
	FUSE_OPT_KEY("--help",		0),
	FUSE_OPT_END
};

static const char *usage =
"usage: cusexmp [options]\n"
"\n"
"options:\n"
"    --help|-h             print this help message\n"
"    --maj=MAJ|-M MAJ      device major number\n"
"    --min=MIN|-m MIN      device minor number\n"
"    --name=NAME|-n NAME   device name (mandatory)\n"
"    -d   -o debug         enable debug output (implies -f)\n"
"    -f                    foreground operation\n"
"    -s                    disable multi-threaded operation\n"
"\n";

static int cusexmp_process_arg(void *data, const char *arg, int key,
			       struct fuse_args *outargs)
{
	struct cusexmp_param *param = (struct cusexmp_param *)data;

	(void)outargs;
	(void)arg;

	switch (key) {
	case 0:
		param->is_help = 1;
		fprintf(stderr, "%s", usage);
		return fuse_opt_add_arg(outargs, "-ho");
	default:
		return 1;
	}
}

int main(int argc, char **argv)
{
	int i, ret = 0;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct cusexmp_param param = { 0, 0, NULL, 0 };

	if (fuse_opt_parse(&args, &param, cusexmp_opts, cusexmp_process_arg)) {
		printf("failed to parse option\n");
		free(param.dev_name);
		goto out;
	}

	signal(SIGCHLD, (__sighandler_t)sig_handler);
	signal(SIGINT,  (__sighandler_t)sig_handler);
	signal(SIGKILL, (__sighandler_t)sig_handler);
	signal(SIGQUIT, (__sighandler_t)sig_handler);

	printf("lht,%s:%d\n", __func__, __LINE__);
	for (i = 0;i < sizeof(tvin_cuse) / sizeof(tvin_cuse[0]);i++) {
		memset(&tvin_cuse[i]->ci, 0, sizeof(tvin_cuse[i]->ci));
		tvin_cuse[i]->ci.dev_major = param.major;
		tvin_cuse[i]->ci.dev_minor = param.minor;
		tvin_cuse[i]->ci.dev_info_argc = 1;
		tvin_cuse[i]->ci.dev_info_argv = &tvin_cuse[i]->dev_name;
		tvin_cuse[i]->ci.flags = CUSE_UNRESTRICTED_IOCTL;
		tvin_cuse[i]->args.argc = args.argc;
		tvin_cuse[i]->args.argv = args.argv;
		printf("lht,i=%d,dev_name:%s\n", i, *(tvin_cuse[i]->ci.dev_info_argv));
		ret = pthread_create(&tvin_cuse[i]->tid, NULL, tvin_cuse_thread, tvin_cuse[i]);
		if (ret != 0) {
			printf("pthread_create failed\n");
			return ret;
		}
	}

	printf("OK,%s:%d\n", __func__, __LINE__);

	while (isRunning)
	{
		sleep(1);
	}
	printf("stop running,%s:%d\n", __func__, __LINE__);

	/*
	 * Wait for tests to finish
	 */
	for (i = 0;i < sizeof(tvin_cuse) / sizeof(tvin_cuse[0]);i++)
		pthread_cancel(tvin_cuse[i]->tid);

	printf("exited,%s:%d\n", __func__, __LINE__);

	return 0;
out:
	fuse_opt_free_args(&args);
	return ret;
}
