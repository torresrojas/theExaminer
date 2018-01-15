/*
Compilar con

cc -o EX3000 EX3000.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX3000:                                */
/*                                         */
/*    Crea un Esquema de Examen            */
/*    The Examiner 0.0                     */
/*    13 de Julio 2011                     */
/*                                         */
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Se incluye scroll window             */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    28 de Enero del 2012                 */
/*******************************************/
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <libpq-fe.h>
#include <locale.h>
#include "examiner.h"

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define CODIGO_SIZE              6
#define DESCRIPCION_SIZE         201
#define MINIMO(a,b) ((a)<(b))?(a):(b)
#define TOTAL_LINEAS             80

char Codigo_materia [CODIGO_MATERIA_SIZE+1];
int  Lineas_Usadas;

struct SELECCION
{
  char tema    [500];
  char subtema [500];
  int          orden_Tema;
  int          orden_Subtema;
  long double  cantidad;
  long double  maximo;
};

struct SELECCION Tabla_Selecciones [TOTAL_LINEAS+1];

int N_materias=0, N_temas=0, N_subtemas =0;

#define REFRESH 0
#define INPUT   1
int MODO = REFRESH;
int ESQUEMA_USADO=0;
#define MAX_COLORES 18
#define REPORTE_PORCENTAJE 0
#define REPORTE_CANTIDADES 1

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

GtkWidget *EB_tema_subtema        = NULL;
GtkWidget *FR_tema_subtema        = NULL;
GtkWidget *EB_cantidades          = NULL;
GtkWidget *FR_cantidades          = NULL;

GtkSpinButton *SP_codigo          = NULL;

GtkWidget *EN_descripcion         = NULL;
GtkComboBox *CB_materia           = NULL;
GtkWidget *EN_total               = NULL;

GtkWidget *FR_lineas              = NULL;
GtkWidget *FR_botones             = NULL;
GtkWidget *FR_ordenamiento        = NULL;
GtkWidget *FR_reporte             = NULL;
GtkWidget *FR_esquemausado        = NULL;

GtkComboBox *CB_tema              [TOTAL_LINEAS+1];
GtkComboBox *CB_subtema           [TOTAL_LINEAS+1];
GtkSpinButton *SP_N               [TOTAL_LINEAS+1];
GtkWidget *EN_porcentaje          [TOTAL_LINEAS+1];

GtkWidget *BN_tema                = NULL;
GtkWidget *BN_orden               = NULL;
GtkWidget *BN_porcentaje          = NULL;

GtkWidget *BN_delete              = NULL;
GtkWidget *BN_saveas              = NULL;
GtkWidget *BN_save                = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_terminar            = NULL;
GtkWidget *BN_print               = NULL;

GtkWidget *window2                = NULL;
GtkWidget *EN_materia             = NULL;
GtkWidget *EN_materia_descripcion = NULL;
GtkWidget *EN_pagina              = NULL;
GtkToggleButton *RB_base          = NULL;
GtkToggleButton *RB_porcentaje    = NULL;
GtkToggleButton *RB_cantidades    = NULL;

GtkWidget *BN_ok                  = NULL;

GtkWidget *window3                = NULL;
GtkWidget *EB_genera_latex        = NULL;
GtkWidget *FR_genera_latex        = NULL;
GtkWidget *PB_genera_latex        = NULL;

GtkWidget *window4                = NULL;
GtkWidget *EB_update              = NULL;
GtkWidget *FR_update              = NULL;
GtkWidget *BN_ok2                 = NULL;

GtkWidget *window5                = NULL;
GtkWidget *EB_delete              = NULL;
GtkWidget *FR_delete              = NULL;
GtkWidget *BN_ok3                 = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Actualiza_Total_Preguntas  ();
void Avanza_Linea               (int N_preguntas);
void Borra_Esquema              (GtkWidget *widget, gpointer user_data);
int  Busca_Tema                 (char * tema);
int  Busca_Tema_Subtema         (char * tema, char * subtema);
void Cambio_Codigo              ();
void Cambia_Descripcion         (GtkWidget *widget, gpointer user_data);
void Cambio_Materia             (GtkWidget *widget, gpointer user_data);
void Cambio_Subtema             (GtkWidget *widget, gpointer user_data, int i);
void Cambio_Tema                (GtkWidget *widget, gpointer user_data, int i);
void Carga_Esquema              (PGresult *res);
void Carga_Subtema              ();
void Carga_Tema                 ();
void colores_pastel             (FILE * Archivo_Latex);
void Congela_Lineas             ();
void Connect_Widgets            ();
void Copia_interfaz_a_tabla     (struct SELECCION * Tabla, int N);
void Copia_tabla_a_interfaz     (struct SELECCION * Tabla, int N);
void Crea_archivo_datos_pastel  (int *Empates);
void Datos_ultimo_tema          ();
void Extrae_codigo              (char * hilera, char * codigo, int N);
void Extrae_descripcion         (char * hilera, char * descripcion);
void Fin_de_Programa            (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                (GtkWidget *widget, gpointer user_data);
void Graba_Descripcion          ();
void Graba_Esquema              (GtkWidget *widget, gpointer user_data);
void Graba_Esquema_Como         (GtkWidget *widget, gpointer user_data);
void Init_Fields                ();
void Interface_Coloring         ();
void Limpia_Lineas              ();
void on_BN_delete_clicked       (GtkWidget *widget, gpointer user_data);
void on_BN_ok_clicked           (GtkWidget *widget, gpointer user_data);
void on_BN_orden_clicked        (GtkWidget *widget, gpointer user_data);
void on_BN_porcentaje_clicked   (GtkWidget *widget, gpointer user_data);
void on_BN_save_clicked         (GtkWidget *widget, gpointer user_data);
void on_BN_saveas_clicked       (GtkWidget *widget, gpointer user_data);
void on_BN_tema_clicked         (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked     (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked         (GtkWidget *widget, gpointer user_data);
void on_CB_materia_changed      (GtkWidget *widget, gpointer user_data);
void on_CB_subtema_changed      (GtkWidget *widget, gpointer user_data);
void on_CB_tema_changed         (GtkWidget *widget, gpointer user_data);
void on_EN_descripcion_activate (GtkWidget *widget, gpointer user_data);
void on_SP_codigo_activate      (GtkWidget *widget, gpointer user_data);
void on_WN_ex3000_destroy       (GtkWidget *widget, gpointer user_data);
void Ordena_por_cantidad        ();
void Ordena_por_tema            ();
void Ordena_por_presentacion    ();
void Reduce_Subtema             (int i);
void Reduce_Tema                (int i);
void Reporte_base               (FILE * Archivo_Latex);
void Reporte_pastel             (FILE * Archivo_Latex, int tipo_reporte);

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX3000.glade", &error))
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

          gtk_widget_set_can_focus(EN_total, FALSE); /* Campo READ-ONLY */
	  Lineas_Usadas = 0;
          Init_Fields ();
          MODO = INPUT;

	  /* Displays main window */
          gtk_widget_show (window1);

          /* start the event loop */
          gtk_main();
	 }
     }

  return 0;
}

