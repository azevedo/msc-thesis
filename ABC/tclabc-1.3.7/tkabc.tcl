#
# Graphical interface to tclabc.
#
# This Tcl/Tk script shall be called from the shell script 'tkabc',
# which is generated from 'tkabc.in' by 'configure' and installed
# by 'make install'.
# It may be run directly from the generation directory calling
# 'wish tkabc.tcl [file_name]'.
#
# Copyright (C) 1999-2011, Jean-François Moine.
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
#fixme: keep the key sig change for voices != 1st.
#

proc abc-load {fn} {
# load an ABC file
    global lang opt tune
    if {[catch {open $fn r} fd]} {
	tk_messageBox -type ok -icon error -title $lang(bad_file) \
		-message [format $lang(read_err) $fn]
	return
    }
    # load the abc file
    abc load [read $fd [file size $fn]]
    close $fd
    set opt(lastdir) [file dirname $fn]
    # update the menu
    set tune(fname) $fn
    .title.tune.menu delete 0 end
    set i 0
    foreach title [abc tune] {
	if {$i % 24 == 0 && $i != 0} {
		.title.tune.menu add command -label $title \
		    -command [list abc-select $i] -columnbreak 1
	} else {
		.title.tune.menu add command -label $title \
		    -command [list abc-select $i]
	}
	incr i
    }
    # display the first tune
    set tune(nvoice) [abc voice]
    tune-display
}

proc abc-select {n} {
# select a tune from an ABC file
    global opt tune
    # remove the old tune
    for {set v 0} {$v <= $tune(nvoice)} {incr v} {
	.f.c delete v$v
    }
    # load the tune
    abc tune $n
    set tune(nvoice) [abc voice]
    set color [.title.tune.menu entrycget [abc get tune] -background]
    if {[string length $color] == 0} {
	set color $opt(nochg_bg)
    }
    .title.tune configure -background $color
    tune-display
}

# balloon definitions
proc balloon {w help} {
    bind $w <Any-Enter> "after 1000 [list balloon:show %W [list $help]]"
    bind $w <Any-Leave> {destroy .balloon}
    bind $w <Any-Button> {destroy .balloon}
}

proc balloon:show {w arg} {
    global font
    if {[eval winfo containing [winfo pointerxy .]] != $w} return
    set top .balloon
    catch {destroy $top}
    toplevel $top -bd 1 -bg black
    wm overrideredirect $top 1
    pack [label $top.txt \
	-font $font(f6) -text $arg -pady 0 \
	-bg LightYellow]
    set wmx [expr {[winfo rootx $w] + [winfo width $w] / 2}]
    set wmy [expr {[winfo rooty $w] + [winfo height $w] + 4}]
    set reqw [winfo reqwidth $top.txt]
    set reqh [winfo reqheight $top.txt]
    if {$reqw + $wmx >= [winfo screenwidth $top]} {
	set wmx [expr {[winfo screenwidth $top] - $reqw - 4}]
    }
    if {$reqh + $wmy >= [winfo screenheight $top]} {
	set wmy [expr {[winfo rooty $w] - $reqh - 1}]
    }
    wm geometry $top ${reqw}x${reqh}+$wmx+$wmy
    raise $top
}

proc bar-draw {btype xoffset yoffset stag} {
# draw a measure bar
    global opt
    # treat the special bars - !! see abcparse.h for btype values !!
    set invis 0
    set dash 0
    switch $btype {
	2 - 3 - 35 {
	    # [ - ] - []
	    set invis 1
	    set btype 1
	}
	4 {
	    # :
	    set dash 1
	    set btype 1
	}
	20 {
	    # |:
	    set btype 0x214
	}
	65 {
	    # :|
	    set btype 0x413
	}
	68 {
	    # ::
	    set btype 0x4324
	}
    }
    set b $btype
    set btype 0
    # invert the bar elements
    while {$b != 0} {
	set btype [expr {($btype << 4) | ($b & 0x0f)}]
	set b [expr {$b >> 4}]
    }
    # draw the bar elements
    incr yoffset 40
    set yoffset2 [expr {$yoffset + 24}]
    while {$btype != 0} {
	set b [expr {$btype & 0x0f}]
	switch $b {
	    1 {
		set l [.f.c create line $xoffset $yoffset \
			$xoffset $yoffset2 \
			-tag $stag]
		if {$invis} {
		    .f.c itemconfigure $l -fill $opt(bg)
		}
		if {$dash} {
		    .f.c itemconfigure $l -fill DarkSeaGreen2
		}
	    }
	    2 - 3 {
		.f.c create rectangle $xoffset $yoffset \
			[expr {$xoffset + 1}] $yoffset2 \
			-tag $stag
		incr xoffset 1
	    }
	    4 {
		.f.c create rectangle $xoffset [expr {$yoffset + 8}] \
			[expr {$xoffset + 1}] [expr {$yoffset + 9}] \
			-tag $stag
		.f.c create rectangle $xoffset [expr {$yoffset + 15}] \
			[expr {$xoffset + 1}] [expr {$yoffset + 16}] \
			-tag $stag
	    }
	}
	incr xoffset 3
	set btype [expr {$btype >> 4}]
    }
    return
}

proc canvas-redraw {} {
# redraw the first elements of the canvas
    global currect opt score tune voicerect
    # clear the canvas
    sel-clear
    .f.c delete all
    # set the canvas size
    canvas-size
    # create the voice rectangle, the staves and the cursor rectangle
    set voicerect [.f.c create rectangle 5 10 \
	$score(width) [expr {$score(height) - 10}] \
	-fill $opt(v_color) -outline {}]
    set currect [.f.c create rectangle 1 1 2 2 \
	-fill LightYellow -outline red]
    set yoffset 40
    for {set v 0} {$v <= $tune(nvoice)} {incr v} {
	set y $yoffset
	for {set i 0} {$i < 5} {incr i} {
	    .f.c create line 10 $y [expr {$score(width) - 8}] $y
	    incr y 6
	}
	incr yoffset $score(height)
    }
}

proc canvas-size {} {
# set the canvas size
    global score tune
    set h [expr {$score(height) * ($tune(nvoice) + 1)}]
    .f.c configure -scrollregion [list 0 0 $score(width) $h]
}

proc cursor-display {} {
# delay a little before displaying the cursor
    global tune
    if {[string length $tune(cursdisp)] != 0} {
	after cancel $tune(cursdisp)
    }
    set tune(cursdisp) [after 100 cursor-display-d]
}

proc chord-coord {s} {
#fixme: KO when inside cursor-display-d
    global tune
    # get all notes
    set notes {}
    foreach item [.f.c find withtag $s] {
	# a note has an image name starting with 'note'
	if {[catch {.f.c itemcget $item -image} res]
	    || [string first note $res] != 0} continue
	lappend notes $item
    }
    if {$tune(chord)} {
	return [.f.c bbox [lindex $notes [expr {$tune(chordx) / 2 - 1}]]]
    }
    return [eval .f.c bbox $notes]
}

proc cursor-display-d {} {
# display the cursor on the current symbol
    global currect midistep opt score tune voice
    set tune(cursdisp) {}
    # if some redisplay running, abort
    if {$score(update)} return
    set s [abc go]
    # for symbol display (see below)
    set sym [abc get]
    set tune(cursym) [join $sym]
    set tune(time) [abc get time]
    # display the symbol if not done
    if {[llength [.f.c find withtag $s]] == 0} {
	set mintime [expr {$tune(time) - $tune(screentime) / 2}]
	while {1} {
	    abc go prev
	    if {[abc get time] < $mintime
		|| [string compare [lindex [abc get] 0] EOT] == 0} {
		    break
	    }
	}
	abc go next
	redisplay 1
	abc go $s
	if {[llength [.f.c find withtag $s]] == 0} {
	    #fixme: advance again?
	    puts "cursor-display: $s - sym: $tune(cursym) not displayed!"
	    return
	}
    }
    # display the note parameters
    if {[string compare [lindex $sym 0] note] == 0
	|| [string compare [lindex $sym 0] grace] == 0} {
	set len [lindex $sym 1]
	if {$len % 15 == 0} {
	    set len [expr {$len / 15 * 8}]
	} elseif {$len % 7 == 0} {
	    set len [expr {$len / 7 * 4}]
	} elseif {$len % 9 == 0} {
	    set len [expr {$len / 3 * 2}]
	}
	set tune(len) $len
	if {$len >= 6144} {
	    set len 3072
	} elseif {$len < 24} {
	    set len 24
	}
	.top2.nl configure -image notes$len
	# if chord editing
	if {$tune(chord)} {
	    if {$tune(chordx) >= [llength $sym]} {
		set tune(chordx) [expr {[llength $sym] - 2}]
	    }
	    foreach [list x1 y1 x2 y2] [chord-coord $s] break
	} else {
	    set tune(chordx) 2
	    foreach [list x1 y1 x2 y2] [chord-coord $s] break
	    incr x1 -1
	    incr y1 -9
	    incr x2
	    incr y2 9
	}
	set acc [lindex $sym [expr {$tune(chordx) + 1}]]
	if {$acc > 5} {
	    if {[llength [info commands acc$acc]] == 0} {
		set acc [expr {$acc & 0x07}]
	    }
	}
	set tune(acc) $acc
	.top2.acc configure -image acc$acc
	.top2.chord configure -state normal
	if {$opt(playnote) && ![info exists midistep]} {
	    abc play note
	}
    } else {
	foreach [list x1 y1 x2 y2] [.f.c bbox $s] break
	.top2.chord configure -state disabled
#	set tune(chordx) 2
    }
    # set the detail variables
    if {$opt(detail)} detail-var
    # display the cursor itself
    incr x1 -2
    incr y1 -2
    incr x2
    incr y2
    .f.c coords $currect $x1 $y1 $x2 $y2
    if {$tune(rmmsg)} {
	set tune(message) {}
	pack forget .top2.mini
	set tune(rmmsg) 0
    } elseif {[string length tune(message)] != 0} {
	set tune(rmmsg) 1
    }
}

proc cursor-move {d} {
# move the cursor in the voice
    switch -- $d {
	Home {
	    set v [abc get voice]
	    abc rewind
	    # skip the clef, keysig and timesig
	    abc go $v
	    abc go next
	    abc go next
	    abc go next
	}
	End {
	    set v [abc get voice]
	    abc rewind
	    abc go $v
	    abc go prev
	}
	1 {
	    abc go next
	}
	-1 {
	    abc go prev
	}
	1b {
	    for {set i 0} {$i < 32} {incr i} {
		switch [lindex [abc get] 0] {
		    bar {
			if {[lindex [abc get] 1] != 0} {
			    abc go next
			    break
			}
		    }
		    EOT break
		}
		abc go next
	    }
	}
	-1b {
	    abc go prev
	    if {[lindex [abc get] 0] == {bar}} {
		abc go prev
	    }
	    for {set i 0} {$i < 32} {incr i} {
		switch [lindex [abc get] 0] {
		    bar {
			if {[lindex [abc get] 1] != 0} {
			    abc go next
			    break
			}
		    }
		    EOT {
			cursor-move Home
			return
		    }
		}
		abc go prev
	    }
	}
    }
    cursor-display
}

proc full-toggle {} {
# toggle displaying the window menus
    global opt
    if {[llength [pack slaves .]] < 3} {
	pack .title .top .top2 -after .mb -side top -pady 0 -fill x
	if {$opt(detail)} {
	    pack .detail -after .top2 -side top -anchor w -fill x
	}
	pack .play -side bottom -pady 0 -fill x
    } else {
	pack forget .title .top .top2 .play
	if {$opt(detail)} {
	    pack forget .detail
	}
    }
}

proc get-xoffset {{nowarning 0}} {
# get the x offset of the current symbol
    global tune
    if {[llength [set t [.f.c find withtag [abc go]]]] == 0} {
	if {$nowarning} {
	    return -1
	}
	puts "get-xoffset: [abc go] ([abc get]) not displayed!"
#	set tune(message) "get-xoffset: symbol [abc go] ([abc get]) not displayed!"
	stack-dump
	return 100
    }
#    return [lindex [.f.c bbox [lindex $t 0]] 0]
    return [expr {int([lindex [.f.c coords [lindex $t 0]] 0])}]
}

