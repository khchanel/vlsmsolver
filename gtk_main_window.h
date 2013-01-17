/*********************************************************
 * GTK MainWindow Header for VLSM Solver
 * Copyright (C) 2011 Nelson Ka Hei Chan <khcha.n.el@gmail.com>
 *
 * Contributor
 * Sindre Wetjen <sindre.w@gmail.com>
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

#ifndef MAIN_WIN_H
#define MAIN_WIN_H

#include <gtk/gtk.h>

/**
 * enum for subnet_tree
 */
enum
{
  COL_NAME = 0,
  COL_ADDR,
  //COL_SMASK,
  COL_DMASK,
  COL_FHOST,
  COL_LHOST,
  COL_BCAST,
  NUM_COLS
};

/**
 * This is the structure with all the variables and widgets in the
 * main window.
 */
typedef struct main_window
{
  GtkWidget *window; // The window top_level widget
  GtkWidget *m_box; // The top or "main box", it separates the network box
                    // from the subnet box.
  GtkWidget *net_frame, *subnet_frame; // Frames for the two sections "Network" and
                                       // VLSM Subnetting.
  GtkWidget *net_table, *adr_box, *mask_box; // Table for the network box,
                                             // and box for making " / " with
                                             // input behind (adr_box) and the
                                             // "Usable: num" layout
  GtkWidget *subnet_box, *nhost_box; // The box for separating the treeview from
                                     // the other "host number" box, and the put
                                     // everything related to "host number" box.

  // This section is just all of the label variables that are not supposed to change,
  // like Address: and such.
  GtkWidget *adr_label, *mask_label, *bcast_label, *fhost_label, *lhost_label;
  GtkWidget *div_label, *uhost_label, *nhost_label;
  GtkWidget *status_label;

  GtkWidget *uhost, *bcast_ip, *fhost_ip, *lhost_ip; // These are the labels that
                                                       // you should actually change
                                                       // when you want to display
                                                       // status output. The names
                                                       // should be self-explanatory.
  GtkWidget *adr_in, *smask_in, *mask_in, *host_in; // These are the entries (input box)
  GtkWidget *subnet_button, *reset_button, *adr_button;

  GtkWidget *scroll_win, *subnet_tree; // This is the Scrolled Window and the Tree View
                                       // widget
  GtkListStore *subnet_store; // the ListStore to hold subnet data, as model for subnet_tree

} MainWindow;

/**
 * This function will make and return a main window in its
 * initial state. Much like a constructor in OOP.
 */
MainWindow* mw_new();

/**
 * This function displays the windows with all its elements
 * on your screen.
 */
void mw_show(MainWindow *mw);

/**
 * This function should free all the variables and the
 * MainWindow object. Havent given it a lot of thought, so
 * it might need some revising.
 */
void mw_free(MainWindow *mw);

#endif
