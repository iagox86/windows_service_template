#include <windows.h>
#include <stdio.h>
#include <aclapi.h>
#include <accctrl.h>

/* Installs the service in the Windows service manager. */
void install_service(char *name, char *display_name, char *user, char *password, char *description) {
  SC_HANDLE scmanager_handle;
  SC_HANDLE service_handle;
  char path[MAX_PATH];

  if(!GetModuleFileName(NULL, path, MAX_PATH)) {
    fprintf(stderr, "Cannot get the current path to install the service: %d\n", GetLastError());
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
      fprintf(stderr, " -> bad user! Check the user account and password, don't forget to use .\\user for local account: %s / %s\n", user, password);
    } else if (error == 1073) {
      fprintf(stderr, " -> service already exists!\n");
    } else if (error == 1072) {
      fprintf(stderr, " -> service marked for deletion (close services.msc or reboot)!\n");
    }

    CloseServiceHandle(scmanager_handle);
    return;
  } else {
    printf("Service installed!");
    if(description) {
      SERVICE_DESCRIPTION sd;
      sd.lpDescription = description;
      if(!ChangeServiceConfig2(service_handle, SERVICE_CONFIG_DESCRIPTION, &sd)) {
        fprintf(stderr, "...but couldn't set the description!");
      }
    }
  }

  CloseServiceHandle(service_handle);
  CloseServiceHandle(scmanager_handle);
}

void give_permission_to(char *service_name, WELL_KNOWN_SID_TYPE sid_type) {
  SC_HANDLE scmanager_handle;
  SC_HANDLE service_handle;

  DWORD dummy;

  DWORD old_security_descriptor_length = 8192;
  PSECURITY_DESCRIPTOR old_security_descriptor = (PSECURITY_DESCRIPTOR) malloc(8192);
  SECURITY_DESCRIPTOR new_security_descriptor;
  PACL old_acl = NULL;
  PACL new_acl = NULL;

  DWORD sid_length = 0x100;
  PSID sid = (PSID) malloc(0x100);

  DWORD account_name_length = 0x200;
  char account_name[0x200];
  DWORD account_domain_length = 0x200;
  char account_domain[0x200];

  SID_NAME_USE eUse;
  EXPLICIT_ACCESS explicit_access;

  /* Get a handle to the service control manager. */
  scmanager_handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if(!scmanager_handle) {
    DWORD error = GetLastError();

    fprintf(stderr, "OpenSCManager failed: %d\n", error);
    if(error == 5) {
      fprintf(stderr, " (access denied)\n");
    }

    goto close_scmanager_handle;
  }

  /* Get a handle to the service. */
  service_handle = OpenService(scmanager_handle, service_name, SERVICE_ALL_ACCESS);
  if(!service_handle) {
    DWORD error = GetLastError();

    fprintf(stderr, "OpenService failed: %d\n", error);
    if(error == 1060) {
      fprintf(stderr, " -> Service does not exist\n");
    }

    goto close_scmanager_handle;
  }

  /* Get the current security descriptor - 8192 is the max size, according to
     MSDN */
  if(!QueryServiceObjectSecurity(service_handle, 4, old_security_descriptor, old_security_descriptor_length, &old_security_descriptor_length)) {
    fprintf(stderr, "Error querying service object security: %d\n", GetLastError());
    goto close_service_handle;
  }

  /* Get the current DACL */
  if(!GetSecurityDescriptorDacl(old_security_descriptor, &dummy, &old_acl, &dummy)) {
    fprintf(stderr, "Error getting security descriptor: %d\n", GetLastError());
    goto close_service_handle;
  }

  /* Get a SID handle for the user type they're granting permission to. */
  if(!CreateWellKnownSid(sid_type, NULL, sid, &sid_length)) {
    fprintf(stderr, "Error getting well known SID: %d\n", GetLastError());
    goto close_service_handle;
  }

  /* Get the account associated with the given SID */
  if(!LookupAccountSid(NULL, sid, account_name, &account_name_length, account_domain, &account_domain_length, &eUse)) {
    fprintf(stderr, "Error looking up account name: %d\n", GetLastError());
    goto close_service_handle;
  }

  /* Build an ACL with permission to start and stop the service. */
  BuildExplicitAccessWithName(&explicit_access, account_name, GENERIC_READ | SERVICE_START | SERVICE_STOP, SET_ACCESS, 0);

  /* Create a new ACL with the new permission. */
  if(SetEntriesInAcl(1, &explicit_access, old_acl, &new_acl) != ERROR_SUCCESS) {
    fprintf(stderr, "Error setting entries in ACL\n");
    goto close_service_handle;
  }

  /* Create an empty security descriptor. */
  if(!InitializeSecurityDescriptor(&new_security_descriptor, SECURITY_DESCRIPTOR_REVISION)) {
    fprintf(stderr, "Error initializing security descriptor: %d\n", GetLastError());
    goto close_service_handle;
  }

  /* Update the DACL in the new security decriptor. */
  if(!SetSecurityDescriptorDacl(&new_security_descriptor, TRUE, new_acl, FALSE)) {
    fprintf(stderr, "Error setting DACL: %d\n", GetLastError());
    goto close_service_handle;
  }

  /* Set the service's security to the new security descriptor. */
  if(!SetServiceObjectSecurity(service_handle, DACL_SECURITY_INFORMATION, &new_security_descriptor)) {
    fprintf(stderr, "Error setting service object security: %d\n", GetLastError());
    goto close_service_handle;
  }

  printf("Done!\n");

close_service_handle:
  CloseServiceHandle(service_handle);
close_scmanager_handle:
  CloseServiceHandle(scmanager_handle);
  free(old_security_descriptor);
  free(sid);
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

    fprintf(stderr, "OpenService failed: %d\n", error);
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
