#
# mini web-browser adapted from 'sdr'
# - some bug fixes & form removed
# - use the http package
#

global urilist uriix cacheduris cachedata statctr
set urilist {}
set uriix -1
set cacheduris {}
set cachedata {}
set statctr 0

option add *web.h1font {-*-times-bold-r-normal--*-240-*} widgetDefault
option add *web.h2font {-*-times-bold-r-normal--*-180-*} widgetDefault
option add *web.h3font {-*-times-bold-r-normal--*-140-*} widgetDefault
option add *web.h4font {-*-times-bold-r-normal--*-140-*} widgetDefault
option add *web.bfont {-*-times-bold-r-normal--*-140-*} widgetDefault
option add *web.ifont {-*-times-medium-i-normal--*-140-*} widgetDefault
option add *web.addressfont {-*-times-medium-i-normal--*-140-*} widgetDefault
option add *web.normalfont {-*-times-medium-r-normal--*-140-*} widgetDefault
option add *web.prefont {-*-courier-medium-r-normal--*-120-*} widgetDefault
option add *web.dtfont {-*-times-bold-r-normal--*-140-*} widgetDefault

#fixme: may corrupt other .tcl...
option add *foreground black
option add *activeBackground gray95
option add *activeForeground black
option add *hotForeground blue
option add *activehotForeground red
option add *selectBackground gray95

image create bitmap bullet0 -foreground black -background white -data {
#define bullet0_width 11
#define bullet0_height 11
static char bullet0_bits[] = {
   0x00, 0x00, 0x70, 0x00, 0xf8, 0x00, 0xfc, 0x01, 0xfe, 0x03, 0xfe, 0x03,
   0xfe, 0x03, 0xfc, 0x01, 0xf8, 0x00, 0x70, 0x00, 0x00, 0x00};
}
image create bitmap bullet1 -foreground black -background white -data {
#define bullet1_width 11
#define bullet1_height 11
static char bullet1_bits[] = {
   0x00, 0x00, 0x70, 0x00, 0x88, 0x00, 0x04, 0x01, 0x02, 0x02, 0x02, 0x02,
   0x02, 0x02, 0x04, 0x01, 0x88, 0x00, 0x70, 0x00, 0x00, 0x00};
}

proc webdisp {uri} {
    global urilist uriix

#    catch
    if ![winfo exists .web] {
	toplevel .web
	frame .web.f -borderwidth 2 -relief groove
	pack .web.f -side top -fill both -expand true
	text .web.f.c -width 80 -height 30 -wrap word \
	   -font [option get .web prefont Sdr] \
	   -yscroll [list .web.f.sb set]
	pack .web.f.c -side left -fill both -expand true
	scrollbar .web.f.sb  -command [list .web.f.c yview]
#		-background [option get . scrollbarBackground Sdr]
#TBD
#  -foreground [option get . scrollbarForeground Sdr] \
#  -activeforeground [option get . scrollbarActiveForeground Sdr]
	pack .web.f.sb -side right -fill y

	frame .web.f0 -borderwidth 2 -relief groove
	pack .web.f0 -side top -fill x
	
	for {set i 0} {$i<5} {incr i} {
	    frame .web.f0.active$i -width 5 -height 10 -bg white
	    pack .web.f0.active$i -side left
	}
	label .web.f0.l -text {Status:}
	pack .web.f0.l -side left
	set webstatus {Looking up address}
	label .web.f0.s -text $webstatus -anchor w
	pack .web.f0.s -side left -anchor w -fill x -expand true
	frame .web.f1 -borderwidth 2 -relief groove
	pack .web.f1 -side top -fill x
	label .web.f1.l -text {Location:}
	pack .web.f1.l -side left
	entry .web.f1.e -width 40 -relief sunken -borderwidth 2
	bind .web.f1.e <Return> {
	    webdisp [.web.f1.e get]
	}
	pack .web.f1.e -side left -fill x -expand true
	frame .web.f2 -borderwidth 2 -relief groove
	pack .web.f2 -side top -fill x
	button .web.f2.back -text Back \
	    -command {incr uriix -1; webdisp2}
	pack .web.f2.back -side left -fill x -expand true
	button .web.f2.forw -text Forward \
	    -command {incr uriix; webdisp2}
	pack .web.f2.forw -side left -fill x -expand true
	button .web.f2.reload -text Reload \
	    -command {webdisp2 r}
	pack .web.f2.reload -side left -fill x -expand true
	button .web.f2.stop -text Stop -command {stop_www_loading}
	pack .web.f2.stop -side left -fill x -expand true
	button .web.f2.dismiss -text Dismiss \
	    -command {stop_www_loading; destroy .web}
	pack .web.f2.dismiss -side left -fill x -expand true
	set uriix -1
	set urilist {}
    }
    # If there's no colon, or if the colon is followed by a port number and
    # potentially a pathname, then add the missing "http://" .
    if {[string first {:} $uri] == -1 || [regexp {^[^:]+:[0-9]+(|/.*)$} $uri]} {
	set uri "http://$uri"
    }
    set urilist [lrange $urilist 0 $uriix]
    lappend urilist $uri
    incr uriix
#    catch {webdisp2}
	webdisp2
}

