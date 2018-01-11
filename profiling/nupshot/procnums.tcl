proc Procnums_Create {win log} {
   global bw color procnums


   set display_gap 40


   set canvas $win.c

   frame $win
   canvas $canvas -height 1 -scrollincrement 1
   pack $canvas -fill both -expand 1

   set np [logfile $log np]

   # set textSize [GetTextSize $canvas]
   # set textWidth [lindex $textSize 0]
   # set nchars [expr int(log10($np)) + 1]
   # set width [expr ($nchars+1)*$textWidth]
   set procnums($win,width) [GetDefault display_left_gap $display_gap]

   # $canvas configure -width $width -bg $color(bg)
   $canvas configure -width $procnums($win,width) -relief sunken

   # can't set the height yet, wait for configure event and
   # a scroll set command


   set procnums($win,created) 0

   # Create widget command
   proc $win {args} "eval Procnums_Cmd $win $log \$args"

   bind $canvas <Configure> "Procnums_Resize $win $log %h"
}


proc Procnums_Cmd {win log cmd args} {

   set canvas $win.c

   switch -- $cmd {
      set {
	 if {[llength $args] != 4} {
	    error "Wrong # of args: $win set\
		  total_units window_units first_unit last_unit"
	 } else {
	    SplitList $args {t w f l}
	    Procnums_Set $win $log $t $w $f $l
	 }
      }
      copy {
	 if {[llength $args] != 3} {
	    error "Wrong # of args: <procnums> copy <dest_canvas> <x> <y>"
	 } else {
	    SplitList $args {dest_canvas x y}
	    Procnums_Copy $win $dest_canvas $x $y
	 }
      }
      
      default {
	 return [eval $canvas $cmd $args]
      }
   }

}


proc Procnums_Set {win log total_units window_units \
      first_unit last_unit} {
   global procnums color

   set canvas $win.c

   if {$procnums($win,created)} {
      if {$procnums($win,tu) != $total_units || \
	    $procnums($win,wu) != $window_units} {

	 set margin 5
	 set np [logfile $log np]
	 set totalheight [expr $procnums($win,height) * 1.0 * \
	       ($total_units - 1) / \
	       ($window_units + 1)]
	 set spacing [expr 1.0 * $totalheight / $np]
	 set offset [expr $spacing / 2.0]

	 for {set i 0} {$i < $np} {incr i} {
	    set y [expr $i * $spacing + $offset]
	    set id $procnums($win,id,$i)
	    set coords [$canvas coords $id]
	    set old_y [lindex $coords 1]
	    $canvas move $id 0 [expr $y - $old_y]
	 }
      }
   } else {

      set font [option get $canvas font Font]
      set margin 5
      set np [logfile $log np]
      set totalheight [expr $procnums($win,height) * 1.0 * \
	    ($total_units - 1) / \
	    ($window_units + 1)]
      set spacing [expr 1.0 * $totalheight / $np]
      set offset [expr $spacing / 2.0]
      
      for {set i 0} {$i < $np} {incr i} {
	 set y [expr $i * $spacing + $offset]
	 set procnums($win,id,$i) [$canvas create text \
	       [expr $procnums($win,width) - $margin] $y -text $i -anchor e \
	       -fill $color(fg) -font $font -tags color_fg]
      }
      set procnums($win,created) 1
   }

   $canvas yview [expr $procnums($win,height) * $first_unit / \
	 ($last_unit - $first_unit + 1)]

   set procnums($win,tu) $total_units
   set procnums($win,wu) $window_units
}

proc Procnums_Resize {win log height} {
   global procnums

   set canvas $win.c
   set spacing [expr 1.0 * $height / [logfile $log np]]
   set offset [expr $spacing / 2.0]

   # puts "Procnums_Resize called with height $height"

   if {[info exists procnums($win,height)]} {

      # puts "$canvas scale all 0 0 1 \[expr 1.0 * $height / $procnums($win,height)]"
      $canvas scale all 0 0 1 [expr 1.0 * $height / $procnums($win,height)]
   }



   set procnums($win,height) $height
}



proc Procnums_Copy {win dest_canvas x y} {
   set width [winfo width $win]
   set height [winfo height $win]
   set canvas $win.c
   set left [$canvas canvasx 0]
   set top [$canvas canvasy 0]
   set right [expr $left + $width]
   set bottom [expr $top + $height]

   CopyCanvas $canvas [list $left $top $right $bottom] $dest_canvas \
	 [list $x $y]
}
