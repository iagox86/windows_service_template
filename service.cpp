#include "log.h"

#include "service.h"

typedef struct {
  SERVICE_STATUS service_status;
  SERVICE_STATUS_HANDLE service_status_handle;
  HANDLE stop_event;
} service_t;
service_t service = { 0, 0, 0 };

//
// Purpose:
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation,
//     in milliseconds
//
// Return value:
//   None
//
void service_report_status( DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
  static DWORD dwCheckPoint = 1;

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
    service.service_status.dwCheckPoint = dwCheckPoint++;

  // Report the status of the service to the SCM.
  SetServiceStatus( service.service_status_handle, &service.service_status );
}

//
// Purpose:
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
//
// Return value:
//   None
//
void WINAPI service_control_handler(DWORD dwCtrl)
{
   // Handle the requested control code.

   switch(dwCtrl)
   {
   case SERVICE_CONTROL_STOP:
     service_report_status(SERVICE_STOP_PENDING, NO_ERROR, 0);

     // Signal the service to stop.

     SetEvent(service.stop_event);
     service_report_status(service.service_status.dwCurrentState, NO_ERROR, 0);

     return;

   case SERVICE_CONTROL_INTERROGATE:
     break;

   default:
     break;
   }

}

//
// Purpose:
//   The service code
//
// Parameters:
//   argc - Number of arguments in the argv array
//   argv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
//
// Return value:
//   None
//
void service_initialize(DWORD argc, LPTSTR *argv)
{
  // TO_DO: Declare and set any required variables.
  //   Be sure to periodically call service_report_status() with
  //   SERVICE_START_PENDING. If initialization fails, call
  //   service_report_status with SERVICE_STOPPED.

  // Create an event. The control handler function, service_control_handler,
  // signals this event when it receives the stop control code.

  service.stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
  if(!service.stop_event)
  {
    service_report_status( SERVICE_STOPPED, NO_ERROR, 0 );
    return;
  }

  // Report running status when initialization is complete.

  service_report_status( SERVICE_RUNNING, NO_ERROR, 0 );

  // TO_DO: Perform work until service stops.

  while(1)
  {
    // Check whether to stop the service.

    WaitForSingleObject(service.stop_event, INFINITE);

    service_report_status( SERVICE_STOPPED, NO_ERROR, 0 );
    return;
  }
}

/* Service entrypoint. */
void WINAPI service_main(int argc, char *argv[])
{
  service_t service;

  service.service_status_handle = RegisterServiceCtrlHandler(SERVICE_NAME, service_control_handler);
  if(!service.service_status_handle)
  {
    log_error("RegisterServiceCtrlHandler");
    return;
  }

  // These SERVICE_STATUS members remain as set here
  service.service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  service.service_status.dwServiceSpecificExitCode = 0;

  // Report initial status to the SCM
  service_report_status(SERVICE_START_PENDING, NO_ERROR, 3000);

  // Perform service-specific initialization and work.

  service_initialize(argc, argv );
}
