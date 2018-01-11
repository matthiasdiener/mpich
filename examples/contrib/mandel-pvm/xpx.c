/*
 *	xpx.c
 *
 *	usage:
 *		xpx wid hgt [imagefile]
 *	options:
 *		-display
 *		+/-color
 *		+/-c
 *		+/-lut
 *		+/-v
 *
 *	Display a raw (no header) 8 bit per pixel greyscale image in an Xwindow.
 *
 *	Jan 1991  Manchek  manchek@CS.UTK.EDU.
 *	Mar 1991  Added framing window for pix and lut, handles expose, lut
 *	          does rubberline.
 */


#include <sys/types.h>
#include <sys/file.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>

#define	NPV	64	/* this is the number of pixel values used,
	must be a power of two */

#define	min(a,b)	((a)<(b)?(a):(b))
#define	max(a,b)	((a)>(b)?(a):(b))

#define MYEVENTS (PointerMotionMask|ButtonPressMask|ButtonReleaseMask|ExposureMask|KeyPressMask)

/*******************************************
* Known arguments and fallbacks, resources *
*******************************************/

char *fallbacks[] = {
	".display:",
	".color:false",
	".lut:false",
	".v:false",
	0
};

XrmOptionDescRec knownargs[] = {
	{ "-display", ".display", XrmoptionSepArg, (caddr_t)0 },
	{ "-color", ".color", XrmoptionNoArg, "true"},
	{ "+color", ".color", XrmoptionSepArg, "false"},
	{ "-c", ".color", XrmoptionNoArg, "true"},
	{ "+c", ".color", XrmoptionNoArg, "false"},
	{ "-lut", ".lut", XrmoptionNoArg, "true"},
	{ "+lut", ".lut", XrmoptionNoArg, "false"},
	{ "-v", ".v", XrmoptionNoArg, "true"},
	{ "+v", ".v", XrmoptionNoArg, "false"}
};

struct {
	char *dispname;
	int docolor;
	int dolut;
	int portrait;
} app_res;


/**********
* Globals *
**********/

char *webe;			/* out a.out name */
Display *dis;		/* display */
Visual *defvi;		/* the default visual */
int sn;				/* def screen number */
u_long bp, wp;		/* black, white on screen */
int np;				/* number of planes on screen */
int bpp;			/* bits per pixel for direct display */
int bypp;			/* bytes per pixel for direct display */
u_long rmsk, gmsk, bmsk;
int rshf, gshf, bshf;
int nomap = 0;		/* not using a colormap */
Colormap cmap;		/* default colormap */
int ctes[NPV];		/* colormap indices given to us */
XImage *img;		/* image to display */
char *ribuf;		/* raw image buffer */
char *imbuf;		/* displayed image buffer */
int wid, hgt;		/* size of displayed image */
int nb;				/* length of raw image in bytes */
int lut[NPV];		/* the (user-specified) lookup table */
u_long reds[NPV];	/* red of rainbow lut */
u_long greens[NPV];	/* green of rainbow lut */
u_long blues[NPV];	/* blue of rainbow lut */
int ibo;			/* display image byte order for displays deeper than 8 */

/*	xparms()
*
*	Parse the command line, get resources, etc.
*/

