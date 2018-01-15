/*
Compilar con

cc -o EX3010 EX3010.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic

*/
/*******************************************/
/*  EX3010:                                */
/*                                         */
/*    Actualiza los parámetros para la     */
/*    asignación de boletos a cada pre-    */
/*    gunta.                               */
/*                                         */
/*    The Examiner 0.2                     */
/*    20 de Setiembre 2012                 */
/*    Autor: Francisco J. Torres-Rojas     */        
/*******************************************/
#include <cairo.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <locale.h>
#include <math.h>
#include "examiner.h"

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define MINIMO(a,b) ((a)<(b))?(a):(b)
#define MAXIMO(a,b) ((a)>(b))?(a):(b)
#define DIAS_MAXIMO       720 /* 2 años o 4 semestres */
#define DA_ANCHO    345
#define DA_ALTO     120
#define DA_INICIO_X 10
#define DA_INICIO_Y 9
#define DA_FINAL_X  9
#define DA_FINAL_Y  12
#define DA_MAXIMO_X (DA_ANCHO - DA_INICIO_X - DA_FINAL_X)
#define DA_MAXIMO_Y (DA_ALTO  - DA_INICIO_Y - DA_FINAL_Y)
#define CORRIGE_X(x) (DA_INICIO_X + x)
#define CORRIGE_Y(y) ((DA_ALTO - DA_INICIO_Y) - y)
#define MIN_BOLETOS 0
#define MAX_BOLETOS 50
#define MIN_P 0.0
#define MAX_P 1.0
#define MIN_Rpb 0.0
#define MAX_Rpb 1.0
#define MIN_NOVEDAD 0
#define MAX_NOVEDAD 730
#define MIN_USOS 0
#define MAX_USOS 200
#define MAX_TOTAL_BOLETOS (6*MAX_BOLETOS)

int         b_dificultad, b_discriminacion, b_novedad, b_usos;
long double m_dificultad, m_discriminacion, m_novedad, m_usos;

#define BASE           0
#define DIFICULTAD     1
#define DISCRIMINACION 2
#define NOVEDAD        3
#define USOS           4
#define EJERCICIO      5

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

GtkWidget *FR_botones                = NULL;

GtkWidget *EB_ecuacion_general       = NULL;
GtkWidget *FR_ecuacion_general       = NULL;
GtkWidget *EN_minimo                 = NULL;
GtkWidget *EN_maximo                 = NULL;

GtkWidget *EB_base                   = NULL;
GtkWidget *FR_base                   = NULL;
GtkWidget *EB_base_A                 = NULL;
GtkWidget *FR_base_A                 = NULL;
GtkWidget *EB_base_B                 = NULL;
GtkWidget *FR_base_B                 = NULL;
GtkWidget *EB_base_C                 = NULL;
GtkWidget *FR_base_C                 = NULL;
GtkWidget *SC_boletos_base           = NULL;

GtkWidget *EB_dificultad             = NULL;
GtkWidget *FR_dificultad             = NULL;
GtkWidget *FR_grafico_dificultad     = NULL;
GtkWidget *EB_dificultad_A           = NULL;
GtkWidget *FR_dificultad_A           = NULL;
GtkWidget *EB_dificultad_B           = NULL;
GtkWidget *FR_dificultad_B           = NULL;
GtkWidget *EB_dificultad_C           = NULL;
GtkWidget *FR_dificultad_C           = NULL;

GtkWidget *EB_discriminacion         = NULL;
GtkWidget *FR_discriminacion         = NULL;
GtkWidget *FR_grafico_discriminacion = NULL;
GtkWidget *EB_discriminacion_A       = NULL;
GtkWidget *FR_discriminacion_A       = NULL;
GtkWidget *EB_discriminacion_B       = NULL;
GtkWidget *FR_discriminacion_B       = NULL;
GtkWidget *EB_discriminacion_C       = NULL;
GtkWidget *FR_discriminacion_C       = NULL;

GtkWidget *EB_novedad                = NULL;
GtkWidget *FR_novedad                = NULL;
GtkWidget *FR_grafico_novedad        = NULL;
GtkWidget *EB_novedad_A              = NULL;
GtkWidget *FR_novedad_A              = NULL;
GtkWidget *EB_novedad_B              = NULL;
GtkWidget *FR_novedad_B              = NULL;
GtkWidget *EB_novedad_C              = NULL;
GtkWidget *FR_novedad_C              = NULL;

GtkWidget *EB_usos                   = NULL;
GtkWidget *FR_usos                   = NULL;
GtkWidget *FR_grafico_usos           = NULL;
GtkWidget *EB_usos_A                 = NULL;
GtkWidget *FR_usos_A                 = NULL;
GtkWidget *EB_usos_B                 = NULL;
GtkWidget *FR_usos_B                 = NULL;
GtkWidget *EB_usos_C                 = NULL;
GtkWidget *FR_usos_C                 = NULL;

GtkWidget *DA_dificultad             = NULL;
GtkWidget *DA_discriminacion         = NULL;
GtkWidget *DA_novedad                = NULL;
GtkWidget *DA_usos                   = NULL;

GtkWidget *EB_ejercicio              = NULL;
GtkWidget *FR_ejercicio              = NULL;
GtkWidget *EB_ejercicio_A            = NULL;
GtkWidget *FR_ejercicio_A            = NULL;
GtkWidget *EB_ejercicio_B            = NULL;
GtkWidget *FR_ejercicio_B            = NULL;
GtkWidget *EB_ejercicio_C            = NULL;
GtkWidget *FR_ejercicio_C            = NULL;
GtkWidget *SC_boletos_ejercicio      = NULL;

GtkLabel *LB_rotulo_dificultad       = NULL;
GtkLabel *LB_ecuacion_dificultad     = NULL;
GtkLabel *LB_rotulo_discriminacion   = NULL;
GtkLabel *LB_ecuacion_discriminacion = NULL;
GtkLabel *LB_ecuacion_novedad        = NULL;
GtkLabel *LB_ecuacion_usos           = NULL;
GtkLabel *LB_ecuacion_general        = NULL;

