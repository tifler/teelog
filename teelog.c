#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

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

ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

static void teelog(struct context *ctx)
{
	char linebuf[MAX_LINE_LENGTH];
	while (fgets(linebuf, sizeof(linebuf) - 1, stdin)) {
		FILE *out = getFile(ctx);
		fputs(linebuf, stdout);
		if (out) {
			fputs(linebuf, out);
			ctx->lineCount++;
		}
	}
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
