#
# Change functions of the tclabc GUI.
#
# This file is part of the tclabc package.
# See the file tkabc.tcl for more information.
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

proc change-set {changed} {
# set the change flag and colorize accordingly
    global opt tune
    if {$tune(changed) != $changed} {
	set tune(changed) $changed
	if {$changed} {
	    .title.fname configure -background $opt(chg_bg)
	    .title.tune configure -background $opt(chg_bg)
	    .title.tune.menu entryconfigure [abc get tune] \
		-background $opt(chg_bg) -activebackground $opt(chg_bg)
	} else {
	    .title.fname configure -background $opt(nochg_bg)
	    .title.tune configure -background $opt(nochg_bg)
	    for {set i [.title.tune.menu index end]} {$i >= 0} {incr i -1} {
		.title.tune.menu entryconfigure $i \
		    -background {} -activebackground {}
	    }
	}
    } elseif {$changed} {
	set i [abc get tune]
	if {[string length [.title.tune.menu entrycget $i -background]] == 0} {
	    .title.tune configure -background $opt(chg_bg)
	    .title.tune.menu entryconfigure $i \
		-background $opt(chg_bg) -activebackground $opt(chg_bg)
	}
    }
}

proc detail-change {typ} {
# change some detail (gchord, deco)
    global detail
    abc set $typ $detail($typ)
    detail-var
    change-set 1
    switch $typ {
	deco {
#	    sym-redraw
	    cursor-display
	}
	default sym-long-redraw
    }
    focus .f.c
}

proc header-change {} {
# change the header fields
    global tune
#    abc set header [.header.t get 1.0 end-1l-1c]
    abc set header [.header.t get 1.0 end-1c]
    set i [abc get tune]
    set new_name [lindex [abc tune] $i]
    if {[string compare $tune(tname) $new_name] != 0} {
	.title.tune.menu entryconfigure $i -label $new_name
    }
    change-set 1
    tune-display
}

proc header-edit {} {
# edit the tune header
    global font lang tune
    set top .header
    if {[winfo exists $top]} {
	raise $top
	focus $top.t
	return
    }
    toplevel $top
    wm title $top "$lang(c_hd) $tune(fname) - $tune(tname)"
    text .header.t -width 72 -height 25 -font $font(f6) -bg linen
    .header.t insert end [abc get header]
    .header.t mark set insert 1.0
    frame .header.b
    button .header.b.ok -text $lang(repl) -underline 0 \
	-command {header-change; destroy .header; focus .f.c}
    button .header.b.abort -text $lang(cancel) -underline 0 \
	-command {destroy .header; focus .f.c}
    pack .header.b.ok .header.b.abort -side left -expand 1
    pack .header.t -side top
    pack .header.b -side top -fill x
    bind .header.t <Alt-r> {.header.b.ok invoke; break}
    bind .header.t <Alt-c> {.header.b.abort invoke; break}

    focus .header.t
}

proc lyrics-change {} {
# change the lyrics under staff
    global opt tune
    abc lyrics [split [.lyrics.t get 1.0 end-1c] "\n"]
    change-set 1
    if {$opt(detail)} {
	set s [abc go]
	abc go $tune(startsym)
	redisplay 0
	abc go $s
	cursor-display
    }
}

proc lyrics-edit {} {
# edit the lyrics under staff
    global lang tune
    set top .lyrics
    if {[winfo exists $top]} {
	raise $top
	focus $top.t
	return
    }
    toplevel $top
    wm title $top "$lang(c_ly) [abc get voice]"
    text .lyrics.t -width 80 -height 17 -bg linen \
	-wrap none -xscroll [list .lyrics.s set]
    scrollbar .lyrics.s -orient horizontal -width 12 \
	-command [list .lyrics.t xview]
    frame .lyrics.b
    button .lyrics.b.ok -text $lang(repl) -underline 0 \
	-command {lyrics-change; destroy .lyrics; focus .f.c}
    button .lyrics.b.abort -text $lang(cancel) -underline 0 \
	-command {destroy .lyrics; focus .f.c}
    pack .lyrics.b.ok .lyrics.b.abort -side left -expand 1
    pack .lyrics.t .lyrics.s .lyrics.b -side top -fill x
#    pack .lyrics.b -side top -fill x
    bind .lyrics.t <Alt-r> {.lyrics.b.ok invoke; break}
    bind .lyrics.t <Alt-c> {.lyrics.b.abort invoke; break}
    .lyrics.t insert end [join [abc lyrics] "\n"]
    .lyrics.t insert end "\n"
    .lyrics.t mark set insert 1.0
    focus .lyrics.t
}

proc measure-set {} {
# set/unset bars to have full measures
    abc set measure
    change-set 1
    sym-long-redraw
}

