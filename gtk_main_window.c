/*********************************************************
 * GTK MainWindow for VLSM Solver
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
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "gtk_main_window.h"
#include "vlsm.h"

/*** PROTOTYPES ***/
static void vlsm_update_view (MainWindow *);
/******************/


/**
 * Function to handle the destroy event from
 * window manager ("x")
 */
static void mw_destroy(GtkWidget *widget, gpointer data)
{
  gtk_main_quit();
}

/**
 * the function for adr_in/smask_in/mask_in/adr_button when activated
 * update other network information with new input
 */
static void update_net (GtkWidget *widget, MainWindow *mw)
{
  const gchar *input;
  gchar ipstr[IPV4_STRLEN];
  ipv4_t addr, bcast, fhost, lhost;
  unsigned char  smask;
  unsigned long  uhosts;

  /* obtain smask */
  input = gtk_entry_get_text (GTK_ENTRY(mw->smask_in));
  smask = (unsigned char) atoi (input);

  /* parse adr_in  input, create ipv4_t update adr_in with parsed addr*/
  input = gtk_entry_get_text (GTK_ENTRY(mw->adr_in));
  strtoipv4(addr,input);
  ipv4tonet(addr,addr,smask); /* convert to network address */
  ipv4tostr(ipstr,addr);
  gtk_entry_set_text(GTK_ENTRY(mw->adr_in),ipstr);


  /* calculate new network info */
  calbroadcast (bcast,addr,smask);
  calfirst (fhost,addr,smask);
  callast (lhost,addr,smask);

  /* update network info */
  ipv4tostr (ipstr,bcast);
  gtk_label_set_text(GTK_LABEL(mw->bcast_ip),ipstr);
  ipv4tostr (ipstr,fhost);
  gtk_label_set_text(GTK_LABEL(mw->fhost_ip),ipstr);
  ipv4tostr (ipstr,lhost);
  gtk_label_set_text(GTK_LABEL(mw->lhost_ip),ipstr);
  uhosts = caluhosts(smask);
  sprintf(ipstr,"%lu",uhosts);
  gtk_label_set_text(GTK_LABEL(mw->uhost),ipstr);
}

/**
 * update mask_in with new smask_in
 */
static void smask_in_changed_handler (GtkEntry *entry, MainWindow *mw)
{
  ipv4_t dmask;
  unsigned char smask;
  ipv4str_t ipstr;
  unsigned long uhosts;
  const gchar *input = gtk_entry_get_text (entry);
  smask = (unsigned char)atoi (input);
  if (smask > 30) {
    smask = 30;
    gtk_entry_set_text(entry, "30");
  }
  masktodot(dmask,smask);
  ipv4tostr(ipstr,dmask);
  gtk_entry_set_text(GTK_ENTRY(mw->mask_in),ipstr);
  uhosts = caluhosts(smask);
  sprintf(ipstr,"%lu",uhosts);
  gtk_label_set_text(GTK_LABEL(mw->uhost),ipstr);
}

/**
 * handler for mask_in activate event
 */
static void mask_in_activate_handler (GtkEntry *entry, MainWindow *mw)
{
  ipv4str_t ipstr;
  ipv4_t dmask;
  unsigned char smask;
  const gchar *input = gtk_entry_get_text(entry);
  strtoipv4(dmask,input);
  ipv4tostr(ipstr,dmask);
  gtk_entry_set_text(entry,ipstr);
  smask = masktoslash(dmask);
  sprintf(ipstr,"%d",smask);
  gtk_entry_set_text(GTK_ENTRY(mw->smask_in),ipstr);
  update_net(GTK_WIDGET(entry),mw);
}


/**
 * Input filter (accept only numberic char and .)
 * copied from GTK+ FAQ with a few changes
 */
