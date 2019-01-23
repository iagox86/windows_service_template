#include <windows.h>
#include <stdio.h>

/* Installs the service in the Windows service manager. */
void install_service(char *name, char *display_name) {
  SC_HANDLE scmanager_handle;
  SC_HANDLE service_handle;
  char path[MAX_PATH];

  if(!GetModuleFileName(NULL, path, MAX_PATH)) {
    printf("Cannot get the current path to install the service (%d)\n", GetLastError());
    return;
  }

  scmanager_handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if(!scmanager_handle) {
    printf("OpenSCManager failed, are you admin? (%d)\n", GetLastError());
    return;
  }

  service_handle = CreateService(
    scmanager_handle,
    name,
    display_name,
    SERVICE_ALL_ACCESS,
    SERVICE_WIN32_OWN_PROCESS, /* Service type. */
    SERVICE_DEMAND_START,      /* Start type. */
    SERVICE_ERROR_NORMAL,      /* Error control. */
    path,                      /* Path to the binary. */
    NULL,                      /* Load ordering group. */
    NULL,                      /* Tag identifier. */
    NULL,                      /* Dependencies. */
    NULL,                      /* Run as LocalSystem. */
    NULL                       /* No password. */
  );

  if(!service_handle) {
    printf("CreateService failed, does the service already exist? (%d)\n", GetLastError());
    CloseServiceHandle(scmanager_handle);
    return;
  } else {
    printf("Service installed successfully\n");
  }

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
    printf("OpenService failed, does the service exist? Is services.msc open? (%d)\n", GetLastError());
    CloseServiceHandle(scmanager_handle);
    return;
  }

  if(!DeleteService(service_handle)) {
    printf("DeleteService failed (%d)\n", GetLastError());
  } else {
    printf("Service uninstalled successfully\n");
  }

  CloseServiceHandle(service_handle);
  CloseServiceHandle(scmanager_handle);
}
