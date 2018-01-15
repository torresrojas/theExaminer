/*
Compilar con

cc -o EX5040 EX5040.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX5040:                                */
/*                                         */
/*    Lista Esquemas, Pre-Exámenes y       */
/*    Exámenes                             */
/*    The Examiner 0.2                     */
/*    25 de Julio del 2012                 */
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

/*************/
/* GTK stuff */
/*************/
GtkBuilder*builder; 
GError    * error;

GtkWidget *window1                = NULL;

GtkWidget *FR_botones             = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_terminar            = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Connect_widgets();
void Fin_de_Programa        (GtkWidget *widget, gpointer user_data);
void Fin_Ventana            (GtkWidget *widget, gpointer user_data);
void Imprime_listado        (GtkWidget *widget, gpointer user_data);
void Init_Fields            ();
void Interface_Coloring     ();
void on_BN_print_clicked    (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked (GtkWidget *widget, gpointer user_data);
void on_WN_ex5040_destroy   (GtkWidget *widget, gpointer user_data);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define CODIGO_SIZE  6
/*----------------------------------------------------------------------------*/

/***************/
/* M A I N ( ) */
/***************/
int main(int argc, char *argv[]) 
{
  GdkColor color;

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX5040.glade", &error))
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

          gtk_widget_show (window1);  

          /* start the event loop */
          gtk_main ();
	 }
     }

  return 0;
}

void Connect_widgets()
{
  window1              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex5040"));
  FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  BN_print               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_botones, GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_ACTIVE, &color);
}

