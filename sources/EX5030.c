/*
Compilar con

cc -o EX5030 EX5030.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX5030:                                */
/*                                         */
/*    Cruza 2 variables asociadas a las    */
/*    preguntas y permite listar las pre-  */
/*    guntas en cada cruce                 */
/*    De momento limitado a Dificultad vs. */
/*    Discriminación.                      */
/*                                         */
/*    The Examiner 0.2                     */
/*    18 de Noviembre 2012                 */
/*    Autor: Francisco J. Torres-Rojas     */        
/*******************************************/
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
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
GtkWidget *window2                = NULL;

GtkComboBox *CB_materia           = NULL;
GtkComboBox *CB_tema              = NULL;
GtkComboBox *CB_subtema           = NULL;

GtkWidget *SC_resolucion          = NULL;
GtkWidget *SC_color               = NULL;
GtkToggleButton *RB_preguntas     = NULL;
GtkToggleButton *RB_usos          = NULL;
GtkToggleButton *CK_animacion     = NULL;
GtkToggleButton *CK_smooth        = NULL;

GtkWidget *FR_grafico             = NULL;
GtkImage  *IM_left                = NULL;

GtkWidget *FR_mapa                = NULL;
GtkImage  *IM_right               = NULL;

GtkWidget *FR_botones             = NULL;
GtkWidget *BN_proceso             = NULL;
GtkWidget *BN_terminar            = NULL;

GtkWidget *FR_reporte             = NULL;
GtkLabel  *LB_dificultad_1        = NULL;
GtkLabel  *LB_dificultad_2        = NULL;
GtkLabel  *LB_discriminacion_1    = NULL;
GtkLabel  *LB_discriminacion_2    = NULL;
GtkWidget *FR_cantidad            = NULL;
GtkLabel  *LB_preguntas           = NULL;
GtkLabel  *LB_usos                = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_ok                  = NULL;
GtkWidget *SC_rotacion            = NULL;
GtkWidget *EN_preguntas           = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Asigna_Banderas        (int i, PGresult * res, int * flags);
void Cajita_con_bandera     (FILE * Archivo_Latex, char * mensaje, int color);
void Calcular_Tabla         (PGresult *res_datos);
void Cambio_Materia         (GtkWidget *widget, gpointer user_data);
void Cambio_Tema            (GtkWidget *widget, gpointer user_data);
void Connect_Widgets        ();
void Cuantas_preguntas_usos (long * preguntas, long * usos);
void Dificultad_vs_Discriminacion (PGresult *res_datos);
PGresult * Ensambla_query   ();
void Extrae_codigo          (char * hilera, char * codigo, int largo);
void Fin_de_Programa        (GtkWidget *widget, gpointer user_data);
void Fin_Ventana            (GtkWidget *widget, gpointer user_data);
void Imprime_Banderas       (FILE * Archivo_Latex, int * flags, int i, PGresult * res);
void Imprime_listado        ();
void Imprime_Opcion         (FILE * Archivo_Latex, PGresult *res, char * alfileres, long double Total, long double Porcentaje, int Frecuencia, long double Rpb, char correcta, int pregunta, int opcion);
void Imprime_pregunta       (int i, PGresult * res, FILE * Archivo_Latex,char * prefijo);
void Init_Fields            ();
void Interface_Coloring     ();
void on_BN_proceso_clicked  (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked (GtkWidget *widget, gpointer user_data);
void on_CB_materia_changed  (GtkWidget *widget, gpointer user_data);
void on_CB_tema_changed     (GtkWidget *widget, gpointer user_data);
void on_WN_ex5030_destroy   (GtkWidget *widget, gpointer user_data);
void Prepara_graficos       (GtkWidget *widget, gpointer user_data);
void Read_Only_Fields       ();
void Resumen_de_Banderas    (FILE * Archivo_Latex);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define CODIGO_SIZE     7
#define FILAS_TABLA    11
#define COLUMNAS_TABLA 21

int N_materias = 0;
int N_temas    = 0;
int N_subtemas = 0;
char Codigo_Materia [CODIGO_MATERIA_SIZE + 1];
char Codigo_Tema    [CODIGO_TEMA_SIZE    + 1];
char Codigo_Subtema [CODIGO_SUBTEMA_SIZE + 1];

int Niveles_Discriminacion;
int Niveles_Dificultad;
int Frecuencia_total [FILAS_TABLA][COLUMNAS_TABLA];
long double Discriminacion_Izquierda, Discriminacion_Derecha;
long double Dificultad_Abajo, Dificultad_Arriba;

#define N_FLAGS                  15

#define TODAS_MALAS                  0
#define TODAS_BUENAS                 1
#define PORCENTAJE_MENOR_30          2
#define RPB_BUENO                    3
#define RPB_NEGATIVO                 4
#define ALFA_SUBE_MUCHO              5
#define ALFA_SUBE                    6
#define RPB_DISTRACTOR_MUY_POSITIVO  7
#define RPB_DISTRACTOR_POSITIVO      8
#define RPB_DISTRACTOR_MUY_NEGATIVO  9
#define SOLO_2_DISTRACTORES          10
#define MENOS_DE_5_DISTRACTORES      11
#define USA_5_DISTRACTORES           12
#define RPB_Y_DIFICULTAD_OK          13
#define RPB_DIFI_DISTRA_OK           14

#define VERDE    0
#define AZUL     1
#define CYAN     2
#define AMARILLO 3
#define ROJO     4
int Banderas [5];
char * Colores_Banderas  [] = {"green", "blue", "cyan", "yellow", "red"};
char * banderas [] = {"FL-verde.jpg", "FL-azul.jpg", "FL-cyan.jpg", "FL-amarilla.jpg", "FL-roja.jpg"};
char * Colores_grafico [] = {"unset pm3d",
                     "set palette gray", 
                     "set palette gray negative", 
                     "set palette rgb 21,22,23",
                     "set palette rgb 34,35,36",
                     "set palette rgb 7,5,15",
                     "set palette rgb 3,11,6",
                     "set palette rgb 23,28,3",
                     "set palette rgb 33,13,10",
                     "set palette rgb 30,31,32"};

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX5030.glade", &error))
         {
          g_warning ("%s\n", error->message);
          g_error_free (error);
         }
      else
	 {
	  /* Conecta funciones con eventos de la interfaz */
          gtk_builder_connect_signals (builder, NULL);

	  /* Crea conexión con los campos de la interfaz a ser controlados */
	  Connect_Widgets ();

	  /* Algunos campos son READ ONLY */
	  Read_Only_Fields ();

	  /* Un poco de color */
	  Interface_Coloring ();

	  /* Despliega ventana principal e inicializa todos los campos */
          gtk_widget_show (window1);
          Init_Fields ();

          /* start the event loop */
          gtk_main();
	 }
     }

  return 0;
}

/**********************************/
/* No es obligatorio, pero en ge- */
/* neral, las variables internas  */
/* asociadas a los widgets de la  */
/* pantalla llevan el mismo nom-  */
/* bre que el widget respectivo.  */
/**********************************/
void Connect_Widgets ()
{
  window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex5030"));
  window2                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_reporte"));

  CB_materia             = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_materia"));
  CB_tema                = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_tema"));
  CB_subtema             = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_subtema"));

  SC_resolucion          = GTK_WIDGET (gtk_builder_get_object (builder, "SC_resolucion"));
  SC_color               = GTK_WIDGET (gtk_builder_get_object (builder, "SC_color"));
  RB_preguntas           = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_preguntas"));
  RB_usos                = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_usos"));
  CK_smooth              = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_smooth"));
  CK_animacion           = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_animacion"));

  FR_grafico             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_grafico"));
  IM_left                = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_left"));

  FR_mapa                = GTK_WIDGET (gtk_builder_get_object (builder, "FR_mapa"));
  IM_right               = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_right"));

  FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  BN_proceso             = GTK_WIDGET (gtk_builder_get_object (builder, "BN_proceso"));
  BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));

  FR_reporte             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reporte"));
  LB_dificultad_1        = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_dificultad_1"));
  LB_dificultad_2        = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_dificultad_2"));
  LB_discriminacion_1    = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_discriminacion_1"));
  LB_discriminacion_2    = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_discriminacion_2"));

  FR_cantidad            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_cantidad"));
  LB_preguntas           = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_preguntas"));
  LB_usos                = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_usos"));
  BN_print               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_ok                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
  SC_rotacion            = GTK_WIDGET (gtk_builder_get_object (builder, "SC_rotacion"));
  EN_preguntas           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_preguntas"));
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_preguntas, FALSE);
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse ("pale goldenrod", &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse ("dark goldenrod", &color);
  gtk_widget_modify_bg(FR_grafico, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_mapa,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_reporte, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_cantidad,GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_proceso, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,      GTK_STATE_ACTIVE, &color);

  gdk_color_parse ("light goldenrod", &color);
  gtk_widget_modify_bg(window2,  GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_proceso, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,      GTK_STATE_PRELIGHT, &color);
}

