%!PS-Adobe-3.0 EPSF-3.0
%%Creator: Tk Canvas Widget
%%For: Rusty Lusk,
%%Title: Window .2.c
%%CreationDate: Sun Nov  6 23:02:19 1994
%%BoundingBox: 36 136 577 656
%%Pages: 1
%%DocumentData: Clean7Bit
%%Orientation: Portrait
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
1.547 1.547 scale
-150 -168 translate
-24 336 moveto 325 336 lineto 325 0 lineto -24 0 lineto closepath clip newpath
gsave
0 25.9999999999999 moveto
0 25.9999999999999 lineto
0 59.5294117647059 lineto
12 59.5294117647059 lineto
12 25.9999999999999 lineto
24 25.9999999999999 lineto
24 42.7647058823529 lineto
36 42.7647058823529 lineto
36 59.5294117647059 lineto
48 59.5294117647059 lineto
48 42.7647058823529 lineto
60 42.7647058823529 lineto
60 25.9999999999999 lineto
72 25.9999999999999 lineto
72 25.9999999999999 lineto
84 25.9999999999999 lineto
84 25.9999999999999 lineto
96 25.9999999999999 lineto
96 25.9999999999999 lineto
108 25.9999999999999 lineto
108 25.9999999999999 lineto
120 25.9999999999999 lineto
120 25.9999999999999 lineto
132 25.9999999999999 lineto
132 25.9999999999999 lineto
144 25.9999999999999 lineto
144 25.9999999999999 lineto
156 25.9999999999999 lineto
156 25.9999999999999 lineto
168 25.9999999999999 lineto
168 25.9999999999999 lineto
180 25.9999999999999 lineto
180 25.9999999999999 lineto
192 25.9999999999999 lineto
192 42.7647058823529 lineto
204 42.7647058823529 lineto
204 277.470588235294 lineto
216 277.470588235294 lineto
216 311 lineto
228 311 lineto
228 210.411764705882 lineto
240 210.411764705882 lineto
240 93.0588235294118 lineto
252 93.0588235294118 lineto
252 143.352941176471 lineto
264 143.352941176471 lineto
264 42.7647058823529 lineto
276 42.7647058823529 lineto
276 25.9999999999999 lineto
288 25.9999999999999 lineto
288 59.5294117647059 lineto
300 59.5294117647059 lineto
300 25.9999999999999 lineto
300 25.9999999999999 lineto
0 25.9999999999999 lineto
0.000 0.000 1.000 setrgbcolor AdjustColor
eofill
grestore
gsave
0 25.9999999999999 moveto
0 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
0 25.9999999999999 moveto
0 59.5294117647059 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
0 59.5294117647059 moveto
12 59.5294117647059 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
12 59.5294117647059 moveto
12 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
12 25.9999999999999 moveto
24 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
24 25.9999999999999 moveto
24 42.7647058823529 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
24 42.7647058823529 moveto
36 42.7647058823529 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
36 42.7647058823529 moveto
36 59.5294117647059 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
36 59.5294117647059 moveto
48 59.5294117647059 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
48 59.5294117647059 moveto
48 42.7647058823529 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
48 42.7647058823529 moveto
60 42.7647058823529 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
60 42.7647058823529 moveto
60 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
60 25.9999999999999 moveto
72 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
72 25.9999999999999 moveto
72 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
72 25.9999999999999 moveto
84 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
84 25.9999999999999 moveto
84 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
84 25.9999999999999 moveto
96 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
96 25.9999999999999 moveto
96 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
96 25.9999999999999 moveto
108 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
108 25.9999999999999 moveto
108 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
108 25.9999999999999 moveto
120 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
120 25.9999999999999 moveto
120 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
120 25.9999999999999 moveto
132 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
132 25.9999999999999 moveto
132 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
132 25.9999999999999 moveto
144 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
144 25.9999999999999 moveto
144 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
144 25.9999999999999 moveto
156 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
156 25.9999999999999 moveto
156 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
156 25.9999999999999 moveto
168 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
168 25.9999999999999 moveto
168 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
168 25.9999999999999 moveto
180 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
180 25.9999999999999 moveto
180 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
180 25.9999999999999 moveto
192 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
192 25.9999999999999 moveto
192 42.7647058823529 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
192 42.7647058823529 moveto
204 42.7647058823529 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
204 42.7647058823529 moveto
204 277.470588235294 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
204 277.470588235294 moveto
216 277.470588235294 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
216 277.470588235294 moveto
216 311 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
216 311 moveto
228 311 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
228 311 moveto
228 210.411764705882 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
228 210.411764705882 moveto
240 210.411764705882 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
240 210.411764705882 moveto
240 93.0588235294118 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
240 93.0588235294118 moveto
252 93.0588235294118 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
252 93.0588235294118 moveto
252 143.352941176471 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
252 143.352941176471 moveto
264 143.352941176471 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
264 143.352941176471 moveto
264 42.7647058823529 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
264 42.7647058823529 moveto
276 42.7647058823529 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
276 42.7647058823529 moveto
276 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
276 25.9999999999999 moveto
288 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
288 25.9999999999999 moveto
288 59.5294117647059 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
288 59.5294117647059 moveto
300 59.5294117647059 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
300 59.5294117647059 moveto
300 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
300 25.9999999999999 moveto
300 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
300 25.9999999999999 moveto
0 25.9999999999999 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
0 326 moveto 300 0 rlineto 0 -300 rlineto -300 0 rlineto closepath
1 setlinewidth 0 setlinejoin 2 setlinecap
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
0 25.9999999999999 moveto
0 16 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
300 25.9999999999999 moveto
300 16 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
0 14 [
    (0.00031)
] 14 -0.5 0 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
300 14 [
    (0.00044)
] 14 -0.5 0 0 () false DrawText
grestore
restore showpage

%%Trailer
end
%%EOF
