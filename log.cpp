#include <stdio.h>
#include <windows.h>
#include "log.h"

#if 0
/* TODO: Use this */
void log_error(char *func) {
  HANDLE event_source;
  char *strings[2];
  char buffer[80];

  event_source = RegisterEventSource(NULL, "TODO: Service Name");
  if(event_source) {
    sprintf(buffer, "%s failed with %d", func, GetLastError());

    strings[0] = "TODO: Service Name";
    strings[1] = buffer;

/*        ReportEvent(event_source,        // event log handle
            EVENTLOG_ERROR_TYPE, // event type
            0,                   // event category
            SVC_ERROR,           // event identifier
            NULL,                // no security identifier
            2,                   // size of strings array
            0,                   // no binary data
            strings,         // array of strings
            NULL);               // no binary data
*/
    DeregisterEventSource(event_source);
  }
}
#endif

void log_error(char *format, ...) {
  va_list  args;
  int      len;
  char    *buffer;
  FILE    *f = fopen("c:\\users\\ron\\desktop\\log.txt", "a");

  va_start( args, format );
  len = _vscprintf( format, args ) + 1; /* +1 for terminating \0 */

  buffer = (char*) malloc(len);

  vsprintf(buffer, format, args);

  fprintf(f, "Error %d: %s\n", GetLastError(), buffer);
  fclose(f);

  free(buffer);
  va_end(args);
}
void log_event(char *format, ...) {
  va_list  args;
  int      len;
  char    *buffer;
  FILE    *f = fopen("c:\\users\\ron\\desktop\\log.txt", "a");

  va_start( args, format );
  len = _vscprintf( format, args ) + 1; /* +1 for terminating \0 */

  buffer = (char*) malloc(len);

  vsprintf(buffer, format, args);

  fprintf(f, "%s\n", buffer);
  fclose(f);

  free(buffer);
  va_end(args);
}
