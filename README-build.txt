                     Some Thing To Know About Building Pterm / DtCyber
                     =================================================

In addition to downloading the sources for DtCyber, you will also need to download the sources for
wxWidgets, which this build relies upon.  You can download the latest sources (at least as of
10/4/2009) at http://www.wxwidgets.org/downloads/

You want to install a system-wide copy of wxWidgets, so follow the configuration, building and
installation instructions for the operating system you are installing it on.  The first step of
getting wxWidgets installed is to configure the installation.  Here is the "../configure"
command that PK has been using (for the Mac - other OSes will vary WRT SDKs, SDK locations, etc.):

../configure --with-macosx-sdk=/Developer/SDKs/MacOSX10.4u.sdk --with-macosx-version-min=10.4 --enable-universal_binary --enable-intl --enable-config --enable-mdi --enable-postscript --enable-printarch --enable-clipboard --enable-gif --with-opengl --prefix=/usr/local/wxstaticu.2810 --disable-shared --enable-static --enable-monolithic --enable-unicode

In addition, apparently wxWidgets makes certain assumptions about what sort of bitmap drawing
it can do, and at least on the Mac you will want to set wxMAC_USE_CORE_GRAPHICS to 0 in file
<your-wxwidgets-sources>/include/mac/carbon/chkconf.h.  As for other OSes, you can update this
ReadMe with the results of your build.  Most likely this is a Mac-only issue pertaining to the
deprecation of QuickDraw (which, if you want to know, is a 32-bit Carbon set of APIs that will
no longer be supported, possibly as soon as Snow Leopard (10.6.x)).

The wxWidgets version currently being used by Pterm is 2.8.10.

It would appear that the latest incarnation of DtCyber/Pterm also contains a dependency on a
sound library called libSDL.  You can download the run-time libraries (and sources if you are
interested) at http://www.libsdl.org/download-1.2.php .

One final note - for the Mac application there is a dependency on the command-line tool
"gettext", which also appears to have a partner in crime, "msgfmt".  There is an Open Source
version (0.17 as of 10/4/2009)  of gettext that can be built from sources that are available
on http://gettext.darwinports.com .  Again, for other OSes you can update this ReadMe with
the results of your build experimentation.  On the MacOSX build at least the only effect NOT
having "gettext" installed is that various non-English language support resources will not be
copied into the Pterm.app application.

"msgfmt" comes in the same package as "gettext".  There are default build rules for

	%.mo	:	%.po

...that uses "msgfmt", and then the %.mo files are used by "gettext" to generate localized
message files that are copied into the Pterm.app application.  The failure to do either of
these tasks on MacOS is not fatal.  It just means that some localizable text will not be
localized if you don't have those tools.  It's not fatal to the build.