proc hmove {args} {
# move in the tune from the horizontal scrollbar
    global score tune
    set x [lindex $args 1]
    set v [abc get voice]
    switch [lindex $args 0] {
	moveto {
	    set time [expr {$tune(totaltime) * $x}]
	}
	scroll {
	    switch [lindex $args 2] {
		units {
		    cursor-move ${x}b
		    return
		}
		pages {
		    abc go $tune(startsym)
		    set time [abc get time]
		    if {$x > 0} {
			set time [expr {$time + $tune(screentime) - 384}]
			if {$time >= $tune(totaltime)} {
				abc rewind
				abc go $v
				abc go prev
				cursor-display
				return
			}
			while {1} {
			    abc go next
			    if {[abc get time] > $time} {
				abc go prev
				redisplay 1
				abc go $tune(startsym)
				abc go $v
				break
			    }
			    if {[string compare [lindex [abc get] 0] EOT] == 0} {
				break
			    }
			}
			cursor-display
			return
		    }
		    set time [expr {$time - $tune(screentime) + 384}]
		}
	    }
	}
    }
    if {$time < 1} {
	cursor-move Home
	return
    }
    if {$time > $tune(time)} {
	if {$time - $tune(time) > $tune(totaltime) - $time} {
	    abc rewind
	    abc go $v
	    abc go prev
	    abc go prev
	    set forw 0
	} else {
	    set forw 1
	}
    } else {
	if {$time > $tune(time) / 2} {
	    abc rewind
	    abc go $v
	    set forw 1
	} else {
	    set forw 0
	}
    }
    if {$forw} {
	while {1} {
	    abc go next
	    if {[abc get time] > $time
		|| [string compare [lindex [abc get] 0] EOT] == 0} {
		    break
	    }
	}
	abc go prev
    } else {
	while {1} {
	    abc go prev
	    if {[abc get time] < $time
		|| [string compare [lindex [abc get] 0] EOT] == 0} {
		    break
	    }
	}
	abc go next
    }
    set s [abc go]
    redisplay 1
    abc go $s
    if {[string compare [lindex $args 0] moveto] == 0} {
	# put the cursor in the middle of the screen
	while {1} {
	    set s [abc go next]
	    set coords [.f.c coords $s]
	    if {[llength $coords] == 0
		|| [lindex $coords 0] > $score(width) / 2} {
		abc go prev
		break
	    }
	    if {[string compare [lindex [abc get] 0] EOT] == 0} {
		break
	    }
	}
    } else {
	while {1} {
	    set s [abc go next]
	    if {[llength [.f.c find withtag $s]] == 0} {
		abc go prev
		break
	    }
	    if {[string compare [lindex [abc get] 0] EOT] == 0} {
		break
	    }
	}
    }
    cursor-display
}

