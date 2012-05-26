#include <glib.h>
#include <locale.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "../src/lib/xmrservice.h"

int main(int argc, char **argv)
{
	GObject *service;
	gint result;
	gchar usr_name[32], usr_pwd[32];
	struct termios term, term_orig;

	setlocale(LC_ALL, "");
	g_type_init();

	service = xmr_service_new();

	g_print("Enter username: ");
	scanf("%s", usr_name);
	g_print("Enter password: ");

	tcgetattr(STDIN_FILENO, &term);
	term_orig = term;
	term.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &term);

	scanf("%s", usr_pwd);

	tcsetattr(STDIN_FILENO, TCSANOW, &term_orig);
	g_print("\n");

	result = xmr_service_login(XMR_SERVICE(service),
				usr_name,
				usr_pwd
				);
	if (result != 0)
	{
		g_print("login failed(%d)\n", result);
	}
	else
	{
		g_print("login ok\n");
	}

	g_object_unref(service);

	return 0;
}