void Connect_Widgets()
{
  int i;
  char hilera[100];

  window1              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex3000"));
  window2              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex3000_usado"));
  window3              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_genera_latex"));
  window4              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));
  window5              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_delete"));

  EB_tema_subtema         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_tema_subtema"));
  FR_tema_subtema         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_tema_subtema"));
  EB_cantidades         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_cantidades"));
  FR_cantidades         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_cantidades"));

  SP_codigo            = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_codigo"));
  EN_descripcion       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));
  CB_materia           = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_materia"));
  EN_total             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_total"));

  for (i=1; i <= TOTAL_LINEAS; i++)
      {
       sprintf (hilera, "CB_tema%02d", i);
       CB_tema                [i-1] = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder,  hilera));

       sprintf (hilera, "CB_subtema%02d", i);
       CB_subtema             [i-1] = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder,  hilera));

       sprintf (hilera, "SP_N%02d", i);
       SP_N                   [i-1] =  (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder,  hilera));

       sprintf (hilera, "EN_porcentaje%02d", i);
       EN_porcentaje [i-1] = GTK_WIDGET (gtk_builder_get_object (builder,  hilera));
       gtk_widget_set_can_focus(EN_porcentaje[i-1], FALSE); /* Campo READ-ONLY */

       gtk_widget_set_visible  (GTK_WIDGET(CB_tema    [i-1]), FALSE);
       gtk_widget_set_visible  (GTK_WIDGET(CB_subtema [i-1]), FALSE);
       gtk_widget_set_visible  (GTK_WIDGET(SP_N       [i-1]), FALSE);
       gtk_widget_set_visible  (EN_porcentaje         [i-1] , FALSE);
      }

  FR_lineas       = GTK_WIDGET (gtk_builder_get_object (builder, "FR_lineas"));
  FR_botones      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  FR_reporte      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reporte"));
  FR_ordenamiento = GTK_WIDGET (gtk_builder_get_object (builder, "FR_ordenamiento"));
  FR_esquemausado = GTK_WIDGET (gtk_builder_get_object (builder, "FR_esquemausado"));

  BN_tema         = GTK_WIDGET (gtk_builder_get_object (builder, "BN_tema"));
  BN_orden        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_orden"));
  BN_porcentaje   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_porcentaje"));

  BN_delete       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_delete"));
  BN_save         = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
  BN_saveas       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_saveas"));
  BN_undo         = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_terminar     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_print        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  RB_base         = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_base"));
  RB_porcentaje   = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_porcentaje"));
  RB_cantidades   = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_cantidades"));

  EB_genera_latex         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_genera_latex"));
  FR_genera_latex         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_genera_latex"));
  PB_genera_latex         = GTK_WIDGET (gtk_builder_get_object (builder, "PB_genera_latex"));

  BN_ok             = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));

  EB_update         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
  FR_update         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));
  BN_ok2            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok2"));

  EB_delete         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_delete"));
  FR_delete         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_delete"));
  BN_ok3            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok3"));
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_cantidades,          GTK_STATE_NORMAL, &color);

  gdk_color_parse (SECONDARY_AREA, &color);
  gtk_widget_modify_bg(EB_tema_subtema,        GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_lineas,             GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones,            GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_ordenamiento,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_reporte,            GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_total,              GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_codigo), GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_descripcion,        GTK_STATE_NORMAL, &color);

  gdk_color_parse (IMPORTANT_WINDOW, &color);
  gtk_widget_modify_bg(window2, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (IMPORTANT_FR, &color);
  gtk_widget_modify_bg(FR_esquemausado,      GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_tema,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_orden,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_porcentaje,   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_saveas,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_delete,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,           GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok2,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok3,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(GTK_WIDGET(RB_base),         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(GTK_WIDGET(RB_porcentaje),   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(GTK_WIDGET(RB_cantidades),   GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_tema,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_orden,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_porcentaje,   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_saveas,       GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_delete,       GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,           GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok2,          GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok3,          GTK_STATE_ACTIVE, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_delete,              GTK_STATE_NORMAL, &color);

  gdk_color_parse (FINISHED_WORK_FR, &color);
  gtk_widget_modify_bg(FR_update,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_delete,              GTK_STATE_NORMAL, &color);

  gdk_color_parse (PROCESSING_WINDOW, &color);
  gtk_widget_modify_bg(EB_genera_latex,  GTK_STATE_NORMAL,   &color); 

  gdk_color_parse (PROCESSING_FR, &color);
  gtk_widget_modify_bg(FR_genera_latex,       GTK_STATE_NORMAL, &color);

  gdk_color_parse (PROCESSING_PB, &color);
  gtk_widget_modify_bg(GTK_WIDGET(PB_genera_latex),  GTK_STATE_PRELIGHT, &color);
}

void Init_Fields()
{
   int  i, j, Last;
   char Codigo              [CODIGO_SIZE];
   char hilera [400];

   Last = 0;
   res = PQEXEC(DATABASE, "SELECT codigo_esquema from EX_esquemas order by codigo_esquema DESC limit 1");
   if (PQntuples(res)) Last = atoi (PQgetvalue (res, 0, 0));

   Last++; /* siguiente esquema */
   sprintf (Codigo,"%05d",Last);
   PQclear(res);

   if (Last > 1)
      {
       gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo), 1);
       gtk_spin_button_set_range (SP_codigo, 1.0, (long double) Last);
      }
   else
      { /* Primer esquema que se realiza */
       gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo), 0);
       gtk_spin_button_set_range (SP_codigo, 0.0, (long double) Last);
      }
   gtk_spin_button_set_value (SP_codigo, Last);

   gtk_entry_set_text(GTK_ENTRY(EN_total) , "     0");

   gtk_widget_set_sensitive(EN_descripcion         , 0);
   gtk_widget_set_sensitive(GTK_WIDGET (CB_materia), 0);
   gtk_widget_set_sensitive(EN_total               , 0);

   Limpia_Lineas  ();

   gtk_combo_box_set_active (CB_materia, -1);

   for (i=0; i < N_materias; i++) gtk_combo_box_remove_text (CB_materia, 0);

   res = PQEXEC(DATABASE, "SELECT DISTINCT codigo_materia, descripcion_materia from bd_materias, bd_ejercicios, bd_texto_preguntas where materia = codigo_materia and texto_ejercicio = codigo_consecutivo_ejercicio and codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_materia");
   N_materias = PQntuples(res);
 
   for (i=0; i < N_materias; i++)
       {
	sprintf (hilera,"%s %s", PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
        gtk_combo_box_append_text(CB_materia, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_materia, -1);

   ESQUEMA_USADO = 0;

   gtk_entry_set_text(GTK_ENTRY(EN_descripcion),"*** Descripción de Esquema ***");

   gtk_widget_set_sensitive(FR_ordenamiento, 0);
   gtk_widget_set_sensitive(FR_reporte,      0);
   gtk_toggle_button_set_active(RB_cantidades, TRUE);
   gtk_widget_set_sensitive(BN_delete,   0);
   gtk_widget_set_sensitive(BN_saveas,   0);
   gtk_widget_set_sensitive(BN_save,     0);

   if (Last > 1)
      {
       gtk_widget_set_sensitive(GTK_WIDGET (SP_codigo), 1);
       gtk_widget_grab_focus   (GTK_WIDGET (SP_codigo));
      }
   else
      {
       gtk_widget_set_sensitive(GTK_WIDGET (SP_codigo), 0);
       on_SP_codigo_activate (NULL, NULL);
      }
}

void Limpia_Lineas()
{
  int i;
  gchar * tema, *subtema;
  int MODO_PREVIO;

  MODO_PREVIO = MODO;
  MODO = REFRESH;

  for (i=0; i < Lineas_Usadas; i++)
      {
       gtk_combo_box_remove_text (CB_tema       [i], 0); /* Sólo tienen una */
       gtk_combo_box_remove_text (CB_subtema    [i], 0); /* Sólo tienen una */

       gtk_widget_set_visible    (GTK_WIDGET(CB_tema       [i]), FALSE);
       gtk_widget_set_visible    (GTK_WIDGET(CB_subtema    [i]), FALSE);
       gtk_widget_set_visible    (GTK_WIDGET(SP_N          [i]), FALSE);
       gtk_widget_set_sensitive  (GTK_WIDGET(SP_N          [i]), 1); /* Podría estar congelado */
       gtk_widget_set_visible    (EN_porcentaje [i], FALSE);
      }

  if (Lineas_Usadas < TOTAL_LINEAS)
     {
      for (i=0; i<N_temas;    i++) gtk_combo_box_remove_text (CB_tema   [Lineas_Usadas], 0);
      for (i=0; i<N_subtemas; i++) gtk_combo_box_remove_text (CB_subtema[Lineas_Usadas], 0);
      gtk_widget_set_visible    (GTK_WIDGET(CB_tema       [Lineas_Usadas]), FALSE);
      gtk_widget_set_visible    (GTK_WIDGET(CB_subtema    [Lineas_Usadas]), FALSE);
      gtk_widget_set_visible    (GTK_WIDGET(SP_N [Lineas_Usadas]), FALSE);
      gtk_widget_set_visible    (EN_porcentaje   [Lineas_Usadas], FALSE);
     }

  Lineas_Usadas = 0;
  N_temas = N_subtemas = 0;

  MODO = MODO_PREVIO;
}

void Cambio_Codigo()
{
  long int k;
  char codigo[10];
  char PG_command [1000];
  PGresult *res;

  k = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
  sprintf (codigo,"%05ld", k);

  sprintf (PG_command,"SELECT * from EX_esquemas where codigo_esquema = '%.5s'", codigo);

  res = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res))
     { /* Esquema ya existe... */
      Carga_Esquema (res);
      gtk_widget_set_sensitive (BN_saveas, 1);
      gtk_widget_set_sensitive (BN_save,   1);
     }
  else
     {
      ESQUEMA_USADO = 0;
      gtk_widget_set_sensitive(GTK_WIDGET (CB_materia), 1);
      gtk_widget_grab_focus   (GTK_WIDGET (CB_materia));
     }

  PQclear(res);

  gtk_widget_set_sensitive(GTK_WIDGET (SP_codigo), 0);
}

