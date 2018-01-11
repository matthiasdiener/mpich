 
# Here are a bunch of common Tcl routines that I use.
# I don't want to try auto_load yet, since that will probably be more
# of a pain to install cleanly.
#
#  LookBusy win
#    Change the cursor to a watch, starting at the given window on down
#    the hierarchy tree.

#  LookBored win
#    Change the cursor to a top_left cursor (normal), starting at the
#    given window on down the hierarchy tree.

#  GetDefault index default
#    Look in my private default database for an item with the given
#    index.  If the given index is not found, return <default>.
#    Will look in the file 'defaultFile' (global variable)

#  and a bunch of others I don't have time to document right now...
# 
# Ed Karrels
# Argonne National Laboratory



if ![info exists package(common)] {
set package(common) yup


proc LookBusy {win} {
   $win config -cursor watch
   foreach child [winfo children $win] {
      LookBusy $child
   }
}


   
proc LookBored {win} {
   $win config -cursor top_left_arrow
   foreach child [winfo children $win] {
      LookBored $child
   }
}

proc GetDefault {index default} {
   global defaults defaultFile

   if ![info exists defaults($index)] {
      if [file readable $defaultFile] {
	 set fileHandle [open $defaultFile r]
	 while {[gets $fileHandle str]>=0} {
	    scan $str "%s %\[^\n\]" readIndex readValue
	    set defaults($readIndex) $readValue
	 }
	 close $fileHandle
      } else {
	 # puts "Cannot read $defaultFile"
	 return $default
      }	
      if [info exists defaults($index)] {
	 return $defaults($index)
      } else {
	 return $default
      }
   } else {
      return $defaults($index)
   }
}



proc UpdateDefaults {{newValueList {}}} {
      # use this function to update the list of defaults and write them
      # out to the file

   global defaults defaultFile

   set numValues [expr [llength $newValueList]/2]
   for {set i 0} {$i<$numValues} {incr i} {
      set defaults([lindex $newValueList [expr $i*2]]) \
	    [lindex $newValueList [expr $i*2+1]]
   }

      # if we have read rights to the file, read it
   if [file readable $defaultFile] {
      set fileHandle [open $defaultFile r]
      while {[gets $fileHandle str]>=0} {
	 scan $str "%s %s" readIndex readValue
	 if ![info exists defaults($readIndex)] {
	    set defaults($readIndex) $readValue
	 }
      }
      close $fileHandle
   }

   if {[file exists $defaultFile] ? [file writable $defaultFile] : \
	 [file writable [file dirname $defaultFile]]} {
      set fileHandle [open $defaultFile w]
      foreach idx [array names defaults] {
	 puts $fileHandle "$idx $defaults($idx)"
      }
      close $fileHandle
   }
}



proc GetUniqueWindowID {} {
   global lastWindowID

   if ![info exists lastWindowID] {
      set lastWindowID 0
   } else {
      set lastWindowID [expr $lastWindowID+1]
   }

   return $lastWindowID
}



proc EraseArrayElements {array_name idx_header} {
   upvar $array_name a

   set pattern "^$idx_header,"
   foreach idx [array names a] {
      if [regexp $pattern $idx] {
	 unset a($idx)
      }
   }
}



proc GetTextSize {canvas} {
   set id [$canvas create text 0 0 -text "W" -anchor nw]
   set sz0 [$canvas bbox $id]
   $canvas delete $id

   set id [$canvas create text 0 0 -text "WW\nWW" -anchor nw]
   set sz1 [$canvas bbox $id]
   $canvas delete $id

   # return list: {width, height}
   set width [expr [lindex $sz1 2]-[lindex $sz0 2]]
   set height [expr [lindex $sz1 3]-[lindex $sz0 3]]
   return [list $width $height]
}



proc SplitList {l vars} {
   set i 0
   if {[llength $l] != [llength $vars]} {
      error "unequal list lengths"
      return
   }
   foreach element $l {
      uplevel "set [lindex $vars $i] {$element}"
      incr i
   }
}



proc minimum {nums} {
   set first 1
   set min 0
   foreach num $nums {
      if $first {
	 set min $num
	 set first 0
      } else {
	 if {$num < $min} {
	    set min $num
	 }
      }
   }
   return $min
}


proc maximum {nums} {
   set first 1
   set max 0
   foreach num $nums {
      if $first {
	 set max $num
	 set first 0
      } else {
	 if {$num > $max} {
	    set max $num
	 }
      }
   }
   return $max
}


proc ArgOpt {arg_list_var opt dest_var} {
   set grab_next 0
   set found 0

   upvar $arg_list_var arg_list
   upvar $dest_var dest

   foreach arg $arg_list {
      if {$grab_next} {
	 set dest $arg
	 set grab_next 0
	 set found 1
      } elseif {"$arg" == "$opt"} {
	 set grab_next 1
      }
   }

   return $found
}

proc ArgPresent {arg_list_var opt} {
   upvar $arg_list_var arg_list

   foreach arg $arg_list {
      if {"$arg" == "$opt"} {
	 return 1
      }
   }

   return 0;
}


proc ArgBool {arg_list_var opt dest_var} {
   upvar $arg_list_var arg_list
   upvar $dest_var dest

   if [ArgOpt arg_list $opt temp] {
      if {$temp} {
	 set dest 1
      } else {
	 set dest 0
      }
   } else {
      return 0
   }
}


proc U {lista listb} {
   if {[llength $lista]<[llength $listb]} {
      set n [llength $listb]
      for {set i 0} {$i<$n} {incr i} {
	 set el [lindex $listb $i]
	 if {[lsearch $lista $el]==-1} {
	    lappend lista $el
	 }
      }
      return $lista
   } else {
      set n [llength $lista]
      for {set i 0} {$i<$n} {incr i} {
	 set el [lindex $lista $i]
	 if {[lsearch $listb $el]==-1} {
	    lappend listb $el
	 }
      }
      return $listb
   }
}


proc N {lista listb} {
   set list {}
   if {[llength $lista]<[llength $listb]} {
      set n [llength $listb]
      for {set i 0} {$i<$n} {incr i} {
	 set el [lindex $listb $i]
	 if {[lsearch $lista $el]!=-1} {
	    lappend list $el
	 }
      }
   } else {
      set n [llength $lista]
      for {set i 0} {$i<$n} {incr i} {
	 set el [lindex $lista $i]
	 if {[lsearch $listb $el]!=-1} {
	    lappend list $el
	 }
      }
   }
   return $list
}


proc swap {av bv} {
   upvar $av a
   upvar $bv b
   set c $a
   set a $b
   set b $c
}


}
# package(common)