static void insert_text_handler (GtkEntry    *entry,
                                 const gchar *text,
                                 gint         length,
                                 gint        *position,
                                 gpointer     data)
{
  GtkEditable *editable = GTK_EDITABLE(entry);
  int i, count=0;
  gchar *result = g_new (gchar, length);

  for (i=0; i < length; i++) {
    if (!(isdigit(text[i]) || text[i] == '.' || text[i] == ',') )
      continue;
    result[count++] = text[i];
  }

  if (count > 0) {
    g_signal_handlers_block_by_func (G_OBJECT (editable),
                                     G_CALLBACK (insert_text_handler),
                                     data);
    gtk_editable_insert_text (editable, result, count, position);
    g_signal_handlers_unblock_by_func (G_OBJECT (editable),
                                       G_CALLBACK (insert_text_handler),
                                       data);
  }
  g_signal_stop_emission_by_name (G_OBJECT (editable), "insert_text");

  g_free (result);
}


/**
 * Handler for host_in  clicked signal
 * call vlsm_update_view()
 */
static void host_in_activate_handler (GtkEntry *entry, MainWindow *mw)
{
  vlsm_update_view(mw);
}

/**
 * Handler for subnet_button  clicked signal
 * call vlsm_update_view()
 */
static void subnet_button_clicked_handler (GtkButton *button, MainWindow *mw)
{
  vlsm_update_view(mw);
}

/**
 * Handler for reset_button clicked signal
 * clear mw->subnet_store & host_in
 */
static void reset_button_clicked_handler (GtkButton *button, MainWindow *mw)
{
  gtk_list_store_clear(mw->subnet_store);
  gtk_entry_set_text(GTK_ENTRY(mw->host_in), "");
  gtk_label_set_text(GTK_LABEL(mw->status_label), "Programmed by Nelson Chan");
}


/**
 * Function that perform VLSM and update ListStore/Treeview
 * intended to be called by other handlers
 */
static void vlsm_update_view(MainWindow *mw)
{
  unsigned long    * n_arr = NULL;
  const gchar      * input;
  char             * tmpstr;
  network_t        * subnets;
  int                i,
                     n_arrc; // element counter for n_arr
  unsigned char      smask;
  guint16            in_length;
  ipv4_t             addr,
                     tmpaddr;
  networkstr_t       ipstr[NUM_COLS];
  GtkTreeIter        iter;
  /* reset store */
  gtk_list_store_clear(mw->subnet_store);

  /* activate the network entires */
  update_net(NULL,mw);
  /* parse base network */
  input = gtk_entry_get_text (GTK_ENTRY(mw->adr_in));
  strtoipv4(addr,input);
  input = gtk_entry_get_text (GTK_ENTRY(mw->smask_in));
  smask = (unsigned char) atoi (input);

  /* parse host_in to unsigned long array */
  input = gtk_entry_get_text(GTK_ENTRY(mw->host_in));
  in_length = gtk_entry_get_text_length(GTK_ENTRY(mw->host_in));
  if (in_length == 0) return; // do nothing
  in_length += 1; // count the \0
  tmpstr = (char *)g_malloc(in_length * sizeof(char) +10); //add room for COL_NAME text and \0
  sprintf(tmpstr,"%s","");

  // really parse now...
  n_arrc = 0;
  for (i=0;i<in_length;i++) {
    if (input[i] == ',' || input[i] == '.' || input[i] == '\0') {
      /* we got ',' or '.' or \0 , now parse the number before it*/
      if (strlen(tmpstr) == 0) continue; // this happen when user enter like ",,123"
      n_arrc += 1;
      n_arr = (unsigned long *) g_realloc (n_arr,sizeof(unsigned long) * n_arrc);
      n_arr[n_arrc-1] = (unsigned long) atol (tmpstr);
      printf("# input: %s  -> %lu\n",tmpstr, n_arr[n_arrc-1]);
      sprintf(tmpstr,"%s","");
    } else {
      sprintf(tmpstr,"%s%c",tmpstr,input[i]);
    }
  }
  printf("# n_arrc=%d\n",n_arrc);
  if (n_arrc == 0) return; // no number entered, nothing to do

  /* call vlsm() */
  subnets = (network_t *) g_malloc (sizeof(network_t) * n_arrc );
  i = vlsm (subnets, addr, smask, n_arr, n_arrc); // we could make use of error code from vlsm()
  if (i == -2) {
    gtk_label_set_markup(GTK_LABEL(mw->status_label), "<span color='red'>Error: no host or too many hosts to address for the base network</span>");
    return;
  }
  for (i=0;i<n_arrc;i++)
      print_network(&subnets[i]);


  /* append new data to store */
  for (i=0;i<n_arrc;i++) {
    // Name
    if (n_arr[i] == 0) {
      //sprintf(tmpstr,"ERROR");
      continue;  // hide it
    } else {
      sprintf(tmpstr,"Subnet %lu", n_arr[i]); // tmpstr be subnet name
    }
    // Addr
    ipv4tostr (ipstr[COL_ADDR], subnets[i].addr);
    sprintf (ipstr[COL_ADDR],"%s /%u",ipstr[COL_ADDR],subnets[i].mask);
    // DMASK
    masktodot (tmpaddr,subnets[i].mask);
    ipv4tostr (ipstr[COL_DMASK], tmpaddr);
    // FHOST
    calfirst(tmpaddr,subnets[i].addr, subnets[i].mask);
    ipv4tostr(ipstr[COL_FHOST], tmpaddr);
    // LHOST
    callast (tmpaddr, subnets[i].addr, subnets[i].mask);
    ipv4tostr (ipstr[COL_LHOST], tmpaddr);
    // BCAST
    calbroadcast (tmpaddr, subnets[i].addr, subnets[i].mask);
    ipv4tostr (ipstr[COL_BCAST], tmpaddr);

    /* set data */
    gtk_list_store_append (mw->subnet_store, &iter);
    gtk_list_store_set (mw->subnet_store, &iter,
                        COL_NAME,  tmpstr,
                        COL_ADDR,  ipstr[COL_ADDR],
                        COL_DMASK, ipstr[COL_DMASK],
                        COL_FHOST, ipstr[COL_FHOST],
                        COL_LHOST, ipstr[COL_LHOST],
                        COL_BCAST, ipstr[COL_BCAST],
                        -1);
    //
  }

  /* free memory */
  g_free(tmpstr);
  g_free(n_arr);
  g_free(subnets);

  /* misc */
  gtk_label_set_text(GTK_LABEL(mw->status_label), "VLSM successul!");
}

