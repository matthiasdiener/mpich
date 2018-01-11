proc Legend_Create {frame log width} {
   global bw color

   canvas $frame -height 1 -width $width -bg $color(bg) -relief raised
   set canvas $frame

   # margins
   set marginVert 5
   set marginHoriz 5
   # dimensions of boxes with sample color or bitmap
   set boxWidth 45
   set boxHeight 15
   set boxMiddle [expr $boxHeight/2]
   # space between box and corresponding label
   set spcBtwnBoxLbl 6
   # space between label and the next box
   set spcBtwnLblBox 10
   # vertical spacing between rows of boxes&labels
   set spcVert 10

   # draw each of the color boxes and labels
   set ycor 0
   set nstatedefs [logfile $log nstatedefs]
   for {set i 0} {$i < $nstatedefs} {incr i} {
      set def [logfile $log get_statedef $i]
      set name [lindex $def 0]
      set state_color [lindex $def 1]
      set bitmap [lindex $def 2]

      if $bw {
	 $canvas create rectangle 0 $ycor $boxWidth \
	       [expr $ycor+$boxHeight]  -fill $color(fg) \
	       -outline $color(fg) -tags [list legend sn_$i color_$i] \
	       -stipple $bitmap
      } else {
	 $canvas create rectangle 0 $ycor $boxWidth \
	       [expr $ycor+$boxHeight] -outline $color(fg) \
	       -fill $state_color \
	       -tags [list legend sn_$i color_$i]
      }
      button $canvas.b$i -text $name -command "Hist_Open $log $i"
      $canvas create window [expr $boxWidth+$spcBtwnBoxLbl] \
	    [expr $ycor+$boxMiddle] -window $canvas.b$i \
	    -tags [list legend sn_$i color_fg] -anchor w
      #set y-coordinate of next box and label
      set ycor [expr [lindex [$canvas bbox sn_$i] 3]+$spcVert]
   }

   # bind $canvas <Button> "Legend_Button $frame $log %x %y"
   bind $canvas <Configure> "Legend_Resize $frame $log %w"
   bind $canvas <Destroy> "Legend_Destroy $frame"
   # return [ResizeLegend $frame $log $givenWidth]
}


proc Legend_Resize {frame log givenWidth} {
   global bw color legend

   set canvas $frame

   if {![info exists legend($frame,width)] \
       || $legend($frame,width) != $givenWidth} {
      set legend($frame,width) $givenWidth
   } else {
      # puts "don't need to resize legend"
      return
   }

   set nstatedefs [logfile $log nstatedefs]

   # these are all repeated in CreateLegend
   # margins
   set marginVert 5
   set marginHoriz 5
   # dimensions of boxes with sample color or bitmap
   set boxWidth 45
   set boxHeight 15
   set boxMiddle [expr $boxHeight/2]
   # space between box and corresponding label
   set spcBtwnBoxLbl 6
   # space between label and the next box
   set spcBtwnLblBox 10
   # vertical spacing between rows of boxes&labels
   set spcVert 10

   set AddToLine {
      $canvas move sn_$i [expr $horiz-[lindex $bbox 0]] \
	    [expr $vert-[lindex $bbox 1]]
      if {$bheight>$maxHeight} {set maxHeight $bheight}
      set horiz [expr $horiz+$bwidth+$spcBtwnLblBox]
   }
   set ResetLine {
      set vert [expr $vert+$maxHeight]
      set horiz $marginHoriz
      set maxHeight 0
   }

   set width [expr $givenWidth-2*$marginHoriz]
   # legendLines = list of {maxHeight {0 1 2...}}
   set legendLines {}
   set horiz $marginHoriz
   set vert $marginVert
   # maxHeight = tallest state in this line
   set maxHeight 0
   # thisLine = {0 1 2... (which states are in this line)}
   set thisLine {}

   for {set i 0} {$i < $nstatedefs} {incr i} {
      # get width of current box and label
      set bbox [$canvas bbox sn_$i]
      set bwidth [expr [lindex $bbox 2]-[lindex $bbox 0]]
      set bheight [expr [lindex $bbox 3]-[lindex $bbox 1]]
      
      if {$bwidth+$spcBtwnLblBox+$horiz <= $width} {
	 # if this state fits, good
	 eval $AddToLine
      } elseif {$horiz==$marginHoriz} {
	 # if this state is too long, but the only one on the line,
	 # well, tough luck.
	 eval $AddToLine
	 eval $ResetLine
      } else {
	 # state is too long, go to next line
	 eval $ResetLine
	 eval $AddToLine
      }
   }

   set height [expr $vert+$marginVert+$maxHeight]
   $canvas configure -height $height
}



proc Legend_Copy {win new_canvas x y} {
   global legend

   set canvas $win

   CopyCanvas $canvas [list 0 0 \
	 [winfo width $canvas] \
	 [winfo height $canvas]] $new_canvas [list $x $y]
}



proc Legend_Destroy {win} {
   global legend
   unset legend($win,width)
}