void Carga_Esquema (PGresult *res)
{
  PGresult *res_aux, *res_aux2;
  char PG_command [1000];
  int i, maximo;
  char hilera [500];
  struct SELECCION Tabla [TOTAL_LINEAS];
  int MODO_PREVIO;

  MODO_PREVIO = MODO;
  MODO = REFRESH;

  for (i=0; i < N_materias; i++) gtk_combo_box_remove_text (CB_materia, 0);

  sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", PQgetvalue(res, 0, 1));
  res_aux = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res_aux))
     sprintf(hilera,"%s %s",PQgetvalue(res, 0, 1),PQgetvalue(res_aux, 0, 0));
  else
     sprintf(hilera,"%s ** Código Inválido de Materia **",PQgetvalue(res, 0, 1));
  PQclear(res_aux);

  gtk_combo_box_append_text(CB_materia, hilera);
  gtk_combo_box_set_active (CB_materia, 0);
  N_materias = 1;

  strcpy (Codigo_materia, PQgetvalue(res, 0, 1));

  gtk_entry_set_text       (GTK_ENTRY(EN_descripcion), PQgetvalue(res, 0, 5));

  sprintf (PG_command,"SELECT * from EX_esquemas_lineas where esquema = '%s' order by tema,subtema", PQgetvalue(res, 0, 0));
  res_aux = PQEXEC(DATABASE, PG_command);
  for (i=0;i<PQntuples(res_aux);i++)
      {
       sprintf (PG_command,"SELECT tema from bd_texto_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and materia = '%s' and tema = '%s' and automatico",
                           PQgetvalue(res,0,1), PQgetvalue(res_aux, i, 1));
       res_aux2 = PQEXEC(DATABASE, PG_command);
       maximo = PQntuples(res_aux2);
       PQclear(res_aux2);

       if (maximo)
	  {
           if (strcmp( PQgetvalue(res_aux, i, 2),CODIGO_VACIO) == 0)
	      { /* NO se usa subtema y sabemos que hay preguntas del tema */
               sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", 
                        PQgetvalue(res, 0, 1), PQgetvalue(res_aux, i, 1));
               res_aux2 = PQEXEC(DATABASE, PG_command);
               if (PQntuples(res_aux2))
                  sprintf(hilera,"%s %s",PQgetvalue(res_aux, i, 1),PQgetvalue(res_aux2, 0, 0));
               else
                  sprintf(hilera,"%s ** Código Inválido de Tema **",PQgetvalue(res_aux, i, 1));
               PQclear(res_aux2);

               strcpy (Tabla[Lineas_Usadas].tema, hilera);

               strcpy (Tabla[Lineas_Usadas].subtema, CODIGO_VACIO);

               Tabla[Lineas_Usadas].cantidad = MINIMO(atof(PQgetvalue(res_aux, i, 3)), (long double) maximo);
               Tabla[Lineas_Usadas].maximo   = (long double) maximo;

	       Lineas_Usadas++;
	      }
	   else
	     { /* Se usa subtema */
 	      sprintf (PG_command,"SELECT tema from bd_texto_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and materia = '%s' and tema = '%s' and subtema = '%s' and automatico",
                       PQgetvalue(res,0,1), PQgetvalue(res_aux, i, 1),PQgetvalue(res_aux, i, 2));
              res_aux2 = PQEXEC(DATABASE, PG_command);
              maximo = PQntuples(res_aux2);
              PQclear(res_aux2);
	      if (maximo)
	         { /* Hay preguntas */
                  sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '     '", 
                           PQgetvalue(res, 0, 1), PQgetvalue(res_aux, i, 1));
                  res_aux2 = PQEXEC(DATABASE, PG_command);
                  if (PQntuples(res_aux2))
                     sprintf(hilera,"%s %s",PQgetvalue(res_aux, i, 1),PQgetvalue(res_aux2, 0, 0));
                  else
                     sprintf(hilera,"%s ** Código Inválido de Tema **",PQgetvalue(res_aux, i, 1));
                  PQclear(res_aux2);

                  strcpy (Tabla[Lineas_Usadas].tema, hilera);

                  sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", 
                           PQgetvalue(res, 0, 1), PQgetvalue(res_aux, i, 1),PQgetvalue(res_aux, i, 2));
                  res_aux2 = PQEXEC(DATABASE, PG_command);
                  if (PQntuples(res_aux2))
                     sprintf(hilera,"%s %s",PQgetvalue(res_aux, i, 2),PQgetvalue(res_aux2, 0, 0));
                  else
                     sprintf(hilera,"%s ** Código Inválido de Subtema **",PQgetvalue(res_aux, i, 2));
                  PQclear(res_aux2);

                  strcpy (Tabla[Lineas_Usadas].subtema, hilera);

                  Tabla[Lineas_Usadas].cantidad = MINIMO(atoi(PQgetvalue(res_aux, i, 3)), (long double) maximo);
                  Tabla[Lineas_Usadas].maximo   = (long double) maximo;

	          Lineas_Usadas++;
		 }
	      }
	  }
      }

  PQclear(res_aux);

  Copia_tabla_a_interfaz (Tabla, Lineas_Usadas);
  gtk_widget_set_sensitive(EN_total,       1);
  Actualiza_Total_Preguntas ();

  sprintf (PG_command,"SELECT * from EX_pre_examenes where esquema = '%s'", PQgetvalue(res, 0, 0));
  res_aux = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res_aux))
     {
      /* Si el esquema ya está siendo usado, no puede ser alterado, solo copiado */
      ESQUEMA_USADO = 1;

      gtk_widget_set_sensitive(BN_delete,     0);

      Congela_Lineas ();
      gtk_widget_set_sensitive(FR_reporte,      1);

      gtk_widget_set_sensitive(window1, 0);
      gtk_widget_show (window2);
      gtk_widget_grab_focus    (EN_descripcion);
     }
  else
     {
      Carga_Tema ();

      if (!Lineas_Usadas)
         {
          gtk_widget_set_sensitive (BN_delete, 1);
          gtk_widget_grab_focus    (EN_descripcion);
         }
      else
	 {
          gtk_widget_set_sensitive(FR_ordenamiento, 1);
          gtk_widget_set_sensitive(FR_reporte,      1);
	  if (Lineas_Usadas < TOTAL_LINEAS)
	     {
              gtk_widget_set_visible   (GTK_WIDGET (CB_tema[Lineas_Usadas]), TRUE);
              gtk_widget_set_sensitive (GTK_WIDGET (CB_tema[Lineas_Usadas]), 1);
              gtk_combo_box_set_active (CB_tema[Lineas_Usadas], -1);
              gtk_widget_grab_focus    (GTK_WIDGET (CB_tema[Lineas_Usadas]));
	     }
	  else
             gtk_widget_grab_focus    (EN_descripcion);

	 }
     }


  gtk_widget_set_sensitive (EN_descripcion, 1);

  MODO = MODO_PREVIO;
}

void Congela_Lineas ()
{
  int i, j;
  int MODO_PREVIO;

  /* Congela la interfaz */
  MODO_PREVIO = MODO;
  MODO = REFRESH;

  for (i=0; i < Lineas_Usadas; i++)
      {
       gtk_widget_set_sensitive(GTK_WIDGET (CB_tema    [i]), 0);
       gtk_widget_set_sensitive(GTK_WIDGET (CB_subtema [i]), 0);
       gtk_widget_set_sensitive(GTK_WIDGET(SP_N       [i]), 0);
      }
  MODO = MODO_PREVIO;
}

void Cambio_Materia(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *materia;
  char PG_command  [1000];
  PGresult *res, *res_aux;

  materia = gtk_combo_box_get_active_text(CB_materia);
  Extrae_codigo (materia, Codigo_materia, CODIGO_MATERIA_SIZE + 1);

  Carga_Tema();

  gtk_widget_set_sensitive (GTK_WIDGET (CB_materia), 0);
  gtk_widget_set_sensitive (EN_descripcion, 1);

  gtk_widget_set_sensitive (BN_save,       1);

  gtk_widget_set_sensitive (EN_total,  1);

  g_free (materia);
  gtk_widget_grab_focus    (EN_descripcion);
}

void Cambia_Descripcion(GtkWidget *widget, gpointer user_data)
{
  if (!Lineas_Usadas && !ESQUEMA_USADO)
     {
      gtk_widget_set_visible   (GTK_WIDGET (CB_tema[0]), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (CB_tema[0]), 1);
      gtk_combo_box_set_active (CB_tema[0], -1);
      gtk_widget_grab_focus    (GTK_WIDGET(CB_tema[0]));
     }
  else
     gtk_widget_grab_focus    (GTK_WIDGET(SP_N[0]));
}

void Cambio_Tema (GtkWidget *widget, gpointer user_data, int i)
{
  int j;
  gchar *tema = NULL;
  char  PG_command  [1000];
  PGresult *res, *res_aux;
  char Codigo_tema [CODIGO_TEMA_SIZE + 1];

  tema    = gtk_combo_box_get_active_text(CB_tema [i]);

  if (tema)
     {
      if (strcmp(tema,CODIGO_VACIO) != 0)
         {
	  gtk_widget_set_sensitive (GTK_WIDGET(CB_tema [i]), 0);

	  Reduce_Tema (i);

	  Carga_Subtema ();

          gtk_widget_set_visible  (GTK_WIDGET(CB_subtema [i]), TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(CB_subtema [i]), 1);
          gtk_widget_grab_focus   (GTK_WIDGET(CB_subtema [i]));
         }
      else
         {
	  gtk_widget_grab_focus (GTK_WIDGET(CB_tema [i]));
         }
     }

  g_free (tema);
}

