/*
 *  statusbar.c
 *  Author: Alex Kozadaev (2015)
 */

#include "build_host.h"

#define _BSD_SOURCE
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <alsa/asoundlib.h>

#define MIN(a, b) ((a) > (b) ? (b) : (a))

#define DTBUFSZ   20
#define STRSZ     64

static void open_display()              __attribute__ ((unused));
static void close_display()             __attribute__ ((unused));
static void set_status(char *str);
static void get_datetime(char *dstbuf);
static void get_load_average(char *dstla);
static int get_vol(void);

static Display *dpy                     __attribute__ ((unused));

int main(int argc, char **argv)
{
    int vol = 0;
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
        vol = get_vol();
        get_load_average(la);
        get_datetime(dt);                       /* date/time */

        snprintf(stat, STRSZ, "%u | %s | %s", vol, la, dt);
        set_status(stat);
    }

#ifndef DEBUG
    close_display();
#endif
    return 0;
}

static void open_display()
{
#ifndef DEBUG
    if (!(dpy = XOpenDisplay(NULL)))
        exit(1);
#endif
    signal(SIGINT, close_display);
    signal(SIGTERM, close_display);
}

static void close_display()
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

static int get_vol(void)
{
    long min, max, volume = 0;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Master";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_get_playback_volume(elem, 0, &volume);
    snd_mixer_close(handle);

    return ((double)volume / max) * 100;
}
