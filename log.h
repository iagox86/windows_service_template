#ifndef __LOG_H__
#define __LOG_H__

void log_error(int log_level, char *service_name, char *format, ...);
void log_event(int log_level, char *service_name, char *format, ...);

#endif