proc jump {} {
# get the measure number
    global tune
    set tune(message) {Measure #:}
    set tune(mini-buffer) {}
    pack .top2.mini -side left
    focus .top2.mini
}

proc jump-exec {abrt} {
# go to a specific measure
    global tune
    pack forget .top2.mini
    set tune(message) {}
    focus .f.c
    set m $tune(mini-buffer)
    if {!$abrt && [string length $m] != 0} {
	if {$m <= 1} {
	    cursor-move Home
	    return
	}
	while {1} {
	    set type [lindex [abc get] 0]
	    switch $type {
		bar {
		    break
		}
		EOT break
	    }
	    abc go next
	}
	set n 1
	set dir next
	if {[string compare $type bar] == 0} {
	    foreach [list type btype repeat n] [abc get] break
	    if {$m < $n} {
		set dir prev
	    }
	} else {
	    abc go next
	}
	while {1} {
	    switch [lindex [abc get] 0] {
		bar {
		    foreach [list type btype repeat n] [abc get] break
		    if {$m == $n} {
			abc go next
			break
		    }
		}
		EOT break
	    }
	    abc go $dir
	}
    }
    cursor-display
}

proc mark-set {} {
# set the mark on the current symbol
    global selection tune
    set selection(mark) [abc go]
    set tune(message) {[mark set]}
}
    
proc midi-load {fn} {
# load a MIDI file
    global opt score tune
    if {$tune(nvoice) >= 0} {
	# remove the old tune
	for {set v 0} {$v <= $tune(nvoice)} {incr v} {
	    .f.c delete v$v
	}
    }
    abc midiload $fn
    set opt(lastdir) [file dirname $fn]
    # update the menu
    set tune(fname) $fn
    .title.tune.menu delete 0 end
    .title.tune.menu add command -label [abc tune] \
	-command [list abc-select 0]
    # display the first tune
    set tune(nvoice) [abc voice]
    tune-display
}

proc midi-open {out {toggle 1}} {
# open or close the MIDI ports
    global opt tune
    if {$out} {
	if {$tune(midiout)} {
	    if {[string length $opt(midiout)] == 0
		|| [catch {abc midiout $opt(midiout)}]} {
		set tune(midiout) 0
	    }
	} else {
	    abc midiout {}
	}
    } else {
	if {$tune(midiin)} {
	    if {[string length $opt(midiin)] == 0
		|| [catch {abc midiin $opt(midiin)}]} {
		set tune(midiin) 0
	    }
	} else {
	    abc midiin {}
	}
    }
}

proc note-identify {len} {
# identify a note
# return its image, number of dots, number of flags
    set img 0
    set dots 0
    set flags 0
    set base 384
    if {$len >= 384} {
	if {$len >= 768} {
	    if {$len >= 1536} {
		if {$len >= 3072} {
		    set img 3
		    set base 3072
		} else {
		    set img 2
		    set base 1536
		}
	    } else {
		set img 1
		set base 768
	    }
	}
    } elseif {$len >= 192} {
	set base 192
	set flags 1
    } elseif {$len >= 96} {
	set base 96
	set flags 2
    } elseif {$len >= 48} {
	set base 48
	set flags 3
    } else {
	set base 24
	set flags 4
    }
    if {2 * $len == 3 * $base} {
	set dots 1
    } elseif {4 * $len == 7 * $base} {
	set dots 2
    } elseif {8 * $len == 15 * $base} {
	set dots 3
    }
    return [list $img $dots $flags]
}

proc note-draw {sym v xoffset yoffset stag} {
# draw a note (or chord)
    global opt tune voice
    # get upper and lower note pitches
    foreach [list yy_up yy_down] [note-pits $sym] break
    set deltav [expr {39 - 2 * $voice($v,clef)}]
    set yy_up [expr {$deltav - $yy_up}]
    set yy_down [expr {$deltav - $yy_down}]
    if {[string compare [lindex $sym 0] grace] == 0} {
	# grace note
	foreach [list pitch acc] [lrange $sym 2 end] {
	    set yy [expr {$deltav - $pitch}]
	    set yoffset2 [expr {$yy * 3 + $yoffset}]
	    # note
#fixme: should handle the grace note length
	    .f.c create image $xoffset [expr {$yoffset2 - 1}] \
		-image notes192 -anchor w -tag $stag
	    # accidentals
	    if {$acc != 0} {
		if {$acc > 5} {
		    if {[llength [info commands acc$acc]] == 0} {
			set acc [expr {$acc & 0x07}]
		    }
		}
		.f.c create image [expr {$xoffset - 1}] [expr {$yoffset2 + 1}] \
		    -image acc$acc -anchor e -tag $stag
	    }
	}
	# extra lines
	for {set i [expr {$yy_up / 2}]} {$i < 6} {incr i} {
	    set yoffset2 [expr {$i * 6 + $yoffset + 5}]
	    .f.c create line [expr {$xoffset - 2}] $yoffset2 \
		[expr {$xoffset + 8}] $yoffset2 -tag $stag
	}
	for {set i [expr {($yy_down - 1) / 2}]} {$i > 10} {incr i -1} {
	    set yoffset2 [expr {$i * 6 + $yoffset + 5}]
	    .f.c create line [expr {$xoffset - 2}] $yoffset2 \
		[expr {$xoffset + 8}] $yoffset2 -tag $stag
	}
#fixme: bad slurs, ...
#	if {$opt(detail)} {
#	    return [detail-display $xoffset $yoffset $stag]
#	}
	return
    }
    # normal note
    set len [lindex $sym 1]
    foreach [list img dots nflags] [note-identify $len] break
    set stem_up [expr {($yy_up + $yy_down) / 2  > 17}]
    # loop on each note of the chord
    foreach [list pitch acc] [lrange $sym 2 end] {
	#fixme: clef may have changed in the line
	set yy [expr {$deltav - $pitch}]
	set yoffset2 [expr {$yy * 3 + $yoffset}]
	#fixme: shift when notes are too close
	# note
	.f.c create image $xoffset [expr {$yoffset2 + 2}] \
	    -image note$img -anchor w -tag $stag
	# dots
	if {$dots > 0} {
	    #fixme: put on the upper line on the staff
	    for {set i 0} {$i < $dots} {incr i} {
		.f.c create rectangle [expr {$xoffset + 10 + 5 * $i}] $yoffset2 \
		    [expr {$xoffset + 11 + 5 * $i}] [expr {$yoffset2 + 1}] \
		    -tag $stag
	    }
	}
	# accidentals
	if {$acc != 0} {
	    if {$acc > 5} {
		if {[llength [info commands acc$acc]] == 0} {
		    set acc [expr {$acc & 0x07}]
		}
	    }
	    .f.c create image [expr {$xoffset - 1}] [expr {$yoffset2 + 1}] \
		-image acc$acc -anchor e -tag $stag
	}
    }
    # extra lines
    for {set i [expr {$yy_up / 2}]} {$i < 6} {incr i} {
	set yoffset2 [expr {$i * 6 + $yoffset + 5}]
	.f.c create line [expr {$xoffset - 3}] $yoffset2 \
	    [expr {$xoffset + 9}] $yoffset2 -tag $stag
    }
    for {set i [expr {($yy_down - 1) / 2}]} {$i > 10} {incr i -1} {
	set yoffset2 [expr {$i * 6 + $yoffset + 5}]
	.f.c create line [expr {$xoffset - 3}] $yoffset2 \
	    [expr {$xoffset + 9}] $yoffset2 -tag $stag
    }
    # stems and flags
    if {$opt(detail)} {
	return [detail-display $xoffset $yoffset $stag]
    }
    if {$len < 1536} {
	stem-draw $xoffset $yoffset $stag $stem_up $yy_down $yy_up $nflags
    }
    return 0
}

proc note-pits {sym} {
# return the max and min pitches
    set up [lindex $sym 2]
    set down $up
    foreach [list pitch acc] [lrange $sym 4 end] {
	if {$pitch > $up} {
	    set up $pitch
	}
	if {$pitch < $down} {
	    set down $pitch
	}
    }
    return [list $up $down]
}

proc quit {} {
# exit the program
    global tune
    # stop playing
    if {[string compare [abc play which] none] != 0} {
	abc play stop
    }
    # ask and save the tune if modified
    if {$tune(changed)} {
	if {[save-check]} return
    }
    # save the preferences
    if {$tune(pref)} {
	pref-save
    }
    exit
}

proc redisplay {all} {
# redisplay all the canvas or only the right side
    global currect flags selection score voice tune
    # if some redisplay running, abort
    if {$score(update)} return
    if {!$all} {
	set s [abc go]
	set v [abc get voice]
	if {[lsearch -exact $voice($v,sig) $s] >= 0} {
	    # the symbol is a clef, key or time: redisplay all
	    set all 1
	}
    }
    if {$all} {
	# full redisplay
#puts "full redisplay [abc go] ([abc get])"
	.f.c configure -cursor watch
#	sel-clear
	update idle
	# set the starting symbol
	set s [abc go 0]
	# get the voice signatures
	for {set v 0} {$v <= $tune(nvoice)} {incr v} {
	    .f.c delete v$v
	    abc go $v
	    # (staff @clef @key @time)
	    set voice($v,sig) [lrange [abc get staff] 1 end]
	}
	# display the beginning of the staves
	set xoffset 14
	set w2 $xoffset
	for {set i 0} {$i < [llength $voice(0,sig)]} {incr i} {
	    for {set v 0} {$v <= $tune(nvoice)} {incr v} {
		set s2 [abc go [lindex $voice($v,sig) $i]]
		sym-draw $xoffset $v
		set w [lindex [.f.c bbox $s2] 2]
		if {$w > $w2} {
		    set w2 $w
		}
	    }
	    incr w2 6
	    set xoffset $w2
	}
	# skip the signatures and set again the starting symbol
	abc go $s
	while {1} {
	    set v [abc get voice]
	    if {[lsearch -exact $voice($v,sig) $s] < 0} break
	    set s [abc go next time]
	}
	set tune(startsym) $s
    } else {
	# partial redisplay - remove all symbols on the right
	set xoffset [get-xoffset]
#KO when beams
#	set xtmp [lindex [.f.c bbox [abc go]] 0]
#fixme:test
	set xtmp $xoffset
	incr xtmp -6
#puts "redisplay [abc go] ([abc get]) - x:$xtmp"
	set ls {}
	foreach s \
		[.f.c find enclosed [expr {$xtmp - 2}] 0 \
		    $score(width) \
		    [expr {$score(height) * ($tune(nvoice) + 1)}]] {
	    if {$s == $currect} continue
	    set t [lindex [.f.c gettags $s] 1]
	    if {[lsearch -exact $ls $t] < 0} {
		lappend ls $t
	    }
	}
	foreach s $ls {
#puts "delete $s bbox: [.f.c bbox $s]"
	    .f.c delete $s
	}
	unset ls xtmp
    }
    # go to the first voice which has some symbol at this time
    set time [abc get time]
    for {set v 0} {$v <= $tune(nvoice)} {incr v} {
	abc go $v
	if {[string compare [abc get time] $time] == 0} break
    }
    # display the symbols
    set starttime $time
    set w2 0
    while {1} {
#puts "sym-draw [abc go] ([abc get])"
	sym-draw $xoffset
	set w [lindex [.f.c bbox [abc go]] 2]
	if {$w > $w2} {
	    set w2 $w
	}
	# go up to the last symbol
	if {[string length [abc go next time]] == 0} break
	set newtime [abc get time]
	if {[string compare $newtime $time] != 0} {
	    # fixme: have a C function...
	    set len [expr {int($newtime) - int($time)}]
	    set w $xoffset
	    if {$len != 0} {
		if {$len >= 384} {
		    if {$len >= 768} {
			incr w 24
		    } else {
			incr w 20
		    }
		} else {
		    if {$len >= 192} {
			incr w 16
		    } elseif {$len >= 96} {
			incr w 14
		    } else {
			incr w 12
		    }
		}
		if {$w > $w2} {
			set w2 $w
		}
	    }
	    incr w2 6
	    set xoffset $w2
	    # if out of the canvas, stop
	    if {$xoffset > $score(width) - 20} {
		if {$all} {
		    # full redisplay: compute the screen time (for cursor-display)
		    set tune(screentime) [expr {$newtime - $starttime}]
		}
		break
	    }
	    set time $newtime
	}
    }
    .f.c configure -cursor top_left_arrow
    # update the horizontal scrollbar
    if {$all} {
	# fixme: is there a quicker way?
	abc rewind
	abc go prev
	abc go prev
	set tune(totaltime) [abc get time]
	if {$tune(totaltime) >= 1} {
	    .f.hs set [expr {$starttime / $tune(totaltime)}] \
		[expr {$time / $tune(totaltime)}]
	} else {
	    .f.hs set 0 1
	}
    }
    # update the selection rectangle
    if {$selection(rect) != 0} {
	sel-redisplay
    }
}

proc save-check {} {
# ask if the current tune shall be saved
# return cancel or not
    global lang tune
    switch [tk_messageBox -type yesnocancel  -title $lang(save_t) \
		-message [format $lang(save_m) $tune(fname)]] {
	yes {
	    tune-save
	    return 0
	}
	no {
	    change-set 0
	    return 0
	}
    }
    return 1
}

proc screen-refresh {} {
# refresh the screen
    global tune
    set s [abc go]
    abc go $tune(startsym)
    redisplay 1
    abc go $s
    cursor-display
}

proc screen-set {w h} {
# window configuration change
    global scrupd
    if {[info exists scrupd]} {
	after cancel $scrupd
    }
    set scrupd [after 100 [list screen-set-d $w $h]]
}

proc screen-set-d {w h} {
# delayed window configuration change
    global opt score scrupd tune
    unset scrupd
#    puts "w:$w h:$h"
    if {$opt(screen-w) == $w && $opt(screen-h) == $h} return
    set tune(pref) 1
    set opt(screen-h) $h
    if {$opt(screen-w) != $w} {
	set opt(screen-w) $w
	set score(width) [expr {$opt(screen-w) - 15}]
	canvas-redraw
	screen-refresh
    } else {
	canvas-size
    }
}

proc sel-clear {} {
# clear the selection
    global selection
    if {$selection(rect) != 0} {
	.f.c delete $selection(rect)
	set selection(rect) 0
    }
    set selection(syms) {}
    catch {unset selection(xy)}
}

proc stack-dump {} {
# dump the tcl stack (debug)
    puts {    ** stack dump **}
    for {set i 1} {$i < 5} {incr i} {
	if {[catch {uplevel $i info level 0} info]} break
	puts "  $info"
    }
}

proc stem-draw {xoffset yoffset stag stem_up yy_down yy_up nflags} {
# draw the stem and flags of a note
	set yoffs_up [expr {$yy_up * 3 + $yoffset}]
	set yoffs_down [expr {$yy_down * 3 + $yoffset}]
	if {$stem_up} {
	    incr xoffset 6
	    .f.c create line $xoffset $yoffs_down \
		    $xoffset [expr {$yoffs_up - 16}] \
		    -tag $stag
	    incr xoffset 3
	    for {set i 0} {$i < $nflags} {incr i} {
		.f.c create image $xoffset \
			[expr {$yoffs_up - 9 - $i * 5}] \
			-image flagu -tag $stag
	    }
	} else {
	    .f.c create line $xoffset [expr {$yoffs_up + 1}] \
		    $xoffset [expr {$yoffs_down + 19}] \
		    -tag $stag
	    incr xoffset 3
	    for {set i 0} {$i < $nflags} {incr i} {
		.f.c create image $xoffset \
			[expr {$yoffs_down + 13 + $i * 5}] \
			-image flagd -tag $stag
	    }
	}
}

proc set-clef {} {
    global voice
    set sym [abc get]
    set v [abc get voice]
    # "clef" type line
    set clef [lindex $sym 1]
    set line [lindex $sym 2]
    if {$clef == 3} {
	# change 'perc' to 'treble'
	set clef 0
    }
    set voice($v,clef) [expr {($clef - 1) * 2 + $line}]
}

proc sym-draw {xoffset {force_v -1}} {
# draw the current symbol
# return the width of the symbol (but 0 for note or rest)
    global flags opt tune score voice
    global font sfTab
    set s [abc go]
    set sym [abc get]
    if {$force_v >= 0} {
	set v $force_v
    } else {
	set v [abc get voice]
    }
    set stag [list v$v $s]
    set yoffset [expr {$score(height) * $v}]
    # symbol tag
    switch [lindex $sym 0] {
	note - grace {
	    note-draw $sym $v $xoffset $yoffset $stag
	}
	rest {
	    # "rest" len
	    set len [lindex $sym 1]
	    #fixme: have 'main' on the 1st rest only
	    set yoffset2 [expr {$yoffset + 52}]
	    if {$len == 0} {
		.f.c create rectangle $xoffset $yoffset2 \
		    [expr {$xoffset + 4}] [expr {$yoffset2 + 6}] \
		    -outline {} -fill green -tag $stag
	    } else {
		foreach [list img dots nflags] [note-identify $len] break
		if {$img == 0 && $nflags != 0} {
		    set img -$nflags
		}
		.f.c create image $xoffset $yoffset2 -image rest$img -tag $stag
		if {$dots > 0} {
		    for {set i 0} {$i < $dots} {incr i} {
			.f.c create rectangle [expr {$xoffset + 10 + 5 * $i}] \
			    $yoffset2 \
			    [expr {$xoffset + 11 + 5 * $i}] \
			    [expr {$yoffset2 + 1}] \
			    -tag $stag
		    }
		}
	    }
	    incr xoffset -12
	    if {$opt(detail)} {
		detail-display $xoffset $yoffset $stag
	    }
	}
	bar {
	    foreach [list type btype repeat number] $sym break
	    # display the bar
	    bar-draw $btype $xoffset $yoffset $stag
	    # display the bar number  (fixme: only on demand)
	    if {$v == 0 && $number % 5 == 0 && $btype != 0} {
		.f.c create text [expr {$xoffset - 2}] [expr {12 + $yoffset}] \
			-text $number \
			-anchor w -font $font(f6) -tag $stag
	    }
	    # display repeat
	    if {[string length $repeat] != 0} {
		.f.c create line $xoffset [expr {$yoffset + 22}] \
		    $xoffset [expr {$yoffset + 36}] \
		    -tag $stag
		.f.c create line $xoffset [expr {$yoffset + 22}] \
		    [expr {$xoffset + 50}] [expr {$yoffset + 22}] \
		    -tag $stag
		.f.c create text [expr {$xoffset + 4}] [expr {$yoffset + 30}] \
		    -text $repeat \
		    -anchor w -font $font(f6) -tag $stag
	    } else {
		# or gchord
		if {$opt(detail)} {
		    set d [abc get gchord]
		    if {[string length $d] != 0} {
			.f.c create text [expr {$xoffset + 4}] [expr {$yoffset + 30}] \
				-text $d \
				-anchor c -font $font(f6) -tag $stag
		    }
		}
	    }
	}
	clef {
	    # "clef" type line
	    set-clef
	    set clef [lindex $sym 1]
	    set line [lindex $sym 2]
	    set y [expr {40 + 25 - $line * 6 + $yoffset}]
	    .f.c create image $xoffset $y \
		    -image clef$clef -anchor w -tag $stag
	}
	time {
	    # "time" up low ?up low? ..
	    if {[llength $sym] == 1} {
		.f.c create rectangle $xoffset [expr {52 - 2 + $yoffset}] \
		    [expr {$xoffset + 4}] [expr {52 + 3 + $yoffset}] \
		    -outline {} -fill blue -tag $stag
	    } else {
		foreach {up low} [lrange $sym 1 end] {
		    set l [expr {[string length $up] * 10}]
		    if {[string length $low] == 0} {
			.f.c create text $xoffset [expr {40 + 12 + $yoffset}] \
			    -text $up -anchor w -font $font(f9) -tag $stag
		    } else {
			.f.c create text $xoffset [expr {40 + 6 + $yoffset}] \
			    -text $up -anchor w -font $font(f9) -tag $stag
			.f.c create text $xoffset [expr {40 + 18 +$yoffset}] \
			    -text $low -anchor w -font $font(f9) -tag $stag
			set l2 [expr {[string length $low] * 10}]
			if {$l2 > $l} {
			    set l $l2
			}
		    }
		    incr xoffset $l
		}
	    }
	}
	key {
	    # type number_of_sharps_flats
	    set sf [lindex $sym 1]
	    if {$sf == 0 || [string compare $sf "none"] == 0} {
		.f.c create rectangle $xoffset [expr {52 - 2 + $yoffset}] \
		    [expr {$xoffset + 4}] [expr {52 + 3 + $yoffset}] \
		    -outline {} -fill blue -tag $stag
	    } else {
		if {$sf > 0} {
		    set keytab $sfTab(sharp)
		    set key acc1
		} else {
		    set keytab $sfTab(flat)
		    set key acc3
		}
		set y $yoffset
		incr y [lindex [list 40 55 49 43 37 52 46] $voice($v,clef)]
		for {set i 0} {$i < [expr {abs($sf)}]} {incr i} {
		    .f.c create image [expr {$xoffset + $i * 6}] \
			[expr {$y + [lindex $keytab $i]}] \
			-image $key -anchor w -tag $stag
		}
	    }
	}
	tempo {
	    # "tempo" string1 len0 len1 len2 len3 speed string2
	    #fixme: have better display
	    set t "[lindex $sym 1] [lindex $sym 2]=[lindex $sym 6] [lindex $sym 7]"
	    .f.c create text $xoffset [expr {20 + $yoffset}] \
			-text $t \
			-anchor c -font $font(f6) -tag $stag
	}
	extra {
	    # "extra" extra_type ?arg?
	    # fixme: have better drawing
	    switch [lindex $sym 1] {
		newline {
		    .f.c create rectangle $xoffset [expr {52 - 14 + $yoffset}] \
			[expr {$xoffset + 12}] [expr {52 + 15 + $yoffset}] \
			-outline {} -fill $opt(v_color) -tag $stag
		}
		default {
		    # blen - staves - vover - info - pscom
		    .f.c create rectangle $xoffset [expr {52 - 3 + $yoffset}] \
			    [expr {$xoffset + 4}] [expr {52 + 4 + $yoffset}] \
			    -outline {} -fill gray75 -tag $stag
		}
	    }
	}
	midi {
	    # "midi" command args
	    .f.c create rectangle $xoffset [expr {52 - 3 + $yoffset}] \
		    [expr {$xoffset + 4}] [expr {52 + 4 + $yoffset}] \
		    -outline {} -fill gray75 -tag $stag
	}
	tuplet {
	    # "tuplet" p q r
	    .f.c create rectangle $xoffset [expr {52 - 3 + $yoffset}] \
		[expr {$xoffset + 4}] [expr {52 + 4 + $yoffset}] \
		-outline {} -fill gray75 -tag $stag
	}
	mrest {
	    # "mrest" len
	    set x1 [expr {$xoffset + 0}]
	    set x2 [expr {$xoffset + 20}]
	    .f.c create rectangle $x1 [expr {52 - 2 + $yoffset}] \
		$x2 [expr {52 + 3 + $yoffset}] \
		-outline {} -fill black -tag $stag
	    .f.c create line $x1 [expr {52 - 4 + $yoffset}] \
		$x1 [expr {52 + 5 + $yoffset}] \
		-tag $stag
	    .f.c create line $x2 [expr {52 - 4 + $yoffset}] \
		$x2 [expr {52 + 5 + $yoffset}] \
		-tag $stag
	    .f.c create text [expr {$x1 + 8}] [expr {32 + $yoffset}] \
			-text [lindex $sym 1] \
			-anchor w -font $font(f6) -tag $stag
	}
	EOT {
	    .f.c create rectangle $xoffset [expr {52 - 8 + $yoffset}] \
		    [expr {$xoffset + 8}] [expr {52 + 8 + $yoffset}] \
		    -outline {} -fill blue -tag $stag
	}
	default {
	    #for extensions
	    .f.c create text $xoffset [expr {10 + $yoffset}] \
			-text "?" \
			-anchor w -font $font(f6) -tag $stag
	}
    }
}

proc sym-sel {} {
# select the current symbol
    global score voicerect
#fixme:only when voice change...
    set v [abc get voice]
    .f.c coords $voicerect 0 [expr {$score(height) * $v + 10}] \
	$score(width) [expr {$score(height) * ($v + 1) - 10}]
    voice-display
    cursor-display
}

proc tune-display {} {
# display a newly loaded/modified tune
    global currect score tune
    # display the tune title
    set tune(tname) [lindex [abc tune] [abc get tune]]
    # draw the canvas
    canvas-redraw
    # skip the clef, keysig and timesig
    abc go next
    abc go next
    abc go next
    # display the tune
    redisplay 1
    abc rewind
    voice-display
    cursor-move Home
    mark-set
}

proc tune-first {} {
# display the first tune
    global argc argv lang tune
    if {$argc >= 1} {
	# load a tune from the command line argument
	if {$argc != 1} {
	    tk_messageBox -type ok -icon warning -title {One arg} \
		-message $lang(arg_m)
	    set argv [lindex $argv 0]
	}
	if {[file isdirectory $argv]} {
	    tune-load $argv
	    return
	}
	switch -- [file extension $argv] {
	    .mid {
		midi-load $argv
		return
	    }
	    .abc {
		abc-load $argv
		return
	    }
	    default {
		tk_messageBox -type ok -icon error -title $lang(ext_t) \
			-message $lang(ext_m)
		.title.tune.menu add command -label [abc tune] \
			-command [list abc-select 0]
	    }
	}
    } else {
	# the null tune is already loaded - update the tune menu
	.title.tune.menu add command -label [abc tune] \
		-command [list abc-select 0]
    }
    set tune(nvoice) [abc voice]
    tune-display
}

proc tune-load {{dir {}}} {
# load a tune
    global filesel lang opt tune
    if {$tune(changed)} {
	if {[save-check]} return
    }
#puts "tune-load '$dir'"
    if {[string length $dir] == 0} {
	set dir $opt(lastdir)
    }
    if {[file isdirectory $dir]} {
	set fn [tk_getOpenFile -defaultextension {.abc} \
		-filetypes $filesel -title {Load File} -initialdir $dir]
    } else {
	set fn $dir
    }
    if {[string length $fn] != 0} {
	switch -- [file extension $fn] {
	    .mid {
		midi-load $fn
	    }
	    .abc {
		abc-load $fn
	    }
	    default {
		tk_messageBox -type ok -icon error -title $lang(ext_t) \
			-message $lang(ext_m)
	    }
	}
    }
}

proc tune-reload {} {
# reload the current tune
    global tune
    set x [abc get tune]
#    set tu [lindex [abc tune] $x]
    set v [abc get voice]
    set ti $tune(time)
    tune-load $tune(fname)
#fixme:check the tune title / X: ?
    abc-select $x
    abc go $v
    while {[abc get time] < $ti} {
	if {[string compare [lindex [abc get] 0] EOT] == 0} break
	abc go next
    }
    cursor-display
}

proc updn {down} {
# handle the Up or Down arrow
    global tune
    if {$tune(chord)} {
	# chord edit: loop thru the notes
	# tune(chordx) is the index of the pitch in the note array (2..n)
	if {$down} {
	    incr tune(chordx) -2
	    if {$tune(chordx) < 2 } {
		set tune(chordx) [expr {[llength $tune(cursym)] - 2}]
	    }
	} else {
	    incr tune(chordx) 2
	    if {$tune(chordx) >= [llength $tune(cursym)]} {
		set tune(chordx) 2
	    }
	}
    } else {
	# normal: loop thru the voices
	voice-select $down
    }
    cursor-display
}

proc voice-clear {} {
# erase all notes from the current voice
#fixme
    puts "voice-clear not coded"
#    set tune(message) "voice-clear not coded"
}

proc voice-delete {} {
# delete a voice from the current tune
#fixme
    puts "voice-delete not coded"
#    set tune(message) "voice-delete not coded"
}

proc voice-display {} {
# display the voice characteristics
    global tune
    set vname [abc voice get]
    if {[string length [lindex $vname 1]] != 0} {
	set tune(vname) [lindex $vname 1]
    } else {
	set tune(vname) [lindex $vname 0]
    }
    set tune(mute) [abc play mute]
}

proc voice-select {down} {
# go to the next / previous voice
    global score voicerect tune
    set v [abc get voice]
    if {$down} {
	incr v
	if {$v > $tune(nvoice)} {
	    set v 0
	}
    } else {
	incr v -1
	if {$v < 0 } {
	    set v $tune(nvoice)
	}
    }
    foreach [list p1 p2] [.f.c yview] break
    set h [expr {double($v) / ($tune(nvoice) + 1)}]
    if {$h < $p1
	|| $h + 1. / ($tune(nvoice) + 1) > $p2} {
	.f.c yview moveto $h
    }
    abc go $v
    # (tied to the score(height))
    .f.c coords $voicerect 0 [expr {$score(height) * $v + 10}] \
	    $score(width) [expr {$score(height) * ($v + 1) - 10}]
    voice-display
}

# --- main ---

# general initialization
    if {[info exists abcversion]} {
	package require -exact tclabc $abcversion
    } else {
	# accept to run in the source directory
	set abclib .
	load ./tclabc[info sharedlibextension]
	set abcversion [package require tclabc]
    }

#fixme - for test - KO on Win32
#puts -nonewline {press return to start}
#flush stdout
#gets stdin

# tell where are the external procs
    foreach f [list change-set detail-change header-change header-edit \
	lyrics-edit measure-set midi-change midi-edit midi-export \
	prevargs preview slur-toggle sym-acc sym-chg sym-del sym-dot \
	sym-add sym-flags sym-insert sym-len sym-pitch tie-toggle \
	tune-del tune-new tune-save tune-saveas voice-edit voice-new \
	words-edit] {
	set auto_index($f) [list source [file join $abclib change.tcl]]
    }
    foreach f [list detail-display detail-edit detail-toggle detail-var] {
	set auto_index($f) [list source [file join $abclib detail.tcl]]
    }
    foreach f [list help help-about show-keys] {
	set auto_index($f) [list source [file join $abclib help.tcl]]
    }
    foreach f [list midi-record program-set tempo-set tune-play vel-set \
	vk-start] {
	set auto_index($f) [list source [file join $abclib play.tcl]]
    }
    foreach f [list pref-edit pref-save] {
	set auto_index($f) [list source [file join $abclib pref.tcl]]
    }
    foreach f [list clip-update clip-export mark-copy mark-exch \
	sel-delete sel-extend sel-fetch sel-insert sel-redisplay \
	sel-start sel-stop] {
	set auto_index($f) [list source [file join $abclib select.tcl]]
    }
    set auto_index(webdisp) [list source [file join $abclib www.tcl]]

# default options
    array set opt {
	abc2ps abcm2ps
	abc2ps-args {}
	bg LightYellow
	chg_bg LightPink1
	detail 0
	ent_bg AntiqueWhite
	follow 0
	ghostview gv
	lang lang-en.tcl
	lastdir .
	midiin /dev/midi00
	midiin-start 1
	midiout /dev/sequencer
	midiout-start 1
	nochg_bg DarkSeaGreen2
	playnote 0
	screen-h 300
	screen-w 680
	tempo 80
	tmpdir /tmp
	twosel 0
	v_color white
	vel 0
    }
    # see tclabc.h for the values
    array set flags {
	space 0x01
	in_header 0x02
	embedded 0x04
	nplet 0x08
	nplet_s 0x10
	nplet_e 0x20
	tie_s 0x40
	tie_e 0x80
	slur_s 0x100
	slur_e 0x200
	word_s 0x400
	word_e 0x800
    }
    array set tune {
	acc 0
	changed 0
	channel 0
	chord 0
	chordx 2
	console 0
	cursdisp {}
	curvoice 0
	cursym {}
	fname {noname.abc}
	len 384
	message {}
	midiin 0
	midiout 0
	mini-buffer {}
	mute 0
	nvoice -1
	pref 0
	preview-args {}
	program {Acoustic Grand Piano}
	rmmsg 0
	startsym {}
	screentime 5000
	time 0
	tname {}
	type {abc}
	vname {}
    }
    array set score {
	height 100
	width 660
	update 0
    }
    array set selection {
	clip {}
	mark 0
	rect 0
	syms {}
    }
    if {[string compare $tcl_platform(platform) {windows}] != 0} {
	# (oa 6x10, 6x13, 9x15)
	array set font {
	    f6s {* 8}
	    f6 {* 11}
	    f9 {Courier 15}
	    t10 {Times 10}
	}
    } else {
	array set font {
	    f6s {Courier 9}
	    f6 {Courier 10}
	    f9 {Courier 15}
	    t10 {Times 10}
	}
    }

    set filesel { {{abc} {.abc}} {{midi} {.mid}} {{all} {*}}}

    set iindex [list 0-0 0-1 0-2 0-3 0-4 0-5 0-6 0-7 0-8 0-9\
	0-10 0-11 0-12 0-13 0-14 0-15 0-16 0-17 0-18 0-19\
	0-20 0-21 0-22 0-23 0-24 0-25 0-26 0-27 0-28 0-29\
	0-30 0-31 0-32 0-33 0-34 0-35 0-36 0-37 0-38 0-39\
	0-40 0-41 0-42 0-43 0-44 0-45 0-46 0-47 0-48 0-49\
	0-50 0-51 0-52 0-53 0-54 0-55 0-56 0-57 0-58 0-59\
	0-60 0-61 0-62 0-63 0-64 0-65 0-66 0-67 0-68 0-69\
	0-70 0-71 0-72 0-73 0-74 0-75 0-76 0-77 0-78 0-79\
	0-80 0-81 0-82 0-83 0-84 0-85 0-86 0-87 0-88 0-89\
	0-90 0-91 0-92 0-93 0-94 0-95 0-96 0-97 0-98 0-99\
	0-100 0-101 0-102 0-103 0-104 0-105 0-106 0-107 0-108 0-109\
	0-110 0-111 0-112 0-113 0-114 0-115 0-116 0-117 0-118 0-119\
	0-120 0-121 0-122 0-123 0-124 0-125 0-126 0-127]
    set iname [list\
	{Acoustic Grand Piano} {Bright Acoustic Piano}\
	{Electric grand Piano} {Honky Tonk Piano}\
	{Electric Piano 1} {Electric Piano 2} {Harpsichord} {Clavinet}\
	{Celesra} {Glockenspiel} {Music Box} {Vibraphone}\
	{Marimba} {Xylophone} {Tubular bells} {Dulcimer}\
	{Drawbar Organ} {Percussive Organ} {Rock Organ} {Church Organ}\
	{Reed Organ} {Accordion} {Harmonica} {Tango Accordion}\
	{Nylon Accustic Guitar}	{Steel Acoustic Guitar} {Jazz Electric Guitar}\
	{Ciean Electric Guitar} {Muted Electric Guitar}	{Overdrive Guitar}\
	{Distorted Guitar} {Guitar Harmonics}\
	{Acoustic Bass} {Electric Fingered Bass} {Electric Picked Bass} {Fretless Bass}\
	{Slap Bass 1} {Slap Bass 2} {Syn Bass 1} {Syn Bass 2}\
	{Violin} {Viola} {Cello} {Contrabass}\
	{Tremolo Strings} {Pizzicato Strings} {Orchestral Harp} {Timpani}\
	{String Ensemble 1} {String Ensemble 2 (Slow)} {Syn Strings 1} {Syn Strings 2}\
	{Choir Aahs} {Voice Oohs} {Syn Choir} {Orchestral Hit}\
	{Trumpet} {Trombone} {Tuba} {Muted Trumpet}\
	{French Horn} {Brass Section} {Syn Brass 1} {Syn Brass 2}\
	{Soprano Sax} {Alto Sax} {Tenor Sax} {Baritone Sax}\
	{Oboe} {English Horn} {Bassoon} {Clarinet}\
	{Piccolo} {Flute} {Recorder} {Pan Flute}\
	{Bottle Blow} {Shakuhachi} {Whistle} {Ocarina}\
	{Syn Square Wave} {Syn Sawtooth Wave} {Syn Calliope} {Syn Chiff}\
	{Syn Charang} {Syn Voice} {Syn Fifths Sawtooth Wave} {Syn Brass & Lead}\
	{New Age Syn Pad} {Warm Syn Pad} {Polysynth Syn Pad} {Choir Syn Pad}\
	{Bowed Syn Pad} {Metal Syn Pad} {Halo Syn Pad} {Sweep Syn Pad}\
	{SFX Rain} {SFX Soundtrack} {SFX Crystal} {SFX Atmosphere}\
	{SFX Brightness} {SFX Goblins} {SFX Echoes} {SFX Sci-fi}\
	{Sitar} {Banjo} {Shamisen} {Koto}\
	{Kalimba} {Bag Pipe} {Fiddle} {Shanai}\
	{Tinkle Bell} {Agogo} {Steel Drums} {Woodblock}\
	{Taiko Drum} {Melodic Tom} {Syn Drum} {Reverse Cymbal}\
	{Guitar Fret Noise} {Breath Noise} {Seashore} {Bird Tweet}\
	{Telephone Ring} {Helicopter} {Applause} {Gun Shot}]

    array set sfTab {
	sharp {0 9 -3 6 15 3 12}
	flat {12 3 15 6 18 9 21}
    }
    # [] | || |] [| |: :| :: : - !! see abcparse.h for values !!
    set bartypes [list 35 1 17 19 49 20 65 68 4]

# create the images
# - the bitmaps were created using 'bitmap'.
# - the photos were created using 'xpaint' and are base64 encoded.
# !! the accidental numbers are the ones in abcparse.h !!
    # no accidental
    image create bitmap acc0 -data {
#define acc0_width 8
#define acc0_height 18
static unsigned char acc0_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x4b, 0xab, 0xad, 0xad, 0x49,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
}
    # sharp
    image create bitmap acc1 -data {
#define sh_width 8
#define sh_height 18
static unsigned char sh_bits[] = {
   0x40, 0x48, 0x48, 0x48, 0x48, 0xe8, 0xfc, 0x5c, 0x48, 0x48, 0x48, 0xe8,
   0xfc, 0x5c, 0x48, 0x48, 0x48, 0x08};
}
    # natural
    image create bitmap acc2 -data {
#define nt_width 8
#define nt_height 18
static unsigned char nt_bits[] = {
   0x00, 0x00, 0x08, 0x08, 0x08, 0x88, 0xe8, 0xf8, 0x98, 0x88, 0x88, 0xe8,
   0xf8, 0xb8, 0x88, 0x80, 0x80, 0x80};
}
    # flat
    image create bitmap acc3 -data {
#define fl_width 8
#define fl_height 18
static unsigned char fl_bits[] = {
   0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x68, 0xd8, 0x88, 0xc8, 0x68, 0x18,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
}
    # double sharp
    image create bitmap acc4 -data {
#define dsh_width 8
#define dsh_height 18
static unsigned char dsh_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0xee, 0x38, 0x10, 0x38, 0xee,
   0xc6, 0x00, 0x00, 0x00, 0x00, 0x00};
}
    # double flat
    image create bitmap acc5 -data {
#define dft_width 8
#define dfl_height 18
static unsigned char dft_bits[] = {
   0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x55, 0xbb, 0x99, 0xdd, 0x55, 0x33,
   0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
}
    # 1/4 tone sharp
    image create bitmap acc9 -data {
#define sh14_width 8
#define sh14_height 18
static unsigned char sh14_bits[] = {
   0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x38, 0x10, 0x10, 0x10, 0x10, 0x38,
   0x38, 0x10, 0x10, 0x10, 0x10, 0x00};
}
    # 3/4 tone sharp
    image create bitmap acc4105 -data {
#define sh34_width 9
#define sh34_height 18
static unsigned char sh34_bits[] = {
   0x80, 0x00, 0x90, 0x00, 0x92, 0x00, 0x92, 0x00, 0xd2, 0x01, 0xfa, 0x01,
   0xbf, 0x00, 0x97, 0x00, 0x92, 0x00, 0x92, 0x00, 0xd2, 0x01, 0xfa, 0x01,
   0xbf, 0x00, 0x97, 0x00, 0x92, 0x00, 0x92, 0x00, 0x12, 0x00, 0x02, 0x00};
}
    # 1/4 tone flat
    image create bitmap acc11 -data {
#define ft14_width 8
#define ft14_height 18
static unsigned char ft14_bits[] = {
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xb0, 0xd8, 0x88, 0x98, 0xb0, 0xc0,
   0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
}
    # 3/4 tone flat
    image create bitmap acc4107 -data {
#define ft34_width 9
#define ft34_height 18
static unsigned char ft34_bits[] = {
   0x28, 0x00, 0x28, 0x00, 0x28, 0x00, 0x28, 0x00, 0x28, 0x00, 0x28, 0x00,
   0xaa, 0x00, 0x6d, 0x01, 0x29, 0x01, 0xab, 0x01, 0xaa, 0x00, 0x6c, 0x00,
   0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
}
    # notes
    # breve
    image create bitmap note3 -data {
#define tmp_width 9
#define tmp_height 8
static unsigned char tmp_bits[] = {
   0x01, 0x01, 0x3d, 0x01, 0xf3, 0x01, 0xc3, 0x01, 0x87, 0x01, 0x9f, 0x01,
   0x79, 0x01, 0x01, 0x01};
}
    # whole note
    image create bitmap note2 -data {
#define note2_width 7
#define note2_height 6
static unsigned char note2_bits[] = {
   0x1e, 0x79, 0x61, 0x43, 0x4f, 0x3c};
}
    # white
    image create bitmap note1 -data {
#define note1_width 7
#define note1_height 6
static char note1_bits[] = {
 0x38,0x4e,0x43,0x61,0x39,0x0e};
}
    # black
    image create bitmap note0 -data {
#define note0_width 7
#define note0_height 6
static char note0_bits[] = {
 0x38,0x7e,0x7f,0x7f,0x3f,0x0e};
}
    # note flags
    image create bitmap flagu -data {
#define flag_width 6
#define flag_height 13
static char flag_bits[] = {
 0xc1,0xc1,0xc1,0xc3,0xc3,0xc7,0xce,0xdc,0xd8,0xf0,0xe0,0xe0,0xe0};
}
    image create bitmap flagd -data {
#define flagI_width 6
#define flagI_height 13
static char flagI_bits[] = {
 0x20,0x20,0x20,0x30,0x18,0x1c,0x0e,0x07,0x03,0x03,0x01,0x01,0x01};
}
    #rests
    image create bitmap rest3 -data {
#define tmp_width 11
#define tmp_height 23
static unsigned char tmp_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x70, 0x00, 0x70, 0x00, 0x70, 0x00, 0x70, 0x00, 0x70, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
}
    image create bitmap rest2 -data {
#define b1_width 11
#define b1_height 23
static char b1_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x01,0xfe,
 0x01,0xfe,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00};
}
    image create bitmap rest1 -data {
#define b2_width 11
#define b2_height 23
static char b2_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0xfe,0x01,0xfe,0x01,0xfe,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00};
}
    image create bitmap rest0 -data {
#define b4_width 11
#define b4_height 23
static char b4_bits[] = {
 0x00,0x00,0x10,0x00,0x20,0x00,0x40,0x00,0xc0,0x00,0xc0,0x01,0xe0,0x01,0xf0,
 0x01,0xf8,0x00,0x7c,0x00,0x38,0x00,0x30,0x00,0x20,0x00,0x40,0x00,0xf0,0x00,
 0xf8,0x01,0x38,0x00,0x18,0x00,0x10,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00};
}
    image create bitmap rest-1 -data {
#define b8_width 11
#define b8_height 23
static char b8_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x18,0x03,0x9c,
 0x01,0xdc,0x01,0xb8,0x01,0x80,0x00,0x80,0x00,0xc0,0x00,0xc0,0x00,0x40,0x00,
 0x60,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00};
}
    image create bitmap rest-2 -data {
#define b16_width 11
#define b16_height 23
static char b16_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x18,0x01,0x9c,
 0x01,0xdc,0x01,0xb8,0x00,0x80,0x00,0x80,0x00,0xc6,0x00,0x67,0x00,0x77,0x00,
 0x6e,0x00,0x20,0x00,0x20,0x00,0x30,0x00,0x30,0x00,0x10,0x00,0x18,0x00,0x18,
 0x00};
}
    image create bitmap rest-3 -data {
#define b32_width 11
#define b32_height 23
static char b32_bits[] = {
 0x60,0x04,0x70,0x06,0x70,0x07,0xe0,0x02,0x00,0x02,0x00,0x03,0x18,0x01,0x9c,
 0x01,0xdc,0x01,0xb8,0x00,0x80,0x00,0x80,0x00,0xc6,0x00,0x67,0x00,0x77,0x00,
 0x2e,0x00,0x20,0x00,0x20,0x00,0x30,0x00,0x10,0x00,0x18,0x00,0x18,0x00,0x18,
 0x00};
}
    image create bitmap rest-4 -data {
#define tmp_width 13
#define tmp_height 23
static unsigned char tmp_bits[] = {
   0x80, 0x11, 0xc0, 0x19, 0xc0, 0x1d, 0x80, 0x0b, 0x00, 0x08, 0x00, 0x0c,
   0x60, 0x04, 0x70, 0x04, 0x70, 0x07, 0xe0, 0x02, 0x00, 0x02, 0x00, 0x03,
   0x18, 0x01, 0x1c, 0x01, 0xdc, 0x01, 0xb8, 0x00, 0x80, 0x00, 0xc0, 0x00,
   0x46, 0x00, 0x47, 0x00, 0x77, 0x00, 0x2e, 0x00, 0x20, 0x00};
}
    # clefs
    # - treble
    image create bitmap clef0 -data {
#define key0_width 18
#define key0_height 42
static unsigned char key0_bits[] = {
   0x00, 0x06, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x80, 0x1f, 0x00,
   0x80, 0x1b, 0x00, 0x80, 0x13, 0x00, 0x80, 0x11, 0x00, 0x80, 0x19, 0x00,
   0x80, 0x1c, 0x00, 0x80, 0x1c, 0x00, 0x80, 0x1c, 0x00, 0x80, 0x0f, 0x00,
   0x80, 0x0f, 0x00, 0x80, 0x07, 0x00, 0xc0, 0x03, 0x00, 0xe0, 0x03, 0x00,
   0xf0, 0x03, 0x00, 0xf8, 0x02, 0x00, 0x78, 0x02, 0x00, 0x3c, 0x02, 0x00,
   0x1e, 0x3f, 0x00, 0x9e, 0x7f, 0x00, 0xce, 0x7f, 0x00, 0xce, 0xf5, 0x00,
   0xce, 0xe4, 0x00, 0xce, 0xc4, 0x00, 0xde, 0xc4, 0x00, 0x9c, 0xc8, 0x00,
   0x18, 0x49, 0x00, 0x30, 0x68, 0x00, 0xe0, 0x38, 0x00, 0x80, 0x1f, 0x00,
   0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x10, 0x00, 0xc0, 0x11, 0x00,
   0xe0, 0x13, 0x00, 0xe0, 0x13, 0x00, 0xe0, 0x13, 0x00, 0xe0, 0x11, 0x00,
   0xc0, 0x08, 0x00, 0x80, 0x07, 0x00};
}
    # - alto
    image create bitmap clef1 -data {
#define key1_width 18
#define key1_height 38
static unsigned char key1_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x5c, 0xf8, 0x00, 0x5c, 0xcc, 0x01, 0x5c, 0x9e, 0x03, 0x5c, 0x9e, 0x03,
   0x5c, 0x8c, 0x03, 0x5c, 0x80, 0x03, 0x5c, 0x80, 0x03, 0x5c, 0x84, 0x03,
   0x5c, 0x84, 0x01, 0x5c, 0xcc, 0x01, 0x5c, 0x7e, 0x00, 0x5c, 0x07, 0x00,
   0x5c, 0x07, 0x00, 0x5c, 0x07, 0x00, 0x5c, 0x7e, 0x00, 0x5c, 0xcc, 0x01,
   0x5c, 0x84, 0x01, 0x5c, 0x84, 0x03, 0x5c, 0x80, 0x03, 0x5c, 0x80, 0x03,
   0x5c, 0x8c, 0x03, 0x5c, 0x9e, 0x03, 0x5c, 0x9e, 0x03, 0x5c, 0xcc, 0x01,
   0x5c, 0xf8, 0x00, 0x00, 0x00, 0x00};
}
    # - bass
    image create bitmap clef2 -data {
#define key2_width 18
#define key2_height 39
static unsigned char key2_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00,
   0x18, 0x87, 0x01, 0x0c, 0xce, 0x03, 0x1e, 0x8e, 0x01, 0x3e, 0x1c, 0x00,
   0x3e, 0x1c, 0x00, 0x3e, 0x1c, 0x00, 0x1c, 0x9c, 0x01, 0x00, 0xdc, 0x03,
   0x00, 0x9e, 0x01, 0x00, 0x0e, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00,
   0x80, 0x07, 0x00, 0xc0, 0x03, 0x00, 0xe0, 0x01, 0x00, 0xf0, 0x00, 0x00,
   0x38, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x03, 0x00, 0x00};
}
    # - perc
    image create bitmap clef3 -data {
#define key3_width 18
#define key3_height 38
static unsigned char key3_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xe0, 0x1f, 0x00, 0x20, 0x10, 0x00, 0x20, 0x10, 0x00,
   0x20, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x10, 0x00,
   0x20, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x10, 0x00,
   0x20, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x10, 0x00,
   0x20, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x10, 0x00, 0xe0, 0x1f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
}

    image create photo new -format gif -data {
R0lGODlhFQAeALMAAP///93d3f/84AAAAHd3d+zs7KWlpURJXMzMzJaWlgAAAAAAAAAAAAAAAAAA
AAAAACH5BAEAAAAALAAAAAAVAB4AQASVEMhJa02ghCG6FwciDkFpENnmcQK7mqg2zHRdw6l9l3xw
5qvPh+SLqTq63c9naDqf0KaxR60WgUIQgsj7ybJZovfYSs5wX6R6iCavZq+rxkqXF+74vP4emNar
Yy4fB1xdRoIeIVs9gWAHj49ih29mbS6XcC2WQWWXm0hmklgtayyiaUOln2BxY3+Afq+GCQS1tre4
uBEAOwA=
}
    image create photo load -format gif -data {
R0lGODlhFQAeALMAAP///93d3f/84AAAAHd3d+zs7KWlpf9LS5aWlgAAAAAAAAAAAAAAAAAAAAAA
AAAAACH5BAEAAAAALAAAAAAVAB4AQASEEMhJa0WghB2G91wYGETWDUKqquiwkaYoi7A232Npo2sv
uLnY6UMEvUojg3LJbCp1uFstOptuPIdsERjU8Hy/FNDqAYvHUCotrebUCvC4fA4PsNtdvPvetl6z
B1xrMWVmRlNbRWJHJl8+ZWiEZj+HbI49lUJ4fpt8apx9BAgEpKWmp6cRADsA
}
    image create photo save -format gif -data {
R0lGODlhFQAeALMAAP///93d3QAAAHd3d//84Ozs7KWlpf/g4DMzM7u7u5aWlgAAAAAAAAAAAAAA
AAAAACH5BAEAAAAALAAAAAAVAB4AQASaEMhJa1WgBMG790FgDNlGnEiqIqcQjmUoz/RLanUuw5pA
CIegMOhziW4bH4KSSLB+NoDIQK1ar1SkTsfbcpHF4bAY7f0+Huix5CMsJU3A09j11ur2Ga/A7/v/
fAFaeTuDhGs4h4gmQGJEal1hjgdki5KOlZEnm5ybdGAobwBNc2UmbhMJo6WLSR4qIK2KeIe0hCMK
A7q7vL29EQA7AA==
}
    image create photo rewind -format gif -data {
R0lGODdhJAAWAMIAANjc2HB0cMjIyPj8yPj8+AAAAAAAAAAAACwAAAAAJAAWAAADdQi63P4wyklB
CALerHZdwUA8l9iF42eZTWm66Ye24SC6rJxbtY3vlNnJ5uvZYhXh73dUrUZLIxEZfBF91yt1MpNK
p86ZQHAdZ5sqoZm41oZz7UEcyN3F41uJcNEm9N9bayOCgA9mKYdOAgR5AGMxj06Sk5QTCQA7
}
    image create photo backward -format gif -data {
R0lGODdhJAAWAMIAANjc2HB0cMjIyPj8yPj8+AAAAAAAAAAAACwAAAAAJAAWAAADYgi63P4wyklp
CGLdXG0YhHKB3TSSZ1hCJ9iqa9MOMxyL36DP5I3ruxywd5vxdLaVcDccJktLWhP47AgEw+uUeNMC
vU7fAjwgc31kcjUGJrTFDW9IDo9jVdo1WgC76uuAgQsJADs=
}
    image create photo forward -format gif -data {
R0lGODdhJAAWAMIAANjc2HB0cMjIyPj8yOjs6Pj8+AAAAAAAACwAAAAAJAAWAAADZAi63P4wyklb
EGLdXGsYmPKF3fSB2UmW0Im6K2sNNFqjsuPS2B3nO57Ax8kBgjyCj2A83m6FZ6GJpEWh1Kf1us1K
uYPC1FsTQ8fkMFjc1JQLLnZb1D3J5075544P3P1oeIKDFQkAOw==
}
    image create photo play0 -format gif -data {
R0lGODdhJAAWAMIAANjc2HB0cMjIyNj82Pj8+Ojs6AAAAAAAACwAAAAAJAAWAAADXgi63P4wyklb
CELhXHsYhDYIXDd9IfCNpQmh4sC6EazKMkk/9orrO0YP92vthkRZIRVEJkFBUcqXZB6htyfBSnPi
tlFp7Mu9TongsDgLKjexq7R6jXJHBemAfM7vdxIAOw==
}
    image create photo play -format gif -data {
R0lGODdhJAAWAMIAANjc2HB0cMjIyNj82Ojs6Pj8+AAAAAAAACwAAAAAJAAWAAADSQi63P4wyklh
EKJqFwbeINB5WViNpEmhqRqxrcsNdP3JDUwTBc7otJ5PARwIh8DC0adTDhcw55NYMy6fKOmUat3+
ul6oNkwOJwAAOw==
}
    image create photo stop -format gif -data {
R0lGODdhJAAWAMIAANjc2HB0cKCkoNjc+Pj8+Ojs6AAAAAAAACwAAAAAJAAWAAADPAi63P4wykmr
vTjrzXf4ICh0TTCc6EmQjJmiK6u47xDL9HuzebqTPZhsVlMNAUHjsEBoOp3HqHRKrVqPCQA7
}
    image create photo record-start -format gif -data {
R0lGODdhJAAWAMIAANjc2HB0cKCkoPgAAPj8+AAAAOjs6AAAACwAAAAAJAAWAAADPAi63P4wykmr
vTjrzXf4ICh0TTCc6EmQTJGmK6uYryrPtX3TdSzzLx8LCLsBiCghyUBoOp3GqHRKrVqNCQA7
}
    image create photo keyb-start -format gif -data {
R0lGODdhHgAWAKEAANjc2AAAAPj8+AAAACwAAAAAHgAWAAACVYSPqcvtD6OcKNiLsw4siH99QgiK
VmdaqVeC3MKOqhvXr2LnZorS6+7qyXQtmZA0BBpholHTqRwJodGYlBmlam8Jq3Xb5FbCT2+4s0lr
KOy2+w2PFAAAOw==
}
    # small notes
    image create bitmap notes3072 -data {
#define note_width 8
#define note_height 18
static unsigned char note_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x5e, 0x66,
   0x66, 0x7a, 0x42, 0x00, 0x00, 0x00};
}
    image create bitmap notes1536 -data {
#define note_width 8
#define note_height 18
static unsigned char note_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x24,
   0x24, 0x38, 0x00, 0x00, 0x00, 0x00};
}
    image create bitmap notes768 -data {
#define note_width 8
#define note_height 18
static unsigned char note_bits[] = {
   0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x24,
   0x24, 0x1c, 0x00, 0x00, 0x00, 0x00};
}
    image create bitmap notes384 -data {
#define note_width 8
#define note_height 18
static unsigned char note_bits[] = {
   0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x3c,
   0x3c, 0x1c, 0x00, 0x00, 0x00, 0x00};
}
    image create bitmap notes192 -data {
#define note_width 8
#define note_height 18
static unsigned char note_bits[] = {
   0x00, 0x20, 0x20, 0x60, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0x20, 0x38, 0x3c,
   0x3c, 0x1c, 0x00, 0x00, 0x00, 0x00};
}
    image create bitmap notes96 -data {
#define note_width 8
#define note_height 18
static unsigned char note_bits[] = {
   0x20, 0x60, 0xa0, 0xa0, 0x20, 0x60, 0xa0, 0xa0, 0xa0, 0x20, 0x38, 0x3c,
   0x3c, 0x1c, 0x00, 0x00, 0x00, 0x00};
}
   image create bitmap notes48 -data {
#define note_width 8
#define note_height 18
static unsigned char note_bits[] = {
   0x20, 0x60, 0xa0, 0x20, 0x60, 0xa0, 0x20, 0x60, 0xa0, 0x20, 0x38, 0x3c,
   0x3c, 0x1c, 0x00, 0x00, 0x00, 0x00};
}
   image create bitmap notes24 -data {
#define note_width 8
#define note_height 21
static unsigned char note_bits[] = {
   0x20, 0x60, 0xa0, 0x20, 0x60, 0xa0, 0x20, 0x60, 0xa0, 0x20, 0x60,
   0xa0, 0x20, 0x38, 0x3c, 0x3c, 0x1c, 0x00, 0x00, 0x00, 0x00};
}
    # get the configuration parameters
    if {[string compare $tcl_platform(platform) {windows}] != 0} {
	set rcfile {.tkabcrc}
    } else {
	set rcfile {tkabc.rc}
    }
    if {![file readable $rcfile]} {
	set rcfile ~/$rcfile
    }
    if {[file readable $rcfile]} {
	if {[catch {source $rcfile}]} {
	    bgerror {Init file error}
	}
    }
    # set the language
    set fn $opt(lang)
    if {![file readable $fn]} {
	set fn [file join $abclib $fn]
	if {![file readable $fn]} {
	    tk_messageBox -type ok -icon error\
		-message "No language file '$fn'" -title {Bad file}
	    exit
	}
    }
    if {[catch {source $fn}]} {
	bgerror {Language file error}
	exit
    }
    unset fn

    if {[info exists geometry]} {
	foreach [list opt(screen-w) opt(screen-h) dum1 dum2] [split $geometry "x+-"] \
	    break
    } else {
	wm geometry . $opt(screen-w)x$opt(screen-h)
    }
    set score(width) [expr {$opt(screen-w) - 10}]

