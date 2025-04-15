// LICENSE NEEDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpd/client.h>

#define FPS 2
#define FIELD_WIDTH 25
#define PADDING_WIDTH 10


int
ustrlen(const char *s)
{
	int l = 0;
	while (*s) {
		if ((*s & 0xC0) != 0x80)
			l++;
		s++;
	}
	return l;
}

void
uputnc(const char *s, int n)
{
	int m = 0;
	while (m < n) {
		putchar(*s);
		if ((*++s & 0xC0) != 0x80)
			m++;
	}
}

int
main ()
{
	struct mpd_connection *conn = mpd_connection_new(NULL, 0, 0);
	if (conn == NULL)
		exit(2);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		fprintf(stderr,
			"%s\n",
			mpd_connection_get_error_message(conn));
		mpd_connection_free(conn);
		exit(3);
	}

	struct mpd_song *song =	mpd_run_current_song(conn);
	if (song == NULL) /* no song is playing */
		exit(0);
	const char *artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
	const char *title  = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	const struct mpd_status *status = mpd_run_status(conn);
	unsigned int secs = mpd_status_get_elapsed_time(status);
	unsigned int secs_total = mpd_status_get_total_time(status);

	int progress = 100 * secs / secs_total;

	int info_width = ustrlen(artist) + ustrlen(title) + 3;
	if (info_width <= FIELD_WIDTH) {
		printf("[ %s - %s ][%d%%]\n",
		       artist,
		       title,
		       progress);
		mpd_connection_free(conn);
		exit(0);
	}

	int padding2 = FPS - (info_width + PADDING_WIDTH) % FPS;
	int text_length = (info_width + PADDING_WIDTH + padding2);
	char *text = malloc(text_length + 1);
	memset(stpcpy(stpcpy(stpcpy(text, artist),
			     " - "),
		      title),
	       0x20,
	       (PADDING_WIDTH+padding2)); // assumes last character of inof is not multibyte
	text[text_length] = '\0';

	int scroll_length = text_length * 3;
	char *scroll = malloc(scroll_length + 1);
	if (scroll == NULL) {
		puts("ERROR!");
		exit(1);
	}
	scroll[scroll_length] = '\0';
	strcpy(scroll, text);
	strcpy(scroll + text_length, text);
	strcpy(scroll + 2 * text_length, text);

	struct timespec time;
	if (clock_gettime(CLOCK_MONOTONIC, &time) < 0)
		exit(1);
	int frames = (time.tv_sec % (text_length / FPS)) * FPS;
	frames += (time.tv_nsec / 100000000) * (FPS / 10.0);

	fputs("[ ", stdout);
	uputnc(scroll + frames, FIELD_WIDTH); // fails for multibyte
	printf(" ][%d%%]\n", progress);

	mpd_connection_free(conn);
	return 0;
}