proc midi-channel-move {next} {
# go the the next or previous channel
    global channel
    if {$next} {
	if {[incr channel(channel)] > 32} {
	    set channel(channel) 1
	}
    } else {
	if {[incr channel(channel) -1] <= 0} {
	    set channel(channel) 32
	}
    }
    midi-prog-see
}

proc midi-channel-change {op} {
# set/add/del/toggle a channel in a voice
    global channel tune
    set curc $channel(channel)
    set insert insert
    set savs [abc go]
    abc go $channel(voice)
    set s [abc get channel]
    if {[string length $s] != 0} {
	abc go $s
	set c [lrange [abc get] 2 end]
	if {$channel(glob) ||
		[lindex [split [abc get time] .] 0] >= [lindex [split $tune(time) .] 0]} {
	    set insert set
	} else {
	    abc go $savs
	    abc go $channel(voice)
	    abc go prev
	}
    } else {
	# if no definition, the channel is the voice number plus one
	set c $channel(voice)
	incr c
	abc rewind
	abc go $channel(voice)
    }
    case $op {
	set {
	    set c $curc
	}
	add {
	    if {[set i [lsearch -exact $c $curc]] >= 0} return
	    lappend c $curc
	}
	del {
	    if {[set i [lsearch -exact $c $curc]] < 0} return
	    if {[llength $c] == 1} return
	    set c [lreplace $c $i $i]
	}
	toggle {
	    if {[set i [lsearch -exact $c $curc]] >= 0} {
		if {[llength $c] == 1} return
		set c [lreplace $c $i $i]
	    } else {
		lappend c $curc
	    }
	}
    }
    abc $insert [list midi channel [lsort -integer $c]]
    change-set 1
    # redisplay
    abc go $savs
    abc go $channel(voice)
    midi-edit
    abc go $savs
    if {[string compare $insert insert] == 0} {
	screen-refresh
    }
}

proc midi-destroy {} {
# destroy the midi window
    global channel
    destroy .midi
    unset channel
    focus .f.c
}

