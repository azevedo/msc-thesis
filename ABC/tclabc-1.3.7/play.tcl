#
# Playing functions of the tclabc GUI.
#
# This file is part of the tclabc package.
# See the file tkabc.tcl for more information.
#
# Copyright (C) 1999-2006, Jean-François Moine.
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

proc note-disp {} {
# redisplay the score after insertion from MIDI in or the virtual keyboard
    set s [abc go]
#fixme: pb when insert while displaying...
    abc go prev
    redisplay 0
    abc go $s
    change-set 1
    cursor-display
}

proc midi-record {} {
# start/stop MIDI recording
    set recording [abc record]
    if {$recording == 2} {
	# display the MIDI sequence
	cursor-display
	change-set 1
	set recording 0
    }
#    .play.record configure -image record-[expr {$opt(kbd) ? "stop" : "start"}]
    .play.record configure -relief [expr {$recording ? "sunken" : "raised"}]
}

proc play-show {} {
# show where the music is playing
    global opt play score tune
    if {$play(bar) != 0} {
	.f.c delete $play(bar)
	set play(bar) 0
    }
    set s [abc play which]
    if {[string compare $s none] == 0} {
	set opt(detail) $play(detail)
	.play.play configure -image play
	abc go $play(s)
	unset play
	sym-sel
	return
    }
    set play(s) $s
    if {!$opt(follow)} {
	after 500 play-show
	return
    }
    # display the symbol in the canvas
    abc go $s
    if {[llength [.f.c find withtag $s]] == 0} {
	redisplay 1
	abc go $s
    }
    set x1 [expr {[get-xoffset] - 1}]
    set play(bar) [.f.c create rectangle [expr {$x1 - 2}] 10 \
	$x1 [expr {$score(height) * ($tune(nvoice) + 1) - 5}] \
	-outline {} -fill red]
    after 500 play-show
}

proc program-set {} {
# set the MIDI program for the current voice
    global iindex iname tune
    set p [lsearch -exact $iname $tune(program)]
    abc set program [lindex $iindex $p]
    change-set 1
}

proc tempo-set {new_tempo} {
# change the default tempo
    global tune
    abc play tempo $new_tempo
    set tune(pref) 1
}

proc tune-play {rewind} {
# start/stop playing the tune
    global opt play tune
    if {[string compare [abc play which] none] == 0} {
	set play(detail) $opt(detail)
	set play(bar) 0
	set play(s) [abc go]
	set opt(detail) 0
	if {$rewind} {
	    abc rewind
	}
	abc play tune
	.play.play configure -image stop
	after 500 play-show
    } else {
	abc play stop
    }
}

proc vel-set {new_vel} {
# change the velocity
    global tune
    abc play velocity $new_vel
    set tune(pref) 1
}

# -- virtual piano keyboard adapted from
#	Andreas Leitgeb <avl@logic.at> 'piano.tcl' --

#  You may use this program freely.

proc vk-blackkey {w p} {
   label $w -text {} -background black -foreground white \
	-borderwidth 2 -relief raised 
   place $w -relx $p -y 0 -relwidth 0.1 -relheight 0.6 -anchor n
   bindtags $w key
}

# Event-handler procedures
proc vk-button {w vel} {
# virtual keyboard button press
    foreach {d o p} [split [winfo name $w] {}] break
    set pit [expr {$o * 12 + "0x$p" + 36}]
    abc midi note [list $pit $vel]
    if {$vel != 0} {
	$w conf -relief sunken
    } else {
	$w conf -relief raised
    }
}

proc vk-key {k vel} {
# get an event from the computer keyboard
    global tcl_platform
#puts "k:$k"
    switch -glob $tcl_platform(machine) {
	intel - i486 - i586 - i686 {
	    incr k -10
	    set o [expr {3 - ($k / 14)}]
	    set pit [expr {$k % 14}]
	}
	sun* {
	    if {$k < 38 + 23} {
		incr k
	    }
	    incr k -38
	    set o [expr {3 - ($k / 23)}]
	    set pit [expr {$k % 23}]
	}
	default {
	    puts "Unknown keyboard $tcl_platform(machine)"
	    return
	}
    }
    if {$pit >= 12} {
	incr pit -12
	incr o
    }
    if {$o < 0 || $o > 3} return
    incr pit [expr {$o * 12 + 36}]
    abc midi note [list $pit $vel]
}

proc vk-start {} {    
# build the keyboard-window
    global midistep
    set t .piano
    if {[winfo exists $t]} {
	vk-stop
	return
    }
    toplevel $t
    wm title $t {Virtual keyboard}
    set start 0		; #starting octava
    set end 5		; #ending octava
    set rw [expr {1. / ($end - $start)}]
    wm geometry $t [expr {($end - $start) * 120}]x70
    for {set o $start} {$o < $end} {incr o} {
	set w $t.f$o
	frame $w -height 70 -width 150
	# the window name is '"o"octava pitch accidental'
	vk-whitekey $w.o${o}0 0.0
	vk-blackkey $w.o${o}1 0.143
	vk-whitekey $w.o${o}2 0.143
	vk-blackkey $w.o${o}3 0.286
	vk-whitekey $w.o${o}4 0.286
	vk-whitekey $w.o${o}5 0.429
	vk-blackkey $w.o${o}6 0.571
	vk-whitekey $w.o${o}7 0.571
	vk-blackkey $w.o${o}8 0.714
	vk-whitekey $w.o${o}9 0.714
	vk-blackkey $w.o${o}a 0.857
	vk-whitekey $w.o${o}b 0.857
	place $w -y 0 -relheight 1.0 -relx [expr {($o - $start) * $rw}] -relwidth $rw
    }
    focus $t
    # Key and Mousebindings
    bind key <1> [list vk-button %W 64]
    bind key <ButtonRelease-1> [list vk-button %W 0]
#    bind $t <KeyPress> {puts "%%k:%k %%K:%K %%A:%A %%t:%t"}
    bind $t <KeyPress> [list vk-key %k 64]
    bind $t <KeyRelease> [list vk-key %k 0]
    bind $t <Escape> vk-stop
    wm protocol $t WM_DELETE_WINDOW vk-stop
    # activate MIDI step recording
    abc midi handler note-disp
    set midistep 1	; # avoid playing
    .play.kbd configure -relief sunken
}

proc vk-stop {} {
# remove the display handler for no stepping from MIDI in
    global midistep
    destroy .piano
    abc midi handler
    unset midistep
    .play.kbd configure -relief raised
}

proc vk-whitekey {w p} {
   label $w -text {} -background white -foreground black \
	-borderwidth 2 -relief raised 
   place $w -relx $p -y 0 -relwidth 0.143 -relheight 1
   bindtags $w key
   lower $w
}
