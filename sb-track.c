// LICENSE NEEDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpd/client.h>

#define FPS 2
#define FIELD_WIDTH 25
#define PADDING_WIDTH 10

static int
u8strlen(const char *s)
{
	int l = 0;
	while (*s) {
		if ((*s & 0xC0) != 0x80)
			l++;
		s++;
	}
	return l;
}

static void
u8putnc(const char *s, int n)
{
	while (*s && n) {
		putchar(*s);
		if ((*++s & 0xC0) != 0x80)
			n--;
	}
}

const char *
u8subscript(const char *s, int i)
{
	while (*s++ && i)
		if ((*s & 0xC0) != 0x80)
			i--;
	return s - 1;
}

int
main ()
{
	struct mpd_connection *conn = mpd_connection_new(NULL, 0, 0);
	if (conn == NULL) {
		puts("[ failed to connect to MPD ]");
		return 1;
	}
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		fprintf(stderr,
			"%s\n",
			mpd_connection_get_error_message(conn));
		mpd_connection_free(conn);
		puts("[ failed to connect to MPD ]");
		return 2;
	}

	struct mpd_song *song =	mpd_run_current_song(conn);
	if (song == NULL) {
		puts("");
		return 0;
	}
	const char *artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
	const char *title  = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	const struct mpd_status *status = mpd_run_status(conn);
	unsigned int secs = mpd_status_get_elapsed_time(status);
	unsigned int secs_total = mpd_status_get_total_time(status);
	if (!secs || !secs_total) {
		puts("");
		return 0;
	}
	int progress = 100 * secs / secs_total;

	int info_width = u8strlen(artist) + u8strlen(title) + 3;
	if (info_width <= FIELD_WIDTH) {
		printf("[ %s - %s ][%d%%]\n",
		       artist,
		       title,
		       progress);
		mpd_connection_free(conn);
		return 0;
	}

	int padding2 = FPS - (info_width + PADDING_WIDTH) % FPS;
	int text_length = (strlen(artist)
			   + 3
			   + strlen(title)
			   + PADDING_WIDTH
			   + padding2);
	char *text = malloc(text_length + 1);
	memset(stpcpy(stpcpy(stpcpy(text, artist),
			     " - "),
		      title),
	       0x20,
	       (PADDING_WIDTH+padding2));
	text[text_length] = '\0';

	int scroll_length = text_length * 3;
	char *scroll = malloc(scroll_length + 1);
	if (scroll == NULL) {
		puts("[ memory error ]");
		return 4;
	}
	scroll[scroll_length] = '\0';
	strcpy(scroll, text);
	strcpy(scroll + text_length, text);
	strcpy(scroll + 2 * text_length, text);

	struct timespec time;
	if (clock_gettime(CLOCK_MONOTONIC, &time) < 0)
		puts("[ failed to get system time ]");
	int frames = (time.tv_sec %
		      ((info_width +
			PADDING_WIDTH +
			padding2) /
		       FPS)) * FPS;
	frames += (time.tv_nsec / 100000000) * (FPS / 10.0);

	fputs("[ ", stdout);
	u8putnc(u8subscript(scroll, frames), FIELD_WIDTH);
	printf(" ][%d%%]\n", progress);

	mpd_connection_free(conn);
	return 0;
}
