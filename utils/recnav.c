/* -*- mode: C -*- Time-stamp: "2010-12-20 20:30:52 jemarch"
 *
 *       File:         recnav.c
 *       Date:         Mon Dec 20 16:43:01 2010
 *
 *       GNU recutils - recnav
 *
 */

/* Copyright (C) 2010 Jose E. Marchesi */

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <xalloc.h>
#include <gettext.h>
#define _(str) gettext (str)

#include <cdk/cdk.h>

#include <rec.h>
#include <recutl.h>

/* Forward prototypes.  */
void recnav_parse_args (int argc, char **argv);
void recnav_navigate (rec_db_t db);
void recnav_init_app (void);
void recnav_cmd_about (void);
/* void recnav_load_db (rec_db_t db); */

/*
 * Global variables.
 */

char      *program_name        = NULL;
CDKSCREEN *recnav_screen       = NULL;
CDKLABEL  *recnav_app_title    = NULL;
CDKMENU   *recnav_menu         = NULL;
WINDOW    *recnav_app_window   = NULL;

/*
 * Command line options management.
 */

enum
{
  COMMON_ARGS
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {NULL, 0, NULL, 0}
  };

/*
 * Functions.
 */

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recnav synopsis.
     no-wrap */
  printf (_("\
Usage: recnav [OPTION]... [FILE]...\n"));

  /* TRANSLATORS: --help output, recnav arguments.
     no-wrap */
  fputs(_("\
Navigate the contents of a recfile.\n"), stdout);

  recutl_print_help_common ();

  puts ("");
  /* TRANSLATORS: --help output, recnav examples.
     no-wrap */
  fputs (_("\
Examples:\n\
\n\
        recnav data.rec\n"),
         stdout);

  puts ("");
  recutl_print_help_footer ();
}

void
recnav_parse_args (int argc,
                   char **argv)
{
  char c;
  char ret;

  while ((ret = getopt_long (argc,
                             argv,
                             "",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }
}

void
recnav_cmd_about ()
{
  char *mesg[15];

  mesg[0] = "<C></U>About recnav";
  mesg[1] = " ";
  mesg[2] = "</B/24>This program is part of the GNU recutils suite.";
  mesg[3] = " ";
  mesg[4] = "<C></B/24>Jose E. Marchesi";
  mesg[5] = "<C><#HL(35)>";
  mesg[6] = "<R></B/24>December 2010";
  
  popupLabel (recnav_screen, mesg, 7);
}

void
recnav_init_app ()
{
  char *menulist[MAX_MENU_ITEMS][MAX_SUB_ITEMS];
  int sub_menu_size[10], menu_locations[10];
  char *title[5];

  /* Set up CDK.  */
  recnav_app_window = initscr ();
  recnav_screen = initCDKScreen (recnav_app_window);

  /* Start CDK color.  */
  initCDKColor ();

  /* Create the menu lists. */

  menulist[0][0] = "</U>File";      menulist[1][0] = "</U>Tools";
  menulist[0][1] = "</B/5>Open   "; menulist[1][1] = "</B/5>Check db";
  menulist[0][2] = "</B/5>Save   ";
  menulist[0][3] = "</B/5>Save As";
  menulist[0][4] = "</B/5>Quit   ";

  menulist[2][0] = "</U>Help";
  menulist[2][1] = "</B/5>About recnav";

  /* Set up the sub-menu sizes and their locations. */

  sub_menu_size[0] = 5; menu_locations[0] = LEFT;
  sub_menu_size[1] = 2; menu_locations[1] = LEFT;
  sub_menu_size[2] = 2; menu_locations[2] = RIGHT;

  /* Create the menu.  */
  recnav_menu = newCDKMenu (recnav_screen,
                            menulist,
                            3,
                            sub_menu_size,
                            menu_locations,
                            TOP,
                            A_BOLD | A_UNDERLINE,
                            A_REVERSE);

  /* Create the application title.  */
  title[0] = "<C></U>recnav";
  title[1] = "<C></B/24>GNU recutils";
  recnav_app_title = newCDKLabel (recnav_screen,
                                  CENTER, CENTER,
                                  title,
                                  2,
                                  FALSE,
                                  FALSE);

  /* Draw the CDK screen.  */
  refreshCDKScreen (recnav_screen);
}

void
recnav_navigate (rec_db_t db)
{
  int selection;

  /* Initialize the application.  */
  recnav_init_app ();

  /* Load the db.  */
  /*  recnav_load_db (db); */

  /* Loop until done.  */
  for (;;)
    {
      /* Activate the menu */
      selection = activateCDKMenu (recnav_menu, 0);

      /* Check the return value of the selection.  */
      if (selection == 0)
        {
          /* Open file. */
        }
      else if (selection == 1)
        {
          /* Save file.  */
        }
      else if (selection == 2)
        {
          /* Save as.  */
          
        }
      else if (selection == 3)
        {
          /* Quit.  */
          endCDK();
          return;
        }
      else if (selection == 200)
        {
          /* About recnav.  */
          recnav_cmd_about ();
        }
    }
}

int
main (int argc, char *argv[])
{
  int res = 0;
  rec_db_t db;

  recutl_init ("recnav");
  
  /* Parse arguments.  */
  recnav_parse_args (argc, argv);

  /* Get the input data.  */
  db = recutl_build_db (argc, argv);
  if (!db)
    {
      res = 1;
    }

  /* Process the data.  */
  recnav_navigate (db);

  rec_db_destroy (db);
  
  return res;
}

/* End of recnav.c */