void Cambio_Subtema (GtkWidget *widget, gpointer user_data, int i)
{
  gchar *tema, *subtema = NULL;
  char PG_command  [2000];
  int  N_preguntas=0;
  PGresult *res, *res_aux;
  char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
  char Codigo_subtema [CODIGO_SUBTEMA_SIZE + 1];

  tema    = gtk_combo_box_get_active_text(CB_tema    [i]);
  subtema = gtk_combo_box_get_active_text(CB_subtema [i]);
  Extrae_codigo (tema   , Codigo_tema,    CODIGO_TEMA_SIZE    + 1);
  Extrae_codigo (subtema, Codigo_subtema, CODIGO_SUBTEMA_SIZE + 1);

  if (subtema)
     {
      if (strcmp(subtema, CODIGO_VACIO) != 0)
         {
	  sprintf (PG_command,"SELECT tema from bd_texto_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and materia = '%s' and tema = '%s' and subtema = '%s' and automatico",Codigo_materia, Codigo_tema, Codigo_subtema);
          res_aux = PQEXEC(DATABASE, PG_command);
          N_preguntas = PQntuples(res_aux);
          PQclear(res_aux);

	  if (N_preguntas)
	     {
	      gtk_widget_set_sensitive (GTK_WIDGET(CB_subtema [i]), 0);
	      Reduce_Subtema (i);
   	      Avanza_Linea (N_preguntas);
	     }
	  else
	     { /* No debiera darse porque solo se cargan los subtemas para los que hay preguntas*/
	      gtk_widget_grab_focus (GTK_WIDGET(CB_subtema[i]));
	     }
         }
      else
	 { /* No se va a usar subtema */
	  gtk_widget_set_visible   (GTK_WIDGET(CB_subtema [i]), FALSE);
	  gtk_widget_set_can_focus (GTK_WIDGET(CB_subtema [i]), FALSE);

	  sprintf (PG_command,"SELECT tema from bd_texto_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and materia = '%s' and tema = '%s' and automatico",Codigo_materia,Codigo_tema);

          res_aux = PQEXEC(DATABASE, PG_command);
          N_preguntas = PQntuples(res_aux);
          PQclear(res_aux);

	  if (N_preguntas)
	     {
	      Reduce_Subtema (i);
              Avanza_Linea (N_preguntas);
	     }
	  else
	     { /* Caso no debiera darse... */
	      gtk_widget_set_visible  (GTK_WIDGET(CB_subtema [i]), FALSE);
	      gtk_widget_set_sensitive(GTK_WIDGET(CB_subtema [i]), 0);
	      gtk_widget_set_sensitive(GTK_WIDGET(CB_tema    [i]), 1);
	      gtk_widget_grab_focus   (GTK_WIDGET(CB_tema    [i]));
	     }
	}
     }

  g_free (subtema);
  g_free (tema);
}

void Avanza_Linea(int N_preguntas)
{
  if (!Lineas_Usadas)
     {
      gtk_widget_set_sensitive (BN_delete, 0);
      gtk_widget_set_sensitive (BN_saveas, 0);
      gtk_widget_set_sensitive (FR_ordenamiento, 1);
      gtk_widget_set_sensitive (FR_reporte,      1);
     }

  Lineas_Usadas++; /* Se incluye una línea al esquema - sólo en interfaz de momento*/

  gtk_spin_button_set_range (SP_N     [Lineas_Usadas-1], (long double) 0.0, (long double) N_preguntas);
  gtk_spin_button_set_value (SP_N     [Lineas_Usadas-1], (long double)                     N_preguntas);
  gtk_widget_set_visible   (GTK_WIDGET(SP_N          [Lineas_Usadas-1]), TRUE);
  gtk_widget_set_sensitive (GTK_WIDGET(SP_N          [Lineas_Usadas-1]), 1);

  gtk_widget_set_visible   (EN_porcentaje [Lineas_Usadas-1], TRUE);
  gtk_widget_set_sensitive (EN_porcentaje [Lineas_Usadas-1], 1);


  if (Lineas_Usadas <  TOTAL_LINEAS)
     {
      Carga_Tema();
      gtk_widget_set_visible   (GTK_WIDGET(CB_tema [Lineas_Usadas]), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_tema [Lineas_Usadas]), 1);
      gtk_widget_grab_focus    (GTK_WIDGET(CB_tema [Lineas_Usadas]));
     }
  else
     {
      gtk_widget_grab_focus    (GTK_WIDGET(CB_tema [Lineas_Usadas-1]));
     }
}

void Carga_Tema ()
{
  int i;

  if (Lineas_Usadas < TOTAL_LINEAS)
     Datos_ultimo_tema ();
}