/**
 * Here we build the main layout of the window. That
 * means initializing main box and frames. And then
 * pack them into the box.
 */
static void m_box_build(MainWindow *mw)
{
  mw->m_box = gtk_vbox_new(FALSE,10);
  mw->net_frame = gtk_frame_new("Network");
  mw->subnet_frame = gtk_frame_new("VLSM Subnetting");

  gtk_box_pack_start(GTK_BOX(mw->m_box), mw->net_frame, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(mw->m_box), mw->subnet_frame, TRUE, TRUE, 2);
  gtk_container_add (GTK_CONTAINER(mw->window), mw->m_box);
}

/**
 * Here we make all the purely for layout widgets.
 * That means all the boxes and the table.
 */
static void skel_box_build(MainWindow *mw)
{
  mw->net_table = gtk_table_new(3, 5, FALSE);
  mw->subnet_box = gtk_vbox_new(FALSE, 5);

  mw->adr_box = gtk_hbox_new(FALSE, 0);
  mw->mask_box = gtk_hbox_new(FALSE, 0);
  mw->nhost_box = gtk_hbox_new(FALSE, 0);

  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->adr_box, 2, 3, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->mask_box, 2, 3, 1, 2);

  gtk_box_pack_start(GTK_BOX(mw->subnet_box), mw->nhost_box, FALSE, FALSE, 0);
  gtk_container_add(GTK_CONTAINER(mw->net_frame), mw->net_table);
  gtk_container_add(GTK_CONTAINER(mw->subnet_frame), mw->subnet_box);
}

/**
 * Here we make all the Lables and put them in
 * Their respective boxes or table place
 */