xparms(ac, av, options, numoptions, fallbacks)
	int *ac;
	char **av;
	XrmOptionDescRec *options;
	int numoptions;
	char **fallbacks;
{
	char *getenv();

	XrmDatabase defdb, udb;
	XrmValue rval;

	char *hodir;
	char *xdfile;
	int l;

	webe = (webe = rindex(av[0], '/')) ? webe + 1 : av[0];

	XrmInitialize();
	defdb = XrmGetStringDatabase("");
	if (fallbacks) {
		while (*fallbacks) {
			XrmPutLineResource(&defdb, *fallbacks);
			fallbacks++;
		}
	}
	if (hodir = getenv("HOME")) {
		l = strlen(hodir);
		xdfile = (char*)malloc(l + 16);
		bcopy(hodir, xdfile, l);
		bcopy("/.Xdefaults", xdfile + l, 12);
		if (udb = XrmGetFileDatabase(xdfile)) {
			XrmMergeDatabases(udb, &defdb);
		}
	}
	if (options && numoptions > 0)
		XrmParseCommand(&defdb, options, numoptions, webe, ac, av);

	/*
	*	Wierd client-specific part -- have to make this parsing
	*	portable
	*/

	{
		char buf[256];
		static char *t[] = {"String", 0};

		strcpy(buf, webe);
		strcat(buf, ".display");
		if (XrmGetResource(defdb, buf, "Display", t, &rval))
			app_res.dispname = rval.addr;
		else
			app_res.dispname = 0;

		strcpy(buf, webe);
		strcat(buf, ".color");
		if (XrmGetResource(defdb, buf, "Color", t, &rval)) {
			if (!strcasecmp(rval.addr, "true")
			|| !strcasecmp(rval.addr, "on"))
				app_res.docolor = 1;
		} else
			app_res.docolor = 0;

		strcpy(buf, webe);
		strcat(buf, ".lut");
		if (XrmGetResource(defdb, buf, "Lut", t, &rval)) {
			if (!strcasecmp(rval.addr, "true")
			|| !strcasecmp(rval.addr, "on"))
				app_res.dolut = 1;
		} else
			app_res.dolut = 0;

		strcpy(buf, webe);
		strcat(buf, ".v");
		if (XrmGetResource(defdb, buf, "V", t, &rval)) {
			if (!strcasecmp(rval.addr, "true")
			|| !strcasecmp(rval.addr, "on"))
				app_res.portrait = 1;
		} else
			app_res.portrait = 0;
	}
}


main(argc, argv)
	int argc;
	char **argv;
{
	char *fn;

	/* get resources, options */

	xparms(&argc, argv, knownargs, sizeof(knownargs)/sizeof(knownargs[0]),
		fallbacks);

	if (argc < 3 || argc > 4) goto usage;

	wid = atoi(argv[1]);
	hgt = atoi(argv[2]);

	setupdpy();
	mkrbow(NPV, reds, greens, blues);

	/* read in the image data */

	fn = (argc > 3) ? argv[3] : 0;
	nb = wid * hgt;
	getbuf(fn, nb);

	setupimg();

	show();

	exit(0);

usage:
	fputs("usage: xpx wid hgt [filename]\n", stderr);
	fputs("options:\n", stderr);
	fputs("    -display <display name>\n", stderr);
	fputs("    +/-color\n", stderr);
	fputs("    +/-c\n", stderr);
	fputs("    +/-lut\n", stderr);
	fputs("    +/-v\n", stderr);
	exit(1);
}

/*	getbuf()
*
*	Read file fn (or stdin if fn = 0) into a buffer.
*/

getbuf(fn, nb)
	char *fn;
	int nb;
{
	int f;
	int i, j;
	char c;
	char *s = "stdin";

	if (fn) {
		if ((f = open(fn, O_RDONLY, 0)) == -1) {
			perror(fn);
			exit(1);
		}
	} else {
		fn = s;
		f = 0;
	}
	ribuf = (char*)malloc(nb);
	bzero(ribuf, nb);

	if ((j = tread(f, ribuf, nb)) == -1) {
		perror(fn);
		exit(1);
	}
	if (j < nb)
		fprintf(stderr, "%s: warning, early EOF\n", fn);
	else
		if (read(f, &c, 1) == 1)
			fprintf(stderr, "%s: warning, extra bytes\n", fn);
	if (fn != s) 
		(void)close(f);
}

/*	setupdpy()
*
*	Setup display.  Initializes globals
*		dis, sn, bp, wp, nomap, np, defvi.
*	If nomap is true, initializes
*		bpp, bypp, rmsk, rshf, gmsk, gshf, bmsk, bshf
*	else initializes
*		cmap, ctes
*/

