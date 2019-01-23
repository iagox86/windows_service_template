#include <windows.h>
#include <stdio.h>

/* Installs the service in the Windows service manager. */
void install_service(char *name, char *display_name) {
  SC_HANDLE scmanager_handle;
  SC_HANDLE service_handle;
  char path[MAX_PATH];

  if(!GetModuleFileName(NULL, path, MAX_PATH))
  {
    printf("Cannot get the current path to install the service (%d)\n", GetLastError());
    return;
  }

  scmanager_handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if(!scmanager_handle) {
    printf("OpenSCManager failed, are you admin? (%d)\n", GetLastError());
    return;
  }

  service_handle = CreateService(
    scmanager_handle,              // SCM database
    name,                          // name of service
    display_name,                  // service name to display
    SERVICE_ALL_ACCESS,            // desired access
    SERVICE_WIN32_OWN_PROCESS,     // service type
    SERVICE_DEMAND_START,          // start type
    SERVICE_ERROR_NORMAL,          // error control type
    path,                          // path to service's binary
    NULL,                          // no load ordering group
    NULL,                          // no tag identifier
    NULL,                          // no dependencies
    NULL,                          // LocalSystem account
    NULL);                         // no password

  if(!service_handle)
  {
    printf("CreateService failed (%d)\n", GetLastError());
    CloseServiceHandle(scmanager_handle);
    return;
  }
  else printf("Service installed successfully\n");

  CloseServiceHandle(service_handle);
  CloseServiceHandle(scmanager_handle);
}

void uninstall_service(char *name) {
  SC_HANDLE scmanager_handle;
  SC_HANDLE service_handle;

  scmanager_handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if(!scmanager_handle) {
    printf("OpenSCManager failed, are you admin? (%d)\n", GetLastError());
    return;
  }

  service_handle = OpenService(scmanager_handle, name, SERVICE_ALL_ACCESS);
  if(!service_handle) {
    printf("OpenService failed (%d)\n", GetLastError());
    CloseServiceHandle(scmanager_handle);
    return;
  }

  if(!DeleteService(service_handle)) {
    printf("DeleteService failed (%d)\n", GetLastError());
  } else {
    printf("Service installed successfully\n");
  }

  CloseServiceHandle(service_handle);
  CloseServiceHandle(scmanager_handle);
}
