#
# Help functions of the tclabc GUI.
#
# This file is part of the tclabc package.
# See the file tkabc.tcl for more information.
#
# Copyright (C) 1999-2010, Jean-François Moine.
#
# Contact: mailto:moinejf@free.fr
# Original site: http://moinejf.free.fr/
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

proc help {} {
# display the help window
    global abclib
    webdisp help:$abclib/index.html
}

proc help-about {} {
    global abcversion
    tk_messageBox -icon info \
	    -message "  tkabc version $abcversion
by Jean-François Moine" \
	-title {About} -type ok
}

proc show-keys {} {
# list the key bindings
    global lang tcl_platform
    if {[winfo exists .keys]} {
	raise .keys
	focus .keys.f.txt
	return
    }
    toplevel .keys
    wm title .keys $lang(h_kb)
    frame .keys.f
    text .keys.f.txt -yscrollcommand {.keys.f.scroll set} \
	    -setgrid true \
	    -exportselection 0 -width 72 -height 25
    scrollbar .keys.f.scroll -command {.keys.f.txt yview}

#    .keys.f.txt insert end "Keystroke Bindings:\n--------- --------\n"
    .keys.f.txt insert end "---- $lang(h_kb) ----\n"
    foreach binding [lsort [bind .f.c]] {
        .keys.f.txt insert end  [format "\n%-20s => '%s'" $binding [bind .f.c $binding]]
    }
    .keys.f.txt configure -state disabled

    button .keys.b -text OK -command [list destroy .keys]

    pack .keys.f.scroll -side left -fill y
    pack .keys.f.txt -side right -fill both
    pack .keys.f .keys.b -side top
    bind .keys <Return> [list destroy .keys]
    bind .keys <Escape> [list destroy .keys]
    bind .keys <q> [list destroy .keys]
    bind .keys <Down> [list .keys.f.txt yview scroll 1 units]
    bind .keys <Up> [list .keys.f.txt yview scroll -1 units]
    if {$tcl_platform(platform) == {unix}} {
	bind .keys <KP_Down> [list .keys.f.txt yview scroll 1 units]
	bind .keys <KP_Up> [list .keys.f.txt yview scroll -1 units]
    }
    focus .keys.f.txt
#    .keys.f.txt yview moveto 0
    .keys.f.txt mark set insert 1.0
}
