#
# Preferences change for tkabc.
#
# Copyright (C) 1999-2010, Jean-François Moine.
#
# Original site: http://moinejf.free.fr/
#

proc devlist {out} {
# show a list of possible MIDI devices
    global font lang opttmp tcl_platform
    if {$out} {
	set l [abc midi devout $opttmp(midiout)]
	set w .pref.midi.o.e
    } else {
	set l [abc midi devin $opttmp(midiin)]
	set w .pref.midi.i.e
    }
    if {[string compare $tcl_platform(platform) unix] == 0} {
	foreach d [glob -nocomplain /dev/midi* /dev/snd/midi* /dev/sound/midi*] {
	    lappend l $d
	}
    }
    set ll [llength $l]
    if {$ll == 0} return
    set top .dvl
    catch {destroy $top}
    toplevel $top -bd 0
    wm transient $top $w
    for {set i 0} {$i < $ll} {incr i} {
	set v [lindex $l $i]
	button $top.l$i -relief flat -text $v -font $font(f6) \
		-command [list devsel $out $v] \
		-bd 0 -pady 0
	pack $top.l$i -side top
    }
    set wmx [expr {[winfo rootx $w]}]
    set wmy [expr {[winfo rooty $w] + 25}]
    wm geometry $top +$wmx+$wmy
    bind $top <Escape> [list destroy $top]
#    focus $top.l0
    tkwait window $top
}

proc devsel {out v} {
# select
    global opttmp
    set v [lindex $v 0]
    if {$out} {
	set opttmp(midiout) $v
    } else {
	set opttmp(midiin) $v
    }
    destroy .dvl
}

proc f-raise {p} {
# raise a preference frame
    set f .pref.$p
    set pl [pack slaves .pref]
    if {[llength $pl] > 2} {
	foreach w $pl {
	    switch $w {
		.pref.m - .pref.c - .pref.w {}
		default {
		    pack forget $w
		}
	    }
	}
    }
    pack $f -side left
    #fixme: set colors
}

proc pref-change {} {
# apply the new preferences
    global abclib lang opt opttmp tune
    set fn $opttmp(lang)
    if {![file readable $fn]} {
	set fn [file join $abclib $fn]
	if {![file readable $fn]} {
	    tk_messageBox -type ok -icon error -title $lang(bad_file) \
		-message [format $lang(lang_err) $fn]
	    set opttmp(lang) $opt(lang)
	}
    }
    array set opt [array get opttmp]
    unset opttmp
    destroy .pref
    set tune(pref) 1
}

