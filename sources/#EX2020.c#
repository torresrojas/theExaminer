/*
Compilar con:

cc -o EX2020 EX2020.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX2020:                                */
/*                                         */
/*    Actualización Archivo de Personas    */
/*    (Autores o Profesores)               */
/*    The Examiner 0.0                     */
/*    7 de Diciembre del 2011              */
/*    Autor: Francisco J. Torres-Rojas     */
/*******************************************/
#include <stdlib.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <locale.h>
#include "examiner.h"

/***************************/
/* Postgres stuff          */
/***************************/
PGconn	 *DATABASE;
PGresult *res, *res_aux;

/*************/
/* GTK stuff */
/*************/
GtkBuilder*builder; 
GError    * error = NULL;

GtkWidget *window1                = NULL;
GtkWidget *window2                = NULL;

GtkComboBox *CB_autor             = NULL;
GtkWidget *EN_autor               = NULL;
GtkWidget *EN_nombre_autor        = NULL;
GtkWidget *EN_afiliacion          = NULL;

GtkWidget *FR_botones             = NULL;

GtkWidget *BN_print               = NULL;
GtkWidget *BN_save                = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_delete              = NULL;
GtkWidget *BN_terminar            = NULL;
GtkWidget *BN_ok                  = NULL;

GtkWidget *FR_update              = NULL;
GtkWidget *EB_update              = NULL;

/**************/
/* Prototipos */
/**************/
void Init_Fields();
void Cambio_Autor_Combo(GtkWidget *widget, gpointer user_data);
void Cambio_Autor(GtkWidget *widget, gpointer user_data);
void Connect_widgets();
void corrige_hilera (unsigned char * antes, unsigned char * despues);
void Autor_ya_existe (char * autor);
void Graba_Registro (GtkWidget *widget, gpointer user_data);
void Borra_Registro (GtkWidget *widget, gpointer user_data);
void Cambio_Nombre_Autor(GtkWidget *widget, gpointer user_data);
void Cambio_Afiliacion(GtkWidget *widget, gpointer user_data);
void Fin_de_Programa (GtkWidget *widget, gpointer user_data);
void Fin_Ventana (GtkWidget *widget, gpointer user_data);
void Imprime_listado (GtkWidget *widget, gpointer user_data);
void Interface_Coloring ();
void on_WN_ex2020_destroy (GtkWidget *widget, gpointer user_data);
void on_BN_print_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_save_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_delete_clicked(GtkWidget *widget, gpointer user_data);
void on_EN_autor_activate(GtkWidget *widget, gpointer user_data);
void on_EN_nombre_autor_activate(GtkWidget *widget, gpointer user_data);
void on_EN_afiliacion_activate(GtkWidget *widget, gpointer user_data);
void on_CB_autor_changed(GtkWidget *widget, gpointer user_data);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define AUTOR_SIZE      11
#define NOMBRE_SIZE     101
#define AFILIACION_SIZE 201

int REGISTRO_YA_EXISTE = 0;

int N_autores=0;

/*----------------------------------------------------------------------------*/

/***************/
/* M A I N ( ) */
/***************/
int main(int argc, char *argv[]) 
{
  /* make a connection to the database */
  DATABASE = EX_connect_data_base ();

  /* check to see that the backend connection was successfully made */
  if (PQstatus(DATABASE) == CONNECTION_BAD)
     {
      fprintf(stderr, "Connection to database failed.\n");
      fprintf(stderr, "%s", PQerrorMessage(DATABASE));
      PQfinish(DATABASE);
     }
  else
     {
      gtk_init(&argc, &argv);
      /* makes sure that numbers are formatted with '.' to separate decimals from integers - must be done after gkt_init() */
      setlocale (LC_NUMERIC, "en_US.UTF-8");
      carga_parametros_EXAMINER (&parametros, DATABASE);

      builder = gtk_builder_new ();
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX2020.glade", &error))
         {
          g_warning ("%s\n", error->message);
          g_error_free (error);
         }
      else
	 {
	  /* Conects functions to interface events */
          gtk_builder_connect_signals (builder, NULL);

	  /* Connects interface fields with memory variables */
	  Connect_widgets ();

	  /* The magic of color...  */
	  Interface_Coloring ();

	  /* Displays main window */
          gtk_widget_show (window1);  

          Init_Fields ();              

          /* start the event loop */
          gtk_main ();
	 }
     }

  return 0;
}

