#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Changes the service name (for commandline) */
#define SERVICE_NAME         "test_service"

/* Changes the service display name */
#define SERVICE_DISPLAY_NAME "Test Service"

void log_error(char *szFunction);
void service_report_status(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
void WINAPI service_control_handler(DWORD dwCtrl);
void service_initialize(DWORD argc, LPTSTR *argv);
void WINAPI service_main(int argc, char *argv[]);

#endif
