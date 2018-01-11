proc NewWin {win is_main_win {logfile {}}} {
      # $logfile will be the default file open

   global win_info color

      # initialize data for this window

   set win_info($win,log) 0
   set win_info($win,empty) 1
   set win_info($win,is_main_win) $is_main_win

   if {$logfile == ""} {
      set title "Upshot"
   } else {
      set title "Upshot - $logfile"
   }

   if !$is_main_win {
      toplevel $win
      wm title $win $title
   }

      # create funky colored background frame
   frame $win.menu -relief raised -border 3 -back $color(menubar)

      # create main menus
   AddLogfileMenu $win $win.menu.logfile
   # AddOptionsMenu $win $win.menu.options

      # just to see the background color...
   # frame $win.menu.junk -width 50

   pack $win.menu -fill x
   # pack $win.menu.logfile $win.menu.options -side left
   pack $win.menu.logfile -side left

      # if a logfile was specified, load it
   if {$logfile != ""} {
      OpenFile $win $logfile [GuessFormat $logfile] \
	    [GetDefault displays timeline]
   }

}


proc AddLogfileMenu {id win} {
   global win_info
   # id is the index id for any neccessary data
   # win the the window to create

   menubutton $win -text "Logfile" -menu $win.menu

      # menu options
   set logmenu $win.menu
   menu $logmenu
   $logmenu add command -label "New frame" -command \
	 {NewWin .[GetUniqueWindowID] 0}

      # load a new logfile
   $logmenu add command -label "Open..." -command "SelectFile $id"

      # print the current view
   $logmenu add command \
	 -label "Print..." \
	 -state disabled

      # pick one of two exit options.  If this is the main window,
      # the user should know that destroying it will close the app
   if $win_info($id,is_main_win) {
      $logmenu add command -label "Exit Nupshot" -command ExitUpshot
   } else {
      $logmenu add command -label "Close frame" -command "CloseFile $id"
   }
}


proc EnablePrintMenu {log w} {
   global win_info

   # be sure to adjust this if the menus get moved
   $w.menu.logfile.menu entryconfigure 2 \
	 -state normal -command "Print_MainWin_Dialog $log $w"
}



proc AddOptionsMenu {id win} {
   # id is the index id for any neccessary data
   # win the the window to create

      # create menu button
   menubutton $win -text "Options" -menu $win.menu

      # add menu items
   menu $win.menu
   $win.menu add command -label "Colors..." -command ColorOptions
   $win.menu add command -label "Print..." -command PrintOptions

}


proc AddZoomMenu {id win} {
   # id is the index id for any neccessary data
   # win the the window to create

      # create menu button
   menubutton $win -text "Zoom" -menu $win.menu

      # add menu items
   menu $win.menu
   $win.menu add command -label "Zoom in horiz" -command \
	 "Zoom $id horiz 2"
   $win.menu add command -label "Zoom out horiz" -command \
	 "Zoom $id horiz .5"

   # These should not be here.  They should be on each widget.
   $win.menu add command -label "Zoom in vert" -command \
	 "Zoom $id vert 2"
   $win.menu add command -label "Zoom out vert" -command \
	 "Zoom $id vert .5"
}


proc SelectFile {win} {
   # Call fileselect and select something.  On 'OK' call OpenFile.
   fileselect "SelectedFile $win" "Open Logfile:" $win.selectfile
}

proc SelectedFile {win filename format} {
   OpenFile $win $filename $format [GetDefault displays timeline]
}