void Connect_widgets()
{
  window1         = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex2020"));
  window2         = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));
  CB_autor        = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_autor"));
  EN_autor        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_autor"));
  EN_nombre_autor = GTK_WIDGET (gtk_builder_get_object (builder, "EN_nombre_autor"));
  EN_afiliacion   = GTK_WIDGET (gtk_builder_get_object (builder, "EN_afiliacion"));
  FR_botones      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  BN_print        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_save         = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
  BN_delete       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_delete"));
  BN_terminar     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_undo         = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_ok           = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
  EB_update       = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
  FR_update       = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_botones,  GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(EN_autor,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_nombre_autor, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_afiliacion  , GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_delete,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,           GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_delete,       GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,                  GTK_STATE_ACTIVE, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,              GTK_STATE_NORMAL, &color);
  gdk_color_parse (FINISHED_WORK_FR    , &color);
  gtk_widget_modify_bg(FR_update,              GTK_STATE_NORMAL, &color);
}

void Init_Fields()
{
   int i;
   char hilera[210];

   gtk_entry_set_text(GTK_ENTRY(EN_autor)        ,"\0");
   gtk_entry_set_text(GTK_ENTRY(EN_nombre_autor), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_afiliacion)  , "\0");

   gtk_widget_set_visible (EN_autor,        FALSE);
   gtk_widget_set_visible (EN_nombre_autor, FALSE);
   gtk_widget_set_visible (GTK_WIDGET(CB_autor), TRUE);

   gtk_combo_box_set_active (CB_autor, -1);
   for (i=0; i < N_autores; i++) gtk_combo_box_remove_text (CB_autor, 0);

   gtk_combo_box_append_text(CB_autor, "*** Inserta Profesor(a) o Autor(a) ***");
   res = PQEXEC(DATABASE,"SELECT * from bd_personas order by codigo_persona"); 
   N_autores = PQntuples(res);

   for (i=0; i < N_autores; i++)
       {
	sprintf (hilera,"%s %s - %s", PQgetvalue (res, i, 0), PQgetvalue (res, i, 1), PQgetvalue (res, i, 2));
        gtk_combo_box_append_text(CB_autor, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_autor, -1);
   N_autores++;

   gtk_widget_set_can_focus(EN_autor       , FALSE);
   gtk_widget_set_can_focus(EN_nombre_autor, FALSE);
   gtk_widget_set_can_focus(EN_afiliacion  , FALSE);

   gtk_widget_set_sensitive(EN_autor       , 0);
   gtk_widget_set_sensitive(EN_nombre_autor, 0);
   gtk_widget_set_sensitive(EN_afiliacion  , 0);

   gtk_widget_set_sensitive(BN_save,   0);
   gtk_widget_set_sensitive(BN_undo,   1);
   gtk_widget_set_sensitive(BN_delete, 0);

   REGISTRO_YA_EXISTE = 0;

   gtk_widget_set_sensitive(GTK_WIDGET(CB_autor), 1);
   gtk_widget_grab_focus   (GTK_WIDGET(CB_autor));
}

void Cambio_Autor_Combo(GtkWidget *widget, gpointer user_data)
{
  int i, j, k;
  char codigo_persona[AUTOR_SIZE];
  char PG_command[200];

  gchar *autor;

  k = gtk_combo_box_get_active(CB_autor);

  gtk_widget_set_visible (EN_autor,        TRUE);
  gtk_widget_set_visible (EN_nombre_autor, TRUE);
  gtk_widget_set_visible (GTK_WIDGET(CB_autor), FALSE);

  if (k == 0)
     { /* Autor nuevo */
      gtk_widget_set_sensitive(GTK_WIDGET(CB_autor), 0);
      gtk_widget_set_sensitive(EN_autor, 1);
      gtk_widget_set_can_focus(EN_autor, TRUE);
      gtk_widget_grab_focus   (EN_autor);
     }
  else
     {
      gtk_widget_set_sensitive(BN_save,   1);

      autor = gtk_combo_box_get_active_text(CB_autor);

      i=0;
      while (autor[i] != ' ') 
	    {
	     codigo_persona[i] = autor[i];
	     i++;
	    }
      codigo_persona[i]='\0';
      sprintf (PG_command,"SELECT * from bd_personas where codigo_persona = '%s'", codigo_persona);
      res = PQEXEC(DATABASE, PG_command);

      gtk_widget_set_sensitive(GTK_WIDGET(CB_autor), 0);
      gtk_entry_set_text(GTK_ENTRY(EN_autor),        PQgetvalue (res, 0, 0));
      gtk_entry_set_text(GTK_ENTRY(EN_nombre_autor), PQgetvalue (res, 0, 1));
      gtk_entry_set_text(GTK_ENTRY(EN_afiliacion),   PQgetvalue (res, 0, 2));

      Autor_ya_existe (codigo_persona);

      g_free (autor);
     }
}