setupdpy()
{
	XVisualInfo *vin;

	if (!(dis = XOpenDisplay(app_res.dispname))) {
		fputs("can't open display\n", stderr);
		exit(1);
	}
	sn = DefaultScreen(dis);

	bp = BlackPixel(dis, sn);
	wp = WhitePixel(dis, sn);

	np = DefaultDepth(dis, sn);
	defvi = DefaultVisual(dis, sn);

	if (defvi->class == TrueColor || defvi->class == DirectColor) {
		XPixmapFormatValues *pfv;
		int i;

		nomap = 1;
		if (!(pfv = XListPixmapFormats(dis, &i))) {
			fprintf(stderr, "can't get pixmap format list for screen\n");
			exit(1);
		}
		while (--i >= 0)
			if (pfv[i].depth == np) break;
		if (i < 0) {
			fprintf(stderr, "no pixmap format matches screen depth?\n");
			exit(1);
		}
		bypp = pfv[i].bits_per_pixel / 8;
/*
		printf("nomap=1, bypp=%d, np=%d\n", bypp, np);
*/
		rmsk = defvi->red_mask;
		rshf = ffs(rmsk & ~(rmsk >> 1)) - 8;
		gmsk = defvi->green_mask;
		gshf = ffs(gmsk & ~(gmsk >> 1)) - 8;
		bmsk = defvi->blue_mask;
		bshf = ffs(bmsk & ~(bmsk >> 1)) - 8;

		ibo = ImageByteOrder(dis);
	} else {
		nomap = 0;
		cmap = DefaultColormap(dis, sn);

		XAllocColorCells(dis, cmap, 0, NULL, 0, ctes, NPV);
	}
}

setupimg()
{
	int i;

	for (i = 0; i < NPV; i++)
		lut[i] = i;

	if (nomap) {
		imbuf = (char*)malloc(nb * bypp);
		colbuf(ribuf, imbuf, nb, NPV, lut, app_res.docolor, ibo);

		img = XCreateImage(dis, defvi, np, ZPixmap, 0,
			imbuf, wid, hgt, bpp, wid * bypp);

	} else {
		imbuf = (char*)malloc(nb);
		setlut(dis, cmap, NPV, ctes, lut, app_res.docolor);

		for (i = 0; i < nb; i++ ) {
			imbuf[i] = ctes[(ribuf[i] >> 2) & 0x3f];
		}

		img = XCreateImage(dis, defvi, 8, ZPixmap, 0, imbuf, wid, hgt, 8, wid);
	}
}

