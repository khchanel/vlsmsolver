/*********************************************************
 * ui_gtk.c  --- Main function of GTK UI VLSM Solver program
 * Copyright (C) 2011 Nelson Ka Hei Chan <khcha.n.el@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
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
 ********************************************************/

#include <stdio.h>
#include <gtk/gtk.h>
#include "gtk_main_window.h"


int main (int argc, char ** argv)
{
  gtk_init(&argc, &argv);

  MainWindow *win = mw_new();
  mw_show(win);
  printf("# Entering GTK main loop... \n");
  gtk_main();
  printf("# Exited from GTK main loop\n");
  mw_free(win);
  printf("# Program Terminated.\n");
  return 0;
}