proc midi-edit {} {
# edit the MIDI parameters
    global abclib channel font iindex iname lang opt tcl_platform tune
    set savs [abc go]
    set top .midi
    if {[winfo exists $top]} {
	raise $top
    } else {
	toplevel $top
	wm title $top {MIDI parameters}

	frame .midi.v
	label .midi.v.l -text $lang(c_vcs) -font $font(f6)
	pack .midi.v.l -side top
	text .midi.v.t -font $font(f6) -background $opt(ent_bg) \
		-bd 0 -relief flat \
		-height [expr {$tune(nvoice) + 1}] -width 20
	.midi.v.t tag configure curv -background $opt(chg_bg)
	set channel(glob) 1
	checkbutton .midi.v.g -font $font(f6) -text $lang(c_glob) \
		-variable channel(glob)
	pack .midi.v.t .midi.v.g -side top
	button .midi.v.h -text $lang(m_help) -underline 0 \
		-command [list webdisp help:$abclib/instr.html]
	button .midi.v.b -text $lang(close) -underline 0 \
		-command {midi-destroy}
	pack .midi.v.b .midi.v.h -side bottom

	frame .midi.c
	label .midi.c.l -text $lang(c_ch) -font $font(f6)
	text .midi.c.t -font $font(f6) -background $opt(ent_bg) \
		-height 32 -width 32 -bd 0 -relief flat
	.midi.c.t tag configure selc -background $opt(chg_bg)
	.midi.c.t tag configure curc -background $opt(nochg_bg)
	pack .midi.c.l .midi.c.t -side top -expand y -fill x -anchor n

	frame .midi.p
	label .midi.p.l -text $lang(c_snd) -font $font(f6)
	frame .midi.p.f
	scrollbar .midi.p.f.s -orient vertical -width 12 \
		-command [list .midi.p.f.t yview]
	text .midi.p.f.t -font $font(f6) -background $opt(ent_bg) \
		-height 32 -width 32 -bd 0 -relief flat \
		-yscroll [list .midi.p.f.s set]
	.midi.p.f.t tag configure selp -background $opt(chg_bg)
	.midi.p.f.t tag configure curp -background $opt(nochg_bg)
	pack .midi.p.f.s .midi.p.f.t -side left -fill y
	pack .midi.p.l .midi.p.f -side top -expand y -fill x -anchor n

	pack .midi.v .midi.c .midi.p -side left -expand y -fill y -anchor n

	set channel(voice) 0
	set channel(channel) 1
	bind .midi <Right> [list midi-voice-move 1]
	bind .midi <Left> [list midi-voice-move 0]
	bind .midi <Down> [list midi-channel-move 1]
	bind .midi <Up> [list midi-channel-move 0]
	bind .midi <Next> [list midi-prog-move 1]
	bind .midi <Prior> [list midi-prog-move 0]
	bind .midi <c> {midi-destroy; break}
	bind .midi <g> {set channel(glob) [expr {!channel(glob)}]}
	bind .midi <q> {midi-destroy; break}
	bind .midi <h> [list webdisp help:$abclib/instr.html]
	bind .midi <question> [list webdisp help:$abclib/instr.html]
	bind .midi <space> [list midi-channel-change set]
	bind .midi <plus> [list midi-channel-change add]
	bind .midi <minus> [list midi-channel-change del]
	bind .midi <Return> [list midi-prog-change curp.first]
	bind .midi.v.t <ButtonRelease-1> {
	    set v [lindex [split [.midi.v.t index current] .] 0]
	    incr v -1
	    midi-voice $v
	}
	bind .midi.c.t <ButtonRelease-1> {
	    set channel(channel) [lindex [split [.midi.c.t index current] .] 0]
	    midi-channel-change set
	}
	bind .midi.c.t <Shift-ButtonRelease-1> {
	    set channel(channel) [lindex [split [.midi.c.t index current] .] 0]
	    midi-channel-change toggle
	}
	bind .midi.p.f.t <ButtonRelease-1> [list midi-prog-change current]
	if {$tcl_platform(platform) == {unix}} {
	    bind .midi <KP_Right> [list midi-voice-move 1]
	    bind .midi <KP_Left> [list midi-voice-move 0]
	    bind .midi <KP_Down> [list midi-channel-move 1]
	    bind .midi <KP_Up> [list midi-channel-move 0]
	    bind .midi <KP_Next> [list midi-prog-move 1]
	    bind .midi <KP_Prior> [list midi-prog-move 0]
	    bind .midi <KP_Add> [list midi-channel-change add]
	    bind .midi <KP_Subtract> [list midi-channel-change del]
	    bind .midi <KP_Enter> [list midi-prog-change curp.first]
	}
	# set the voice names
	for {set i 0} {$i <= $tune(nvoice)} {incr i} {
	    abc go $i
	    set vname [abc voice get]
	    if {[string length [lindex $vname 1]] != 0} {
		set vname [lindex $vname 1]
	    } else {
		set vname [lindex $vname 0]
	    }
	    .midi.v.t insert end [format "%-20.20s\n" $vname]
	}
	.midi.v.t configure -state disabled
	# set the program names
	foreach i $iindex j $iname {
	    .midi.p.f.t insert end [format "%6s %-25s\n" $i $j]
	}
	.midi.p.f.t configure -state disabled
    }
    # update the channels
    .midi.c.t configure -state normal
    .midi.c.t delete 1.0 end
    set i 1
    foreach s [abc get program] {
	if {[string length $s] == 0} {
	    set p 0
	} else {
	    abc go $s
	    # midi program [channel [bank]] [bank-]prog
	    set sym [abc get]
	    if {[llength $sym] == 5} {
		set b [lindex $sym 3]
	    } else {
		set b 0
	    }
	    set c [lindex $sym end]
	    if {[string first - $c ] < 0} {
		set c $b-$c
	    }
	    set p [lsearch -exact $iindex $c]
	    if {$p < 0} {
		set p 0
	    }
	}
	set channel($i) $p
	.midi.c.t insert end [format "%2d %s\n" $i [lindex $iname $p]]
	incr i
    }
    .midi.c.t configure -state disabled
    midi-prog-see
    abc go $savs
    midi-voice [abc get voice]
    focus $top
}

proc midi-export {} {
# create a midi file without changing the file name
    global lang tune
    set fs { {{midi} {.mid}} {{all} *}}
    set fn [tk_getSaveFile -defaultextension {.mid} \
	-filetypes $fs -initialfile "[file rootname $tune(fname)].mid" \
	-title $lang(m_midiexp)]
    if {[string length $fn] == 0} return
    abc midisave $fn
}

proc midi-prog-change {mark} {
# change the program of a channel
    global channel iindex tune
    set savs [abc go]
    set p [lindex [split [.midi.p.f.t index $mark] .] 0]
    incr p -1
    # search the set/insertion point
    set c $channel(channel)
    incr c -1
    set s [lindex [abc get program] $c]
    set insert insert
    if {[string length $s] != 0} {
	abc go $s
	if {$channel(glob) ||
		[lindex [split [abc get time] .] 0] >= [lindex [split $tune(time) .] 0]} {
	    set insert set
#fixme: do not change if same program
	} else {
	    abc go $savs
	    abc go $channel(voice)
	    abc go prev
	}
    } else {
	abc rewind
	abc go $channel(voice)
    }
    # set/insert the program
    foreach {b p} [split [lindex $iindex $p] -] break
    if {[string length [abc get channel]] <= 1} {
	if {$b == 0} {
	    abc $insert [list midi program $p]
	} else {
	    abc $insert [list midi program $b-$p]
	}
    } else {
	abc $insert [list midi program $channel(channel) $b-$p]
    }
    change-set 1
    # redisplay
    abc go $savs
    abc go $channel(voice)
    midi-edit
    if {[string compare $insert insert] == 0} {
	screen-refresh
    }
    abc go $savs
}

