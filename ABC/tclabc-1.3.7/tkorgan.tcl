# small organ with tclabc
#
# Usage:
#	- you must have MIDI input (keyboard) and ouput
#	- do them work with tkabc (this program uses its MIDI devices)
#	- adjust the GUI according to your sound font (the actual GUI
#		is defined for Jeux14.sf2)
#	- source this file in wish
#	- the mouse and computer keys toggle the stops
#	- the MIDI channels are;
#		0: grand orgue (great)
#		1: recit (swell)
#		2: positif (positive)
#		3: pedale (pedal)
#	- press Escape to exit.
#
# Copyright (C) 2006, Jean-François Moine.
#
proc org-gui {} {
# create the stop buttons
    global organ
    # pedale
    set stolis [list\
 0-110 {Principal 16}\
 0-111 {Subbasso 16}\
 0-102 {Bombarde 16}\
 0-116 {Fagotto 8}\
]
    set shcut [list\
	F1 F2 F3 F4\
]
    frame .ped
    label .ped.lab -text {Pédale}
    frame .ped.st
    stops .ped.st 3 $stolis 8 $shcut
    pack .ped.lab .ped.st -side top -fill x
    frame .kbd
    # grand orgue
    set stolis [list\
 0-15  {Bourdon 16}\
 0-19  {Gamba 8}\
 0-0   {Montre 8}\
 0-64  {Salicionale 8}\
 0-53  {Rohrflte 8}\
 0-1   {Prestant 4}\
 0-54  {Rohrflte 4}\
 0-18  {Open Flute 4}\
 0-2   {Doublette 2}\
 0-11  {Nazard 2 2/3}\
 0-12  {Tierce 1 3/5}\
 0-13  {Larigot 1 1/3}\
 0-35  {Mixture VI}\
 0-25  {Trompette 8}\
 0-78  {Clairon 4}\
 0-72  {Cromorne 8}\
]
    set shcut [list\
	1 2 3 4\
	q w e r\
	a s d f\
	z x c v\
]
    frame .kbd.go
    label .kbd.go.lab -text {Grand orgue}
    frame .kbd.go.st
    stops .kbd.go.st 0 $stolis 4 $shcut
    pack .kbd.go.lab .kbd.go.st -side top -fill x
    # recit
    set stolis [list\
 0-93  {Flute Celeste 8}\
 0-45  {Principal 8}\
 0-51  {Gedackt 8}\
 0-65  {Quintadena 8}\
 0-77  {Regal 8}\
 0-46  {Principal 4}\
 0-56  {Blockflüte 4}\
 0-103 {Gobletflte 2}\
 0-31  {Nazard III}\
 0-29  {Septade III}\
 0-57  {Sifflte 1}\
 0-33  {Fourniture III}\
 0-34  {Cymbale III}\
 1-54  {Tromp. Rom. Récit}\
 0-96  {Vox Humana 8}\
 0-71  {Krummhorn 8}\
]
    set shcut [list\
	5 6 7 8\
	t y u i\
	g h j k\
	b n m comma\
]
    frame .kbd.rec
    label .kbd.rec.lab -text {Récit}
    frame .kbd.rec.st
    stops .kbd.rec.st 1 $stolis 4 $shcut
    pack .kbd.rec.lab .kbd.rec.st -side top -fill x
    # positif
    set stolis [list\
 1-62  {Open diapason 16}\
 1-55  {Voix Celeste 8}\
 1-0   {Viola 8}\
 1-5   {Flute harmonique}\
 1-63  {Diapasons 8}\
 0-54  {Rohrflüte 4}\
 0-17  {Holzflüte 4}\
 0-66  {Viola 4}\
 1-49  {Quart de Nazard 2}\
 0-28  {Terzian II}\
 0-14  {Neuvième 8/9}\
 1-70  {Cymbale IV}\
 1-69  {Plein Jeu VI}\
 0-98  {Grand Cornet V}\
 0-74  {Cor Anglais 8}\
 1-53  {Hautbois 8}\
]
    set shcut [list\
	9 0 minus equal\
	o p bracketleft bracketright\
	l semicolon apostrophe Return\
	period slash Shift_R backslash
]
    frame .kbd.pos
    label .kbd.pos.lab -text {Positif}
    frame .kbd.pos.st
    stops .kbd.pos.st 2 $stolis 4 $shcut
    pack .kbd.pos.lab .kbd.pos.st -side top -fill x
#
    pack .kbd.pos -side bottom -fill x -padx 10 -pady 10
    pack .kbd.go .kbd.rec -side left -fill x -padx 10 -pady 10
#    pack .kbd.pos -side bottom -fill x -padx 10 -pady 10
    # accouplements
    frame .acc
    label .acc.t -text {Tirasse: }
    checkbutton .acc.pgo -text {G.O.} -variable acc(30) \
	-command [list coupl 3 0] \
	-indicatoron 0 -selectcolor LightPink1
    checkbutton .acc.prec -text {Rec.} -variable acc(31) \
	-command [list coupl 3 1] \
	-indicatoron 0 -selectcolor LightPink1
    checkbutton .acc.ppos -text {Pos.} -variable acc(32) \
	-command [list coupl 3 2] \
	-indicatoron 0 -selectcolor LightPink1
    label .acc.a -text { Accouplement: }
    checkbutton .acc.gorec -text {G.O. - Réc.} -variable acc(01) \
	-command [list coupl 0 1] \
	-indicatoron 0 -selectcolor LightPink1
    checkbutton .acc.gopos -text {G.O. - Pos} -variable acc(02) \
	-command [list coupl 0 2] \
	-indicatoron 0 -selectcolor LightPink1
    checkbutton .acc.recpos -text {Réc. - Pos.} -variable acc(12) \
	-command [list coupl 1 2] \
	-indicatoron 0 -selectcolor LightPink1
    pack .acc.t .acc.pgo .acc.prec .acc.ppos \
	.acc.a .acc.gorec .acc.gopos .acc.recpos \
	-side left -padx 10
    pack .ped .kbd .acc -side top -fill x -padx 10 -pady 10
    bind . <Shift_L> [list .acc.pgo invoke]
    bind . <Control_L> [list .acc.prec invoke]
    bind . <Alt_L> [list .acc.ppos invoke]
    bind . <space> [list .acc.gorec invoke]
    bind . <Alt_R> [list .acc.gopos invoke]
    bind . <Control_R> [list .acc.recpos invoke]
    bind . <Escape> exit
}

