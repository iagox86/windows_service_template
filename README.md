# windows_service_template

This is a quick and easy template to create a Windows service in C. I had
trouble finding a good basic implementation online, and I needed to write a
service, so I went ahead and wrote this one.

## Compiling

I wrote this in Microsoft Visual Studio 2008. I have no idea if it will compile
on a newer version of Visual Studio - I don't have any of them.

To build it, I simply run...

```
$ build.bat
```

Which builds both `Debug` and `Release` builds, based on the `.vcproj` file.
You can also load up the `.vcproj` file in Visual Studio and compile it there.

## Writing a service

Basically, there are three parts to writing a service...

* First, fill out `do_service_startup_stuff()` with any startup bits you need.
  If anything is particularly slow, you'll want to call
  `service_report_status()` to let the Service Manager know that stuff is
  happening!

* Second, fill out `do_service_run_stuff()` with code that implements the
  service. Right now, it simply waits for `stop_event` to fire - that event
  should always stop the service immediately, unless you change
  `do_service_shutdown_stuff()`.

* Finally, fill out `do_service_shutdown_stuff()` with any cleanup you need.
  When it gets called, it's the service's responsibility to stop everything and
  exit. Right now set an event that is consumed by `do_service_run_stuff()`,
  which is a common way to do it.

Other than that, the world is your oyster!

## Logging

I created some simple log functions: `SERVICE_INFO()`, `SERVICE_WARNING()`, and
`SERVICE_ERROR()`. They take printf-style varargs, and send the output to the
Windows eventlog.

`SERVICE_ERROR()` additionally includes the result of `GetLastError()`.
