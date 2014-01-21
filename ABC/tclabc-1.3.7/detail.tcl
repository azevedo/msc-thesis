#
# Detail functions of the tclabc GUI.
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

#fixme: draw slurs at the end - no special tag

proc beam-draw {xoffset yoffset stag} {
# draw the beams
    global flags voice
    set s [abc go]
    set sym [abc get]
#puts "beam-draw $s ($sym) stag:$stag"
    set v [abc get voice]
    set deltav [expr {39 - 2 * $voice($v,clef)}]
    foreach [list yhigh ylow] [note-pits $sym] break
    set ylow [expr {($deltav - $ylow) * 3 + $yoffset}]
    set yhigh [expr {($deltav - $yhigh) * 3 + $yoffset}]
    set ylower $ylow
    set yhigher $yhigh
    set nnotes 1
    set x1 [get-xoffset]
    set xtb $x1
    set xl 0
    set xh 0
    set maxfl [flag-get [lindex $sym 1]]
    set minfl $maxfl
    set fltb $maxfl
    set sigma [expr {$ylow + $yhigh}]
    for {} {1} {} {
	abc go prev
	set sym2 [abc get]
	switch [lindex $sym2 0] {
	    EOT return
	    note {}
	    default continue
	}
	foreach [list yh yl] [note-pits $sym2] break
	set yl [expr {($deltav - $yl) * 3 + $yoffset}]
	set yh [expr {($deltav - $yh) * 3 \
		+ $yoffset}]
	lappend ylow $yl
	lappend yhigh $yh
	if {[set lastx [get-xoffset 1]] < 0} {
	    abc go $s
	    return
	}
	lappend xtb $lastx
	if {$yl > $ylower} {
	    set ylower $yl
	    set xl $nnotes
	}
	if {$yh < $yhigher} {
	    set yhigher $yh
	    set xh $nnotes
	}
	set nfl [flag-get [lindex $sym2 1]]
	lappend fltb $nfl
	if {$nfl > $maxfl} {
	    set maxfl $nfl
	}
	if {$nfl < $minfl} {
	    set minfl $nfl
	}
	incr sigma $yl
	incr sigma $yh
	incr nnotes
	if {[abc get flags] & $flags(word_s)} break
    }
    set stem_up [expr {$sigma / ($nnotes * 2) > 17 * 3 + $yoffset}]
    if {$stem_up} {
	set x [lindex $xtb $xh]
	set y $yhigher
	set y [expr {$y - 14 - 2 * $maxfl}]
	set a [expr {double(([lindex $yhigh 0] - $yh)) / (2. * ($x1 - $lastx))}]
	if {$a * $a < 0.01} {
	    set a 0
	}
	incr x1 6
	incr lastx 6
	set b [expr {$y - $a * $x}]
	foreach x $xtb y $ylow {
	    incr x 6
	    .f.c create line $x $y $x [expr {$a * $x + $b}] \
		-tag $stag
	}
    } else {
	set x [lindex $xtb $xl]
	set y $ylower
	set y [expr {$y + 17 + 2 * $maxfl}]
	set a [expr {double(([lindex $ylow 0] - $yl)) / (2. * ($x1 - $lastx))}]
	if {$a * $a < 0.01} {
	    set a 0
	}
	set b [expr {$y - $a * $x}]
	foreach x $xtb y $yhigh {
	    incr y
	    .f.c create line $x $y $x [expr {$a * $x + $b}] \
		-tag $stag
	}
    }
    # whole beam at 1st level
    beam-draw-one $lastx $x1 $a $b $stag
    for {set f 2} {$f <= $maxfl} {incr f} {
	set b [expr {$b + ($stem_up ? 3 : -3)}]
	if {$minfl >= $f} {
	    beam-draw-one $lastx $x1 $a $b $stag
	} else {
	    set inbeam 0
	    set n 1
	    foreach fl $fltb x $xtb {
		if {!$inbeam && $fl >= $f} {
		    set x1 $x
		    incr inbeam
		}
		if {$inbeam && ($fl < $f || $n == $nnotes)} {
		    if {$fl >= $f} {
			set x2 $x
		    }
		    if {$x1 == $x2} {
			if {$x2 == $x} {
			    if {$stem_up} {
				incr x1 6
			    } else {
				incr x1 7
			    }
			} else {
			    if {$stem_up} {
				incr x2 -7
			    } else {
				incr x2 -6
			    }
			}
		    }
		    if {$stem_up} {
			incr x1 6
			incr x2 6
		    }
		    beam-draw-one $x2 $x1 $a $b $stag
		    incr inbeam -1
		}
		set x2 $x
		incr n
	    }
	}
    }
    abc go $s
}