# open the midi in/out devices
    if {[abc play]} {
	if {$opt(midiout-start)} {
	    set tune(midiout) 1
	    midi-open 1
	}
	if {$opt(midiin-start)} {
	    set tune(midiin) 1
	    midi-open 0
	}
    }

# create the main window
    wm title . {tkabc}
    wm protocol . WM_DELETE_WINDOW quit

# menubar
    frame .mb -relief raised -borderwidth 2
    foreach m [list File Edit Symbol Pitch Options Pref Help] {
	menubutton .mb.m$m -text $lang(t_$m) -underline 0 -menu .mb.m$m.menu
	menu .mb.m$m.menu -tearoff 0
    }
    pack .mb.mFile .mb.mEdit .mb.mSymbol .mb.mPitch .mb.mOptions \
	    -side left
    pack .mb.mHelp -side right

    .mb.mFile.menu add command -label $lang(m_open) -accel o -command tune-load
    .mb.mFile.menu add command -label $lang(m_reload) -accel O -command tune-reload
    .mb.mFile.menu add command -label $lang(m_save) -accel s -command tune-save
    .mb.mFile.menu add command -label $lang(m_saveas) -command tune-saveas
    .mb.mFile.menu add separator
    .mb.mFile.menu add command -label $lang(m_midiexp) -accel x \
	-command midi-export
    .mb.mFile.menu add separator
    .mb.mFile.menu add command -label $lang(m_preview) -accel P -command preview
    .mb.mFile.menu add command -label $lang(m_prargs) -accel a \
	-command prevargs
    .mb.mFile.menu add separator
    .mb.mFile.menu add command -label $lang(m_quit) -accel q -command quit

    .mb.mEdit.menu add command -label $lang(m_chedit) -accel c \
	-command [list .top2.chord invoke]
    .mb.mEdit.menu add command -label $lang(m_tie) -accel t -command tie-toggle
    .mb.mEdit.menu add command -label $lang(m_slst) -accel ( \
	-command [list slur-toggle 0]
    .mb.mEdit.menu add command -label $lang(m_slen) -accel ) \
	-command [list slur-toggle 1]
    .mb.mEdit.menu add command -label $lang(m_hded) -accel h \
	-command header-edit
    .mb.mEdit.menu add command -label $lang(m_gched) -accel e \
	-command detail-edit
    .mb.mEdit.menu add command -label $lang(m_lyed) -accel l \
	-command lyrics-edit
    .mb.mEdit.menu add command -label $lang(m_wded) -accel w \
	-command words-edit
    .mb.mEdit.menu add command -label $lang(m_midied) -accel i \
	-command midi-edit
    .mb.mEdit.menu add command -label $lang(m_rawed) -accel R \
	-command [list focus .top.sym.e2]
    .mb.mEdit.menu add separator
    .mb.mEdit.menu add command -label $lang(m_barset) -accel M \
	-command measure-set
    .mb.mEdit.menu add separator
    .mb.mEdit.menu add command -label $lang(m_voed) -accel v -command voice-edit
    .mb.mEdit.menu add command -label $lang(m_vonew) -command voice-new