proc OpenFile {w logfile format displays} {
   global win_info

   # w is used to index into win_info(), as well as the toplevel window name

   # display is a list of display widgets that should be used to display
   # this file.  For example:
   #   { {timeline} {msgq in 3} }
   # would specify that two displays are to be opened:  timeline and
   # msgq (input queue on process 3)

   # left_gap is the width of the gap to leave to leave on the left
   # side for whatever various widgets wish to do with it
   # Better way to do this?
   set left_gap [GetDefault display_left_gap 40]

      # close the old logfile
   if {$win_info($w,log) != "0"} {
      logfile $win_info($w,log) close
   }

      # open the new logfile
   set openStatus [catch "logfile open $logfile $format" win_info($w,log)]
   if {$openStatus} {
      # if there was an error, let the user know
      set errwin .[GetUniqueWindowID]
      message $errwin -aspect 200 -text "Error opening logfile: \
	    $win_info($w,log)"
      return -1
   }

      # set the title of the window to the name of the logfile
   if $win_info($w,is_main_win) {
      wm title . "nupshot - $logfile"
   } else {
      wm title $w "nupshot - $logfile"
   }

      # if a logfile hasn't already been loaded in this window

   if {$win_info($w,empty)} {
      set win_info($w,empty) 0
      AddLogManipulationButtons $w
   } else {
         # close any existing displays
      destroy $w.controls
      # destroy $w.sep
      destroy $w.legend
      destroy $w.displays
      destroy $w.bottom
   }

   set font [option get $w font Font]

   EnablePrintMenu $win_info($w,log) $w

      # get the requested display widget width and height
   set widget_width [GetDefault "display widget width" 700]
   # set widget_height [GetDefault "display widget height" 300]

   Create_Controls $w.controls $w $win_info($w,log)
   # frame $w.sep -height 4 -relief raised -borderwidth 2

   # The names of the windows used in this procedure are highly
   # depended on by stuff in print.tcl.  Change those if
   # you change these.

   Legend_Create $w.legend $win_info($w,log) $widget_width


   frame $w.bottom

      # attach time labels and a scrollbar to the whole mess
   time_lbl $w.bottom.tlbl \
	 [logfile $win_info($w,log) starttime] \
	 [logfile $win_info($w,log) endtime] \
	 -font $font
   scrollbar $w.bottom.xscroll -command "XviewDisplays $w" -orient horiz
      # create list of windows that are scrollable

   set win_info($w,scrollers) [list $w.bottom.tlbl $w.bottom.xscroll]
   # set win_info($w,scrollers) {}

   # Let's hope this doesn't change much
   set scroll_width 19
   set right_gap $scroll_width

   frame $w.bottom.left_gap -width $left_gap
   frame $w.bottom.right_gap -width $right_gap
   pack $w.bottom.left_gap -side left -fill y
   pack $w.bottom.right_gap -side right -fill y
   pack $w.bottom.tlbl $w.bottom.xscroll -fill x

   # might want to throw in a static scale for the scrollbar

   pack $w.controls -fill x
   # pack $w.sep -fill x -padx 15 -pady 4
   pack $w.legend -fill x
   pack $w.bottom -side bottom -fill x

   update
   update
   update

      # create frame for all displays
   frame $w.displays

   set ndisplays [llength $displays]
   set maxheight [expr ([winfo screenheight .] - \
	 [winfo reqheight $w.menu] - \
	 [winfo reqheight $w.controls] - \
	 [winfo reqheight $w.legend] - \
	 [winfo reqheight $w.bottom] - 40) / $ndisplays]
   # fudge factor of 40 for the window manager's border
   # What to do if this is really small?

   set i 0
      # open up each display and attach it to the logfile
   foreach display $displays {
      # have each display send scroll commands to a central
      # point, attach to the same log, and tell a certain
      # 'ptrtime($w)' variable what time the pointer
      # is over

      OpenDisplay $w.displays.$i "$display -width $widget_width \
	    -xscrollcommand {ScrollDisplays $w} -timevar ptrtime($w) \
	    -setZoomPtCmd {Zoom_SetPoint $w} \
	    -maxheight $maxheight" \
	    $win_info($w,log)
	    
      pack $w.displays.$i -expand 1 -fill both
      incr i
   }
      # remember what displays are shown in this frame
   set win_info($w,displays) $displays
      # remember how many displays to update when scrolling & stuff
   set win_info($w,ndisplays) $i

   pack $w.displays -fill both -expand 1

   if $win_info($w,is_main_win) {
      set win .
   } else {
      set win $w
   }

   #wm minsize $win [winfo reqwidth $win] [winfo reqheight $win]
   
   wm minsize $win \
	 [maximum [list [winfo reqwidth $w.menu] \
	          [winfo reqwidth $w.controls]]] \
         [expr [winfo reqheight $w.menu] + \
	       [winfo reqheight $w.controls] + \
	       [winfo reqheight $w.legend] + \
	       [winfo reqheight $w.bottom]]

   update idletasks

   # puts stderr "About to load"
   logfile $win_info($w,log) load
}


   # send an 'xview' command to each of the display widgets, keeping
   # them all in sync
   #
   # win_info($id,ndisplays) should be set to the number of displays
   # The displays should be the windows $id.display.[0 -> (n-1)]

proc XviewDisplays {id xview} {
   global win_info

   set n $win_info($id,ndisplays)
   for {set i 0} {$i<$n} {incr i} {
      # puts "$id.displays.$i xview $xview"
      $id.displays.$i xview $xview
   }
}



   # send a 'set' command to all the x-scrolling widgets on this window

proc ScrollDisplays {id t w l r} {
   global win_info

   foreach scroller $win_info($id,scrollers) {
      # puts "$scroller set $t $w $l $r"
      $scroller set $t $w $l $r
   }
}


proc OpenDisplay {frame cmd log} {
      # separate out the widget command and the arguments
   set args [lrange $cmd 1 end]
   set widget [lindex $cmd 0]

   eval $widget $frame $log $args
}


proc ExitUpshot {} {
   exit
}

proc CloseFile win {
   destroy $win
}


proc AddLogManipulationButtons {win} {
   AddZoomMenu $win $win.menu.zoom
   pack $win.menu.zoom -side left
}


proc Create_Controls {frame win log} {
   frame $frame

      # display of the time the pointer is currently at
   frame $frame.time
   label $frame.time.lbl -text "Pointer: "
   entry $frame.time.val -relief sunken -textvariable ptrtime($win)
   label $frame.units -text "(in seconds)"

   pack $frame.time.lbl $frame.time.val -side left

      # zoom buttons
   frame $frame.z
   button $frame.z.ih -bitmap zoom_in_horiz  -command "Zoom $win horiz 2"
   button $frame.z.oh -bitmap zoom_out_horiz -command "Zoom $win horiz .5"
   button $frame.z.iv -bitmap zoom_in_vert   -command "Zoom $win vert 2"
   button $frame.z.ov -bitmap zoom_out_vert  -command "Zoom $win vert .5"
   pack $frame.z.ih $frame.z.oh $frame.z.iv $frame.z.ov -side left

   pack $frame.time $frame.units $frame.z -side left -padx 5 -pady 3
}