proc beam-draw-one {x1 x2 a b stag} {
# draw a single beam
#puts "x1: $x1 y1: [expr {$x1 * $a + $b}] x2: $x2 y2: [expr {$x2 * $a + $b}]"
#fixme: pb when small beams
    .f.c create line $x1 [expr {$x1 * $a + $b}] \
	$x2 [expr {$x2 * $a + $b}] \
	-width 2 -tag $stag
}

proc detail-display {xoffset yoffset stag} {
# draw the details of a note
    global flags font score voice
    set s [abc go]
    set sym [abc get]
#puts "detail-display $s ($sym)"
    set v [abc get voice]
    set deltav [expr {39 - 2 * $voice($v,clef)}]
    set sflags [abc get flags]
    # stem and flags
    set len [lindex $sym 1]
    if {[string compare [lindex $sym 0] note] == 0 && $len < 1536} {
	if {$sflags & $flags(word_e)} {
	    if {$sflags & $flags(word_s)} {
		foreach [list yy_up yy_down] [note-pits $sym] break
		set yy_up [expr {$deltav - $yy_up}]
		set yy_down [expr {$deltav - $yy_down}]
		set stem_up [expr {($yy_up + $yy_down) / 2  > 17}]
		set nflags [lindex [note-identify $len] 2]
		stem-draw $xoffset $yoffset $stag $stem_up $yy_down $yy_up $nflags
	    } else {
		beam-draw $xoffset $yoffset $stag
	    }
	}
    }
    # n-plet end
    if {$sflags & $flags(nplet_e)} {
	set s1 {}
	for {} {1} {} {
	    abc go prev
	    switch [lindex [abc get] 0] {
		EOT break
		note - rest {
		    if {[abc get flags] & $flags(nplet_s)} {
			set s2 [abc go]
		    }
		}
		tuplet {
		    set s1 [abc go]
		    break
		}
	    }
	}
	if {[string length $s1] != 0} {
	    if {[set t [.f.c find withtag $s2]] != {}} {
		set x1 [lindex [.f.c coords [lindex $t 0]] 0]
		set x4 [expr {$xoffset + 4}]
		set y [expr {$yoffset + 30}]
#		.f.c create line $x1 $y $x4 $y -tag $stag
		slur-draw $x1 $y $x4 $y $stag
		set x1 [expr {($x1 + $x4) * 0.5}]
		set y [expr {$y - 9}]
#		foreach [list d p q r] [abc get] break
#		.f.c create text $x1 $y \
#			-text "$p:$q:$r" -font "Courier 8" -tag $stag
		.f.c create text $x1 $y \
			-text [lindex [abc get] 1] -font $font(f6s) -tag $stag
	    }
	}
	abc go $s
    }
    # starting slur
    if {$sflags & $flags(slur_s)} {
	for {} {1} {} {
	    set s2 [abc go next]
	    if {[string compare [lindex [abc get] 0] EOT] == 0} break
	    if {[abc get flags] & $flags(slur_e)} break
	}
	set x1 [expr {$xoffset + 8}]
	set y1 [slur-y $sym $v $yoffset]
	if {[llength [set t [.f.c find withtag $s2]]] != 0} {
	    # the note ending the slur is displayed
	    set x4 [expr {[lindex [.f.c coords [lindex $t 0]] 0] - 1}]
	    set y4 [slur-y [abc get] $v $yoffset]
#	 else 
#	    set x4 [expr {$score(width) - 20}]
#	    set y4 $y1
#	
	set ntag [linsert $stag end $s2]
	slur-draw $x1 $y1 $x4 $y4 $ntag
      }
	abc go $s
    }
    # ending slur
    if {$sflags & $flags(slur_e)} {
	for {} {1} {} {
	    set s2 [abc go prev]
	    if {[string compare [lindex [abc get] 0] EOT] == 0} {
		abc go next
		abc go next
		set s2 [abc go next]
		break
	    }
	    if {[abc get flags] & $flags(slur_s)} break
	}
	set x4 [expr {$xoffset - 1}]
	set y4 [slur-y $sym $v $yoffset]
	if {[llength [set t [.f.c find withtag $s2]]] != 0} {
	    # the note starting the slur is displayed
	    set x1 [expr {[lindex [.f.c coords [lindex $t 0]] 0] + 8}]
	    set y1 [slur-y [abc get] $v $yoffset]
	} else {
	    set x1 40
	    set y1 $y4
	}
	set ntag [linsert $stag end $s2]
	slur-draw $x1 $y1 $x4 $y4 $ntag
	abc go $s
    }
    # ending ties
#puts stdout "s: $s [format 0x%04x $sflags]"
    if {$sflags & $flags(tie_e)} {
#puts stdout {tie_e}
	for {} {1} {} {
	    set s2 [abc go prev]
	    if {[string compare [lindex [abc get] 0] EOT] == 0} {
		abc go next
		abc go next
		set s2 [abc go next]
		break
	    }
#puts stdout "s2: $s2 [format 0x%04x [abc get flags]]"
	    if {[abc get flags] & $flags(tie_s)} break
	}
	set x4 [expr {$xoffset - 1}]
	set ntag $stag
	if {[llength [set t [.f.c find withtag $s2]]] != 0} {
	    # the note starting the tie is displayed
	    set x1 [expr {[lindex [.f.c coords [lindex $t 0]] 0] + 8}]
#	    lappend ntag $s2
	} else {
	    set x1 40
	}
	set i 2		;# skip 'note' <length>
	set sym2 [abc get]
	foreach t [abc get ties] {
	    if {$t} {
		set pitch [lindex $sym2 $i]
		set y1 [expr {($deltav - $pitch) * 3 + $yoffset - 2}]
#puts stdout "end tie ($ntag) $x1 - $x4"
		slur-draw $x1 $y1 $x4 $y1 $ntag
	    }
	    incr i 2
	}
	abc go $s
    }
    # guitar chord
    set d [abc get gchord]
    if {[string length $d] != 0} {
	.f.c create text [expr {$xoffset + 4}] [expr {$yoffset + 30}] \
		-text $d \
		-anchor c -font $font(f6) -tag $stag
    }
    # lyrics
    set d [abc get lyric]
    if {[llength $d] != 0} {
	set x [expr {$xoffset - 2}]
	set y [expr {$yoffset + 80}]
	set w 0
	foreach t $d {
	    .f.c create text $x $y \
		-text $t \
		-anchor w -font $font(t10) -tag $stag
	    set l [string length $t]
	    if {$l > $w} {
		set w $l
	    }
	    incr y 8
	}
	return [expr {$w * 5}]
    }
    return 0
}