proc pref-edit {} {
# edit the preferences
    global font lang opt opttmp pref
    set top .pref
    if {[winfo exists $top]} {
        raise $top
        focus $top.m
	#fixme: focus on 1st entry of current page
        return
    }
    array set opttmp [array get opt]
    toplevel $top
    wm title $top $lang(m_pref)
    wm geometry $top 500x200
   # warning
    set w $top.w
    label $w -text $lang(p_warn)
#	-width 50
    pack $w -side top
    # menu on the left
    set pref cmd
    set m $top.m
    frame $m -borderwidth 18
    foreach p [list cmd clr misc scr midi] {
	radiobutton $m.$p -indicatoron 0 \
		-selectcolor $opt(chg_bg) -background $opt(ent_bg) \
		-variable pref -value $p -text $lang(p_$p) \
		-command [list f-raise $p]
	pack $m.$p -side top -fill x
	frame $top.$p
	label $top.$p.l -text $lang(p_$p)
	pack $top.$p.l -side top
    }
    # commands
    set label_w 14
    set entry_w 30
    set button_w 7
    set c $top.cmd
    frame $c.ps
    label $c.ps.l -width $label_w -anchor e -text {abc(m)2ps:}
    entry $c.ps.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(abc2ps)
    button $c.ps.b -width $button_w -padx 1 -text $lang(p_browse) \
	-command [list setpath abc2ps]
    pack $c.ps.l $c.ps.e $c.ps.b -side left
    frame $c.psa
    label $c.psa.l -width $label_w -anchor e -text {abc(m)2ps args:}
    entry $c.psa.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(abc2ps-args)
    label $c.psa.b -width $button_w -text {}
    pack $c.psa.l $c.psa.e $c.psa.b -side left
    frame $c.gv
    label $c.gv.l -width $label_w -anchor e -text $lang(p_gv)
    entry $c.gv.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(ghostview)
    button $c.gv.b -width $button_w -padx 1 -text $lang(p_browse) \
	-command [list setpath ghostview]
    pack $c.gv.l $c.gv.e $c.gv.b -side left
    frame $c.d
    label $c.d.l -width $label_w -anchor e -text $lang(p_tmp)
    entry $c.d.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(tmpdir)
    button $c.d.b -width $button_w -padx 1 -text $lang(p_browse) \
	-command [list setpath tmpdir 1]
    pack $c.d.l $c.d.e $c.d.b -side left
    pack $c.ps $c.psa $c.gv $c.d -side top
    # colors
    set c $top.clr
    frame $c.bg
    label $c.bg.l -width $label_w -anchor e -text $lang(p_bg)
    entry $c.bg.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(bg)
    pack $c.bg.l $c.bg.e -side left
    frame $c.cbg
    label $c.cbg.l -width $label_w -anchor e -text $lang(p_chc)
    entry $c.cbg.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(chg_bg)
    pack $c.cbg.l $c.cbg.e -side left
    frame $c.ncbg
    label $c.ncbg.l -width $label_w -anchor e -text $lang(p_nchc)
    entry $c.ncbg.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(nochg_bg)
    pack $c.ncbg.l $c.ncbg.e -side left
    frame $c.vc
    label $c.vc.l -width $label_w -anchor e -text $lang(p_vc)
    entry $c.vc.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(v_color)
    pack $c.vc.l $c.vc.e -side left
    pack $c.bg $c.cbg $c.ncbg $c.vc -side top
    # misc
    set c $top.misc
    frame $c.lg
    label $c.lg.l -width $label_w -anchor e -text $lang(p_lgf)
    entry $c.lg.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(lang)
    pack $c.lg.l $c.lg.e -side left
    frame $c.s
    label $c.s.l -width $entry_w -anchor e -text $lang(p_twosel)
    checkbutton $c.s.e -width $label_w -anchor w \
	-variable opttmp(twosel)
    pack $c.s.l $c.s.e -side left
    frame $c.f
    label $c.f.l -width $entry_w -anchor e -text $lang(p_follow)
    checkbutton $c.f.e -width $label_w -anchor w \
	-variable opttmp(follow)
    pack $c.f.l $c.f.e -side left
    frame $c.p
    label $c.p.l -width $entry_w -anchor e -text $lang(p_playnot)
    checkbutton $c.p.e -width $label_w -anchor w \
	-variable opttmp(playnote)
    pack $c.p.l $c.p.e -side left
    pack $c.lg $c.s $c.f $c.p -side top
    # screen
    set c $top.scr
    frame $c.h
    label $c.h.l -width $label_w -anchor e -text $lang(p_hght)
    entry $c.h.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(screen-h)
    pack $c.h.l $c.h.e -side left
    frame $c.w
    label $c.w.l -width $label_w -anchor e -text $lang(p_wid)
    entry $c.w.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(screen-w)
    pack $c.w.l $c.w.e -side left
    pack $c.h $c.w -side top
    # midi
    set c $top.midi
    frame $c.i
    label $c.i.l -width $label_w -anchor e -text $lang(p_midin)
    entry $c.i.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(midiin)
    pack $c.i.l $c.i.e -side left
    bind $c.i.e <ButtonRelease-3> [list devlist 0]
    frame $c.is
    label $c.is.l -width $label_w -anchor e -text { }
    checkbutton $c.is.c \
	-text $lang(p_opst) -variable opttmp(midiin-start)
# -indicatoron 0 -relief groove
    pack $c.is.l $c.is.c -side left
    frame $c.o
    label $c.o.l -width $label_w -anchor e -text $lang(p_midout)
    entry $c.o.e -relief sunken -width $entry_w -background $opt(ent_bg) \
	-textvariable opttmp(midiout)
    pack $c.o.l $c.o.e -side left
    bind $c.o.e <ButtonRelease-3> [list devlist 1]
    frame $c.os
    label $c.os.l -width $label_w -anchor e -text { }
    checkbutton $c.os.c \
	-text $lang(p_opst) -variable opttmp(midiout-start)
#-indicatoron 0 -relief groove
    pack $c.os.l $c.os.c -side left
    pack $c.i $c.is $c.o $c.os -side top
    # window control on the bottom
    set c $top.c
    frame $c
    button $c.ok -text {OK} -width 8 \
        -command pref-change
    button $c.abort -text $lang(cancel) -width 8 \
        -command [list destroy $top]
    pack $c.ok $c.abort -side left -padx 10
# -fill x -expand 1
#
    pack $m -side left
    pack $c -side bottom
    bind $top <Escape> [list destroy $top]
    f-raise cmd
}

proc pref-save {} {
# save the preferences
    global opt tcl_platform
    set fn $::rcfile
    if {[string compare $tcl_platform(platform) {windows}] != 0} {
	set fnbak $fn~
    } else {
	set fnbak "[file rootname $fn].bak"
    }
    set old {}
    if {![catch {open $fn r} fd]} {
	set old [split [read $fd [file size $fn]] "\n"]
	close $fd
	file rename -force $fn $fnbak
	set s [lsearch $old {# USER SETTINGS}]
	set e [lsearch $old {# END OF USER SETTINGS}]
	if {$s >= 0 && $e > $s} {
	    set old [lreplace $old $s $e]
	}
    }
    if {[catch {open $fn w} fd]} {
	tk_messageBox -type ok -icon error -title $lang(bad_file) \
		-message [format $lang(wr_err) $fn]
	return
    }
    puts -nonewline $fd [join $old "\n"]
    puts $fd "# USER SETTINGS
# everything between here and the END OF USER SETTINGS comment line
# will be rewritten on a Preference Save
array set opt {"
    foreach v [lsort [array names opt]] {
	if {[string length $opt($v)] > 0 && [string first " " $opt($v)] < 0} {
	    puts $fd "\t$v $opt($v)"
	} else {
	    puts $fd "\t$v {$opt($v)}"
	}
    }
    puts $fd "}
# END OF USER SETTINGS"
    close $fd
}

proc setpath {name {isdir 0}} {
# browse a file name
    global opttmp
    set filedir [file dirname $opttmp($name)]
    if {[string length $filedir] == 0} {
	set filedir [pwd]
    }
    if {!$isdir} {
	set filename [tk_getOpenFile -initialdir $filedir]
    } else {
	set filename [tk_chooseDirectory -initialdir $filedir]
    }
    if {[string length $filename] != 0} {
	set opttmp($name) $filename
    }
}
