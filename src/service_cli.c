#include <windows.h>
#include <stdio.h>
#include <aclapi.h>
#include <accctrl.h>

/* Installs the service in the Windows service manager. */
void install_service(char *name, char *display_name, char *user, char *password) {
  SC_HANDLE scmanager_handle;
  SC_HANDLE service_handle;
  char path[MAX_PATH];

  if(!GetModuleFileName(NULL, path, MAX_PATH)) {
    fprintf(stderr, "Cannot get the current path to install the service (%d)\n", GetLastError());
    return;
  }

  scmanager_handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if(!scmanager_handle) {
    DWORD error = GetLastError();

    fprintf(stderr, "OpenSCManager failed: %d\n", error);
    if(error == 5) {
      fprintf(stderr, " (access denied)\n");
    }
    return;
  }

  service_handle = CreateService(
    scmanager_handle,
    name,
    display_name,
    SERVICE_ALL_ACCESS,
    SERVICE_WIN32_OWN_PROCESS, /* Service runs in its own process - not shared or driver. */
    SERVICE_DEMAND_START,      /* Start type - on-demand. */
    SERVICE_ERROR_NORMAL,      /* Error control. */
    path,                      /* Path to the binary. */
    NULL,                      /* Load ordering group. */
    NULL,                      /* Tag identifier. */
    NULL,                      /* Dependencies. */
    user,                      /* Run as the requested user. */
    password                   /* Requested password. */
  );

  if(!service_handle) {
    DWORD error = GetLastError();
    fprintf(stderr, "CreateService failed: %d\n", error);
    if(error == 1057) {
      fprintf(stderr, " -> check the user account and password, don't forget to use .\\user for local account: %s / %s\n", user, password);
    } else if (error == 1073) {
      fprintf(stderr, " -> service already exists\n");
    } else if (error == 1072) {
      fprintf(stderr, " -> service marked for deletion (close services.msc or reboot)\n");
    }

    CloseServiceHandle(scmanager_handle);
    return;
  } else {
    printf("Service installed successfully\n");
  }

  CloseServiceHandle(service_handle);
  CloseServiceHandle(scmanager_handle);
}

void update_service_permissions(char *name) {
  SC_HANDLE scmanager_handle;
  SC_HANDLE service_handle;
  DWORD dummy, dummy2;
  PSECURITY_DESCRIPTOR security_descriptor = NULL;
  SECURITY_DESCRIPTOR new_security_descriptor;
  PACL acl = NULL;
  PACL new_acl = NULL;
  PSID sid = NULL;
  char account_name[0x200];
  char account_domain[0x200];
  SID_NAME_USE eUse;
  EXPLICIT_ACCESS explicit_access;

  scmanager_handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if(!scmanager_handle) {
    DWORD error = GetLastError();

    fprintf(stderr, "OpenSCManager failed: %d\n", error);
    if(error == 5) {
      fprintf(stderr, " (access denied)\n");
    }
    return;
  }

  service_handle = OpenService(scmanager_handle, name, SERVICE_ALL_ACCESS);
  if(!service_handle) {
    DWORD error = GetLastError();

    fprintf(stderr, "OpenService failed: (%d)\n", error);
    if(error == 1060) {
      fprintf(stderr, " -> Service does not exist\n");
    }
    CloseServiceHandle(scmanager_handle);
    return;
  }

  /* 8192 is the max possible size according to documentation. */
  security_descriptor = (PSECURITY_DESCRIPTOR) malloc(8192);
  printf("%p\n", security_descriptor);
  if(!QueryServiceObjectSecurity(service_handle, 4, security_descriptor, 8192, &dummy)) {
    DWORD error = GetLastError();
    fprintf(stderr, "Error querying service object security: %d\n", error);
    return;
  }

  if(!GetSecurityDescriptorDacl(security_descriptor, &dummy, &acl, &dummy2)) {
    DWORD error = GetLastError();
    fprintf(stderr, "Error getting security descriptor: %d\n", error);
    return;
  }
  if(!dummy) {
    fprintf(stderr, "No DACL was included in the security descriptor!\n");
    return;
  }

  sid = (PSID) malloc(0x44); /* TODO: No idea where this comes from. */
  dummy = 0x44;
  if(!CreateWellKnownSid(WinBuiltinUsersSid, NULL, sid, &dummy)) {
    DWORD error = GetLastError();
    fprintf(stderr, "Error getting well known SID: %d\n", error);
    return;
  }

  dummy = 0x200;
  if(!LookupAccountSid(NULL, sid, account_name, &dummy, account_domain, &dummy, &eUse)) {
    DWORD error = GetLastError();
    fprintf(stderr, "Error looking up account name: %d\n", error);
    return;
  }

  printf("%s\\%s\n", account_domain, account_name);

  /* FreeSid(&sid); TODO y u no work? */

  BuildExplicitAccessWithName(&explicit_access, account_name, 0x80000030, 2, 0);

  if(SetEntriesInAcl(1, &explicit_access, acl, &new_acl) != ERROR_SUCCESS) {
    fprintf(stderr, "Error setting entries in ACL\n");
    return;
  }

  if(!InitializeSecurityDescriptor(&new_security_descriptor, SECURITY_DESCRIPTOR_REVISION)) {
    DWORD error = GetLastError();
    fprintf(stderr, "Error initializing security descriptor: %d\n", error);
    return;
  }

  if(!SetSecurityDescriptorDacl(&new_security_descriptor, 1, new_acl, 0)) {
    DWORD error = GetLastError();
    fprintf(stderr, "Error setting DACL: %d\n", error);
    return;
  }

  if(!SetServiceObjectSecurity(service_handle, 4 /* TODO */, &new_security_descriptor)) {
    DWORD error = GetLastError();
    fprintf(stderr, "Error setting service object security: %d\n", error);
    return;
  }

  printf("okay5...\n");


  CloseServiceHandle(service_handle);
  CloseServiceHandle(scmanager_handle);
}

void uninstall_service(char *name) {
  SC_HANDLE scmanager_handle;
  SC_HANDLE service_handle;

  scmanager_handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if(!scmanager_handle) {
    DWORD error = GetLastError();

    fprintf(stderr, "OpenSCManager failed: %d\n", error);
    if(error == 5) {
      fprintf(stderr, " (access denied)\n");
    }
    return;
  }

  service_handle = OpenService(scmanager_handle, name, SERVICE_ALL_ACCESS);
  if(!service_handle) {
    DWORD error = GetLastError();

    fprintf(stderr, "OpenService failed: (%d)\n", error);
    if(error == 1060) {
      fprintf(stderr, " -> Service does not exist\n");
    }
    CloseServiceHandle(scmanager_handle);
    return;
  }

  if(!DeleteService(service_handle)) {
    DWORD error = GetLastError();
    fprintf(stderr, "DeleteService failed: %d\n", error);

    if(error == 1072) {
      fprintf(stderr, " -> service marked for deletion (close services.msc or reboot)\n");
    } else if(error == 1060) {
      fprintf(stderr, " -> service does not exist\n");
    }
  } else {
    printf("Service uninstalled successfully\n");
  }

  CloseServiceHandle(service_handle);
  CloseServiceHandle(scmanager_handle);
}