GtkSpinButton *SP_dificultad_1     = NULL;
GtkSpinButton *SP_dificultad_2     = NULL;
GtkSpinButton *SP_discriminacion_1 = NULL;
GtkSpinButton *SP_discriminacion_2 = NULL;
GtkSpinButton *SP_novedad_1        = NULL;
GtkSpinButton *SP_novedad_2        = NULL;
GtkSpinButton *SP_usos_1           = NULL;
GtkSpinButton *SP_usos_2           = NULL;

GtkWidget *EB_boletos              = NULL;
GtkWidget *FR_boletos              = NULL;
GtkWidget *EB_boleto [MAX_TOTAL_BOLETOS];
GtkWidget *FR_boleto [MAX_TOTAL_BOLETOS];

GtkWidget *BN_grabar              = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_terminar            = NULL;
GtkWidget *BN_ok                  = NULL;

GtkWidget *EB_update              = NULL;
GtkWidget *FR_update              = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Connect_Widgets                      ();
void Despliega_boletos                    (int base, int maximo_dificultad, int maximo_discriminacion, int maximo_novedad, int maximo_usos, int ejercicios, int maximo);
void Ecuacion_General                     ();
void Fin_de_Programa                      (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                          (GtkWidget *widget, gpointer user_data);
void GDK_color_parse                      (char * color_text, GdkColor *color);
void Graba_Asignacion_Boletos             ();
void Init_Fields                          ();
void Interface_Coloring                   ();
void on_BN_grabar_clicked                 (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked               (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked                   (GtkWidget *widget, gpointer user_data);
void on_DA_dificultad_expose              (GtkWidget *widget, gpointer user_data);
void on_DA_discriminacion_expose          (GtkWidget *widget, gpointer user_data);
void on_DA_novedad_expose                 (GtkWidget *widget, gpointer user_data);
void on_DA_usos_expose                    (GtkWidget *widget, gpointer user_data);
void on_SC_boletos_base_value_changed     (GtkWidget *widget, gpointer user_data);
void on_SP_dificultad_1_value_changed     (GtkWidget *widget, gpointer user_data);
void on_SP_dificultad_2_value_changed     (GtkWidget *widget, gpointer user_data);
void on_SP_discriminacion_1_value_changed (GtkWidget *widget, gpointer user_data);
void on_SP_discriminacion_2_value_changed (GtkWidget *widget, gpointer user_data);
void on_SP_novedad_1_value_changed        (GtkWidget *widget, gpointer user_data);
void on_SP_novedad_2_value_changed        (GtkWidget *widget, gpointer user_data);
void on_SP_usos_1_value_changed           (GtkWidget *widget, gpointer user_data);
void on_SP_usos_2_value_changed           (GtkWidget *widget, gpointer user_data);
void on_WN_ex3010_destroy                 (GtkWidget *widget, gpointer user_data); 
void Prepara_m                            (char * m_hilera, long double m, char * variable, int decimales, int tipo);
void Read_Only_Fields                     ();
void Redibuja_dificultad                  ();
void Redibuja_discriminacion              ();
void Redibuja_novedad                     ();
void Redibuja_usos                        ();

/*----------------------------------------------------------------------------*/
/***************/
/* M A I N ( ) */
/***************/
int main(int argc, char *argv[]) 
{
  /* connection to the database */
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
      /* make sure that numbers are formatted with '.' to separate decimals from integers - must be done after gkt_init() */
      setlocale (LC_NUMERIC, "en_US.UTF-8");
      carga_parametros_EXAMINER (&parametros, DATABASE);

      builder = gtk_builder_new ();
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX3010.glade", &error))
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

	  /* Read only fields */
	  Read_Only_Fields();

	  /* The magic of color...  */
	  Interface_Coloring ();

          g_signal_connect( G_OBJECT( DA_dificultad    ), "expose-event", G_CALLBACK( on_DA_dificultad_expose ),     NULL);
          g_signal_connect( G_OBJECT( DA_discriminacion), "expose-event", G_CALLBACK( on_DA_discriminacion_expose ), NULL);
          g_signal_connect( G_OBJECT( DA_novedad       ), "expose-event", G_CALLBACK( on_DA_novedad_expose ),        NULL);
          g_signal_connect( G_OBJECT( DA_usos          ), "expose-event", G_CALLBACK( on_DA_usos_expose ),           NULL);

	  /* Displays main window */
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
  int i;
  char hilera[100];

  window1                    = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex3010"));
  window2                    = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));

  EB_ecuacion_general        = GTK_WIDGET (gtk_builder_get_object (builder, "EB_ecuacion_general"));
  FR_ecuacion_general        = GTK_WIDGET (gtk_builder_get_object (builder, "FR_ecuacion_general"));
  EN_minimo                  = GTK_WIDGET (gtk_builder_get_object (builder, "EN_minimo"));
  EN_maximo                  = GTK_WIDGET (gtk_builder_get_object (builder, "EN_maximo"));

  EB_base                    = GTK_WIDGET (gtk_builder_get_object (builder, "EB_base"));
  FR_base                    = GTK_WIDGET (gtk_builder_get_object (builder, "FR_base"));
  EB_base_A                  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_base_A"));
  FR_base_A                  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_base_A"));
  EB_base_B                  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_base_B"));
  FR_base_B                  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_base_B"));
  EB_base_C                  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_base_C"));
  FR_base_C                  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_base_C"));
  SC_boletos_base            = GTK_WIDGET (gtk_builder_get_object (builder, "SC_boletos_base"));

  EB_ejercicio               = GTK_WIDGET (gtk_builder_get_object (builder, "EB_ejercicio"));
  FR_ejercicio               = GTK_WIDGET (gtk_builder_get_object (builder, "FR_ejercicio"));
  EB_ejercicio_A             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_ejercicio_A"));
  FR_ejercicio_A             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_ejercicio_A"));
  EB_ejercicio_B             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_ejercicio_B"));
  FR_ejercicio_B             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_ejercicio_B"));
  EB_ejercicio_C             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_ejercicio_C"));
  FR_ejercicio_C             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_ejercicio_C"));
  SC_boletos_ejercicio       = GTK_WIDGET (gtk_builder_get_object (builder, "SC_boletos_ejercicio"));

  EB_dificultad              = GTK_WIDGET (gtk_builder_get_object (builder, "EB_dificultad"));
  FR_dificultad              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dificultad"));
  FR_grafico_dificultad      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_grafico_dificultad"));
  EB_dificultad_A            = GTK_WIDGET (gtk_builder_get_object (builder, "EB_dificultad_A"));
  FR_dificultad_A            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dificultad_A"));
  EB_dificultad_B            = GTK_WIDGET (gtk_builder_get_object (builder, "EB_dificultad_B"));
  FR_dificultad_B            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dificultad_B"));
  EB_dificultad_C            = GTK_WIDGET (gtk_builder_get_object (builder, "EB_dificultad_C"));
  FR_dificultad_C            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dificultad_C"));

  EB_discriminacion          = GTK_WIDGET (gtk_builder_get_object (builder, "EB_discriminacion"));
  FR_discriminacion          = GTK_WIDGET (gtk_builder_get_object (builder, "FR_discriminacion"));
  FR_grafico_discriminacion  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_grafico_discriminacion"));
  EB_discriminacion_A            = GTK_WIDGET (gtk_builder_get_object (builder, "EB_discriminacion_A"));
  FR_discriminacion_A            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_discriminacion_A"));
  EB_discriminacion_B            = GTK_WIDGET (gtk_builder_get_object (builder, "EB_discriminacion_B"));
  FR_discriminacion_B            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_discriminacion_B"));
  EB_discriminacion_C            = GTK_WIDGET (gtk_builder_get_object (builder, "EB_discriminacion_C"));
  FR_discriminacion_C            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_discriminacion_C"));

  EB_novedad                 = GTK_WIDGET (gtk_builder_get_object (builder, "EB_novedad"));
  FR_novedad                 = GTK_WIDGET (gtk_builder_get_object (builder, "FR_novedad"));
  FR_grafico_novedad         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_grafico_novedad"));
  EB_novedad_A                  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_novedad_A"));
  FR_novedad_A                  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_novedad_A"));
  EB_novedad_B                  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_novedad_B"));
  FR_novedad_B                  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_novedad_B"));
  EB_novedad_C                  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_novedad_C"));
  FR_novedad_C                  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_novedad_C"));

  EB_usos                    = GTK_WIDGET (gtk_builder_get_object (builder, "EB_usos"));
  FR_usos                    = GTK_WIDGET (gtk_builder_get_object (builder, "FR_usos"));
  FR_grafico_usos            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_grafico_usos"));
  EB_usos_A                  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_usos_A"));
  FR_usos_A                  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_usos_A"));
  EB_usos_B                  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_usos_B"));
  FR_usos_B                  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_usos_B"));
  EB_usos_C                  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_usos_C"));
  FR_usos_C                  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_usos_C"));

  DA_dificultad              = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dificultad"));
  DA_discriminacion          = GTK_WIDGET (gtk_builder_get_object (builder, "DA_discriminacion"));
  DA_novedad                 = GTK_WIDGET (gtk_builder_get_object (builder, "DA_novedad"));
  DA_usos                    = GTK_WIDGET (gtk_builder_get_object (builder, "DA_usos"));

  LB_rotulo_dificultad       = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_rotulo_dificultad"));
  LB_ecuacion_dificultad     = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_ecuacion_dificultad"));
  LB_rotulo_discriminacion   = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_rotulo_discriminacion"));
  LB_ecuacion_discriminacion = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_ecuacion_discriminacion"));
  LB_ecuacion_novedad        = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_ecuacion_novedad"));
  LB_ecuacion_usos           = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_ecuacion_usos"));
  LB_ecuacion_general        = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_ecuacion_general"));

  SP_dificultad_1     = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_dificultad_1"));
  SP_dificultad_2     = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_dificultad_2"));
  SP_discriminacion_1 = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_discriminacion_1"));
  SP_discriminacion_2 = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_discriminacion_2"));
  SP_novedad_1        = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_novedad_1"));
  SP_novedad_2        = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_novedad_2"));
  SP_usos_1           = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_usos_1"));
  SP_usos_2           = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_usos_2"));

  EB_boletos          = GTK_WIDGET (gtk_builder_get_object (builder, "EB_boletos"));
  FR_boletos          = GTK_WIDGET (gtk_builder_get_object (builder, "FR_boletos"));
  for (i=0; i< MAX_TOTAL_BOLETOS; i++)
      {
       sprintf (hilera, "EB_%03d", i+1);
       EB_boleto[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));
       sprintf (hilera, "FR_%03d", i+1);
       FR_boleto[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));
      }

  FR_botones                 = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));

  BN_grabar   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_grabar"));
  BN_undo     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_terminar = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_ok       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
  EB_update   = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
  FR_update   = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));
}