void Init_Fields()
{
   int i;
   PGresult *res;
   char hilera[300];

   gtk_range_set_update_policy (GTK_RANGE(SC_rotacion),   GTK_UPDATE_DISCONTINUOUS);
   gtk_range_set_update_policy (GTK_RANGE(SC_resolucion), GTK_UPDATE_DISCONTINUOUS);
   gtk_range_set_update_policy (GTK_RANGE(SC_color),      GTK_UPDATE_DISCONTINUOUS);

   gtk_range_set_value (GTK_RANGE(SC_rotacion),  30.0);
   gtk_range_set_value (GTK_RANGE(SC_resolucion), (gdouble) 10.0);
   gtk_range_set_value (GTK_RANGE(SC_color),      (gdouble) 7.0);
   gtk_toggle_button_set_active(CK_smooth, TRUE);

   gtk_combo_box_set_active (CB_materia, -1);
   for (i=0; i < N_materias; i++) gtk_combo_box_remove_text (CB_materia, 0);

   gtk_combo_box_set_active (CB_tema, -1);
   for (i=0; i < N_temas; i++) gtk_combo_box_remove_text (CB_tema, 0);

   gtk_combo_box_set_active (CB_subtema, -1);
   for (i=0; i < N_subtemas; i++) gtk_combo_box_remove_text (CB_subtema, 0);

   res = PQEXEC(DATABASE,"SELECT codigo_materia, descripcion_materia from bd_materias where codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_materia"); 
   N_materias = PQntuples(res);
   for (i=0; i < N_materias; i++)
       {
	sprintf (hilera,"%s %s",PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
	gtk_combo_box_append_text(CB_materia, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_materia, 0);

   gtk_widget_set_sensitive(BN_proceso, 1);

   gtk_widget_grab_focus (GTK_WIDGET(CB_materia));
}

void Cambio_Materia(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *materia;
  char PG_command  [1000];
  PGresult *res;
  char hilera [300];

  materia = gtk_combo_box_get_active_text(CB_materia);

  Extrae_codigo (materia, Codigo_Materia, CODIGO_MATERIA_SIZE + 1);

  gtk_combo_box_set_active (CB_tema, -1);
  for (i=0; i<N_temas; i++) gtk_combo_box_remove_text (CB_tema, 0);
  gtk_combo_box_append_text(CB_tema, CODIGO_VACIO);

  sprintf (PG_command,"SELECT codigo_tema, descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema != '          ' and codigo_subtema = '          ' order by orden", Codigo_Materia); 
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

  g_free (materia);
}

void Cambio_Tema(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *tema = NULL;
  char PG_command  [1000];
  PGresult *res;
  char hilera[500];

  tema    = gtk_combo_box_get_active_text(CB_tema);
  Extrae_codigo (tema, Codigo_Tema, CODIGO_TEMA_SIZE + 1);

  gtk_combo_box_set_active (CB_subtema, -1);
  for (i=0; i<N_subtemas; i++) gtk_combo_box_remove_text (CB_subtema, 0);
  gtk_combo_box_append_text(CB_subtema, CODIGO_VACIO);

  sprintf (PG_command,"SELECT codigo_subtema, descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' order by orden",
           Codigo_Materia, Codigo_Tema); 
  res = PQEXEC(DATABASE, PG_command);
  N_subtemas = PQntuples(res)+1;

  for (i=1; i < N_subtemas; i++)
      {
       sprintf (hilera,"%s %s", PQgetvalue (res, i-1, 0), PQgetvalue (res, i-1, 1));
       gtk_combo_box_append_text(CB_subtema, hilera);
      }
  PQclear(res);

  gtk_widget_set_sensitive (GTK_WIDGET(CB_subtema), 1);
  gtk_widget_set_can_focus (GTK_WIDGET(CB_subtema), TRUE);
  gtk_combo_box_set_active (CB_subtema, 0);

  g_free (tema);
}

void Cambio_Subtema(GtkWidget *widget, gpointer user_data)
{
  gchar *subtema = NULL;

  subtema    = gtk_combo_box_get_active_text(CB_subtema);
  Extrae_codigo (subtema, Codigo_Subtema, CODIGO_SUBTEMA_SIZE + 1);

  gtk_widget_grab_focus (GTK_WIDGET(CB_materia));
}

void Extrae_codigo (char * hilera, char * codigo, int largo)
{
  int i;

  i = 0;

  while (hilera[i] != ' ') codigo [i] = hilera[i++];

  while (i < (largo -1)) codigo [i++] = ' ';

  codigo[i] = '\0';
}

void Prepara_graficos (GtkWidget *widget, gpointer user_data)
{
   char PG_command [1000];
   PGresult *res_datos;
   int Total_datos;
   char hilera [20];

   res_datos = Ensambla_query ();

   Total_datos = PQntuples(res_datos);

   if (Total_datos) 
      Dificultad_vs_Discriminacion(res_datos);
   else
      {
       gtk_image_set_from_file (IM_left,  ".imagenes/empty.png");
       gtk_image_set_from_file (IM_right, ".imagenes/empty.png");
      }
   sprintf (hilera,"%10d", Total_datos);
   gtk_entry_set_text(GTK_ENTRY(EN_preguntas), hilera);

   PQclear(res_datos);
}

PGresult * Ensambla_query ()
{
  PGresult * res_datos;
  char PG_command [3000];

  if (strcmp (Codigo_Tema, CODIGO_VACIO) == 0)
     sprintf (PG_command,"SELECT dificultad, point_biserial, frecuencia from bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and codigo_unico_pregunta = pregunta and materia = '%s' and frecuencia > 0", Codigo_Materia);
  else
     if (strcmp (Codigo_Subtema, CODIGO_VACIO) == 0)
        sprintf (PG_command,"SELECT dificultad, point_biserial, frecuencia from bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and codigo_unico_pregunta = pregunta and materia = '%s' and tema = '%s' and frecuencia > 0", Codigo_Materia, Codigo_Tema);
     else
        sprintf (PG_command,"SELECT dificultad, point_biserial, frecuencia from bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and codigo_unico_pregunta = pregunta and materia = '%s' and tema = '%s' and subtema = '%s' and frecuencia > 0", Codigo_Materia, Codigo_Tema, Codigo_Subtema);

   res_datos = PQEXEC(DATABASE, PG_command);

   return (res_datos);
}

void Dificultad_vs_Discriminacion (PGresult *res_datos)
{
  int i;
  FILE *Archivo_gnuplot;
  int Color;
  int Smooth, Animacion;
  int Rotacion;

  int Total_preguntas;
  long double dificultad, discriminacion;
  int frecuencia;
  char comando[1000];
  char Hilera_Antes [2000], Hilera_Despues [2000];

  Color     = (int) gtk_range_get_value (GTK_RANGE(SC_color));
  Smooth    = gtk_toggle_button_get_active(CK_smooth);
  Animacion = gtk_toggle_button_get_active(CK_animacion);
  Rotacion  = (int) gtk_range_get_value (GTK_RANGE(SC_rotacion));

  Calcular_Tabla (res_datos);

  Archivo_gnuplot = fopen ("EX5030p.gp","w");
      fprintf (Archivo_gnuplot, "set term gif animate delay 15 truecolor size 586,481 font \"Arial\" 7\n");
      fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
      fprintf (Archivo_gnuplot, "set pm3d\n");
      fprintf (Archivo_gnuplot, "set style line 100 lt 5 lw 0.5 lc 2\n");
      fprintf (Archivo_gnuplot, "set pm3d hidden3d 100\n");
      fprintf (Archivo_gnuplot, "%s\n", Colores_grafico[Color]);
      fprintf (Archivo_gnuplot, "set size 1.02, 1.17\n");

      if (Niveles_Discriminacion < 12)
         fprintf (Archivo_gnuplot, "set xtics -1.0, %4.3Lf, 1.0 offset 1\n",(long double)2.0/(Niveles_Discriminacion));
      else
         fprintf (Archivo_gnuplot, "set xtics -1.0, %4.3Lf, 1.0 offset 1\n",2*(long double)2.0/(Niveles_Discriminacion));

      fprintf (Archivo_gnuplot, "set ytics  0.0, %4.3Lf, 1.0 offset 2\n",(long double)1.0/(Niveles_Dificultad));

      fprintf (Archivo_gnuplot, "set grid xtics\n");
      fprintf (Archivo_gnuplot, "set grid ytics\n");
      fprintf (Archivo_gnuplot, "set contour base\n");
      fprintf (Archivo_gnuplot, "set nokey\n");
      fprintf (Archivo_gnuplot, "set output \"EX5030p.gif\"\n");

      hilera_GNUPLOT ("set xlabel \"Discriminación\"\n", Hilera_Despues);
      fprintf (Archivo_gnuplot, "%s", Hilera_Despues);
      fprintf (Archivo_gnuplot, "set ylabel \"Dificultad\"\n");
      fprintf (Archivo_gnuplot, "set xyplane at -0.5\n");
      fprintf (Archivo_gnuplot, "set hidden3d\n");

      if (Smooth) fprintf (Archivo_gnuplot, "set dgrid3 %d,%d,gauss 0.15 0.15\n",Niveles_Dificultad+1,Niveles_Discriminacion+1);

      if (Animacion)
	 {
          for (i=0;i<360;i+=5)
	      {
	       fprintf (Archivo_gnuplot, "set view 60,%d,1,1\n", i);
               fprintf (Archivo_gnuplot, "splot \"EX5030p.dat\" title \" \" with lines lt 1 lw 1 lc 2\n");
	      }
	 }
      else
	 {
	  fprintf (Archivo_gnuplot, "set view 60,%d,1,1\n", Rotacion);
 	  fprintf (Archivo_gnuplot, "splot \"EX5030p.dat\" title \" \" with lines lt 1 lw 1 lc 2\n");
	 }

  fclose (Archivo_gnuplot);

  sprintf(comando, "%s EX5030p.gp", parametros.gnuplot);
  system (comando);

  sprintf(comando, "mv EX5030p.gp %s", parametros.ruta_gnuplot);
  system (comando);

  gtk_image_set_from_file (IM_left, "EX5030p.gif");
  system ("rm EX5030p.gif");

  Archivo_gnuplot = fopen ("EX5030c.gp","w");
      fprintf (Archivo_gnuplot, "set term png truecolor size 586,481 font \"Arial\" 7\n");
      fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
      fprintf (Archivo_gnuplot, "set pm3d\n");
      fprintf (Archivo_gnuplot, "%s\n", Colores_grafico[Color]);
      fprintf (Archivo_gnuplot, "set size 1.05, 1.15\n");
      fprintf (Archivo_gnuplot, "set output \"EX5030c.png\"\n");

      hilera_GNUPLOT ("set xlabel \"Discriminación\"\n", Hilera_Despues);
      fprintf (Archivo_gnuplot, "%s", Hilera_Despues);
      fprintf (Archivo_gnuplot, "set ylabel \"Dificultad\"\n");
      fprintf (Archivo_gnuplot, "set view map\n");
      fprintf (Archivo_gnuplot, "set contour\n");

      if (Niveles_Discriminacion == 18)
         fprintf (Archivo_gnuplot, "set xtics -1.0, %4.3Lf, 1.0\n",2*(long double)2.0/(Niveles_Discriminacion));
      else
         fprintf (Archivo_gnuplot, "set xtics -1.0, %4.3Lf, 1.0\n",(long double)2.0/(Niveles_Discriminacion));

      fprintf (Archivo_gnuplot, "set ytics  0.0, %4.3Lf, 1.0\n",(long double)1.0/(Niveles_Dificultad));
      fprintf (Archivo_gnuplot, "set key at -1.1, -0.07 left box lt 1\n");
      if (Smooth) fprintf (Archivo_gnuplot, "set dgrid3 %d,%d,gauss 0.15 0.15\n",Niveles_Dificultad+1,Niveles_Discriminacion+1);

      fprintf (Archivo_gnuplot, "splot \"EX5030p.dat\" notitle with lines lt 1 lw 1 lc 2\n");
  fclose (Archivo_gnuplot);

  sprintf(comando, "%s EX5030c.gp", parametros.gnuplot);
  system (comando);

  sprintf(comando, "mv EX5030c.gp %s", parametros.ruta_gnuplot);
  system (comando);

  system ("rm EX5030p.dat");

  gtk_image_set_from_file (IM_right, "EX5030c.png");
  system ("rm EX5030c.png");
}

void Calcular_Tabla(PGresult *res_datos)
{
  int i, j, k;
  int Total_preguntas;
  long double dificultad, discriminacion;
  int frecuencia;
  FILE * Archivo_Datos;

  Niveles_Dificultad     = (int) gtk_range_get_value (GTK_RANGE(SC_resolucion));
  Niveles_Discriminacion = Niveles_Dificultad * 2;
  for (i=0; i <= Niveles_Dificultad; i++)
      for (j=0; j <= Niveles_Discriminacion; j++)
	  Frecuencia_total [i][j] = 0;

  Total_preguntas = PQntuples(res_datos);

  for (k=0; k <Total_preguntas; k++)
      {
       dificultad     = atof(PQgetvalue (res_datos, k, 0));
       discriminacion = atof(PQgetvalue (res_datos, k, 1));
       frecuencia     = atoi(PQgetvalue (res_datos, k, 2));

       i = (int) round (dificultad * Niveles_Dificultad);
       j = (int) round ((discriminacion+1.0)/2 * Niveles_Discriminacion);

       if (gtk_toggle_button_get_active(RB_preguntas))
          Frecuencia_total [i][j]++;
       else
          Frecuencia_total [i][j]+= frecuencia;
      }

  Archivo_Datos = fopen ("EX5030p.dat","w");
  for (j=0; j <= Niveles_Discriminacion; j++)
      {
       for (i=0; i <= Niveles_Dificultad; i++)
 	   {
	    fprintf (Archivo_Datos, "%Lf %Lf %d\n",(long double) j/Niveles_Discriminacion*2.0 - 1.0, (long double) i/Niveles_Dificultad, Frecuencia_total[i][j]);
	   }
       fprintf (Archivo_Datos, "\n");
      }
  fclose (Archivo_Datos);
}

void Despliegue_preguntas (int x, int y)
{
#define X_MIN 92
#define X_MAX 523
#define Y_MIN 13
#define Y_MAX 384
#define X_ANCHO (X_MAX - X_MIN + 1)
#define Y_ALTO  (Y_MAX - Y_MIN + 1)
  long double p, q;
  int i, j;
  char hilera[30];
  long preguntas, usos;

  if ((x >= X_MIN) && (x <= X_MAX) && (y >= Y_MIN) && (y <= Y_MAX))
     {
      gtk_widget_set_sensitive(window1,  0);
      gtk_widget_show (window2);

      p = (long double) (x - X_MIN)/X_ANCHO;
      q = 1.0 - (long double) (y - Y_MIN)/Y_ALTO;

      j = (int) floor (p * (Niveles_Discriminacion));
      i = (int) floor (q * (Niveles_Dificultad));

      Dificultad_Abajo         = (long double)  i    / Niveles_Dificultad;
      Dificultad_Arriba        = (long double) (i+1) / Niveles_Dificultad;
      Discriminacion_Izquierda = (long double) j / Niveles_Discriminacion * 2.0 - 1.0;
      Discriminacion_Derecha   = (long double) (j+1) / Niveles_Discriminacion * 2.0 - 1.0;

      Cuantas_preguntas_usos (&preguntas, &usos);

      sprintf (hilera,"%4.3Lf", Dificultad_Abajo);
      gtk_label_set_markup(LB_dificultad_1, hilera);
      sprintf (hilera,"%4.3Lf", Dificultad_Arriba);
      gtk_label_set_markup(LB_dificultad_2, hilera);
      sprintf (hilera,"%4.3Lf", Discriminacion_Izquierda);
      gtk_label_set_markup(LB_discriminacion_1, hilera);
      sprintf (hilera,"%4.3Lf", Discriminacion_Derecha);
      gtk_label_set_markup(LB_discriminacion_2, hilera);
      sprintf (hilera,"%ld", preguntas);
      gtk_label_set_markup(LB_preguntas, hilera);
      sprintf (hilera,"%ld", usos);
      gtk_label_set_markup(LB_usos, hilera);

      if (preguntas)
         gtk_widget_set_sensitive(BN_print, 1);
      else
         gtk_widget_set_sensitive(BN_print, 0);
     }
}

void Cuantas_preguntas_usos (long * preguntas, long * usos)
{
   char PG_command [3000];
   int i, Total_preguntas, Total_usos;

   if (Dificultad_Arriba      > (1.0 - (1.0/Niveles_Dificultad)))     Dificultad_Arriba = 1.0;
   if (Discriminacion_Derecha > (1.0 - (2.0/Niveles_Discriminacion))) Discriminacion_Derecha = 1.0;

   if (strcmp (Codigo_Tema, CODIGO_VACIO) == 0)
      sprintf (PG_command,"SELECT frecuencia from bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and codigo_unico_pregunta = pregunta and materia = '%s' and frecuencia > 0 and (point_biserial >= %4.3Lf) and (point_biserial %s %4.3Lf) and (dificultad >= %4.3Lf) and (dificultad %s %4.3Lf)", Codigo_Materia, Discriminacion_Izquierda, (Discriminacion_Derecha == 1.0)? "<=":"<", Discriminacion_Derecha, Dificultad_Abajo, (Dificultad_Arriba == 1.0)? "<=":"<", Dificultad_Arriba);
   else
      if (strcmp (Codigo_Subtema, CODIGO_VACIO) == 0)
         sprintf (PG_command,"SELECT frecuencia from bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and codigo_unico_pregunta = pregunta and materia = '%s' and tema = '%s' and frecuencia > 0 and (point_biserial >= %4.3Lf) and (point_biserial %s %4.3Lf) and (dificultad >= %4.3Lf) and (dificultad %s %4.3Lf)", Codigo_Materia, Codigo_Tema, Discriminacion_Izquierda, (Discriminacion_Derecha == 1.0)? "<=":"<", Discriminacion_Derecha, Dificultad_Abajo, (Dificultad_Arriba == 1.0)? "<=":"<", Dificultad_Arriba); 
      else
	 sprintf (PG_command,"SELECT frecuencia from bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and codigo_unico_pregunta = pregunta and materia = '%s' and tema = '%s' and subtema = '%s' and frecuencia > 0 and (point_biserial >= %4.3Lf) and (point_biserial %s %4.3Lf) and (dificultad >= %4.3Lf) and (dificultad %s %4.3Lf)", Codigo_Materia, Codigo_Tema, Codigo_Subtema, Discriminacion_Izquierda, (Discriminacion_Derecha == 1.0)? "<=":"<", Discriminacion_Derecha, Dificultad_Abajo, (Dificultad_Arriba == 1.0)? "<=":"<", Dificultad_Arriba); 

   res = PQEXEC(DATABASE, PG_command);
   Total_preguntas = PQntuples(res);

   Total_usos = 0;
   for (i=0; i <Total_preguntas; i++) Total_usos += (atoi(PQgetvalue (res, i, 0)));

   PQclear(res);

   *preguntas = Total_preguntas;
   *usos      = Total_usos;
}

/*********************************************************************/

/*******************************************************/
/* Query monstruoso usado para imprimir listado:       */
/*

BD_EJERCICIOS:
0   codigo_ejercicio char(6) PRIMARY KEY,
1   materia          char(5),
2   tema             char(5),
3   subtema          char(5),
4   liga_year            int,
5   liga_month           int,
6   liga_day             int,
7   automatico       boolean,
8   comentario       char varying(100),
9   texto_ejercicio  char(6) REFERENCES BD_Texto_Ejercicios ON DELETE RESTRICT,
10  objetivo1        char(1),
11  objetivo2        char(1),
12  objetivo3        char(1),
13  objetivo4        char(1),
14  objetivo5        char(1),
15  objetivo6        char(1),
16  objetivo7        char(1),
17  objetivo8        char(1),
18  bloom                int,

BD_TEXTO_EJERCICIOS:
19  consecutivo_texto char(6) PRIMARY KEY,
20  descripcion_ejercicio char varying(200),
21  contador_referencias int,
22  autor             char(10) REFERENCES bd_autores ON DELETE RESTRICT,
23  usa_header        boolean,
24  texto_header      char varying(4096)

BD_TEXTO_PREGUNTAS:
25  codigo_consecutivo_ejercicio char(6) REFERENCES BD_Texto_Ejercicios ON DELETE RESTRICT,
26  secuencia_pregunta           int,
27  codigo_unico_pregunta        char(6) UNIQUE NOT NULL,
28  tipo             char(1),
29  creacion_year    int,
30  creacion_month   int,
31  creacion_day     int,
32  dificultad_estimada      float,
33  texto_pregunta   char varying (4096),
34  texto_opcion_A   char varying (4096),
35  texto_opcion_B   char varying (4096),
36  texto_opcion_C   char varying (4096),
37  texto_opcion_D   char varying (4096),
38  texto_opcion_E   char varying (4096),
39  respuesta        char,
40  alfileres_opciones char(5),

BD_ESTADISTICAS_PREGUNTAS:
41  pregunta        char(6) UNIQUE NOT NULL,
42  ultimo_uso_year          int,
43  ultimo_uso_month         int,
44  ultimo_uso_day           int,
45  ultimo_examen            char(5),
46  N_examenes               int,
47  N_estudiantes            int,
48  media_examenes           float,
49  suma_cuadrados_examenes  float,
50  varianza_examenes        float,
51  dificultad               float,
52  varianza_dificultad      float,
53  point_biserial           float,
54  media_correctos          float,
55  media_incorrectos        float,
56  N_correctos              int,
57  N_incorrectos            int,
58  cronbach_alpha_media_con float,
59  cronbach_alpha_media_sin float,
60  ponderacion_cronbach   int,
61  frecuencia       int,
62  frecuencia_1     int,
63  frecuencia_2     int,
64  frecuencia_3     int,
65  frecuencia_4     int,
66  frecuencia_5     int,
67  media_ex_1_1        float,
68  media_ex_1_0        float,
69  media_ex_2_1        float,
70  media_ex_2_0        float,
71  media_ex_3_1        float,
72  media_ex_3_0        float,
73  media_ex_4_1        float,
74  media_ex_4_0        float,
75  media_ex_5_1        float,
76  media_ex_5_0        float,
77  point_biserial_1          float,
78  point_biserial_2          float,
79  point_biserial_3          float,
80  point_biserial_4          float,
81  point_biserial_5          float,

BD_PERSONAS:
82  codigo_persona    char(10),
83  nombre            char varying(100),
84  afiliacion        char varying(100)
*/
/*******************************************************/

void Imprime_listado ()
{
   FILE * Archivo_Latex, * Archivo_gnuplot;
   gchar *materia_descripcion=" ", *tema_descripcion=" ", *subtema_descripcion=" ";
   char ejercicio_actual [CODIGO_SIZE] = "000000";
   char PG_command [3000];
   int i, actual, N_preguntas, N_preguntas_ejercicio;
   char hilera_antes [4000], hilera_despues [4000];
   char alfileres [6];
   char Correcta;
   PGresult *res, *res_materia, *res_tema, *res_subtema, *res_preguntas, *res_datos;
   int Color, Smooth;
   char comando[1000];
   int Color_flecha [] = {1, 1, 2, 2, 2, 5, 6, 7, 8};

   Banderas [0] =Banderas [1] =Banderas [2] =Banderas [3] = Banderas [4] = 0;

   res_datos = Ensambla_query ();
   Calcular_Tabla (res_datos);
   PQclear(res_datos);

   Color                  = (int) gtk_range_get_value (GTK_RANGE(SC_color));
   Smooth    = gtk_toggle_button_get_active(CK_smooth);

   Archivo_Latex = fopen ("EX5030.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, oneside, onecolumn]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);
   fprintf (Archivo_Latex, "\\SetWatermarkText{Confidencial}\n");
   fprintf (Archivo_Latex, "\\SetWatermarkLightness{0.9}\n");

   fprintf (Archivo_Latex, "\\begin{document}\n");

   sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", Codigo_Materia);

   res_materia = PQEXEC(DATABASE, PG_command);
   if (strcmp(Codigo_Tema, CODIGO_VACIO) == 0)
      {
       sprintf (PG_command,"SELECT * from bd_ejercicios, bd_texto_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas, bd_personas where (frecuencia > 0) and (point_biserial >= %4.3Lf) and (point_biserial %s %4.3Lf) and (dificultad >= %4.3Lf) and (dificultad %s %4.3Lf) and texto_ejercicio = consecutivo_texto and consecutivo_texto = codigo_consecutivo_ejercicio and materia = '%s' and pregunta = codigo_unico_pregunta and autor = codigo_persona order by materia, tema, subtema, codigo_ejercicio, secuencia_pregunta", 
		Discriminacion_Izquierda, (Discriminacion_Derecha == 1.0)? "<=":"<", Discriminacion_Derecha, Dificultad_Abajo, (Dificultad_Arriba == 1.0)? "<=":"<", Dificultad_Arriba,
                Codigo_Materia);
       sprintf (hilera_antes, "\\title{%s}\n", PQgetvalue (res_materia, 0, 0));
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s", hilera_despues);
      }
   else
      {
       sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", Codigo_Materia, Codigo_Tema);
       res_tema = PQEXEC(DATABASE, PG_command);

       if (strcmp(Codigo_Subtema, CODIGO_VACIO) == 0)
	  {
           sprintf (PG_command,"SELECT * from bd_ejercicios, bd_texto_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas, bd_personas where (frecuencia > 0) and (point_biserial >= %4.3Lf) and (point_biserial %s %4.3Lf) and (dificultad >= %4.3Lf) and (dificultad %s %4.3Lf) and texto_ejercicio = consecutivo_texto and consecutivo_texto = codigo_consecutivo_ejercicio and materia = '%s' and tema = '%s' and pregunta = codigo_unico_pregunta and autor = codigo_persona order by materia, tema, subtema, codigo_ejercicio, secuencia_pregunta", 
		Discriminacion_Izquierda, (Discriminacion_Derecha == 1.0)? "<=":"<", Discriminacion_Derecha, Dificultad_Abajo, (Dificultad_Arriba == 1.0)? "<=":"<", Dificultad_Arriba,
                Codigo_Materia, Codigo_Tema);
           sprintf (hilera_antes, "\\title{%s\\\\%s}\n", PQgetvalue (res_materia, 0, 0), PQgetvalue (res_tema, 0, 0));
           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf (Archivo_Latex, "%s", hilera_despues);
	  }
       else
	  {
	   sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", Codigo_Materia, Codigo_Tema, Codigo_Subtema);
           res_subtema = PQEXEC(DATABASE, PG_command);

           sprintf (PG_command,"SELECT * from bd_ejercicios, bd_texto_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas, bd_personas where (frecuencia > 0) and (point_biserial >= %4.3Lf) and (point_biserial %s %4.3Lf) and (dificultad >= %4.3Lf) and (dificultad %s %4.3Lf) and texto_ejercicio = consecutivo_texto and consecutivo_texto = codigo_consecutivo_ejercicio and materia = '%s' and tema = '%s' and subtema = '%s' and pregunta = codigo_unico_pregunta and autor = codigo_persona order by materia, tema, subtema, codigo_ejercicio, secuencia_pregunta", 
		Discriminacion_Izquierda, (Discriminacion_Derecha == 1.0)? "<=":"<", Discriminacion_Derecha, Dificultad_Abajo, (Dificultad_Arriba == 1.0)? "<=":"<", Dificultad_Arriba,
		    Codigo_Materia, Codigo_Tema, Codigo_Subtema);
           sprintf (hilera_antes, "\\title{%s\\\\%s\\\\%s}\n", PQgetvalue (res_materia, 0, 0), PQgetvalue (res_tema, 0, 0), PQgetvalue (res_subtema, 0, 0));
           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf (Archivo_Latex, "%s", hilera_despues);

           PQclear(res_subtema);
	  }
       PQclear(res_tema);
      }
   PQclear(res_materia);

   fprintf (Archivo_Latex, "\\maketitle\n\n");
   fprintf (Archivo_Latex, "\n\n");

   Archivo_gnuplot = fopen ("EX5030x.gp","w");
       fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 14\n");
       fprintf (Archivo_gnuplot, "set termoption enhanced\n");
       fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
       fprintf (Archivo_gnuplot, "set pm3d\n");
       fprintf (Archivo_gnuplot, "%s\n", Colores_grafico[Color]);
       fprintf (Archivo_gnuplot, "set size 1.37, 1.0\n");
       fprintf (Archivo_gnuplot, "set xtics -1.0, %4.3Lf, 1.0\n",(long double)2.0/(Niveles_Discriminacion));
       fprintf (Archivo_gnuplot, "set ytics  0.0, %4.3Lf, 1.0\n",(long double)1.0/(Niveles_Dificultad));
       fprintf (Archivo_gnuplot, "set output \"EX5030x.eps\"\n");

       hilera_GNUPLOT ("set xlabel \"Discriminación\"\n", hilera_despues);
       fprintf (Archivo_gnuplot, "%s", hilera_despues);
       fprintf (Archivo_gnuplot, "set ylabel \"Dificultad\"\n");
       fprintf (Archivo_gnuplot, "set view map\n");
       fprintf (Archivo_gnuplot, "set contour\n");
       fprintf (Archivo_gnuplot, "set key at -1.25, 1.2 left box lt 1\n");

       if (Smooth) fprintf (Archivo_gnuplot, "set dgrid3 %d,%d,gauss 0.15 0.15\n",Niveles_Dificultad+1,Niveles_Discriminacion+1);
       fprintf (Archivo_gnuplot, "LABEL1 = \"%4.3Lf {/Symbol \\243} {/Times-Italic r_{pb}} < %4.3Lf\"\n", Discriminacion_Izquierda, Discriminacion_Derecha);
       fprintf (Archivo_gnuplot, "LABEL2 = \"%4.3Lf {/Symbol \\243} {/Times-Italic p} < %4.3Lf\"\n", Dificultad_Abajo, Dificultad_Arriba);

       fprintf (Archivo_gnuplot, "set arrow from +0.0,+1.03 to %Lf, %Lf size graph 0.05,17 front lw 5 lc %d\n",(Discriminacion_Izquierda+Discriminacion_Derecha)/2,(Dificultad_Abajo+Dificultad_Arriba)/2, Color_flecha[Color]);
       fprintf (Archivo_gnuplot, "set obj 10 rect at 0.0,1.115 size char 17,4.35 lw 5\n");
       fprintf (Archivo_gnuplot, "set obj 10 fillstyle empty border -1 front\n");

       fprintf (Archivo_gnuplot, "set label at 0.0,1.15 LABEL1 front center\n");
       fprintf (Archivo_gnuplot, "set label at 0.0,1.08 LABEL2 front center\n");

       fprintf (Archivo_gnuplot, "splot \"EX5030p.dat\" notitle with lines lt 1 lw 5 lc 2\n");

   fclose (Archivo_gnuplot);

   sprintf(comando, "%s EX5030x.gp", parametros.gnuplot);
   system (comando);

   sprintf(comando, "mv EX5030x.gp %s", parametros.ruta_gnuplot);
   system (comando);

   sprintf(comando, "%s EX5030x.eps", parametros.epstopdf);
   system (comando);

   system ("rm EX5030x.eps");
   system ("rm EX5030p.dat");
 
   fprintf (Archivo_Latex, "\\begin{figure}[!h]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=1.0]{EX5030x.pdf}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n\n");

   fprintf (Archivo_Latex, "\\clearpage\n");
   fprintf (Archivo_Latex, "\\twocolumn\n");

   actual      = 0;
   res         = PQEXEC(DATABASE, PG_command);
   N_preguntas = PQntuples(res);

   if (N_preguntas)
      {
       strcpy (ejercicio_actual, PQgetvalue (res, actual, 0));
       N_preguntas_ejercicio = 0;
       i = actual;
       for (i = actual; (i < N_preguntas) && (strcmp(ejercicio_actual, PQgetvalue (res, i, 0)) == 0); i++) N_preguntas_ejercicio++;

       if (N_preguntas_ejercicio > 1)
          fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

       if (*PQgetvalue (res, actual, 23) == 't') /* 23 => usa_header */
	  {
           if (N_preguntas_ejercicio > 1)
	      {
	       if (N_preguntas_ejercicio == 2)
	          fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
	       else
	          fprintf (Archivo_Latex, "\\textbf{Las preguntas %d a %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	       strcpy (hilera_antes, PQgetvalue (res, actual, 24)); /* 24 => texto_header */
               hilera_LATEX (hilera_antes, hilera_despues);
               fprintf (Archivo_Latex, "%s\n", hilera_despues);
               fprintf (Archivo_Latex, "\\rule{8.9cm}{1pt}\n");
               fprintf (Archivo_Latex, "\\begin{questions}\n");
	      }
	   else
	      {
               fprintf (Archivo_Latex, "\\begin{questions}\n");
	       Imprime_pregunta (actual, res, Archivo_Latex, PQgetvalue (res, actual, 24));  /* 24 => texto_header */
	       actual++;
	       N_preguntas_ejercicio = 0;
	      }
	  }
       else
	  {
           fprintf (Archivo_Latex, "\\begin{questions}\n");
	  }


       for (i=0; i < N_preguntas_ejercicio; i++)
	   {
	    Imprime_pregunta (actual+i, res, Archivo_Latex," ");
	   }

       if (N_preguntas_ejercicio > 1)
          fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

       actual += N_preguntas_ejercicio;

       while (actual < N_preguntas)
             {
              strcpy (ejercicio_actual, PQgetvalue (res, actual, 0));
              N_preguntas_ejercicio = 0;
              i = actual;
              for (i = actual; (i < N_preguntas) && (strcmp(ejercicio_actual, PQgetvalue (res, i, 0)) == 0); i++) N_preguntas_ejercicio++;

              if (N_preguntas_ejercicio > 1)
                 fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

              if (*PQgetvalue (res, actual, 23) == 't') /* 23 => usa_header */
		 {
                  if (N_preguntas_ejercicio > 1)
		     {
  	              if (N_preguntas_ejercicio == 2)
	                 fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
	              else
	                 fprintf (Archivo_Latex, "\\textbf{Las preguntas %d a %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	              strcpy (hilera_antes, PQgetvalue (res, actual, 24)); /* 24 => texto_header */
                      hilera_LATEX (hilera_antes, hilera_despues);
                      fprintf (Archivo_Latex, "%s\n", hilera_despues);

                      fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
		     }
		  else
		     {
		      Imprime_pregunta (actual, res, Archivo_Latex, PQgetvalue(res,actual, 24));  /* 24 => texto_header */
		      N_preguntas_ejercicio = 0;
		      actual++;
		     }
		 }

	      for (i=0; i < N_preguntas_ejercicio; i++)
		  {
		   Imprime_pregunta (actual+i, res, Archivo_Latex, " ");
		  }

              if (N_preguntas_ejercicio > 1)
                 fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

              actual += N_preguntas_ejercicio;
             }

       fprintf (Archivo_Latex, "\\end{questions}\n");
      }
   else
      fprintf (Archivo_Latex, "No hay ejercicios registrados.\n\n");

   fprintf (Archivo_Latex, "\\onecolumn\n");
   fprintf (Archivo_Latex, "\\clearpage\n");
   fprintf  (Archivo_Latex, "\\SetWatermarkText{}\n");

   Resumen_de_Banderas (Archivo_Latex);

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX5030", 1, NULL, 0.0, 0.0, NULL, NULL);

   system ("rm EX5030x.pdf");

   gtk_widget_grab_focus (GTK_WIDGET(CB_materia));
}

void Imprime_pregunta (int i, PGresult * res, FILE * Archivo_Latex,char * prefijo)
{
   char alfileres[6];
   char hilera_antes [6000], hilera_despues [6000];
   long double media, varianza, desviacion;
   long double Por_A,Por_B,Por_C,Por_D,Por_E, Total;
   long double cota_inferior;
   PGresult * res_aux;
   char PG_command[2000];
   int j, N_links;
#define MINIMA 1.15
#define MAXIMA 6.65
   int flags [N_FLAGS];

   cota_inferior = MINIMA/MAXIMA*100.0;

   strcpy (alfileres, PQgetvalue (res, i, 40)); /* 40 => alfileres_opciones */

   sprintf (hilera_antes,"\\question %s\n%s", prefijo, PQgetvalue (res, i, 33)); /* 33 => texto_pregunta */
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   for (j=0; j < N_FLAGS;j++) flags[j] = 0;

   Total = (long double) atof(PQgetvalue (res, i, 61)); /* 61 => frecuencia */
   if (Total > 0.0)
      {
       Por_A = (long double) atof(PQgetvalue (res, i, 62))/Total*100.0;  /* 62 => frecuencia_1 */
       Por_B = (long double) atof(PQgetvalue (res, i, 63))/Total*100.0;  /* 63 => frecuencia_2 */
       Por_C = (long double) atof(PQgetvalue (res, i, 64))/Total*100.0;  /* 64 => frecuencia_3 */
       Por_D = (long double) atof(PQgetvalue (res, i, 65))/Total*100.0;  /* 65 => frecuencia_4 */
       Por_E = (long double) atof(PQgetvalue (res, i, 66))/Total*100.0;  /* 66 => frecuencia_5 */

       Asigna_Banderas (i, res, flags);
      }

   fprintf (Archivo_Latex, "\\begin{answers}\n");

   Imprime_Opcion (Archivo_Latex, res, alfileres, Total, Por_A, atoi(PQgetvalue (res, i, 62)), atof(PQgetvalue (res, i, 77)), *(PQgetvalue (res, i, 39)), i, 0);
   Imprime_Opcion (Archivo_Latex, res, alfileres, Total, Por_B, atoi(PQgetvalue (res, i, 63)), atof(PQgetvalue (res, i, 78)), *(PQgetvalue (res, i, 39)), i, 1);
   Imprime_Opcion (Archivo_Latex, res, alfileres, Total, Por_C, atoi(PQgetvalue (res, i, 64)), atof(PQgetvalue (res, i, 79)), *(PQgetvalue (res, i, 39)), i, 2);
   Imprime_Opcion (Archivo_Latex, res, alfileres, Total, Por_D, atoi(PQgetvalue (res, i, 65)), atof(PQgetvalue (res, i, 80)), *(PQgetvalue (res, i, 39)), i, 3);
   Imprime_Opcion (Archivo_Latex, res, alfileres, Total, Por_E, atoi(PQgetvalue (res, i, 66)), atof(PQgetvalue (res, i, 81)), *(PQgetvalue (res, i, 39)), i, 4);

   fprintf (Archivo_Latex, "\\end{answers}\n");

   Imprime_Banderas (Archivo_Latex, flags, i, res);

   fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{%.6s.%s - %.6s - %.6s - %s %s %s}\n\n", PQgetvalue (res, i, 0), PQgetvalue (res, i, 26), PQgetvalue (res, i, 9), PQgetvalue (res, i, 27), 
                                                                           PQgetvalue (res, i, 1), PQgetvalue (res, i,  2), PQgetvalue (res, i,  3));

   sprintf (PG_command,"SELECT * from bd_ejercicios where texto_ejercicio = '%.6s' and materia != '%s'", PQgetvalue (res, i, 9),PQgetvalue (res, i, 1));
   res_aux = PQEXEC(DATABASE, PG_command);
   N_links = PQntuples(res_aux);
   for (j=0;j<N_links;j++)
       {
        fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Ligada con ejercicio %.6s - %s %s %s}\n\n", PQgetvalue (res_aux, j, 0),
                                                                                         PQgetvalue (res_aux, j, 1), PQgetvalue (res_aux, j,  2), PQgetvalue (res_aux, j,  3));
       }
   PQclear(res_aux);

   sprintf (hilera_antes, "\\framebox[7.5cm][l]{Autor: %s}", PQgetvalue (res, i, 83));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n\n", hilera_despues);

   if (atoi(PQgetvalue (res, i, 61)))
      {
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Creada: %s/%s/%s - \\'{U}ltimo Uso: %s/%s/%s - %s %s}\n\n",    PQgetvalue (res, i, 31), PQgetvalue (res, i, 30), PQgetvalue (res, i, 29),
	  	                  PQgetvalue (res, i, 44), PQgetvalue (res, i, 43), PQgetvalue (res, i, 42), PQgetvalue (res, i, 61), strcmp("1",PQgetvalue (res, i, 61))==0?"vez":"veces");
      }
   else
      {
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Creada: %s/%s/%s - \\textbf{No ha sido usada}}\n\n",    PQgetvalue (res, i, 31), PQgetvalue (res, i, 30), PQgetvalue (res, i, 29));
      }

   media      = (long double) atof(PQgetvalue (res, i, 51));
   varianza   = (long double) atof(PQgetvalue (res, i, 52));
   desviacion = sqrt(varianza);
   fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Media: %6.4Lf - Varianza: %6.4Lf - Desviaci\\'{o}n: %6.4Lf}\n\n", media, varianza, desviacion);
   if (atoi(PQgetvalue (res, i, 61)))
      {
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{\\'{I}ndice de Discriminaci\\'{o}n ($r_{pb}$): %6.4Lf}\n\n", (long double)atof(PQgetvalue (res, i, 53)));
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{$\\alpha$ de Cronbach promedio: %6.4Lf (con) %6.4Lf (sin)}\n\n", 
                   (long double)atof(PQgetvalue (res, i, 58)), (long double)atof(PQgetvalue (res, i, 59)));
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Media de Ex\\'{a}menes: %6.2Lf - %6.2Lf (\\textbf{+}) - %6.2Lf (\\textbf{-})}\n\n", (long double)atof(PQgetvalue (res, i, 48)),(long double)atof(PQgetvalue (res, i, 54)),(long double)atof(PQgetvalue (res, i, 55)));
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Ha aparecido en %s %s}\n\n", PQgetvalue (res, i, 46), strcmp("1",PQgetvalue (res, i, 46))==0?"examen":"ex\\'{a}menes diferentes");
      }

   fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
   fprintf (Archivo_Latex, "\n\n");
}