#    .mb.mEdit.menu add command -label "Delete a voice" -command voice-delete
    .mb.mEdit.menu add separator
    .mb.mEdit.menu add command -label $lang(m_tunenew) -command tune-new
    .mb.mEdit.menu add command -label $lang(m_tunedel) -command tune-del

    .mb.mSymbol.menu add command -label $lang(m_note) -accel n \
	-command [list sym-add n]
    .mb.mSymbol.menu add command -label $lang(m_rest) -accel z \
	-command [list sym-add z]
    .mb.mSymbol.menu add command -label $lang(m_bar) -accel b \
	-command [list sym-add b]
    .mb.mSymbol.menu add command -label $lang(m_tempo) -accel Q \
	-command [list sym-add Q]
    .mb.mSymbol.menu add command -label $lang(m_mrest) -accel Z \
	-command [list sym-add Z]
    .mb.mSymbol.menu add command -label $lang(m_nl) -accel Return \
	-command [list sym-add Return]
    .mb.mSymbol.menu add separator
    .mb.mSymbol.menu add command -label $lang(m_longer) -accel + \
	-command [list sym-len plus]
    .mb.mSymbol.menu add command -label $lang(m_shorter) -accel - \
	-command [list sym-len minus]
    .mb.mSymbol.menu add separator
    .mb.mSymbol.menu add command -label $lang(m_dot) -accel . \
	-command [list sym-dot 1]
    .mb.mSymbol.menu add command -label $lang(m_ddot) -accel : \
	-command [list sym-dot 2]

    .mb.mPitch.menu add command -label $lang(m_upper) -accel PgUp \
	-command [list sym-pitch 1]
    .mb.mPitch.menu add command -label $lang(m_lower) -accel PgDn \
	-command [list sym-pitch -1]
    .mb.mPitch.menu add command -label $lang(m_oct+) -accel ' \
	-command [list sym-pitch 7]
    .mb.mPitch.menu add command -label $lang(m_oct-) -accel , \
	-command [list sym-pitch -7]
    .mb.mPitch.menu add separator
    .mb.mPitch.menu add command -label $lang(m_sharp) -accel ^ \
	-command [list sym-acc 1]
    .mb.mPitch.menu add command -label $lang(m_flat) -accel _ \
	-command [list sym-acc 3]
    .mb.mPitch.menu add command -label $lang(m_nat) -accel = \
	-command [list sym-acc 0]

    .mb.mOptions.menu add check -label $lang(m_details) -accel d \
	-variable opt(detail) -command [list detail-toggle 0]