void Interface_Coloring ()
{
  GdkColor color;
  int i;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_boletos,       GTK_STATE_NORMAL, &color);

  gdk_color_parse (SECONDARY_AREA, &color);
  gtk_widget_modify_bg(EB_ecuacion_general, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_base,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_dificultad,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_discriminacion, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_novedad,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_usos,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_ejercicio,      GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_ecuacion_general, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_base,             GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_dificultad,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_discriminacion,   GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_novedad,          GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_usos,             GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_ejercicio,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_boletos,          GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones,          GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_minimo,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_maximo,           GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_grabar,   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,       GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_grabar,   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,       GTK_STATE_ACTIVE, &color);

  gdk_color_parse (BOLETOS_FR, &color);
  gtk_widget_modify_bg(FR_base_A, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_base_B, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_base_C, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_dificultad_A, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_dificultad_B, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_dificultad_C, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_discriminacion_A, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_discriminacion_B, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_discriminacion_C, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_novedad_A, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_novedad_B, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_novedad_C, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_usos_A, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_usos_B, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_usos_C, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_ejercicio_A, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_ejercicio_B, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_ejercicio_C, GTK_STATE_NORMAL, &color);
  for (i=0; i<MAX_TOTAL_BOLETOS;i++) 
      gtk_widget_modify_bg(FR_boleto [i], GTK_STATE_NORMAL, &color); 

  gdk_color_parse (COLOR_BOLETOS_BASE, &color);
  gtk_widget_modify_bg(EB_base_A,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_base_B,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_base_C,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(SC_boletos_base, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(SC_boletos_base, GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (COLOR_BOLETOS_DIF, &color);
  gtk_widget_modify_bg(FR_grafico_dificultad,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_dificultad_A,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_dificultad_B,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_dificultad_C,       GTK_STATE_NORMAL, &color);

  gdk_color_parse (COLOR_BOLETOS_DISC, &color);
  gtk_widget_modify_bg(FR_grafico_discriminacion, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_discriminacion_A,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_discriminacion_B,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_discriminacion_C,       GTK_STATE_NORMAL, &color);

  gdk_color_parse (COLOR_BOLETOS_DIAS, &color);
  gtk_widget_modify_bg(FR_grafico_novedad,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_novedad_A,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_novedad_B,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_novedad_C,       GTK_STATE_NORMAL, &color);

  gdk_color_parse (COLOR_BOLETOS_EST, &color);
  gtk_widget_modify_bg(FR_grafico_usos,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_usos_A,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_usos_B,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_usos_C,       GTK_STATE_NORMAL, &color);

  gdk_color_parse (COLOR_BOLETOS_EJER, &color);
  gtk_widget_modify_bg(EB_ejercicio_A,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_ejercicio_B,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_ejercicio_C,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(SC_boletos_ejercicio, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(SC_boletos_ejercicio, GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,              GTK_STATE_NORMAL, &color);

  gdk_color_parse (FINISHED_WORK_FR, &color);
  gtk_widget_modify_bg(FR_update,              GTK_STATE_NORMAL, &color);
}

void Read_Only_Fields()
{
  gtk_widget_set_can_focus(EN_minimo, FALSE);
  gtk_widget_set_can_focus(EN_maximo, FALSE);
}

void Init_Fields()
{
  int i;

  carga_parametros_EXAMINER (&parametros, DATABASE);

  gtk_range_set_value       (GTK_RANGE(SC_boletos_base),      (gdouble) parametros.boletos_base);
  gtk_range_set_value       (GTK_RANGE(SC_boletos_ejercicio), (gdouble) parametros.boletos_ejercicio);
  gtk_spin_button_set_value (SP_dificultad_1,                 (gdouble) parametros.boletos_p_1);
  gtk_spin_button_set_value (SP_dificultad_2,                 (gdouble) parametros.boletos_p_2);
  gtk_spin_button_set_value (SP_discriminacion_1,             (gdouble) parametros.boletos_Rpb_1);
  gtk_spin_button_set_value (SP_discriminacion_2,             (gdouble) parametros.boletos_Rpb_2);
  gtk_spin_button_set_value (SP_novedad_1,                    (gdouble) parametros.boletos_dias_1);
  gtk_spin_button_set_value (SP_novedad_2,                    (gdouble) parametros.boletos_dias_2);
  gtk_spin_button_set_value (SP_usos_1,                       (gdouble) parametros.boletos_usos_1);
  gtk_spin_button_set_value (SP_usos_2,                       (gdouble) parametros.boletos_usos_2);
}

void Graba_Asignacion_Boletos ()
{
   char hilera[5000];
   char PG_command[5000];
   PGresult *res;
   int B, E, P_1, P_2, Rpb_1, Rpb_2, Dias_1, Dias_2, Usos_1, Usos_2;

   B      = (int) gtk_range_get_value (GTK_RANGE(SC_boletos_base));
   E      = (int) gtk_range_get_value (GTK_RANGE(SC_boletos_ejercicio));
   P_1    = gtk_spin_button_get_value_as_int (SP_dificultad_1);
   P_2    = gtk_spin_button_get_value_as_int (SP_dificultad_2);
   Rpb_1  = gtk_spin_button_get_value_as_int (SP_discriminacion_1);
   Rpb_2  = gtk_spin_button_get_value_as_int (SP_discriminacion_2);
   Dias_1 = gtk_spin_button_get_value_as_int (SP_novedad_1);
   Dias_2 = gtk_spin_button_get_value_as_int (SP_novedad_2);
   Usos_1 = gtk_spin_button_get_value_as_int (SP_usos_1);
   Usos_2 = gtk_spin_button_get_value_as_int (SP_usos_2);

   sprintf (hilera, "BOLETOS_BASE=%d BOLETOS_EJERCICIO=%d MIN_BOLETOS=%d MAX_BOLETOS=%d MIN_P=%Lf MAX_P=%Lf BOLETOS_P_1=%d BOLETOS_P_2=%d MIN_RPB=%Lf MAX_RPB=%Lf BOLETOS_RPB_1=%d BOLETOS_RPB_2=%d MIN_DIAS=%d MAX_DIAS=%d BOLETOS_DIAS_1=%d BOLETOS_DIAS_2=%d MIN_USOS=%d MAX_USOS=%d BOLETOS_USOS_1=%d BOLETOS_USOS_2=%d B_DIFICULTAD=%d B_DISCRIMINACION=%d B_NOVEDAD=%d B_USOS=%d M_DIFICULTAD=%Lf M_DISCRIMINACION=%Lf M_NOVEDAD=%Lf M_USOS=%Lf",
            B, E, MIN_BOLETOS, MAX_BOLETOS, 
            (long double) MIN_P,       (long double) MAX_P,       P_1,    P_2, 
            (long double) MIN_Rpb,     (long double) MAX_Rpb,     Rpb_1,  Rpb_2,
            MIN_NOVEDAD, MAX_NOVEDAD, Dias_1, Dias_2,
	    MIN_USOS,    MAX_USOS,    Usos_1, Usos_2,
            b_dificultad, b_discriminacion, b_novedad, b_usos,
            m_dificultad, m_discriminacion, m_novedad, m_usos);

   sprintf (PG_command,"UPDATE PA_parametros SET hilera_parametro = E'%s' where codigo_parametro = 'BOLETOS'", hilera); 
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

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

void Redibuja_dificultad ()
{
   cairo_t  *cr;
   int X0, Y0, X1, Y1;
   int x0, y0, x1, y1;
   long double m;
   int b;
   char ecuacion [200];
   char rotulo[200];
   GdkColor color;
   unsigned int R, G, B;
   float FR, FG, FB;

   gdk_color_parse (COLOR_BOLETOS_DIF, &color);
   FR = (float) color.red  /65535.0;
   FG = (float) color.green/65535.0;
   FB = (float) color.blue /65535.0;
   R = (int)roundf(FR*255);
   G = (int)roundf(FG*255);
   B = (int)roundf(FB*255);

   y0 = gtk_spin_button_get_value_as_int (SP_dificultad_1);
   y1 = gtk_spin_button_get_value_as_int (SP_dificultad_2);

   m = (long double) (y1 - y0)/(MAX_P - MIN_P);
   b = y0 - m * MIN_P;

   if (m == 0.0)
      sprintf (ecuacion, "<i>boletos</i> = <b>%d</b>", b);
   else
      if (b != 0)
	 {
          if (m < 0)
	     if (m == -1.0)
	       sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> -  <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">p</span></b></i></span>", b, R, G, B);
	     else
	       sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> - <b>%.0Lf</b> * <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">p</span></b></i></span>", b, -1 * m, R, G, B);
          else
	     if (m == 1.0)
	       sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> + <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">p</span></b></i></span>", b, R, G, B);
	     else
	       sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> + <b>%.0Lf</b> * <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">p</span></b></i></span>", b, m, R, G, B);
	 }
      else
	 if (m == 1.0)
	   sprintf (ecuacion, "<i>boletos</i> = <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">p</span></b></i></span>", R, G, B);
	 else
	   sprintf (ecuacion, "<i>boletos</i> = <b>%.0Lf</b> * <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">p</span></b></i></span>", m, R, G, B);

   gtk_label_set_markup(LB_ecuacion_dificultad, ecuacion);

   sprintf(rotulo,"Dificultad (<span font_desc=\"Times 12\"><i><b><span foreground=\"#%02X%02X%02X\">p</span></b></i></span>)", R, G, B);

   gtk_label_set_markup(LB_rotulo_dificultad, rotulo);

   b_dificultad = b;
   m_dificultad = m;

   Ecuacion_General ();

   x0 = (MIN_P / (MAX_P - MIN_P)) * DA_MAXIMO_X;
   x1 = (MAX_P / (MAX_P - MIN_P)) * DA_MAXIMO_X;

   y0 = ((long double) y0 / MAX_BOLETOS) * DA_MAXIMO_Y;
   y1 = ((long double) y1 / MAX_BOLETOS) * DA_MAXIMO_Y;

   X0 = CORRIGE_X(x0);
   Y0 = CORRIGE_Y(y0);

   X1 = CORRIGE_X(x1);
   Y1 = CORRIGE_Y(y1);

   cr = gdk_cairo_create(DA_dificultad->window );

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_rectangle(cr, 0, 0, 345, 118);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_line_width (cr, 3);
   cairo_set_source_rgb(cr, FR, FG, FB);
   cairo_move_to(cr, X0, Y0);
   cairo_line_to(cr, X1, Y1);
   cairo_stroke(cr);

   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
   cairo_move_to(cr, X0, Y0);
   cairo_arc( cr, X0, Y0, 2, 0, 2 * G_PI );
   cairo_move_to(cr, X1, Y1);
   cairo_arc( cr, X1, Y1, 2, 0, 2 * G_PI );
   cairo_stroke(cr);

   cairo_destroy( cr );
}

void Redibuja_discriminacion ()
{
   cairo_t  *cr;
   int X0, Y0, X1, Y1;
   int x0, y0, x1, y1;
   long double m;
   int b;
   char ecuacion [200];
   char rotulo[200];
   GdkColor color;
   unsigned int R, G, B;
   float FR, FG, FB;

   gdk_color_parse (COLOR_BOLETOS_DISC, &color);
   FR = (float) color.red  /65535.0;
   FG = (float) color.green/65535.0;
   FB = (float) color.blue /65535.0;
   R = (int)roundf(FR*255);
   G = (int)roundf(FG*255);
   B = (int)roundf(FB*255);

   y0 = gtk_spin_button_get_value_as_int (SP_discriminacion_1);
   y1 = gtk_spin_button_get_value_as_int (SP_discriminacion_2);

   m = (long double) (y1 - y0)/(MAX_Rpb - MIN_Rpb);
   b = y0 - m * MIN_Rpb;

   if (m == 0.0)
      sprintf (ecuacion, "<i>boletos</i> = <b>%d</b>", b);
   else
      if (b != 0)
	 {
          if (m < 0)
	     if (m == -1.0)
	       sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> -  <span font_desc=\"Times 10\"><span foreground=\"#%02X%02X%02X\"><b><i>r</i></b><sub>pb</sub></span></span>", b, R, G, B);
	     else
	       sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> - <b>%.0Lf</b> * <span font_desc=\"Times 10\"><span foreground=\"#%02X%02x%02x\"><b><i>r</i></b><sub>pb</sub></span></span>", b, -1 * m, R, G, B);
          else
	     if (m == 1.0)
	       sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> + <span font_desc=\"Times 10\"><span foreground=\"#%02X%02x%02x\"><b><i>r</i></b><sub>pb</sub></span></span>", b, R, G, B);
	     else
	       sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> + <b>%.0Lf</b> * <span font_desc=\"Times 10\"><span foreground=\"#%02X%02x%02x\"><b><i>r</i></b><sub>pb</sub></span></span>", b, m, R, G, B);
	 }
      else
	 if (m == 1.0)
	   sprintf (ecuacion, "<i>boletos</i> = <span font_desc=\"Times 10\"><span foreground=\"#%02X%02x%02x\"><b><i>r</i></b><sub>pb</sub></span></span>", R, G, B);
	 else
	   sprintf (ecuacion, "<i>boletos</i> = <b>%.0Lf</b> * <span font_desc=\"Times 10\"><span foreground=\"#%02X%02X%02X\"><b><i>r</i></b><sub>pb</sub></span></span>", m, R, G, B);

   gtk_label_set_markup(LB_ecuacion_discriminacion, ecuacion);

   sprintf(rotulo,"Discriminación (<span font_desc=\"Times 12\"><span foreground=\"#%02X%02X%02X\"><b><i>r</i></b><sub>pb</sub></span></span>)", R, G, B);
   gtk_label_set_markup(LB_rotulo_discriminacion, rotulo);

   b_discriminacion = b;
   m_discriminacion = m;

   Ecuacion_General ();

   x0 = (MIN_Rpb / (MAX_Rpb - MIN_Rpb)) * DA_MAXIMO_X;
   x1 = (MAX_Rpb / (MAX_Rpb - MIN_Rpb)) * DA_MAXIMO_X;

   y0 = ((long double) y0 / MAX_BOLETOS) * DA_MAXIMO_Y;
   y1 = ((long double) y1 / MAX_BOLETOS) * DA_MAXIMO_Y;

   X0 = CORRIGE_X(x0);
   Y0 = CORRIGE_Y(y0);

   X1 = CORRIGE_X(x1);
   Y1 = CORRIGE_Y(y1);

   cr = gdk_cairo_create(DA_discriminacion->window );

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_rectangle(cr, 0, 0, 345, 118);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_line_width (cr, 3);
   cairo_set_source_rgb(cr, FR, FG, FB);
   cairo_move_to(cr, X0, Y0);
   cairo_line_to(cr, X1, Y1);
   cairo_stroke(cr);

   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
   cairo_move_to(cr, X0, Y0);
   cairo_arc( cr, X0, Y0, 2, 0, 2 * G_PI );
   cairo_move_to(cr, X1, Y1);
   cairo_arc( cr, X1, Y1, 2, 0, 2 * G_PI );
   cairo_stroke(cr);

   cairo_destroy( cr );
}

