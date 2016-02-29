/*
 * ZLog.h
 *
 *  Created on: 2015年3月5日
 *      Author: Administrator
 */

#ifndef ZLOG_H_
#define ZLOG_H_

#define ZLOG_FATAL 		5	// 每个严重的错误事件将会导致应用程序的退出。
#define ZLOG_ERROR 		4  	// 指出虽然发生错误事件，但仍然不影响系统的继续运行。
#define ZLOG_WARNING 	3 	// 表明会出现潜在错误的情形。
#define ZLOG_INFO  		2 	// 消息在粗粒度级别上突出强调应用程序的运行过程。
#define ZLOG_DEBUG 		1 	// 指出细粒度信息事件对调试应用程序是非常有帮助的。
#define ZLOG_ALL 		0 	// 最低等级,打开所有日志记录

/**
 * 设置日志等级
 * @return 最终使用的 level 等级
 */
const char* zlog_set_level(int level);

/**
 * 设置日志等级, 必须是一下几种
 *  "ALL","DEBUG","INFO","WARNING","ERROR","FATAL",
 * 否则返回，默认等级
 *
 * @level
 * @return 最终使用的 level 等级
 */
const char* zlog_set_strlevel(const char* level);

void zlog2(const char* filename, int line, const char* funcname, int level, const char* format, ...);

#define LOGE(format, ...) \
		zlog2(__FILE__, __LINE__, __func__, ZLOG_ERROR,	format, ##__VA_ARGS__)

#define LOGW(format, ...) \
		zlog2(__FILE__, __LINE__, __func__, ZLOG_WARNING,	format, ##__VA_ARGS__)

#define LOGI(format, ...) \
		zlog2(__FILE__, __LINE__, __func__, ZLOG_INFO,	format, ##__VA_ARGS__)

#define LOGD(format, ...) \
		zlog2(__FILE__, __LINE__, __func__, ZLOG_DEBUG,	format, ##__VA_ARGS__)

#define LOGA(format, ...) \
		zlog2(__FILE__, __LINE__, __func__, ZLOG_ALL,	format, ##__VA_ARGS__)

#define LOGF(format, ...) \
		zlog2(__FILE__, __LINE__, __func__, ZLOG_FATAL,	format, ##__VA_ARGS__)

#endif /* ZLOG_H_ */