static void label_build(MainWindow *mw)
{
  mw->adr_label = gtk_label_new("Net Address:");
  mw->mask_label = gtk_label_new("Mask:");
  mw->bcast_label = gtk_label_new("Broadcast:");
  mw->fhost_label = gtk_label_new("First Host:");
  mw->lhost_label = gtk_label_new("Last Host:");
  mw->div_label = gtk_label_new("/");
  mw->uhost_label = gtk_label_new("Usable:");
  mw->nhost_label = gtk_label_new("Subnets (separate by comma):");
  mw->uhost = gtk_label_new("254");
  mw->fhost_ip = gtk_label_new("192.168.0.1");
  mw->lhost_ip = gtk_label_new("192.168.0.254");
  mw->bcast_ip = gtk_label_new("192.168.0.255");
  mw->status_label = gtk_label_new("Programmed by Nelson Chan");

  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->adr_label, 0, 1, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->mask_label, 0, 1, 1, 2);
  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->fhost_label, 0, 1, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->lhost_label, 0, 1, 3, 4);
  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->bcast_label, 0, 1, 4, 5);

  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->fhost_ip, 1, 2, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->lhost_ip, 1, 2, 3, 4);
  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->bcast_ip, 1, 2, 4, 5);

  gtk_box_pack_start(GTK_BOX(mw->adr_box), mw->div_label, FALSE, FALSE, 4);
  gtk_box_pack_start(GTK_BOX(mw->mask_box), mw->uhost_label, FALSE, FALSE, 4);
  gtk_box_pack_start(GTK_BOX(mw->mask_box), mw->uhost, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(mw->nhost_box), mw->nhost_label, FALSE, FALSE, 0);

  gtk_box_pack_end(GTK_BOX(mw->m_box), mw->status_label, FALSE, FALSE, 0);
}

/**
 * Building and placing Entry boxes
 */
static void input_build(MainWindow *mw)
{
  mw->adr_in = gtk_entry_new_with_max_length(IPV4_STRLEN-1);
  gtk_entry_set_text (GTK_ENTRY(mw->adr_in), "192.168.0.0");
  mw->smask_in = gtk_entry_new_with_max_length(2);
  gtk_entry_set_text (GTK_ENTRY(mw->smask_in), "24");
  mw->mask_in = gtk_entry_new_with_max_length(IPV4_STRLEN-1);
  gtk_entry_set_text (GTK_ENTRY(mw->mask_in), "255.255.255.0");
  mw->host_in = gtk_entry_new();

  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->adr_in, 1, 2, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(mw->net_table), mw->mask_in, 1, 2, 1, 2);

  gtk_box_pack_start(GTK_BOX(mw->adr_box), mw->smask_in, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(mw->nhost_box), mw->host_in, TRUE, TRUE, 5);
}

/**
 * Build and place buttons
 */
static void button_build(MainWindow *mw)
{
  mw->subnet_button = gtk_button_new_with_label("VLSM");
  mw->reset_button = gtk_button_new_with_label("Reset");
  mw->adr_button = gtk_button_new_with_label("Enter");

  gtk_box_pack_end(GTK_BOX(mw->nhost_box), mw->reset_button, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(mw->nhost_box), mw->subnet_button, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(mw->adr_box), mw->adr_button, TRUE, TRUE, 10);
}

/**
 * Build and place the Tree View widget within a scrolled window
 * and then placed in its respective place inside the subnet_box
 */
static void tree_build(MainWindow *mw)
{
  /* init the store */
  mw->subnet_store = gtk_list_store_new (NUM_COLS,
                                         G_TYPE_STRING,
                                         G_TYPE_STRING,
                                         G_TYPE_STRING,
                                         G_TYPE_STRING,
                                         G_TYPE_STRING,
                                         G_TYPE_STRING
                                         );

  /* init the view */
  GtkCellRenderer *cell_renderer;
  mw->scroll_win = gtk_scrolled_window_new(NULL, NULL);
  mw->subnet_tree = gtk_tree_view_new();
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(mw->scroll_win),
                                        mw->subnet_tree);
  gtk_box_pack_start(GTK_BOX(mw->subnet_box), mw->scroll_win, TRUE, TRUE, 0);

  /* view's columns */
  cell_renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(mw->subnet_tree),
                                               -1,
                                               "Name",
                                               cell_renderer,
                                               "text", COL_NAME,
                                               NULL);
  cell_renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(mw->subnet_tree),
                                               -1,
                                               "Address",
                                               cell_renderer,
                                               "text", COL_ADDR,
                                               NULL);
  cell_renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(mw->subnet_tree),
                                               -1,
                                               "Mask",
                                               cell_renderer,
                                               "text", COL_DMASK,
                                               NULL);
  cell_renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(mw->subnet_tree),
                                               -1,
                                               "First Host",
                                               cell_renderer,
                                               "text", COL_FHOST,
                                               NULL);
  cell_renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(mw->subnet_tree),
                                               -1,
                                               "Last Host",
                                               cell_renderer,
                                               "text", COL_LHOST,
                                               NULL);
  cell_renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(mw->subnet_tree),
                                               -1,
                                               "Broadcast",
                                               cell_renderer,
                                               "text", COL_BCAST,
                                               NULL);
  /* connect tree model */
  gtk_tree_view_set_model(GTK_TREE_VIEW(mw->subnet_tree), GTK_TREE_MODEL(mw->subnet_store));
}

