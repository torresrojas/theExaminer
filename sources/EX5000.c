/*
Compilar con

cc -o EX5000 EX5000.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX5000:                                */
/*                                         */
/*    Lista Materias, Temas y Subtemas     */
/*    The Examiner 0.0                     */
/*    28 de Junio 2011                     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    23 de Enero del 2012                 */
/*******************************************/
#include <stdlib.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <locale.h>
#include <string.h>
#include "examiner.h"

/***************************/
/* Postgres stuff          */
/***************************/
PGconn	 *DATABASE;

/*************/
/* GTK stuff */
/*************/
GtkBuilder*builder; 
GError    * error;

GtkWidget *window1                = NULL;
GtkWidget *EB_reporte = NULL;
GtkWidget *FR_reporte = NULL;

GtkComboBox *CB_materia           = NULL;
GtkComboBox *CB_tema              = NULL;

GtkWidget *FR_botones             = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_terminar            = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Cambio_Materia         (GtkWidget *widget, gpointer user_data);
void Cambio_Tema            (GtkWidget *widget, gpointer user_data);
void Connect_widgets        ();
void Extrae_codigo          (char * hilera, char * codigo);
void Fin_de_Programa        (GtkWidget *widget, gpointer user_data);
void Fin_Ventana            (GtkWidget *widget, gpointer user_data);
void Imprime_listado        (GtkWidget *widget, gpointer user_data);
void Init_Fields            ();
void Interface_Coloring ();
void on_BN_print_clicked    (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked (GtkWidget *widget, gpointer user_data);
void on_CB_materia_changed  (GtkWidget *widget, gpointer user_data);
void on_CB_tema_changed     (GtkWidget *widget, gpointer user_data);
void on_WN_ex5000_destroy   (GtkWidget *widget, gpointer user_data);
void Subtemas               (FILE * Archivo_Latex, char * materia, char * tema);
void Temas_Subtemas         (FILE * Archivo_Latex, char * materia);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define CODIGO_SIZE  6

int N_materias = 0;
int N_temas    = 0;
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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX5000.glade", &error))
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

	  /* Despliega ventana */
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
  int i;
  
  window1              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex5000"));

  EB_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "EB_reporte"));
  FR_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reporte"));

  CB_materia             = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_materia"));
  CB_tema                = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_tema"));

  FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  BN_print               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
}

void Interface_Coloring ()
{
  GdkColor color;
  
  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_reporte, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_botones, GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_ACTIVE, &color);
}

void Init_Fields()
{
   int i;
   PGresult *res;
   char hilera[300];

   gtk_combo_box_set_active (CB_materia, -1);
   for (i=0; i < N_materias; i++) gtk_combo_box_remove_text (CB_materia, 0);

   gtk_combo_box_set_active (CB_tema, -1);
   for (i=0; i < N_temas; i++) gtk_combo_box_remove_text (CB_tema, 0);

   res = PQEXEC(DATABASE,"SELECT codigo_materia, descripcion_materia from bd_materias where codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_materia"); 
   N_materias = PQntuples(res);
   for (i=0; i < N_materias; i++)
       {
	sprintf (hilera,"%s %s",PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
	gtk_combo_box_append_text(CB_materia, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_materia, -1);

   gtk_widget_set_sensitive(BN_print, 0);

   gtk_widget_grab_focus (GTK_WIDGET(CB_materia));
}

void Cambio_Materia(GtkWidget *widget, gpointer user_data)
{
  int i, k;
  gchar *materia;
  char PG_command  [1000];
  PGresult *res;
  char hilera [300];
  char Codigo_materia [CODIGO_MATERIA_SIZE + 1];

  gtk_widget_set_sensitive(BN_print, 1);

  gtk_combo_box_set_active (CB_tema, -1);
  for (i=0; i<N_temas; i++)
      {
       gtk_combo_box_remove_text (CB_tema, 0);
      }

  materia = gtk_combo_box_get_active_text(CB_materia);

  Extrae_codigo (materia, Codigo_materia);

  if (strcmp(materia,CODIGO_VACIO) != 0)
     {
      gtk_combo_box_append_text(CB_tema, "*** Todos los Temas ***");
      sprintf (PG_command,"SELECT codigo_tema, descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema != '          ' and codigo_subtema = '          ' order by orden", Codigo_materia); 
      res = PQEXEC(DATABASE, PG_command);
      N_temas = PQntuples(res)+1;
      for (i=1; i < N_temas; i++)
          {
  	   sprintf (hilera,"%s %s",PQgetvalue (res, i-1, 0), PQgetvalue (res, i-1, 1));
	   gtk_combo_box_append_text(CB_tema, hilera);
          }
      PQclear(res);
      gtk_combo_box_set_active (CB_tema, 0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_tema), 1);
     }
  else
     {
      gtk_combo_box_append_text(CB_tema, "*** Todos los Temas ***");
      N_temas = 1;
      gtk_combo_box_set_active (CB_tema, 0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_tema), 0);
     }

  g_free (materia);
}

