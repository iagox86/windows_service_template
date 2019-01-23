#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "service_cli.h"
#include "service.h"

#pragma comment(lib, "advapi32.lib")

int main(int argc, char *argv[])
{
  if(argc > 1) {
    if(!strcmp( argv[1], "install")) {
      printf("Installing the service %s => %s...\n", SERVICE_NAME, SERVICE_DISPLAY_NAME);
      install_service(SERVICE_NAME, SERVICE_DISPLAY_NAME);
    } else if(!strcmp(argv[1], "uninstall")) {
      printf("Installing the service %s...\n", SERVICE_NAME);
      install_service(SERVICE_NAME, SERVICE_DISPLAY_NAME);
    } else {
      printf("Unknown command: %s!\n", argv[1]);
      printf("Usage: %s <install|uninstall>", argv[0]);
    }

    return 0;
  } else {
    printf("To install the service, run\n  %s install\n", argv[0]);
  }

  SERVICE_TABLE_ENTRY DispatchTable[] =
  {
    { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) service_main },
    { NULL, NULL }
  };

  // This call returns when the service has stopped.
  // The process should simply terminate when the call returns.

  if(!StartServiceCtrlDispatcher( DispatchTable ))
  {
    log_error("StartServiceCtrlDispatcher");
  }

  return 0;
}
