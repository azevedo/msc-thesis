#
# Selection functions of the tclabc GUI.
#
# This file is part of the tclabc package.
# See the file tkabc.tcl for more information.
#
# Copyright (C) 1999-2010, Jean-François Moine.
#
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

proc clip-export {} {
# export the selection when opt(twosel)
    global selection
    clipboard clear
    clipboard append $selection(clip)
    selection own -command {} .
}

proc clip-update {} {
# update the clipboard when something has changed in the selection
    global opt selection tune
#    if {[lsearch -exact $selection(syms) [abc go] < 0} return
    set clip {}
    abc go [lindex $selection(syms) 0]
    set v [abc get voice]
    abc go [lindex $selection(syms) end]
    if {[abc get voice] != $v} {
	set v -1
    } else {
	append clip "L:1/8\n"
    }
    foreach s $selection(syms) {
	abc go $s
	if {[abc get voice] != $v} {
	    if {[string length $clip] > 0 &&
		[string index $clip end] != "\n"} {
		append clip "\n"
	    }
	    set v [abc voice get]
	    append clip "V:[lindex $v 0]\nL:1/8\n"
	    set v [abc get voice]
	}
	set sym [abc get]
	if {[lindex $sym 0] == {extra} && [lindex $sym 1] == {newline}} {
	    if {[string length $clip] > 0 &&
		[string index $clip end] != "\n"} {
		append clip "\n"
	    }
	}
	append clip [abc dump $s]
    }
    # copy the selection to the clipboard
    if {!$opt(twosel)} {
	clipboard clear
	clipboard append $clip
	selection own -command sel-clear .
    } else {
	set selection(clip) $clip
    }
}

proc closest-sym {x y} {
# search the symbol closest to the mouse pointer
# ('current' is not usable)
    global score tune
    set v [expr {int($y / $score(height))}]
    if {$v > $tune(nvoice)} {
	set v $tune(nvoice)
    }
    abc go $v
    set xs [get-xoffset]
#puts "x:$x xs:$xs [abc go]"
    while {$xs < $x} {
	if {[string compare [lindex [abc get] 0] EOT] == 0} return
	abc go next
	set xnew [get-xoffset 1]
	if {$xnew < 0} return
#puts "xnew:$xnew [abc go]"
	if {$xnew >= $x} {
	    if {($xnew + $xs) / 2 > $x} {
		abc go prev
	    }
	    return
	}
	set xs $xnew
    }
    while {$xs > $x} {
	abc go prev
	if {[string compare [lindex [abc get] 0] EOT] == 0} {
	    abc go next
	    return
	}
	set xnew [get-xoffset 1]
	if {$xnew < 0} return
	if {$xnew <= $x} {
	    if {($xs + $xnew) / 2 < $x} {
		abc go next
	    }
	    return
	}
	set xs $xnew
    }
}

proc mark-copy {} {
# copy from mark to current to the selection
    global selection
#puts "mark-copy"
    set selection(syms) [list $selection(mark) [abc go]]
    sel-build
    sel-redisplay
    clip-update
#    abc go $s
}

proc mark-exch {} {
# exchange dot and mark
    global selection
    set mark $selection(mark)
    mark-set
    abc go $mark
    sym-sel
}

proc sel-build {} {
# build the selection from the 1st and last symbols
    global selection
    set s1 [lindex $selection(syms) 0]
    set s2 [lindex $selection(syms) end]
    set selection(syms) {}
    abc go $s1
    set t1 [abc get time]
    set v1 [abc get voice]
    abc go $s2
    set t2 [abc get time]
    set v2 [abc get voice]
    if {$v1 > $v2} {
	foreach [list v1 v2] [list $v2 $v1] break
    }
    if {$t1 > $t2} {
	foreach [list t1 t2] [list $t2 $t1] break
    }
    while {$v1 <= $v2} {
	abc go $s1
	set s [abc go $v1]
	while {1} {
	    if {[abc get time] >= $t1} break
	    set s [abc go next]
	}
	while {1} {
	    lappend selection(syms) $s
	    if {[abc get time] >= $t2} break
	    set s [abc go next]
	}
	incr v1
    }
}

