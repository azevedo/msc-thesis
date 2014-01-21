In order to build tclabc.dll, you require
Microsoft Visual C++ (version 6 or higher)
and Tcl/Tk (version 8.3 or higher) installed
on your computer.

Copy file config.h.in to config.h
and edit the line 
#define VERSION xxx
change xxx to "0.18.0" or whatever version
this distribution is assigned.

Open the directory win32 and
click on tkabc.dsw to start Microsoft
Visual C++. Click on menu item 
Build/Set Active Configuration and change
it to tclabc - win32 release.

Press F7. It should build tclabc.dll
issueing numerous warnings. Tclabc.dll
should be found in the new directory called 
Release in the win32 subdirectory.


Once you have created tclabc.dll, create
a new directory called wintkabc
directly under C:\. Copy tclabc.dll
into the wintkabc folder as well as all the
files with a tcl and html extension.

To create an executable using freewrap,
copy freewrap.exe (from freewrap55.zip)
to wintkabc. Open a DOS command window
and cd to wintkabc. Then type 
freewrap tkabc.tcl.

A tkabc.exe should be created shortly.

Now remove freewrap.exe from wintkabc.


