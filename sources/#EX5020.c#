/*
Compilar con

cc -o EX5020 EX5020.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX5020:                                */
/*                                         */
/*    Análisis de la Base de Datos         */
/*    The Examiner 0.0                     */
/*    11 de Julio 2011                     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    4 de Abril del 2012                  */
/*******************************************/
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
GError    * error;

GtkWidget *window1                = NULL;
GtkComboBox *CB_materia           = NULL;
GtkComboBox *CB_tema              = NULL;

GtkWidget *EB_reporte = NULL;
GtkWidget *FR_reporte = NULL;

GtkWidget *EB_grafico             = NULL;
GtkWidget *FR_grafico             = NULL;
GtkToggleButton *RB_preguntas     = NULL;
GtkToggleButton *RB_usos          = NULL;
GtkToggleButton *CK_smooth        = NULL;

GtkWidget *FR_botones             = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_terminar            = NULL;

GtkWidget *BN_undo                = NULL;

GtkSpinButton *SP_resolucion = NULL;
GtkSpinButton *SP_color      = NULL;
GtkSpinButton *SP_rotacion   = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Analisis_por_materia   (FILE * Archivo_Latex);
void Analisis_por_subtema   (FILE * Archivo_Latex, gchar * materia, gchar * materia_descripcion, gchar * tema, gchar * tema_descripcion);
void Analisis_por_tema      (FILE * Archivo_Latex, gchar * materia, gchar * materia_descripcion);
void Cambio_Materia         (GtkWidget *widget, gpointer user_data);
void Cambio_Tema            (GtkWidget *widget, gpointer user_data);
long double CDF             (long double X, long double Media, long double Desv);
void Connect_Widgets();
void Dificultad_vs_Discriminacion (FILE * Archivo_Latex, char * materia, char * tema);
void Extrae_codigo          (char * hilera, char * codigo);
void Fin_de_Programa        (GtkWidget *widget, gpointer user_data);
void Fin_Ventana            (GtkWidget *widget, gpointer user_data);
void Grafico_y_Tabla        (FILE * Archivo_Latex, long double media, long double varianza);
void Imprime_listado        (GtkWidget *widget, gpointer user_data);
void Init_Fields            ();
void Interface_Coloring ();
void on_BN_print_clicked    (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked (GtkWidget *widget, gpointer user_data);
void on_CB_materia_changed  (GtkWidget *widget, gpointer user_data);
void on_CB_tema_changed     (GtkWidget *widget, gpointer user_data);
void on_WN_ex5020_destroy   (GtkWidget *widget, gpointer user_data);
void Prepara_Grafico_Normal (FILE * Archivo_Latex, long double media, long double varianza);
void Tabla_Probabilidades   (FILE * Archivo_Latex, long double media, long double varianza);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

int N_materias = 0;
int N_temas    = 0;

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX5020.glade", &error))
         {
          g_warning ("%s\n", error->message);
          g_error_free (error);
         }
      else
	 {
          /* Conects functions to interface events */
          gtk_builder_connect_signals (builder, NULL);


          /* Connects interface fields with memory variables */
	  Connect_Widgets ();
	  
	  /* The magic of color...  */
	  Interface_Coloring ();

          gtk_widget_show (window1);

          Init_Fields ();

          /* start the event loop */
          gtk_main();
	 }
     }

  return 0;
}