void Redibuja_novedad ()
{
   cairo_t  *cr;
   int X0, Y0, X1, Y1;
   int x0, y0, x1, y1;
   long double m;
   int b;
   char ecuacion [200];
   GdkColor color;
   unsigned int R, G, B;
   float FR, FG, FB;

   gdk_color_parse (COLOR_BOLETOS_DIAS, &color);
   FR = (float) color.red  /65535.0;
   FG = (float) color.green/65535.0;
   FB = (float) color.blue /65535.0;
   R = (int)roundf(FR*255);
   G = (int)roundf(FG*255);
   B = (int)roundf(FB*255);

   y0 = gtk_spin_button_get_value_as_int (SP_novedad_1);
   y1 = gtk_spin_button_get_value_as_int (SP_novedad_2);

   m = (long double) (y1 - y0)/(MAX_NOVEDAD - MIN_NOVEDAD);
   b = y0 - m * MIN_NOVEDAD;

   if (m == 0.0)
      sprintf (ecuacion, "<i>boletos</i> = <b>%d</b>", b);
   else
      if (b != 0)
	 {
          if (m < 0)
             sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> - <b>%4.3Lf</b> * <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">días</span></b></i></span>", 
                      b, -1 * m, R, G, B);
          else
             sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> + <b>%4.3Lf</b> * <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">días</span></b></i></span>", 
		      b, m, R, G, B);
	 }
      else
	sprintf (ecuacion, "<i>boletos</i> = <b>%4.3Lf</b> * <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">días</span></b></i></span>", m, R, G, B);

   gtk_label_set_markup(LB_ecuacion_novedad, ecuacion);

   b_novedad = b;
   m_novedad = m;

   Ecuacion_General ();

   x0 = (MIN_NOVEDAD / (MAX_NOVEDAD - MIN_NOVEDAD)) * DA_MAXIMO_X;
   x1 = (MAX_NOVEDAD / (MAX_NOVEDAD - MIN_NOVEDAD)) * DA_MAXIMO_X;

   y0 = ((long double) y0 / MAX_BOLETOS) * DA_MAXIMO_Y;
   y1 = ((long double) y1 / MAX_BOLETOS) * DA_MAXIMO_Y;

   X0 = CORRIGE_X(x0);
   Y0 = CORRIGE_Y(y0);

   X1 = CORRIGE_X(x1);
   Y1 = CORRIGE_Y(y1);

   cr = gdk_cairo_create(DA_novedad->window );

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_rectangle(cr, 0, 0, 345, 118);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_line_width (cr, 3);
   cairo_set_source_rgb(cr, FR, FG, FB);
   cairo_move_to(cr, X0, Y0);
   cairo_line_to(cr, X1, Y1);
   cairo_stroke(cr);

   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
   cairo_move_to(cr, X0, Y0);
   cairo_arc( cr, X0, Y0, 2, 0, 2 * G_PI );
   cairo_move_to(cr, X1, Y1);
   cairo_arc( cr, X1, Y1, 2, 0, 2 * G_PI );
   cairo_stroke(cr);

   cairo_destroy( cr );
}

