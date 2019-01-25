#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <windows.h>

/* Changes the service name (for commandline) */
#define SERVICE_NAME         "test_service"

/* Changes the service display name */
#define SERVICE_DISPLAY_NAME "Test Service"

/* Changes the service decription */
#define SERVICE_DESCRIPTION "This is my test service"

/* Log helpers that include service information */
#define SERVICE_INFO(f, ...)    log_event(EVENTLOG_INFORMATION_TYPE, SERVICE_DISPLAY_NAME, "%s:%d %s() :: "f, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define SERVICE_WARNING(f, ...) log_event(EVENTLOG_WARNING_TYPE,     SERVICE_DISPLAY_NAME, "%s:%d %s() :: "f, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define SERVICE_ERROR(f, ...)   log_error(EVENTLOG_ERROR_TYPE,       SERVICE_DISPLAY_NAME, "%s:%d %s() :: "f, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

void service_report_status(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
void WINAPI service_control_handler(DWORD dwCtrl);
void service_initialize(DWORD argc, LPTSTR *argv);
void WINAPI service_main(int argc, char *argv[]);

#endif