show()
{
	GC gc[4];
	Window mw, pixw, wl, w;
	XEvent event;
	XButtonEvent *bev = (XButtonEvent*)&event;
	XMotionEvent *mev = (XMotionEvent*)&event;
	XExposeEvent *eev = (XExposeEvent*)&event;
	XSizeHints xsh;
	int lwid = NPV, lhgt = NPV;
	int mwid, mhgt;
	int pixwx, pixwy, lutwx, lutwy;
	int dragging = 0;
	int x1, y1, x2, y2;
	int x, y;
	int ox, oy;
	int i, j;
	char keys[2];			/* for key press interpretation */

	if (app_res.dolut) {

	/* display image and look-up table, edit lut */

		if (app_res.portrait) {
			mwid = (wid > lwid * 4) ? wid + 10 : lwid * 4 + 10;
			mhgt = hgt + lhgt * 4 + 15;
			pixwx = 5;
			pixwy = 5;
			lutwx = 5;
			lutwy = hgt + 10;
		} else {
			mwid = wid + lwid * 4 + 15;
			mhgt = (hgt > lhgt * 4) ? hgt + 10 : lhgt * 4 + 10;
			pixwx = 5;
			pixwy = 5;
			lutwx = wid + 10;
			lutwy = 5;
		}
		mw = XCreateSimpleWindow(dis,
			RootWindow(dis, sn),
			0, 0, mwid, mhgt,
			1, wp, bp);
		XStoreName(dis, mw, "xpx");
		xsh.flags = USSize;
		xsh.width = mwid;
		xsh.height = mhgt;
		XSetNormalHints(dis, mw, &xsh);

		pixw = XCreateSimpleWindow(dis, mw, pixwx, pixwy, wid, hgt, 1, wp, bp);
	
		gc[2] = XCreateGC(dis, mw, (u_long)0, (XGCValues*)0);
		XSetBackground(dis, gc[2], bp);
		XSetFunction(dis, gc[2], GXcopy);
	
		for (x = 0; x < lwid; x++)
			lut[x] = x;
	
		wl = XCreateSimpleWindow(dis,
			mw, lutwx, lutwy, lwid * 4, lhgt * 4, 1, wp, bp);
	
		gc[0] = XCreateGC(dis, mw, (u_long)0, (XGCValues*)0);
		XSetBackground(dis, gc[0], wp);
		XSetForeground(dis, gc[0], bp);
		XSetFunction(dis, gc[0], GXcopy);
	
		gc[1] = XCreateGC(dis, mw, (u_long)0, (XGCValues*)0);
		XSetBackground(dis, gc[1], bp);
		XSetForeground(dis, gc[1], wp);
		XSetFunction(dis, gc[1], GXcopy);

		gc[3] = XCreateGC(dis, mw, (u_long)0, (XGCValues*)0);
		XSetForeground(dis, gc[3], bp ^ wp);
		XSetFunction(dis, gc[3], GXxor);
	
		XSelectInput(dis, mw, MYEVENTS);
		XMapWindow(dis, mw);
		XMapSubwindows(dis, mw);
	
		while (1) {
			XWindowEvent (dis, mw, MYEVENTS, &event);
	
			switch (event.type) {
	
			case Expose:
				if (eev->count == 0) {
					XClearWindow(dis, wl);
					for (x = 0; x < lwid; x++) {
						XFillRectangle(dis, wl, gc[1],
							x * 4, lhgt * 4 - 1 - lut[x] * 4,
							4, lut[x] * 4);
					}
					XPutImage(dis, pixw, gc[2], img, 0, 0, 0, 0, wid, hgt);
				}
				break;
	
			case ButtonPress:
				if (bev->subwindow != wl
				|| bev->button != Button1)
					break;
				XTranslateCoordinates(dis, mw, wl,
					(int)mev->x, (int)mev->y, &x, &y, &w);
				dragging = 1;
				ox = x1 = x;
				oy = y1 = y;
				XDrawLine(dis, wl, gc[3], x1, y1, ox, oy);
				break;

			case MotionNotify:
				if (mev->subwindow != wl)
					break;
				if (dragging) {
					XTranslateCoordinates(dis, mw, wl,
						(int)mev->x, (int)mev->y, &x, &y, &w);
					XDrawLine(dis, wl, gc[3], x1, y1, ox, oy);
					ox = x;
					oy = y;
					XDrawLine(dis, wl, gc[3], x1, y1, ox, oy);
				}
				break;
	
			case ButtonRelease:
				if (bev->subwindow != wl)
					break;
				XTranslateCoordinates(dis, mw, wl,
					(int)mev->x, (int)mev->y, &x, &y, &w);
				switch (bev->button) {
				case Button1:
					if (dragging) {
						dragging = 0;
						XDrawLine(dis, wl, gc[3], x1, y1, ox, oy);
						x1 /= 4;
						y1 /= 4;
						x2 = x / 4;
						y2 = y / 4;
						if (x2 < 0) x2 = 0;
						if (x2 >= lwid * 4) x2 = lwid * 4 - 1;
						if (y2 < 0) y2 = 0;
						if (y2 >= lhgt * 4) y2 = lhgt * 4 - 1;
						{
							int big, small;		/* dimensions of rectangle */
							int dx, dy;			/* delta x, y */
							int xdir, ydir;		/* x, y direction */
							int x, y;			/* position accumulator */
							int a = 0;			/* accum for interpolating */
							int i;
	
							if ((dx = x2 - x1) < 0) {
								dx = -dx; xdir = -1;
							} else
								xdir = 1;
							if ((dy = y2 - y1) < 0) {
								dy = -dy; ydir = -1;
							} else
								ydir = 1;
							big = max(dx, dy);
							small = min(dx, dy);
	
							x = x1;
							y = y1;
	
							for (i = 0; i <= big; i++) {
								lut[x] = lhgt - 1 - y;
								XFillRectangle(dis, wl, gc[0], x * 4, 0, 4, y * 4);
								XFillRectangle(dis, wl, gc[1], x * 4, y * 4, 4, (lhgt - y) * 4);
								if ((a += small) > big) {
									a -= big;
									if (dx < dy) x += xdir; else y += ydir;
								}
								if (dy > dx) y += ydir; else x += xdir;
							}
	
							if (nomap) {
								colbuf(ribuf, imbuf, nb, NPV, lut, app_res.docolor, ibo);
								XPutImage(dis, pixw, gc[2], img, 0, 0, 0, 0, wid, hgt);
							} else
								setlut(dis, cmap, NPV, ctes, lut, app_res.docolor);
							XFlush(dis);
						}
					}
					break;

				case Button2:
					printf("lut values:\n");
					for (i = 0; i < lwid; i++) {
						printf("%2d [%2d]%s", i, lut[i], (i & 7) == 7 ? "\n" : "   ");
					}
					break;

/*
				case Button3:
					exit(0);
					break;
*/

				default:
					break;
				}
				break;
	
			case KeyPress:
				if (XLookupString((XKeyEvent*)&event, keys, sizeof(keys), 0, 0)
				== 1 && (keys[0] == 'q' || keys[0] == 'Q'))
					exit(0);
				break;

			default:
				break;
			}
		}

	} else {

	/* no look-up table, just display */

		pixw = XCreateSimpleWindow(dis,
			RootWindow(dis, sn),
			0, 0, wid, hgt,
			1, 1, 1);
		XStoreName(dis, pixw, "xpx");
	
		xsh.flags = USSize;
		xsh.width = wid;
		xsh.height = hgt;
		XSetNormalHints(dis, pixw, &xsh);
	
		XSelectInput(dis, pixw, MYEVENTS);
	
		XMapWindow(dis, pixw);
	
		gc[2] = XCreateGC(dis, pixw, (u_long)0, (XGCValues*)0);
		XSetBackground(dis, gc[2], 0);
		XSetFunction(dis, gc[2], GXcopy);
	
		while (1) {
			XWindowEvent (dis, pixw, MYEVENTS, &event);
			switch (event.type) {
			case Expose:
				if (eev->count == 0)
					XPutImage(dis, pixw, gc[2], img, 0, 0, 0, 0, wid, hgt);
				break;
/*
			case ButtonRelease:
				exit(0);
*/
			case KeyPress:
				if (XLookupString((XKeyEvent*)&event, keys, sizeof(keys), 0, 0)
				== 1 && (keys[0] == 'q' || keys[0] == 'Q'))
					exit(0);
				break;

			default:
				break;
			}
		}
	}
}