void Redibuja_usos ()
{
   cairo_t  *cr;
   int X0, Y0, X1, Y1;
   int x0, y0, x1, y1;
   long double m;
   int b;
   char ecuacion [200];
   GdkColor color;
   unsigned int R, G, B;
   float FR, FG, FB;

   gdk_color_parse (COLOR_BOLETOS_EST, &color);
   FR = (float) color.red  /65535.0;
   FG = (float) color.green/65535.0;
   FB = (float) color.blue /65535.0;
   R = (int)roundf(FR*255);
   G = (int)roundf(FG*255);
   B = (int)roundf(FB*255);

   y0 = gtk_spin_button_get_value_as_int (SP_usos_1);
   y1 = gtk_spin_button_get_value_as_int (SP_usos_2);

   m = (long double) (y1 - y0)/(MAX_USOS - MIN_USOS);
   b = y0 - m * MIN_USOS;

   if (m == 0.0)
      sprintf (ecuacion, "<i>boletos</i> = <b>%d</b>", b);
   else
      if (b != 0)
	 {
          if (m < 0)
             sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> - <b>%4.3Lf</b> * <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">estudiantes</span></b></i></span>", 
                      b, -1 * m, R, G, B);
          else
             sprintf (ecuacion, "<i>boletos</i> = <b>%d</b> + <b>%4.3Lf</b> * <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">estudiantes</span></b></i></span>",
                      b, m, R, G, B);
	 }
      else
	sprintf (ecuacion, "<i>boletos</i> = <b>%4.3Lf</b> * <span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">estudiantes</span></b></i></span>", m, R, G, B);

   gtk_label_set_markup(LB_ecuacion_usos, ecuacion);

   b_usos = b;
   m_usos = m;

   Ecuacion_General ();

   x0 = (MIN_USOS / (MAX_USOS - MIN_USOS)) * DA_MAXIMO_X;
   x1 = (MAX_USOS / (MAX_USOS - MIN_USOS)) * DA_MAXIMO_X;

   y0 = ((long double) y0 / MAX_BOLETOS) * DA_MAXIMO_Y;
   y1 = ((long double) y1 / MAX_BOLETOS) * DA_MAXIMO_Y;

   X0 = CORRIGE_X(x0);
   Y0 = CORRIGE_Y(y0);

   X1 = CORRIGE_X(x1);
   Y1 = CORRIGE_Y(y1);

   cr = gdk_cairo_create(DA_usos->window );

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_rectangle(cr, 0, 0, 345, 118);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_line_width (cr, 3);
   cairo_set_source_rgb(cr, FR, FG, FB);
   cairo_move_to(cr, X0, Y0);
   cairo_line_to(cr, X1, Y1);
   cairo_stroke(cr);

   cairo_set_source_rgb(cr, 0, 0, 0);
   cairo_move_to(cr, X0, Y0);
   cairo_arc( cr, X0, Y0, 2, 0, 2 * G_PI );
   cairo_move_to(cr, X1, Y1);
   cairo_arc( cr, X1, Y1, 2, 0, 2 * G_PI );
   cairo_stroke(cr);

   cairo_destroy( cr );
}

