#ifndef __SERVICE_CLI__
#define __SERVICE_CLI__

#include <windows.h>

/* Create a service with the given name and displan name.
 *
 * name: The name the service uses internally
 * display_name: The name displayed to the user
 * user: The user account the service runs under (or NULL for SYSTEM)
 * password: The password for the user account (or NULL for SYSTEM)
 * description: A description for the service (or NULL for blank)
 */
void install_service(char *name, char *display_name, char *user, char *password, char *description);

/* Uninstall the service with the given name.
 *
 * name: The name the service was created under
 */
void uninstall_service(char *name);

/* Grant start / stop permissions for the service to the given sid_type.
 *
 * name: The name the service was created under
 * sid_type: The account type to grant permission to. WinBuiltinUsersSid is a
 *  good one, it means any user account. For more options, see:
 *  https://docs.microsoft.com/en-us/windows/desktop/api/winnt/ne-winnt-well_known_sid_type
 */
void give_permission_to(char *service_name, WELL_KNOWN_SID_TYPE sid_type);

#endif
