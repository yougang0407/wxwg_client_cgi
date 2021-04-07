#ifndef __LOG_H__
#define __LOG_H__

#define WWC_DEBUG(format, ...) \
		printf("[DEBUG][%s][%s][%d] "format, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define WWC_INFO(format, ...) \
		printf("[INFO][%s][%s][%d] "format, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define WWC_WARN(format, ...) \
		printf("[WARN][%s][%s][%d] "format, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define WWC_ERROR(format, ...) \
		printf("[ERROR][%s][%s][%d] "format, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#endif

