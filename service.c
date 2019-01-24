#include "log.h"

#include "service.h"

typedef struct {
  SERVICE_STATUS service_status;
  SERVICE_STATUS_HANDLE service_status_handle;
  HANDLE stop_event;
} service_t;
service_t service = { 0, 0, 0 };

void do_service_startup_stuff() {
  /* TODO: Put service startup stuff here. Occasionally call
     service_report_status() if you're doing anything slow! */
  SERVICE_INFO("(do_service_startup_stuff goes here)");
}

void do_service_run_stuff() {
  /* TODO: Put service running stuff here. */
  SERVICE_INFO("Waiting for service.stop_event signal...");
  WaitForSingleObject(service.stop_event, INFINITE);
  SERVICE_INFO("...received service.stop_event signal!");
}

void do_service_shutdown_stuff() {
  /* TODO: Put stop-service stuff here. */
  SERVICE_INFO("(do_service_shutdown_stuff goes here)");
  SetEvent(service.stop_event);
}

/* Report the current status - the code here is a little black magic-y, it's
   pulled from MSDN. */
void service_report_status(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
  static DWORD checkpoint = 1;

  /* Fill in the SERVICE_STATUS structure */
  service.service_status.dwCurrentState = dwCurrentState;
  service.service_status.dwWin32ExitCode = dwWin32ExitCode;
  service.service_status.dwWaitHint = dwWaitHint;

  if(dwCurrentState == SERVICE_START_PENDING)
    service.service_status.dwControlsAccepted = 0;
  else
    service.service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

  if((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
    service.service_status.dwCheckPoint = 0;
  else
    service.service_status.dwCheckPoint = checkpoint++;

  /* Report the status of the service to the SCM. */
  SetServiceStatus(service.service_status_handle, &service.service_status);
}

/* This is a callback for when the service status changes. In particular, when
   the user attempts to stop a service this is notified. */
void WINAPI service_control_handler(DWORD dwCtrl) {
   switch(dwCtrl) {
     /* If the user asked for the service to stop... */
     case SERVICE_CONTROL_STOP:
       SERVICE_INFO("Received a SERVICE_CONTROL_STOP signal!");

       /* Report that we're stopping. */
       service_report_status(SERVICE_STOP_PENDING, NO_ERROR, 0);

       do_service_shutdown_stuff();

       /* Report that se've stopped. */
       service_report_status(SERVICE_STOPPED, NO_ERROR, 0);

       return;

     case SERVICE_CONTROL_INTERROGATE:
       break;

     default:
       break;
   }
}

/* Service entrypoint. */
void WINAPI service_main(int argc, char *argv[]) {
  /* Register a function to handle service-control requests (in particular,
     it'll be told when the service is being shut down). */
  service.service_status_handle = RegisterServiceCtrlHandler(SERVICE_NAME, service_control_handler);
  if(!service.service_status_handle) {
    return;
  }

  /* Service runs in its own process. */
  service.service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

  /* Process has no exit code. */
  service.service_status.dwServiceSpecificExitCode = 0;

  /* Let the OS know that the service is starting. */
  service_report_status(SERVICE_START_PENDING, NO_ERROR, 3000);

  /* Do startup stuff */
  do_service_startup_stuff();

  /* Create an event that'll kill the process. */
  service.stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
  if(!service.stop_event) {
    /* If the event creation fails, report the event stopped and return. */
    service_report_status(SERVICE_STOPPED, NO_ERROR, 0);
    SERVICE_ERROR("CreateEvent failed!");
    return;
  }

  /* Report that the service is now running. */
  service_report_status(SERVICE_RUNNING, NO_ERROR, 0);

  /* Do running stuff. */
  do_service_run_stuff();

  /* Report that the service is done. */
  service_report_status(SERVICE_STOPPED, NO_ERROR, 0);
}