#    .mb.mOptions.menu add check -label {Overwrite} -accel Insert \
#	-variable tune(overw)
    if {[abc play]} {
	.mb.mOptions.menu add check -label $lang(m_midin) \
		-variable tune(midiin) -command [list midi-open 0]
	.mb.mOptions.menu add check -label $lang(m_midout) \
		-variable tune(midiout) -command [list midi-open 1]
    }
    if {[string compare $tcl_platform(platform) {unix}] != 0} {
	.mb.mOptions.menu add check -label {Console} \
		-variable tune(console) -offvalue hide -onvalue show \
		-command {console $tune(console)}
    }
    .mb.mOptions.menu add separator
    .mb.mOptions.menu add command -label $lang(m_full) -accel F \
	-command full-toggle
    .mb.mOptions.menu add separator
    .mb.mOptions.menu add command -label $lang(m_pref) -accel f \
	-command pref-edit

    .mb.mHelp.menu add command -label $lang(m_help) -command help
    .mb.mHelp.menu add command -label $lang(m_about) -command help-about
    .mb.mHelp.menu add command -label {Keys} -command show-keys

# title
    set t .title
    frame $t -borderwidth 2 -relief flat
    label $t.e -text { }
    button $t.new -image new \
	    -bd 0 -height 28 -command tune-new
    button $t.load -image load \
	    -bd 0 -height 28 -command tune-load
    button $t.save -image save \
	    -bd 0 -height 28 -command tune-save
    label $t.fname -textvariable tune(fname) \
	    -background $opt(nochg_bg)
    menubutton $t.tune -menu .title.tune.menu -textvariable tune(tname) \
	-indicatoron 1 -direction flush -background $opt(nochg_bg)
    menu $t.tune.menu -tearoff 0
    pack $t.e $t.new $t.load $t.save -side left
    pack $t.fname $t.tune -side left -expand y -fill x