void Cambio_Tema(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (GTK_WIDGET(CB_materia));
}

void Extrae_codigo (char * hilera, char * codigo)
{
  int i;

  i = 0;

  while (hilera[i] != ' ') codigo [i] = hilera[i++];

  codigo[i] = '\0';
}

void Imprime_listado (GtkWidget *widget, gpointer user_data)
{
   PGresult *res, *res_aux;
   FILE * Archivo_Latex;
   gchar *materia, *tema;
   char Codigo_materia [CODIGO_MATERIA_SIZE + 1];
   char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
   char PG_command [1000];
   int i, N_registros;
   char hilera_antes [2000], hilera_despues [2000];
   int N_preguntas, N_nuevas;
   int k_tema;

   k_tema    = gtk_combo_box_get_active(CB_tema);

   materia = gtk_combo_box_get_active_text(CB_materia);
   tema    = gtk_combo_box_get_active_text(CB_tema);

   Extrae_codigo (materia, Codigo_materia);
   Extrae_codigo (tema   , Codigo_tema);

   Archivo_Latex = fopen ("EX5000.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[10pt,journal, onecolumn, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");

   if (!k_tema)
      Temas_Subtemas (Archivo_Latex, Codigo_materia);
   else
      Subtemas       (Archivo_Latex, Codigo_materia, Codigo_tema);

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX5000", 1, NULL, 0.0, 0.0, NULL, NULL);

   gtk_widget_grab_focus (GTK_WIDGET(CB_materia));

   g_free (materia);
   g_free (tema);
}