void Imprime_listado (GtkWidget *widget, gpointer user_data)
{
   PGresult *res, *res_aux;
   FILE * Archivo_Latex;
   int N_esquemas, N_preexamenes;
   char hilera_antes [2000], hilera_despues [2000];

   char PG_command [1000];
   int i, N_registros;
   int N_preguntas, N_nuevas;

   Archivo_Latex = fopen ("EX5040.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, onecolumn, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");

   fprintf (Archivo_Latex, "\\begin{document}\n");

   sprintf (hilera_antes, "\\title{Esquemas, Pre-Exámenes y Exámenes}\n");
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\\maketitle\n\n");


   /*********************/
   /*                   */
   /*  E S Q U E M A S  */
   /*                   */
   /*********************/
   fprintf (Archivo_Latex, "\\begin{center}\n");
   fprintf (Archivo_Latex, "\\begin{longtable}{|c|l|l|}\n");
   fprintf (Archivo_Latex, "\\caption*{\\textbf{Esquemas Registrados}}\\\\\n");

   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{C\\'odigo} & \\textbf{Materia} & \\textbf{Descripci\\'{o}n} \\\\ \\hline \\hline \\hline \\hline \\hline \n");
   fprintf (Archivo_Latex, "\\endfirsthead\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{C\\'odigo} & \\textbf{Materia} & \\textbf{Descripci\\'{o}n} \\\\ \\hline \\hline \\hline \\hline \\hline \n");
   fprintf (Archivo_Latex, "\\endhead\n");

   sprintf (PG_command,"SELECT codigo_esquema, materia, descripcion_esquema, descripcion_materia from ex_esquemas, bd_materias where codigo_materia = materia and codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_esquema");
   res = PQEXEC(DATABASE, PG_command);
   N_esquemas = PQntuples(res);

   for (i=0; i< N_esquemas; i++)
       {
	if (i % 2) fprintf (Archivo_Latex,"\\rowcolor{green}\n");
	sprintf (hilera_antes,"%s & %s -  %.55s & %.55s \\\\ \\hline \\hline \\hline", 
                               PQgetvalue (res, i, 0), PQgetvalue (res, i, 1), PQgetvalue (res, i, 3),
		               PQgetvalue (res, i, 2));
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s\n", hilera_despues);
       }
   PQclear(res);

   fprintf (Archivo_Latex, "\\hline\n");

   fprintf (Archivo_Latex, "\\end{longtable}\n");
   fprintf (Archivo_Latex, "\\end{center}\n");

   /****************************/
   /*                          */
   /*  P R E - E X A M E N E S */
   /*                          */
   /****************************/
   fprintf (Archivo_Latex, "\\begin{center}\n");
   fprintf (Archivo_Latex, "\\begin{longtable}{|c|c|l|l|}\n");
   fprintf (Archivo_Latex, "\\caption*{\\textbf{Pre-ex\\'amenes Registrados}}\\\\\n");

   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{C\\'odigo} & \\textbf{Esquema} & \\textbf{Materia} & \\textbf{Descripci\\'{o}n} \\\\ \\hline \\hline \\hline \\hline \\hline \n");
   fprintf (Archivo_Latex, "\\endfirsthead\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{C\\'odigo} & \\textbf{Esquema} & \\textbf{Materia} & \\textbf{Descripci\\'{o}n} \\\\ \\hline \\hline \\hline \\hline \\hline \n");
   fprintf (Archivo_Latex, "\\endhead\n");

   sprintf (PG_command,"SELECT codigo_pre_examen, esquema, materia, descripcion_pre_examen, descripcion_materia from ex_pre_examenes, ex_esquemas, bd_materias where esquema = codigo_esquema and codigo_materia = materia and codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_pre_examen");
   res = PQEXEC(DATABASE, PG_command);
   N_esquemas = PQntuples(res);

   for (i=0; i< N_esquemas; i++)
       {
	if (i % 2) fprintf (Archivo_Latex,"\\rowcolor{green}\n");
	sprintf (hilera_antes,"%s & %s & %s - %.55s & %.55s \\\\ \\hline \\hline \\hline", 
                               PQgetvalue (res, i, 0), PQgetvalue (res, i, 1), 
                               PQgetvalue (res, i, 2), PQgetvalue (res, i, 4),
        		       PQgetvalue (res, i, 3));
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s\n", hilera_despues);
       }

   fprintf (Archivo_Latex, "\\hline\n");

   fprintf (Archivo_Latex, "\\end{longtable}\n");
   fprintf (Archivo_Latex, "\\end{center}\n");

   /********************/
   /*                  */
   /*  E X A M E N E S */
   /*                  */
   /********************/
   fprintf (Archivo_Latex, "\\begin{center}\n");
   fprintf (Archivo_Latex, "\\begin{longtable}{|c|c|l|c|l|}\n");
   fprintf (Archivo_Latex, "\\caption*{\\textbf{Ex\\'amenes Registrados}}\\\\\n");

   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{C\\'odigo} & \\textbf{Pre-examen} & \\textbf{Materia} & \\textbf{Descripci\\'{o}n} \\\\ \\hline \\hline \\hline \\hline \\hline \n");
   fprintf (Archivo_Latex, "\\endfirsthead\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{C\\'odigo} & \\textbf{Pre-examen} & \\textbf{Materia} & \\textbf{Descripci\\'{o}n} \\\\ \\hline \\hline \\hline \\hline \\hline \n");
   fprintf (Archivo_Latex, "\\endhead\n");

   sprintf (PG_command,"SELECT codigo_examen, pre_examen, materia, descripcion, descripcion_materia from ex_examenes, bd_materias where codigo_materia = materia and codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_examen");
   res = PQEXEC(DATABASE, PG_command);
   N_esquemas = PQntuples(res);

   for (i=0; i< N_esquemas; i++)
       {
	if (i % 2) fprintf (Archivo_Latex,"\\rowcolor{green}\n");
	sprintf (hilera_antes,"%s & %s & %s - %.55s & %.55s \\\\ \\hline \\hline \\hline", 
                               PQgetvalue (res, i, 0), PQgetvalue (res, i, 1),
                               PQgetvalue (res, i, 2), PQgetvalue (res, i, 4),
        		       PQgetvalue (res, i, 3));
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s\n", hilera_despues);
       }

   fprintf (Archivo_Latex, "\\hline\n");

   fprintf (Archivo_Latex, "\\end{longtable}\n");
   fprintf (Archivo_Latex, "\\end{center}\n");

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX5040", 1, NULL, 0.0, 0.0, NULL, NULL);

   gtk_widget_grab_focus (BN_print);
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
void on_WN_ex5040_destroy (GtkWidget *widget, gpointer user_data) 
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

