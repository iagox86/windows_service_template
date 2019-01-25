#include <windows.h>
#include <stdio.h>

#include "log.h"
#include "service_cli.h"
#include "service.h"

void usage(char *name) {
  printf("Usage: %s <install|uninstall> [.\\<user> <password>]", name);
}

int main(int argc, char *argv[]) {
  HANDLE event = CreateEvent(NULL, TRUE, FALSE, NULL);

  SERVICE_TABLE_ENTRY DispatchTable[] = {
    { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) service_main },
    { NULL, NULL }
  };

  if(argc > 1) {
    if(!strcmp(argv[1], "install")) {
      if(argc > 3) {
        /* Install as a user account */
        printf("Installing the service %s => %s...\n", SERVICE_NAME, SERVICE_DISPLAY_NAME);
        install_service(SERVICE_NAME, SERVICE_DISPLAY_NAME, argv[2], argv[3]);
      } else {
        /* Install as LocalSystem */
        printf("Installing the service %s => %s...\n", SERVICE_NAME, SERVICE_DISPLAY_NAME);
        install_service(SERVICE_NAME, SERVICE_DISPLAY_NAME, NULL, NULL);
      }
    } else if(!strcmp(argv[1], "uninstall")) {
      printf("Uninstalling the service %s...\n", SERVICE_NAME);
      uninstall_service(SERVICE_NAME);
    } else if(!strcmp(argv[1], "test")) { /* TODO: Delete me */
      update_service_permissions(SERVICE_NAME);
    } else {
      printf("Unknown command: %s!\n", argv[1]);
      usage(argv[0]);
    }

    return 0;
  } else {
    /* This will try to print when being started as an actual service, but
     * nobody will see it */
    usage(argv[0]);
  }

  /* This call returns when the service stops. */
  if(!StartServiceCtrlDispatcher( DispatchTable )) {
    SERVICE_ERROR("StartServiceCtrlDispatcher failed!");
  }

  return 0;
}