/*	setlut()
*
*	Set colormap according to lut and whether we are in color.
*/

setlut(dis, cmap, n, ctes, lut, cbw)
	Display *dis;		/* display connection */
	Colormap cmap;		/* colormap to write */
	int n;				/* number of colormap entries */
	int *ctes;			/* colormap indices */
	int *lut;			/* lookup table values */
	int cbw;			/* color/b&w */
{
	XColor colr;
	int i, j;

	colr.flags = DoRed|DoGreen|DoBlue;

	if (cbw) {
		colr.pixel = ctes[n - 1];
		colr.red = colr.green = colr.blue = 0;
		XStoreColor(dis, cmap, &colr);
		for (i = 0; i < n; i++) {
			colr.pixel = ctes[i];
			colr.blue = (blues[lut[i]]) << 10;
			colr.green = (greens[lut[i]]) << 10;
			colr.red = (reds[lut[i]]) << 10;
			XStoreColor(dis, cmap, &colr);
		}

	} else {
		for (i = 0; i < n; i++) {
			colr.pixel = ctes[i];
			colr.red = colr.green = colr.blue = (lut[i] << 10);
			XStoreColor(dis, cmap, &colr);
		}
	}
}

u_long
sw4(lp)
	u_long lp;
{
	u_char *cp = (u_char*)&lp;
	u_char c;

	c = cp[0];
	cp[0] = cp[3];
	cp[3] = c;
	c = cp[1];
	cp[1] = cp[2];
	cp[2] = c;
	return lp;
}