proc sel-define {x y} {
# redefine the selection
    global selection tune
    set x [.f.c canvasx $x]
    set y [.f.c canvasy $y]
    if {[info exists selection(xy)]} {
	# if no selection rectangle yet
	foreach [list x1 y1] $selection(xy) break
	if {$x1 > $x} {
	    foreach [list x1 x] [list $x $x1] break
	}
	if {$y1 > $y} {
	    foreach [list y1 y] [list $y $y1] break
	}
	if {$x < $x1 + 3 && $y > $y1 - 3 && $y < $y1 + 3} {
	    return 0	; # no change
	}
	unset selection(xy)
	set selection(syms) [abc go]
	return 2	; # change + sel active
    }
#    if {[llength $selection(syms)] == 0}
    if {$selection(rect) == 0} {
	# mouse button-3 without previous selection
	set selection(syms) [abc go]
    }
    closest-sym $x $y
    set s [abc go]
    if {$s == [lindex $selection(syms) 0]
     || $s == [lindex $selection(syms) end]} {
	return 1	; # sel active
    }
    set selection(syms) [list [lindex $selection(syms) 0] $s]
    return 2	; # change + sel active
}

proc sel-delete {} {
# delete the symbols of the selection
    global selection
    if {$selection(rect) == 0} {
	set selection(syms) [list $selection(mark) [abc go]]
	sel-build
	clip-update
    }
    foreach s $selection(syms) {
	.f.c delete $s
	abc go $s
	abc delete
    }
    sel-clear
    set s [abc go]
    abc go prev
    redisplay 0
    abc go $s
    change-set 1
    cursor-display
}

proc sel-extend {x y} {
# extend the selection - bound to B1-Motion
    global selection
#puts "sel-extend $x $y"
    if {[sel-define $x $y] < 2} return
    sel-redisplay
}

proc sel-fetch {offs maxc} {
# return the selection
    global opt selection
#puts "sel-fetch"
    set s [selection get -selection CLIPBOARD]
    if {$offs != 0} {
	set s [string range $s $offs end]
    }
    if {$maxc < [string length $s]} {
	incr maxc -1
	set s [string range $s 0 $maxc]
    }
    return $s
}

proc sel-insert {x y {sel 3}} {
# insert the selection after the current symbol or mouse position
    global selection tune
    if {$sel & 1} {
	if {[catch {selection get} tmp]
	    && [catch {selection get -selection CLIPBOARD} tmp]} {
	    if {!($sel & 2)} return
	    set tmp $selection(clip)
	}
    } else {
	set tmp $selection(clip)
    }
    if {$x != 0} {
	# paste from mouse: search the nearest symbol
	closest-sym $x $y
    }
#puts "sel:$tmp"
#fixme: split into voices and dispatch
    set s [abc go]
    abc include $tmp
    abc go $tune(startsym)
    redisplay 1
#fixme: bad cursor position - should be at the end of inserted sequence
    abc go $s
    abc go next
    change-set 1
    cursor-display
}

proc sel-redisplay {} {
# redisplay the selection rectangle
    global selection voicerect
    set bbox [eval .f.c bbox $selection(syms)]
    if {[llength $bbox] > 0} {
	foreach [list x1 y1 x2 y2] $bbox break
	set x1 [expr {($x1 + $x2) / 2}]
	foreach s $selection(syms) {
	    set item [lindex [.f.c find withtag $s] 0]
	    set bbox [.f.c bbox $item]
	    set x [lindex $bbox 0]
	    if {$x < $x1} {
		set x1 $x
	    }
	    set x [lindex $bbox 2]
	    if {$x > $x2} {
		set x2 $x
	    }
	}
	incr x1 -5
	incr y1 -5
	incr x2 4
	incr y2 4
    } else {
	# no symbol of the selection is displayed
	set bbox [eval .f.c bbox $selection(rect)]
	foreach [list x1 y1 x2 y2] $bbox break
	set x1 2
	set x2 10
    }
    if {$selection(rect) == 0} {
	set selection(rect) [.f.c create rectangle $x1 $y1 $x2 $y2 \
		-outline black -fill DarkSeaGreen2]
	.f.c raise $selection(rect) $voicerect
    } else {
	.f.c coords $selection(rect) $x1 $y1 $x2 $y2
    }
}

proc sel-start {x y} {
# start the selection - bound to <Button-1>
    global selection
#puts "sel-start $x $y"
    focus .f.c
    set x [.f.c canvasx $x]
    set y [.f.c canvasy $y]
    set selection(xy) [list $x $y]
    if {$selection(rect) != 0} {
	.f.c delete $selection(rect)
	set selection(rect) 0
    }
    closest-sym $x $y
    sym-sel
}

proc sel-stop {x y} {
# stop the selection - bound to ButtonRelease-1 and ButtonRelease-3
    global selection
#puts "sel-stop $x $y"
    if {[sel-define $x $y] == 0} {
	catch {unset selection(xy)}
	return
    }
    sel-build
    clip-update
    sel-redisplay
    cursor-display
}