proc midi-prog-move {next} {
# go the the next or previous MIDI program
    global iindex
    set p [lindex [split [.midi.p.f.t index curp.first] .] 0]
    catch {.midi.p.f.t tag remove curp 1.0 end}
    if {$next} {
	if {[incr p] > [llength $iindex]} {
	    set p 1
	}
    } else {
	if {[incr p -1] <= 0} {
	    set p [llength $iindex]
	}
    }
    .midi.p.f.t tag add curp $p.0 $p.3
    .midi.p.f.t see $p.0
}

proc midi-prog-see {} {
# select a channel and highlight the associated program
    global channel
    set c $channel(channel)
    catch {.midi.c.t tag remove curc 1.0 end}
    .midi.c.t tag add curc $c.0 $c.3
    set i $channel($c)
    incr i 1
    catch {.midi.p.f.t tag remove curp 1.0 end}
    .midi.p.f.t tag add curp $i.0 $i.6
    catch {.midi.p.f.t tag remove selp 1.0 end}
    .midi.p.f.t tag add selp $i.6 $i.end
    .midi.p.f.t see $i.0
}

proc midi-voice {v} {
# select a voice in the midi parameters
    global channel
    set savs [abc go]
    abc go $v
    set s [abc get channel]
    if {[string length $s] != 0} {
	abc go $s
	set c [lrange [abc get] 2 end]
    } else {
	set c $v
	incr c
    }
    catch {.midi.c.t tag remove selc 1.0 end}
    foreach i $c {
	.midi.c.t tag add selc $i.3 $i.end
    }
    set channel(voice) $v
    incr v
    catch {.midi.v.t tag remove curv 1.0 end}
    .midi.v.t tag add curv $v.0 $v.end
    set channel(channel) [lindex $c 0]
    midi-prog-see
    abc go $savs
}

proc midi-voice-move {next} {
# go to the next or previous voice
    global channel tune
    set v $channel(voice)
    if {$next} {
	if {[incr v] > $tune(nvoice)} {
	    set v 0
	}
    } else {
	if {[incr v -1] < 0} {
	    set v $tune(nvoice)
	}
    }
    midi-voice $v
}

proc prevargs {} {
# change the preview arguments
    global font opt tune
    set top .prev
    if {[winfo exists $top]} {
	raise $top
	focus $top.e
	return
    }
    toplevel $top
    wm title $top {Preview arguments}
#    frame .prev.f
    label .prev.l -font $font(f6) -text {abc(m)2ps arguments: }
    entry .prev.e -relief sunken -width 50 -background $opt(ent_bg) \
	-textvariable tune(preview-args)
    pack .prev.l .prev.e -side left
    bind .prev.e <Return> {destroy .prev; focus .f.c; break}
    focus .prev.e
    tkwait window .prev
}

proc preview {} {
# preview the current tune
    global opt tune
    set abcfile $opt(tmpdir)/tmp.abc
    set fd [open $abcfile {w}]
    puts $fd [abc dump]
    close $fd
    set npsfile [file nativename $opt(tmpdir)/tmp.ps]
    set prg "exec {$opt(abc2ps)} $opt(abc2ps-args) $tune(preview-args) {[file nativename $abcfile]} -O {$npsfile}"
    catch {eval $prg} rc
    tk_messageBox -type ok -message "abc(m)2ps result\n$rc" \
	-title {abc(m)2ps result}
    exec $opt(ghostview) $npsfile &
#    exec rm $abcfile $psfile
}

proc slur-toggle {stop} {
# toggle starting/ending a slur
    global opt tune
    set sym [abc get]
    switch [lindex $sym 0] {
	note - grace {}
	default return
    }
    set slurs [abc get slurs]
    if {[llength $slurs] == 0} {
	set n [llength $sym]
	for {set i 0} {$i < $n} {incr i} {
	    lappend slurs 0
	}
    }
    set v [expr {$stop ? 1 : 3}]
    if {$tune(chord)} {
	incr stop $tune(chordx)
    }
    set new [expr {[lindex $slurs $stop] ? 0 : $v}]
    abc set slurs [lreplace $slurs $stop $stop $new]
    sym-redraw
    change-set 1
    cursor-display
}