void Cambio_Autor(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *autor = NULL;
  char PG_command[200];


  autor = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);

  if (autor[0])
     {
      gtk_widget_set_can_focus(EN_autor    , FALSE);
      gtk_widget_set_sensitive(BN_save,   1);

      sprintf (PG_command,"SELECT * from bd_personas where codigo_persona = '%s'", autor);
      res = PQEXEC(DATABASE, PG_command);
      if (PQntuples(res))
	 {
          gtk_entry_set_text(GTK_ENTRY(EN_nombre_autor),PQgetvalue (res, 0, 1));
          gtk_entry_set_text(GTK_ENTRY(EN_afiliacion),  PQgetvalue (res, 0, 2));

	  Autor_ya_existe (autor);
	 }
      else
	{ /* AUTOR NO EXISTE */
	  REGISTRO_YA_EXISTE = 0;

          gtk_widget_set_sensitive(BN_delete, 0);

          gtk_widget_set_sensitive(EN_nombre_autor, 1);
          gtk_widget_set_sensitive(EN_afiliacion, 1);
          gtk_widget_set_can_focus(EN_nombre_autor, TRUE);
          gtk_widget_set_can_focus(EN_afiliacion, TRUE);

          gtk_widget_grab_focus (EN_nombre_autor);
	 }
      PQclear(res);
     }

  g_free (autor);
}

void Autor_ya_existe (char * autor)
{
  char PG_command[500];
  PGresult *res_aux;

  REGISTRO_YA_EXISTE = 1;

  gtk_widget_set_sensitive(EN_autor,        1);
  gtk_widget_set_sensitive(EN_nombre_autor, 1);
  gtk_widget_set_sensitive(EN_afiliacion  , 1);
  gtk_widget_set_can_focus(EN_autor, FALSE);
  gtk_widget_set_can_focus(EN_nombre_autor, TRUE);
  gtk_widget_set_can_focus(EN_afiliacion, TRUE);

  gtk_widget_set_sensitive(BN_delete, 1);

  sprintf (PG_command,"SELECT * from bd_texto_ejercicios where autor = '%s'", autor);
  res_aux = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res_aux)) gtk_widget_set_sensitive(BN_delete, 0);
  PQclear(res_aux);

  sprintf (PG_command,"SELECT * from EX_examenes where profesor = '%s'", autor);
  res_aux = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res_aux)) gtk_widget_set_sensitive(BN_delete, 0);
  PQclear(res_aux);

  gtk_widget_grab_focus   (EN_nombre_autor);
}

void Graba_Registro (GtkWidget *widget, gpointer user_data)
{
   gchar *autor, *nombre_autor, *afiliacion;
   char PG_command [1000];

   autor        = gtk_editable_get_chars(GTK_EDITABLE(EN_autor)            , 0, -1);
   nombre_autor = gtk_editable_get_chars(GTK_EDITABLE(EN_nombre_autor), 0, -1);
   afiliacion   = gtk_editable_get_chars(GTK_EDITABLE(EN_afiliacion)   , 0, -1);

   if (REGISTRO_YA_EXISTE)
      {
       sprintf (PG_command,"UPDATE bd_personas SET nombre='%.100s', afiliacion ='%.200s' where codigo_persona = '%.10s'", nombre_autor, afiliacion, autor);
       res = PQEXEC(DATABASE, PG_command); PQclear(res);
      }
   else
      {
       sprintf (PG_command,"INSERT into bd_personas values ('%.10s','%.100s','%.200s')",	autor, nombre_autor, afiliacion);
       res = PQEXEC(DATABASE, PG_command); PQclear(res);
      }

  g_free (autor);
  g_free (nombre_autor);
  g_free (afiliacion);

  carga_parametros_EXAMINER (&parametros, DATABASE);
  if (parametros.report_update)
      {
       gtk_widget_set_sensitive(window1, 0);
       gtk_widget_show (window2);

       if (parametros.timeout)
	  {
           while (gtk_events_pending ()) gtk_main_iteration ();
           catNap(parametros.timeout);
           gtk_widget_hide (window2);
           gtk_widget_set_sensitive(window1, 1);
	  }
      }

  Init_Fields ();
}