# top
    set t .top
    frame $t -borderwidth 1 -relief raised
    label $t.e -text { }
#    button $t.addTr -image [image create photo -file $imgdir/gif/addTrack.gif] \
#	    -bd 0 -height 28 -fg black -command voice-new

    label $t.v1 -text $lang(d_voice) -font $font(f6)
    label $t.v2 -textvariable tune(vname) -font $font(f6) \
	-width 8 -anchor w
    frame $t.sym
    label $t.sym.l1 -text $lang(d_time) -font $font(f6)
    label $t.sym.v1 -textvariable tune(time) -font $font(f6) \
	-width 11 -anchor w
    label $t.sym.l2 -text $lang(d_sym) -font $font(f6)
    entry $t.sym.e2 -relief groove -background $opt(ent_bg) \
	-font $font(f6) -width 40 -textvariable tune(cursym)
    pack $t.sym.l1 $t.sym.v1 $t.sym.l2 $t.sym.e2 -side left
    pack $t.e $t.v1 $t.v2 $t.sym -side left

# top2
    set t .top2
    frame $t -borderwidth 1 -relief raised
    label $t.e -text { }
    label $t.ln -text $lang(d_dur) -font $font(f6)
    menubutton $t.nl -menu $t.nl.menu -relief groove -bd 2 -width 16 \
	-height 22 \
	-image notes384 -background $opt(ent_bg) -direction flush
    menu $t.nl.menu -tearoff 0
#fixme: keep the dots
    foreach i [list 3072 1536 768 384 192 96 48 24] {
	$t.nl.menu add radiobutton -variable tune(len) -value $i\
		-image notes$i \
		-command [list sym-len $i] \
		-indicatoron y
    }
    label $t.la -text $lang(d_acc) -font $font(f6)
    menubutton $t.acc -menu $t.acc.menu -relief groove -bd 2 -width 16 \
	    -image acc0 -background $opt(ent_bg) -direction flush
    menu $t.acc.menu -tearoff 0
    foreach i [list 5 3 0 1 4 2 9 4105 11 4107] {
	$t.acc.menu add radiobutton -variable tune(acc) -value $i\
		-image acc$i \
		-command [list sym-acc $i] \
		-indicatoron y
    }
    checkbutton $t.chord -font $font(f6) -background $opt(ent_bg) \
	-indicatoron 0 -state disabled -text $lang(d_chord) -relief groove \
	-variable tune(chord) -selectcolor $opt(chg_bg) \
	-command {cursor-display}
    checkbutton $t.mute -font $font(f6) -background $opt(ent_bg) \
	-indicatoron 0 -text $lang(d_mute) -relief groove \
	-variable tune(mute) -selectcolor $opt(chg_bg) \
	-command {abc play mute $tune(mute)}
    if {![abc play]} {
#    if {$tune(midi) == 0}
	$t.mute configure -state disabled
    }
    label $t.msg -textvariable tune(message) -font $font(f6)
    entry $t.mini -background $opt(ent_bg) -bd 0 -relief flat \
	-textvariable tune(mini-buffer) -width 16
    pack $t.e $t.ln $t.nl $t.la $t.acc $t.chord $t.mute $t.msg -side left -padx 2

# detail
    set t .detail
    frame $t -borderwidth 1 -relief raised
    label $t.e -text { }
    label $t.gchl -anchor e -text $lang(d_gch) -font $font(f6)
    entry $t.gche -relief groove -width 15 -background $opt(ent_bg) \
	-textvariable detail(gchord)
    label $t.decl -anchor e -text $lang(d_deco) -font $font(f6)
    entry $t.dece -relief groove -width 15 -background $opt(ent_bg) \
	-textvariable detail(deco)
    pack $t.e $t.gchl $t.gche $t.decl $t.dece -side left