proc sym-acc {acc} {
# change the accidental for a note
    global tune
# !! for the 'acc' value, see abcparse.h !!
    set sym [abc get]
    # type len (pitch acc)*
    switch [lindex $sym 0] {
	note - grace {}
	default return
    }
    set x [expr $tune(chordx) + 1];	# acc index in symbol
    if {$acc == 0} {
	# change no acc <-> natural sign
	switch -- [lindex $sym $x] {
	    0 {set acc 2}
	    2 {set acc 0}
	}
    }
    abc set [lreplace $sym $x $x $acc]
    sym-redraw
    change-set 1
    cursor-display
}

proc sym-add {type} {
# add a symbol
    global selection tune voice
#fixme: if first note, set the default values
    set l $tune(len)
    set p 21
    switch $type {
	n - g {
	    # if in a chord, add a new head
	    set sym [abc get]
	    if {[string compare [lindex $sym 0] EOT] == 0} {
		# at EOT, duplicate the previous symbol
		abc go prev
		set sym [abc get]
	    }
	    if {$tune(chord) \
		    && ([string compare [lindex $sym 0] note] == 0 \
			|| [string compare [lindex $sym 0] grace] == 0)} {
		set pitch [lindex $sym $tune(chordx)]
		incr pitch 2
		abc set [linsert $sym $tune(chordx) $pitch 0]
	    } else {
		# go to the previous note to find the pitch
		set s [abc go]
		while {1} {
		    switch [lindex $sym 0] {
			note - grace {
			    # type len (pitch acc)*
#			    set l [lindex $sym 1]
			    set p [lindex $sym 2]
			    break
			}
			EOT {
			    break
			}
		    }
		    abc go prev
		    set sym [abc get]
		}
		abc go $s
		if {[string compare $type n] == 0} {
		    set t note
		} else {
		    set t grace
		}
		abc insert [list $t $l $p 0]
	    }
	}
	z {
	    abc insert [list rest $l]
	}
	Z {
	    abc insert [list mrest 2]
	}
	b - bar {
	    abc insert [list bar 1 {}]
	}
	Q {
	    abc insert [list tempo {} $l 0 0 0 80 {}]
	}
	Return {
	    abc insert [list extra newline]
	}
	default return
    }
    change-set 1
    if {$selection(rect) != 0} {
	.f.c delete $selection(rect)
	set selection(rect) 0
    }
    set s [abc go]
    abc go prev
    redisplay 0
    abc go $s
    cursor-display
}

proc sym-chg {action} {
# change the symbol raw value
    global selection tune voice
#    set a $tune(cursym)
#    abc $action [eval list $a]
    abc $action [split $tune(cursym)]
    change-set 1
    if {$selection(rect) != 0} {
	.f.c delete $selection(rect)
	set selection(rect) 0
    }
    if {[string compare $action insert] == 0} {
	set s [abc go]
	abc go prev
	redisplay 0
	abc go $s
	cursor-display
    } else {
	sym-long-redraw
    }
}

proc sym-del {{back 0}} {
# delete the current symbol / selection
    global selection tune
    # if selection active and last symbol, delete the selection
    if {$selection(rect) != 0} {
	if {[string compare [lindex $selection(syms) end] [abc go]] == 0} {
	    sel-delete
	    return
	}
	.f.c delete $selection(rect)
	set selection(rect) 0
    }
    # if in chord, delete the head
    set sym [abc get]
    if {$tune(chord) \
	    && ([string compare [lindex $sym 0] note] == 0 \
		|| [string compare [lindex $sym 0] grace] == 0) \
	    && [llength $sym] > 4} {
	    abc set [lreplace $sym $tune(chordx) [expr {$tune(chordx) + 1}]]
	    if {$tune(chordx) >= [llength $sym] - 2} {
		set tune(chordx) [expr {[llength $sym] - 4}]
	    }
	    sym-redraw
    } else {
	.f.c delete [abc go]
	abc delete
	set s [abc go prev]
	redisplay 0
	abc go $s
	if {!$back} {
	    abc go next
	}
	cursor-display
    }
    change-set 1
}

proc sym-dot {ndots} {
# change the number of dots of a note / rest
    set sym [abc get]
    switch [lindex $sym 0] {
	note - rest - grace {}
	default return
    }
    set len [lindex $sym 1]
    set set_it 1
    if {$len % 9 == 0} {
	set len [expr {$len / 3 * 2}]
	if {$ndots == 1} {
	    set set_it 0
	}
    } elseif {$len % 7 == 0} {
	set len [expr {$len / 7 * 4}]
	if {$ndots == 2} {
	    set set_it 0
	}
    }
    if {$set_it} {
	if {$ndots == 1} {
	    set len [expr {$len / 2 * 3}]
	} else {
	    set len [expr {$len / 4 * 7}]
	}
    }
    # (the length is the second list item)
    abc set [lreplace $sym 1 1 $len]
    change-set 1
    sym-long-redraw
}