void Ecuacion_General ()
{
   char ecuacion [1500];
   char m_dificultad_hilera     [200];
   char m_discriminacion_hilera [200];
   char m_novedad_hilera        [200];
   char m_usos_hilera           [200];
   char m_ejercicios_hilera     [200];
   char hilera[20];
   int B;
   int minimo, maximo;
   int base, ejercicios;
   int minimo_dificultad,     maximo_dificultad;
   int minimo_discriminacion, maximo_discriminacion;
   int minimo_novedad,        maximo_novedad;
   int minimo_usos,           maximo_usos;
   int ejercicio;
   int y0, y1;
   int i;
   GdkColor color;
   unsigned int Red, Green, Blue;
   float FR, FG, FB;

   gdk_color_parse (COLOR_BOLETOS_EJER, &color);
   FR = (float) color.red  /65535.0;
   FG = (float) color.green/65535.0;
   FB = (float) color.blue /65535.0;
   Red   = (int)roundf(FR*255);
   Green = (int)roundf(FG*255);
   Blue  = (int)roundf(FB*255);

   Prepara_m (m_dificultad_hilera,     m_dificultad,     "<span font_desc=\"Times 10\"><i><b><span foreground=\"#%02X%02X%02X\">p</span></b></i></span>", 0, DIFICULTAD);
   Prepara_m (m_discriminacion_hilera, m_discriminacion, "<span font_desc=\"Times 10\"><span foreground=\"#%02X%02X%02X\"><b><i>r</i></b><sub>pb</sub></span></span>", 0, DISCRIMINACION);
   Prepara_m (m_novedad_hilera,        m_novedad,        "<span font_desc=\"Times 10\"><span foreground=\"#%02X%02X%02X\"><b><i>días</i></b></span></span>", 3, NOVEDAD);
   Prepara_m (m_usos_hilera,           m_usos,           "<span font_desc=\"Times 10\"><span foreground=\"#%02X%02X%02X\"><b><i>estudiantes</i></b></span></span>", 3, USOS);

   base = (int) gtk_range_get_value (GTK_RANGE(SC_boletos_base));
   ejercicios = (int) gtk_range_get_value (GTK_RANGE(SC_boletos_ejercicio));
   B = base + b_dificultad + b_discriminacion + b_novedad + b_usos;

   sprintf (ecuacion, "<i>Total de boletos</i> = <b>%d</b>%s%s%s%s + (<span font_desc=\"Times 10\"><span foreground=\"#%02X%02X%02X\"><b><i>ejercicio?</i></b></span></span>) * <b>%d</b>", B, m_dificultad_hilera, m_discriminacion_hilera, m_novedad_hilera, m_usos_hilera, Red, Green, Blue, ejercicios);
   gtk_label_set_markup(LB_ecuacion_general, ecuacion);

   y0 = gtk_spin_button_get_value_as_int (SP_dificultad_1);
   y1 = gtk_spin_button_get_value_as_int (SP_dificultad_2);
   minimo_dificultad = MINIMO (y0, y1);
   maximo_dificultad = MAXIMO (y0, y1);

   y0 = gtk_spin_button_get_value_as_int (SP_discriminacion_1);
   y1 = gtk_spin_button_get_value_as_int (SP_discriminacion_2);
   minimo_discriminacion = MINIMO (y0, y1);
   maximo_discriminacion = MAXIMO (y0, y1);

   y0 = gtk_spin_button_get_value_as_int (SP_novedad_1);
   y1 = gtk_spin_button_get_value_as_int (SP_novedad_2);
   minimo_novedad = MINIMO (y0, y1);
   maximo_novedad = MAXIMO (y0, y1);

   y0 = gtk_spin_button_get_value_as_int (SP_usos_1);
   y1 = gtk_spin_button_get_value_as_int (SP_usos_2);
   minimo_usos = MINIMO (y0, y1);
   maximo_usos = MAXIMO (y0, y1);

   minimo = base + minimo_dificultad + minimo_discriminacion + minimo_novedad + minimo_usos;
   maximo = base + maximo_dificultad + maximo_discriminacion + maximo_novedad + maximo_usos + ejercicios;

   sprintf (hilera, "%d", minimo);
   gtk_entry_set_text(GTK_ENTRY(EN_minimo), hilera);
   sprintf (hilera, "%d", maximo);
   gtk_entry_set_text(GTK_ENTRY(EN_maximo), hilera);

   Despliega_boletos (base, maximo_dificultad, maximo_discriminacion, maximo_novedad, maximo_usos, ejercicios, maximo);
}

