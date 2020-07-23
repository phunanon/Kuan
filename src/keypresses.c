#include <sys/time.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>

//http://ubuntuforums.org/showthread.php?t=554845

static struct termios g_old_kbd_mode;

static int kb_has_key () {
  struct timeval timeout;
  fd_set read_handles;
  //Check stdin (fd 0) for activity
  FD_ZERO(&read_handles);
  FD_SET(0, &read_handles);
  timeout.tv_sec = timeout.tv_usec = 0;
  return select(1, &read_handles, NULL, NULL, &timeout);
}

static void kb_restore () {
  tcsetattr(0, TCSANOW, &g_old_kbd_mode);
}

static void kb_listen () {
  struct termios new_kbd_mode;
  //Put stdin in raw, unbuffered mode
  tcgetattr(0, &g_old_kbd_mode);
  memcpy(&new_kbd_mode, &g_old_kbd_mode, sizeof(struct termios));
  new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
  new_kbd_mode.c_cc[VTIME] = 0;
  new_kbd_mode.c_cc[VMIN] = 1;
  tcsetattr(0, TCSANOW, &new_kbd_mode);
  atexit(kb_restore);
}