void Connect_Widgets()
{
  window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex5020"));

  CB_materia             = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_materia"));
  CB_tema                = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_tema"));

  EB_reporte             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_reporte"));
  FR_reporte             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reporte"));

  EB_grafico             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_grafico"));
  FR_grafico             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_grafico"));

  SP_resolucion = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_resolucion"));
  SP_color      = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_color"));
  SP_rotacion   = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_rotacion"));

  RB_preguntas           = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_preguntas"));
  RB_usos                = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_usos"));
  CK_smooth              = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_smooth"));

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
  gtk_widget_modify_bg(EB_reporte,   GTK_STATE_NORMAL,   &color);

  gdk_color_parse (SECONDARY_AREA, &color);
  gtk_widget_modify_bg(EB_grafico,     GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_grafico, GTK_STATE_NORMAL, &color);
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

   gtk_combo_box_append_text(CB_materia, CODIGO_VACIO);
   res = PQEXEC(DATABASE,"SELECT codigo_materia, descripcion_materia from bd_materias where codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_materia"); 
   N_materias = PQntuples(res)+1;
   for (i=1; i < N_materias; i++)
       {
	sprintf (hilera,"%s %s",PQgetvalue (res, i-1, 0), PQgetvalue (res, i-1, 1));
	gtk_combo_box_append_text(CB_materia, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_materia, 0);

   gtk_spin_button_set_value (SP_resolucion, 10);
   gtk_spin_button_set_value (SP_color,      7);
   gtk_spin_button_set_value (SP_rotacion,   30);
   gtk_toggle_button_set_active(CK_smooth, TRUE);

   gtk_widget_set_sensitive(BN_print, 1);

   gtk_widget_grab_focus (GTK_WIDGET(CB_materia));
}

void Cambio_Materia(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *materia;
  char PG_command  [1000];
  PGresult *res;
  char hilera [300];
  char Codigo_materia [CODIGO_MATERIA_SIZE + 1];

  gtk_combo_box_set_active (CB_tema, -1);
  for (i=0; i<N_temas; i++)
      {
       gtk_combo_box_remove_text (CB_tema, 0);
      }

  materia = gtk_combo_box_get_active_text(CB_materia);

  Extrae_codigo (materia, Codigo_materia);

  if (strcmp(materia,CODIGO_VACIO) != 0)
     {
      gtk_combo_box_append_text(CB_tema, CODIGO_VACIO);
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
      gtk_widget_set_sensitive (GTK_WIDGET(CB_tema),    1);
      gtk_widget_set_sensitive (GTK_WIDGET(FR_grafico), 1);
     }
  else
     {
      gtk_combo_box_append_text(CB_tema, CODIGO_VACIO);
      N_temas = 1;
      gtk_combo_box_set_active (CB_tema, 0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_tema),    0);
      gtk_widget_set_sensitive (GTK_WIDGET(FR_grafico), 0);
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

   char PG_command [2000];
   int i, N_preguntas;
   char hilera_antes [2000], hilera_despues [2000];
   char Correcta;

   materia = gtk_combo_box_get_active_text(CB_materia);
   tema    = gtk_combo_box_get_active_text(CB_tema);

   Extrae_codigo (materia, Codigo_materia);
   Extrae_codigo (tema   , Codigo_tema);

   Archivo_Latex = fopen ("EX5020.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, onecolumn, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");

   if (strcmp(materia,CODIGO_VACIO) == 0)
      {
       Analisis_por_materia (Archivo_Latex);
      }
   else
      {
       sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", Codigo_materia);
       res = PQEXEC(DATABASE, PG_command);
       if (strcmp(tema,CODIGO_VACIO) == 0)
	  {
	   Analisis_por_tema (Archivo_Latex, Codigo_materia, PQgetvalue (res, 0, 0));
	  }
       else
	  {
	   sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", Codigo_materia, Codigo_tema);
           res_aux = PQEXEC(DATABASE, PG_command);
	   Analisis_por_subtema (Archivo_Latex, Codigo_materia, PQgetvalue (res, 0, 0), Codigo_tema, PQgetvalue (res_aux, 0, 0));
           PQclear(res_aux);
	  }
       PQclear(res);
      }

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX5020", 1, NULL, 0.0, 0.0, NULL, NULL);

   if (strcmp(materia,CODIGO_VACIO) != 0) system ("rm EX5020*.pdf");

   g_free (materia);
   g_free (tema);
}

void Analisis_por_materia (FILE * Archivo_Latex)
{
  PGresult *res, *res_aux;
  char hilera_antes [1000], hilera_despues [1000];
  int Total_materias;
  int Total_preguntas;
  int Total_preguntas_materia;
  char PG_command [1000];
  int i;

  sprintf (hilera_antes, "\\title{Análisis por Materia}");
  hilera_LATEX (hilera_antes, hilera_despues);
  fprintf (Archivo_Latex, "%s\n", hilera_despues);
  fprintf (Archivo_Latex, "\\maketitle\n\n");

  sprintf (PG_command,"SELECT * from bd_ejercicios, bd_texto_preguntas where texto_ejercicio = codigo_consecutivo_ejercicio");
  res = PQEXEC(DATABASE, PG_command);
  Total_preguntas = PQntuples(res);
  PQclear(res);

  sprintf (PG_command,"SELECT DISTINCT codigo_materia, descripcion_materia from bd_materias, bd_ejercicios, bd_texto_preguntas where materia = codigo_materia and texto_ejercicio = codigo_consecutivo_ejercicio and codigo_tema = '          ' and codigo_subtema = '          ' order by descripcion_materia");
  res = PQEXEC(DATABASE, PG_command);
  Total_materias = PQntuples(res);

  if (!Total_materias)
     fprintf (Archivo_Latex, "No hay datos registrados\n\n");
  else
     {
      fprintf (Archivo_Latex, "\\begin{table}[H]\n");
      fprintf (Archivo_Latex, "\\begin{center}\n");
      fprintf (Archivo_Latex, "\\begin{tabular}{|c|l|r|r|}\n");
      fprintf (Archivo_Latex, "\\hline\n");
      fprintf (Archivo_Latex, "\\textbf{C\\'{o}digo} & \\textbf{Descripci\\'{o}n} & \\textbf{Cantidad} & \\textbf{Porcentaje} \\\\\n");
      fprintf (Archivo_Latex, "\\hline\\hline\n");

      for (i=0; i < Total_materias; i++)
          {
           sprintf (PG_command,"SELECT * from bd_texto_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and materia = '%s'",PQgetvalue (res, i, 0));
           res_aux = PQEXEC(DATABASE, PG_command);
           Total_preguntas_materia = PQntuples(res_aux);
           PQclear(res_aux);

           sprintf (hilera_antes,"%s & %s & %d & %6.2Lf \\\\ \\hline", PQgetvalue (res, i, 0), PQgetvalue (res, i, 1), Total_preguntas_materia, (long double) Total_preguntas_materia / (long double) Total_preguntas * 100.0);
           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
          }

      fprintf (Archivo_Latex, "\\hline\n");
      fprintf (Archivo_Latex, "\\hline\n");
      fprintf (Archivo_Latex,"  &  & \\textbf{%3d} & 100.00 \\\\ \\hline", Total_preguntas);

      fprintf (Archivo_Latex, "\\end{tabular}\n");
      fprintf (Archivo_Latex, "\\end{center}\n");
      fprintf (Archivo_Latex, "\\end{table}\n");
     }

  PQclear(res);
}

void Analisis_por_tema (FILE * Archivo_Latex, gchar * materia, gchar * materia_descripcion)
{
  PGresult *res, *res_aux;
  FILE * Archivo_Datos, *Archivo_gnuplot;
  char hilera_antes [1000], hilera_despues [1000];
  int Total_temas;
  int Total_preguntas;
  int Total_preguntas_tema;
  char PG_command [1000];
  int i, j;
  int Total_nuevas, nuevas;
  long double media, varianza, desviacion;
  long double media_tema, varianza_tema, desviacion_tema;
  char comando[1000];

  sprintf (hilera_antes, "\\title{Análisis por Temas de %s}", materia_descripcion);
  hilera_LATEX (hilera_antes, hilera_despues);
  fprintf (Archivo_Latex, "%s\n", hilera_despues);
  fprintf (Archivo_Latex, "\\maketitle\n\n");

  sprintf (PG_command,"SELECT * from bd_ejercicios, bd_texto_preguntas where texto_ejercicio = codigo_consecutivo_ejercicio and materia = '%s'", materia);
  res = PQEXEC(DATABASE, PG_command);
  Total_preguntas = PQntuples(res);
  PQclear(res);

  sprintf (PG_command,"SELECT DISTINCT codigo_tema, descripcion_materia, orden from bd_materias, bd_ejercicios, bd_texto_preguntas  where codigo_materia = '%s' and codigo_subtema = '          ' and codigo_tema != '          ' and materia = codigo_materia and tema = codigo_tema and texto_ejercicio = codigo_consecutivo_ejercicio order by orden", materia);
  res = PQEXEC(DATABASE, PG_command);
  Total_temas = PQntuples(res);

  if (!Total_temas)
     fprintf (Archivo_Latex, "No hay preguntas registradas para esta materia\n\n");
  else
     {
      Archivo_Datos   = fopen ("EX5020.dat","w");

      fprintf (Archivo_Latex, "\\begin{table}[H]\n");
      fprintf (Archivo_Latex, "\\begin{center}\n");
      fprintf (Archivo_Latex, "\\begin{tabular}{|l|l|r|r|r|r|c|c|c|}\n");
      fprintf (Archivo_Latex, "\\hline\n");
      fprintf (Archivo_Latex, "\\textbf{Descripci\\'{o}n} & \\textbf{C\\'{o}digo} & \\textbf{Cantidad} & \\textbf{Porcentaje} & \\textbf{Nuevas} & \\textbf{Media} & $\\sigma^2$ & $\\sigma$ &\\textbf{P}(\\textit{nota} $\\geq$ 67.5)  \\\\\n");
      fprintf (Archivo_Latex, "\\hline\\hline\n");

      Total_nuevas = 0;
      media = varianza = 0.0;
      for (i=0; i < Total_temas; i++)
          {
           sprintf (PG_command,"SELECT dificultad, varianza_dificultad, frecuencia from bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and codigo_unico_pregunta = pregunta and materia = '%s' and tema = '%s'",materia, PQgetvalue (res, i, 0));
           res_aux = PQEXEC(DATABASE, PG_command);
           Total_preguntas_tema = PQntuples(res_aux);

	   nuevas = 0;
           media_tema = varianza_tema = 0.0;
           for (j=0; j < Total_preguntas_tema; j++)
	       {
                media_tema    += atof(PQgetvalue (res_aux, j, 0));
                varianza_tema += atof(PQgetvalue (res_aux, j, 1));
		if (!atoi(PQgetvalue(res_aux, j, 2))) nuevas++;
	       }

           PQclear(res_aux);

           media        += media_tema;
           varianza     += varianza_tema;
	   Total_nuevas += nuevas;

	   /* Se escalan la media y varianza por tema para que sea un número entre 0 y 100 */

           media_tema      =  media_tema * (100.0/Total_preguntas_tema);
           varianza_tema   =  varianza_tema * (100.0/Total_preguntas_tema) * (100.0/Total_preguntas_tema);

           desviacion_tema =  sqrt(varianza_tema);
 
           sprintf (hilera_antes,"%.55s & %s & %3d & %6.2Lf & %d & %6.2Lf & %6.2Lf & %6.2Lf & %7.6Lf\\\\ \\hline", PQgetvalue (res, i, 1), PQgetvalue (res, i, 0), Total_preguntas_tema, (long double) Total_preguntas_tema / (long double) Total_preguntas * 100.0, nuevas, media_tema, varianza_tema, desviacion_tema,1.0-CDF(67.5, media_tema, desviacion_tema));

           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
           fprintf(Archivo_Datos,"%s %5d %5d\n",  PQgetvalue (res, i, 0), Total_preguntas_tema-nuevas, nuevas);
          }

      fprintf (Archivo_Latex, "\\hline\n");
      fprintf (Archivo_Latex, "\\hline\n");

      /* Se escala la media y varianza por materia para que sea un número entre 0 y 100 */
      media      =  media    * (100.0/Total_preguntas);
      varianza   =  varianza * (100.0/Total_preguntas) * (100.0/Total_preguntas);
      desviacion =  sqrt(varianza);

      fprintf (Archivo_Latex,"  &  & \\textbf{%3d} & 100.00 & %d & %6.2Lf & %6.2Lf & %6.2Lf & %7.6Lf\\\\ \\hline", Total_preguntas, Total_nuevas, media, varianza, desviacion, 1.0-CDF(67.5, media, desviacion));
      fprintf (Archivo_Latex, "\\end{tabular}\n");
      fprintf (Archivo_Latex, "\\end{center}\n");
      fprintf (Archivo_Latex, "\\end{table}\n");
      fclose (Archivo_Datos);
     }

  if (Total_temas >= 1)
     {
      Archivo_gnuplot = fopen ("EX5020.gp","w");
      fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 10\n");
      fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
      fprintf (Archivo_gnuplot, "set size 1.0, 0.7\n");
      fprintf (Archivo_gnuplot, "set grid ytics\n");
      fprintf (Archivo_gnuplot, "set output \"EX5020x.eps\"\n");

      fprintf (Archivo_gnuplot, "set xlabel \"Tema\"\n");
      fprintf (Archivo_gnuplot, "set ylabel \"Cantidad de Preguntas\"\n");
      fprintf (Archivo_gnuplot, "set yrange [0:]\n");

      fprintf (Archivo_gnuplot, "set xrange [-1:%d]\n", Total_temas);
      fprintf (Archivo_gnuplot, "set xtics 0, 1\n");
      fprintf (Archivo_gnuplot, "set xtics border in scale 1,0.5 nomirror rotate by -45  offset character 0, 0, 0\n");

      fprintf (Archivo_gnuplot, "set xtics (");
      for (i=0; i < Total_temas; i++)
          {
	   fprintf (Archivo_gnuplot,"\"%s\" %d", PQgetvalue (res, i, 0),i);
           if (i == (Total_temas - 1))
	      fprintf (Archivo_gnuplot,")\n ");
	   else
	      fprintf (Archivo_gnuplot,", ");
	  }

      fprintf (Archivo_gnuplot, "set style fill solid 1.0 border -1\n");
      fprintf (Archivo_gnuplot, "set style histogram rowstacked\n");
      fprintf (Archivo_gnuplot, "set style data histograms\n");
      fprintf (Archivo_gnuplot, "set boxwidth 0.3\n");
      fprintf (Archivo_gnuplot, "plot \"EX5020.dat\" using 2 t \"Usadas\" lc 3 lt 1, \'\' using 3:xtic(1) t \"Nuevas\" lc 2 lt 1\n");

      fclose (Archivo_gnuplot);

      sprintf(comando, "%s EX5020.gp", parametros.gnuplot);
      system (comando);

      sprintf(comando, "mv EX5020.gp %s", parametros.ruta_gnuplot);
      system (comando);

      sprintf(comando, "%s EX5020x.eps", parametros.epstopdf);
      system (comando);

      system ("rm EX5020x.eps");
      system ("rm EX5020.dat");

      fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
      fprintf (Archivo_Latex, "\\centering\n");
      fprintf (Archivo_Latex, "\\includegraphics[scale=1.0]{EX5020x.pdf}\n");
      fprintf (Archivo_Latex, "\\end{figure}\n");
     }

  if (Total_temas) Grafico_y_Tabla (Archivo_Latex, media, varianza);

  if (Total_temas) Dificultad_vs_Discriminacion(Archivo_Latex, materia, CODIGO_VACIO);
}

void Dificultad_vs_Discriminacion (FILE * Archivo_Latex, char * materia, char * tema)
{
  int i, j, k;
  FILE * Archivo_Datos, *Archivo_gnuplot;
  int Niveles_Discriminacion;
  int Niveles_Dificultad;
  int Rotacion;
  int Color;
  int Smooth;
  int Frecuencia_total [11][21];
  char PG_command[1000];
  PGresult *res;
  int Total_preguntas;
  long double dificultad, discriminacion;
  int frecuencia;
  char comando[1000];
  char Hilera_Antes [2000], Hilera_Despues [2000];
  char * Colores [] = {"unset pm3d",
                       "set palette gray", 
                       "set palette gray negative", 
                       "set palette rgb 21,22,23",
                       "set palette rgb 34,35,36",
                       "set palette rgb 7,5,15",
                       "set palette rgb 3,11,6",
                       "set palette rgb 23,28,3",
                       "set palette rgb 33,13,10",
                       "set palette rgb 30,31,32"};

  Niveles_Dificultad = (int) gtk_spin_button_get_value_as_int (SP_resolucion);
  Niveles_Discriminacion = Niveles_Dificultad * 2;


  Color    = (int) gtk_spin_button_get_value_as_int (SP_color);
  Rotacion = (int) gtk_spin_button_get_value_as_int (SP_rotacion);

  
  Smooth = gtk_toggle_button_get_active(CK_smooth);

  for (i=0; i <= Niveles_Dificultad; i++)
      for (j=0; j <= Niveles_Discriminacion; j++)
	  Frecuencia_total [i][j] = 0;

  if (strcmp (tema, CODIGO_VACIO) == 0)
     sprintf (PG_command,"SELECT dificultad, point_biserial, frecuencia from bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and codigo_unico_pregunta = pregunta and materia = '%s'",materia);
  else
     sprintf (PG_command,"SELECT dificultad, point_biserial, frecuencia from bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and codigo_unico_pregunta = pregunta and materia = '%s' and tema = '%s'",materia, tema);

  res = PQEXEC(DATABASE, PG_command);
  Total_preguntas = PQntuples(res);

  Archivo_Datos = fopen ("EX5020p.dat","w");
  for (k=0; k <Total_preguntas; k++)
      {
       dificultad     = atof(PQgetvalue (res, k, 0));
       discriminacion = atof(PQgetvalue (res, k, 1));
       frecuencia     = atoi(PQgetvalue (res, k, 2));

       if (frecuencia)
	  {
           i = (int) round (dificultad * Niveles_Dificultad);
           j = (int) round ((discriminacion+1.0)/2 * Niveles_Discriminacion);

           if (gtk_toggle_button_get_active(RB_preguntas))
	      Frecuencia_total [i][j]++;
	   else
	      Frecuencia_total [i][j]+= frecuencia;
	  }
      }

  for (j=0; j <= Niveles_Discriminacion; j++)
      {
       for (i=0; i <= Niveles_Dificultad; i++)
 	   {
	     fprintf (Archivo_Datos, "%Lf %Lf %d\n",(long double) j/Niveles_Discriminacion*2.0 - 1.0, (long double) i/Niveles_Dificultad, Frecuencia_total[i][j]);
	   }
       fprintf (Archivo_Datos, "\n");
      }

  fclose (Archivo_Datos);

  Archivo_gnuplot = fopen ("EX5020p.gp","w");
      fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 12\n");
      fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
      fprintf (Archivo_gnuplot, "set pm3d\n");
      fprintf (Archivo_gnuplot, "set style line 100 lt 5 lw 0.5 lc 2\n");
      fprintf (Archivo_gnuplot, "set pm3d hidden3d 100\n");
      fprintf (Archivo_gnuplot, "%s\n", Colores[Color]);
      fprintf (Archivo_gnuplot, "set size 1.55, 1.0\n");
      fprintf (Archivo_gnuplot, "set xtics -1.0, %4.3Lf, 1.0\n",(long double)2.0/(Niveles_Discriminacion));
      fprintf (Archivo_gnuplot, "set ytics  0.0, %4.3Lf, 1.0\n",(long double)1.0/(Niveles_Dificultad));
      fprintf (Archivo_gnuplot, "set grid xtics\n");
      fprintf (Archivo_gnuplot, "set grid ytics\n");
      fprintf (Archivo_gnuplot, "set output \"EX5020p.eps\"\n");

      hilera_GNUPLOT ("set xlabel \"Discriminación\"\n", Hilera_Despues);
      fprintf (Archivo_gnuplot, "%s", Hilera_Despues);
      fprintf (Archivo_gnuplot, "set ylabel \"Dificultad\"\n");
      fprintf (Archivo_gnuplot, "set xyplane at -1\n");
      fprintf (Archivo_gnuplot, "set hidden3d\n");

      if (Smooth) fprintf (Archivo_gnuplot, "set dgrid3 %d,%d,gauss 0.15 0.15\n",Niveles_Dificultad+1,Niveles_Discriminacion+1);
      fprintf (Archivo_gnuplot, "set view 60,%d,1,1\n", Rotacion);
      fprintf (Archivo_gnuplot, "splot \"EX5020p.dat\" notitle with lines lt 1 lw 5 lc 2\n");
  fclose (Archivo_gnuplot);

  sprintf(comando, "%s EX5020p.gp", parametros.gnuplot);
  system (comando);

  sprintf(comando, "mv EX5020p.gp %s", parametros.ruta_gnuplot);
  system (comando);

  sprintf(comando, "%s EX5020p.eps", parametros.epstopdf);
  system (comando);

  system ("rm EX5020p.eps");

  fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
  fprintf (Archivo_Latex, "\\centering\n");
  fprintf (Archivo_Latex, "\\includegraphics[scale=1.0]{EX5020p.pdf}\n");

  hilera_LATEX ("\\caption*{\\textbf{Cruce entre Coeficiente de Discriminación ($r_{pb}$) y Dificultad ($p$)}}", Hilera_Despues);
  fprintf (Archivo_Latex, "%s\n\n", Hilera_Despues);
  fprintf (Archivo_Latex, "\\end{figure}\n\n");

  Archivo_gnuplot = fopen ("EX5020c.gp","w");
      fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 12\n");
      fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
      fprintf (Archivo_gnuplot, "set pm3d\n");
      fprintf (Archivo_gnuplot, "%s\n", Colores[Color]);
      fprintf (Archivo_gnuplot, "set size 1.55, 1.0\n");
      fprintf (Archivo_gnuplot, "set xtics -1.0, %4.3Lf, 1.0\n",(long double)2.0/(Niveles_Discriminacion));
      fprintf (Archivo_gnuplot, "set ytics  0.0, %4.3Lf, 1.0\n",(long double)1.0/(Niveles_Dificultad));
      fprintf (Archivo_gnuplot, "set output \"EX5020c.eps\"\n");

      hilera_GNUPLOT ("set xlabel \"Discriminación\"\n", Hilera_Despues);
      fprintf (Archivo_gnuplot, "%s", Hilera_Despues);
      fprintf (Archivo_gnuplot, "set ylabel \"Dificultad\"\n");
      fprintf (Archivo_gnuplot, "set view map\n");
      fprintf (Archivo_gnuplot, "set contour\n");
      fprintf (Archivo_gnuplot, "set key at -1.2, 1.0 left box lt 1\n");

      if (Smooth) fprintf (Archivo_gnuplot, "set dgrid3 %d,%d,gauss 0.15 0.15\n",Niveles_Dificultad+1,Niveles_Discriminacion+1);

      fprintf (Archivo_gnuplot, "splot \"EX5020p.dat\" notitle with lines lt 1 lw 5 lc 2\n");
  fclose (Archivo_gnuplot);

  sprintf(comando, "%s EX5020c.gp", parametros.gnuplot);
  system (comando);

  sprintf(comando, "mv EX5020c.gp %s", parametros.ruta_gnuplot);
  system (comando);

  sprintf(comando, "%s EX5020c.eps", parametros.epstopdf);
  system (comando);

  system ("rm EX5020c.eps");
  system ("rm EX5020p.dat");

  fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
  fprintf (Archivo_Latex, "\\centering\n");
  fprintf (Archivo_Latex, "\\includegraphics[scale=1.0]{EX5020c.pdf}\n");

  hilera_LATEX ("\\caption*{\\textbf{Líneas de Contorno del Cruce entre Coeficiente de Discriminación ($r_{pb}$) y Dificultad ($p$)}}", Hilera_Despues);
  fprintf (Archivo_Latex, "%s\n\n", Hilera_Despues);
  fprintf (Archivo_Latex, "\\end{figure}\n\n");

  PQclear(res);
}

void Analisis_por_subtema (FILE * Archivo_Latex, gchar * materia, gchar * materia_descripcion, gchar * tema, gchar * tema_descripcion)
{
  PGresult *res, *res_aux;
  FILE * Archivo_Datos, *Archivo_gnuplot;
  char hilera_antes [1000], hilera_despues [1000];
  int Total_subtemas;
  int Total_preguntas;
  int Total_preguntas_subtema;
  char PG_command [2000];
  int i, j;
  int nuevas, Total_nuevas;
  long double media, varianza, desviacion;
  long double media_subtema, varianza_subtema, desviacion_subtema;
  char comando[600];


  sprintf (hilera_antes, "\\title{Análisis por Subtemas de %s - %s}", materia_descripcion, tema_descripcion);
  hilera_LATEX (hilera_antes, hilera_despues);
  fprintf (Archivo_Latex, "%s\n", hilera_despues);
  fprintf (Archivo_Latex, "\\maketitle\n\n");

  sprintf (PG_command,"SELECT * from bd_ejercicios, bd_texto_preguntas where texto_ejercicio = codigo_consecutivo_ejercicio and materia = '%s' and tema = '%s'", materia, tema);
  res = PQEXEC(DATABASE, PG_command);
  Total_preguntas = PQntuples(res);
  PQclear(res);

  sprintf (PG_command,"SELECT DISTINCT codigo_subtema, descripcion_materia, orden from bd_materias, bd_ejercicios, bd_texto_preguntas  where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' and materia = codigo_materia and tema = codigo_tema and subtema = codigo_subtema and texto_ejercicio = codigo_consecutivo_ejercicio order by orden", materia, tema);
  res = PQEXEC(DATABASE, PG_command);
  Total_subtemas = PQntuples(res);

  if (!Total_subtemas)
     fprintf (Archivo_Latex, "No hay datos registrados para este tema\n\n");
  else
     {
      Archivo_Datos   = fopen ("EX5020.dat","w");

      fprintf (Archivo_Latex, "\\begin{table}[H]\n");
      fprintf (Archivo_Latex, "\\begin{center}\n");
      fprintf (Archivo_Latex, "\\begin{tabular}{|l|l|r|r|r|r|c|c|c|}\n");
      fprintf (Archivo_Latex, "\\hline\n");
      fprintf (Archivo_Latex, "\\textbf{Descripci\\'{o}n} & \\textbf{C\\'{o}digo} & \\textbf{Cantidad} & \\textbf{Porcentaje} & \\textbf{Nuevas} & \\textbf{Media} & $\\sigma^2$ & $\\sigma$ &\\textbf{P}(\\textit{nota} $\\geq$ 67.5)  \\\\\n");
      fprintf (Archivo_Latex, "\\hline\\hline\n");

      Total_nuevas = 0;
      media = varianza = 0.0;

      for (i=0; i < Total_subtemas; i++)
          {
           sprintf (PG_command,"SELECT dificultad, varianza_dificultad, frecuencia from bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and codigo_unico_pregunta = pregunta and materia = '%s' and tema = '%s' and subtema = '%s'",materia, tema, PQgetvalue (res, i, 0));
           res_aux = PQEXEC(DATABASE, PG_command);
           Total_preguntas_subtema = PQntuples(res_aux);

           nuevas = 0;
           media_subtema = varianza_subtema = 0.0;
           for (j=0; j < Total_preguntas_subtema; j++)
	       {
                media_subtema    += atof(PQgetvalue (res_aux, j, 0));
                varianza_subtema += atof(PQgetvalue (res_aux, j, 1));
		if (!atoi(PQgetvalue(res_aux, j, 2))) nuevas++;
	       }
           PQclear(res_aux);

           media        += media_subtema;
           varianza     += varianza_subtema;
	   Total_nuevas += nuevas;

	   /* Se escala la media y varianza por subtema para que sea un número entre 0 y 100 */
           media_subtema      =  media_subtema * (100.0/Total_preguntas_subtema);
           varianza_subtema   =  varianza_subtema * (100.0/Total_preguntas_subtema) * (100.0/Total_preguntas_subtema);
           desviacion_subtema =  sqrt(varianza_subtema);

           sprintf (hilera_antes,"%s & %s & %3d & %6.2Lf & %d & %6.2Lf & %6.2Lf & %6.2Lf & %7.6Lf\\\\ \\hline", PQgetvalue (res, i, 1), PQgetvalue (res, i, 0), Total_preguntas_subtema, (long double) Total_preguntas_subtema / (long double) Total_preguntas * 100.0, nuevas, media_subtema, varianza_subtema, desviacion_subtema,1.0-CDF(67.5, media_subtema, desviacion_subtema));
           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf(Archivo_Latex, "%s\n\n", hilera_despues);
           fprintf(Archivo_Datos,"%s %d %d\n",  PQgetvalue (res, i, 0), Total_preguntas_subtema - nuevas, nuevas);
          }
      fprintf (Archivo_Latex, "\\hline\n");
      fprintf (Archivo_Latex, "\\hline\n");

      /* Se escala la media y varianza por tema para que sea un número entre 0 y 100 */
      media      =  media    * (100.0/Total_preguntas);
      varianza   =  varianza * (100.0/Total_preguntas) * (100.0/Total_preguntas);
      desviacion =  sqrt(varianza);

      fprintf (Archivo_Latex,"  &  & \\textbf{%3d} & 100.00 & %d & %6.2Lf & %6.2Lf & %6.2Lf & %7.6Lf\\\\ \\hline", Total_preguntas, Total_nuevas, media, varianza, desviacion, 1.0-CDF(67.5, media, desviacion));

      fprintf (Archivo_Latex, "\\end{tabular}\n");
      fprintf (Archivo_Latex, "\\end{center}\n");
      fprintf (Archivo_Latex, "\\end{table}\n");

      fclose (Archivo_Datos);
     }

  if (Total_subtemas >= 1)
     {
      Archivo_gnuplot = fopen ("EX5020.gp","w");
      fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 10\n");
      fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
      fprintf (Archivo_gnuplot, "set size 1.0, 0.7\n");
      fprintf (Archivo_gnuplot, "set grid ytics\n");
      fprintf (Archivo_gnuplot, "set output \"EX5020x.eps\"\n");

      fprintf (Archivo_gnuplot, "set xlabel \"Subtema\"\n");
      fprintf (Archivo_gnuplot, "set ylabel \"Cantidad de Preguntas\"\n");
      fprintf (Archivo_gnuplot, "set yrange [0:]\n");

      fprintf (Archivo_gnuplot, "set xrange [-1:%d]\n", Total_subtemas);
      fprintf (Archivo_gnuplot, "set xtics 0, 1\n");
      fprintf (Archivo_gnuplot, "set xtics border in scale 1,0.5 nomirror rotate by -45  offset character 0, 0, 0\n");

      fprintf (Archivo_gnuplot, "set xtics (");
      for (i=0; i < Total_subtemas; i++)
          {
	   fprintf (Archivo_gnuplot,"\"%s\" %d", PQgetvalue (res, i, 0),i);
           if (i == (Total_subtemas - 1))
	      fprintf (Archivo_gnuplot,")\n ");
	   else
	      fprintf (Archivo_gnuplot,", ");
	  }

      fprintf (Archivo_gnuplot, "set style fill solid 1.0 border -1\n");
      fprintf (Archivo_gnuplot, "set style histogram rowstacked\n");
      fprintf (Archivo_gnuplot, "set style data histograms\n");
      fprintf (Archivo_gnuplot, "set boxwidth 0.3\n");

      fprintf (Archivo_gnuplot, "plot \"EX5020.dat\" using 2 t \"Usadas\" lc 3 lt 1, \'\' using 3:xtic(1) t \"Nuevas\" lc 2 lt 1");

      fclose (Archivo_gnuplot);

      sprintf(comando, "%s EX5020.gp", parametros.gnuplot);
      system (comando);

      sprintf(comando, "mv EX5020.gp %s", parametros.ruta_gnuplot);
      system (comando);

      sprintf(comando, "%s EX5020x.eps", parametros.epstopdf);
      system (comando);

      system ("rm EX5020x.eps");
      system ("rm EX5020.dat");

      fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
      fprintf (Archivo_Latex, "\\centering\n");
      fprintf (Archivo_Latex, "\\includegraphics[scale=1.0]{EX5020x.pdf}\n");
      fprintf (Archivo_Latex, "\\end{figure}\n");
     }

  if (Total_subtemas) Grafico_y_Tabla (Archivo_Latex, media, varianza);
  if (Total_subtemas) Dificultad_vs_Discriminacion(Archivo_Latex, materia, tema);
}

void Grafico_y_Tabla(FILE * Archivo_Latex, long double media, long double varianza)
{
    fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
    fprintf (Archivo_Latex, "\\centering\n");
    fprintf (Archivo_Latex, "\\begin{minipage}[c]{0.25\\textwidth}\n");
    fprintf (Archivo_Latex, "\\centering\n");

    Tabla_Probabilidades   (Archivo_Latex, media, varianza);
    fprintf (Archivo_Latex, "\\end{minipage}\n");

    Prepara_Grafico_Normal (Archivo_Latex, media, varianza);

    fprintf (Archivo_Latex, "\\begin{minipage}[c]{0.71\\textwidth}\n");
    fprintf (Archivo_Latex, "\\includegraphics[scale=1.0]{EX5020-n.pdf}\n");
    fprintf (Archivo_Latex, "\\end{minipage}\n");
    fprintf (Archivo_Latex, "\\end{figure}\n");
}

void Prepara_Grafico_Normal(FILE * Archivo_Latex, long double media, long double varianza)
{
    FILE * Archivo_gnuplot;
    char Hilera_Antes [2000], Hilera_Despues [2000];
    long double desviacion;
    char comando[600];

    desviacion = sqrt(varianza);

    Archivo_gnuplot = fopen ("EX5020-n.gp","w");

    fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 12\n");
    fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
    fprintf (Archivo_gnuplot, "set size 1.1, 0.8\n");
    fprintf (Archivo_gnuplot, "set grid xtics\n");

    fprintf (Archivo_gnuplot, "set output \"EX5020-n.eps\"\n");
    sprintf (Hilera_Antes,"set xlabel \"Media = %Lf - Desviación = %Lf\"", media, desviacion);
    hilera_GNUPLOT (Hilera_Antes, Hilera_Despues);
    fprintf (Archivo_gnuplot, "%s\n", Hilera_Despues);
    hilera_GNUPLOT ("set ylabel \"Proporción\"\n", Hilera_Despues);
    fprintf (Archivo_gnuplot, "%s", Hilera_Despues);

    fprintf (Archivo_gnuplot, "set xrange [%Lf:%Lf]\n", media - 3*desviacion, media + 3*desviacion);
    fprintf (Archivo_gnuplot, "load \".scripts/stat.inc\"\n");
    fprintf (Archivo_gnuplot, "set style fill pattern 8 border 2\n");

    if ((media+1.7*desviacion) >= 67.5)
       {
        fprintf (Archivo_gnuplot, "set arrow from %Lf, 0.03 to %Lf, 0.01 lw 6 lt 1 lc 2 front\n", media+2.4*desviacion, media+1.7*desviacion);
        sprintf (Hilera_Antes, "set label \"%Lf\" at %Lf, 0.032 front\n", 1.0-CDF(67.5, media, desviacion), media+2.4*desviacion);
        hilera_GNUPLOT (Hilera_Antes, Hilera_Despues);
        fprintf (Archivo_gnuplot,"%s",Hilera_Despues);
       }

    fprintf (Archivo_gnuplot, "plot normal (x, %Lf, %Lf) with filledcurve above x1=67.50 notitle, normal (x, %Lf, %Lf) with lines linetype 1 lw 7 lc 3 notitle",
             media, desviacion,media, desviacion);

    fclose (Archivo_gnuplot);

    sprintf(comando, "%s EX5020-n.gp", parametros.gnuplot);
    system (comando);

    sprintf(comando, "mv EX5020-n.gp %s", parametros.ruta_gnuplot);
    system (comando);

    sprintf(comando, "%s EX5020-n.eps", parametros.epstopdf);
    system (comando);

    system ("rm EX5020-n.eps");
}

void Tabla_Probabilidades (FILE * Archivo_Latex, long double media, long double varianza)
{
   long double desviacion;
   fprintf (Archivo_Latex, "\\rowcolors{1}{white}{green}\n");
   fprintf (Archivo_Latex, "\\begin{tabular}[b]{|r|r|}\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Nota} & \\textbf{Probabilidad} \\\\\n");
   fprintf (Archivo_Latex, "\\hline\\hline\n");

   desviacion = sqrt(varianza);

   fprintf (Archivo_Latex, "67.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 67.5, media, desviacion));
   fprintf (Archivo_Latex, "70.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 70.0, media, desviacion));
   fprintf (Archivo_Latex, "72.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 72.5, media, desviacion));
   fprintf (Archivo_Latex, "75.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 75.0, media, desviacion));
   fprintf (Archivo_Latex, "77.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 77.5, media, desviacion));
   fprintf (Archivo_Latex, "80.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 80.0, media, desviacion));
   fprintf (Archivo_Latex, "82.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 82.5, media, desviacion));
   fprintf (Archivo_Latex, "85.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 85.0, media, desviacion));
   fprintf (Archivo_Latex, "87.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 87.5, media, desviacion));
   fprintf (Archivo_Latex, "90.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 90.0, media, desviacion));
   fprintf (Archivo_Latex, "92.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 92.5, media, desviacion));
   fprintf (Archivo_Latex, "95.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 95.0, media, desviacion));
   fprintf (Archivo_Latex, "97.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 97.5, media, desviacion));
   fprintf (Archivo_Latex, "100.00 & %Lf\\\\ \\hline\n\n",1.0-CDF(100.0, media, desviacion));

   fprintf (Archivo_Latex, "\\hline\n");

   fprintf (Archivo_Latex, "\\end{tabular}\n");
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

long double CDF (long double X, long double Media, long double Desv)
{
  return (0.5*(1 + (long double)erf((X-Media)/(Desv*M_SQRT2))));
}

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex5020_destroy (GtkWidget *widget, gpointer user_data) 
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