void Datos_ultimo_tema ()
{
  int i, p;
  char PG_command  [2000];
  PGresult *res, *res_aux;
  int subtema_disponible;
  int MODO_PREVIO;
  char hilera [500];

  MODO_PREVIO = MODO;
  MODO = REFRESH;

  gtk_combo_box_append_text(CB_tema [Lineas_Usadas], CODIGO_VACIO);
  N_temas = 1;

  sprintf (PG_command,"SELECT DISTINCT codigo_tema, descripcion_materia, orden from bd_materias, bd_ejercicios, bd_texto_preguntas  where codigo_materia = '%s' and codigo_subtema = '          ' and codigo_tema != '          ' and materia = codigo_materia and tema = codigo_tema and texto_ejercicio = codigo_consecutivo_ejercicio order by orden", Codigo_materia);
  res = PQEXEC(DATABASE, PG_command);
  for (i=0; i < PQntuples(res); i++)
      {
       if (!Busca_Tema_Subtema (PQgetvalue (res, i, 0), CODIGO_VACIO))
	  {
	   subtema_disponible = 0;
	   sprintf (PG_command,"SELECT DISTINCT codigo_subtema from bd_materias, bd_ejercicios, bd_texto_preguntas where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' and materia = codigo_materia and tema = codigo_tema and subtema = codigo_subtema and texto_ejercicio = codigo_consecutivo_ejercicio", Codigo_materia, PQgetvalue (res, i, 0));
           res_aux = PQEXEC(DATABASE, PG_command);
           for (p=0; p < PQntuples(res_aux) && !subtema_disponible; p++) subtema_disponible = !Busca_Tema_Subtema (PQgetvalue (res, i, 0), PQgetvalue (res_aux, p, 0));
	   if (subtema_disponible)
	      {
	       sprintf (hilera, "%s %s",PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
               gtk_combo_box_append_text(CB_tema[Lineas_Usadas], hilera);
	       N_temas++;
	      }
	  }
      }

  PQclear(res);

  gtk_combo_box_set_active (CB_tema [Lineas_Usadas], -1);

  MODO = MODO_PREVIO;
}

void Carga_Subtema ()
{
  int i;
  gchar *tema;
  char PG_command  [2000];
  PGresult *res, *res_aux;
  int MODO_PREVIO;
  char Codigo_tema[CODIGO_TEMA_SIZE + 1];
  char hilera [500];

  MODO_PREVIO = MODO;
  MODO = REFRESH;

  tema    = gtk_combo_box_get_active_text(CB_tema [Lineas_Usadas]);
  Extrae_codigo (tema, Codigo_tema, CODIGO_TEMA_SIZE + 1);

  N_subtemas = 0;
  if (!Busca_Tema (Codigo_tema))
     {
      gtk_combo_box_append_text(CB_subtema [Lineas_Usadas], CODIGO_VACIO);
      N_subtemas = 1;
     }

  sprintf (PG_command,"SELECT DISTINCT codigo_subtema, descripcion_materia, orden from bd_materias, bd_ejercicios, bd_texto_preguntas where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' and materia = codigo_materia and tema = codigo_tema and subtema = codigo_subtema and texto_ejercicio = codigo_consecutivo_ejercicio order by orden", Codigo_materia, Codigo_tema);
  res    = PQEXEC(DATABASE, PG_command);

  for (i=0; i < PQntuples(res); i++)
      {
       if (!Busca_Tema_Subtema (Codigo_tema, PQgetvalue (res, i, 0)))
	  {
	   sprintf (hilera,"%s %s",PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
           gtk_combo_box_append_text(CB_subtema[Lineas_Usadas], hilera);
	   N_subtemas++;
	  }
      }

  PQclear(res);

  gtk_combo_box_set_active (CB_subtema [Lineas_Usadas], -1);

  MODO = MODO_PREVIO;
}

void Reduce_Tema (int i)
{
  int j;
  gchar *tema;
  int MODO_PREVIO;

  MODO_PREVIO = MODO;
  MODO = REFRESH;

  tema = gtk_combo_box_get_active_text(CB_tema [i]);

  for (j=0; j < N_temas; j++) gtk_combo_box_remove_text (CB_tema   [i], 0);

  gtk_combo_box_append_text(CB_tema [i],tema);
  N_temas = 1;

  gtk_combo_box_set_active (CB_tema [i], 0);

  MODO = MODO_PREVIO;
}

void Reduce_Subtema (int i)
{
  int j;
  gchar *subtema;
  int MODO_PREVIO;

  MODO_PREVIO = MODO;
  MODO = REFRESH;

  subtema = gtk_combo_box_get_active_text(CB_subtema [i]);

  for (j=0; j < N_subtemas; j++) gtk_combo_box_remove_text (CB_subtema   [i], 0);

  gtk_combo_box_append_text(CB_subtema [i], subtema);
  N_subtemas = 1;

  gtk_combo_box_set_active (CB_subtema [i], 0);

  MODO = MODO_PREVIO;
}

void Actualiza_Total_Preguntas ()
{
  int i, j, N;
  char hilera[50];
  int Borrada = 0;
  struct SELECCION Tabla [TOTAL_LINEAS];
  int Total;

  N = Lineas_Usadas;

  Copia_interfaz_a_tabla (Tabla, Lineas_Usadas);

  /* Revisa si debe borrar alguna lína del esquema */
  for (i=0; (i < Lineas_Usadas) && !Borrada; i++)
      {
       if (Tabla [i].cantidad == 0)
	  {
	   Borrada = 1;
	   for (j=i; j < (Lineas_Usadas-1); j++)
	       {
		Tabla [j] = Tabla[j+1];
	       }

	   N--;
	  }
      }

  if (Borrada)
     {
      Limpia_Lineas ();
      Copia_tabla_a_interfaz (Tabla, N);
      Lineas_Usadas = N;
      Carga_Tema ();
      gtk_widget_set_sensitive(GTK_WIDGET(CB_tema[Lineas_Usadas]), 1);
      gtk_widget_set_visible  (GTK_WIDGET(CB_tema[Lineas_Usadas]), TRUE);
      gtk_widget_grab_focus   (GTK_WIDGET(CB_tema[Lineas_Usadas]));
     }

  Total = 0;
  for (i=0;i<Lineas_Usadas;i++)
      {
       Total += gtk_spin_button_get_value_as_int (SP_N [i]);
      }

  sprintf (hilera,"%d ", Total);
  gtk_entry_set_text(GTK_ENTRY(EN_total), hilera);

  for (i=0;i<Lineas_Usadas;i++)
      {
       sprintf (hilera,"%6.2Lf%% ", (long double) gtk_spin_button_get_value_as_int (SP_N [i]) / Total * 100);
       gtk_entry_set_text(GTK_ENTRY(EN_porcentaje[i]), hilera);
      }
}

int Busca_Tema_Subtema (char * tema, char * subtema)
{
  int i;
  int found = 0;
  struct SELECCION Tabla [TOTAL_LINEAS];
  char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
  char Codigo_subtema [CODIGO_SUBTEMA_SIZE + 1];

  Copia_interfaz_a_tabla (Tabla, Lineas_Usadas);

  for (i=0; i<Lineas_Usadas && !found; i++)
      {
       Extrae_codigo (Tabla[i].tema,    Codigo_tema, CODIGO_TEMA_SIZE + 1);
       if (strcmp (CODIGO_VACIO,Tabla[i].subtema) == 0)
	  strcpy (Codigo_subtema, CODIGO_VACIO);
       else
	  Extrae_codigo (Tabla[i].subtema, Codigo_subtema, CODIGO_SUBTEMA_SIZE + 1);

       found = ((strcmp(tema,Codigo_tema) == 0) && (strcmp(subtema, Codigo_subtema) == 0));
      }

  return (found);
}

int Busca_Tema (char * tema)
{
  int i;
  int found = 0;
  struct SELECCION Tabla [TOTAL_LINEAS];
  char Codigo_tema    [CODIGO_TEMA_SIZE + 1];

  Copia_interfaz_a_tabla (Tabla, Lineas_Usadas);

  for (i=0; i<Lineas_Usadas && !found; i++) 
      {
       Extrae_codigo (Tabla[i].tema, Codigo_tema, CODIGO_TEMA_SIZE + 1);
       found = (strcmp(tema, Codigo_tema) == 0);
      }

  return (found);
}

void Borra_Esquema (GtkWidget *widget, gpointer user_data)
{
   long int k;
   char codigo[10];
   char PG_command [2000];
   PGresult *res, *res_aux;

   k = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
   sprintf (codigo, "%05ld", k);

   /* Aunque el siguiente DELETE también borraría las líneas asociadas por ON DELETE CASCADE, el botón de DELETE sólo se activa si no hay líneas en el esquema */
   sprintf (PG_command,"DELETE from EX_esquemas where codigo_esquema = '%.5s'", codigo);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   Init_Fields ();
   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show (window5);
}

void Graba_Esquema (GtkWidget *widget, gpointer user_data)
{
   long int k;
   int i;
   char codigo[10];
   gchar *descripcion;
   char PG_command [2000];
   time_t curtime;
   struct tm *loctime;
   int month, year, day;
   char hilera[200];
   PGresult *res;
   struct SELECCION Tabla [TOTAL_LINEAS];
   char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
   char Codigo_subtema [CODIGO_SUBTEMA_SIZE + 1];

   /* Get the current time.  */
   curtime = time (NULL);
   /* Convert it to local time representation.  */
   loctime = localtime (&curtime);
   strftime (hilera, 100, "%m", loctime);
   month = atoi(hilera);
   strftime (hilera, 100, "%Y", loctime);
   year  = atoi(hilera);
   strftime (hilera, 100, "%d", loctime);
   day   = atoi(hilera);

   k = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
   sprintf (codigo, "%05ld", k);

   descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion), 0, -1);

   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);
      /* Al borrar el ESQUEMA se borran las líneas automáticamente por ON DELETE CASCADE */
      sprintf (PG_command,"DELETE from EX_esquemas where codigo_esquema = '%.5s'", codigo);
      res = PQEXEC(DATABASE, PG_command); PQclear(res);
      sprintf (PG_command,"INSERT into EX_esquemas values ('%.5s','%s',%d,%d,%d,'%.200s')", codigo, Codigo_materia,year,month,day,descripcion);
      res = PQEXEC(DATABASE, PG_command); PQclear(res);

      Copia_interfaz_a_tabla (Tabla, Lineas_Usadas);

      for (i=0;i<Lineas_Usadas;i++)
          {
	   Extrae_codigo (Tabla[i].tema,    Codigo_tema, CODIGO_TEMA_SIZE + 1);
           if (strcmp (Tabla[i].subtema,CODIGO_VACIO) != 0)
	      Extrae_codigo (Tabla[i].subtema, Codigo_subtema, CODIGO_SUBTEMA_SIZE + 1);
	   else
	      strcpy (Codigo_subtema, CODIGO_VACIO);

           sprintf (PG_command,"INSERT into EX_esquemas_lineas values ('%.5s','%s','%s',%d)", codigo, Codigo_tema, Codigo_subtema, (int) Tabla [i].cantidad);

           res = PQEXEC(DATABASE, PG_command); 
           PQclear(res);
          }
   res = PQEXEC(DATABASE, "END"); PQclear(res);

   g_free (descripcion);

   carga_parametros_EXAMINER (&parametros, DATABASE);
   if (parametros.report_update)
      {
       gtk_widget_set_sensitive(window1, 0);
       gtk_widget_show (window4);

       if (parametros.timeout)
          {
           while (gtk_events_pending ()) gtk_main_iteration ();
           catNap(parametros.timeout);
           gtk_widget_hide (window4);
           gtk_widget_set_sensitive(window1, 1);
	  }
      }
}

