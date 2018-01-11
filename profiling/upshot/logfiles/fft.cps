%!PS-Adobe-3.0 EPSF-3.0
%%Creator: Tk Canvas Widget
%%For: Rusty Lusk,
%%Title: Window .0.c
%%CreationDate: Sun Nov  6 23:00:12 1994
%%BoundingBox: 64 36 547 757
%%Pages: 1
%%DocumentData: Clean7Bit
%%Orientation: Landscape
%%DocumentNeededResources: font Helvetica-Bold
%%EndComments

% This file contains the standard Postscript prolog used when
% generating Postscript from canvas widgets.
%
% $Header: /user6/ouster/wish/library/RCS/prolog.ps,v 1.6 93/04/01 14:03:52 ouster Exp $ SPRITE (Berkeley);

%%BeginProlog
50 dict begin

% The definitions below just define all of the variables used in
% any of the procedures here.  This is needed for obscure reasons
% explained on p. 716 of the Postscript manual (Section H.2.7,
% "Initializing Variables," in the section on Encapsulated Postscript).

/baseline 0 def
/stipimage 0 def
/height 0 def
/justify 0 def
/maxwidth 0 def
/spacing 0 def
/stipple 0 def
/strings 0 def
/xoffset 0 def
/yoffset 0 def
/tmpstip null def
/encoding {ISOLatin1Encoding} def

% Override setfont to automatically encode the font in the style defined by 
% by 'encoding' (ISO Latin1 by default).

systemdict /encodefont known {
    /realsetfont /setfont load def
    /setfont {
	encoding encodefont realsetfont
    } def
} if

% desiredSize EvenPixels closestSize
%
% The procedure below is used for stippling.  Given the optimal size
% of a dot in a stipple pattern in the current user coordinate system,
% compute the closest size that is an exact multiple of the device's
% pixel size.  This allows stipple patterns to be displayed without
% aliasing effects.

/EvenPixels {
    % Compute exact number of device pixels per stipple dot.
    dup 0 matrix currentmatrix dtransform
    dup mul exch dup mul add sqrt

    % Round to an integer, make sure the number is at least 1, and compute
    % user coord distance corresponding to this.
    dup round dup 1 lt {pop 1} if
    exch div mul
} bind def

% width height string filled StippleFill --
%
% Given a path and other graphics information already set up, this
% procedure will fill the current path in a stippled fashion.  "String"
% contains a proper image description of the stipple pattern and
% "width" and "height" give its dimensions.  If "filled" is true then
% it means that the area to be stippled is gotten by filling the
% current path (e.g. the interior of a polygon); if it's false, the
% area is gotten by stroking the current path (e.g. a wide line).
% Each stipple dot is assumed to be about one unit across in the
% current user coordinate system.

/StippleFill {
    % Turn the path into a clip region that we can then cover with
    % lots of images corresponding to the stipple pattern.  Warning:
    % some Postscript interpreters get errors during strokepath for
    % dashed lines.  If this happens, turn off dashes and try again.

    1 index /tmpstip exch def %% Works around NeWSprint bug

    gsave
    {eoclip}
    {{strokepath} stopped {grestore gsave [] 0 setdash strokepath} if clip}
    ifelse

    % Change the scaling so that one user unit in user coordinates
    % corresponds to the size of one stipple dot.
    1 EvenPixels dup scale

    % Compute the bounding box occupied by the path (which is now
    % the clipping region), and round the lower coordinates down
    % to the nearest starting point for the stipple pattern.

    pathbbox
    4 2 roll
    5 index div cvi 5 index mul 4 1 roll
    6 index div cvi 6 index mul 3 2 roll

    % Stack now: width height string y1 y2 x1 x2
    % Below is a doubly-nested for loop to iterate across this area
    % in units of the stipple pattern size, going up columns then
    % across rows, blasting out a stipple-pattern-sized rectangle at
    % each position

    6 index exch {
	2 index 5 index 3 index {
	    % Stack now: width height string y1 y2 x y

	    gsave
	    1 index exch translate
	    5 index 5 index true matrix tmpstip imagemask
	    grestore
	} for
	pop
    } for
    pop pop pop pop pop
    grestore
    newpath
} bind def

% -- AdjustColor --
% Given a color value already set for output by the caller, adjusts
% that value to a grayscale or mono value if requested by the CL
% variable.

/AdjustColor {
    CL 2 lt {
	currentgray
	CL 0 eq {
	    .5 lt {0} {1} ifelse
	} if
	setgray
    } if
} bind def

% x y strings spacing xoffset yoffset justify stipple stipimage DrawText --
% This procedure does all of the real work of drawing text.  The
% color and font must already have been set by the caller, and the
% following arguments must be on the stack:
%
% x, y -	Coordinates at which to draw text.
% strings -	An array of strings, one for each line of the text item,
%		in order from top to bottom.
% spacing -	Spacing between lines.
% xoffset -	Horizontal offset for text bbox relative to x and y: 0 for
%		nw/w/sw anchor, -0.5 for n/center/s, and -1.0 for ne/e/se.
% yoffset -	Vertical offset for text bbox relative to x and y: 0 for
%		nw/n/ne anchor, +0.5 for w/center/e, and +1.0 for sw/s/se.
% justify -	0 for left justification, 0.5 for center, 1 for right justify.
% stipple -	Boolean value indicating whether or not text is to be
%		drawn in stippled fashion.
% stipimage -	Image for stippling, if stipple is True.
%
% Also, when this procedure is invoked, the color and font must already
% have been set for the text.

/DrawText {
    /stipimage exch def
    /stipple exch def
    /justify exch def
    /yoffset exch def
    /xoffset exch def
    /spacing exch def
    /strings exch def

    % First scan through all of the text to find the widest line.

    /maxwidth 0 def
    strings {
	stringwidth pop
	dup maxwidth gt {/maxwidth exch def} {pop} ifelse
	newpath
    } forall

    % Compute the baseline offset and the actual font height.

    0 0 moveto (TXygqPZ) false charpath
    pathbbox dup /baseline exch def
    exch pop exch sub /height exch def pop
    newpath

    % Translate coordinates first so that the origin is at the upper-left
    % corner of the text's bounding box. Remember that x and y for
    % positioning are still on the stack.

    translate
    maxwidth xoffset mul
    strings length 1 sub spacing mul height add yoffset mul translate

    % Now use the baseline and justification information to translate so
    % that the origin is at the baseline and positioning point for the
    % first line of text.

    justify maxwidth mul baseline neg translate

    % Iterate over each of the lines to output it.  For each line,
    % compute its width again so it can be properly justified, then
    % display it.

    strings {
	dup stringwidth pop
	justify neg mul 0 moveto
	show
	0 spacing neg translate
    } forall
} bind def

%%EndProlog
%%BeginSetup
/CL 2 def
%%IncludeResource: font Helvetica-Bold
%%EndSetup

