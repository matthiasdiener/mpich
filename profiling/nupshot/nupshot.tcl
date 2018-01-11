#!/usr/local/tcl/bin/wish -f
#
# Upshot - parallel program performance analysis tool
#
# Ed Karrels
# Argonne National Laboratory
# karrels@mcs.anl.gov
#

set defaultFile ~/.upshotrc


proc SetGlobalVars {} {
   global tcl_precision programName progName

   set programName "Nupshot"
   set progName nupshot
   set tcl_precision 17
}


proc SetDefaults {} {
   option add *Entry.Relief sunken
   option add *Scrollbar.Relief sunken
   option add *Menubutton.Relief raised
   option add *Canvas.Font [option get . font Font] 30
}

proc SetColors {} {
   global color bw

   # if no one else cares, set #colors automatically
   if {![info exists bw]} {set bw [expr \
	 "[winfo depth .] < 4"]}

   set color(bw_fg)        [GetDefault bw_fg black]
   set color(bw_bg)        [GetDefault bw_bg white]
   set color(bw_activefg)  [GetDefault bw_activefg white]
   set color(bw_activebg)  [GetDefault bw_activebg black]
   set color(bw_arrowfg)   [GetDefault bw_arrowfg black]
   set color(bw_menubar)   [GetDefault bw_menubar white]
   set color(bw_disp_bg)   [GetDefault bw_disp_bg white]
   set color(bw_timelinefg) [GetDefault bw_timelinefg black]

   set color(color_fg)        [GetDefault color_fg white]
   set color(color_bg)        [GetDefault color_bg SteelBlue]
   set color(color_activefg)  [GetDefault color_activefg white]
   set color(color_activebg)  [GetDefault color_activebg SteelBlue2]
   set color(color_arrowfg)   [GetDefault color_arrowfg black]
   set color(color_menubar)   [GetDefault color_menubar SteelBlue4]
   set color(color_disp_bg) [GetDefault bw_disp_bg black]
   set color(color_timelinefg) [GetDefault bw_timelinefg red]

   if {$bw} {
      set color(fg)         $color(bw_fg)
      set color(bg)         $color(bw_bg)
      set color(activefg)   $color(bw_activefg)
      set color(activebg)   $color(bw_activebg)
      set color(arrowfg)    $color(bw_arrowfg)
      set color(menubar)    $color(bw_menubar)
      set color(disp_bg)    $color(bw_disp_bg)
      set color(timelinefg) $color(bw_timelinefg)

      option add *foreground $color(fg) 30
      option add *background $color(bg) 30
      option add *selectForeground white 30
      option add *selectBackground black 30
      option add *activeForeground white 30
      option add *activeBackground $color(activebg) 30

   } else {
      set color(fg)         $color(color_fg)
      set color(bg)         $color(color_bg)
      set color(activefg)   $color(color_activefg)
      set color(activebg)   $color(color_activebg)
      set color(arrowfg)    $color(color_arrowfg)
      set color(menubar)    $color(color_menubar)
      set color(disp_bg)    $color(color_disp_bg)
      set color(timelinefg) $color(color_timelinefg)

      # Why doesn't GhostView like 'White' ?

      option add *foreground $color(fg) 30
      option add *background $color(bg) 30
      option add *activeForeground $color(activefg) 30
      option add *activeBackground $color(activebg) 30
   }

}


proc ProcessCmdLineArgs {} {
   global argv bw logFileName

   foreach parameter $argv {
      if {$parameter == "-bw" } {
	 #black and white screen
	 set bw 1
      } elseif {$parameter == "-c" } {
	 #color screen
	 set bw 0
      } else {
	 if [file exists $parameter] {
	    set logFileName $parameter
	 }
      }
   }
}




proc SigDigits {num start end ninterest {factor 1}} {
   # ninterest is the number of interesting digits to leave
   if {!($end-$start)} {
      set ndigits 0
   } else {
      set ndigits [expr int($ninterest-log10($end*$factor-$start*$factor))]
   }
   if {$ndigits<0} {set ndigits 0}
   return [format [format "%%.%df" $ndigits] [expr $num*$factor]]
}



proc LogFormatError {filename line lineNo} {
   puts "Logfile format error in line $lineNo of $filename:\n$line\n\n"
}


proc GuessFormat {filename} {

   if [regexp {.log$} $filename] {
      return alog
   } elseif [regexp {.trf$} $filename] {
      return picl
   } else {
      return [GetDefault logfileformat alog]
   }
}


# progdir should have been defined by nupshot.c

source $progdir/common.tcl
source $progdir/entries.tcl
source $progdir/fileselect.tcl
source $progdir/zoom.tcl
source $progdir/mainwin.tcl
source $progdir/timelines.tcl
# source $progdir/mtn.tcl
source $progdir/legend.tcl
source $progdir/procnums.tcl
source $progdir/hist.tcl
source $progdir/print.tcl
source $progdir/print_mainwin.tcl
source $progdir/print_hist.tcl
source $progdir/copy_canvas.tcl
source $progdir/trim_canvas.tcl

if 0 {
source optionswin.tcl
source timelinewin.tcl
source lists.tcl
source readaloglog.tcl
source readpicllog.tcl
source gatherevt.tcl
source scales.tcl
source pctdone.tcl
}


SetDefaults
SetGlobalVars
ProcessCmdLineArgs
SetColors
EntryBindings


   # Creating windows within the main window is always a pain because
   # the main window is just ".", so you can't add windows named ".x",
   # ".y" to it, unlike any other window.  So, how's this for a fix:

frame .f
pack .f -expand 1 -fill both


   # open main window
if [info exists logFileName] {
   NewWin .f 1 $logFileName
} else {
   NewWin .f 1
}

