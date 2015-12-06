/*
 *  statusbar.c
 *  Author: Alex Kozadaev (2015)
 */

#include "build_host.h"

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <X11/Xlib.h>

#define MIN(a, b) ((a) > (b) ? (b) : (a))

#define DTBUFSZ   20
#define STRSZ     64

static void open_display(void)          __attribute__ ((unused));
static void close_display()             __attribute__ ((unused));
static void set_status(char *str);
static void get_datetime(char *dstbuf);
static void get_load_average(char *dstla);

static Display *dpy                     __attribute__ ((unused));

int main(int argc, char **argv)
{
    char  la[STRSZ] = { 0 };    /* load average   */
    char  dt[STRSZ] = { 0 };    /* date/time      */
    char  stat[STRSZ] = { 0 };  /* full string    */
    /* should be the same order as the enum above (C, D, U, F) */

    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        printf("dwm-statusbar v%s"
#ifdef DEBUG
               " (debug)"
#endif
               " [%s %s]\n\nUsage: %s [-v]\n\n",
                BUILD_VERSION, BUILD_OS, BUILD_KERNEL, argv[0]);
        exit(0);
    }

#ifndef DEBUG
    open_display();
#endif

    while (!sleep(1)) {
        get_load_average(la);
        get_datetime(dt);                       /* date/time */

        snprintf(stat, STRSZ, "%s | %s", la, dt);
        set_status(stat);
    }

#ifndef DEBUG
    close_display();
#endif
    return 0;
}

static void open_display(void)
{
#ifndef DEBUG
    if (!(dpy = XOpenDisplay(NULL)))
        exit(1);
#endif
    signal(SIGINT, close_display);
    signal(SIGTERM, close_display);
}

static void close_display(void)
{
#ifndef DEBUG
    XCloseDisplay(dpy);
#endif
    exit(0);
}

static void set_status(char *str)
{
#ifndef DEBUG
    XStoreName(dpy, DefaultRootWindow(dpy), str);
    XSync(dpy, False);
#else
    puts(str);
#endif
}

static void get_load_average(char *dstla)
{
    double avgs[3];

    if (getloadavg(avgs, 3) < 0) {
        set_status("getloadavg");
        exit(1);
    }

    snprintf(dstla, STRSZ, "%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}

static void get_datetime(char *dstbuf)
{
    time_t rawtime;
    time(&rawtime);
    snprintf(dstbuf, DTBUFSZ, "%s", ctime(&rawtime));
}

