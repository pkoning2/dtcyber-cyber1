              Some Thing To Know About Building Pterm / DtCyber
              =================================================

If you downloaded the Pterm source kit, you have sources for Pterm but
not for the other parts of DtCyber (the Cyber emulator created by Tom
Hunter).   

If you checked out the sources from the Subversion server at
akdesign.dyndns.org, that also gives you the rest of the DtCyber
package: the emulator itself (dtcyber), the operator interface
(dtoper), the Cyber console display (dd60) and the PLATO black box
emulator (blackbox).  If you want to run your own emulated Cyber
system, you'll need the full kit.  But most users need only Pterm,
which is how you connect to the PLATO system at cyber1.org.  

Prerequisites for building Pterm

Pterm relies on wxWidgets for the user interface framework.  The
current version was built against both V3.0 and V3.1; it should work
with later versions (but that has not yet been tested); it will not
work with older versions.

In addition, it uses libSDL for audio output, and libsndfile for
writing audio files.

Building on Linux:

Using the package manager, install the development files for
libsndfile and libSDL.  For example, on CentOS you would use "yum
install libsndfile-devel libSDL-devel".

If a packaged version of wxWidgets 3.0 or later is available for your
system, install that (the development files, not just the runtime
library -- typically this would have a package name like
"wxwidgets-dev").  If no packaged versionis available, build wxWidgets
from sources; see the wxWidgets documentation for details.

    Note: you should build the wxWidgets shared libraries.  For
    reasons that are not clear, currently wxWidgets static libraries
    do not work in 32 bit builds.  (Things build fine, but the pterm
    executable crashes deep in the bowels of X11.)  Shared libraries
    work fine in both 32 and 64 bit builds.

Now build pterm with "make pterm".  The result should be a pterm
executable, which you can run from the build directory or move to any
other convenient directory.

Building on Mac OS:

Install libsndfile and libSDL from the released kits.

If a pre-built kit of wxWidgets 3.0 or 3.1 is available, install that.
Otherwise, build it from sources.  The configure I used to build it
was as follows:

../configure --enable-universal_binary=i386,ppc --disable-shared \
	      --enable-static --enable-monolithic \
	      --with-macosx-sdk=/Developer/SDKs/MacOSX10.5.sdk \
	      --with-macosx-version-min=10.4

You can get away with fewer switches and/or some different settings,
especially if you intend to build it just for your own use rather than
for distributing kits to others.  If you have a system which uses the
clang compiler rather than gcc, and as a result does not have PowerPC
support in the compiler, omit ",ppc" from the configure string.  If
you use wxWidgets 3.1, you may need to set the min version higher,
perhaps 10.5 or 10.6, and/or change the SDK version used.

Now build pterm with "make Pterm.app".  The result should be a Pterm
application (a "package" which is really a directory with a .app
extension).  You can execute that from the build directory, or move it
to some other location like /Applications.

Building on Windows:

Install libsndfile, libSDL, and wxWidgets (3.0 or later) from the
released kits. 

Now open Pterm.vxcproj with Microsoft Visual Studio.  I used the 2010
edition, so the released project files are for that version.  If you
have a different version, you may need to convert.

Build Pterm (typically you'll want to use the Release build).  You may
need to copy some of the DLLs is needs into the build output
directory for Pterm to run properly after building.

The build scripts in my sources include the building of an installer,
using InnoSetup (an open source Windows installer creator).  If you
have that installed, you should get a setup.exe which you can use to
install the Pterm you built.

Building the other parts of DtCyber:

The Pterm prerequisites are sufficient for building the rest of
DtCyber.  On Linux and Mac OS, "make" with no arguments (or "make
all") will build all the DtCyber components (dtcyber, dtoper, dd60,
blackbox, pterm).  On Mac OS, dtoper, dd60, and pterm will be build as
Application packages (with .app extensions) while dtcyber and blackbox
are simple Unix command line applications that require no packaging.

There is no current Windows project file for building the other parts
of DtCyber.  There is a DtCyber.dsw file that was used some time ago
with Visual C++ 2006, but it fails conversion to VS 2010 and in any
case will likely need updating.