/*	colbuf()
*
*	Recolor memory-image according to lut and whether we are in color.
*/

colbuf(ribuf, imbuf, len, n, lut, cbw, ibo)
	unsigned char *ribuf;
	unsigned char *imbuf;
	int len;
	int n;				/* number of colormap entries */
	int *lut;			/* lookup table values */
	int cbw;			/* color/b&w */
	int	ibo;			/* image byte order, either LSBFirst or MSBFirst */
{
	int i, j;
	u_long pixv, r, g, b;
	int lbs;
	int lbm;
	u_long mbo = 0x04030201;	/* machine byte order */
	int bs = 0;

	if ((*(char*)&mbo == 1 && ibo == MSBFirst)
	|| (*(char*)&mbo == 4 && ibo == LSBFirst))
		bs = 1;

	lbm = n - 1;
	lbs = 8 - (ffs(n) - 1);

	if (cbw) {
		for (i = 0; i < len; i++) {
			r = reds[lut[lbm & (ribuf[i] >> lbs)]] << lbs;
			g = greens[lut[lbm & (ribuf[i] >> lbs)]] << lbs;
			b = blues[lut[lbm & (ribuf[i] >> lbs)]] << lbs;
			pixv = (rmsk & (r << rshf))
				| (gmsk & (g << gshf))
				| (bmsk & (b << bshf));
			if (bs)
				((u_long*)imbuf)[i] = sw4(pixv);
			else
				((u_long*)imbuf)[i] = pixv;
		}

	} else {
		for (i = 0; i < len; i++) {
			pixv = lut[lbm & (ribuf[i] >> lbs)] << lbs;
			pixv = (rmsk & (pixv << rshf))
				| (gmsk & (pixv << gshf))
				| (bmsk & (pixv << bshf));
			if (bs)
				((u_long*)imbuf)[i] = sw4(pixv);
			else
				((u_long*)imbuf)[i] = pixv;
		}
	}
}

/*	mkrbow()
*
*	Generate a rainbow lut.  0 is black, n-1 is white, and entries
*	between those two go through the spectrum from red to violet.
*/

mkrbow(n, r, g, b)
	int n;				/* number of entries */
	u_long *r, *g, *b;	/* red, grn, blu lut return */
{
	int i, j;
	double d, e;

	for (i = 1; i < n - 1; i++) {
		j = n - 1 - i;
		d = (d = cos((double)((j - n * 0.16) * (3.1415926535 / n)))) < 0.0
			? 0.0 : d;
		b[i] = d * n;
		d = (d = cos((double)((j - n * 0.52) * (3.1415926535 / n)))) < 0.0
			? 0.0 : d;
		g[i] = d * n;
		d = (d = cos((double)((j - n * .83) * (3.1415926535 / n)))) < 0.0
			? 0.0 : d;
		e = (e = cos((double)(j * (3.1415926535 / n)))) < 0.0
			? 0.0 : e;
		r[i] = d * n + e * (n / 2);
	}
	r[i] = g[i] = b[i] = i;
	r[0] = g[0] = b[0] = 0;
}

/*	tread()
*
*	Tenacious read.  Just like read(2) except that it will keep trying
*	to get those n bytes until there really aren't any more.
*	Returns the number it got or -1 if error.
*/

int
tread(d, b, n)
	int d;
	char *b;
	int n;
{
	int e;
	int x = n;

	while (x > 0) {
		if ((e = read(d, b, x)) == -1)
			return e;
		if (!e)
			return n - x;
		b += e;
		x -= e;
	}
	return n - x;
}