/**
 * Here you can connect all your signals to the diffrent widgets.
 * In here all objects should already be made, so if you have to
 * do something very spesific, it has to be done before this
 */
static void connect_signals(MainWindow *mw)
{
  g_signal_connect(mw->window, "destroy", G_CALLBACK(mw_destroy), NULL);
  /* network section */
  g_signal_connect(G_OBJECT(mw->adr_in), "activate", G_CALLBACK(update_net), mw);
  g_signal_connect(G_OBJECT(mw->smask_in), "activate", G_CALLBACK(update_net),mw);
  g_signal_connect(G_OBJECT(mw->mask_in), "activate",G_CALLBACK(mask_in_activate_handler),mw);
  g_signal_connect(G_OBJECT(mw->smask_in),"changed",G_CALLBACK(smask_in_changed_handler),mw);
  g_signal_connect(G_OBJECT(mw->adr_in), "insert_text",G_CALLBACK(insert_text_handler),NULL);
  g_signal_connect(G_OBJECT(mw->mask_in),"insert_text",G_CALLBACK(insert_text_handler),NULL);
  g_signal_connect(G_OBJECT(mw->smask_in),"insert_text",G_CALLBACK(insert_text_handler),NULL);
  g_signal_connect(G_OBJECT(mw->adr_button),"clicked",G_CALLBACK(update_net),mw);
  /* subnetting section */
  g_signal_connect(G_OBJECT(mw->host_in),"insert_text",G_CALLBACK(insert_text_handler),NULL);
  g_signal_connect(G_OBJECT(mw->host_in),"activate",G_CALLBACK(host_in_activate_handler),mw);
  g_signal_connect(G_OBJECT(mw->subnet_button),"clicked",G_CALLBACK(subnet_button_clicked_handler),mw);
  g_signal_connect(G_OBJECT(mw->reset_button),"clicked",G_CALLBACK(reset_button_clicked_handler),mw);
}

/**
 * This function only serves the purpose of making aesthetic
 * changes. That means size requests, scrollwindow policies etc.
 */
static void aesthetics(MainWindow *mw)
{
  gtk_window_set_title(GTK_WINDOW(mw->window), "VLSM Solver "VLSM_VERSION);
  gtk_window_set_default_size ( GTK_WINDOW(mw->window), 720,480);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mw->scroll_win),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(mw->scroll_win, 300, 200);
  gtk_widget_set_size_request(mw->smask_in, 30, -1);
  gtk_widget_set_size_request(mw->host_in, 200, -1);
  gtk_label_set_width_chars(GTK_LABEL(mw->uhost),10); /* 2^32 = 10 digits */
}

/**
 * This function will make and return a main window in its
 * initial state. Much like a constructor in OOP.
 */
MainWindow* mw_new()
{
  MainWindow *mw = g_malloc(sizeof(MainWindow)); // Make new MainWindow struct
  mw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL); // Make a pointer to a gtk
                                                    // Window.
  // Construct and place
  m_box_build(mw); // Main layout
  skel_box_build(mw); // Boxes and Tables
  label_build(mw); // Labels
  input_build(mw); // Text Entries
  button_build(mw); // Buttons
  tree_build(mw); // Scroll and Tree View

  connect_signals(mw); // Connect Signals to their respective widgets
  aesthetics(mw); // Make purly aesthetic changes

  return mw; // Return our created MainWindow.
}

/**
 * Show all the widgets in the window.
 */
void mw_show(MainWindow *mw)
{
  gtk_widget_show_all(mw->window);
}

/**
 * Free the space from the struct.
 */
void mw_free(MainWindow *mw)
{
  g_free(mw);
}
