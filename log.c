#include <stdio.h>
#include <windows.h>
#include "log.h"

/* Types: EVENTLOG_SUCCESS, EVENTLOG_INFORMATION_TYPE, EVENTLOG_WARNING_TYPE, EVENTLOG_ERROR_TYPE */
static void log_internal(int type, char *log_name, char *message) {
  HANDLE event_source;

  event_source = RegisterEventSource(NULL, log_name);
  if(event_source) {
    const char *strings[1];
    strings[0] = message;

    ReportEvent(
      event_source, /* hEventLog */
      type,         /* dwType */
      0,            /* wCategory */
      0,            /* DWORD  dwEventID TODO Might want to use this? */
      NULL,         /* PSID   lpUserSid, */
      1,            /* WORD   wNumStrings - number of text strings */
      0,            /* DWORD  dwDataSize - bytes of binary data */
      strings,      /* LPCSTR *lpStrings - text strings */
      NULL          /* LPVOID lpRawData - binary data */
    );

    DeregisterEventSource(event_source);
  }
}

void log_error(int log_level, char *service_name, char *format, ...) {
  va_list  args;
  int      len;
  char    *buffer;

  va_start( args, format );
  len = _vscprintf( format, args ) + 1; /* +1 for terminating \0 */

  buffer = (char*) malloc(len);

  vsprintf(buffer, format, args);

  log_internal(log_level, service_name, buffer);

  free(buffer);
  va_end(args);
}

void log_event(int log_level, char *service_name, char *format, ...) {
  va_list  args;
  int      len;
  char    *buffer;

  va_start( args, format );
  len = _vscprintf( format, args ) + 1; /* +1 for terminating \0 */
  buffer = (char*) malloc(len);
  vsprintf(buffer, format, args);

  log_internal(log_level, service_name, buffer);

  free(buffer);
  va_end(args);
}