proc stops {top ch stolis wrap shcut} {
# define the stops of a keyboard
    set i 0
    set j 0
    set k 0
    foreach {p n} $stolis {
	checkbutton $top.s$k -text $n -variable v$ch$k \
		-command [list stop-toggle $ch $p v$ch$k] \
		-indicatoron 0 -selectcolor LightPink1
	grid $top.s$k -column $i -row $j -pady 5
	bind . <KeyPress-[lindex $shcut $k]> [list $top.s$k invoke]
	incr k
	incr i
	if {$i >= $wrap} {
	    set i 0
	    incr j
	}
    }
}

proc chupd {add ch c} {
# update the channels of a voice
    global acc
#puts "chupd add:$add ch:$ch c:$c"
    abc rewind
    abc go $ch
    set s [abc get channel]
    if {[string length $s] == 0} {
	if {!$add} return
#puts "abc insert \[list midi channel $c\]"
	abc insert [list midi channel $c]
	return
    }
    abc go $s
    set sym [abc get]
    set i [lsearch -exact $sym $c]
    if {$add} {
	if {$i >= 0} return
	lappend sym $c
    } else {
	if {$i < 0} return
	set sym [lreplace $sym $i $i]
    }
#puts "abc set $sym"
    abc set $sym
    # do the coupling job
    switch $ch {
	0 {
	    if {$acc(30)} {
		chupd $add 3 $c
	    }
	}
	1 {
	    if {$acc(01)} {
		chupd $add 0 $c
	    }
	    if {$acc(31)} {
		chupd $add 3 $c
	    }
	}
	2 {
	    if {$acc(02)} {
		chupd $add 0 $c
	    }
	    if {$acc(12)} {
		chupd $add 1 $c
	    }
	    if {$acc(32)} {
		chupd $add 3 $c
	    }
	}
	3 return
    }
}