void Graba_Esquema_Como (GtkWidget *widget, gpointer user_data)
{
   int i, Last;
   long int k;
   char codigo[10];
   char codigo_nuevo [6];
   char PG_command [2000];
   time_t curtime;
   struct tm *loctime;
   int month, year, day;
   char hilera[200];
   PGresult *res;
   struct SELECCION Tabla [TOTAL_LINEAS];
   char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
   char Codigo_subtema [CODIGO_SUBTEMA_SIZE + 1];

   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);
      res = PQEXEC(DATABASE, "DECLARE ultimo_esquema CURSOR FOR select codigo_esquema from EX_esquemas order by codigo_esquema DESC"); PQclear(res);
      res = PQEXEC(DATABASE, "FETCH 1 in ultimo_esquema");
      Last = atoi (PQgetvalue (res, 0, 0));
      sprintf (codigo_nuevo,"%05d",Last+1);
      PQclear(res);
      res = PQEXEC(DATABASE, "CLOSE ultimo_esquema");  PQclear(res);
   res = PQEXEC(DATABASE, "END"); PQclear(res);

   /* Get the current time.  */
   curtime = time (NULL);
   /* Convert it to local time representation.  */
   loctime = localtime (&curtime);
   strftime (hilera, 100, "%m", loctime);
   month = atoi(hilera);
   strftime (hilera, 100, "%Y", loctime);
   year  = atoi(hilera);
   strftime (hilera, 100, "%d", loctime);
   day   = atoi(hilera);

   k = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
   sprintf (codigo, "%05ld", k);

   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);
      sprintf (PG_command,"INSERT into EX_esquemas values ('%.5s','%s',%d,%d,%d,'Copia del Esquema %.5s')", codigo_nuevo, Codigo_materia,year,month,day,codigo);
      res = PQEXEC(DATABASE, PG_command); PQclear(res);

      Copia_interfaz_a_tabla (Tabla, Lineas_Usadas);

      for (i=0;i<Lineas_Usadas;i++)
          {
	   Extrae_codigo (Tabla[i].tema,    Codigo_tema, CODIGO_TEMA_SIZE + 1);
           if (strcmp (Tabla[i].subtema,CODIGO_VACIO) != 0)
	      Extrae_codigo (Tabla[i].subtema, Codigo_subtema, CODIGO_SUBTEMA_SIZE + 1);
	   else
	      strcpy (Codigo_subtema, CODIGO_VACIO);

           sprintf (PG_command,"INSERT into EX_esquemas_lineas values ('%.5s','%s','%s',%d)", codigo_nuevo, Codigo_tema, Codigo_subtema, (int) Tabla [i].cantidad);

           res = PQEXEC(DATABASE, PG_command); 
           PQclear(res);
          }
   res = PQEXEC(DATABASE, "END"); PQclear(res);

   carga_parametros_EXAMINER (&parametros, DATABASE);
   if (parametros.report_update)
      {
       gtk_widget_set_sensitive(window1, 0);
       gtk_widget_show (window4);

       if (parametros.timeout)
          {
           while (gtk_events_pending ()) gtk_main_iteration ();
           catNap(parametros.timeout);
           gtk_widget_hide (window4);
           gtk_widget_set_sensitive(window1, 1);
	  }
      }
}

void Graba_Descripcion ()
{
  long int k;
  char codigo[10];
  gchar * descripcion;
  char PG_command [2000];
  PGresult *res, *res_aux;

  k = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
  sprintf (codigo, "%05ld", k);

  descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion) , 0, -1);

  sprintf (PG_command,"UPDATE EX_esquemas set descripcion_esquema = '%s' where codigo_esquema = '%.5s'", descripcion, codigo);
  res = PQEXEC(DATABASE, PG_command); PQclear(res);

  g_free (descripcion);

  Init_Fields ();
  gtk_widget_set_sensitive(window1, 0);
  gtk_widget_show (window4);
}

void Ordena_por_tema()
{
  int i,j, N;
  struct SELECCION temporal;
  struct SELECCION Tabla [TOTAL_LINEAS];

  N = Lineas_Usadas;

  Copia_interfaz_a_tabla (Tabla, Lineas_Usadas);

  /* Ordena por tema y subtema */
  for (i= N; i >= 0; i--)
      for (j=0; j < (i-1); j++)
	  {
	   if ((strcmp(Tabla[j].tema   ,Tabla[j+1].tema) > 0) ||
	      ((strcmp(Tabla[j].tema   ,Tabla[j+1].tema) == 0) && 
               (strcmp(Tabla[j].subtema,Tabla[j+1].subtema) > 0)))
	      {
	       temporal       = Tabla [j+1];
               Tabla    [j+1] = Tabla [j];
               Tabla    [j]   = temporal;
	      }
	  }

  Limpia_Lineas ();

  Copia_tabla_a_interfaz (Tabla, N);

  Lineas_Usadas = N;

  Actualiza_Total_Preguntas ();
  Carga_Tema ();

  if (Lineas_Usadas < TOTAL_LINEAS)
     {
      gtk_widget_set_sensitive(GTK_WIDGET(CB_tema[Lineas_Usadas]), 1);
      gtk_widget_set_visible  (GTK_WIDGET(CB_tema[Lineas_Usadas]), TRUE);
      gtk_widget_grab_focus   (GTK_WIDGET(CB_tema[Lineas_Usadas]));
     }
  else
     gtk_widget_grab_focus   (EN_descripcion);
}

void Ordena_por_presentacion ()
{
  int i,j, N;
  struct SELECCION temporal;
  struct SELECCION Tabla [TOTAL_LINEAS];
  char PG_command [1000];
  PGresult *res;
  char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
  char Codigo_subtema [CODIGO_SUBTEMA_SIZE + 1];

  N = Lineas_Usadas;

  Copia_interfaz_a_tabla (Tabla, Lineas_Usadas);

  /* Le agrega a la tabla información del orden establecido para cada tema */
  for (i=0; i < N; i++)
      {
       Extrae_codigo (Tabla[i].tema,    Codigo_tema,    CODIGO_TEMA_SIZE    + 1);
       Extrae_codigo (Tabla[i].subtema, Codigo_subtema, CODIGO_SUBTEMA_SIZE + 1);

       sprintf (PG_command,"SELECT orden from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", Codigo_materia, Codigo_tema);
       res = PQEXEC(DATABASE, PG_command);
       Tabla [i].orden_Tema = atoi (PQgetvalue (res, 0, 0));
       PQclear(res);

       if (strcmp (Tabla[i].subtema,CODIGO_VACIO) != 0)
	  {
	   sprintf (PG_command,"SELECT orden from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", Codigo_materia, Codigo_tema, Codigo_subtema);
           res = PQEXEC(DATABASE, PG_command);
           Tabla [i].orden_Subtema = atoi (PQgetvalue (res, 0, 0));
           PQclear(res);
	  }
       else
	  {
	   Tabla [i].orden_Subtema = 0;
	  }
      }

  /* Ordena por orden de presentación */
  for (i= N; i >= 0; i--)
      for (j=0; j < (i-1); j++)
	  {
	   if  (Tabla[j].orden_Tema  > Tabla[j+1].orden_Tema ||
              ((Tabla[j].orden_Tema == Tabla[j+1].orden_Tema) && (Tabla[j].orden_Subtema > Tabla[j+1].orden_Subtema)))
	      {
	       temporal       = Tabla [j+1];
               Tabla    [j+1] = Tabla [j];
               Tabla    [j]   = temporal;
	      }
	  }

  Limpia_Lineas ();

  Copia_tabla_a_interfaz (Tabla, N);

  Lineas_Usadas = N;

  Actualiza_Total_Preguntas ();
  Carga_Tema ();

  if (Lineas_Usadas < TOTAL_LINEAS)
     {
      gtk_widget_set_sensitive(GTK_WIDGET(CB_tema[Lineas_Usadas]), 1);
      gtk_widget_set_visible  (GTK_WIDGET(CB_tema[Lineas_Usadas]), TRUE);
      gtk_widget_grab_focus   (GTK_WIDGET(CB_tema[Lineas_Usadas]));
     }
  else
     gtk_widget_grab_focus   (EN_descripcion);
}

void Ordena_por_cantidad ()
{
  static int sentido = 0;
  int i,j, N;
  struct SELECCION temporal;
  struct SELECCION Tabla [TOTAL_LINEAS];

  N = Lineas_Usadas;

  Copia_interfaz_a_tabla (Tabla, Lineas_Usadas);

  /* Ordena por cantidad de preguntas */
  if (sentido) /* Mayor a menor */
     {
      for (i=N; i >= 0; i--)
          for (j=0; j < (i-1); j++)
	      {
	       if ((Tabla[j].cantidad < Tabla[j+1].cantidad) ||
		   ((Tabla[j].cantidad == Tabla[j+1].cantidad) &&
		    (Tabla[j].maximo < Tabla[j+1].maximo)))
	          {
	           temporal       = Tabla [j+1];
                   Tabla    [j+1] = Tabla [j];
                   Tabla    [j]   = temporal;
	          }
	      }
     }
  else
     { /* Menor a mayor */
      for (i=N; i >= 0; i--)
          for (j=0; j < (i-1); j++)
	      {
	       if ((Tabla[j].cantidad > Tabla[j+1].cantidad) ||
		   ((Tabla[j].cantidad == Tabla[j+1].cantidad) &&
		    (Tabla[j].maximo > Tabla[j+1].maximo)))
	          {
	           temporal       = Tabla[j+1];
                   Tabla    [j+1] = Tabla[j];
                   Tabla    [j]   = temporal;
	          }
	      }
     }

  sentido = 1 - sentido;

  Limpia_Lineas ();

  Copia_tabla_a_interfaz (Tabla, N);

  Lineas_Usadas = N;

  Actualiza_Total_Preguntas ();

  Carga_Tema ();
  if (Lineas_Usadas < TOTAL_LINEAS)
     {
      gtk_widget_set_sensitive(GTK_WIDGET(CB_tema[Lineas_Usadas]), 1);
      gtk_widget_set_visible  (GTK_WIDGET(CB_tema[Lineas_Usadas]), TRUE);
      gtk_widget_grab_focus   (GTK_WIDGET(CB_tema[Lineas_Usadas]));
     }
  else
     gtk_widget_grab_focus   (EN_descripcion);
}