# webdisp2 only ever displays $urilist[$uriix]
proc webdisp2 {{reload {}}} {
    global urilist uriix webproxy webstatus cacheduris cachedata

    if {$uriix==-1} {
	puts {webdisp2 called with no url}
	return
    } 
    set webstatus {Looking up address}
    .web.f0.s configure -text $webstatus
    update
    .web.f2.stop configure -state normal
    if {$uriix==0} {
	.web.f2.back configure -state disabled
    } else {
	.web.f2.back configure -state normal
    }
    if {$uriix<[expr {[llength $urilist]-1}]} {
	.web.f2.forw configure -state normal
    } else {
	# Range checking
	if {$uriix>[expr {[llength $urilist] -1}]} {
		puts {webdisp2 had to reset uriix}
	    set uriix [expr {[llength $urilist] -1}]
	}
	.web.f2.forw configure -state disabled
    }
    set uri [lindex $urilist $uriix]
    set cacheix [lsearch $cacheduris $uri]
    if {$cacheix == -1 || $reload != {}} {
#	set str [webto $uri $webproxy $reload]
	set str [webto $uri]
	if {[string length $str] < 10000} {
	    if {$cacheix == -1} {
		if {[llength $cacheduris] > 20} {
		    set cacheduris [lrange $cacheduris 1 end]
		    set cachedata [lrange $cachedata 1 end]
		}
		lappend cacheduris $uri
		lappend cachedata $str
	    } else {
		set cachedata [lreplace $cachedata $cacheix $cacheix $str]
	    }
	}
    } else {
	set str [lindex $cachedata $cacheix]
    }
    .web.f2.stop configure -state disabled
    set webstatus {Parsing}
    .web.f0.s configure -text $webstatus
    show_inactive
    update
#    puts "uriix=$uriix"
#    puts "urilist=$urilist"
    .web.f1.e delete 0 end
    .web.f1.e insert 0 $uri
    .web.f.c configure -state normal
    .web.f.c delete 1.0 end
    wm title .web {Help browser}
    #parse the header
    #
    # Some bonehead web servers forget to reply with HTTP 1.0 headers
    # in certain situations.  Guess using bogus heuristic:
    if {[string index $str 0] == {<}} {
	set header "Content-type: text/html\n"
	set data $str
    } else {
	set hend [string first "\r\n\r\n" $str]
#	puts $hend
	if {$hend == -1} {
	    set hend [string first "\n\n" $str]
	    if {$hend == -1} {
		puts {Failed to find CRLFCRLF or LFLF in header!}
		puts [split [string range $str 0 400] "\n"]
		set data {Failed to find CRLFCRLF or LFLF in header!}
	    } else {
		set data [string range $str [expr {$hend + 2}] end]
	    }
	} else {
	    set data [string range $str [expr {$hend + 4}] end]
	}
	set header [string range $str 0 $hend]
    }

    set ctype text
    set cstype html
    set hlist [split $header "\n"]
    set redirect 0
    foreach h $hlist {
	set h [string trim $h "\r"]
	set field [string tolower [lindex [split $h ":"] 0]]
	set value [string trim [lindex [split $h ":"] 1] " "]
#	puts "->$field<->$value<-"
	if {$field == {content-type}} {
	    set ctype [lindex [split $value {/}] 0]
	    set cstype [lindex [split $value {/}] 1]
	} elseif {$field == {location}} {
	    set redirect \
		[string trim [join [lrange [split $h {:}] 1 end] {:}] { }]
	}
    }
    if {$redirect != 0} {
	set urilist [lreplace $urilist $uriix $uriix $redirect]
	webdisp2
	return
    }
    if {$ctype != {text}} {
	if {$ctype == {image}} {
	    msgpopup {Starting external viewer} {Data is an image}
	    save_www_data_to_file [expr {$hend + 4}] /tmp/sdrimage
	    exec xv /tmp/sdrimage &
	} else {
	    # Server CERN/3.0pre6 uses content-type "www/unknown"
	    # for error messages!  How to handle this??
	    msgpopup {Invalid Content Type} "$ctype/$cstype is not a text type"
	    set webstatus {Finished}
	    .web.f0.s configure -text $webstatus
	    return
	}
    }
    if {$cstype == {plain}} {
# We need to use prefont to set the width properly, but we want to default
# to normalfont.  Is there a better way to do this?
	.web.f.c tag configure hack -font [option get .web normalfont Sdr]
	.web.f.c tag add hack 1.0 end
	.web.f.c insert 1.0 $data
	highlight_url .web.f.c 1
	.web.f.c configure -state disabled
    } elseif {$cstype == {html}} {
	parse_html .web.f.c $data 
	if {[regexp {#(.*)$} $uri junk anchor]} {
	    set anchor "M$anchor"
	    catch {.web.f.c yview $anchor}
	}
    }
    set webstatus {Finished}
#    .web.f0.s configure -text $webstatus
    webstatus
    wm minsize .web 1 1
    focus .web.f.c
}

proc webstatus {} {
    global webstatus
    #the things we do to get around broken Tcl_Eval calls!
    .web.f0.s configure -text $webstatus
}

proc show_active {} {
    global statctr
    .web.f0.active$statctr configure -background white
    incr statctr
    if {$statctr == 5} {set statctr 0}    
    .web.f0.active$statctr configure -background blue
}
proc show_inactive {} {
    global statctr
    .web.f0.active$statctr configure -background white
}

proc overhref {{href {}}} {
    global webstatus
    if {$href == {}} {
	.web.f0.s configure -text $webstatus
    } else {
	set width [winfo width .web.f0.s]
	.web.f0.s configure -text $href

	#trim the string until it fits...
	set reqwidth [winfo reqwidth .web.f0.s]
	while {$width<$reqwidth} {
	    set href [string range $href 0 [expr {[string length $href]-2}]]
	    .web.f0.s configure -text $href
	    set reqwidth [winfo reqwidth .web.f0.s]
	}
    }
}

proc parse_html {win data} {
    global uriix urilist href
    set normal normal
    set type $normal
    set ptype $normal
    set fontstack $normal
    set ltype ul
    set href 0
    set refnum 0
    set break 1
    set space 2
    set listindent 0
    set bulletnum 0
    foreach tag {h1 h2 h3 h4 b i address normal pre dt} {
	$win tag configure $tag -font [option get .web ${tag}font Sdr]
    }

    $win tag configure ref -foreground blue
    $win tag configure dd -lmargin1 40 -lmargin2 40 \
	-font [option get .web normalfont Sdr]
    $win tag configure center -justify center

    $win insert end { }
    $win tag add normal 1.0 end

    regsub -all "\r\n" $data "\n" data
    # Some web servers violate HTTP and send \r-seperated lines
    regsub -all "\r" $data "\n" data
    while {$data != {}} {
#	puts [string length $data]
	set ix1 [string first {<} $data]
	set cur [$win index {end - 1 chars}]
	if {$type == {title}} {
	    wm title .web [string range $data 0 [expr {$ix1 - 1}]]
	} else {
	    set idata [string range $data 0 [expr {$ix1 - 1}]]
	    if {$break == 1} {
                set llist [split $idata "\n\r"]
                set idata {}
                foreach line $llist {
		    if {$line != {}} {
			if {$idata == {}} {
			    # Note that source HTML like
			    #
			    # 		<A HREF="foo">foo</A>
			    #		blah
			    #
			    # will get rendered like
			    #
			    # foo		blah
			    #
			    # because of the following if.
			    #
			    if {$space > 0} {
				set line [string trimleft $line " \t"]
			    }
			    set idata $line
			} else {
			    set line [string trimleft $line " \t"]
			    append idata { }
			    append idata $line
			}
		    }
		}
		if {[lindex $llist end] == {}} {
		    append idata { }
		}
	    }
	    if {[string trim $idata " \t"] != {} || $break == 0} {
		$win insert end [parse_for_escaped_chars $idata]
		set space 0
	    }
	    $win tag remove $ptype $cur end
	    $win tag add $type $cur end
	    update
	    set ptype $type
	    if {$href != 0} {
		$win tag configure ref$refnum -foreground blue
		$win tag bind ref$refnum <1> \
		    "webdisp {$href}"
		$win tag bind ref$refnum <Enter> \
		    "$win tag configure ref$refnum -foreground \
                     [option get . activehotForeground Sdr];\
		     overhref {$href}"
		$win tag bind ref$refnum <Leave> \
		    "$win tag configure ref$refnum -foreground \
                     [option get . hotForeground Sdr];\
		     overhref"
		$win tag add ref$refnum $cur end
	    } else {
		$win tag remove ref$refnum $cur end
	    }
	}
	set ix2 [string first {>} $data]
#	set tag [string range $data [expr {$ix1 + 1}] [expr {$ix2 - 1}]]
#	if {$ix2 != -1} {
#	    set data [string range $data [expr {$ix2 + 1}] end]
#	} else {
#	    $win insert end [parse_for_escaped_chars $data]
#	    $win configure -state disabled
#	    return
#	}
	if {$ix2 == -1} {
	    $win insert end [parse_for_escaped_chars $data]
	    $win configure -state disabled
	    return
	}
	set tag [string range $data [expr {$ix1 + 1}] [expr {$ix2 - 1}]]
	set data [string range $data [expr {$ix2 + 1}] end]
#
	set tname [lindex [split $tag " \n\r\t"] 0]
	set tname [string tolower $tname]
	set cur [$win index {end - 1 chars}]
	switch -- $tname {
	    base {
		set base [parseref $tag]

		if {$base != 0} {
		    .web.f1.e delete 0 end
		    .web.f1.e insert 0 $base
		    set urilist [lreplace $urilist $uriix $uriix $base]
		}
	    }
	    p {
		set space [addspace $win 2 $space]
		rmtags $win $cur end
		set data [string trimleft $data { }]
	    }
	    hr {
		addspace $win 2 $space
		$win insert end "--------\n"
		set space 1
		set data [string trimleft $data { }]
	    }
	    a {
		set href [fixuri [parseref $tag]]
		if {[parsestr $tag {name} mark]} {
		    set mark "M$mark"
		    $win mark set $mark $cur
		    $win mark gravity $mark left
		}
#		set space 0
		incr refnum
	    }
	    /a {
		set href 0
	    }
	    address {
		set space [addspace $win 1 $space]
		set type $tname
	    }
	    /address {
		set type $normal
	    }
	    h1 - h2 - h3 - h4 {
		set space [addspace $win 2 $space]
		set type $tname
		set data [string trimleft $data { }]
		set fontstack $tname
	    }
	    /h1 - /h2 - /h3 - /h4 {
		set space [addspace $win 2 $space]
		set type $normal
		set data [string trimleft $data { }]
		set fontstack $normal
	    }
	    br - /br {
		# Sigh, netscape treats </br> as <br> so people will use it
		set space [addspace $win 1 $space]
		set data [string trimleft $data { }]
	    }
	    img {
		#is there an alternate text string
		if {[parsestr $tag {alt} alt]} {
		    $win insert end $alt
		} else {
		    #is it an active map (can't do them!)
		    set ix [string first {ismap} [string tolower $tag]]
		    if {$ix!=-1} {
			set href 0
			$win insert end "\[ACTIVE MAP\]"
		    } else {
			$win insert end "\[IMAGE\]"
		    }
		}
		rmtags $win $cur end

		if {$href!=0} {
		    $win tag add ref$refnum $cur end
		    $win tag configure ref$refnum -foreground blue
		    $win tag bind ref$refnum <1> \
			    "webdisp $href"
		}
		set space 0
	    }
	    dl {
		set space [addspace $win 2 $space]
		set type $normal
		set ltype dl
		incr listindent
	    }
	    /dl {
		set space [addspace $win 2 $space]
		set type $normal
		set fontstack $normal
		set ltype ul
		incr listindent -1
	    }
	    dt {
		set space [addspace $win 2 $space]
#		set type $normal
		set type $tname
#		set fontstack $normal
		set fontstack $tname
	    }
	    dd {
		set space [addspace $win 1 $space]
		set type $tname
		set fontstack $tname
	    }
	    ul {
		set space [addspace $win 2 $space]
		set type $normal
		set ltype ul
		incr listindent
	    }
	    /ul {
		set space [addspace $win 2 $space]
		set type $normal
		set ltype ul
		incr listindent -1
	    }
	    menu {
		set space [addspace $win 2 $space]
		set type $normal
		set ltype ul
		incr listindent
	    }
	    /menu {
		set space [addspace $win 2 $space]
		set type $normal
		set ltype ul
		incr listindent -1
	    }
	    ol {
		set space [addspace $win 2 $space]
		set type $normal
		set ltype ol
		set lnum 1
		incr listindent
	    }
	    /ol {
		set space [addspace $win 2 $space]
		set type $normal
		set ltype ul
		incr listindent -1
	    }
	    li {
		set space [addspace $win 1 $space]
		for {set ind 1} {$ind < $listindent} {incr ind} {
		    $win insert end {  }
		}
		if {$ltype == {ul}} { 
		    if {$listindent == 1} {
#                    $win window create end -create "label $win.bullet$bulletnum -bitmap bullet0"
			$win window create end -create "label $win.bullet$bulletnum -image bullet0"
			incr bulletnum
		    } else {
#                    $win window create end -create "label $win.bullet$bulletnum -bitmap bullet1"
			$win window create end -create "label $win.bullet$bulletnum -image bullet1"
			incr bulletnum
		    }
		    rmtags $win $cur end
		    set data [string trimleft $data " \n\t\r"]
		} elseif {$ltype == {ol}} {
		    $win insert end " $lnum. "
		    rmtags $win $cur end
		    set data [string trimleft $data " \n\t\r"]
		    incr lnum
		}
	    }
	    /b {
		if {[lindex $fontstack 0] == {b}} {
		    set type $normal
		    catch {
			set type [lindex $fontstack 1]
			set fontstack [lrange $fontstack 1 end]
		    }
		} else {
		    set type $normal
		}
#		set space 0
	    }
	    /i {
		if {[lindex $fontstack 0] == {i}} {
		    set type $normal
		    catch {
			set type [lindex $fontstack 1]
			set fontstack [lrange $fontstack 1 end]
		    }
		} else {
		    set type $normal
		}
#		set space 0
	    }
	    pre {
		set type $tname
		if {[lindex $fontstack 0] != $tname} {
		    set fontstack [list $tname $fontstack]
		}
		set break 0
		set space [addspace $win 1 0]
#		set fontstack pre
	    }
	    /pre {
		if {[lindex $fontstack 0] == {pre}} {
		    set type $normal
		    catch {
			set type [lindex $fontstack 1]
			set fontstack [lrange $fontstack 1 end]
		    }
		} else {
		    set type $normal
		}
		set break 1
		set space [addspace $win 1 0]
#		set fontstack $normal
	    }
	    ! {
		# do anything here, or just completely ignore?
	    }
	    !doctype {
		# do nothing.
	    }
	    b - i {
		set type $tname
		if {[lindex $fontstack 0] != $tname} {
		    set fontstack [list $tname $fontstack]
		}
	    }
	    title {
		set type $tname
	    }
	    /title {
		set type $normal
	    }
	    default {
#fixme
		if {[string match {!--*} $tname]} {
		    # A comment may have embedded HTML; if the end of the tag
		    # is not "--" then find the real end tag.
		    # Note that this doesn't parse "<!-- foo -- >" properly
		    if {[string range $tag [expr {[string length $tag]-2}] end] != "--"} {
			set cix [string first {-->} $data]
			if {$cix != -1} {
			    set data [string range $data [expr {$cix+3}] end]
			}
		    }
		}
#		set space 0
	    }
	}
    }
    $win configure -state disabled
}

proc addspace {win num space} {
    set cur [$win index {end - 1 chars}]
    while {$num > $space} {
	$win insert end "\n"
	incr space
    }
#    rmtags $win $cur end
    rmtags $win $cur end-1c
    return $space
}
proc gohome {} {
    global home uriix urilist

    webdisp $home
}
proc rmtags {win p1 p2} {
    set tlist [$win tag names $p1]
    foreach t $tlist {
	$win tag remove $t $p1 $p2
    }
    $win tag add normal $p1 $p2
}

proc parseref {ref} {
    if {[parsestr $ref {href} tmp]==0} {
	return 0
    }
    return $tmp
}

# parsestr returns the value for a particular key/value pair
# attempting to allow optional quoting and the always-forgotten
# trailing quote.
#
# parsestr returns the value in the variable $ret, and returns
# 1 if it found a match or 0 if it failed.
#
# XXX todo: remove regexp special characters from $key
proc parsestr {str key ret} {
    if {[regexp -nocase "^(.*\[ \t\r\n]+)?${key}\[ \t\r\n=]+(.*)$" $str \
		junk junk2 str]==0} {uplevel "set $ret {}"; return 0}

    if {[regexp {^"([^"]*)"?} $str junk tmp]} {
	uplevel "set $ret {$tmp}"
    } else {
	uplevel "set $ret {[lindex [split $str] 0]}"
    }
    return 1
}

proc webrefresh {newurl cururl} {
    global uriix urilist

    if {$cururl != [lindex $urilist $uriix]} {
#	puts "Refreshing from $cururl to $newurl failed because we're on [lindex $urilist $uriix] now"
	# We're not currently on the page that the refresh belongs to
	return
    } else {
#	puts "Refreshing from $cururl to $newurl"
	# If it's a different URL, then add it to the history
	# If it's the same URL, just re-display it
	if {[string compare $cururl $newurl]} {
	    webdisp $newurl
	} else {
	    webdisp2
	}
    }
}

proc getprevuri {} {
    global uriix urilist
#    put $urilist
    return [lindex $urilist [expr {$uriix-1}]]
}

proc getnexturi {} {
    global uriix urilist
#    puts $urilist
    if {[llength $urilist] > $uriix} {
	return [lindex $urilist [expr {$uriix+1}]]
    } else {
	return [lindex $urilist $uriix]
    }
}

proc fixuri {uri} {
    global uriix urilist
    if {$uri==0} {return 0}

    set ix [string first {://} $uri]
    if {$ix > -1} {
	if {[string index $uri 0]!="/"} {
	    return $uri
	}
    }
    if {[string range $uri 0 4] == {help:}} {return $uri}
    if {[string range $uri 0 6] == {mailto:}} {return $uri}
    #break up the current url
    set cururi [lindex [split [lindex $urilist $uriix] {#}] 0]
    set ix [string first {://} $cururi]
    if {$ix != -1} {
	set proto [string range $cururi 0 [expr {$ix - 1}]]
	set host [string range $cururi [expr {$ix + 3}] end]
	set ix [string first {/} $host]
	if {$ix!=-1} {
	    set path [string range $host [expr {$ix+1}] end]
	    set host [string range $host 0 [expr {$ix-1}]]
	} else {
	    set path ""
	}
    } else {
	# when 'help:xxx'
	set proto {help}
	set host {}
	set path [string range $cururi 5 end]
    }

    # Have to parse host-less URL's like http:/foo/bar/baz

    #there's no protocol field present
    if {[lindex [split $uri {#}] 0]==""} {
	#it's in the same file
	return "$cururi$uri"
    } elseif {[string index $uri 0]!="/"} {
	#it's a relative filename
	set dropctr 1
	while {[string range $uri 0 2]=="../"} {
	    incr dropctr
	    set uri [string range $uri 3 end]
	}
	set plist [split $path {/}]
	set plist [lrange $plist 0 [expr {[llength $plist]-($dropctr+1)}]]
	lappend plist $uri
	set path [join $plist {/}]
	if {$proto == {help}} {
	    set ix [string length $path]
	    incr ix -1
	    if {[string index $path $ix] == "/"} {
		append path {index.html}
	    }
	    return "help:$path"
	}
	return "$proto://$host/$path"
    } elseif {[string index $uri 1] != {/}} {
	#it's an absolute filename
	if {$proto == {help}} {
	    set ix [string length $uri]
	    incr ix -1
	    if {[string index $uri $ix] == {/}} {
		append uri {index.html}
	    }
	    return "help:$uri"
	}
	return "$proto://$host$uri"
    } else {
	#only the proto is missing
	return "$proto:$uri"
    }
}

proc parse_for_escaped_chars {data} {
    set result {}
    while {[string first {&} $data] != -1} {
	set ix [string first {&} $data]
	set result $result[string range $data 0 [expr {$ix - 1}]]
	set data [string range $data $ix end]
	set ix2  [string first {;} $data]
	if {$ix2 == -1} {
	    break;
	}
	set esc [string range $data 0 $ix2]
	set data [string range $data [expr {$ix2+1}] end]
	switch -regexp $esc {
	    {^&quot;$} {set result "$result''"}
            {^&AElig;$} {set result [format {%s%c} $result 198]}
            {^&Aacute;$} {set result [format {%s%c} $result 193]}
            {^&Acirc;$} {set result [format {%s%c} $result 194]}
            {^&Agrave;$} {set result [format {%s%c} $result 192]}
            {^&Aring;$} {set result [format {%s%c} $result 197]}
            {^&Atilde;$} {set result [format {%s%c} $result 195]}
            {^&Auml;$} {set result [format {%s%c} $result 196]}
            {^&Ccedil;$} {set result [format {%s%c} $result 199]}
            {^&ETH;$} {set result [format {%s%c} $result 208]}
            {^&Eacute;$} {set result [format {%s%c} $result 201]}
            {^&Ecirc;$} {set result [format {%s%c} $result 202]}
            {^&Egrave;$} {set result [format {%s%c} $result 200]}
            {^&Euml;$} {set result [format {%s%c} $result 203]}
            {^&Iacute;$} {set result [format {%s%c} $result 205]}
            {^&Icirc;$} {set result [format {%s%c} $result 206]}
            {^&Igrave;$} {set result [format {%s%c} $result 204]}
            {^&Iuml;$} {set result [format {%s%c} $result 207]}
            {^&Ntilde;$} {set result [format {%s%c} $result 209]}
            {^&Oacute;$} {set result [format {%s%c} $result 211]}
            {^&Ocirc;$} {set result [format {%s%c} $result 212]}
            {^&Ograve;$} {set result [format {%s%c} $result 210]}
            {^&Oslash;$} {set result [format {%s%c} $result 216]}
            {^&Otilde;$} {set result [format {%s%c} $result 213]}
            {^&Ouml;$} {set result [format {%s%c} $result 214]}
            {^&THORN;$} {set result [format {%s%c} $result 222]}
            {^&Uacute;$} {set result [format {%s%c} $result 218]}
            {^&Ucirc;$} {set result [format {%s%c} $result 219]}
            {^&Ugrave;$} {set result [format {%s%c} $result 217]}
            {^&Uuml;$} {set result [format {%s%c} $result 220]}
            {^&Yacute;$} {set result [format {%s%c} $result 221]}
            {^&aacute;$} {set result [format {%s%c} $result 225]}
            {^&acirc;$} {set result [format {%s%c} $result 226]}
            {^&acute;$} {set result [format {%s%c} $result 180]}
            {^&aelig;$} {set result [format {%s%c} $result 230]}
            {^&agrave;$} {set result [format {%s%c} $result 224]}
            {^&amp;$} {append result {&}}
            {^&aring;$} {set result [format {%s%c} $result 229]}
            {^&atilde;$} {set result [format {%s%c} $result 227]}
            {^&auml;$} {set result [format {%s%c} $result 228]}
            {^&brvbar;$} {set result [format {%s%c} $result 166]}
            {^&ccedil;$} {set result [format {%s%c} $result 231]}
            {^&cedil;$} {set result [format {%s%c} $result 184]}
            {^&cent;$} {set result [format {%s%c} $result 162]}
            {^&copy;$} {set result [format {%s%c} $result 169]}
            {^&curren;$} {set result [format {%s%c} $result 164]}
            {^&deg;$} {set result [format {%s%c} $result 176]}
            {^&divide;$} {set result [format {%s%c} $result 247]}
            {^&eacute;$} {set result [format {%s%c} $result 233]}
            {^&ecirc;$} {set result [format {%s%c} $result 234]}
            {^&egrave;$} {set result [format {%s%c} $result 232]}
            {^&eth;$} {set result [format {%s%c} $result 240]}
            {^&euml;$} {set result [format {%s%c} $result 235]}
            {^&frac12;$} {set result [format {%s%c} $result 189]}
            {^&frac14;$} {set result [format {%s%c} $result 188]}
            {^&frac34;$} {set result [format {%s%c} $result 190]}
            {^&gt;$} {set result [format {%s%c} $result 62]}
            {^&iacute;$} {set result [format {%s%c} $result 237]}
            {^&icirc;$} {set result [format {%s%c} $result 238]}
            {^&iexcl;$} {set result [format {%s%c} $result 161]}
            {^&igrave;$} {set result [format {%s%c} $result 236]}
            {^&iquest;$} {set result [format {%s%c} $result 191]}
            {^&iuml;$} {set result [format {%s%c} $result 239]}
            {^&laquo;$} {set result [format {%s%c} $result 171]}
            {^&lt;$} {set result [format {%s%c} $result 60]}
            {^&macr;$} {set result [format {%s%c} $result 175]}
            {^&micro;$} {set result [format {%s%c} $result 181]}
            {^&middot;$} {set result [format {%s%c} $result 183]}
            {^&nbsp;$} {set result [format {%s%c} $result 32]}
            {^&not;$} {set result [format {%s%c} $result 172]}
            {^&ntilde;$} {set result [format {%s%c} $result 241]}
            {^&oacute;$} {set result [format {%s%c} $result 243]}
            {^&ocirc;$} {set result [format {%s%c} $result 244]}
            {^&ograve;$} {set result [format {%s%c} $result 242]}
            {^&ordf;$} {set result [format {%s%c} $result 170]}
            {^&ordm;$} {set result [format {%s%c} $result 186]}
            {^&oslash;$} {set result [format {%s%c} $result 248]}
            {^&otilde;$} {set result [format {%s%c} $result 245]}
            {^&ouml;$} {set result [format {%s%c} $result 246]}
            {^&para;$} {set result [format {%s%c} $result 182]}
            {^&plusmn;$} {set result [format {%s%c} $result 177]}
            {^&pound;$} {set result [format {%s%c} $result 163]}
            {^&raquo;$} {set result [format {%s%c} $result 187]}
            {^&reg;$} {set result [format {%s%c} $result 174]}
            {^&sect;$} {set result [format {%s%c} $result 167]}
            {^&shy;$} {set result [format {%s%c} $result 173]}
            {^&sup1;$} {set result [format {%s%c} $result 185]}
            {^&sup2;$} {set result [format {%s%c} $result 178]}
            {^&sup3;$} {set result [format {%s%c} $result 179]}
            {^&szlig;$} {set result [format {%s%c} $result 223]}
            {^&thorn;$} {set result [format {%s%c} $result 254]}
            {^&times;$} {set result [format {%s%c} $result 215]}
            {^&uacute;$} {set result [format {%s%c} $result 250]}
            {^&ucirc;$} {set result [format {%s%c} $result 251]}
            {^&ugrave;$} {set result [format {%s%c} $result 249]}
            {^&uml;$} {set result [format {%s%c} $result 168]}
            {^&uuml;$} {set result [format {%s%c} $result 252]}
            {^&yacute;$} {set result [format {%s%c} $result 253]}
            {^&yen;$} {set result [format {%s%c} $result 165]}
            {^&yuml;$} {set result [format {%s%c} $result 255]}
            {^&#[0-9]+;$} {regexp {0*([0-9]+)} $esc junk tmp
                         set result [format {%s%c} $result $tmp]}
	    default {set result [format {%s%c} $result 38]
		     set data [string range $esc 1 end]$data}
	}
    }
    append result $data
    return $result
}

proc stop_www_loading {} {
    destroy .web
#fixme: for test
#    exit 0
}

proc webto {uri {webproxy {}} {reload {}}} {
# get the content of an URI
    # treat "http:"
    if {[string range $uri 0 4] == {http:}} {
	package require http
	set token [::http::geturl $uri]
	# -timeout ms
	set body [::http::data $token]
	::http::cleanup $token
	return $body
    }
    # treat "help:xxx" and "file://localhost/xxx"
    if {[string range $uri 0 4] == {help:}} {
	set uri [string range $uri 5 end]
    } elseif {[string range $uri 0 4] == {file:}} {
	set ix [string first {/} $uri]
	incr ix 2
	set uri [string range $uri $ix end]
	set ix [string first {/} $uri]
	set uri [string range $uri $ix end]
    } else {
	return "<b>scheme non implemented</b>:<p>'$uri'"
    }
    set fd [open $uri {r}]
    set str [read $fd]
    close $fd
    return $str
}

#for test
#wm withdraw .
#webdisp "help:index.html"