# canvas with 2 scrollbars
    frame .f
    canvas .f.c -background $opt(bg) \
	-cursor top_left_arrow -borderwidth 0 \
	-yscrollcommand [list .f.s set]
    scrollbar .f.s -orient vertical -command [list .f.c yview] \
	-width 12 -jump 1
    scrollbar .f.hs -orient horizontal -command hmove \
	-width 12 -jump 1
    pack .f.s -side left -fill y
    pack .f.hs -side bottom -fill x
    pack .f.c -side right -expand 1 -fill both

# play
    frame .play
    set t .play
    label $t.e -text { }
    button $t.kbd -image keyb-start \
	-command {vk-start}
    button $t.rew -image rewind \
	-command [list cursor-move Home]
    button $t.back -image backward \
	-command [list cursor-move -1b]
    button $t.forw -image forward \
	-command [list cursor-move 1b]
    pack $t.e $t.kbd $t.rew $t.back $t.forw -side left
    if {[abc play]} {
#    if {$tune(midi) > 0}
	label $t.e1 -text { }
	button $t.play0 -image play0 -command [list tune-play 1]
	button $t.play -image play -command [list tune-play 0]
	pack $t.e1 $t.play0 $t.play -side left
#	if {$tune(midi) > 1}
	    button $t.record -image record-start -command {midi-record}
	    pack $t.record -side left
	label $t.tempol -text $lang(d_tempo) -font $font(f6)
	scale $t.tempo -orient horizontal -length 170 -from 30 -to 210 \
		-font $font(f6s) -width 8 -bigincrement 0.5 \
		-sliderlength 20 \
		-command {tempo-set} -tickinterval 30 -variable opt(tempo)
	label $t.vell -text $lang(d_vel) -font $font(f6)
	scale $t.vel -orient horizontal -length 100 -from 0 -to 127 \
		-font $font(f6s) -width 8 -bigincrement 0.5 \
		-sliderlength 20 \
		-command {vel-set} -tickinterval 50 -variable opt(vel)
	pack $t.tempo $t.tempol $t.vel $t.vell -side right
    }

# all in the main frame (except .detail)
    pack .mb .title .top .top2 -side top -pady 0 -fill x
    pack .play -side bottom -pady 0 -fill x
    pack .f -side top -pady 0 -expand 1 -fill both

# balloons
    balloon .title.new $lang(b_new)
    balloon .title.load $lang(b_load)
    balloon .title.save $lang(b_save)
#    balloon .top.addTr {Insert a new voice}
    balloon .top2.chord $lang(b_ched)
#fixme: have an icon for the MIDI parameters
#    balloon .midi.pn {Select the instrument MIDI program}
    balloon .play.kbd $lang(b_kbd)
    balloon .play.rew $lang(b_rew)
    balloon .play.back $lang(b_back)
    balloon .play.forw $lang(b_forw)
    if {[abc play]} {
#    if {$tune(midi) > 0}
	balloon .top2.mute $lang(b_mute)
	balloon .play.play0 $lang(b_play0)
	balloon .play.play $lang(b_play)
	balloon .play.tempo $lang(b_tempo)
	balloon .play.vel $lang(b_vel)
#	if {$tune(midi) > 1}
	    balloon .play.record $lang(b_rec)
    }

    # window bindings
    bind . <Configure> {if {[string compare %W .] == 0} {screen-set %w %h; break}}

    # canvas bindings
    # all this code deals with exact character matching
    if {[string compare $tcl_platform(platform) {windows}] != 0} {
	set alt 8
    } else {
	set alt 0x20000	;# windows
    }
    bindtags .f.c {mytag all}
    bind mytag <KeyPress> {if {[ok_bind %K %s %A]} break}

proc ok_bind {k s a} {
# handle the key press in the canvas
    global alt prvkey
#puts "%K:$k %s:$s %A:$a"
	if {[string is digit $k]} {
	    set k "Key-$k"
	} elseif {[string compare $k ??] == 0} { ;# compose char
	    set k $a
	}
	if {$s & 1} {
	    if {[string length $a] == 0} {
		set k "Shift-$k"
	    }
	}
	if {$s & 4} {
	    set k "Control-$k"
	}
	if {$s & $alt} {
	    set k "Alt-$k"
	}
#puts " --> k:$k"
	if {[info exists prvkey]} {
	    set script [bind .f.c <$prvkey><$k>]
	    unset prvkey
	    if {[string length $script] != 0} {
		eval $script
		return 1
	    }
	}
	set script [bind .f.c <$k>]
#puts "<$k> script: '$script'"
	if {[string length $script] != 0} {
	    eval $script
	    return 1
	}
	switch $k {
	    Escape - Control-x {
		set prvkey $k
		return 1
	    }
	}
	return 0
}
    # (follows the help file - keys.html)
    # - displacement
    bind .f.c <Home> [list cursor-move Home]
    bind .f.c <End> [list cursor-move End]
    bind .f.c <Right> [list cursor-move 1]
    bind .f.c <Left> [list cursor-move -1]
    bind .f.c <Shift-Right> [list cursor-move 1b]
    bind .f.c <Shift-Left> [list cursor-move -1b]
    bind .f.c <Down> [list updn 1]
    bind .f.c <Up> [list updn 0]
    bind .f.c <j> jump
    # - symbol insert
    foreach i [list n z b g bar Q Z Return] {
	bind .f.c <$i> [list sym-add $i]
    }
    # - length change
    bind .f.c <plus> [list sym-len plus]
#    bind .f.c <asterisk> [list sym-len plus]
    bind .f.c <minus> [list sym-len minus]
#    bind .f.c <slash> [list sym-len minus]
    bind .f.c <period> [list sym-dot 1]
    bind .f.c <colon> [list sym-dot 2]
    # - pitch change
    bind .f.c <Next> [list sym-pitch -1]
    bind .f.c <Prior> [list sym-pitch 1]
    catch {bind .f.c <apostrophe> [list sym-pitch 7]}
    bind .f.c <quoteright> [list sym-pitch 7]
    bind .f.c <comma> [list sym-pitch -7]
    bind .f.c <asciicircum> [list sym-acc 1]
    bind .f.c <underscore> [list sym-acc 3]
    bind .f.c <equal> [list sym-acc 0]
    # - misc changes
    bind .f.c <space> [list sym-flags $flags(space)]
    bind .f.c <c> {.top2.chord invoke} ; # chord edit
    bind .f.c <e> detail-edit
    bind .f.c <f> pref-edit
    bind .f.c <h> header-edit
    bind .f.c <l> lyrics-edit
    bind .f.c <M> measure-set
    bind .f.c <R> [list focus .top.sym.e2]
    bind .f.c <t> tie-toggle
    bind .f.c <v> voice-edit
    bind .f.c <w> words-edit
    bind .f.c <parenleft> [list slur-toggle 0]
    bind .f.c <parenright> [list slur-toggle 1]
    # - deletion
    bind .f.c <Delete> sym-del
    bind .f.c <BackSpace> [list sym-del 1]
    # - play/record
    bind .f.c <k> [list .play.kbd invoke]
    if {[abc play]} {
#    if {$tune(midi) > 0}
	bind .f.c <m> [list .top2.mute invoke] ; # voice mute toggle
	bind .f.c <p> [list tune-play 0]
#	if {$tune(midi) > 1}
	    bind .f.c <r> [list .play.record invoke]
    }
    # - program control
    bind .f.c <question> help
    bind .f.c <F1> help
    bind .f.c <Control-l> screen-refresh
    bind .f.c <a> prevargs
    bind .f.c <d> detail-toggle
    bind .f.c <F> full-toggle
    bind .f.c <i> midi-edit
    bind .f.c <o> tune-load
    bind .f.c <O> tune-reload
    bind .f.c <P> preview
    bind .f.c <q> quit
    bind .f.c <s> tune-save
    bind .f.c <x> midi-export
#    bind .f.c <Insert> {set tune(overw) [expr {!$tune(overw)}]}
    # - selection and mark
    bind .f.c <Control-space> mark-set
    bind .f.c <Control-w> sel-delete
    bind .f.c <Control-y> \
	[list sel-insert 0 0 [expr {$opt(twosel) ? 2 : 3}]]
    bind .f.c <Control-x><Control-x> mark-exch
    bind .f.c <Escape><w> mark-copy
    if {$opt(twosel)} {
	bind .f.c <Control-c> clip-export
	bind .f.c <Control-v> [list sel-insert 0 0 1]
	bind .f.c <Control-x> {sel-delete; clip-export}
    } elseif {[string compare $tcl_platform(platform) {windows}] == 0} {
	bind .f.c <Control-c> mark-copy
	bind .f.c <Control-v> [list sel-insert 0 0 3]
	bind .f.c <Control-x> sel-delete
#fixme:what for mark-exch?
    }
    # - keypad
    if {[string compare $tcl_platform(platform) {windows}] != 0} {
	bind .f.c <KP_Home> [list cursor-move Home]
	bind .f.c <KP_End> [list cursor-move End]
	bind .f.c <KP_Right> [list cursor-move 1]
	bind .f.c <KP_Left> [list cursor-move -1]
	bind .f.c <Shift-KP_Right> [list cursor-move 1b]
	bind .f.c <Shift-KP_Left> [list cursor-move -1b]
	bind .f.c <KP_6> [list cursor-move 1b]
	bind .f.c <KP_4> [list cursor-move -1b]
	bind .f.c <KP_Next> [list sym-pitch -1]
	bind .f.c <KP_Prior> [list sym-pitch 1]
	bind .f.c <KP_Add> [list sym-len plus]
	bind .f.c <KP_Subtract> [list sym-len minus]
	bind .f.c <KP_Down> [list updn 1]
	bind .f.c <KP_Up> [list updn 0]
	bind .f.c <KP_Delete> sym-del
#	bind .f.c <KP_Insert> {set tune(overw) [expr {!$tune(overw)}]}
	# solaris
	if {$tcl_platform(os) == {SunOS}} {
	    bind .f.c <F27> [list cursor-move Home]
	    bind .f.c <F33> [list cursor-move End]
	    bind .f.c <F29> [list sym-pitch 1]
	    bind .f.c <F35> [list sym-pitch -1]
	    bind .f.c <F24> [list sym-len minus]
	    # (I remapped the keypad 'Del' to F9)
	    bind .f.c <F9> sym-del
	}
    }
    #other bindings
    bind .top2.mini <Return> [list jump-exec 0]
    bind .top2.mini <Escape> [list jump-exec 1]
    # - selection
    selection handle . sel-fetch
    bind mytag <Button-1> [list sel-start %x %y]
    bind mytag <B1-Motion> [list sel-extend %x %y]
    bind mytag <ButtonRelease-1> [list sel-stop %x %y]
    bind mytag <ButtonRelease-2> \
	[list sel-insert %x %y [expr {$opt(twosel) ? 2 : 3}]]
    bind mytag <ButtonRelease-3> [list sel-stop %x %y]
    if {[string compare $tcl_platform(platform) {windows}] != 0} {
	bind mytag <4> [list cursor-move -1]
	bind mytag <5> [list cursor-move 1]
    } else {
	bind mytag <MouseWheel> {if {%D > 0} {cursor-move -1} {cursor-move 1}}
    }
#    bind mytag <FocusIn> {puts {focusin}; set tune(cursym) [abc get]}
    # raw symbol
    bind .top.sym.e2 <Return> {sym-chg set; focus .f.c}
    bind .top.sym.e2 <Insert> {sym-chg insert; focus .f.c}
    if {[string compare $tcl_platform(platform) {windows}] != 0} {
	bind .top.sym.e2 <KP_Insert> {sym-chg insert; focus .f.c}
    }
    bind .top.sym.e2 <Escape> {detail-var; focus .f.c}
    # details
    bind .detail.gche <Return> [list detail-change gchord]
    bind .detail.gche <Escape> {detail-var; focus .f.c}
    bind .detail.dece <Return> [list detail-change deco]
    bind .detail.dece <Escape> {detail-var; focus .f.c}

    if {$opt(detail)} {
	pack .detail -after .top2 -side top -anchor w -fill x
    }

    catch {post-init}

tune-first
after 100 [list set tune(pref) 0]
focus .f.c