void Temas_Subtemas (FILE * Archivo_Latex, char * materia)
{
   PGresult *res, *res_tema, *res_tema_subtema, *res_aux;
   char PG_command [1000];
   char hilera_antes [2000], hilera_despues [2000];
   int i, j, N_temas, N_registros;
   int N_nuevas, N_preguntas;

   sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", materia);
   res = PQEXEC(DATABASE, PG_command);

   sprintf (hilera_antes, "\\title{Temas y Subtemas de %s}\n", PQgetvalue (res, 0, 3));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\\maketitle\n\n");

   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\begin{center}\n");
   fprintf (Archivo_Latex, "\\begin{longtable}{|c|c|c|l|r|r|c|}\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Materia} & \\textbf{Tema} & \\textbf{Subtema} & \\textbf{Descripci\\'{o}n} & \\textbf{Preg.} & \\textbf{Nuevas} & \\textbf{Desarrollo} \\\\ \\hline \\hline \\hline \\hline \\hline \n");
   fprintf (Archivo_Latex, "\\endfirsthead\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Materia} & \\textbf{Tema} & \\textbf{Subtema} & \\textbf{Descripci\\'{o}n} & \\textbf{Preg.} & \\textbf{Nuevas} & \\textbf{Desarrollo} \\\\ \\hline \\hline \\hline \\hline \\hline\n");
   fprintf (Archivo_Latex, "\\endhead\n");

   sprintf (PG_command,"SELECT N_estudiantes from bd_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas where materia = '%s' and codigo_consecutivo_ejercicio = texto_ejercicio and pregunta = codigo_unico_pregunta",materia);
   res_aux = PQEXEC(DATABASE, PG_command);
   N_preguntas = PQntuples(res_aux);
   PQclear(res_aux);

   sprintf (PG_command,"SELECT N_estudiantes from bd_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas where materia = '%s' and codigo_consecutivo_ejercicio = texto_ejercicio and pregunta = codigo_unico_pregunta and N_estudiantes = 0", materia);
   res_aux = PQEXEC(DATABASE, PG_command);
   N_nuevas = PQntuples(res_aux);
   PQclear(res_aux);
   fprintf (Archivo_Latex,"\\rowcolor{blue}\n");

   sprintf (hilera_antes,"{\\color{white} \\textbf{%s}} &  &  & {\\color{white} {\\small \\textbf{%.60s}}} & {\\color{white} %d} & {\\color{white} %d} & \\\\ \\hline \\hline \\hline ", PQgetvalue (res, 0, 0), PQgetvalue (res, 0, 3), N_preguntas, N_nuevas);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   PQclear(res);

   sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema != '          ' and codigo_subtema = '          ' order by orden", materia);
   res_tema = PQEXEC(DATABASE, PG_command);
   N_temas  = PQntuples(res_tema);

   for (i=0; i<N_temas; i++)
       {
  	sprintf (PG_command,"SELECT N_estudiantes from bd_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas where materia = '%s' and tema = '%s' and codigo_consecutivo_ejercicio = texto_ejercicio and pregunta = codigo_unico_pregunta",PQgetvalue (res_tema, i, 0),PQgetvalue (res_tema, i, 1));

        res_aux = PQEXEC(DATABASE, PG_command);
        N_preguntas = PQntuples(res_aux);
        PQclear(res_aux);

	sprintf (PG_command,"SELECT N_estudiantes from bd_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas where materia = '%s' and tema = '%s' and codigo_consecutivo_ejercicio = texto_ejercicio and pregunta = codigo_unico_pregunta and N_estudiantes = 0", PQgetvalue (res_tema, i, 0),PQgetvalue (res_tema, i, 1));

        res_aux = PQEXEC(DATABASE, PG_command);
        N_nuevas = PQntuples(res_aux);
        PQclear(res_aux);

        fprintf (Archivo_Latex, "\\hline \\hline\n");
        fprintf (Archivo_Latex,"\\rowcolor{cyan}\n");

	sprintf (hilera_antes,"{\\color{white} %s} & {\\color{white} %s} &  & {\\color{white} {\\small %.60s}} & {\\color{white} %d} & {\\color{white} %d} & \\\\ \\hline \\hline \\hline", PQgetvalue (res_tema, i, 0),PQgetvalue (res_tema, i, 1),
       		               PQgetvalue (res_tema, i, 3), N_preguntas, N_nuevas);
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s\n", hilera_despues);

        sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' order by orden",
                 PQgetvalue (res_tema, i, 0),PQgetvalue (res_tema, i, 1)); 
        res_tema_subtema = PQEXEC(DATABASE, PG_command);
        N_registros      = PQntuples(res_tema_subtema);
	for (j=0;j<N_registros;j++)
	    {
  	     sprintf (PG_command,"SELECT N_estudiantes from bd_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas where materia = '%s' and tema = '%s' and subtema = '%s' and codigo_consecutivo_ejercicio = texto_ejercicio and pregunta = codigo_unico_pregunta",
		      PQgetvalue (res_tema_subtema, j, 0),PQgetvalue (res_tema_subtema, j, 1),PQgetvalue (res_tema_subtema, j, 2));
             res_aux = PQEXEC(DATABASE, PG_command);
             N_preguntas = PQntuples(res_aux);
             PQclear(res_aux);

	     sprintf (PG_command,"SELECT N_estudiantes from bd_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas where materia = '%s' and tema = '%s' and subtema = '%s' and codigo_consecutivo_ejercicio = texto_ejercicio and pregunta = codigo_unico_pregunta and N_estudiantes = 0",
		      PQgetvalue (res_tema_subtema, j, 0),PQgetvalue (res_tema_subtema, j, 1),PQgetvalue (res_tema_subtema, j, 2));
             res_aux = PQEXEC(DATABASE, PG_command);
             N_nuevas = PQntuples(res_aux);
             PQclear(res_aux);

	     sprintf (hilera_antes,"%s & %s & %s & {\\small %.60s} & %d & %d & \\\\ \\hline", 
                      PQgetvalue (res_tema_subtema, j, 0),PQgetvalue (res_tema_subtema, j, 1),PQgetvalue (res_tema_subtema, j, 2),PQgetvalue (res_tema_subtema, j, 3),
                      N_preguntas, N_nuevas);
             hilera_LATEX (hilera_antes, hilera_despues);
             fprintf (Archivo_Latex, "%s\n", hilera_despues);
	    }
        PQclear(res_tema_subtema);
       }

   PQclear(res_tema);

   fprintf (Archivo_Latex, "\\hline\n");

   fprintf (Archivo_Latex, "\\end{longtable}\n");
   fprintf (Archivo_Latex, "\\end{center}\n");

}