%%Page: 1 1
save
306.0 396.0 translate
90 rotate
0.96 0.96 scale
-345 -251 translate
-30 503 moveto 720 503 lineto 720 0 lineto -30 0 lineto closepath clip newpath
gsave
0 416.5 moveto
700 416.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 391.5 moveto
700 391.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 366.5 moveto
700 366.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 341.5 moveto
700 341.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 316.5 moveto
700 316.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 291.5 moveto
700 291.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 266.5 moveto
700 266.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 241.5 moveto
700 241.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 216.5 moveto
700 216.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 191.5 moveto
700 191.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 166.5 moveto
700 166.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 141.5 moveto
700 141.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 116.5 moveto
700 116.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 91.5 moveto
700 91.5 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 66.4999999999999 moveto
700 66.4999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
0 41.4999999999999 moveto
700 41.4999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.000 0.000 setrgbcolor AdjustColor
stroke
grestore
gsave
43.7958755679832 99.4999999999999 moveto 138.116043341489 0 rlineto 0 -15.9999999999999 rlineto -138.116043341489 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
43.7958755679832 99.4999999999999 moveto 138.116043341489 0 rlineto 0 -15.9999999999999 rlineto -138.116043341489 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
45.3862285914016 149.5 moveto 136.770360013981 0 rlineto 0 -16 rlineto -136.770360013981 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
45.3862285914016 149.5 moveto 136.770360013981 0 rlineto 0 -16 rlineto -136.770360013981 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
45.6308982873121 74.5 moveto 136.525690318071 0 rlineto 0 -16 rlineto -136.525690318071 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
45.6308982873121 74.5 moveto 136.525690318071 0 rlineto 0 -16 rlineto -136.525690318071 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
43.7958755679832 299.5 moveto 138.360713037399 0 rlineto 0 -16 rlineto -138.360713037399 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
43.7958755679832 299.5 moveto 138.360713037399 0 rlineto 0 -16 rlineto -138.360713037399 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
44.8968891995806 224.5 moveto 137.504369101713 0 rlineto 0 -16 rlineto -137.504369101713 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
44.8968891995806 224.5 moveto 137.504369101713 0 rlineto 0 -16 rlineto -137.504369101713 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
44.162880111849 199.5 moveto 138.3607130374 0 rlineto 0 -16 rlineto -138.3607130374 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
44.162880111849 199.5 moveto 138.3607130374 0 rlineto 0 -16 rlineto -138.3607130374 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
43.673540720028 399.5 moveto 139.951066060818 0 rlineto 0 -16 rlineto -139.951066060818 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
43.673540720028 399.5 moveto 139.951066060818 0 rlineto 0 -16 rlineto -139.951066060818 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
45.3862285914016 174.5 moveto 138.483047885355 0 rlineto 0 -16 rlineto -138.483047885355 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
45.3862285914016 174.5 moveto 138.483047885355 0 rlineto 0 -16 rlineto -138.483047885355 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
46.2425725270884 324.5 moveto 137.626703949668 0 rlineto 0 -16 rlineto -137.626703949668 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
46.2425725270884 324.5 moveto 137.626703949668 0 rlineto 0 -16 rlineto -137.626703949668 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
45.2638937434463 274.5 moveto 139.094722125131 0 rlineto 0 -16 rlineto -139.094722125131 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
45.2638937434463 274.5 moveto 139.094722125131 0 rlineto 0 -16 rlineto -139.094722125131 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
44.2852149598043 124.5 moveto 141.174414540371 0 rlineto 0 -16 rlineto -141.174414540371 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
44.2852149598043 124.5 moveto 141.174414540371 0 rlineto 0 -16 rlineto -141.174414540371 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
43.7958755679832 424.5 moveto 142.153093324013 0 rlineto 0 -16 rlineto -142.153093324013 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
43.7958755679832 424.5 moveto 142.153093324013 0 rlineto 0 -16 rlineto -142.153093324013 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
44.5298846557148 349.5 moveto 141.419084236281 0 rlineto 0 -16 rlineto -141.419084236281 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
44.5298846557148 349.5 moveto 141.419084236281 0 rlineto 0 -16 rlineto -141.419084236281 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
47.5882558545963 49.5 moveto 138.60538273331 0 rlineto 0 -16 rlineto -138.60538273331 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
47.5882558545963 49.5 moveto 138.60538273331 0 rlineto 0 -16 rlineto -138.60538273331 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
44.2852149598043 374.5 moveto 141.908423628102 0 rlineto 0 -16 rlineto -141.908423628102 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
44.2852149598043 374.5 moveto 141.908423628102 0 rlineto 0 -16 rlineto -141.908423628102 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
48.4445997902831 249.5 moveto 137.993708493534 0 rlineto 0 -16 rlineto -137.993708493534 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
48.4445997902831 249.5 moveto 137.993708493534 0 rlineto 0 -16 rlineto -137.993708493534 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
193.166724921356 149.5 moveto 132.121635791681 0 rlineto 0 -16 rlineto -132.121635791681 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
193.166724921356 149.5 moveto 132.121635791681 0 rlineto 0 -16 rlineto -132.121635791681 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
194.390073400909 299.5 moveto 131.142957008039 0 rlineto 0 -16 rlineto -131.142957008039 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
194.390073400909 299.5 moveto 131.142957008039 0 rlineto 0 -16 rlineto -131.142957008039 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
196.225096120238 174.5 moveto 130.164278224397 0 rlineto 0 -16 rlineto -130.164278224397 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
196.225096120238 174.5 moveto 130.164278224397 0 rlineto 0 -16 rlineto -130.164278224397 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
193.533729465222 224.5 moveto 133.589653967144 0 rlineto 0 -16 rlineto -133.589653967144 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
193.533729465222 224.5 moveto 133.589653967144 0 rlineto 0 -16 rlineto -133.589653967144 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
192.922055225446 74.5 moveto 134.690667598742 0 rlineto 0 -16 rlineto -134.690667598742 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
192.922055225446 74.5 moveto 134.690667598742 0 rlineto 0 -16 rlineto -134.690667598742 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
192.922055225446 99.4999999999999 moveto 134.935337294652 0 rlineto 0 -15.9999999999999 rlineto -134.935337294652 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
192.922055225446 99.4999999999999 moveto 134.935337294652 0 rlineto 0 -15.9999999999999 rlineto -134.935337294652 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
195.246417336596 324.5 moveto 134.445997902831 0 rlineto 0 -16 rlineto -134.445997902831 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
195.246417336596 324.5 moveto 134.445997902831 0 rlineto 0 -16 rlineto -134.445997902831 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
194.757077944775 199.5 moveto 134.935337294652 0 rlineto 0 -16 rlineto -134.935337294652 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
194.757077944775 199.5 moveto 134.935337294652 0 rlineto 0 -16 rlineto -134.935337294652 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
197.081440055925 374.5 moveto 133.100314575323 0 rlineto 0 -16 rlineto -133.100314575323 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
197.081440055925 374.5 moveto 133.100314575323 0 rlineto 0 -16 rlineto -133.100314575323 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
194.390073400909 399.5 moveto 135.914016078294 0 rlineto 0 -16 rlineto -135.914016078294 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
194.390073400909 399.5 moveto 135.914016078294 0 rlineto 0 -16 rlineto -135.914016078294 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
196.959105207969 424.5 moveto 134.568332750786 0 rlineto 0 -16 rlineto -134.568332750786 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
196.959105207969 424.5 moveto 134.568332750786 0 rlineto 0 -16 rlineto -134.568332750786 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
196.592100664103 124.5 moveto 134.935337294652 0 rlineto 0 -16 rlineto -134.935337294652 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
196.592100664103 124.5 moveto 134.935337294652 0 rlineto 0 -16 rlineto -134.935337294652 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
197.570779447746 249.5 moveto 134.078993358965 0 rlineto 0 -16 rlineto -134.078993358965 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
197.570779447746 249.5 moveto 134.078993358965 0 rlineto 0 -16 rlineto -134.078993358965 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
196.714435512059 274.5 moveto 135.669346382384 0 rlineto 0 -16 rlineto -135.669346382384 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
196.714435512059 274.5 moveto 135.669346382384 0 rlineto 0 -16 rlineto -135.669346382384 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
196.959105207969 349.5 moveto 136.770360013981 0 rlineto 0 -16 rlineto -136.770360013981 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
196.959105207969 349.5 moveto 136.770360013981 0 rlineto 0 -16 rlineto -136.770360013981 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
198.549458231388 49.5 moveto 135.669346382384 0 rlineto 0 -16 rlineto -135.669346382384 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
198.549458231388 49.5 moveto 135.669346382384 0 rlineto 0 -16 rlineto -135.669346382384 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
336.0538273331 299.5 moveto 132.243970639636 0 rlineto 0 -16 rlineto -132.243970639636 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
336.0538273331 299.5 moveto 132.243970639636 0 rlineto 0 -16 rlineto -132.243970639636 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
336.910171268787 174.5 moveto 133.100314575323 0 rlineto 0 -16 rlineto -133.100314575323 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
336.910171268787 174.5 moveto 133.100314575323 0 rlineto 0 -16 rlineto -133.100314575323 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
339.723872771758 224.5 moveto 131.142957008039 0 rlineto 0 -16 rlineto -131.142957008039 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
339.723872771758 224.5 moveto 131.142957008039 0 rlineto 0 -16 rlineto -131.142957008039 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
338.13351974834 74.5 moveto 132.977979727368 0 rlineto 0 -16 rlineto -132.977979727368 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
338.13351974834 74.5 moveto 132.977979727368 0 rlineto 0 -16 rlineto -132.977979727368 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
340.213212163579 199.5 moveto 130.898287312129 0 rlineto 0 -16 rlineto -130.898287312129 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
340.213212163579 199.5 moveto 130.898287312129 0 rlineto 0 -16 rlineto -130.898287312129 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
341.314225795177 324.5 moveto 131.876966095771 0 rlineto 0 -16 rlineto -131.876966095771 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
341.314225795177 324.5 moveto 131.876966095771 0 rlineto 0 -16 rlineto -131.876966095771 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
337.888850052429 149.5 moveto 136.03635092625 0 rlineto 0 -16 rlineto -136.03635092625 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
337.888850052429 149.5 moveto 136.03635092625 0 rlineto 0 -16 rlineto -136.03635092625 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
342.904578818595 274.5 moveto 131.020622160084 0 rlineto 0 -16 rlineto -131.020622160084 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
342.904578818595 274.5 moveto 131.020622160084 0 rlineto 0 -16 rlineto -131.020622160084 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
341.803565186998 124.5 moveto 132.243970639637 0 rlineto 0 -16 rlineto -132.243970639637 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
341.803565186998 124.5 moveto 132.243970639637 0 rlineto 0 -16 rlineto -132.243970639637 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
338.37818944425 99.4999999999999 moveto 136.525690318071 0 rlineto 0 -15.9999999999999 rlineto -136.525690318071 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
338.37818944425 99.4999999999999 moveto 136.525690318071 0 rlineto 0 -15.9999999999999 rlineto -136.525690318071 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
344.617266689969 49.5 moveto 131.509961551905 0 rlineto 0 -16 rlineto -131.509961551905 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
344.617266689969 49.5 moveto 131.509961551905 0 rlineto 0 -16 rlineto -131.509961551905 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
341.803565186998 424.5 moveto 135.424676686473 0 rlineto 0 -16 rlineto -135.424676686473 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
341.803565186998 424.5 moveto 135.424676686473 0 rlineto 0 -16 rlineto -135.424676686473 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
344.005592450192 349.5 moveto 133.7119888151 0 rlineto 0 -16 rlineto -133.7119888151 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
344.005592450192 349.5 moveto 133.7119888151 0 rlineto 0 -16 rlineto -133.7119888151 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
343.516253058371 249.5 moveto 136.403355470115 0 rlineto 0 -16 rlineto -136.403355470115 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
343.516253058371 249.5 moveto 136.403355470115 0 rlineto 0 -16 rlineto -136.403355470115 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
344.250262146103 399.5 moveto 136.403355470115 0 rlineto 0 -16 rlineto -136.403355470115 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
344.250262146103 399.5 moveto 136.403355470115 0 rlineto 0 -16 rlineto -136.403355470115 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
340.45788185949 374.5 moveto 155.976931142957 0 rlineto 0 -16 rlineto -155.976931142957 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
340.45788185949 374.5 moveto 155.976931142957 0 rlineto 0 -16 rlineto -155.976931142957 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
478.818594896889 299.5 moveto 132.610975183502 0 rlineto 0 -16 rlineto -132.610975183502 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
478.818594896889 299.5 moveto 132.610975183502 0 rlineto 0 -16 rlineto -132.610975183502 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
481.63229639986 74.5 moveto 130.531282768263 0 rlineto 0 -16 rlineto -130.531282768263 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
481.63229639986 74.5 moveto 130.531282768263 0 rlineto 0 -16 rlineto -130.531282768263 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
480.531282768263 174.5 moveto 132.488640335547 0 rlineto 0 -16 rlineto -132.488640335547 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
480.531282768263 174.5 moveto 132.488640335547 0 rlineto 0 -16 rlineto -132.488640335547 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
481.38762670395 224.5 moveto 132.733310031458 0 rlineto 0 -16 rlineto -132.733310031458 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
481.38762670395 224.5 moveto 132.733310031458 0 rlineto 0 -16 rlineto -132.733310031458 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
481.38762670395 199.5 moveto 132.855644879413 0 rlineto 0 -16 rlineto -132.855644879413 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
481.38762670395 199.5 moveto 132.855644879413 0 rlineto 0 -16 rlineto -132.855644879413 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
484.323663054876 274.5 moveto 132.121635791681 0 rlineto 0 -16 rlineto -132.121635791681 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
484.323663054876 274.5 moveto 132.121635791681 0 rlineto 0 -16 rlineto -132.121635791681 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
484.690667598742 149.5 moveto 131.876966095771 0 rlineto 0 -16 rlineto -131.876966095771 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
484.690667598742 149.5 moveto 131.876966095771 0 rlineto 0 -16 rlineto -131.876966095771 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
485.914016078294 124.5 moveto 131.020622160084 0 rlineto 0 -16 rlineto -131.020622160084 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
485.914016078294 124.5 moveto 131.020622160084 0 rlineto 0 -16 rlineto -131.020622160084 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
483.589653967144 324.5 moveto 133.834323663055 0 rlineto 0 -16 rlineto -133.834323663055 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
483.589653967144 324.5 moveto 133.834323663055 0 rlineto 0 -16 rlineto -133.834323663055 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
486.525690318071 49.5 moveto 131.509961551905 0 rlineto 0 -16 rlineto -131.509961551905 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
486.525690318071 49.5 moveto 131.509961551905 0 rlineto 0 -16 rlineto -131.509961551905 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
488.727717581265 99.4999999999999 moveto 131.142957008039 0 rlineto 0 -15.9999999999999 rlineto -131.142957008039 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
488.727717581265 99.4999999999999 moveto 131.142957008039 0 rlineto 0 -15.9999999999999 rlineto -131.142957008039 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
491.052079692415 399.5 moveto 131.142957008039 0 rlineto 0 -16 rlineto -131.142957008039 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
491.052079692415 399.5 moveto 131.142957008039 0 rlineto 0 -16 rlineto -131.142957008039 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
490.318070604684 249.5 moveto 131.876966095771 0 rlineto 0 -16 rlineto -131.876966095771 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
490.318070604684 249.5 moveto 131.876966095771 0 rlineto 0 -16 rlineto -131.876966095771 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
487.749038797623 424.5 moveto 136.525690318071 0 rlineto 0 -16 rlineto -136.525690318071 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
487.749038797623 424.5 moveto 136.525690318071 0 rlineto 0 -16 rlineto -136.525690318071 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
487.993708493534 349.5 moveto 136.770360013981 0 rlineto 0 -16 rlineto -136.770360013981 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
487.993708493534 349.5 moveto 136.770360013981 0 rlineto 0 -16 rlineto -136.770360013981 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
507.44494931842 374.5 moveto 135.057672142607 0 rlineto 0 -16 rlineto -135.057672142607 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
507.44494931842 374.5 moveto 135.057672142607 0 rlineto 0 -16 rlineto -135.057672142607 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
60.5557497378539 246.5 moveto 39.6364907375044 0 rlineto 0 -9.99999999999997 rlineto -39.6364907375044 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
60.5557497378539 246.5 moveto 39.6364907375044 0 rlineto 0 -9.99999999999997 rlineto -39.6364907375044 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
55.907025515554 296.5 moveto 49.0562740300594 0 rlineto 0 -10 rlineto -49.0562740300594 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
55.907025515554 296.5 moveto 49.0562740300594 0 rlineto 0 -10 rlineto -49.0562740300594 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
55.907025515554 96.5 moveto 49.790283117791 0 rlineto 0 -10.0000000000001 rlineto -49.790283117791 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
55.907025515554 96.5 moveto 49.790283117791 0 rlineto 0 -10.0000000000001 rlineto -49.790283117791 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
57.3750436910171 271.5 moveto 49.0562740300594 0 rlineto 0 -10 rlineto -49.0562740300594 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
57.3750436910171 271.5 moveto 49.0562740300594 0 rlineto 0 -10 rlineto -49.0562740300594 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
57.3750436910171 171.5 moveto 49.3009437259699 0 rlineto 0 -10 rlineto -49.3009437259699 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
57.3750436910171 171.5 moveto 49.3009437259699 0 rlineto 0 -10 rlineto -49.3009437259699 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
56.8857042991961 221.5 moveto 49.790283117791 0 rlineto 0 -9.99999999999997 rlineto -49.790283117791 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
56.8857042991961 221.5 moveto 49.790283117791 0 rlineto 0 -9.99999999999997 rlineto -49.790283117791 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
56.1516952114645 196.5 moveto 50.5242922055225 0 rlineto 0 -9.99999999999997 rlineto -50.5242922055225 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
56.1516952114645 196.5 moveto 50.5242922055225 0 rlineto 0 -9.99999999999997 rlineto -50.5242922055225 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
57.3750436910171 146.5 moveto 49.5456134218805 0 rlineto 0 -10 rlineto -49.5456134218805 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
57.3750436910171 146.5 moveto 49.5456134218805 0 rlineto 0 -10 rlineto -49.5456134218805 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
56.1516952114645 371.5 moveto 50.7689619014331 0 rlineto 0 -10 rlineto -50.7689619014331 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
56.1516952114645 371.5 moveto 50.7689619014331 0 rlineto 0 -10 rlineto -50.7689619014331 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
57.8643830828382 71.5 moveto 49.3009437259699 0 rlineto 0 -10.0000000000001 rlineto -49.3009437259699 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
57.8643830828382 71.5 moveto 49.3009437259699 0 rlineto 0 -10.0000000000001 rlineto -49.3009437259699 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
58.2313876267039 321.5 moveto 49.790283117791 0 rlineto 0 -10 rlineto -49.790283117791 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
58.2313876267039 321.5 moveto 49.790283117791 0 rlineto 0 -10 rlineto -49.790283117791 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
55.6623558196435 396.5 moveto 53.093324012583 0 rlineto 0 -10 rlineto -53.093324012583 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
55.6623558196435 396.5 moveto 53.093324012583 0 rlineto 0 -10 rlineto -53.093324012583 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
59.5770709542118 46.5 moveto 50.4019573575673 0 rlineto 0 -10.0000000000001 rlineto -50.4019573575673 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
59.5770709542118 46.5 moveto 50.4019573575673 0 rlineto 0 -10.0000000000001 rlineto -50.4019573575673 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
58.720727018525 421.5 moveto 51.5029709891646 0 rlineto 0 -10 rlineto -51.5029709891646 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
58.720727018525 421.5 moveto 51.5029709891646 0 rlineto 0 -10 rlineto -51.5029709891646 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
59.210066410346 346.5 moveto 51.5029709891646 0 rlineto 0 -10 rlineto -51.5029709891646 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
59.210066410346 346.5 moveto 51.5029709891646 0 rlineto 0 -10 rlineto -51.5029709891646 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
59.4547361062566 121.5 moveto 51.3806361412094 0 rlineto 0 -10.0000000000001 rlineto -51.3806361412094 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
59.4547361062566 121.5 moveto 51.3806361412094 0 rlineto 0 -10.0000000000001 rlineto -51.3806361412094 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
121.111499475708 96.5 moveto 34.0090877315624 0 rlineto 0 -10.0000000000001 rlineto -34.0090877315624 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
121.111499475708 96.5 moveto 34.0090877315624 0 rlineto 0 -10.0000000000001 rlineto -34.0090877315624 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
122.579517651171 71.5 moveto 32.6634044040545 0 rlineto 0 -10.0000000000001 rlineto -32.6634044040545 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
122.579517651171 71.5 moveto 32.6634044040545 0 rlineto 0 -10.0000000000001 rlineto -32.6634044040545 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
120.255155540021 296.5 moveto 34.9877665152045 0 rlineto 0 -10 rlineto -34.9877665152045 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
120.255155540021 296.5 moveto 34.9877665152045 0 rlineto 0 -10 rlineto -34.9877665152045 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
121.967843411395 221.5 moveto 33.2750786438308 0 rlineto 0 -9.99999999999997 rlineto -33.2750786438308 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
121.967843411395 221.5 moveto 33.2750786438308 0 rlineto 0 -9.99999999999997 rlineto -33.2750786438308 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
122.33484795526 146.5 moveto 33.1527437958756 0 rlineto 0 -10 rlineto -33.1527437958756 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
122.33484795526 146.5 moveto 33.1527437958756 0 rlineto 0 -10 rlineto -33.1527437958756 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
121.845508563439 196.5 moveto 33.8867528836071 0 rlineto 0 -9.99999999999997 rlineto -33.8867528836071 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
121.845508563439 196.5 moveto 33.8867528836071 0 rlineto 0 -9.99999999999997 rlineto -33.8867528836071 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
124.169870674589 396.5 moveto 32.6634044040545 0 rlineto 0 -10 rlineto -32.6634044040545 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
124.169870674589 396.5 moveto 32.6634044040545 0 rlineto 0 -10 rlineto -32.6634044040545 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
123.068857042992 321.5 moveto 33.7644180356519 0 rlineto 0 -10 rlineto -33.7644180356519 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
123.068857042992 321.5 moveto 33.7644180356519 0 rlineto 0 -10 rlineto -33.7644180356519 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
122.09017825935 171.5 moveto 34.8654316672493 0 rlineto 0 -10 rlineto -34.8654316672493 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
122.09017825935 171.5 moveto 34.8654316672493 0 rlineto 0 -10 rlineto -34.8654316672493 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
121.723173715484 271.5 moveto 35.4771059070255 0 rlineto 0 -10 rlineto -35.4771059070255 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
121.723173715484 271.5 moveto 35.4771059070255 0 rlineto 0 -10 rlineto -35.4771059070255 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
125.637888850052 421.5 moveto 33.3974134917861 0 rlineto 0 -10 rlineto -33.3974134917861 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
125.637888850052 421.5 moveto 33.3974134917861 0 rlineto 0 -10 rlineto -33.3974134917861 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
126.004893393918 121.5 moveto 33.1527437958756 0 rlineto 0 -10.0000000000001 rlineto -33.1527437958756 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
126.004893393918 121.5 moveto 33.1527437958756 0 rlineto 0 -10.0000000000001 rlineto -33.1527437958756 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
126.249563089829 371.5 moveto 33.0304089479203 0 rlineto 0 -10 rlineto -33.0304089479203 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
126.249563089829 371.5 moveto 33.0304089479203 0 rlineto 0 -10 rlineto -33.0304089479203 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
125.882558545963 346.5 moveto 33.3974134917861 0 rlineto 0 -10 rlineto -33.3974134917861 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
125.882558545963 346.5 moveto 33.3974134917861 0 rlineto 0 -10 rlineto -33.3974134917861 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
125.270884306187 46.5 moveto 34.3760922754282 0 rlineto 0 -10.0000000000001 rlineto -34.3760922754282 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
125.270884306187 46.5 moveto 34.3760922754282 0 rlineto 0 -10.0000000000001 rlineto -34.3760922754282 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
126.371897937784 246.5 moveto 33.3974134917861 0 rlineto 0 -9.99999999999997 rlineto -33.3974134917861 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
126.371897937784 246.5 moveto 33.3974134917861 0 rlineto 0 -9.99999999999997 rlineto -33.3974134917861 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
209.192590003495 246.5 moveto 40.3704998252359 0 rlineto 0 -9.99999999999997 rlineto -40.3704998252359 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
209.192590003495 246.5 moveto 40.3704998252359 0 rlineto 0 -9.99999999999997 rlineto -40.3704998252359 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
204.666200629151 146.5 moveto 50.6466270534778 0 rlineto 0 -10 rlineto -50.6466270534778 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
204.666200629151 146.5 moveto 50.6466270534778 0 rlineto 0 -10 rlineto -50.6466270534778 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
206.011883956659 296.5 moveto 49.3009437259699 0 rlineto 0 -10 rlineto -49.3009437259699 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
206.011883956659 296.5 moveto 49.3009437259699 0 rlineto 0 -10 rlineto -49.3009437259699 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
206.011883956659 396.5 moveto 49.5456134218804 0 rlineto 0 -10 rlineto -49.5456134218804 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
206.011883956659 396.5 moveto 49.5456134218804 0 rlineto 0 -10 rlineto -49.5456134218804 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
206.011883956659 96.5 moveto 50.0349528137015 0 rlineto 0 -10.0000000000001 rlineto -50.0349528137015 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
206.011883956659 96.5 moveto 50.0349528137015 0 rlineto 0 -10.0000000000001 rlineto -50.0349528137015 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
206.256553652569 196.5 moveto 49.790283117791 0 rlineto 0 -9.99999999999997 rlineto -49.790283117791 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
206.256553652569 196.5 moveto 49.790283117791 0 rlineto 0 -9.99999999999997 rlineto -49.790283117791 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
207.479902132122 71.5 moveto 48.8116043341489 0 rlineto 0 -10.0000000000001 rlineto -48.8116043341489 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
207.479902132122 71.5 moveto 48.8116043341489 0 rlineto 0 -10.0000000000001 rlineto -48.8116043341489 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
207.969241523943 171.5 moveto 48.5669346382384 0 rlineto 0 -10 rlineto -48.5669346382384 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
207.969241523943 171.5 moveto 48.5669346382384 0 rlineto 0 -10 rlineto -48.5669346382384 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
205.277874868927 221.5 moveto 51.7476406850751 0 rlineto 0 -9.99999999999997 rlineto -51.7476406850751 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
205.277874868927 221.5 moveto 51.7476406850751 0 rlineto 0 -9.99999999999997 rlineto -51.7476406850751 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
208.580915763719 271.5 moveto 49.5456134218805 0 rlineto 0 -10 rlineto -49.5456134218805 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
208.580915763719 271.5 moveto 49.5456134218805 0 rlineto 0 -10 rlineto -49.5456134218805 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
206.868227892345 321.5 moveto 51.6253058371199 0 rlineto 0 -10 rlineto -51.6253058371199 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
206.868227892345 321.5 moveto 51.6253058371199 0 rlineto 0 -10 rlineto -51.6253058371199 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
208.580915763719 371.5 moveto 50.279622509612 0 rlineto 0 -10 rlineto -50.279622509612 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
208.580915763719 371.5 moveto 50.279622509612 0 rlineto 0 -10 rlineto -50.279622509612 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
210.048933939182 46.5 moveto 49.4232785739252 0 rlineto 0 -10.0000000000001 rlineto -49.4232785739252 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
210.048933939182 46.5 moveto 49.4232785739252 0 rlineto 0 -10.0000000000001 rlineto -49.4232785739252 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
211.39461726669 421.5 moveto 49.3009437259699 0 rlineto 0 -10 rlineto -49.3009437259699 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
211.39461726669 421.5 moveto 49.3009437259699 0 rlineto 0 -10 rlineto -49.3009437259699 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
211.761621810556 121.5 moveto 48.9339391821042 0 rlineto 0 -10.0000000000001 rlineto -48.9339391821042 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
211.761621810556 121.5 moveto 48.9339391821042 0 rlineto 0 -10.0000000000001 rlineto -48.9339391821042 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
212.128626354422 346.5 moveto 51.6253058371199 0 rlineto 0 -10 rlineto -51.6253058371199 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
212.128626354422 346.5 moveto 51.6253058371199 0 rlineto 0 -10 rlineto -51.6253058371199 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
269.136665501573 146.5 moveto 31.8070604683678 0 rlineto 0 -10 rlineto -31.8070604683678 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
269.136665501573 146.5 moveto 31.8070604683678 0 rlineto 0 -10 rlineto -31.8070604683678 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
269.136665501573 296.5 moveto 32.0517301642782 0 rlineto 0 -10 rlineto -32.0517301642782 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
269.136665501573 296.5 moveto 32.0517301642782 0 rlineto 0 -10 rlineto -32.0517301642782 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
269.870674589304 196.5 moveto 31.6847256204125 0 rlineto 0 -9.99999999999997 rlineto -31.6847256204125 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
269.870674589304 196.5 moveto 31.6847256204125 0 rlineto 0 -9.99999999999997 rlineto -31.6847256204125 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
270.360013981126 171.5 moveto 31.5623907724572 0 rlineto 0 -10 rlineto -31.5623907724572 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
270.360013981126 171.5 moveto 31.5623907724572 0 rlineto 0 -10 rlineto -31.5623907724572 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
269.870674589304 96.5 moveto 32.0517301642783 0 rlineto 0 -10.0000000000001 rlineto -32.0517301642783 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
269.870674589304 96.5 moveto 32.0517301642783 0 rlineto 0 -10.0000000000001 rlineto -32.0517301642783 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
270.971688220902 71.5 moveto 32.0517301642782 0 rlineto 0 -10.0000000000001 rlineto -32.0517301642782 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
270.971688220902 71.5 moveto 32.0517301642782 0 rlineto 0 -10.0000000000001 rlineto -32.0517301642782 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
270.971688220902 221.5 moveto 32.0517301642782 0 rlineto 0 -9.99999999999997 rlineto -32.0517301642782 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
270.971688220902 221.5 moveto 32.0517301642782 0 rlineto 0 -9.99999999999997 rlineto -32.0517301642782 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
271.950367004544 271.5 moveto 32.0517301642782 0 rlineto 0 -10 rlineto -32.0517301642782 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
271.950367004544 271.5 moveto 32.0517301642782 0 rlineto 0 -10 rlineto -32.0517301642782 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
272.439706396365 321.5 moveto 32.9080740999651 0 rlineto 0 -10 rlineto -32.9080740999651 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
272.439706396365 321.5 moveto 32.9080740999651 0 rlineto 0 -10 rlineto -32.9080740999651 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
273.540720027962 46.5 moveto 32.0517301642782 0 rlineto 0 -10.0000000000001 rlineto -32.0517301642782 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
273.540720027962 46.5 moveto 32.0517301642782 0 rlineto 0 -10.0000000000001 rlineto -32.0517301642782 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
272.929045788186 371.5 moveto 32.6634044040545 0 rlineto 0 -10 rlineto -32.6634044040545 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
272.929045788186 371.5 moveto 32.6634044040545 0 rlineto 0 -10 rlineto -32.6634044040545 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
269.626004893394 396.5 moveto 36.2111149947571 0 rlineto 0 -10 rlineto -36.2111149947571 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
269.626004893394 396.5 moveto 36.2111149947571 0 rlineto 0 -10 rlineto -36.2111149947571 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
275.008738203425 421.5 moveto 32.0517301642783 0 rlineto 0 -10 rlineto -32.0517301642783 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
275.008738203425 421.5 moveto 32.0517301642783 0 rlineto 0 -10 rlineto -32.0517301642783 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
274.764068507515 246.5 moveto 32.2963998601888 0 rlineto 0 -9.99999999999997 rlineto -32.2963998601888 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
274.764068507515 246.5 moveto 32.2963998601888 0 rlineto 0 -9.99999999999997 rlineto -32.2963998601888 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
275.253407899336 121.5 moveto 31.9293953163229 0 rlineto 0 -10.0000000000001 rlineto -31.9293953163229 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
275.253407899336 121.5 moveto 31.9293953163229 0 rlineto 0 -10.0000000000001 rlineto -31.9293953163229 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
277.577770010486 346.5 moveto 32.0517301642783 0 rlineto 0 -10 rlineto -32.0517301642783 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
277.577770010486 346.5 moveto 32.0517301642783 0 rlineto 0 -10 rlineto -32.0517301642783 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
348.164977280671 296.5 moveto 48.8116043341489 0 rlineto 0 -10 rlineto -48.8116043341489 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
348.164977280671 296.5 moveto 48.8116043341489 0 rlineto 0 -10 rlineto -48.8116043341489 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
349.877665152045 171.5 moveto 49.0562740300593 0 rlineto 0 -10 rlineto -49.0562740300593 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
349.877665152045 171.5 moveto 49.0562740300593 0 rlineto 0 -10 rlineto -49.0562740300593 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
349.75533030409 146.5 moveto 49.1786088780146 0 rlineto 0 -10 rlineto -49.1786088780146 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
349.75533030409 146.5 moveto 49.1786088780146 0 rlineto 0 -10 rlineto -49.1786088780146 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
350 96.5 moveto 49.4232785739252 0 rlineto 0 -10.0000000000001 rlineto -49.4232785739252 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
350 96.5 moveto 49.4232785739252 0 rlineto 0 -10.0000000000001 rlineto -49.4232785739252 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
351.468018175463 71.5 moveto 48.8116043341489 0 rlineto 0 -10.0000000000001 rlineto -48.8116043341489 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
351.468018175463 71.5 moveto 48.8116043341489 0 rlineto 0 -10.0000000000001 rlineto -48.8116043341489 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
351.223348479553 221.5 moveto 49.3009437259699 0 rlineto 0 -9.99999999999997 rlineto -49.3009437259699 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
351.223348479553 221.5 moveto 49.3009437259699 0 rlineto 0 -9.99999999999997 rlineto -49.3009437259699 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
351.957357567284 196.5 moveto 48.8116043341489 0 rlineto 0 -9.99999999999997 rlineto -48.8116043341489 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
351.957357567284 196.5 moveto 48.8116043341489 0 rlineto 0 -9.99999999999997 rlineto -48.8116043341489 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
353.058371198882 321.5 moveto 49.7902831177909 0 rlineto 0 -10 rlineto -49.7902831177909 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
353.058371198882 321.5 moveto 49.7902831177909 0 rlineto 0 -10 rlineto -49.7902831177909 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
354.281719678434 271.5 moveto 48.9339391821042 0 rlineto 0 -10 rlineto -48.9339391821042 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
354.281719678434 271.5 moveto 48.9339391821042 0 rlineto 0 -10 rlineto -48.9339391821042 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
355.138063614121 121.5 moveto 48.4445997902831 0 rlineto 0 -10.0000000000001 rlineto -48.4445997902831 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
355.138063614121 121.5 moveto 48.4445997902831 0 rlineto 0 -10.0000000000001 rlineto -48.4445997902831 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
355.138063614121 421.5 moveto 49.4232785739252 0 rlineto 0 -10 rlineto -49.4232785739252 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
355.138063614121 421.5 moveto 49.4232785739252 0 rlineto 0 -10 rlineto -49.4232785739252 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
355.872072701853 396.5 moveto 49.3009437259698 0 rlineto 0 -10 rlineto -49.3009437259698 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
355.872072701853 396.5 moveto 49.3009437259698 0 rlineto 0 -10 rlineto -49.3009437259698 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
355.382733310031 246.5 moveto 50.0349528137016 0 rlineto 0 -9.99999999999997 rlineto -50.0349528137016 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
355.382733310031 246.5 moveto 50.0349528137016 0 rlineto 0 -9.99999999999997 rlineto -50.0349528137016 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
355.627403005942 346.5 moveto 49.9126179657462 0 rlineto 0 -10 rlineto -49.9126179657462 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
355.627403005942 346.5 moveto 49.9126179657462 0 rlineto 0 -10 rlineto -49.9126179657462 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
352.32436211115 371.5 moveto 53.582663404404 0 rlineto 0 -10 rlineto -53.582663404404 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
352.32436211115 371.5 moveto 53.582663404404 0 rlineto 0 -10 rlineto -53.582663404404 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
356.116742397763 46.5 moveto 50.0349528137016 0 rlineto 0 -10.0000000000001 rlineto -50.0349528137016 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
356.116742397763 46.5 moveto 50.0349528137016 0 rlineto 0 -10.0000000000001 rlineto -50.0349528137016 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
411.534428521496 296.5 moveto 32.2963998601887 0 rlineto 0 -10 rlineto -32.2963998601887 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
411.534428521496 296.5 moveto 32.2963998601887 0 rlineto 0 -10 rlineto -32.2963998601887 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
413.614120936735 171.5 moveto 31.8070604683677 0 rlineto 0 -10 rlineto -31.8070604683677 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
413.614120936735 171.5 moveto 31.8070604683677 0 rlineto 0 -10 rlineto -31.8070604683677 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
413.124781544914 146.5 moveto 32.2963998601887 0 rlineto 0 -10 rlineto -32.2963998601887 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
413.124781544914 146.5 moveto 32.2963998601887 0 rlineto 0 -10 rlineto -32.2963998601887 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
414.348130024467 221.5 moveto 32.0517301642783 0 rlineto 0 -9.99999999999997 rlineto -32.0517301642783 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
414.348130024467 221.5 moveto 32.0517301642783 0 rlineto 0 -9.99999999999997 rlineto -32.0517301642783 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
414.348130024467 71.5 moveto 32.2963998601888 0 rlineto 0 -10.0000000000001 rlineto -32.2963998601888 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
414.348130024467 71.5 moveto 32.2963998601888 0 rlineto 0 -10.0000000000001 rlineto -32.2963998601888 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
414.592799720377 196.5 moveto 32.1740650122336 0 rlineto 0 -9.99999999999997 rlineto -32.1740650122336 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
414.592799720377 196.5 moveto 32.1740650122336 0 rlineto 0 -9.99999999999997 rlineto -32.1740650122336 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
416.550157287662 321.5 moveto 32.418734708144 0 rlineto 0 -10 rlineto -32.418734708144 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
416.550157287662 321.5 moveto 32.418734708144 0 rlineto 0 -10 rlineto -32.418734708144 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
417.161831527438 271.5 moveto 32.0517301642782 0 rlineto 0 -10 rlineto -32.0517301642782 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
417.161831527438 271.5 moveto 32.0517301642782 0 rlineto 0 -10 rlineto -32.0517301642782 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
417.406501223348 121.5 moveto 32.4187347081441 0 rlineto 0 -10.0000000000001 rlineto -32.4187347081441 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
417.406501223348 121.5 moveto 32.4187347081441 0 rlineto 0 -10.0000000000001 rlineto -32.4187347081441 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
413.614120936735 96.5 moveto 36.9451240824886 0 rlineto 0 -10.0000000000001 rlineto -36.9451240824886 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
413.614120936735 96.5 moveto 36.9451240824886 0 rlineto 0 -10.0000000000001 rlineto -36.9451240824886 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
418.507514854946 421.5 moveto 32.2963998601888 0 rlineto 0 -10 rlineto -32.2963998601888 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
418.507514854946 421.5 moveto 32.2963998601888 0 rlineto 0 -10 rlineto -32.2963998601888 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
419.975533030409 46.5 moveto 31.5623907724572 0 rlineto 0 -10.0000000000001 rlineto -31.5623907724572 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
419.975533030409 46.5 moveto 31.5623907724572 0 rlineto 0 -10.0000000000001 rlineto -31.5623907724572 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
419.241523942677 346.5 moveto 32.2963998601888 0 rlineto 0 -10 rlineto -32.2963998601888 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
419.241523942677 346.5 moveto 32.2963998601888 0 rlineto 0 -10 rlineto -32.2963998601888 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
419.241523942677 246.5 moveto 32.5410695560993 0 rlineto 0 -9.99999999999997 rlineto -32.5410695560993 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
419.241523942677 246.5 moveto 32.5410695560993 0 rlineto 0 -9.99999999999997 rlineto -32.5410695560993 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
419.363858790633 396.5 moveto 32.5410695560992 0 rlineto 0 -10 rlineto -32.5410695560992 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
419.363858790633 396.5 moveto 32.5410695560992 0 rlineto 0 -10 rlineto -32.5410695560992 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
419.975533030409 371.5 moveto 39.514155889549 0 rlineto 0 -10 rlineto -39.514155889549 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
419.975533030409 371.5 moveto 39.514155889549 0 rlineto 0 -10 rlineto -39.514155889549 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
493.131772107655 71.5 moveto 38.0461377140861 0 rlineto 0 -10.0000000000001 rlineto -38.0461377140861 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
493.131772107655 71.5 moveto 38.0461377140861 0 rlineto 0 -10.0000000000001 rlineto -38.0461377140861 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
493.865781195386 171.5 moveto 37.5567983222649 0 rlineto 0 -10 rlineto -37.5567983222649 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
493.865781195386 171.5 moveto 37.5567983222649 0 rlineto 0 -10 rlineto -37.5567983222649 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
499.737853897239 346.5 moveto 39.2694861936385 0 rlineto 0 -10 rlineto -39.2694861936385 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
499.737853897239 346.5 moveto 39.2694861936385 0 rlineto 0 -10 rlineto -39.2694861936385 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
499.248514505418 421.5 moveto 39.8811604334148 0 rlineto 0 -10 rlineto -39.8811604334148 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
499.248514505418 421.5 moveto 39.8811604334148 0 rlineto 0 -10 rlineto -39.8811604334148 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
492.153093324013 296.5 moveto 49.1786088780147 0 rlineto 0 -10 rlineto -49.1786088780147 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
492.153093324013 296.5 moveto 49.1786088780147 0 rlineto 0 -10 rlineto -49.1786088780147 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
494.477455435163 221.5 moveto 48.8116043341489 0 rlineto 0 -9.99999999999997 rlineto -48.8116043341489 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
494.477455435163 221.5 moveto 48.8116043341489 0 rlineto 0 -9.99999999999997 rlineto -48.8116043341489 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
493.131772107655 196.5 moveto 50.279622509612 0 rlineto 0 -9.99999999999997 rlineto -50.279622509612 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
493.131772107655 196.5 moveto 50.279622509612 0 rlineto 0 -9.99999999999997 rlineto -50.279622509612 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
496.434813002447 271.5 moveto 49.3009437259699 0 rlineto 0 -10 rlineto -49.3009437259699 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
496.434813002447 271.5 moveto 49.3009437259699 0 rlineto 0 -10 rlineto -49.3009437259699 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
496.679482698357 146.5 moveto 49.790283117791 0 rlineto 0 -10 rlineto -49.790283117791 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
496.679482698357 146.5 moveto 49.790283117791 0 rlineto 0 -10 rlineto -49.790283117791 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
497.413491786089 121.5 moveto 49.4232785739253 0 rlineto 0 -10.0000000000001 rlineto -49.4232785739253 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
497.413491786089 121.5 moveto 49.4232785739253 0 rlineto 0 -10.0000000000001 rlineto -49.4232785739253 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
496.067808458581 321.5 moveto 51.5029709891646 0 rlineto 0 -10 rlineto -51.5029709891646 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
496.067808458581 321.5 moveto 51.5029709891646 0 rlineto 0 -10 rlineto -51.5029709891646 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
498.14750087382 46.5 moveto 49.790283117791 0 rlineto 0 -10.0000000000001 rlineto -49.790283117791 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
498.14750087382 46.5 moveto 49.790283117791 0 rlineto 0 -10.0000000000001 rlineto -49.790283117791 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
500.594197832926 96.5 moveto 48.9339391821041 0 rlineto 0 -10.0000000000001 rlineto -48.9339391821041 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
500.594197832926 96.5 moveto 48.9339391821041 0 rlineto 0 -10.0000000000001 rlineto -48.9339391821041 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
501.817546312478 246.5 moveto 49.5456134218805 0 rlineto 0 -9.99999999999997 rlineto -49.5456134218805 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
501.817546312478 246.5 moveto 49.5456134218805 0 rlineto 0 -9.99999999999997 rlineto -49.5456134218805 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
502.673890248165 396.5 moveto 49.0562740300593 0 rlineto 0 -10 rlineto -49.0562740300593 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
502.673890248165 396.5 moveto 49.0562740300593 0 rlineto 0 -10 rlineto -49.0562740300593 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
520.290108353722 371.5 moveto 51.8699755330305 0 rlineto 0 -10 rlineto -51.8699755330305 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
520.290108353722 371.5 moveto 51.8699755330305 0 rlineto 0 -10 rlineto -51.8699755330305 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
555.033205173016 296.5 moveto 32.0517301642783 0 rlineto 0 -10 rlineto -32.0517301642783 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
555.033205173016 296.5 moveto 32.0517301642783 0 rlineto 0 -10 rlineto -32.0517301642783 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
555.522544564838 71.5 moveto 31.8070604683678 0 rlineto 0 -10.0000000000001 rlineto -31.8070604683678 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
555.522544564838 71.5 moveto 31.8070604683678 0 rlineto 0 -10.0000000000001 rlineto -31.8070604683678 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
556.256553652569 171.5 moveto 32.0517301642782 0 rlineto 0 -10 rlineto -32.0517301642782 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
556.256553652569 171.5 moveto 32.0517301642782 0 rlineto 0 -10 rlineto -32.0517301642782 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
556.868227892345 196.5 moveto 31.9293953163229 0 rlineto 0 -9.99999999999997 rlineto -31.9293953163229 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
556.868227892345 196.5 moveto 31.9293953163229 0 rlineto 0 -9.99999999999997 rlineto -31.9293953163229 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
557.357567284166 221.5 moveto 32.2963998601887 0 rlineto 0 -9.99999999999997 rlineto -32.2963998601887 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
557.357567284166 221.5 moveto 32.2963998601887 0 rlineto 0 -9.99999999999997 rlineto -32.2963998601887 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
559.804264243272 271.5 moveto 32.1740650122334 0 rlineto 0 -10 rlineto -32.1740650122334 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
559.804264243272 271.5 moveto 32.1740650122334 0 rlineto 0 -10 rlineto -32.1740650122334 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
560.415938483048 146.5 moveto 31.8070604683678 0 rlineto 0 -10 rlineto -31.8070604683678 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
560.415938483048 146.5 moveto 31.8070604683678 0 rlineto 0 -10 rlineto -31.8070604683678 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
560.905277874869 121.5 moveto 31.8070604683678 0 rlineto 0 -10.0000000000001 rlineto -31.8070604683678 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
560.905277874869 121.5 moveto 31.8070604683678 0 rlineto 0 -10.0000000000001 rlineto -31.8070604683678 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
561.149947570779 321.5 moveto 32.0517301642782 0 rlineto 0 -10 rlineto -32.0517301642782 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
561.149947570779 321.5 moveto 32.0517301642782 0 rlineto 0 -10 rlineto -32.0517301642782 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
561.883956658511 46.5 moveto 31.5623907724572 0 rlineto 0 -10.0000000000001 rlineto -31.5623907724572 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
561.883956658511 46.5 moveto 31.5623907724572 0 rlineto 0 -10.0000000000001 rlineto -31.5623907724572 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
563.474309681929 96.5 moveto 32.0517301642782 0 rlineto 0 -10.0000000000001 rlineto -32.0517301642782 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
563.474309681929 96.5 moveto 32.0517301642782 0 rlineto 0 -10.0000000000001 rlineto -32.0517301642782 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
565.798671793079 396.5 moveto 32.0517301642783 0 rlineto 0 -10 rlineto -32.0517301642783 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
565.798671793079 396.5 moveto 32.0517301642783 0 rlineto 0 -10 rlineto -32.0517301642783 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
565.431667249214 246.5 moveto 32.4187347081441 0 rlineto 0 -9.99999999999997 rlineto -32.4187347081441 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
565.431667249214 246.5 moveto 32.4187347081441 0 rlineto 0 -9.99999999999997 rlineto -32.4187347081441 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
563.474309681929 421.5 moveto 36.2111149947572 0 rlineto 0 -10 rlineto -36.2111149947572 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
563.474309681929 421.5 moveto 36.2111149947572 0 rlineto 0 -10 rlineto -36.2111149947572 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
565.309332401258 346.5 moveto 34.8654316672491 0 rlineto 0 -10 rlineto -34.8654316672491 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
565.309332401258 346.5 moveto 34.8654316672491 0 rlineto 0 -10 rlineto -34.8654316672491 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
585.983921705697 371.5 moveto 32.2963998601887 0 rlineto 0 -10 rlineto -32.2963998601887 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
585.983921705697 371.5 moveto 32.2963998601887 0 rlineto 0 -10 rlineto -32.2963998601887 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
104.963299545613 291.5 moveto
153.939070192871 96.2112476351153 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
155.12058720727 91.5 moveto
156.082921797047 102.051014744437 lineto
153.59003955691 99.6584609525613 lineto
152.620076808504 99.4152074495968 lineto
149.293182558204 100.348240223685 lineto
155.12058720727 91.5 lineto
fill
grestore
gsave
106.431317721077 266.5 moveto
154.091299131053 71.2186440187 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
155.242922055225 66.4999999999999 moveto
156.272133048346 77.0447012632715 lineto
153.764131598268 74.6680009799767 lineto
152.792646065006 74.4309021426471 lineto
149.471734315513 75.3850094019642 lineto
155.242922055225 66.4999999999999 lineto
fill
grestore
gsave
105.697308633345 91.5 moveto
154.074975818155 286.785369757658 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
155.242922055225 291.5 moveto
149.441019774862 282.635016642479 lineto
152.765213596452 283.577625228644 lineto
153.735872763993 283.337165709247 lineto
156.235633947649 280.951800006701 lineto
155.242922055225 291.5 lineto
fill
grestore
gsave
110.22369800769 416.5 moveto
154.176286443069 221.238578395001 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
155.242922055225 216.5 moveto
156.461471403065 227.024501769057 lineto
153.911162022208 224.693257986842 lineto
152.935572352649 224.47365653728 lineto
149.632343716152 225.487291622125 lineto
155.242922055225 216.5 lineto
fill
grestore
gsave
110.713037399511 341.5 moveto
154.426475681854 146.239817446084 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
155.487591751136 141.5 moveto
156.718397709351 152.023075438921 lineto
154.165374958281 149.694803473982 lineto
153.189530189969 149.476338400894 lineto
149.88748433117 150.493819927308 lineto
155.487591751136 141.5 lineto
fill
grestore
gsave
108.755679832227 391.5 moveto
154.621627154796 196.228459390073 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
155.732261447046 191.5 moveto
156.852933641142 202.035373454861 lineto
154.324403179832 199.680525430798 lineto
153.350896834817 199.451865429452 lineto
150.038389226037 200.434753445441 lineto
155.732261447046 191.5 lineto
fill
grestore
gsave
106.675987416987 191.5 moveto
155.651758064245 386.788752364885 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
156.833275078644 391.5 moveto
151.005870429578 382.651759776315 lineto
154.332764679878 383.584792550403 lineto
155.302727428284 383.341539047439 lineto
157.79560966842 380.948985255563 lineto
156.833275078644 391.5 lineto
fill
grestore
gsave
110.835372247466 116.5 moveto
155.744604854708 311.766435394102 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
156.833275078644 316.5 moveto
151.18094423982 307.53890876687 lineto
154.488853046028 308.537164665932 lineto
155.46341046489 308.313026678651 lineto
158.002846171849 305.969942855903 lineto
156.833275078644 316.5 lineto
fill
grestore
gsave
106.920657112898 366.5 moveto
155.776804728008 171.211926892309 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
156.955609926599 166.5 moveto
157.92401713131 177.050459112563 lineto
155.429758238504 174.659340527912 lineto
154.459655643029 174.416645339967 lineto
151.133298962982 175.351592796946 lineto
156.955609926599 166.5 lineto
fill
grestore
gsave
107.165326808808 66.4999999999999 moveto
156.021474423918 261.788073107691 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
157.20027962251 266.5 moveto
151.377968658892 257.648407203054 lineto
154.704325338939 258.583354660033 lineto
155.674427934415 258.340659472088 lineto
158.16868682722 255.949540887437 lineto
157.20027962251 266.5 lineto
fill
grestore
gsave
106.675987416987 216.5 moveto
157.805175305627 411.801210346326 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
159.035302341839 416.5 moveto
153.116795310961 407.712436371471 lineto
156.453151403952 408.611048373931 lineto
157.420549273826 408.357786925299 lineto
159.888580400079 405.939606231048 lineto
159.035302341839 416.5 lineto
fill
grestore
gsave
108.021670744495 316.5 moveto
157.954468076866 121.205764637165 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
159.157637189794 116.5 moveto
160.071442945841 127.055328466714 lineto
157.58958918039 124.651336495612 lineto
156.620755284503 124.403625207656 lineto
153.289605674633 125.321349451024 lineto
159.157637189794 116.5 lineto
fill
grestore
gsave
106.675987416987 166.5 moveto
158.044467866544 361.802621350409 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
159.279972037749 366.5 moveto
153.35141118777 357.719216080093 lineto
156.688793590589 358.614008909499 lineto
157.655900959622 358.359640403663 lineto
160.121162771003 355.938636539239 lineto
159.279972037749 366.5 lineto
fill
grestore
gsave
106.920657112898 141.5 moveto
158.049845001537 336.801210346326 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
159.279972037749 341.5 moveto
153.361465006871 332.712436371471 lineto
156.697821099863 333.611048373931 lineto
157.665218969737 333.357786925299 lineto
160.13325009599 330.939606231048 lineto
159.279972037749 341.5 lineto
fill
grestore
gsave
100.192240475358 241.5 moveto
158.26293630506 46.1557780496461 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
159.646976581615 41.4999999999999 moveto
160.152380783305 52.0827485367892 lineto
157.765237967896 49.5846843484535 lineto
156.806695428263 49.299734879751 lineto
153.442583005874 50.0881022558715 lineto
159.646976581615 41.4999999999999 lineto
fill
grestore
gsave
109.979028311779 41.4999999999999 moveto
158.595933376137 236.786718693053 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
159.76931142957 241.5 moveto
153.957198024848 232.641707965378 lineto
157.282475792116 233.580485511297 lineto
158.252857237664 233.338907676767 lineto
160.749868143684 230.950663123665 lineto
159.76931142957 241.5 lineto
fill
grestore
gsave
259.472212513107 41.4999999999999 moveto
299.083057234972 137.013381162456 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
300.943725969941 141.5 moveto
293.879932647363 133.603619569961 lineto
297.307786188776 134.03789552926 lineto
298.2315018318 133.654816672061 lineto
300.345942148529 130.922067569564 lineto
300.943725969941 141.5 lineto
fill
grestore
gsave
255.557497378539 391.5 moveto
299.172039420387 295.91883969228 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
301.188395665851 291.5 moveto
300.221237879922 302.050573719809 lineto
298.20360968603 299.245586735629 lineto
297.293848572913 298.830454567445 lineto
293.852910088106 299.144648542522 lineto
301.188395665851 291.5 lineto
fill
grestore
gsave
256.04683677036 91.5 moveto
299.543521241739 187.079120025051 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
301.555400209717 191.5 moveto
294.227662352521 183.84792460203 lineto
297.668280796334 184.165604054144 lineto
298.578461967647 183.751393678384 lineto
300.598930551712 180.94845197171 lineto
301.555400209717 191.5 lineto
fill
grestore
gsave
256.291506466271 66.4999999999999 moveto
299.906048508119 162.08116030772 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
301.922404753583 166.5 moveto
294.586919175837 158.855351457478 lineto
298.027857660645 159.169545432555 lineto
298.937618773762 158.754413264371 lineto
300.955246967653 155.949426280191 lineto
301.922404753583 166.5 lineto
fill
grestore
gsave
256.04683677036 191.5 moveto
299.897112559964 95.9147512121477 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
301.922404753583 91.5 moveto
300.93390331665 102.048595399824 lineto
298.921954224542 99.2395321465363 lineto
298.013034857335 98.8225602243206 lineto
294.571467746202 99.129791944314 lineto
301.922404753583 91.5 lineto
fill
grestore
gsave
256.536176162181 166.5 moveto
300.975894961872 70.9044845970555 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
303.02341838518 66.4999999999999 moveto
301.981748767424 77.0434778136744 lineto
299.983987136293 74.2243070179645 lineto
299.077181483958 73.8027580778718 lineto
295.63410920108 74.0926352330248 lineto
303.02341838518 66.4999999999999 lineto
fill
grestore
gsave
260.69556099266 116.5 moveto
301.130116278176 212.027054229447 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
303.02341838518 216.5 moveto
295.902291359626 208.655285225968 lineto
299.333217432145 209.06457949066 lineto
300.254118031965 208.674781998042 lineto
302.348595558363 205.926702777638 lineto
303.02341838518 216.5 lineto
fill
grestore
gsave
258.860538273331 366.5 moveto
302.003687218719 270.926984798486 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
304.002097168822 266.5 moveto
303.077756905754 277.054411166809 lineto
301.048763924373 274.25763391581 lineto
300.137325877626 273.846196573141 lineto
296.697690578525 274.174349768132 lineto
304.002097168822 266.5 lineto
fill
grestore
gsave
260.69556099266 416.5 moveto
303.36741590842 320.935086564392 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
305.34778049633 316.5 moveto
304.466430486739 327.058087997388 lineto
302.426064522112 324.269596964482 lineto
301.512958464737 323.861874843442 lineto
298.074688085116 324.204033150106 lineto
305.34778049633 316.5 lineto
fill
grestore
gsave
255.312827682628 141.5 moveto
303.410567927615 45.8394961133764 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
305.59245019224 41.4999999999999 moveto
304.227329493828 52.0064953946958 lineto
302.317128576022 49.1272753677064 lineto
301.423702905621 48.6780643132248 lineto
297.973349801021 48.8620180133245 lineto
305.59245019224 41.4999999999999 lineto
fill
grestore
gsave
258.126529185599 266.5 moveto
303.509680111356 362.112072753011 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
305.59245019224 366.5 moveto
298.142505391854 358.966851755661 lineto
301.587791072953 359.229115145816 lineto
302.491187859098 358.800309540928 lineto
304.466282894867 355.965212521445 lineto
305.59245019224 366.5 lineto
fill
grestore
gsave
255.312827682628 291.5 moveto
303.646775171953 387.164769122766 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
305.837119888151 391.5 moveto
298.203670281502 384.15286129825 lineto
301.654375723158 384.330082812563 lineto
302.546923256706 383.87912948864 lineto
304.451503016339 380.996188030789 lineto
305.837119888151 391.5 lineto
fill
grestore
gsave
258.493533729465 316.5 moveto
304.938522273065 412.130883212119 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
307.060468367704 416.5 moveto
299.543421663946 409.033808946088 lineto
302.990915948686 409.265236401004 lineto
303.890439993249 408.828365146226 lineto
305.840089975892 405.975710162638 lineto
307.060468367704 416.5 lineto
fill
grestore
gsave
263.753932191541 341.5 moveto
305.130237854142 245.957134382001 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
307.060468367704 241.5 moveto
306.29822296799 252.067354538891 lineto
304.226544854482 249.302047086867 lineto
303.30889954054 248.904646687017 lineto
299.874705770401 249.285551739935 lineto
307.060468367704 241.5 lineto
fill
grestore
gsave
257.025515554002 216.5 moveto
305.00516153893 120.841625670468 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
307.182803215659 116.5 moveto
305.827947673172 127.007824059195 lineto
303.914934762611 124.130471610462 lineto
303.021070653985 123.682133618194 lineto
299.570898912791 123.869458113321 lineto
307.182803215659 116.5 lineto
fill
grestore
gsave
249.563089828731 241.5 moveto
307.128487884899 337.336254779572 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
309.629500174764 341.5 moveto
301.480011404439 334.729783402111 lineto
304.934446613479 334.654597653815 lineto
305.791688276508 334.139683358843 lineto
307.480703045643 331.125383337304 lineto
309.629500174764 341.5 lineto
fill
grestore
gsave
405.417686123733 241.5 moveto
440.871718878649 287.64831106088 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
443.830828381685 291.5 moveto
434.963062375187 285.702351679 lineto
438.386438309244 285.234086023285 lineto
439.179433090827 284.624857596189 lineto
440.514025846271 281.437752689331 lineto
443.830828381685 291.5 lineto
fill
grestore
gsave
400.524292205522 216.5 moveto
442.176043834283 170.113989330524 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
445.421181405103 166.5 moveto
441.344214012174 176.278974224171 lineto
440.257386803611 172.999098902008 lineto
439.513330176738 172.33098234331 lineto
436.135817624066 171.602158313283 lineto
445.421181405103 166.5 lineto
fill
grestore
gsave
400.768961901433 191.5 moveto
442.18587023252 145.12278875333 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
445.421181405103 141.5 moveto
441.370785592627 151.290009895923 lineto
440.275055305893 148.013098141094 lineto
439.529187033149 147.347004664386 lineto
436.149707683416 146.627355558966 lineto
445.421181405103 141.5 lineto
fill
grestore
gsave
398.933939182104 166.5 moveto
443.055767871611 212.97737550835 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
446.399860188745 216.5 moveto
436.976602475661 211.657251392773 lineto
440.332609008904 210.835061841007 lineto
441.057855227774 210.146572246303 lineto
442.053326007745 206.837824229843 lineto
446.399860188745 216.5 lineto
fill
grestore
gsave
403.582663404404 116.5 moveto
443.474865067693 70.1803616239169 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
446.644529884656 66.4999999999999 moveto
442.770774666966 76.3612382849935 lineto
441.616315364063 73.1045529719574 lineto
440.858593853257 72.4519749214061 lineto
437.466724091321 71.793191931135 lineto
446.644529884656 66.4999999999999 lineto
fill
grestore
gsave
398.933939182104 141.5 moveto
443.409246924959 187.990265053821 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
446.766864732611 191.5 moveto
437.325048476228 186.693535001616 lineto
440.677867522157 185.85844221907 lineto
441.400460011076 185.167167964554 lineto
442.383195898662 181.854615219999 lineto
446.766864732611 191.5 lineto
fill
grestore
gsave
403.215658860538 266.5 moveto
445.68992155163 312.916678976017 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
448.968891995806 316.5 moveto
439.635971519927 311.485361888339 lineto
443.00648289739 310.724816975401 lineto
443.744225461151 310.049734825129 lineto
444.800169466255 306.759786836438 lineto
448.968891995806 316.5 lineto
fill
grestore
gsave
402.848654316672 316.5 moveto
445.910949418146 270.061543051144 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
449.213561691716 266.5 moveto
444.980471856513 276.212412184782 lineto
443.946323127361 272.915548233055 lineto
443.21306426389 272.23559864732 lineto
439.847659812217 271.452765084636 lineto
449.213561691716 266.5 lineto
fill
grestore
gsave
400.279622509612 66.4999999999999 moveto
446.406430123315 113.049840064481 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
449.825235931493 116.5 moveto
440.300373431708 111.860280788647 lineto
443.637991912415 110.966368943191 lineto
444.348318957963 110.262497159155 lineto
445.272662750544 106.933178300391 lineto
449.825235931493 116.5 lineto
fill
grestore
gsave
406.151695211465 41.4999999999999 moveto
447.333832174283 87.8683958483363 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
450.559245019224 91.5 moveto
441.301797935058 86.3473624730767 lineto
444.683228562241 85.6369383576706 lineto
445.430911769936 84.9728827719474 lineto
446.535580388926 81.6989733730141 lineto
450.559245019224 91.5 lineto
fill
grestore
gsave
405.907025515554 366.5 moveto
447.558777144314 412.886010669476 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
450.803914715135 416.5 moveto
441.518550934097 411.397841686717 lineto
444.896063486769 410.66901765669 lineto
445.640120113642 410.000901097992 lineto
446.726947322206 406.721025775829 lineto
450.803914715135 416.5 lineto
fill
grestore
gsave
399.423278573925 91.5 moveto
448.033032728614 44.8626738307963 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
451.537923802866 41.4999999999999 moveto
446.745074792774 50.9487352786741 lineto
445.905149570548 47.5971235572374 lineto
445.21283437009 46.8755283360679 lineto
441.898868389568 45.8975687304874 lineto
451.537923802866 41.4999999999999 lineto
fill
grestore
gsave
405.173016427822 391.5 moveto
448.235311529296 345.061543051144 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
451.537923802866 341.5 moveto
447.304833967663 351.212412184782 lineto
446.27068523851 347.915548233055 lineto
445.53742637504 347.23559864732 lineto
442.172021923367 346.452765084636 lineto
451.537923802866 341.5 lineto
fill
grestore
gsave
396.97658161482 291.5 moveto
448.194351352705 244.773584432369 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
451.782593498777 241.5 moveto
446.753942568425 250.825377730724 lineto
445.998461176458 247.45372778202 lineto
445.32448791097 246.714972046064 lineto
442.036129710011 245.654087579032 lineto
451.782593498777 241.5 lineto
fill
grestore
gsave
405.540020971688 341.5 moveto
448.602316073161 387.938456948856 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
451.904928346732 391.5 moveto
442.539026467232 386.547234915364 lineto
445.904430918905 385.76440135268 lineto
446.637689782376 385.084451766945 lineto
447.671838511529 381.787587815218 lineto
451.904928346732 391.5 lineto
fill
grestore
gsave
404.561342188046 416.5 moveto
455.897815192607 369.769599342651 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
459.489688919958 366.5 moveto
454.450689595852 375.819790008989 lineto
453.698951317397 372.447303527043 lineto
453.025798511557 371.707800112588 lineto
449.738619954973 370.643266107807 lineto
459.489688919958 366.5 lineto
fill
grestore
gsave
547.570779447746 316.5 moveto
582.98032529337 294.096923780554 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
587.084935337295 291.5 moveto
580.505580324027 299.804341491616 lineto
580.350283886834 296.352579836056 lineto
579.815623108484 295.507513062307 lineto
576.76295487558 293.888874075373 lineto
587.084935337295 291.5 lineto
fill
grestore
gsave
547.937783991611 41.4999999999999 moveto
583.228639739531 63.8973243777277 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
587.329605033205 66.4999999999999 moveto
577.010983759591 64.0966575334109 lineto
580.065918218174 62.4822997775315 lineto
580.60176319923 61.6379833935396 lineto
580.761898626983 58.186442845468 lineto
587.329605033205 66.4999999999999 lineto
fill
grestore
gsave
546.469765816148 141.5 moveto
584.138791638012 164.008580383532 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
588.308283816847 166.5 moveto
577.928747548554 164.375093683197 lineto
580.939151021844 162.67914367267 lineto
581.452090354646 161.820718812321 lineto
581.51932287817 158.366119660758 lineto
588.308283816847 166.5 lineto
fill
grestore
gsave
543.289059769311 216.5 moveto
584.540544343312 193.838614176994 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
588.797623208668 191.5 moveto
581.718226936946 199.382394840905 lineto
581.77628719187 195.927629361599 lineto
581.294807802489 195.051171948144 lineto
578.347871211279 193.247192946716 lineto
588.797623208668 191.5 lineto
fill
grestore
gsave
543.411394617267 191.5 moveto
585.381262124634 214.190058756345 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
589.653967144355 216.5 moveto
579.192705031118 214.823099586097 lineto
582.127446982689 212.999349277559 lineto
582.603023121089 212.119674714676 lineto
582.521737999915 208.66537764591 lineto
589.653967144355 216.5 lineto
fill
grestore
gsave
551.363159734359 241.5 moveto
587.841978166867 263.953937077325 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
591.978329255505 266.5 moveto
581.627649319912 264.238711678953 lineto
584.660106215201 262.582517038091 lineto
585.184295640457 261.730915343371 lineto
585.296975296708 258.277499815916 lineto
591.978329255505 266.5 lineto
fill
grestore
gsave
531.422579517651 166.5 moveto
587.730783303674 143.347115402154 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
592.222998951416 141.5 moveto
584.305329304676 148.539922397661 lineto
584.749951784901 145.113395532119 lineto
584.369663319752 144.188527604643 lineto
581.64331004863 142.065846905327 lineto
592.222998951416 141.5 lineto
fill
grestore
gsave
549.52813701503 91.5 moveto
588.508781008927 114.066495845111 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
592.712338343237 116.5 moveto
582.304401131575 114.518878348482 lineto
585.29105628641 112.781447343721 lineto
585.792071847711 111.91600906901 lineto
585.81151006068 108.460810425505 lineto
592.712338343237 116.5 lineto
fill
grestore
gsave
541.331702202027 291.5 moveto
588.826227251838 314.391144906925 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
593.201677735058 316.5 moveto
582.673781158419 315.311137656578 lineto
585.520585945278 313.352955332145 lineto
585.954761993852 312.452127291482 lineto
585.713013498439 309.005341371938 lineto
593.201677735058 316.5 lineto
fill
grestore
gsave
531.177909821741 66.4999999999999 moveto
588.93891922406 43.3096761296608 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
593.446347430968 41.4999999999999 moveto
585.470379510178 48.4738035336914 lineto
585.943495385473 45.0510945366032 lineto
585.570915005837 44.1230946116515 lineto
582.862316852726 41.9778040590299 lineto
593.446347430968 41.4999999999999 lineto
fill
grestore
gsave
546.836770360014 116.5 moveto
591.205192079526 93.7185831766826 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
595.526039846208 91.5 moveto
588.228861733297 99.1812233132786 lineto
588.383565453586 95.7294350420874 lineto
587.926798328975 94.8398487371825 lineto
585.031491861019 92.9541191789437 lineto
595.526039846208 91.5 lineto
fill
grestore
gsave
539.129674938833 416.5 moveto
593.381421075836 393.402642009231 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
597.850401957358 391.5 moveto
590.020580413818 398.637499183631 lineto
590.422706542772 395.20572557708 lineto
590.030986129106 394.285641277943 lineto
587.278537518162 392.196909089674 lineto
597.850401957358 391.5 lineto
fill
grestore
gsave
545.735756728417 266.5 moveto
593.471083087497 243.600810074894 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
597.850401957358 241.5 moveto
590.347976249848 248.980882869241 lineto
590.596059040011 245.534547070247 lineto
590.16353931871 244.63292259704 lineto
587.320338200736 242.669511556795 lineto
597.850401957358 241.5 lineto
fill
grestore
gsave
551.730164278224 391.5 moveto
595.378414000145 414.254672458895 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
599.685424676687 416.5 moveto
589.200093143892 414.980848049938 lineto
592.107034511001 413.11310411717 lineto
592.569307828287 412.226366624941 lineto
592.436006364897 408.773685604333 lineto
599.685424676687 416.5 lineto
fill
grestore
gsave
572.160083886753 366.5 moveto
596.550796043569 344.733990180708 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
600.174764068507 341.5 moveto
595.044028706203 350.769603801781 lineto
594.325611721038 347.389862310834 lineto
593.659790213245 346.643751246876 lineto
590.383278151653 345.546826354075 lineto
600.174764068507 341.5 lineto
fill
grestore
gsave
539.007340090877 341.5 moveto
613.648071336203 365.0391459538 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
0.000 0.000 0.000 setrgbcolor AdjustColor
stroke
618.280321565886 366.5 moveto
607.690661559719 366.830304335096 lineto
610.227865610495 364.484804150715 lineto
610.52862967883 363.531105574016 lineto
609.796010038065 360.1544142982 lineto
618.280321565886 366.5 lineto
fill
grestore
gsave
0 29 moveto 700 0 rlineto 0 -29 rlineto -700 0 rlineto closepath
0.275 0.510 0.706 setrgbcolor AdjustColor
fill
0 29 moveto 700 0 rlineto 0 -29 rlineto -700 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
0.275 0.510 0.706 setrgbcolor AdjustColor
stroke
grestore
gsave
-30 483 moveto 30 0 rlineto 0 -483 rlineto -30 0 rlineto closepath
0.275 0.510 0.706 setrgbcolor AdjustColor
fill
-30 483 moveto 30 0 rlineto 0 -483 rlineto -30 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
0.275 0.510 0.706 setrgbcolor AdjustColor
stroke
grestore
gsave
-30 503 moveto 730 0 rlineto 0 -20 rlineto -730 0 rlineto closepath
0.275 0.510 0.706 setrgbcolor AdjustColor
fill
-30 503 moveto 730 0 rlineto 0 -20 rlineto -730 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
0.275 0.510 0.706 setrgbcolor AdjustColor
stroke
grestore
gsave
700 503 moveto 20 0 rlineto 0 -503 rlineto -20 0 rlineto closepath
0.275 0.510 0.706 setrgbcolor AdjustColor
fill
700 503 moveto 20 0 rlineto 0 -503 rlineto -20 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
0.275 0.510 0.706 setrgbcolor AdjustColor
stroke
grestore
gsave
0 429 moveto 700 0 rlineto 0 -400 rlineto -700 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
50 29 moveto
50 17 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
50 13.9999999999999 [
    (0.00041)
] 14 -0.5 0 0 () false DrawText
grestore
gsave
150 29 moveto
150 17 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
150 13.9999999999999 [
    (0.00123)
] 14 -0.5 0 0 () false DrawText
grestore
gsave
250 29 moveto
250 17 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
250 13.9999999999999 [
    (0.00204)
] 14 -0.5 0 0 () false DrawText
grestore
gsave
350 29 moveto
350 17 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
350 13.9999999999999 [
    (0.00286)
] 14 -0.5 0 0 () false DrawText
grestore
gsave
450 29 moveto
450 17 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
450 13.9999999999999 [
    (0.00368)
] 14 -0.5 0 0 () false DrawText
grestore
gsave
550 29 moveto
550 17 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
550 13.9999999999999 [
    (0.00450)
] 14 -0.5 0 0 () false DrawText
grestore
gsave
650 29 moveto
650 17 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
650 13.9999999999999 [
    (0.00531)
] 14 -0.5 0 0 () false DrawText
grestore
gsave
0 483 moveto 700 0 rlineto 0 -54 rlineto -700 0 rlineto closepath
0.275 0.510 0.706 setrgbcolor AdjustColor
fill
0 483 moveto 700 0 rlineto 0 -54 rlineto -700 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
7 476 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
1.000 0.000 0.000 setrgbcolor AdjustColor
fill
7 476 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
62 469 [
    (recv)
] 14 0 0.5 0 () false DrawText
grestore
gsave
113 476 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
0.000 0.000 1.000 setrgbcolor AdjustColor
fill
113 476 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
168 469 [
    (send)
] 14 0 0.5 0 () false DrawText
grestore
gsave
220 476 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
0.000 1.000 0.000 setrgbcolor AdjustColor
fill
220 476 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
275 469 [
    (state_0)
] 14 0 0.5 0 () false DrawText
grestore
gsave
344 476 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
0.000 1.000 1.000 setrgbcolor AdjustColor
fill
344 476 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
399 469 [
    (state_1)
] 14 0 0.5 0 () false DrawText
grestore
gsave
468 476 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
1.000 1.000 0.000 setrgbcolor AdjustColor
fill
468 476 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
523 469 [
    (state_2)
] 14 0 0.5 0 () false DrawText
grestore
gsave
7 451 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
1.000 0.000 1.000 setrgbcolor AdjustColor
fill
7 451 moveto 45 0 rlineto 0 -15 rlineto -45 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
62 444 [
    (state_3)
] 14 0 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 416.5 [
    (0)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 391.5 [
    (1)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 366.5 [
    (2)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 341.5 [
    (3)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 316.5 [
    (4)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 291.5 [
    (5)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 266.5 [
    (6)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 241.5 [
    (7)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 216.5 [
    (8)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 191.5 [
    (9)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 166.5 [
    (10)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 141.5 [
    (11)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 116.5 [
    (12)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 91.5 [
    (13)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 66.4999999999999 [
    (14)
] 14 -1 0.5 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
-5 41.4999999999999 [
    (15)
] 14 -1 0.5 0 () false DrawText
grestore
restore showpage

%%Trailer
end
%%EOF
