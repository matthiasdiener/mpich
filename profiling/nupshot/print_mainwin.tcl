
# The names of the windows used in this procedure are highly
# dependent on the names given in mainwin.tcl.  Change these if
# you change those.


proc Print_MainWin_Dialog {log id} {
   
   set pwin .[GetUniqueWindowID]
   toplevel $pwin
   wm title $pwin "Print Logfile Display"

   frame $pwin.printbtn_frame -relief groove -borderwidth 4
   button $pwin.printbtn_frame.btn -text "Print" \
	 -command "Print_MainWin $log $id $pwin; destroy $pwin"
   pack $pwin.printbtn_frame.btn -padx 4 -pady 4

   button $pwin.cancel -text "Cancel" \
	 -command "destroy $pwin"

   Print_Options $pwin.options
   pack $pwin.options -fill both -expand 1 -side right
   pack $pwin.printbtn_frame $pwin.cancel -expand 1 -padx 10 -pady 10

   update idletasks
   wm minsize $pwin [winfo reqwidth $pwin] [winfo reqheight $pwin]
   bind $pwin <Destroy> "Print_SaveOpts"
}



proc Print_MainWin {log id pwin} {
   global win_info color printOpts

   LookBusy $pwin
   update

   set pc $pwin.pcanv
   canvas $pc
   if {$printOpts(colormode) == "mono"} {
      set fg $color(bw_fg)
      set bg $color(bw_bg)
   } else {
      set fg $color(color_fg)
      set bg $color(color_bg)
   }

   set fg black
   set bg white

   set bg_rect [$pc create rect 0 0 0 0 -outline $fg -fill $bg]

   set legend_width [winfo width $id.legend]
   set legend_height [winfo height $id.legend]
   set left_gap [GetDefault display_left_gap 40]

   set y [expr $legend_height + 1]
   set display_width [winfo width $id.displays.0]
   for {set i 0} {$i < $win_info($id,ndisplays)} {incr i} {
      set this_height [winfo height $id.displays.$i]
      # copy over each display
      $id.displays.$i copy $pc 0 $y
      $pc create rect 0 $y [expr $left_gap] [expr $y + $this_height + 1] \
	    -outline $fg
      # scrollbars are 19 pixels wide -- that the gap on the right
      $pc create rect $left_gap $y [expr $display_width - 19] \
	    [expr $y + $this_height + 1] -outline $fg
      incr y [expr 1 + $this_height]
   }

   # copy over legend
   $pc create rect 0 0 [expr $legend_width + 1] [expr $legend_height + 1] \
	 -outline $fg -fill $bg
   Legend_Copy $id.legend $pc 1 1

   set time_lbl_height [winfo height $id.bottom.tlbl]

   # copy over time_lbl
   
   $pc create rect 0 $y [expr $legend_width + 1] \
	 [expr $time_lbl_height + $y + 2] \
	 -outline $fg -fill $bg
   $id.bottom.tlbl copy $pc $left_gap [expr $y + 2]

   $pc coords $bg_rect 0 0 [expr $legend_width + 1] \
	 [expr $y + $time_lbl_height + 3]

   if {$printOpts(colormode) != "mono"} {
      # if we're in b&w, colorize it
      Print_Mainwin_Recolor $pc $log 0
   } else {
      # if we're in color, b&w-ize it
      Print_Mainwin_Recolor $pc $log 1
   }

   Print_Canvas $log $pc 0 0 [expr $legend_width + 2] \
	 [expr $y + $time_lbl_height + 3]

   destroy $pc

   # $pc config -height [expr $y + $time_lbl_height + 2] \
	 -width [expr $legend_width + 1]
   # pack $pc

   LookBored $pwin
}


proc Print_Mainwin_Recolor {canvas log bw} {
   global color

   set fg black
   set bg white
   set timelinefg black
   $canvas config -bg white

   if {$bw} {
      # set fg $color(bw_fg)
      # set bg $color(bw_bg)
      # set timelinefg $color(bw_timelinefg)

      # $canvas config -bg $color(bw_timelinebg)

      set n [logfile $log nstatedefs]
      for {set i 0} {$i < $n} {incr i} {
	 set st_bitmap [lindex [logfile $log get_statedef $i] 2]
	 $canvas itemconfig color_$i -stipple $st_bitmap -fill $fg \
	       -outline $fg
      }
   } else {
      # set fg $color(color_fg)
      # set bg $color(color_bg)
      # set timelinefg $color(color_timelinefg)

      # $canvas config -bg $color(color_timelinebg)

      set n [logfile $log nstatedefs]
      for {set i 0} {$i < $n} {incr i} {
	 set st_color [lindex [logfile $log get_statedef $i] 1]
	 $canvas itemconfig color_$i -stipple "" -fill $st_color \
	       -outline $fg
      }
   }
   $canvas itemconfig color_fg -stipple "" -fill $fg
   $canvas itemconfig color_bg -stipple "" -fill $bg
   $canvas itemconfig color_timelinefg -fill $timelinefg
   
}