proc detail-edit {} {
# edit the symbol details
    global opt
    if {!$opt(detail)} detail-toggle
    .detail.gche selection range 0 end
    focus .detail.gche
}

proc detail-toggle {{toggle 1}} {
# toggle displaying the symbol details
    global opt tune
    set tune(pref) 1
    if {$toggle} {
	set opt(detail) [expr {!$opt(detail)}]
    }
    if {$opt(detail)} {
	pack .detail -after .top2 -side top -anchor w -fill x
	detail-var
    } else {
	pack forget .detail
    }
    set s [abc go]
    abc go $tune(startsym)
    redisplay 1
    abc go $s
    cursor-display
#    focus .f.c
}

proc detail-var {} {
# set the detail variables
    global detail opt
    if {!$opt(detail)} return
    set detail(gchord) [abc get gchord]
    set detail(deco) [abc get deco]
}

proc flag-get {len} {
# get the number of note flags
    if {$len >= 192} {
	return 1
    }
    if {$len >= 96} {
	return 2
    }
    if {$len >= 48} {
	return 3
    }
    return 4
}

proc slur-draw {x1 y1 x4 y4 tag} {
# draw a slur/tie
    # compute the intermediate coordinates
    set deltax [expr {$x4 - $x1}]
    set deltay [expr {$y4 - $y1}]
    set x2 [expr {$x1 + $deltax / 4}]
    set x3 [expr {$x1 + ($deltax * 3) / 4}]
    set y2 [expr {$y1 + $deltay / 4 - 4}]
    set y3 [expr {$y1 + ($deltay * 3) / 4 - 4}]
    if {$deltax > 50} {
	set y2 [expr {$y2 - 4}]
	set y3 [expr {$y3 - 4}]
    }
    .f.c create line \
	$x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 \
	-smooth 1 -tag $tag
}

proc slur-y {sym v yoffset} {
# compute the vertical position of a slur
    global voice
    set min 39
    if {[string compare [lindex $sym 0] note] != 0} {
	set y $min
    } else {
	set deltav [expr {39 - 2 * $voice($v,clef)}]
	foreach [list y ylow] [note-pits $sym] break
	set y [expr {($deltav - $y) * 3}]
	if {$y > $min} {
	    set y $min
	}
    }
    return [expr {$y + $yoffset - 4}]
}
