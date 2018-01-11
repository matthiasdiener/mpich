%!PS-Adobe-3.0 EPSF-3.0
%%Creator: Tk Canvas Widget
%%For: Rusty Lusk,
%%Title: Window .1.c
%%CreationDate: Wed Feb 15 16:49:38 1995
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
/CL 1 def
%%IncludeResource: font Helvetica-Bold
%%EndSetup

%%Page: 1 1
save
306.0 396.0 translate
1.547 1.547 scale
-150 -168 translate
-24 336 moveto 325 336 lineto 325 0 lineto -24 0 lineto closepath clip newpath
gsave
0 26 moveto
0 26 lineto
0 33.8473091364205 lineto
6 33.8473091364205 lineto
6 53.8222778473091 lineto
12 53.8222778473091 lineto
12 26.3566958698373 lineto
18 26.3566958698373 lineto
18 201.49436795995 lineto
24 201.49436795995 lineto
24 182.946182728411 lineto
30 182.946182728411 lineto
30 32.7772215269086 lineto
36 32.7772215269086 lineto
36 39.19774718398 lineto
42 39.19774718398 lineto
42 89.848560700876 lineto
48 89.848560700876 lineto
48 41.3379224030037 lineto
54 41.3379224030037 lineto
54 107.326658322904 lineto
60 107.326658322904 lineto
60 55.9624530663329 lineto
66 55.9624530663329 lineto
66 45.9749687108886 lineto
72 45.9749687108886 lineto
72 260.349186483104 lineto
78 260.349186483104 lineto
78 210.055068836045 lineto
84 210.055068836045 lineto
84 39.5544430538173 lineto
90 39.5544430538173 lineto
90 36.3441802252816 lineto
96 36.3441802252816 lineto
96 44.5481852315394 lineto
102 44.5481852315394 lineto
102 27.4267834793492 lineto
108 27.4267834793492 lineto
108 57.0325406758448 lineto
114 57.0325406758448 lineto
114 29.566958698373 lineto
120 29.566958698373 lineto
120 182.946182728411 lineto
126 182.946182728411 lineto
126 311 lineto
132 311 lineto
132 46.3316645807259 lineto
138 46.3316645807259 lineto
138 93.0588235294117 lineto
144 93.0588235294117 lineto
144 50.9687108886108 lineto
150 50.9687108886108 lineto
150 28.1401752190238 lineto
156 28.1401752190238 lineto
156 36.7008760951189 lineto
162 36.7008760951189 lineto
162 29.566958698373 lineto
168 29.566958698373 lineto
168 26.3566958698373 lineto
174 26.3566958698373 lineto
174 27.0700876095119 lineto
180 27.0700876095119 lineto
180 27.0700876095119 lineto
186 27.0700876095119 lineto
186 26 lineto
192 26 lineto
192 26.7133917396745 lineto
198 26.7133917396745 lineto
198 28.1401752190238 lineto
204 28.1401752190238 lineto
204 26.3566958698373 lineto
210 26.3566958698373 lineto
210 61.3128911138924 lineto
216 61.3128911138924 lineto
216 44.9048811013767 lineto
222 44.9048811013767 lineto
222 29.566958698373 lineto
228 29.566958698373 lineto
228 33.8473091364205 lineto
234 33.8473091364205 lineto
234 34.2040050062578 lineto
240 34.2040050062578 lineto
240 28.4968710888611 lineto
246 28.4968710888611 lineto
246 51.6821026282853 lineto
252 51.6821026282853 lineto
252 32.7772215269086 lineto
258 32.7772215269086 lineto
258 62.3829787234043 lineto
264 62.3829787234043 lineto
264 37.4142678347935 lineto
270 37.4142678347935 lineto
270 27.7834793491865 lineto
276 27.7834793491865 lineto
276 30.9937421777221 lineto
282 30.9937421777221 lineto
282 30.2803504380475 lineto
288 30.2803504380475 lineto
288 26 lineto
294 26 lineto
294 27.0700876095119 lineto
300 27.0700876095119 lineto
300 26 lineto
300 26 lineto
0 26 lineto
0.804 0.522 0.000 setrgbcolor AdjustColor
eofill
grestore
gsave
0 26 moveto
0 26 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
0 26 moveto
0 33.8473091364205 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
0 33.8473091364205 moveto
6 33.8473091364205 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
6 33.8473091364205 moveto
6 53.8222778473091 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
6 53.8222778473091 moveto
12 53.8222778473091 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
12 53.8222778473091 moveto
12 26.3566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
12 26.3566958698373 moveto
18 26.3566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
18 26.3566958698373 moveto
18 201.49436795995 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
18 201.49436795995 moveto
24 201.49436795995 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
24 201.49436795995 moveto
24 182.946182728411 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
24 182.946182728411 moveto
30 182.946182728411 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
30 182.946182728411 moveto
30 32.7772215269086 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
30 32.7772215269086 moveto
36 32.7772215269086 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
36 32.7772215269086 moveto
36 39.19774718398 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
36 39.19774718398 moveto
42 39.19774718398 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
42 39.19774718398 moveto
42 89.848560700876 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
42 89.848560700876 moveto
48 89.848560700876 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
48 89.848560700876 moveto
48 41.3379224030037 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
48 41.3379224030037 moveto
54 41.3379224030037 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
54 41.3379224030037 moveto
54 107.326658322904 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
54 107.326658322904 moveto
60 107.326658322904 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
60 107.326658322904 moveto
60 55.9624530663329 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
60 55.9624530663329 moveto
66 55.9624530663329 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
66 55.9624530663329 moveto
66 45.9749687108886 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
66 45.9749687108886 moveto
72 45.9749687108886 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
72 45.9749687108886 moveto
72 260.349186483104 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
72 260.349186483104 moveto
78 260.349186483104 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
78 260.349186483104 moveto
78 210.055068836045 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
78 210.055068836045 moveto
84 210.055068836045 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
84 210.055068836045 moveto
84 39.5544430538173 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
84 39.5544430538173 moveto
90 39.5544430538173 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
90 39.5544430538173 moveto
90 36.3441802252816 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
90 36.3441802252816 moveto
96 36.3441802252816 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
96 36.3441802252816 moveto
96 44.5481852315394 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
96 44.5481852315394 moveto
102 44.5481852315394 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
102 44.5481852315394 moveto
102 27.4267834793492 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
102 27.4267834793492 moveto
108 27.4267834793492 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
108 27.4267834793492 moveto
108 57.0325406758448 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
108 57.0325406758448 moveto
114 57.0325406758448 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
114 57.0325406758448 moveto
114 29.566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
114 29.566958698373 moveto
120 29.566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
120 29.566958698373 moveto
120 182.946182728411 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
120 182.946182728411 moveto
126 182.946182728411 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
126 182.946182728411 moveto
126 311 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
126 311 moveto
132 311 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
132 311 moveto
132 46.3316645807259 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
132 46.3316645807259 moveto
138 46.3316645807259 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
138 46.3316645807259 moveto
138 93.0588235294117 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
138 93.0588235294117 moveto
144 93.0588235294117 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
144 93.0588235294117 moveto
144 50.9687108886108 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
144 50.9687108886108 moveto
150 50.9687108886108 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
150 50.9687108886108 moveto
150 28.1401752190238 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
150 28.1401752190238 moveto
156 28.1401752190238 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
156 28.1401752190238 moveto
156 36.7008760951189 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
156 36.7008760951189 moveto
162 36.7008760951189 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
162 36.7008760951189 moveto
162 29.566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
162 29.566958698373 moveto
168 29.566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
168 29.566958698373 moveto
168 26.3566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
168 26.3566958698373 moveto
174 26.3566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
174 26.3566958698373 moveto
174 27.0700876095119 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
174 27.0700876095119 moveto
180 27.0700876095119 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
180 27.0700876095119 moveto
180 27.0700876095119 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
180 27.0700876095119 moveto
186 27.0700876095119 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
186 27.0700876095119 moveto
186 26 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
186 26 moveto
192 26 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
192 26 moveto
192 26.7133917396745 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
192 26.7133917396745 moveto
198 26.7133917396745 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
198 26.7133917396745 moveto
198 28.1401752190238 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
198 28.1401752190238 moveto
204 28.1401752190238 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
204 28.1401752190238 moveto
204 26.3566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
204 26.3566958698373 moveto
210 26.3566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
210 26.3566958698373 moveto
210 61.3128911138924 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
210 61.3128911138924 moveto
216 61.3128911138924 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
216 61.3128911138924 moveto
216 44.9048811013767 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
216 44.9048811013767 moveto
222 44.9048811013767 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
222 44.9048811013767 moveto
222 29.566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
222 29.566958698373 moveto
228 29.566958698373 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
228 29.566958698373 moveto
228 33.8473091364205 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
228 33.8473091364205 moveto
234 33.8473091364205 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
234 33.8473091364205 moveto
234 34.2040050062578 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
234 34.2040050062578 moveto
240 34.2040050062578 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
240 34.2040050062578 moveto
240 28.4968710888611 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
240 28.4968710888611 moveto
246 28.4968710888611 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
246 28.4968710888611 moveto
246 51.6821026282853 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
246 51.6821026282853 moveto
252 51.6821026282853 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
252 51.6821026282853 moveto
252 32.7772215269086 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
252 32.7772215269086 moveto
258 32.7772215269086 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
258 32.7772215269086 moveto
258 62.3829787234043 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
258 62.3829787234043 moveto
264 62.3829787234043 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
264 62.3829787234043 moveto
264 37.4142678347935 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
264 37.4142678347935 moveto
270 37.4142678347935 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
270 37.4142678347935 moveto
270 27.7834793491865 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
270 27.7834793491865 moveto
276 27.7834793491865 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
276 27.7834793491865 moveto
276 30.9937421777221 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
276 30.9937421777221 moveto
282 30.9937421777221 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
282 30.9937421777221 moveto
282 30.2803504380475 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
282 30.2803504380475 moveto
288 30.2803504380475 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
288 30.2803504380475 moveto
288 26 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
288 26 moveto
294 26 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
294 26 moveto
294 27.0700876095119 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
294 27.0700876095119 moveto
300 27.0700876095119 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
300 27.0700876095119 moveto
300 26 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
300 26 moveto
300 26 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
300 26 moveto
0 26 lineto
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
0 26 moveto
0 16 lineto
1 setlinewidth
0 setlinecap
1 setlinejoin
1.000 0.980 0.980 setrgbcolor AdjustColor
stroke
grestore
gsave
300 26 moveto
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
    (0.00000)
] 14 -0.5 0 0 () false DrawText
grestore
gsave
/Helvetica-Bold findfont 12 scalefont setfont
1.000 0.980 0.980 setrgbcolor AdjustColor
300 14 [
    (0.00100)
] 14 -0.5 0 0 () false DrawText
grestore
restore showpage

%%Trailer
end
%%EOF