void Imprime_Opcion (FILE * Archivo_Latex, PGresult *res, char * alfileres, long double Total, long double Porcentaje, int Frecuencia, long double Rpb, char correcta, int pregunta, int opcion)
{
   char hilera_antes [6000], hilera_despues [6000];

   fprintf (Archivo_Latex, "\\item ");
   if (alfileres [opcion] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   strcpy (hilera_antes, PQgetvalue (res, pregunta, 34 + opcion)); /* 34 ==> Inicio de textos de opciones */
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);

   if (Total > 0.0)
      {
       fprintf (Archivo_Latex,"\n\n{\\color{green} \\rule{%Lfcm}{5pt}\n\n{\\footnotesize \\textbf{\\texttt{%6.2Lf \\%%}}}} (\\textbf{%d})\n\n",
   	                          0.05+MAXIMA*Porcentaje/100.0, Porcentaje, Frecuencia);
       fprintf (Archivo_Latex," $r_{pb}$ = \\textbf{%Lf}\n\n", Rpb);
      }

   if (correcta == ('A' + opcion))
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }

   fprintf (Archivo_Latex, "\n\n");
}

void Asigna_Banderas (int i, PGresult * res, int * flags)
{
  int j, N_distractores_usados;

  for (j=0;j<N_FLAGS;j++) flags[j] = 0;

  if (atof(PQgetvalue (res, i, 51)) == 1.0) 
     flags[TODAS_BUENAS] = 1; /* 51 dificultad */

  if (atof(PQgetvalue (res, i, 51)) == 0.0) 
     flags[TODAS_MALAS]  = 1; /* 51 dificultad */

  if ((atof(PQgetvalue (res, i, 51)) >  0.0) && (atof(PQgetvalue (res, i, 51)) <=  0.3))
     flags[PORCENTAJE_MENOR_30]    = 1; /* 51 dificultad */

  if (atof(PQgetvalue (res, i, 53)) >= 0.3) 
     flags[RPB_BUENO]    = 1; /* 53 point_biserial */

  if (atof(PQgetvalue (res, i, 53)) < 0.0) 
     flags[RPB_NEGATIVO] = 1; /* 53 point_biserial */

  if ((atof(PQgetvalue (res, i, 59)) - atof(PQgetvalue (res, i, 58))) > 0.1)
     flags[ALFA_SUBE_MUCHO] = 1; /* 58 Cronbach con - 59 Cronbach sin */
  else
     if ((atof(PQgetvalue (res, i, 59)) - atof(PQgetvalue (res, i, 58))) > 0.04)
        flags[ALFA_SUBE] = 1; /* 58 Cronbach con - 59 Cronbach sin */

  N_distractores_usados = 0;
  for (j=0;j<5;j++)
      {
       if ((atof(PQgetvalue (res, i, 77+j)) > 0.3) && (*PQgetvalue (res, i, 39) != ('A' + j)))
          flags[RPB_DISTRACTOR_MUY_POSITIVO] = 1;
       else
          if ((atof(PQgetvalue (res, i, 77+j)) > 0.0) && (*PQgetvalue (res, i, 39) != ('A' + j)))
             flags[RPB_DISTRACTOR_POSITIVO] = 1;
	  else
             if ((atof(PQgetvalue (res, i, 77+j)) < -0.29) && (*PQgetvalue (res, i, 39) != ('A' + j)))
                flags[RPB_DISTRACTOR_MUY_NEGATIVO] = 1;

       if (atoi(PQgetvalue (res, i, 62 + j)) > 0) N_distractores_usados++;  /* 62 frecuencia_1 */
      }

  if (N_distractores_usados == 2)
     flags[SOLO_2_DISTRACTORES] = 1;
  else
     if ((N_distractores_usados == 3) || (N_distractores_usados == 4))
        flags[MENOS_DE_5_DISTRACTORES] = 1;
     else
        if (N_distractores_usados == 5)
           flags[USA_5_DISTRACTORES] = 1;

  if ((atof(PQgetvalue (res, i, 53)) >= 0.3) && (atof(PQgetvalue (res, i, 51)) <= 0.75))
     {
      if (N_distractores_usados <= 3) 
         flags[RPB_Y_DIFICULTAD_OK] = 1;
      else
	if ((N_distractores_usados == 5) && !flags[RPB_DISTRACTOR_MUY_POSITIVO])
           flags[RPB_DIFI_DISTRA_OK]  = 1;
     }
}