void Copia_interfaz_a_tabla (struct SELECCION * Tabla, int N)
{
  int i;
  gchar * tema, * subtema;
  gdouble minimum, maximum;

  for (i=0; i < N; i++)
      {
       tema    = gtk_combo_box_get_active_text(CB_tema    [i]);
       subtema = gtk_combo_box_get_active_text(CB_subtema [i]);

       strcpy (Tabla [i].tema   , tema);
       strcpy (Tabla [i].subtema, subtema);

       Tabla         [i].cantidad = (long double) gtk_spin_button_get_value_as_int (SP_N [i]);
       gtk_spin_button_get_range (SP_N[i], &minimum, &maximum);
       Tabla         [i].maximo   = maximum;

       g_free (tema);
       g_free (subtema);
      }
}

void Copia_tabla_a_interfaz (struct SELECCION * Tabla, int N)
{
  int i;
  int MODO_PREVIO;
  char hilera [100];

  MODO_PREVIO = MODO;
  MODO        = REFRESH;

  for (i=0; i < N; i++)
      {
       sprintf (hilera,"%s",Tabla [i].tema);
       gtk_combo_box_append_text(CB_tema [i], hilera);
       gtk_widget_set_visible   (GTK_WIDGET(CB_tema [i]), TRUE);
       gtk_widget_set_sensitive (GTK_WIDGET(CB_tema [i]), 0);
       gtk_combo_box_set_active (CB_tema [i], 0);

       sprintf (hilera,"%s",Tabla [i].subtema);
       gtk_combo_box_append_text(CB_subtema [i], hilera);
       gtk_combo_box_set_active (CB_subtema [i], 0);
       if (strcmp (Tabla[i].subtema,CODIGO_VACIO) != 0)
	  {
	   gtk_widget_set_visible   (GTK_WIDGET(CB_subtema [i]), TRUE);
	   gtk_widget_set_sensitive (GTK_WIDGET(CB_subtema [i]), 0);
	  }

       gtk_spin_button_set_range    (SP_N [i], (long double) 0.0, (long double)Tabla [i].maximo);
       gtk_spin_button_set_value    (SP_N [i], (long double)               Tabla [i].cantidad);

       gtk_widget_set_visible (GTK_WIDGET(SP_N          [i]), TRUE);
       gtk_widget_set_visible (GTK_WIDGET(EN_porcentaje [i]), TRUE);
      }

  MODO        = MODO_PREVIO;
}

void Extrae_codigo (char * hilera, char * codigo, int N)
{
  int i;

  i = 0;

  while (hilera[i] != ' ') codigo [i] = hilera[i++];

  while (i < (N-1)) codigo[i++] = ' ';

  codigo[i] = '\0';
}

void Extrae_descripcion (char * hilera, char * descripcion)
{
  int i;

  i = 0;

  while (hilera[i++] != ' ');
  while (hilera[i++] == ' ');

  strcpy(descripcion, hilera+i-1);
}

void Imprime_reporte ()
{
   FILE * Archivo_Latex;
   gchar *descripcion_esquema;
   char   descripcion_materia[400];
   long int k;
   char codigo[10];
   char hilera_antes [2000], hilera_despues [2000];
   gchar * materia;

   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show         (window3);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_genera_latex), 0.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   materia             = gtk_combo_box_get_active_text(CB_materia);
   Extrae_codigo (materia, Codigo_materia, CODIGO_MATERIA_SIZE + 1);
   Extrae_descripcion (materia, descripcion_materia);
   descripcion_esquema = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion), 0, -1);
   k = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
   sprintf (codigo, "%05ld", k);

   Archivo_Latex = fopen ("EX3000.tex","w");

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_genera_latex), 0.05);
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\documentclass[9pt, twocolumn, journal,oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");

   sprintf (hilera_antes,"\\title{%s \\\\ Esquema %s\\\\ %s}\n\n", descripcion_materia, codigo, descripcion_esquema);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
   fprintf (Archivo_Latex, "\\maketitle\n\n");

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_genera_latex), 0.1);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Reporte_base(Archivo_Latex);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_genera_latex), 0.2);
   while (gtk_events_pending ()) gtk_main_iteration ();

   if (gtk_toggle_button_get_active(RB_porcentaje))
      Reporte_pastel(Archivo_Latex, REPORTE_PORCENTAJE);
   else
      if (gtk_toggle_button_get_active(RB_cantidades))
         Reporte_pastel(Archivo_Latex, REPORTE_CANTIDADES);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_genera_latex), 0.4);
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_genera_latex), 0.5);
   while (gtk_events_pending ()) gtk_main_iteration ();

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX3000", 1, PB_genera_latex, 0.5, 0.45, NULL, NULL);

   system ("rm EX3000.dat >/dev/null 2>&1");

   g_free (materia);
   g_free (descripcion_esquema);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_genera_latex), 1.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   gtk_widget_hide (window3);
   gtk_widget_set_sensitive(window1, 1);
}

void Reporte_base(FILE * Archivo_Latex)
{
  int i;
  struct SELECCION Tabla [TOTAL_LINEAS];
  char Descripcion_Tema    [400];
  char Descripcion_Subtema [400];
  char hilera_antes   [1000];
  char hilera_despues [1000];
  char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
  char Codigo_tema_old[CODIGO_TEMA_SIZE    + 1]="\0";
  char Codigo_subtema [CODIGO_SUBTEMA_SIZE      + 1];
  char PG_command [2000];
  PGresult *res_tema, *res_subtema;

  Ordena_por_presentacion();
  Copia_interfaz_a_tabla (Tabla, Lineas_Usadas);

  fprintf (Archivo_Latex, "\\begin{enumerate}\n");
  i=0;
  while (i < Lineas_Usadas)
        {
         Extrae_codigo (Tabla[i].tema, Codigo_tema, CODIGO_TEMA_SIZE + 1);
         sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '",
                  Codigo_materia, Codigo_tema);
         res_tema = PQEXEC(DATABASE, PG_command);
         sprintf (hilera_antes,"\\item [$\\diamondsuit$] \\textbf{%s}", PQgetvalue (res_tema, 0, 0));
         hilera_LATEX (hilera_antes, hilera_despues);
         fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
         strcpy (Codigo_tema_old, Codigo_tema);

         if (strcmp (Tabla[i].subtema,CODIGO_VACIO) != 0)
	    {
             fprintf (Archivo_Latex, "\\begin{enumerate}\n");
	     while ((i < Lineas_Usadas) && (strcmp(Codigo_tema_old, Codigo_tema)==0))
	           {
                    Extrae_codigo (Tabla[i].subtema, Codigo_subtema, CODIGO_SUBTEMA_SIZE + 1);
                    sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'",
			     Codigo_materia, Codigo_tema, Codigo_subtema);
                    res_subtema = PQEXEC(DATABASE, PG_command);
                    sprintf (hilera_antes,"\\item [$\\Box$] \\textit{%s}", PQgetvalue (res_subtema, 0, 0));
                    hilera_LATEX (hilera_antes, hilera_despues);
                    fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
  		    i++;
		    if (i < Lineas_Usadas) Extrae_codigo (Tabla[i].tema, Codigo_tema, CODIGO_TEMA_SIZE + 1);
	           }
             fprintf (Archivo_Latex, "\\end{enumerate}\n");
	    }
         else
	    i++;
        }
  fprintf (Archivo_Latex, "\\end{enumerate}\n");
}