proc stop-toggle {ch p uv} {
# toggle a stop
    global chanlis
    upvar $uv v
#puts "stop-toggle $ch $p $uv:$v"
    set c [lsearch -exact $chanlis $p]
    if {$c < 0} {
	if {!$v} return
	set c [lsearch -exact $chanlis {}]
	if {$c < 0} {
	    puts "Too many channels"
	    return
	}
	set chanlis [lreplace $chanlis $c $c $p]
	set s [lindex [abc get program] $c]
#puts "c:$c s:$s prog: [abc get program]"
	if {[string length $s] != 0} {
	    abc go $s
	    set action set
	} else {
	    abc rewind
	    abc go $ch
	    set action insert
	}
	incr c
#puts "abc $action [list midi program $c $p]"
	abc $action [list midi program $c $p]
    } else {
	if {$v} return
	set chanlis [lreplace $chanlis $c $c {}]
	incr c
    }
    #update the channels of the voice
    chupd $v $ch $c
}

proc coupl {ch1 ch2} {
# coupling
    global acc
#puts "coupl $ch1 $ch2"
    abc rewind
    abc go $ch2
    set s [abc get channel]
    abc go $s
    set sym2 [abc get]
#puts "sym2: $sym2"
    abc rewind
    abc go $ch1
    set s [abc get channel]
    abc go $s
    set sym1 [abc get]
#puts "sym1: $sym1"
    if {$acc($ch1$ch2)} {
	foreach c [lrange $sym2 2 end] {
	    set i [lsearch -exact $sym1 $c]
	    if {$i < 0} {
		lappend sym1 $c
	    }
	}
    } else {
	foreach c [lrange $sym2 2 end] {
	    set i [lsearch -exact $sym1 $c]
	    if {$i >= 0} {
		set sym1 [lreplace $sym1 $i $i]
	    }
	}
    }
#puts "abc set $sym1"
    abc set $sym1
}

proc org-init {} {
# initialize
    global chanlis tcl_platform
    set chanlis [list {} {} {} {} {} {} {} {} {} {perc} {} {} {} {} {} {}\
			{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}]
    # get the configuration parameters (from tkabc values)
    if {[string compare $tcl_platform(platform) {windows}] != 0} {
	set rcfile {.tkabcrc}
    } else {
	set rcfile {tkabc.rc}
    }
    if {![file readable $rcfile]} {
	set rcfile ~/$rcfile
    }
    if {![file readable $rcfile]} {
	bgerror "Cannot get MIDI in/out from $rcfile"
	exit
    }
    set fd [open $rcfile r]
#	    bgerror {Init file error}
    while {[gets $fd line] >= 0} {
	if {[string compare [string range $line 1 4] "midi"] == 0} {
	    foreach {o v} $line break
	    switch $o {
		midiin - midiout {
		    set $o $v
		    if {[info exists midiin] && [info exists midiout]} {
			break
		    }
		}
	    }
	}
    }
    close $fd
    if {![info exists midiin] || ![info exists midiout]} {
	bgerror "Lack of MIDI in/out"
	exit
    }
#puts "OK in:$midiin out:$midiout"
    if {[catch {abc midiout $midiout}]} {
	bgerror "Cannot open midiout ($midiout)"
	exit
    }
    if {[catch {abc midiin $midiin}]} {
	bgerror "Cannot open midiin ($midiin)"
	exit
    }
    abc play velocity 127
    abc voice new [list 2 {} {}]
    abc voice new [list 3 {} {}]
    abc voice new [list 4 {} {}]
}

# ------ main ------
# run in the tclabc source directory
set abclib .
load ./tclabc[info sharedlibextension]
set abcversion [package require tclabc]

org-init
org-gui
#fixme - for test - KO on Win32
#puts -nonewline {press return to start}
#flush stdout
#gets stdin

# set the first stop
#.ped.st.s0 invoke
.kbd.go.st.s4 invoke
.kbd.rec.st.s2 invoke
.kbd.pos.st.s4 invoke
.ped.st.s1 invoke
