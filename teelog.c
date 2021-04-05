#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/prctl.h>

/*****************************************************************************/

#define	MAX_LINE_LENGTH					(1024)
#define	MAX_LINES						(1000)
#define	MAX_SWITCHING_FILES				(4)

/*****************************************************************************/

#define	DBG								printf

/*****************************************************************************/

struct context {
	const char *prefix;
	int maxLineCount;
	int maxFileCount;

	int lineCount;
	int fileIndex;
	int option;
	FILE *lastFile;
};

/*****************************************************************************/


static FILE *getFile(struct context *ctx)
{
	while (ctx->lastFile) {
		if (ctx->lineCount >= ctx->maxLineCount)
			break;
		return ctx->lastFile;
	}

	if (ctx->lastFile) {
		fclose(ctx->lastFile);
		ctx->fileIndex++;
		if (ctx->fileIndex >= ctx->maxFileCount)
			ctx->fileIndex = 0;
	}

	ctx->lineCount = 0;

	char path[256];
	int len = sprintf(path, "%s-%d", ctx->prefix, ctx->fileIndex);
	path[len] = 0;
	//DBG("path=%s\n", path);
	ctx->lastFile = fopen(path, "w");
	assert(ctx->lastFile);
	/* unbuffered output */
	setbuf(ctx->lastFile, NULL);

	return ctx->lastFile;
}

static void *ppid_monitor(void *param)
{
	prctl(PR_SET_NAME, "ppid_monitor", 0UL, 0UL, 0UL);
	do {
		sleep(1);
	} while (getppid() != 1);

	//fprintf(stderr, "Terminate teelog (ppid=1)\n");
	exit(EXIT_SUCCESS);
	return NULL;
}

static void start_ppid_monitor(void)
{
	pthread_t thid;
	pthread_create(&thid, NULL, ppid_monitor, NULL);
	pthread_detach(thid);
}

static void teelog(struct context *ctx)
{
	char linebuf[MAX_LINE_LENGTH];
	start_ppid_monitor();
	while (!feof(stdin)) {
		char *p = fgets(linebuf, sizeof(linebuf) - 1, stdin);
		if (!p)
			continue;
		FILE *out = getFile(ctx);
		fputs(linebuf, stdout);
		if (out) {
			fputs(linebuf, out);
			ctx->lineCount++;
		}
	}
	fprintf(stderr, "teelog terminated.\n");
}

static void fatal_exit(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s [-l lines] [-f files] prefix\n", argv[0]);
	exit(EXIT_FAILURE);
}

static void parseOption(struct context *ctx, int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "l:f:")) != -1) {
		switch (opt) {
			case 'l':
				ctx->maxLineCount = atoi(optarg);
				break;
			case 'f':
				ctx->maxFileCount = atoi(optarg);
				break;
			default: /* '?' */
				fatal_exit(argc, argv);
				break;
		}
	}

	if (optind >= argc) {
		fatal_exit(argc, argv);
		/* never returns here */
	}
	ctx->prefix = (const char *)argv[optind];
}

int main(int argc, char **argv)
{
	struct context ctx = {
		.fileIndex = 0,
		.lineCount = 0,
		.prefix = NULL,
		.maxLineCount = MAX_LINES,
		.maxFileCount = MAX_SWITCHING_FILES,
	};
	parseOption(&ctx, argc, argv);
	teelog(&ctx);
	return 0;
}
