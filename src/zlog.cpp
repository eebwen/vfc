/*
 * ZLog.c
 *
 *  Created on: 2015年3月5日
 *      Author: Administrator
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#define INT64_C
#define __STDC_CONSTANT_MACROS
#include <stdint.h>

extern "C" {
#include <libavutil/log.h>
}

#define ZLOG_FATAL 		5	// 每个严重的错误事件将会导致应用程序的退出。
#define ZLOG_ERROR 		4  	// 指出虽然发生错误事件，但仍然不影响系统的继续运行。
#define ZLOG_WARNING 	3 	// 表明会出现潜在错误的情形。
#define ZLOG_INFO  		2 	// 消息在粗粒度级别上突出强调应用程序的运行过程。
#define ZLOG_DEBUG 		1 	// 指出细粒度信息事件对调试应用程序是非常有帮助的。
#define ZLOG_ALL 		0 	// 最低等级,打开所有日志记录

#define MAX_PRINT_LEN 		1024

static int current_level = ZLOG_DEBUG;
static const char *levels[] = {
		"ALL",
		"DEBUG",
		"INFO",
		"WARNING",
		"ERROR",
		"FATAL",
		NULL
};

const char* zlog_set_level(int level) {
	if (level >= ZLOG_ALL && level <= ZLOG_FATAL)
		current_level = level;

	return levels[current_level];
}

const char* zlog_set_strlevel(const char* level) {
	assert(level);
	for (int i = 0; levels[i] != NULL; i++) {
		if (strcmp(level, levels[i]) == 0) {
			current_level = i;
			break;
		}
	}

	return levels[current_level];
}

void zlog2(const char* filename, int line, const char* funcname, int level, const char* format, ...) {

	if (level < current_level)
		return ;

	int str_len = 0;
	char str[MAX_PRINT_LEN] = "";

	time_t t = time(NULL);
	struct tm tm;

	// 1. time
	localtime_r(&t, &tm);
	sprintf(str, "[%d/%02d/%02d %02d:%02d:%02d][%s]",
			tm.tm_year + 1990, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
			levels[level]);

	// 2. 日志所在文件，及其位置
	str_len = strlen(str);
	snprintf(str + str_len, MAX_PRINT_LEN - str_len - 1, "[%s::%s:%d]",
			filename, funcname, line);

	// 写入 空格
	str_len = strlen(str);
	snprintf(str + str_len, MAX_PRINT_LEN - str_len - 1, " ");

	// 3. 日志信息
	va_list args;
	va_start(args, format);

	str_len = strlen(str);
	vsnprintf(str + str_len, MAX_PRINT_LEN - str_len - 1, format, args);

	va_end(args);

	av_log(NULL, AV_LOG_INFO, "%s", str);
}