proc sym-flags {val} {
# toggle the symbol flags (only 'space' for now)
    set f [abc get flags]
    abc set flags [expr {$f ^ $val}]
    sym-redraw 1
    change-set 1
    cursor-display
}

proc sym-len {newlen} {
# change the length of a symbol
    global opt selection tune
    # if the symbol is the last of the selection,
    # change the length of all notes / rests
    if {$selection(rect) != 0 &&
	!$tune(chord) &&
	[string compare [lindex $selection(syms) end] [abc go]] == 0} {
	foreach s $selection(syms) {
	    abc go $s
	    set sym [abc get]
	    switch [lindex $sym 0] {
		note - rest {
		    # 'note' len (pitch acc)*
		    set oldlen [lindex $sym 1]
		    switch $newlen {
			plus {
			    set newlen [expr {$oldlen * 2}]
			    if {$newlen > 3072} continue
			}
			minus {
			    set newlen [expr {$oldlen / 2}]
			    if {$newlen < 24} continue
			}
		    }
		    abc set [lreplace $sym 1 1 $newlen]
		}
	    }
	}
	change-set 1
	clip-update
	foreach s $selection(syms) {
	    if {[llength [.f.c find withtag $s]] != 0} {
		abc go $s
		break
	    }
	}
	redisplay 0
	abc go [lindex $selection(syms) end]
	cursor-display
	return
    }
    set sym [abc get]
    switch [lindex $sym 0] {
	clef {
	    # 'clef' clef line
	    set line [lindex $sym 2]
	    switch $newlen {
		plus {
		    incr line 1
		    if {$line > 5} {
			set line 1
		    }
		}
		minus {
		    incr line -1
		    if {$line <= 0} {
			set line 5
		    }
		}
	    }
	    abc set [lreplace $sym 2 2 $line]
	    change-set 1
	    sym-long-redraw
	    return
	}
	time {
	    # 'time' num div
	    switch [lindex $sym 1] {
		C - C| - none {
		    set sym [list time 4 4]
		}
	    }
	    set div [lindex $sym 2]
	    switch $newlen {
		plus {
		    set div [expr {$div << 1}]
		    if {$div > 32} {
			set div 32
		    }
		}
		minus {
		    set div [expr {$div >> 1}]
		    if {$div <= 0} {
			set div 1
		    }
		}
		default return
	    }
	    abc set [lreplace $sym 2 2 $div]
	    sym-redraw
	    change-set 1
	    cursor-display
	    return
	}
	note - rest - grace {
	    set oldlen [lindex $sym 1]
	}
	tempo {
	    set oldlen [lindex $sym 2]
	}
	extra {
	    switch [lindex $sym 1] {
		blen {
		    # 'extra' 'blen' value
		    set oldlen [lindex $sym 2]
		}
		default return
	    }
	}
	mrest {
	    # 'mrest' len
	    set len [lindex $sym 1]
	    switch $newlen {
		plus {
		    incr len 1
		}
		minus {
		    incr len -1
		    if {$len <= 0} return
		}
	    }
	    abc set [lreplace $sym 1 1 $len]
	    sym-redraw
	    change-set 1
	    cursor-display
	    return
	}
	default return
    }
    switch $newlen {
	plus {
	    set newlen [expr {$oldlen * 2}]
	    if {$newlen > 3072} return
	}
	minus {
	    set newlen [expr {$oldlen / 2}]
	    if {$newlen < 24} return
	}
    }
    # (the length is the second list item for all but 'extra')
    if {[string compare [lindex $sym 0] extra] != 0} {
	abc set [lreplace $sym 1 1 $newlen]
    } else {
	abc set [lreplace $sym 2 2 $newlen]
    }
    set s [abc go]
    if {$opt(detail)} {
	# beaming may change
	abc go prev
    }
    change-set 1
    redisplay 0
    abc go $s
    cursor-display
}

proc sym-long-redraw {} {
# redraw the current symbol and the next ones
    set s [abc go]
    redisplay 0
    abc go $s
    cursor-display
}