void Imprime_Banderas (FILE * Archivo_Latex, int * flags, int i, PGresult * res)
{
   char mensaje [1000];

   if (flags[RPB_BUENO])
      {
       sprintf (mensaje,"Esta pregunta muestra un buen \\'{i}ndice de discrimaci\\'{o}n ($r_{pb}$ = \\textbf{%6.4Lf}). Separa muy bien a los estudiantes de buen rendimiento de los de bajo rendimiento.",
		         (long double) atof(PQgetvalue (res, i, 53)));
       Cajita_con_bandera (Archivo_Latex, mensaje, AZUL);
      }

   if (flags[RPB_Y_DIFICULTAD_OK])
      {
       sprintf (mensaje,"Esta pregunta no es f\\'acil (\\textit{p} = \\textbf{%6.4Lf}) y tiene un buen \\'indice de discriminaci\\'on ($r_{pb}$ = \\textbf{%6.4Lf}). \\textbf{Buena pregunta}\n", 
		(long double) atof(PQgetvalue (res, i, 51)), (long double) atof(PQgetvalue (res, i, 53)));

       Cajita_con_bandera (Archivo_Latex, mensaje, AZUL);
      }

   if (flags[USA_5_DISTRACTORES])
      {
       sprintf (mensaje,"Todos los distractores han sido utilizados. \\textbf{Pregunta bien dise\\~nada}.");
       Cajita_con_bandera (Archivo_Latex, mensaje, CYAN);
      }

   if (flags[RPB_DISTRACTOR_MUY_NEGATIVO])
      {
       sprintf (mensaje,"Hay al menos un \\textbf{distractor} con un $r_{pb}$ muy negativo, que atrae a los estudiantes de bajo rendimiento y no es considerado por los estudiantes de buen rendimiento. \\textbf{Buen distractor}.");
       Cajita_con_bandera (Archivo_Latex, mensaje, CYAN);
      }

   if (flags[PORCENTAJE_MENOR_30])
      {
       sprintf (mensaje,"S\\'{o}lo el \\textbf{%5.2Lf}\\%% de los estudiantes evaluados han contestado correctamente esta pregunta.",
                       (long double) atof(PQgetvalue (res, i, 51)) * 100.0);
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO);
      }

   if (flags[ALFA_SUBE])
      {
       sprintf (mensaje,"En promedio, al eliminar esta pregunta de los ex\\'{a}menes donde aparece, el $\\alpha$ de Cronbach sube de \\textbf{%6.4Lf} a \\textbf{%6.4Lf}.",
		(long double) atof(PQgetvalue (res, i, 58)), (long double) atof(PQgetvalue (res, i, 59)));
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO);
      }

   if (flags[RPB_DISTRACTOR_POSITIVO])
      {
       sprintf (mensaje,"Hay al menos un \\textbf{distractor} con un $r_{pb}$ ligeramente positivo. Es necesario revisar el enunciado y las opciones.");
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO);
      }

   if (flags[MENOS_DE_5_DISTRACTORES])
      {
       sprintf (mensaje,"Hay por lo menos un distractor que \\textbf{nunca} ha atra\\'ido a los estudiantes. Mejorar redacci\\'on.");
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO);
      }

   if (flags[TODAS_BUENAS])
      {
       sprintf (mensaje,"\\textbf{Todos} los estudiantes han contestado correctamente, por lo que la pregunta no discrimina de manera efectiva.");
       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO);
      }

   if (flags[SOLO_2_DISTRACTORES])
      {
       sprintf (mensaje,"Los estudiantes s\\'olo se interesan en 2 distractores, los otros 3 han sido in\\'{u}tiles.");
       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO);
      }

   if (flags[TODAS_MALAS])
      {
       sprintf (mensaje,"Ni un solo estudiante ha contestado correctamente esta pregunta. La pregunta no discrimina de manera efectiva y posiblemente est\\'a mal redactada.");

       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO);
      }

   if (flags[RPB_NEGATIVO])
      {
       sprintf (mensaje, "Esta pregunta muestra un \\'{i}ndice de discrimaci\\'{o}n ($r_{pb}$ = \\textbf{%6.4Lf}) negativo. Los estudiantes de buen rendimiento en este examen tendieron a equivocarse, mientras que los de bajo rendimiento tendieron a contestarla bien. \\textbf{Revisar muy bien el enunciado y las opciones de la pregunta}",
                         (long double) atof(PQgetvalue (res, i, 53)));

       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO);
      }

   if (flags[ALFA_SUBE_MUCHO])
      {
       sprintf (mensaje, "Si esta pregunta se elimina del examen, el $\\alpha$ de Cronbach subir\\'{i}a \\textbf{considerablemente} (de \\textbf{%6.4Lf} a \\textbf{%6.4Lf}).",
                         (long double) atof(PQgetvalue (res, i, 58)), (long double) atof(PQgetvalue (res, i, 59)));

       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO);
      }

   if (flags[RPB_DIFI_DISTRA_OK])
      {
       sprintf (mensaje,"Esta pregunta no es f\\'acil (\\textit{p} = \\textbf{%6.4Lf}), muestra un buen \\'indice de discriminaci\\'on ($r_{pb}$ = \\textbf{%6.4Lf}), y los estudiantes usaron todos los distractores. \\textbf{Excelente pregunta}\n", 
		(long double) atof(PQgetvalue (res, i, 51)), (long double) atof(PQgetvalue (res, i, 53)));

       Cajita_con_bandera (Archivo_Latex, mensaje, VERDE);
      }
}