void Despliega_boletos(int base, int maximo_dificultad, int maximo_discriminacion, int maximo_novedad, int maximo_usos, int ejercicios, int maximo)
{
  int i, j;
  GdkColor color;

  j = 0;
  gdk_color_parse (COLOR_BOLETOS_BASE, &color);
  for (i=0; i < base; i++)
      gtk_widget_modify_bg(EB_boleto [j + i], GTK_STATE_NORMAL, &color);
  j =base;
  gdk_color_parse (COLOR_BOLETOS_DIF, &color);
  for (i=0; i < maximo_dificultad; i++)
      gtk_widget_modify_bg(EB_boleto [j + i], GTK_STATE_NORMAL, &color);
  j += maximo_dificultad;
  gdk_color_parse (COLOR_BOLETOS_DISC, &color);
  for (i=0; i < maximo_discriminacion; i++)
      gtk_widget_modify_bg(EB_boleto [j + i], GTK_STATE_NORMAL, &color);
  j += maximo_discriminacion;
  gdk_color_parse (COLOR_BOLETOS_DIAS, &color);
  for (i=0; i < maximo_novedad; i++)
      gtk_widget_modify_bg(EB_boleto [j + i], GTK_STATE_NORMAL, &color);
  j += maximo_novedad;
  gdk_color_parse (COLOR_BOLETOS_EST, &color);
  for (i=0; i < maximo_usos; i++)
      gtk_widget_modify_bg(EB_boleto [j + i], GTK_STATE_NORMAL, &color);
  j += maximo_usos;
  gdk_color_parse (COLOR_BOLETOS_EJER, &color);
  for (i=0; i < ejercicios; i++)
      gtk_widget_modify_bg(EB_boleto [j + i], GTK_STATE_NORMAL, &color);

  for (i=0     ;i<maximo           ;i++) gtk_widget_set_visible (EB_boleto [i], TRUE);
  for (i=maximo;i<MAX_TOTAL_BOLETOS;i++) gtk_widget_set_visible (EB_boleto [i], FALSE);
}