proc sym-pitch {delta} {
# change the pitch of a note (or some other parameter for an other symbol)
    global bartypes selection tune
    # if the symbol is the last of the selection,
    # change the pitches of all notes
    if {$selection(rect) != 0 &&
	!$tune(chord) &&
	[string compare [lindex $selection(syms) end] [abc go]] == 0} {
	foreach s $selection(syms) {
	    abc go $s
	    set sym [abc get]
	    switch [lindex $sym 0] {
		note - grace {
		    # 'note' len (pitch acc)*
		    for {set i 2} { $i < [llength $sym]} {incr i 2} {
			set p [lindex $sym $i]
			incr p $delta
			set sym [lreplace $sym $i $i $p]
		    }
		    abc set $sym
		}
	    }
	}
	change-set 1
	clip-update
	foreach s $selection(syms) {
	    if {[llength [.f.c find withtag $s]] != 0} {
		abc go $s
		break
	    }
	}
	redisplay 0
	abc go [lindex $selection(syms) end]
	cursor-display
	return
    }
    set sym [abc get]
    switch [lindex $sym 0] {
	note - grace {
	    # 'note' len (pitch acc)*
	    if {$tune(chord)} {
		set p [lindex $sym $tune(chordx)]
		incr p $delta
		set sym [lreplace $sym $tune(chordx) $tune(chordx) $p]
	    } else {
		for {set i 2} { $i < [llength $sym]} {incr i 2} {
		    set p [lindex $sym $i]
		    incr p $delta
		    set sym [lreplace $sym $i $i $p]
		}
	    }
	    abc set $sym
	    sym-redraw 1
	    cursor-display
	}
	bar {
	    # 'bar' bartype repeat number
	    set p [lindex $sym 1]
	    if {[set i [lsearch -exact $bartypes $p]] >= 0} {
		incr i $delta
		if {$i >= [llength $bartypes]} {
		    set i 0
		} elseif {$i < 0} {
		    set i [llength $bartypes]
		    incr i -1
		}
	    } else {
		set i 0
	    }
	    abc set [lreplace $sym 1 1 [lindex $bartypes $i]]
	    sym-long-redraw
	}
	clef {
	    # 'clef' clef line
	    set clef [expr {([lindex $sym 1] + $delta) % 4} ]
	    set line [lindex [list 2 3 4 2] $clef]
	    abc set [list clef $clef $line]
	    sym-long-redraw
	}
	time {
	    # 'time' num div ?num div?
	    switch [lindex $sym 1] {
		C - C| - none {
		    set sym [list time 4 4]
		}
	    }
	    set num [lindex $sym 1]
	    incr num $delta
	    if {$num > 16} {
		set num 1
	    } elseif {$num < 1} {
		set num 16
	    }
	    abc set [lreplace $sym 1 1 $num]
	    sym-long-redraw
	}
	tempo {
	    # 'tempo' string1 len0 len1 len2 len3 speed string2
	    set speed [lindex $sym 6]
	    incr speed $delta
	    if {$speed > 200} {
		set speed 200
	    } elseif {$speed < 40} {
		set speed 40
	    }
	    abc set [lreplace $sym 6 6 $speed]
	    sym-long-redraw
	}
	key {
	    # 'key' number_of_sharps_flats
	    set sf [lindex $sym 1]
	    if {[string compare $sf "none"} return
	    incr sf $delta
	    if {$sf > 7} {
		set sf -7
	    } elseif {$sf < -7} {
		set sf 7
	    }
	    abc set [list key $sf]
	    sym-long-redraw
	}
	default return
    }
    change-set 1
}

proc sym-redraw {{beam 0}} {
# redisplay a symbol at the same place and update the beam if needed
    global flags opt
    set s [abc go]
    abc go [lindex [abc get staff] 1]
    set-clef
    abc go $s
    set x [get-xoffset]
    .f.c delete $s
    sym-draw $x
    if {!$beam || !$opt(detail)} return
    if {[abc get flags] & $flags(word_e)} return
    # search the end of word
    abc go next
    while {1} {
	if {[abc get flags] & $flags(word_e)} break
#fixme: stop on EOT - is it needed?
	if {[string compare [lindex [abc get] 0] EOT] == 0} {
	    abc go $s
	    return
	}
	abc go next
    }
    set x [get-xoffset]
    .f.c delete [abc go]
    sym-draw $x
    abc go $s
}

proc tie-toggle {} {
# toggle the tie of a note/chord
    global flags opt tune
    set sym [abc get]
    if {[string compare [lindex $sym 0] note] != 0} return
    set l [expr {([llength $sym] - 2) / 2}]	;# number of notes in chord
    set t [abc get ties]
    # if chord editing, toggle only one note
    if {$tune(chord)} {
	if {[llength $t] == 0} {
	    for {set i 0} {$i < $l} {incr i} {
		lappend t 0
	    }
	}
	set j [expr {($tune(chordx) - 2) / 2}]
	set newt [expr {[lindex $t $j] ? 0 : 3}]
	set t [lreplace $t $j $j $newt]
    } else {
	# not chord editing
	if {[llength $t] == 0} {
	    set t {}
	    for {set i 0} {$i < $l} {incr i} {
		lappend t 3
	    }
	} else {
	    set t {}
	}
    }
    abc set ties $t
    if {$opt(detail)} {
	set s [abc go]
	while {1} {
	    abc go next
	    switch [lindex [abc get] 0] {
		EOT break
		note {
		    sym-redraw
		    break
		}
	    }
	}
	abc go $s
    }
    change-set 1
    cursor-display
}

proc tune-del {} {
# delete the current tune
    global tune
    abc tune del
    set tune(nvoice) [abc voice]
    change-set 1
    tune-display
}

proc tune-new {} {
# add a new tune to the current file
    global tune
    abc tune new
    set t [abc tune]
    set i [abc get tune]
    .title.tune.menu insert $i command -label [lindex $t $i] \
	-command [list abc-select $i]
    set tune(nvoice) [abc voice]
    change-set 1
    tune-display
}

proc tune-save {} {
# save a tune
    global tune
    if {!$tune(changed)} return
    if {[string compare $tune(fname) {noname.abc}] == 0} {
	tune-saveas
	return
    }
    tune-savef
}

proc tune-saveas {} {
# save a tune under a different name
    global filesel tune
    set fn [tk_getSaveFile -defaultextension [file extension $tune(fname)] \
		-filetypes $filesel -initialfile $tune(fname) \
		-title {Save As}]
    if {[llength $fn] == 0} return
    set tune(fname) $fn
    tune-savef
}

proc tune-savef {} {
# force a tune save
    global tune
    if {[file exists $tune(fname)]} {
        file rename -force $tune(fname) $tune(fname)~
    }
    switch -- [file extension $tune(fname)] {
	.mid {
	    abc midisave $tune(fname)
	}
	.abc {
	    set fd [open $tune(fname) {w}]
	    puts -nonewline $fd [abc dump]
	    close $fd
	}
    }
    change-set 0
}

proc voice-change {} {
# change the voice characteristics
    global tune vc
    abc voice set [list $vc(n) $vc(fn) $vc(nn)]
    unset vc
    destroy .voice
    change-set 1
    voice-display
}

proc voice-edit {} {
# edit the characteristics of the current voice
    global vc lang opt tune
    set top .voice
    if {[winfo exists $top]} {
	raise $top
#	tkTabToWindow $top.n.e
	focus $top.n.e
	return
    }
    foreach a [list n fn nn] b [abc voice get] {
	set vc($a) $b
    }
    toplevel $top
    wm title $top "$lang(c_vc) $tune(vname)"
    set label_w 10
    set entry_w 30
    # items
    foreach a [list n fn nn] \
	    b [list $lang(c_nam) $lang(c_fnam) $lang(c_nnam)] {
	set c $top.$a
	frame $c
	label $c.l -width $label_w -anchor e -text $b
	entry $c.e -relief sunken -width $entry_w -background $opt(ent_bg) \
		-textvariable vc($a)
	pack $c.l $c.e -side left
    }
    # window control on the bottom
    set c $top.c
    frame $c
    button $c.ok -text OK -command voice-change
    button $c.abort -text $lang(cancel) \
        -command [list destroy $top]
    pack $c.ok $c.abort -side left
# -expand 1
    pack $top.n $top.fn $top.nn $top.c -side top
#    tkTabToWindow $top.n.e
    focus $top.n.e
}

proc voice-new {} {
# add a new voice to the current tune
    global tune
    set nv $tune(nvoice)
    incr nv 2
    abc voice new [list $nv {} {}]
    set tune(nvoice) [abc voice]
    change-set 1
    tune-display
    abc go $tune(nvoice)
    sym-sel
}

proc words-change {} {
# change the words fields
    abc set words [.words.t get 1.0 end-1c]
    focus .f.c
    change-set 1
}

proc words-edit {} {
# edit the words (lyric after tune)
    global lang tune
    if {[winfo exists .words]} {
	raise .words
	focus .words.t
	return
    }
    toplevel .words
    wm title .words "$lang(c_wrd) $tune(fname) - $tune(tname)"
    text .words.t -width 72 -height 25 -bg linen
    .words.t insert end [abc get words]
    .words.t mark set insert 1.0
    frame .words.b
    button .words.b.ok -text $lang(repl) -underline 0 \
	-command {words-change; destroy .words}
    button .words.b.abort -text $lang(cancel) -underline 0 \
	-command {destroy .words}
    pack .words.b.ok .words.b.abort -side left -expand 1
    pack .words.t -side top
    pack .words.b -side top -fill x
    bind .words.t <Alt-r> {.words.b.ok invoke; break}
    bind .words.t <Alt-c> {.words.b.abort invoke; break}

    focus .words.t
}