void Subtemas (FILE * Archivo_Latex, char * materia, char * tema)
{
   PGresult *res, *res_tema_subtema, *res_aux;
   char PG_command [1000];
   char hilera_antes [2000], hilera_despues [2000];
   int i, j, N_temas, N_registros;
   int N_nuevas, N_preguntas;

   sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", materia);
   res_aux = PQEXEC(DATABASE, PG_command);

   sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", materia, tema);
   res     = PQEXEC(DATABASE, PG_command);

   sprintf (hilera_antes, "\\title{Subtemas de %s\\\\(%s)}\n",PQgetvalue (res, 0, 3), PQgetvalue (res_aux, 0, 0));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\\maketitle\n\n");
   PQclear(res_aux);

   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\begin{center}\n");
   fprintf (Archivo_Latex, "\\begin{longtable}{|c|c|c|l|r|r|c|}\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Materia} & \\textbf{Tema} & \\textbf{Subtema} & \\textbf{Descripci\\'{o}n} & \\textbf{Preg.} & \\textbf{Nuevas} & \\textbf{Desarrollo} \\\\ \\hline \\hline \\hline \\hline \\hline \n");
   fprintf (Archivo_Latex, "\\endfirsthead\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Materia} & \\textbf{Tema} & \\textbf{Subtema} & \\textbf{Descripci\\'{o}n} & \\textbf{Preg.} & \\textbf{Nuevas} & \\textbf{Desarrollo} \\\\ \\hline \\hline \\hline \\hline \\hline\n");
   fprintf (Archivo_Latex, "\\endhead\n");

   sprintf (PG_command,"SELECT N_estudiantes from bd_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas where materia = '%s' and tema= '%s' and  codigo_consecutivo_ejercicio = texto_ejercicio and pregunta = codigo_unico_pregunta",materia, tema);
   res_aux = PQEXEC(DATABASE, PG_command);
   N_preguntas = PQntuples(res_aux);
   PQclear(res_aux);

   sprintf (PG_command,"SELECT N_estudiantes from bd_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas where materia = '%s' and tema = '%s' and codigo_consecutivo_ejercicio = texto_ejercicio and pregunta = codigo_unico_pregunta and N_estudiantes = 0", materia, tema);
   res_aux = PQEXEC(DATABASE, PG_command);
   N_nuevas = PQntuples(res_aux);
   PQclear(res_aux);
   fprintf (Archivo_Latex,"\\rowcolor{cyan}\n");

   sprintf (hilera_antes,"{\\color{white} %s} & {\\color{white} %s} &  & {\\color{white} {\\small %.60s}} & {\\color{white} %d} & {\\color{white} %d} & \\\\ \\hline \\hline \\hline ", materia, tema, PQgetvalue (res, 0, 3), N_preguntas, N_nuevas);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   PQclear(res);

   sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' order by orden", materia, tema);
   res_tema_subtema = PQEXEC(DATABASE, PG_command);
   N_registros      = PQntuples(res_tema_subtema);
   for (i=0;i<N_registros;i++)
       {
	sprintf (PG_command,"SELECT N_estudiantes from bd_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas where materia = '%s' and tema = '%s' and subtema = '%s' and codigo_consecutivo_ejercicio = texto_ejercicio and pregunta = codigo_unico_pregunta", 
                 materia, tema, PQgetvalue (res_tema_subtema, i, 2));
        res_aux = PQEXEC(DATABASE, PG_command);
        N_preguntas = PQntuples(res_aux);
        PQclear(res_aux);

	sprintf (PG_command,"SELECT N_estudiantes from bd_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas where materia = '%s' and tema = '%s' and subtema = '%s' and codigo_consecutivo_ejercicio = texto_ejercicio and pregunta = codigo_unico_pregunta and N_estudiantes = 0",
		 materia, tema, PQgetvalue (res_tema_subtema, i, 2));
        res_aux = PQEXEC(DATABASE, PG_command);
        N_nuevas = PQntuples(res_aux);
        PQclear(res_aux);

	sprintf (hilera_antes,"%s & %s & %s & {\\small %.60s} & %d & %d & \\\\ \\hline", 
		 materia, tema, PQgetvalue (res_tema_subtema, i, 2),PQgetvalue (res_tema_subtema, i, 3),
                 N_preguntas, N_nuevas);
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s\n", hilera_despues);
       }

   PQclear(res_tema_subtema);

   fprintf (Archivo_Latex, "\\hline\n");

   fprintf (Archivo_Latex, "\\end{longtable}\n");
   fprintf (Archivo_Latex, "\\end{center}\n");

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

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex5000_destroy (GtkWidget *widget, gpointer user_data) 
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

void on_CB_materia_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * materia;

  materia = gtk_combo_box_get_active_text(CB_materia);

  if (materia)
     {
      g_free (materia);
      Cambio_Materia (widget, user_data);
     }
  else
     g_free (materia);
}

void on_CB_tema_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * tema;

  tema = gtk_combo_box_get_active_text(CB_tema);

  if (tema)
     {
      g_free (tema);
      Cambio_Tema (widget, user_data);
     }
  else
     g_free (tema);
}
