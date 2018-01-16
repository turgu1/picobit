/* ESP32 Scheme (picobit) VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#include "esp32_scheme_vm.h"
#include "testing.h"

#if WORKSTATION
  #include <termios.h>
  #include <unistd.h>
  #include <fcntl.h>

  PRIVATE struct termios oldattr, newattr;
  PRIVATE int oldf;
#endif

void kb_init()
{
  #if WORKSTATION
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
  #endif
}

void kb_restore()
{
  #if WORKSTATION
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
  #endif
}

char kb_getch(void)
{
  int ch;

  ch = getchar();

  return (char)((ch == EOF) ? 0 : ch);
}

#if TESTS
void kb_tests()
{
  char ch = 0;

  TESTM("kb");

  TEST("Keyboard entry");

  kb_init();

  printf("Please press any key, followed with character 'X': ");
  while (ch != 'X') {
    ch = kb_getch();
    if (ch) printf("\nch = %c", ch);
  }
  putchar('\n');

  EXPECT_TRUE(ch == 'X', "kb_getch");

  kb_restore();
}
#endif
