
#
#  Nupshot - a new Upshot (pre-release); faster, less features
#
#  No, this is not a final product.  Nupshot is the current state of
#  development in a new Upshot.  It is at a useable point, and is much
#  faster, so we thought some people might want to use it.  All the
#  features of the all-Tcl Upshot and more will be added.
#
#  Ed Karrels
#  Argonne National Laboratory
#


set tcl_precision 17

proc ZoomIn {logfile timeline} {
   ZoomCenter $logfile $timeline 2
}
proc ZoomOut {logfile timeline} {
   ZoomCenter $logfile $timeline .5
}

proc ZoomCenter {logfile timeline factor} {
   # puts "Canvas is [$timeline canvas]"
   global width

   set width [expr $width * $factor]
   set left_coord [$timeline canvasx 0]
   set right_coord [$timeline canvasx [winfo width $timeline]]

   set center_coord [expr ($right_coord + $left_coord)/2.0]
   set center_time [$timeline pix2time $center_coord]

   $timeline zoom_time $center_time $factor
   # puts "zooming in on $center_time"
}


proc SetScrolls {scrollbars a b c d} {
   foreach scrollbar $scrollbars {
      $scrollbar set $a $b $c $d
   }
   # puts "scroll $a $b $c $d"
}



set bw [expr [winfo depth .] == 1]
if !$bw {
   . config -bg SteelBlue
   option add *foreground Snow
   option add *background SteelBlue
   option add *activeForeground snow
   option add *activeBackground steelblue2
}

proc PrintUsage {} {
   puts "\n   nupshot <filename>\n"
}


if {$argv==""} {
   puts "Please specify logfile filename:"
   PrintUsage
   exit
}

if ![file readable $argv] {
   puts "Could not open \"$argv\"."
   PrintUsage
   exit
}


proc SetTimeMark {tl mark} {
   global time_mark

   # puts "mark: $mark, canvasx: [$tl canvasx $mark], time:\
	 [$tl pix2time [$tl canvasx $mark]]"

   set time_mark "time: [format %.6f [$tl pix2time [$tl canvasx $mark]]] sec."
}

proc CheckStateInfoPost {tl log x y screen_x screen_y} {
   global tl_post
   # puts "Clicked at $x $y"
   set tl_post($tl) "0"
   set item [$tl currentitem]
   if {$item == ""} return

   # puts $item
   if {[lindex $item 0] == "state" } {
      # puts [logfile $log state [lindex $item 1]]
      # puts "item = $item, index = [lindex $item 1]"
      set stateInfo [logfile $log get_state [lindex $item 1]]
         # info: type, proc, startTime, endTime, parent, firstChild,
         # overlapLevel
      set stateName [lindex [logfile $log get_statedef \
	    [lindex $stateInfo 0]] 0]
         # def: name, color, bitmap
      set startTime [format "%.6f" [lindex $stateInfo 2]]
      set endTime [format "%.6f" [lindex $stateInfo 3]]
      set len [format "%.6f" [expr $endTime - $startTime]]
      PostStateInfo $tl $x $y $stateName $startTime $endTime $len \
	    $screen_x $screen_y
   } else {
      puts "Wierd. $tag is not a state."
   }
}
 
proc PostStateInfo {tl x y name start end len screen_x screen_y} {
   global tl_post
   toplevel $tl.infowin
   label $tl.infowin.l1 \
	 -text "State: $name, from $start to $end" \
	 -borderwidth 3 -relief raised
   label $tl.infowin.l2 \
	 -text "Length: $len sec."
   pack $tl.infowin.l1 $tl.infowin.l2
   # puts [wm overrideredirect $tl.infowin 1]
   wm transient $tl.infowin $tl
   set x_cor [expr $screen_x - [winfo reqwidth $tl.infowin.l1] / 2 + \
	 [winfo vrootx $tl]]
   set y_cor [expr $screen_y - [winfo reqheight $tl.infowin.l1] - \
	 [winfo reqheight $tl.infowin.l2] + \
	 [winfo vrooty $tl] - 10]
      # move it up by, oh, 10 pixels to account for a border
   wm geometry $tl.infowin +$x_cor+$y_cor
   set tl_post($tl) 1
}


proc CloseIt_Note {tl} {
   global tl_post
   if {[info exists tl_post($tl)]} {
      if {$tl_post($tl)} {
	 destroy $tl.infowin
      }
   }
   set tl_post($tl) 0
}




# wm minsize . 50 50
set width 500
set l [logfile open $argv alog]
timeline .t $l -width $width -height [expr [logfile $l np]*25] \
      -bg black -xscrollcommand {SetScrolls {.s .tlbl}}


time_lbl .tlbl $l

scrollbar .s -orient h -command {.t xview} -relief sunken

set time_mark "time:"
label .time_mark -textvariable time_mark
bind .t.c <Motion> "SetTimeMark .t %x"


button .zoomin -text "Zoom in" -command "ZoomIn $l .t"
button .zoomout -text "Zoom out" -command "ZoomOut $l .t"
button .printstates -text "Print states" -command ".t printstates"
button .close -text Close -command exit

entry .xview_point -textvariable xview_point -relief sunken
button .xview -text "Xview" -command {.t xview $xview_point}
set xview_point 0
# pack .xview .xview_point -side bottom

pack .close -pady 10 -side bottom
pack .zoomout .zoomin .time_mark -side bottom
pack .s .tlbl -fill x -side bottom
pack .t -fill both -expand 1 -side top

bind .t.c <1> "CheckStateInfoPost .t $l %x %y %X %Y"
bind .t.c <ButtonRelease-1> "CloseIt_Note .t"


wm minsize . [winfo reqwidth .zoomout] [expr [winfo reqheight .zoomin] + \
      [winfo reqheight .zoomout] + [winfo reqheight .printstates] + \
      [winfo reqheight .close]]

logfile $l load

