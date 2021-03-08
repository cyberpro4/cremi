Version 0.9.72

This archive contains several W32 prebuild binaries of GNU libmicrohttpd.

The binaries are compiled without external dependencies, so HTTPS is not
supported.

Default versions are compatible with Vista and later, versions in *-xp
directories are compatible with XP and later.

Binaries are available for x86 32-bit (directory x86) and x86 64-bit
(directory x86_64).

For MinGW-compiled applications: use binaries in directories named MinGW:
MinGW/shared directory contains DLL (shared) versions of the library,
MinGW/static directory contains static lib versions.
To install libraries into existing MSYS2-MinGW toolchain, copy content of
static or shared directory to MSYS2 root directory (e.g. copy
libmicrohttpd-0.9.72-w32-bin/x86_64/MinGW/static/mingw64 to 
C:\msys64\ so copied dir becomes C:\msys64\mingw64)

For applications compiled by Visual Studio 2017/2019, use binaries in
VS2017/VS2019 directories.
VS201?/Debug-* directories contain versions suitable for debugging,
VS201?/Release-* directories contain versions optimized for release,
VS201?/*-dll contains DLL versions,
VS201?/*-static contains static versions.

I recommend to use static version when possible to avoid conflicts with other
.dlls in present and future and other DLL-Hell related problems. Don't be
afraid of huge size of the .lib files - they contains information for link
time optimization. The final application .exe file will grow for less then
120Kb for 64-bit version and less then 90Kb for 32-bit.

Karlson2k aka Evgeny Grin