void Reporte_pastel(FILE * Archivo_Latex, int tipo_reporte)
{
  int Empates;

  Crea_archivo_datos_pastel(&Empates);

  fprintf(Archivo_Latex, "\\DTLloaddb{datosesquema}{EX3000.dat}\n");
  fprintf(Archivo_Latex, "\\begin{figure}[htpb]\n");
  fprintf(Archivo_Latex, "\\centering\n");
  fprintf(Archivo_Latex, "\\setlength{\\DTLpieoutlinewidth}{1pt}\n");

  colores_pastel (Archivo_Latex);

  fprintf(Archivo_Latex, "\\renewcommand*{\\DTLdisplayinnerlabel}[1]{\\textsf{#1}}\n");
  fprintf(Archivo_Latex, "\\renewcommand*{\\DTLdisplayouterlabel}[1]{\\textsf{#1}}\n");
  fprintf(Archivo_Latex, "\\DTLpiechart{variable=\\quantity,%%\n");
  fprintf(Archivo_Latex, "radius=3.2cm,%%\n");
  fprintf(Archivo_Latex, "outerratio=1.05,%%\n");

  if (Empates < MAX_COLORES)
     if (Empates == 1)
        fprintf(Archivo_Latex, "cutaway={1},%%\n");
     else
        fprintf(Archivo_Latex, "cutaway={1-%d},%%\n",Empates);

  fprintf(Archivo_Latex, "innerlabel={},%%\n");

  if (tipo_reporte == REPORTE_PORCENTAJE)
     fprintf(Archivo_Latex, "outerlabel={\\DTLpiepercent\\%%},rotateouter}{datosesquema}{%%\n");
  else
     fprintf(Archivo_Latex, "outerlabel={\\DTLpievariable}}{datosesquema}{%%\n");

  fprintf(Archivo_Latex, "\\name=Name,\\quantity=Quantity}\n");
  fprintf(Archivo_Latex, "\\begin{tabular}[b]{ll}\n");
  fprintf(Archivo_Latex, "\\DTLforeach{datosesquema}{\\name=Name,\\quantity=Quantity}{\\DTLiffirstrow{}{\\\\}%%\n");
  fprintf(Archivo_Latex, "\\DTLdocurrentpiesegmentcolor\\rule{10pt}{10pt}&\n");
  fprintf(Archivo_Latex, "\\textsf{\\name}\n");
  fprintf(Archivo_Latex, "}\n");
  fprintf(Archivo_Latex, "\\end{tabular}\n");
  fprintf(Archivo_Latex, "\\end{figure}\n");
}

void Crea_archivo_datos_pastel(int *Empates)
{
  int i,j, N;
  struct SELECCION temporal;
  struct SELECCION Tabla [TOTAL_LINEAS];
  FILE *Archivo_Datos;
  char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
  char Codigo_subtema [CODIGO_SUBTEMA_SIZE + 1];
  char PG_command [2000];
  char hilera_antes   [2000];
  char hilera_despues [2000];
  int N_otros;
  PGresult *res_tema, *res_subtema;
  char Descripcion[300];

  Copia_interfaz_a_tabla (Tabla, Lineas_Usadas);

  /* Ordena por cantidad de preguntas */
  /* Mayor a menor */
  for (i=Lineas_Usadas; i >= 0; i--)
      for (j=0; j < (i-1); j++)
          {
           if ((Tabla[j].cantidad < Tabla[j+1].cantidad) ||
   	      ((Tabla[j].cantidad == Tabla[j+1].cantidad) &&
	       (Tabla[j].maximo < Tabla[j+1].maximo)))
	      {
	       temporal       = Tabla [j+1];
               Tabla    [j+1] = Tabla [j];
               Tabla    [j]   = temporal;
	      }
	  }

  Archivo_Datos = fopen ("EX3000.dat","w");
  fprintf(Archivo_Datos,"Name, Quantity\n");

  N = Lineas_Usadas;
  if (Lineas_Usadas >  MAX_COLORES) N = MAX_COLORES - 1;

  for (i=0; i < N; i++)
      {
       Extrae_codigo (Tabla[i].tema, Codigo_tema, CODIGO_TEMA_SIZE + 1);
       if (strcmp (Tabla[i].subtema,CODIGO_VACIO) != 0)
	  { /* Hay subtema */
           Extrae_codigo (Tabla[i].subtema, Codigo_subtema, CODIGO_SUBTEMA_SIZE + 1);
           sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'",
		    Codigo_materia, Codigo_tema, Codigo_subtema);
           res_subtema = PQEXEC(DATABASE, PG_command);
	   strcpy(Descripcion, PQgetvalue (res_subtema, 0, 0));
	   if (strlen(Descripcion) > 57)
	      {
	       Descripcion[54] ='.';
	       Descripcion[55] ='.';
	       Descripcion[56] ='.';
	       Descripcion[57] ='\0';
	      }

           sprintf (hilera_antes,"\"\\textit{%s}\", %d", Descripcion,(int) Tabla[i].cantidad);
           hilera_LATEX (hilera_antes, hilera_despues);
           PQclear(res_subtema);
	  }
       else
	  { /* No hay subtemas */
           sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '",
                  Codigo_materia, Codigo_tema);
           res_tema = PQEXEC(DATABASE, PG_command);
	   strcpy(Descripcion, PQgetvalue (res_tema, 0, 0));
	   if (strlen(Descripcion) > 57)
	      {
	       Descripcion[54] ='.';
	       Descripcion[55] ='.';
	       Descripcion[56] ='.';
	       Descripcion[57] ='\0';
	      }

           sprintf (hilera_antes,"\"\\textbf{%s}\", %d", Descripcion,(int) Tabla[i].cantidad);
           hilera_LATEX (hilera_antes, hilera_despues);
           PQclear(res_tema);
	  }
       fprintf(Archivo_Datos,"%s\n", hilera_despues);
      }

  N_otros = 0;
  for (i=N; i < Lineas_Usadas; i++) N_otros += (int) Tabla[i].cantidad;

  if (N_otros)
     fprintf(Archivo_Datos,"\"\\textbf{Otros} (%d)\", %d\n", Lineas_Usadas - N, N_otros);
  fclose (Archivo_Datos);

  i=1;
  while((i < Lineas_Usadas) && Tabla[0].cantidad == Tabla[i++].cantidad);
  *Empates = i - 1;
}

void colores_pastel (FILE * Archivo_Latex)
{
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{1}{green}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{2}{red}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{3}{yellow}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{4}{blue}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{5}{magenta}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{6}{cyan}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{7}{orange}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{8}{violet}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{9}{gray}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{10}{lime}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{11}{teal}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{12}{pink}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{13}{brown}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{14}{purple}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{15}{darkgray}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{16}{olive}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{17}{black}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{18}{lightgray}\n");
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
void on_WN_ex3000_destroy (GtkWidget *widget, gpointer user_data) 
{
  Fin_Ventana (widget, user_data);
}

void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data)
{
  Fin_de_Programa (widget, user_data);
}

void on_BN_undo_clicked(GtkWidget *widget, gpointer user_data)
{
  Init_Fields ();
}

void on_BN_save_clicked(GtkWidget *widget, gpointer user_data)
{
  if (ESQUEMA_USADO)
     Graba_Descripcion ();
  else
     Graba_Esquema (widget, user_data);
}

void on_BN_saveas_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Esquema_Como (widget, user_data);
}

void on_BN_delete_clicked(GtkWidget *widget, gpointer user_data)
{
  Borra_Esquema (widget, user_data);
}

void on_BN_tema_clicked(GtkWidget *widget, gpointer user_data)
{
   Ordena_por_tema ();
}

void on_BN_orden_clicked(GtkWidget *widget, gpointer user_data)
{
   Ordena_por_presentacion ();
}

void on_BN_porcentaje_clicked(GtkWidget *widget, gpointer user_data)
{
   Ordena_por_cantidad ();
}

void on_BN_print_clicked(GtkWidget *widget, gpointer user_data)
{
   Imprime_reporte ();
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (EN_descripcion);
}

void on_BN_ok2_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window4);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (GTK_WIDGET(SP_codigo));
}

void on_BN_ok3_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window5);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (GTK_WIDGET(SP_codigo));
}

void on_SP_codigo_activate(GtkWidget *widget, gpointer user_data)
{
  long int k;
  gchar * codigo;

  codigo = gtk_editable_get_chars(GTK_EDITABLE(SP_codigo), 0, -1);
  k = atoi(codigo);
  g_free(codigo);
  gtk_spin_button_set_value (SP_codigo, k);

  Cambio_Codigo ();
}

void on_EN_descripcion_activate(GtkWidget *widget, gpointer user_data)
{
  Cambia_Descripcion (widget, user_data);
}

void on_CB_materia_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * materia;

  materia = gtk_combo_box_get_active_text(CB_materia);

  if (materia)
     {
      g_free (materia);
      if (MODO == INPUT) Cambio_Materia (widget, user_data);
     }
  else
     g_free (materia);
}

void on_CB_tema_changed(GtkWidget *widget, gpointer user_data)
{
  int k;

  k = atoi (gtk_buildable_get_name (GTK_BUILDABLE (widget))+ 7) - 1;

  if (MODO == INPUT) Cambio_Tema (widget, user_data, k);
}

void on_CB_subtema_changed(GtkWidget *widget, gpointer user_data)
{
  int k;

  k = atoi (gtk_buildable_get_name (GTK_BUILDABLE (widget))+ 10) - 1;

  if (MODO == INPUT) Cambio_Subtema (widget, user_data, k);
}

void on_SP_N_value_changed(GtkWidget *widget, gpointer user_data)
{
  if (MODO == INPUT) Actualiza_Total_Preguntas ();
}