void Prepara_m (char * m_hilera, long double m, char * variable, int decimales, int tipo)
{
   GdkColor color;
   unsigned int R, G, B;
   float FR, FG, FB;
   char buffer[1000];

   switch(tipo)
         {
	 case BASE:           gdk_color_parse (COLOR_BOLETOS_BASE, &color); break;
	 case DIFICULTAD:     gdk_color_parse (COLOR_BOLETOS_DIF,  &color); break;
	 case DISCRIMINACION: gdk_color_parse (COLOR_BOLETOS_DISC, &color); break;
	 case NOVEDAD:        gdk_color_parse (COLOR_BOLETOS_DIAS, &color); break;
	 case USOS:           gdk_color_parse (COLOR_BOLETOS_EST,  &color); break;
	 case EJERCICIO:      gdk_color_parse (COLOR_BOLETOS_EJER, &color); break;
         }
   FR = (float) color.red  /65535.0;
   FG = (float) color.green/65535.0;
   FB = (float) color.blue /65535.0;
   R = (int)roundf(FR*255);
   G = (int)roundf(FG*255);
   B = (int)roundf(FB*255);

   sprintf(buffer, variable, R, G, B); 

   if (m == 0.0)
      m_hilera[0] = '\0';
   else
      {
       if (m == 1.0)
          sprintf (m_hilera," + %s",buffer);
       else
          if (m == -1.0)
             sprintf (m_hilera," - %s",buffer);
	  else
	     if (m < 0.0)
 	        sprintf (m_hilera," - <b>%.*Lf</b> * %s",decimales,-1 * m, buffer);
	     else
	        sprintf (m_hilera," + <b>%.*Lf</b> * %s",decimales,     m, buffer);
      }
}

void GDK_color_parse (char * color_text, GdkColor *color)
{
  if (!gdk_color_parse (color_text, color))
     color->red = color->green = color->blue = 0;
}

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex3010_destroy (GtkWidget *widget, gpointer user_data) 
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

void on_BN_grabar_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Asignacion_Boletos ();
}

void on_DA_dificultad_expose(GtkWidget *widget, gpointer user_data)
{
  Redibuja_dificultad ();
}

void on_DA_discriminacion_expose(GtkWidget *widget, gpointer user_data)
{
  Redibuja_discriminacion ();
}

void on_DA_novedad_expose(GtkWidget *widget, gpointer user_data)
{
  Redibuja_novedad ();
}

void on_DA_usos_expose(GtkWidget *widget, gpointer user_data)
{
  Redibuja_usos ();
}

void on_SP_dificultad_1_value_changed(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_queue_draw(DA_dificultad);
}

void on_SP_dificultad_2_value_changed(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_queue_draw(DA_dificultad);
}

void on_SP_discriminacion_1_value_changed(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_queue_draw(DA_discriminacion);
}

void on_SP_discriminacion_2_value_changed(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_queue_draw(DA_discriminacion);
}

void on_SP_novedad_1_value_changed(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_queue_draw(DA_novedad);
}

void on_SP_novedad_2_value_changed(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_queue_draw(DA_novedad);
}

void on_SP_usos_1_value_changed(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_queue_draw(DA_usos);
}

void on_SP_usos_2_value_changed(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_queue_draw(DA_usos);
}

void on_SC_boletos_base_value_changed(GtkWidget *widget, gpointer user_data)
{
  Ecuacion_General ();
}

void on_SC_boletos_ejercicio_value_changed(GtkWidget *widget, gpointer user_data)
{
  Ecuacion_General ();
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (SC_boletos_base);
}