void Cajita_con_bandera (FILE * Archivo_Latex, char * mensaje, int color)
{
   int i;

   Banderas [color]++;

   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{4\\fboxrule}\n");
   fprintf (Archivo_Latex, "\\fcolorbox{%s}{white}{\n", Colores_Banderas[color]);
   fprintf (Archivo_Latex, "\\begin{minipage}{1.4 cm}\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=0.07, angle=45]{.imagenes/%s}\n", banderas[color]);
   fprintf (Archivo_Latex, "\\end{minipage}\n");
   fprintf (Archivo_Latex, "\\begin{minipage}{6.1 cm}\n");
   fprintf (Archivo_Latex, "\\small{%s}\n", mensaje); 
   fprintf (Archivo_Latex, "\\end{minipage}\n");
   fprintf (Archivo_Latex, "}\n");
   fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.25\\fboxrule}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n");
}

void Resumen_de_Banderas (FILE * Archivo_Latex)
{
   int color;

   fprintf (Archivo_Latex, "\\center{\\textbf{Resumen de Observaciones}}\n\n");

   for (color=0; color<5; color++)
       {
        fprintf (Archivo_Latex, "\\begin{minipage}{3.50 cm}\n");
        fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
        fprintf (Archivo_Latex, "\\centering\n");
        fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{4\\fboxrule}\n");
        fprintf (Archivo_Latex, "\\fcolorbox{%s}{white}{\n", Colores_Banderas[color]);
        fprintf (Archivo_Latex, "\\begin{minipage}{1.5 cm}\n");
        fprintf (Archivo_Latex, "\\includegraphics[scale=0.07, angle=45]{.imagenes/%s}\n", banderas[color]);
        fprintf (Archivo_Latex, "\\end{minipage}\n");
        fprintf (Archivo_Latex, "\\begin{minipage}{0.5 cm}\n");
        fprintf (Archivo_Latex, "\\large{\\textbf{%d}}\n", Banderas[color]); 
        fprintf (Archivo_Latex, "\\end{minipage}\n");
        fprintf (Archivo_Latex, "}\n");
        fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.25\\fboxrule}\n");
        fprintf (Archivo_Latex, "\\end{figure}\n");
        fprintf (Archivo_Latex, "\\end{minipage}\n");
       }
}


/*********************************************************************/
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
void on_WN_ex5030_destroy (GtkWidget *widget, gpointer user_data) 
{
  Fin_Ventana (widget, user_data);
}

void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data)
{
  Fin_de_Programa (widget, user_data);
}

void on_BN_proceso_clicked(GtkWidget *widget, gpointer user_data)
{
  Prepara_graficos (widget, user_data);
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

void on_CB_subtema_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * subtema;

  subtema = gtk_combo_box_get_active_text(CB_subtema);

  if (subtema)
     {
      g_free (subtema);
      Cambio_Subtema (widget, user_data);
     }
  else
     g_free (subtema);
}

void on_EB_right_button_press_event(GtkWidget *event_box, GdkEventButton *event, gpointer data)
{
  Despliegue_preguntas ((int) event->x, (int) event->y);
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_print_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_listado ();
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
}

void on_SC_rotacion_value_changed(GtkWidget *widget, gpointer user_data)
{
  Prepara_graficos (widget, user_data);
}

void on_SC_color_value_changed(GtkWidget *widget, gpointer user_data)
{
  Prepara_graficos (widget, user_data);
}

void on_SC_resolucion_value_changed(GtkWidget *widget, gpointer user_data)
{
  Prepara_graficos (widget, user_data);
}