void Borra_Registro (GtkWidget *widget, gpointer user_data)
{
   char PG_command [500];
   gchar *autor;

   autor = gtk_editable_get_chars(GTK_EDITABLE(EN_autor), 0, -1);

   sprintf (PG_command,"DELETE from bd_personas where codigo_persona = '%.10s'", autor);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   g_free (autor);

   carga_parametros_EXAMINER (&parametros, DATABASE);
   if (parametros.report_update)
      {
       gtk_widget_set_sensitive(window1, 0);
       gtk_widget_show (window2);

       if (parametros.timeout)
	  {
           while (gtk_events_pending ()) gtk_main_iteration ();
           catNap(parametros.timeout);
           gtk_widget_hide (window2);
           gtk_widget_set_sensitive(window1, 1);
	  }
      }

   Init_Fields ();
}

void Cambio_Nombre_Autor(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_afiliacion);
}

void Cambio_Afiliacion(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_nombre_autor);
}

void Fin_de_Programa (GtkWidget *widget, gpointer user_data)
{
   /* close the connection to the database and cleanup */
   PQfinish(DATABASE);
   gtk_main_quit ();
   exit (0);
}

void Fin_Ventana (GtkWidget *widget, gpointer user_data)
{
  /* close the connection to the database and cleanup */

  PQfinish(DATABASE);
  gtk_main_quit ();
  exit (0);
}

void Imprime_listado (GtkWidget *widget, gpointer user_data)
{
   FILE * Archivo_Latex;
   char PG_command [1000];
   int i, N_registros;
   char hilera_antes [2000], hilera_despues [2000];

   Archivo_Latex = fopen ("EX2020.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, onecolumn, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");
   fprintf (Archivo_Latex, "\\title{Archivo de Profesores o Autores de Ejercicios}\n");
   fprintf (Archivo_Latex, "\\maketitle\n\n");

   sprintf (PG_command,"SELECT * from bd_personas order by codigo_persona");
   res = PQEXEC(DATABASE, PG_command);
   N_registros = PQntuples(res);

   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\begin{center}\n");
   fprintf (Archivo_Latex, "\\begin{longtable}{||l|l|l||}\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Codigo} & \\textbf{Nombre} & \\textbf{Afiliaci\\'{o}n}\\\\ \\hline \\hline\n");
   fprintf (Archivo_Latex, "\\endfirsthead\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Codigo} & \\textbf{Nombre} & \\textbf{Afiliaci\\'{o}n}\\\\ \\hline \\hline\n");
   fprintf (Archivo_Latex, "\\endhead\n");

   for (i=0; i<N_registros; i++)
       {
	sprintf (hilera_antes,"%s & %s & %s \\\\ \\hline", PQgetvalue (res, i, 0),PQgetvalue (res, i, 1),PQgetvalue (res, i, 2));
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s\n", hilera_despues);
       }

   fprintf (Archivo_Latex, "\\end{longtable}\n");
   fprintf (Archivo_Latex, "\\end{center}\n");

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX2020", 1, NULL, 0.0, 0.0, NULL, NULL);

   Init_Fields ();
}

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex2020_destroy (GtkWidget *widget, gpointer user_data) 
{
  Fin_Ventana (widget, user_data);
}

void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data)
{
  Fin_de_Programa (widget, user_data);
}

void on_BN_print_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_listado (widget, user_data);
}

void on_BN_undo_clicked(GtkWidget *widget, gpointer user_data)
{
  Init_Fields ();
}

void on_BN_save_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Registro (widget, user_data);
}

void on_BN_delete_clicked(GtkWidget *widget, gpointer user_data)
{
  Borra_Registro (widget, user_data);
}

void on_EN_autor_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Autor (widget, user_data);
}

void on_EN_nombre_autor_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Nombre_Autor (widget, user_data);
}

void on_EN_afiliacion_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Afiliacion (widget, user_data);
}

void on_CB_autor_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * autor;

  autor = gtk_combo_box_get_active_text(CB_autor);

  if (autor)
     {
      g_free (autor);
      Cambio_Autor_Combo (widget, user_data);
     }
  else
     g_free (autor);
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
}
