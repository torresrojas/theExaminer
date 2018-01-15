/*
Compilar con

cc -o EX3030 EX3030.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic

*/
/*******************************************/
/*  EX3030:                                */
/*                                         */
/*    Refina un Pre-Examen                 */
/*    The Examiner 0.0                     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    30 de Julio 2011                     */
/*                                         */
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Se cambia distribución de campos     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    17 de Febrero del 2012               */
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
GtkWidget *window3                = NULL;
GtkWidget *window4                = NULL;
GtkWidget *FR_preexamenusado      = NULL;

GtkWidget *EB_refinamiento        = NULL;
GtkWidget *FR_refinamiento        = NULL;
GtkWidget *FR_dif  = NULL;
GtkWidget *FR_disc = NULL;
GtkWidget *FR_Cron = NULL;
GtkWidget *FR_dias = NULL;
GtkWidget *DA_dif  = NULL;
GtkWidget *DA_disc = NULL;
GtkWidget *DA_Cron = NULL;
GtkWidget *DA_dias = NULL;

GtkWidget *EB_pre_examen          = NULL;
GtkWidget *FR_pre_examen          = NULL;

GtkWidget *EB_actual              = NULL;
GtkWidget *FR_actual              = NULL;
GtkWidget *EB_nuevo               = NULL;
GtkWidget *FR_nuevo               = NULL;
GtkWidget *FR_botones             = NULL;
GtkWidget *FR_botones_actual      = NULL;
GtkWidget *FR_botones_nuevo       = NULL;
GtkWidget *FR_pregunta_actual     = NULL;
GtkWidget *FR_pregunta_nueva      = NULL;

GtkSpinButton *SP_codigo          = NULL;
GtkWidget *EN_descripcion         = NULL;
GtkWidget *EN_materia             = NULL;
GtkWidget *EN_materia_descripcion = NULL;
GtkWidget *EN_esquema             = NULL;
GtkWidget *EN_esquema_descripcion = NULL;
GtkWidget *EN_preguntas           = NULL;

GtkWidget *SC_preguntas           = NULL;

GtkWidget *EN_ejercicio_viejo     = NULL;
GtkWidget *EN_secuencia_viejo     = NULL;
GtkWidget *EN_tema_viejo          = NULL;
GtkWidget *EN_tema_descripcion_viejo = NULL;
GtkWidget *EN_subtema_viejo       = NULL;
GtkWidget *EN_subtema_descripcion_viejo = NULL;
GtkWidget *EN_estudiantes_viejo   = NULL;
GtkWidget *EN_media_viejo         = NULL;
GtkWidget *EN_desviacion_vieja    = NULL;
GtkWidget *EN_alfa_viejo          = NULL;
GtkWidget *EN_Rpb_viejo           = NULL;
GtkTextView *TV_pregunta          = NULL;
GtkTextBuffer *buffer_TV_pregunta;
GtkWidget *BN_delete              = NULL;
GtkWidget *BN_print_actual        = NULL;

GtkWidget *SC_preguntas_nuevas    = NULL;
GtkWidget *EN_preguntas_nuevas    = NULL;
GtkWidget *EN_ejercicio           = NULL;
GtkWidget *EN_secuencia           = NULL;

GtkWidget *EN_tema_nuevo          = NULL;
GtkWidget *EN_tema_descripcion_nuevo = NULL;
GtkWidget *EN_subtema_nuevo       = NULL;
GtkWidget *EN_subtema_descripcion_nuevo = NULL;
GtkWidget *EN_estudiantes_nuevo   = NULL;
GtkWidget *EN_media_nuevo         = NULL;
GtkWidget *EN_desviacion_nueva    = NULL;
GtkWidget *EN_alfa_nuevo          = NULL;
GtkWidget *EN_Rpb_nuevo           = NULL;
GtkTextView *TV_pregunta_nueva    = NULL;
GtkTextBuffer * buffer_TV_pregunta_nueva;
GtkWidget *BN_add                 = NULL;
GtkWidget *BN_print_nuevo         = NULL;
GtkComboBox *CB_tema              = NULL;
GtkComboBox *CB_subtema           = NULL;

GtkWidget *EN_estudiantes         = NULL;
GtkWidget *EN_media               = NULL;
GtkWidget *EN_desviacion          = NULL;
GtkWidget *EN_alfa                = NULL;
GtkWidget *EN_Rpb                 = NULL;

GtkWidget *BN_save                = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_terminar            = NULL;

GtkWidget *EB_reporte             = NULL;
GtkWidget *FR_reporte             = NULL;
GtkWidget *PB_reporte             = NULL;

GtkWidget *FR_update              = NULL;
GtkWidget *EB_update              = NULL;
GtkWidget *BN_ok                  = NULL;

/***********************/
/* Prototypes          */
/***********************/
void        Agrega_Pregunta                      ();
void        Borra_Pregunta                       ();
int         Busca_pregunta                       (char * ejercicio, int secuencia);
void        Cambia_Pregunta                      ();
void        Cambio_Codigo                        ();
void        Cambio_Preguntas_Nuevas              ();
void        Cambio_Subtema                       (GtkWidget *widget, gpointer user_data);
void        Cambio_Tema                          (GtkWidget *widget, gpointer user_data);
void        Carga_Pre_Examen                     (PGresult * res, gchar * codigo);
void        Carga_SC_preguntas_nuevas            ();
long double CDF                                  (long double X, long double Media, long double Desv);
void        Connect_widgets();
void        Despliega_Pregunta                   (int k);
void        Establece_SC_preguntas               (int k);
void        Extrae_codigo                        (char * hilera, char * codigo);
void        Fin_de_Programa                      (GtkWidget *widget, gpointer user_data);
void        Fin_Ventana                          (GtkWidget *widget, gpointer user_data);
void        Graba_Pre_Examen                     ();
void        Grafico_y_Tabla                      (FILE * Archivo_Latex, long double media, long double desviacion, int N_preguntas);
void        Interface_Coloring ();
void        Imprime_Pregunta                     (char * ejercicio, int secuencia);
void        Imprime_Pregunta_actual              ();
void        Imprime_Pregunta_nueva               ();
void        Imprime_Pre_Examen                   ();
void        Imprime_Pregunta_Pre_Examen          (char * ejercicio, int secuencia, FILE * Archivo_Latex,char * prefijo);
void        Init_Fields                          ();
void        Limpia_Datos_Nuevo                   ();
int         main                                 (int argc, char *argv[]);
void        on_BN_add_clicked                    (GtkWidget *widget, gpointer user_data);
void        on_CB_subtema_changed                (GtkWidget *widget, gpointer user_data);
void        on_CB_tema_changed                   (GtkWidget *widget, gpointer user_data);
void        on_BN_delete_clicked                 (GtkWidget *widget, gpointer user_data);
void        on_BN_OK_clicked                     (GtkWidget *widget, gpointer user_data);
void        on_BN_print_clicked                  (GtkWidget *widget, gpointer user_data);
void        on_BN_print_actual_clicked           (GtkWidget *widget, gpointer user_data);
void        on_BN_print_nuevo_clicked            (GtkWidget *widget, gpointer user_data);
void        on_BN_save_clicked                   (GtkWidget *widget, gpointer user_data);
void        on_BN_terminar_clicked               (GtkWidget *widget, gpointer user_data);
void        on_BN_undo_clicked                   (GtkWidget *widget, gpointer user_data);
void        on_SC_preguntas_value_changed        (GtkWidget *widget, gpointer user_data);
void        on_SC_preguntas_nuevas_value_changed (GtkWidget *widget, gpointer user_data);
void        on_SP_codigo_activate                (GtkWidget *widget, gpointer user_data);
void        on_WN_ex3030_destroy                 (GtkWidget *widget, gpointer user_data);
void        Prepara_Grafico_Normal               (FILE * Archivo_Latex, long double media, long double desviacion, int N);
void        Read_Only_Fields ();
void        Tabla_Probabilidades                 (FILE * Archivo_Latex, long double media, long double desviacion, int N);
void        Update_Dials();
void Update_dificultad          ();
void Update_novedad             ();
void Update_alfa                ();
void Update_rpb                 ();

void on_DA_dif_expose         (GtkWidget *widget, gpointer user_data);
void on_DA_disc_expose        (GtkWidget *widget, gpointer user_data);
void on_DA_Cron_expose        (GtkWidget *widget, gpointer user_data);
void on_DA_dias_expose        (GtkWidget *widget, gpointer user_data);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define MINIMO(a,b) ((a)<(b))?(a):(b)

#define CODIGO_SIZE              7
#define EJERCICIO_SIZE           7
#define PREGUNTA_SIZE            7
#define TEXTO_PREGUNTA_SIZE      4097
#define PI 3.141592654
#define DA_HEIGHT 95
#define DA_WIDTH  181
#define DA_MIDDLE ((DA_WIDTH / 2) - 1)
#define DA_BEGIN   5
#define CERO (DA_BEGIN + 84)
#define ARROW_LENGTH 68
#define ARROW_SIZE   16
#define ARROW_WIDTH  10
#define ARROW_BODY (ARROW_LENGTH - ARROW_SIZE + 2)
#define SHADOW_OFFSET 4

struct PREGUNTA
{
  char ejercicio [EJERCICIO_SIZE];
  int  secuencia;
  char codigo    [PREGUNTA_SIZE];
  char tema      [CODIGO_TEMA_SIZE    + 1];
  char descripcion_tema [DESCRIPCION_MATERIA_SIZE + 1];
  char subtema   [CODIGO_SUBTEMA_SIZE + 1];
  char descripcion_subtema [DESCRIPCION_MATERIA_SIZE + 1];
  long double media;
  long double varianza;
  long double alfa;
  long double Rpb;
  int Header;
  int frecuencia;
  int Year;
  int Month;
  int Day;
  char texto_pregunta [TEXTO_PREGUNTA_SIZE];
};

int Dias_Acumulados [12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334}; 

#define MAX_PREGUNTAS 2000
struct PREGUNTA Pre_Examen [MAX_PREGUNTAS];
int             Numero_Preguntas = 0;
long double     novedad_pre_examen;

int N_temas=0;
int N_subtemas=0;
PGresult *res_preguntas_nuevas;
long double media, varianza, desviacion, probabilidad, alfa, Rpb, novedad;

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX3030.glade", &error))
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

          /* Some fields are READ ONLY */
          Read_Only_Fields ();

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

void Connect_widgets()
{
  window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex3030"));
  window2                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex3030_usado"));
  window3                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));
  window4                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_reporte"));

  EB_refinamiento              = GTK_WIDGET (gtk_builder_get_object (builder, "EB_refinamiento"));
  FR_refinamiento              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_refinamiento"));

  EB_pre_examen          = GTK_WIDGET (gtk_builder_get_object (builder, "EB_pre_examen"));
  FR_pre_examen          = GTK_WIDGET (gtk_builder_get_object (builder, "FR_pre_examen"));
  
  FR_dif  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dif"));
  FR_disc = GTK_WIDGET (gtk_builder_get_object (builder, "FR_disc"));
  FR_Cron = GTK_WIDGET (gtk_builder_get_object (builder, "FR_Cron"));
  FR_dias = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dias"));
  DA_dif  = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dif"));
  DA_disc = GTK_WIDGET (gtk_builder_get_object (builder, "DA_disc"));
  DA_Cron = GTK_WIDGET (gtk_builder_get_object (builder, "DA_Cron"));
  DA_dias = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dias"));
  
  EB_actual              = GTK_WIDGET (gtk_builder_get_object (builder, "EB_actual"));
  FR_actual              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_actual"));
  EB_nuevo               = GTK_WIDGET (gtk_builder_get_object (builder, "EB_nuevo"));
  FR_nuevo               = GTK_WIDGET (gtk_builder_get_object (builder, "FR_nuevo"));
  FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  FR_botones_actual      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones_actual"));
  FR_botones_nuevo       = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones_nuevo"));
  FR_pregunta_actual     = GTK_WIDGET (gtk_builder_get_object (builder, "FR_pregunta_actual"));
  FR_pregunta_nueva      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_pregunta_nueva"));
  FR_preexamenusado      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_preexamenusado"));

  SP_codigo              = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_codigo"));

  EN_descripcion         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));
  EN_materia             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia"));
  EN_materia_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia_descripcion"));
  EN_esquema             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema"));
  EN_esquema_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema_descripcion"));
  EN_preguntas           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_preguntas"));
  SC_preguntas           = GTK_WIDGET (gtk_builder_get_object (builder, "SC_preguntas"));

  EN_ejercicio_viejo     = GTK_WIDGET (gtk_builder_get_object (builder, "EN_ejercicio_viejo"));
  EN_secuencia_viejo     = GTK_WIDGET (gtk_builder_get_object (builder, "EN_secuencia_viejo"));
  EN_tema_viejo          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_tema_viejo"));
  EN_tema_descripcion_viejo          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_tema_descripcion_viejo"));
  EN_subtema_viejo       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_subtema_viejo"));
  EN_subtema_descripcion_viejo          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_subtema_descripcion_viejo"));
  EN_estudiantes_viejo   = GTK_WIDGET (gtk_builder_get_object (builder, "EN_estudiantes_viejo"));
  EN_media_viejo         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_viejo"));
  EN_desviacion_vieja    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion_vieja"));
  EN_alfa_viejo          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_viejo"));
  EN_Rpb_viejo           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb_viejo"));
  TV_pregunta            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_pregunta"));
  buffer_TV_pregunta     = gtk_text_view_get_buffer(TV_pregunta);

  SC_preguntas_nuevas    = GTK_WIDGET (gtk_builder_get_object (builder, "SC_preguntas_nuevas"));
  EN_preguntas_nuevas    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_preguntas_nuevas"));
  EN_ejercicio           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_ejercicio"));
  EN_secuencia           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_secuencia"));
  EN_tema_nuevo          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_tema_nuevo"));
  EN_tema_descripcion_nuevo = GTK_WIDGET (gtk_builder_get_object (builder, "EN_tema_descripcion_nuevo"));
  EN_subtema_nuevo       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_subtema_nuevo"));
  EN_subtema_descripcion_nuevo = GTK_WIDGET (gtk_builder_get_object (builder, "EN_subtema_descripcion_nuevo"));
  EN_estudiantes_nuevo   = GTK_WIDGET (gtk_builder_get_object (builder, "EN_estudiantes_nuevo"));
  EN_media_nuevo         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_nuevo"));
  EN_desviacion_nueva    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion_nueva"));
  EN_alfa_nuevo          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_nuevo"));
  EN_Rpb_nuevo           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb_nuevo"));
  TV_pregunta_nueva      = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_pregunta_nueva"));
  buffer_TV_pregunta_nueva = gtk_text_view_get_buffer(TV_pregunta_nueva);
  CB_tema                = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_tema"));
  CB_subtema             = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_subtema"));

  EN_estudiantes         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_estudiantes"));
  EN_media               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media"));
  EN_desviacion          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion"));
  EN_alfa                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa"));
  EN_Rpb                 = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb"));

  BN_delete      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_delete"));
  BN_add         = GTK_WIDGET (gtk_builder_get_object (builder, "BN_add"));
  BN_save        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
  BN_print       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_print_actual= GTK_WIDGET (gtk_builder_get_object (builder, "BN_print_actual"));
  BN_print_nuevo = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print_nuevo"));
  BN_undo        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_terminar    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));

  EB_reporte     = GTK_WIDGET (gtk_builder_get_object (builder, "EB_reporte"));
  FR_reporte     = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reporte"));
  PB_reporte     = GTK_WIDGET (gtk_builder_get_object (builder, "PB_reporte"));

  EB_update      = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
  FR_update      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));
  BN_ok          = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));

  g_signal_connect( G_OBJECT( DA_dif),  "expose-event", G_CALLBACK( on_DA_dif_expose  ), NULL);
  g_signal_connect( G_OBJECT( DA_disc), "expose-event", G_CALLBACK( on_DA_disc_expose ), NULL);
  g_signal_connect( G_OBJECT( DA_Cron), "expose-event", G_CALLBACK( on_DA_Cron_expose ), NULL);
  g_signal_connect( G_OBJECT( DA_dias), "expose-event", G_CALLBACK( on_DA_dias_expose ), NULL);
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_descripcion        , FALSE);
  gtk_widget_set_can_focus(EN_materia            , FALSE);
  gtk_widget_set_can_focus(EN_materia_descripcion, FALSE);
  gtk_widget_set_can_focus(EN_esquema            , FALSE);
  gtk_widget_set_can_focus(EN_esquema_descripcion, FALSE);
  gtk_widget_set_can_focus(EN_preguntas          , FALSE);

  gtk_widget_set_can_focus(EN_ejercicio_viejo    , FALSE);
  gtk_widget_set_can_focus(EN_secuencia_viejo    , FALSE);
  gtk_widget_set_can_focus(EN_tema_viejo         , FALSE);
  gtk_widget_set_can_focus(EN_tema_descripcion_viejo, FALSE);
  gtk_widget_set_can_focus(EN_subtema_viejo      , FALSE);
  gtk_widget_set_can_focus(EN_subtema_descripcion_viejo, FALSE);
  gtk_widget_set_can_focus(EN_estudiantes_viejo  , FALSE);
  gtk_widget_set_can_focus(EN_media_viejo        , FALSE);
  gtk_widget_set_can_focus(EN_desviacion_vieja   , FALSE);
  gtk_widget_set_can_focus(EN_alfa_viejo         , FALSE);
  gtk_widget_set_can_focus(EN_Rpb_viejo          , FALSE);

  gtk_widget_set_can_focus(EN_ejercicio          , FALSE);
  gtk_widget_set_can_focus(EN_secuencia          , FALSE);
  gtk_widget_set_can_focus(EN_tema_nuevo         , FALSE);
  gtk_widget_set_can_focus(EN_tema_descripcion_nuevo, FALSE);
  gtk_widget_set_can_focus(EN_subtema_nuevo      , FALSE);
  gtk_widget_set_can_focus(EN_subtema_descripcion_nuevo, FALSE);
  gtk_widget_set_can_focus(EN_estudiantes_nuevo  , FALSE);
  gtk_widget_set_can_focus(EN_media_nuevo        , FALSE);
  gtk_widget_set_can_focus(EN_desviacion_nueva   , FALSE);
  gtk_widget_set_can_focus(EN_alfa_nuevo         , FALSE);
  gtk_widget_set_can_focus(EN_Rpb_nuevo          , FALSE);

  gtk_widget_set_can_focus(EN_estudiantes        , FALSE);
  gtk_widget_set_can_focus(EN_media              , FALSE);
  gtk_widget_set_can_focus(EN_desviacion         , FALSE);
  gtk_widget_set_can_focus(EN_alfa               , FALSE);
  gtk_widget_set_can_focus(EN_Rpb                , FALSE);

  gtk_widget_set_can_focus(GTK_WIDGET(TV_pregunta),       FALSE);
  gtk_widget_set_can_focus(GTK_WIDGET(TV_pregunta_nueva), FALSE);
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,        GTK_STATE_NORMAL, &color);
  
  gdk_color_parse (IMPORTANT_WINDOW, &color);
  gtk_widget_modify_bg(window2, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (IMPORTANT_FR, &color);
  gtk_widget_modify_bg(FR_preexamenusado,     GTK_STATE_NORMAL, &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_refinamiento, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (SECONDARY_AREA, &color);
  gtk_widget_modify_bg(EB_pre_examen, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (SPECIAL_AREA_1, &color);
  gtk_widget_modify_bg(EB_nuevo,     GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_refinamiento, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_pre_examen,  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones_actual, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones_nuevo,  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_reporte,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_actual,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_pregunta_actual,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_nuevo,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_pregunta_nueva,     GTK_STATE_NORMAL, &color);

  gdk_color_parse (SECONDARY_AREA, &color);
  gtk_widget_modify_bg(EB_actual,     GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_add,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_delete,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print_actual, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print_nuevo,  GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,           GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_add,          GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_delete,       GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print_actual, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print_nuevo,  GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,           GTK_STATE_ACTIVE, &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_codigo),  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_estudiantes, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_media,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_desviacion,  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_alfa,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_Rpb,         GTK_STATE_NORMAL, &color);

  gdk_color_parse (PROCESSING_WINDOW, &color);
  gtk_widget_modify_bg(EB_reporte,  GTK_STATE_NORMAL,   &color); 

  gdk_color_parse (PROCESSING_PB, &color);
  gtk_widget_modify_bg(GTK_WIDGET(PB_reporte),  GTK_STATE_PRELIGHT, &color);
  
  gdk_color_parse (PROCESSING_FR, &color);
  gtk_widget_modify_bg(FR_reporte,  GTK_STATE_NORMAL, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,              GTK_STATE_NORMAL, &color);

  gdk_color_parse (FINISHED_WORK_FR, &color);
  gtk_widget_modify_bg(FR_update,              GTK_STATE_NORMAL, &color);
}

void Init_Fields()
{
   int  i, Last;
   char hilera [100];
   char Codigo [CODIGO_SIZE];


   /* Recupera el pre-examen con código más alto */
   res = PQEXEC(DATABASE, "SELECT codigo_pre_examen from EX_pre_examenes order by codigo_pre_examen DESC limit 1");
   Last = 0;
   if (PQntuples(res))
      {
       Last = atoi (PQgetvalue (res, 0, 0));
      }
   PQclear(res);

   sprintf (Codigo,"%05d",Last);

   if (Last > 1)
      {
       gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo), 1);
       gtk_spin_button_set_range (SP_codigo, 1.0, (long double) Last);
      }
   else
      { /* Primer pre-examen que se registra */
       gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo), 0);
       gtk_spin_button_set_range (SP_codigo, 0.0, (long double) Last);
      }
   gtk_spin_button_set_value (SP_codigo, Last);

   Numero_Preguntas = 0;
   sprintf (hilera,"%11d", Numero_Preguntas);
   gtk_entry_set_text(GTK_ENTRY(EN_preguntas),        hilera);
   gtk_entry_set_text(GTK_ENTRY(EN_preguntas_nuevas), hilera);

   gtk_entry_set_text(GTK_ENTRY(EN_descripcion)        , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia)            , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema)            , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion), "\0");

   gtk_range_set_range (GTK_RANGE(SC_preguntas), (gdouble) 0.0, (gdouble) 1.0);

   gtk_entry_set_text(GTK_ENTRY(EN_ejercicio_viejo),  "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_secuencia_viejo),  "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_tema_viejo),       "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_subtema_viejo),    "\0");

   gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_viejo),"\0");
   gtk_entry_set_text(GTK_ENTRY(EN_media_viejo)     , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_desviacion_vieja), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_alfa_viejo),       "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_Rpb_viejo),        "\0");

   Limpia_Datos_Nuevo ();

   gtk_text_buffer_set_text(buffer_TV_pregunta      , "\0", -1);
   gtk_text_buffer_set_text(buffer_TV_pregunta_nueva, "\0", -1);

   gtk_range_set_value (GTK_RANGE(SC_preguntas), (gdouble) 0.0);

   Update_Dials ();
   gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo) , 1);
   gtk_widget_set_sensitive(EN_descripcion        , 1);

   gtk_widget_set_sensitive(FR_actual             , 0);
   gtk_widget_set_sensitive(FR_nuevo              , 0);
   gtk_widget_set_sensitive(FR_pre_examen         , 0);
   gtk_widget_set_sensitive(EN_materia            , 0);
   gtk_widget_set_sensitive(EN_materia_descripcion, 0);
   gtk_widget_set_sensitive(EN_esquema            , 0);
   gtk_widget_set_sensitive(EN_esquema_descripcion, 0);
   gtk_widget_set_sensitive(EN_preguntas          , 0);

   gtk_widget_set_sensitive(BN_delete, 0);
   gtk_widget_set_sensitive(BN_save,   0);
   gtk_widget_set_sensitive(BN_print,  0);
   gtk_widget_set_sensitive(BN_undo,   1);

   gtk_combo_box_set_active (CB_tema, -1);
   for (i=0; i<N_temas; i++)
       {
	gtk_combo_box_remove_text (CB_tema, 0);
       }

   gtk_combo_box_set_active (CB_subtema, -1);
   for (i=0; i<N_subtemas; i++)
       {
	gtk_combo_box_remove_text (CB_subtema, 0);
       }

   if (Last > 1)
      gtk_widget_grab_focus   (GTK_WIDGET(SP_codigo));
   else
      Cambio_Codigo ();
}

void Cambio_Codigo()
{
  long int k;
  int i;
  char codigo[10];
  char PG_command [4000];
  PGresult *res, *res_aux;

  k = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
  sprintf (codigo, "%05ld", k);

  sprintf (PG_command,"SELECT codigo_Pre_Examen, esquema, descripcion_Pre_Examen, descripcion_esquema, materia, descripcion_materia from EX_pre_examenes, EX_esquemas, BD_materias where codigo_Pre_Examen = '%s' and codigo_esquema = esquema and materia =codigo_materia and codigo_tema = '          ' and codigo_subtema = '          ' ", codigo);

  res = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res))
     { /* Pre Examen ya existe... */
      sprintf (PG_command,"SELECT * from EX_examenes where pre_examen = '%.5s'", codigo);
      res_aux = PQEXEC(DATABASE, PG_command);
      if (PQntuples(res_aux))
	 { /* Pre examen ya fue usado - no se puede refinar */
          gtk_widget_set_sensitive(window1, 0);
          gtk_widget_show         (window2);
          gtk_widget_grab_focus   (GTK_WIDGET(SP_codigo));
         }
      else
	 {
          Carga_Pre_Examen (res, codigo);
          gtk_widget_set_sensitive(GTK_WIDGET(GTK_WIDGET(SP_codigo)), 0);
          gtk_widget_grab_focus   (SC_preguntas);
	 }
     }
  else
     {
      gtk_entry_set_text(GTK_ENTRY(EN_descripcion),"**** Pre_Examen no está registrado ****"); 
      gtk_widget_grab_focus (GTK_WIDGET(SP_codigo));
     }

  PQclear(res);
}

void Carga_Pre_Examen (PGresult * res, gchar * codigo)
{
  int i;
  char PG_command [2000];
  char hilera [500];
  PGresult *Q_preguntas, *Q_temas;

  gtk_widget_set_sensitive(EN_descripcion        , 1);
  gtk_widget_set_sensitive(EN_materia            , 1);
  gtk_widget_set_sensitive(EN_materia_descripcion, 1);
  gtk_widget_set_sensitive(EN_esquema            , 1);
  gtk_widget_set_sensitive(EN_esquema_descripcion, 1);
  gtk_widget_set_sensitive(EN_preguntas          , 1);
  gtk_widget_set_sensitive(EN_ejercicio          , 1);
  gtk_widget_set_sensitive(EN_media              , 1);
  gtk_widget_set_sensitive(EN_desviacion         , 1);

  gtk_widget_set_sensitive(SC_preguntas          , 1);

  gtk_entry_set_text(GTK_ENTRY(EN_descripcion),         PQgetvalue (res, 0, 2));
  gtk_entry_set_text(GTK_ENTRY(EN_materia),             PQgetvalue (res, 0, 4));
  gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), PQgetvalue (res, 0, 5));
  gtk_entry_set_text(GTK_ENTRY(EN_esquema),             PQgetvalue (res, 0, 1));
  gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion), PQgetvalue (res, 0, 3));

  gtk_combo_box_append_text(CB_tema, CODIGO_VACIO);
  sprintf (PG_command,"SELECT codigo_tema, descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema != '          ' and codigo_subtema = '          ' order by orden", PQgetvalue (res, 0, 4));
  Q_temas = PQEXEC(DATABASE, PG_command);
  N_temas = PQntuples(Q_temas)+1;
  for (i=1; i < N_temas; i++)
      {
       sprintf (hilera,"%s %s", PQgetvalue (Q_temas, i-1, 0), PQgetvalue (Q_temas, i-1, 1));
       gtk_combo_box_append_text(CB_tema, hilera);
      }
  gtk_combo_box_set_active (CB_tema, 0);
  PQclear(Q_temas);

  sprintf (PG_command,"SELECT ejercicio, secuencia, codigo_pregunta, texto_pregunta, dificultad, varianza_dificultad, usa_header, frecuencia, ultimo_uso_year, ultimo_uso_month, ultimo_uso_day, cronbach_alpha_media_con, point_biserial, tema, subtema from EX_pre_examenes_lineas, BD_ejercicios, BD_texto_ejercicios, BD_texto_preguntas, BD_estadisticas_preguntas where Pre_Examen = '%s' and codigo_pregunta = codigo_unico_pregunta and codigo_consecutivo_ejercicio = consecutivo_texto and codigo_unico_pregunta = pregunta and consecutivo_texto = texto_ejercicio and materia = '%s' order by ejercicio, secuencia_pregunta", codigo, PQgetvalue (res, 0, 4));
  Q_preguntas = PQEXEC(DATABASE, PG_command);

  Numero_Preguntas = PQntuples(Q_preguntas);
  sprintf (hilera,"%11d", Numero_Preguntas);
  gtk_entry_set_text(GTK_ENTRY(EN_preguntas), hilera);

  for (i=0;i<Numero_Preguntas;i++)
      {
       strcpy (Pre_Examen[i].ejercicio     , PQgetvalue (Q_preguntas, i, 0));
       Pre_Examen[i].secuencia =        atoi(PQgetvalue (Q_preguntas, i, 1));
       strcpy (Pre_Examen[i].codigo        , PQgetvalue (Q_preguntas, i, 2));
       strcpy (Pre_Examen[i].texto_pregunta, PQgetvalue (Q_preguntas, i, 3));
       Pre_Examen[i].media    =         atof(PQgetvalue (Q_preguntas, i, 4));
       Pre_Examen[i].varianza =         atof(PQgetvalue (Q_preguntas, i, 5));
       if (*PQgetvalue (Q_preguntas, i, 6) == 't') 
          Pre_Examen[i].Header = 1; 
       else 
          Pre_Examen[i].Header = 0; 
       Pre_Examen[i].frecuencia =       atoi(PQgetvalue (Q_preguntas, i, 7));
       Pre_Examen[i].Year       =       atoi(PQgetvalue (Q_preguntas, i, 8));
       Pre_Examen[i].Month      =       atoi(PQgetvalue (Q_preguntas, i, 9));
       Pre_Examen[i].Day        =       atoi(PQgetvalue (Q_preguntas, i, 10));
       Pre_Examen[i].alfa       =       atof(PQgetvalue (Q_preguntas, i, 11));
       Pre_Examen[i].Rpb        =       atof(PQgetvalue (Q_preguntas, i, 12));
       strcpy (Pre_Examen[i].tema          , PQgetvalue (Q_preguntas, i, 13));
       strcpy (Pre_Examen[i].subtema       , PQgetvalue (Q_preguntas, i, 14));
       /*
       */

      }

  PQclear(Q_preguntas);

  Update_Dials ();

  Establece_SC_preguntas (1);

  if (Numero_Preguntas)
     {
      gtk_widget_set_sensitive(BN_print,    1);
      gtk_widget_set_sensitive(BN_delete,   1);

      gtk_widget_set_sensitive(FR_pre_examen, 1);
      gtk_widget_set_sensitive(FR_actual    , 1);
      gtk_widget_set_sensitive(FR_nuevo     , 1);

      Despliega_Pregunta (1);

      gtk_text_buffer_set_text(buffer_TV_pregunta, Pre_Examen[0].texto_pregunta, -1);
     }
}

void Establece_SC_preguntas (int k)
{
  if (Numero_Preguntas > 1)
     {
      gtk_range_set_range (GTK_RANGE(SC_preguntas), (gdouble) 1.0, (gdouble) Numero_Preguntas);
      gtk_widget_set_sensitive(SC_preguntas, 1);
     }
  else
     {
      gtk_range_set_range (GTK_RANGE(SC_preguntas), (gdouble) 0.0, (gdouble) 1.0);
      gtk_widget_set_sensitive(SC_preguntas, 0);
     }

  gtk_range_set_value (GTK_RANGE(SC_preguntas), MINIMO((gdouble) k, (gdouble) Numero_Preguntas));
}

void Cambio_Tema(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *materia, *tema = NULL;
  char PG_command  [1000];
  PGresult *res;
  char hilera [300];
  char Codigo_materia [CODIGO_MATERIA_SIZE + 1];
  char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];

  materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);
  tema    = gtk_combo_box_get_active_text(CB_tema);

  Extrae_codigo (tema,    Codigo_tema);

  gtk_combo_box_set_active(CB_subtema, -1);
  for (i=0; i<N_subtemas; i++)
      {
       gtk_combo_box_remove_text (CB_subtema, 0);
      }

  if (strcmp(tema, CODIGO_VACIO) != 0)
     {
      gtk_combo_box_append_text(CB_subtema, CODIGO_VACIO);
      sprintf (PG_command,"SELECT codigo_subtema, descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' order by orden",
               materia, Codigo_tema); 
      res = PQEXEC(DATABASE, PG_command);
      N_subtemas = PQntuples(res)+1;
      for (i=1; i < N_subtemas; i++)
          {
	   sprintf(hilera,"%s %s" ,PQgetvalue (res, i-1, 0),PQgetvalue (res, i-1, 1));
           gtk_combo_box_append_text(CB_subtema, hilera);
          }
      PQclear(res);

      gtk_widget_set_sensitive (GTK_WIDGET(CB_subtema), 1);
      gtk_combo_box_set_active (CB_subtema, 0);
     }
  else
     {
      N_subtemas = 1;
      gtk_combo_box_append_text(CB_subtema, CODIGO_VACIO);
      gtk_combo_box_set_active (CB_subtema   , 0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_subtema), 0);
     }

  g_free (tema);
  g_free (materia);
}

void Cambio_Subtema(GtkWidget *widget, gpointer user_data)
{
  Carga_SC_preguntas_nuevas ();
}

void Carga_SC_preguntas_nuevas ()
{
   long int k;
   char * basic_query = "SELECT codigo_ejercicio, secuencia_pregunta, tema, subtema, N_estudiantes, dificultad, varianza_dificultad, cronbach_alpha_media_con, point_biserial, texto_pregunta, pregunta, usa_header, ultimo_uso_year, ultimo_uso_month, ultimo_uso_day from bd_ejercicios, bd_texto_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas where texto_ejercicio = consecutivo_texto and consecutivo_texto = codigo_consecutivo_ejercicio and pregunta = codigo_unico_pregunta ";

   char * order_query = "order by codigo_ejercicio, secuencia_pregunta";
   gchar *materia, *tema, *subtema;
   char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
   char Codigo_subtema [CODIGO_SUBTEMA_SIZE + 1];
   char PG_command [5000];
   int N_preguntas;
   char hilera[10];
   GdkColor color;

   if (res_preguntas_nuevas) PQclear(res_preguntas_nuevas);

   k = (long int) gtk_spin_button_get_value_as_int (SP_codigo);

   materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);
   tema    = gtk_combo_box_get_active_text(CB_tema);
   subtema = gtk_combo_box_get_active_text(CB_subtema);

   Extrae_codigo (tema,    Codigo_tema);
   Extrae_codigo (subtema, Codigo_subtema);

   if (strcmp(tema, CODIGO_VACIO) == 0)
      {
       sprintf (PG_command,"%s and materia = '%s' %s", 
		             basic_query, materia, order_query);
      }
   else
      {
       if (strcmp(subtema, CODIGO_VACIO) == 0)
          {
           sprintf (PG_command,
                    "%s and materia = '%s' and tema = '%s' %s", 
      		    basic_query, materia, Codigo_tema, order_query);
          }
       else
          {
           sprintf (PG_command,
                    "%s and materia = '%s' and tema = '%s' and subtema = '%s' %s", 
		    basic_query, materia, Codigo_tema, Codigo_subtema, order_query);
          }
      }

   res_preguntas_nuevas         = PQEXEC(DATABASE, PG_command);
   N_preguntas = PQntuples(res_preguntas_nuevas);
   sprintf (hilera,"%d", N_preguntas);
   gtk_entry_set_text(GTK_ENTRY(EN_preguntas_nuevas), hilera);

   if (N_preguntas == 0)
      {
       gdk_color_parse (DARK_AREA, &color);
       gtk_widget_modify_bg(EB_nuevo,     GTK_STATE_NORMAL, &color);

       gtk_widget_set_sensitive (BN_add,         0);
       gtk_widget_set_sensitive (BN_print_nuevo, 0);
       gtk_widget_set_sensitive (SC_preguntas_nuevas, 0);
       gtk_range_set_range (GTK_RANGE(SC_preguntas_nuevas),  (gdouble) 0.0, (gdouble) 1.0);
       gtk_range_set_value (GTK_RANGE(SC_preguntas_nuevas),  (gdouble) 0.0);
       gtk_entry_set_text(GTK_ENTRY(EN_ejercicio),         "\0");
       gtk_entry_set_text(GTK_ENTRY(EN_secuencia),         "\0");
       gtk_entry_set_text(GTK_ENTRY(EN_tema_nuevo),        "\0");
       gtk_entry_set_text(GTK_ENTRY(EN_tema_descripcion_nuevo),        "\0");
       gtk_entry_set_text(GTK_ENTRY(EN_subtema_nuevo),     "\0");
       gtk_entry_set_text(GTK_ENTRY(EN_subtema_descripcion_nuevo),     "\0");
       gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_nuevo), "\0");
       gtk_entry_set_text(GTK_ENTRY(EN_media_nuevo),       "\0");
       gtk_entry_set_text(GTK_ENTRY(EN_desviacion_nueva),  "\0");
       gtk_entry_set_text(GTK_ENTRY(EN_alfa_nuevo),        "\0");
       gtk_entry_set_text(GTK_ENTRY(EN_Rpb_nuevo),         "\0");
       gtk_text_buffer_set_text(buffer_TV_pregunta_nueva, 
       "\n\n                                                                       *** No hay preguntas disponibles de este Tema y Subtema ***",-1);
      }
   else
      {
       gdk_color_parse (SPECIAL_AREA_1, &color);
       gtk_widget_modify_bg(EB_nuevo,     GTK_STATE_NORMAL, &color);


       gtk_widget_set_sensitive (BN_add,         1);
       gtk_widget_set_sensitive (BN_print_nuevo, 1);

       if (N_preguntas > 1)
          {
           gtk_widget_set_sensitive (SC_preguntas_nuevas, 1);
           gtk_range_set_range (GTK_RANGE(SC_preguntas_nuevas),  (gdouble) 1.0, (gdouble) N_preguntas);
           gtk_range_set_value (GTK_RANGE(SC_preguntas_nuevas),  (gdouble) N_preguntas);  /* Para forzar que refresque */
	  }
       else
	  { /* Una sola pregunta - se desactiva selector */
           gtk_widget_set_sensitive (SC_preguntas_nuevas, 0);
           gtk_range_set_range (GTK_RANGE(SC_preguntas_nuevas),  (gdouble) 0.0, (gdouble) 1.0);
           gtk_range_set_value (GTK_RANGE(SC_preguntas_nuevas),  (gdouble) 0.0);  /* Para forzar que refresque */
	  }

       gtk_range_set_value (GTK_RANGE(SC_preguntas_nuevas),  (gdouble) 1);
      }
}

void Extrae_codigo (char * hilera, char * codigo)
{
  int i;

  i = 0;

  while (hilera[i] != ' ') codigo [i] = hilera[i++];

  codigo[i] = '\0';
}

int Busca_pregunta (char * ejercicio, int secuencia)
{
  int i;

  for (i=0; (i < Numero_Preguntas) && ((strcmp (ejercicio, Pre_Examen [i].ejercicio) != 0) || (secuencia != Pre_Examen [i].secuencia)); i++);

  if (i == Numero_Preguntas)
     return (0);
  else
     return (i+1);
}

void Borra_Pregunta ()
{
  int i, j, k, n;
  char hilera[20];

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  for (j=k; j < Numero_Preguntas; j++) Pre_Examen [j-1] = Pre_Examen [j];
  Numero_Preguntas--;

  Update_Dials();

  sprintf (hilera,"%11d", Numero_Preguntas);
  gtk_entry_set_text(GTK_ENTRY(EN_preguntas), hilera);

  Establece_SC_preguntas (k);

  if (!Numero_Preguntas)
     {  /* Borramos todas las preguntas */
      gtk_widget_set_sensitive(FR_actual, 0);
      gtk_widget_set_sensitive(BN_print,  0);

      gtk_entry_set_text(GTK_ENTRY(EN_ejercicio_viejo),   "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_secuencia_viejo),   "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_tema_viejo),        "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_tema_descripcion_viejo),        "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_subtema_viejo),     "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_subtema_descripcion_viejo),     "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_viejo), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_media_viejo),       "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_desviacion_vieja),  "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_viejo),        "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_Rpb_viejo),         "\0");
      gtk_text_buffer_set_text(buffer_TV_pregunta,        "\0", -1);
     }
  else
     if (k <= Numero_Preguntas) Despliega_Pregunta(k);

   gtk_widget_set_sensitive(BN_save,   1);
}

void Cambio_Preguntas_Nuevas ()
{
  long int k;
  int N_estudiantes;
  long double Rpb, Dificultad;
  char hilera[50];
  PGresult *res_aux;
  GdkColor color;
  long double frecuencia_total, frecuencia_opcion, p;
  int posicion;

  int n;
  char codigo[10];
  char PG_command [2000];
  char text   [2];
  PGresult *res, *Q_descripcion;
  gchar *materia;
  
  k = (long int) gtk_range_get_value (GTK_RANGE(SC_preguntas_nuevas)) - 1;

  gtk_entry_set_text(GTK_ENTRY(EN_ejercicio), PQgetvalue (res_preguntas_nuevas, k, 0));
  sprintf (hilera,"%d", atoi (PQgetvalue (res_preguntas_nuevas, k, 1)));
  gtk_entry_set_text(GTK_ENTRY(EN_secuencia), hilera);
  gtk_entry_set_text(GTK_ENTRY(EN_tema_nuevo),    PQgetvalue (res_preguntas_nuevas, k, 2));
  gtk_entry_set_text(GTK_ENTRY(EN_subtema_nuevo), PQgetvalue (res_preguntas_nuevas, k, 3));
  sprintf (hilera,"%d", atoi (PQgetvalue (res_preguntas_nuevas, k, 4)));
  gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_nuevo), hilera);
  sprintf (hilera,"%7.4Lf", (long double) atof (PQgetvalue (res_preguntas_nuevas, k, 5)));
  gtk_entry_set_text(GTK_ENTRY(EN_media_nuevo), hilera);
  sprintf (hilera,"%7.4Lf", (long double) sqrt(atof (PQgetvalue (res_preguntas_nuevas, k, 6))));
  gtk_entry_set_text(GTK_ENTRY(EN_desviacion_nueva), hilera);
  sprintf (hilera,"%7.4Lf", (long double) atof (PQgetvalue (res_preguntas_nuevas, k, 7)));
  gtk_entry_set_text(GTK_ENTRY(EN_alfa_nuevo), hilera);
  sprintf (hilera,"%7.4Lf", (long double) atof (PQgetvalue (res_preguntas_nuevas, k, 8)));
  gtk_entry_set_text(GTK_ENTRY(EN_Rpb_nuevo), hilera);

  materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);
  
  sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          ' order by orden", materia, PQgetvalue (res_preguntas_nuevas, k, 2));
  Q_descripcion = PQEXEC(DATABASE, PG_command);
  gtk_entry_set_text(GTK_ENTRY(EN_tema_descripcion_nuevo), PQgetvalue (Q_descripcion, 0, 0));
  PQclear(Q_descripcion);

  sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s' order by orden", materia, PQgetvalue (res_preguntas_nuevas, k, 2), PQgetvalue (res_preguntas_nuevas, k, 3));
  Q_descripcion = PQEXEC(DATABASE, PG_command);
  gtk_entry_set_text(GTK_ENTRY(EN_subtema_descripcion_nuevo), PQgetvalue (Q_descripcion, 0, 0));
  PQclear(Q_descripcion);
  
  g_free (materia);
  
  posicion = Busca_pregunta (PQgetvalue (res_preguntas_nuevas, k, 0), atoi (PQgetvalue (res_preguntas_nuevas, k, 1)));

  if (!posicion)
     {
      gdk_color_parse (SPECIAL_AREA_1, &color);
      gtk_widget_modify_bg(EB_nuevo,     GTK_STATE_NORMAL, &color);
      gtk_widget_set_sensitive (GTK_WIDGET (TV_pregunta_nueva), 1);

      gtk_text_buffer_set_text(buffer_TV_pregunta_nueva, PQgetvalue (res_preguntas_nuevas, k, 9), -1);
      if (Numero_Preguntas < MAX_PREGUNTAS)
         gtk_widget_set_sensitive (BN_add,            1);
      else
         gtk_widget_set_sensitive (BN_add,            0);

      gtk_widget_set_sensitive (BN_print_nuevo,    1);
     }
  else
     {
      gdk_color_parse (DARK_AREA, &color);
      gtk_widget_modify_bg(EB_nuevo,     GTK_STATE_NORMAL, &color);
      gtk_widget_set_sensitive (GTK_WIDGET (TV_pregunta_nueva), 0);

      gtk_range_set_value (GTK_RANGE(SC_preguntas),  (gdouble) posicion);
      gtk_text_buffer_set_text(buffer_TV_pregunta_nueva, 
      "\n\n                  *** Pregunta ya está incluida en el Preexamen ***",-1);
      gtk_widget_set_sensitive (BN_add,            0);
      gtk_widget_set_sensitive (BN_print_nuevo,    0);
     }
}

void Agrega_Pregunta ()
{
   gchar * ejercicio, *secuencia;
   int numero_secuencia;
   int i, j, k;
   struct PREGUNTA temporal;
   char hilera[20];

   ejercicio = gtk_editable_get_chars(GTK_EDITABLE(EN_ejercicio), 0, -1);
   secuencia = gtk_editable_get_chars(GTK_EDITABLE(EN_secuencia), 0, -1);
   numero_secuencia = atoi (secuencia);

   k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas_nuevas))-1;

   strcpy (Pre_Examen[Numero_Preguntas].ejercicio, ejercicio);
	   Pre_Examen[Numero_Preguntas].secuencia = numero_secuencia;
   strcpy (Pre_Examen[Numero_Preguntas].tema,              PQgetvalue (res_preguntas_nuevas, k, 2));
   strcpy (Pre_Examen[Numero_Preguntas].subtema,           PQgetvalue (res_preguntas_nuevas, k, 3));
	   Pre_Examen[Numero_Preguntas].frecuencia = atoi (PQgetvalue (res_preguntas_nuevas, k, 4));
 	   Pre_Examen[Numero_Preguntas].media      = atof (PQgetvalue (res_preguntas_nuevas, k, 5));
	   Pre_Examen[Numero_Preguntas].varianza   = atof (PQgetvalue (res_preguntas_nuevas, k, 6));
	   Pre_Examen[Numero_Preguntas].alfa       = atof (PQgetvalue (res_preguntas_nuevas, k, 7));
	   Pre_Examen[Numero_Preguntas].Rpb        = atof (PQgetvalue (res_preguntas_nuevas, k, 8));
   strcpy (Pre_Examen[Numero_Preguntas].texto_pregunta,    PQgetvalue (res_preguntas_nuevas, k, 9));
   strcpy (Pre_Examen[Numero_Preguntas].codigo,            PQgetvalue (res_preguntas_nuevas, k, 10));
	   Pre_Examen[Numero_Preguntas].Header     =     (*PQgetvalue (res_preguntas_nuevas, k, 11) == 't');
 	   Pre_Examen[Numero_Preguntas].Year       = atoi (PQgetvalue (res_preguntas_nuevas, k, 12));
 	   Pre_Examen[Numero_Preguntas].Month      = atoi (PQgetvalue (res_preguntas_nuevas, k, 13));
 	   Pre_Examen[Numero_Preguntas].Day        = atoi (PQgetvalue (res_preguntas_nuevas, k, 14));

   Numero_Preguntas++;

   if (Numero_Preguntas == 1)
      {
       gtk_range_set_range (GTK_RANGE(SC_preguntas), (gdouble) 0.0, (gdouble) Numero_Preguntas);
       gtk_range_set_value (GTK_RANGE(SC_preguntas), (gdouble) 1.0);
       gtk_widget_set_sensitive(SC_preguntas, 0);

       gtk_widget_set_sensitive(BN_print,  1);
       gtk_widget_set_sensitive(BN_delete, 1);
       gtk_widget_set_sensitive(FR_actual, 1);
      }
   else
      {
       gtk_range_set_range (GTK_RANGE(SC_preguntas), (gdouble) 1.0, (gdouble) Numero_Preguntas);
      }

   sprintf (hilera,"%11d", Numero_Preguntas);
   gtk_entry_set_text(GTK_ENTRY(EN_preguntas), hilera);

   for (i= Numero_Preguntas; i >= 0; i--)
       for (j=0; j < (i-1); j++)
 	   {
	    if ((strcmp(Pre_Examen[j].ejercicio,Pre_Examen[j+1].ejercicio)  > 0) ||
	       ((strcmp(Pre_Examen[j].ejercicio,Pre_Examen[j+1].ejercicio) == 0) && 
                (Pre_Examen[j].secuencia > Pre_Examen[j+1].secuencia)))
	       {
	        temporal         = Pre_Examen [j+1];
                Pre_Examen [j+1] = Pre_Examen [j];
                Pre_Examen   [j] = temporal;
	       }
	   } 


   i = Busca_pregunta (ejercicio, numero_secuencia);
   Establece_SC_preguntas (i);
   Despliega_Pregunta (i);
   Cambio_Preguntas_Nuevas ();
   Update_Dials ();
   gtk_widget_set_sensitive(BN_save,   1);

   g_free (ejercicio);
   g_free (secuencia);
}

void Limpia_Datos_Nuevo ()
{
  gtk_widget_set_sensitive (BN_add,         0);
  gtk_widget_set_sensitive (BN_print_nuevo, 0);

  gtk_range_set_range (GTK_RANGE(SC_preguntas_nuevas), (gdouble) 0.0, (gdouble) 1.0);
  gtk_range_set_value (GTK_RANGE(SC_preguntas_nuevas), (gdouble) 0.0);

  gtk_entry_set_text(GTK_ENTRY(EN_ejercicio),        "\0");
  gtk_entry_set_text(GTK_ENTRY(EN_secuencia),        "\0");
  gtk_entry_set_text(GTK_ENTRY(EN_tema_nuevo),       "\0");
  gtk_entry_set_text(GTK_ENTRY(EN_subtema_nuevo),    "\0");
  gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_nuevo),"\0");
  gtk_entry_set_text(GTK_ENTRY(EN_media_nuevo)     , "\0");
  gtk_entry_set_text(GTK_ENTRY(EN_desviacion_nueva), "\0");
  gtk_entry_set_text(GTK_ENTRY(EN_alfa_nuevo),       "\0");
  gtk_entry_set_text(GTK_ENTRY(EN_Rpb_nuevo),        "\0");
  gtk_text_buffer_set_text(buffer_TV_pregunta_nueva, "\0", -1);
}

void Imprime_Pregunta_actual ()
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas)) - 1;
  Imprime_Pregunta (Pre_Examen[k].ejercicio, Pre_Examen[k].secuencia);
}

void Imprime_Pregunta_nueva ()
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas_nuevas)) - 1;
  Imprime_Pregunta (PQgetvalue (res_preguntas_nuevas, k, 0), atoi(PQgetvalue (res_preguntas_nuevas, k, 1)));
}

void Imprime_Pregunta (char * ejercicio, int secuencia)
{
   FILE * Archivo_Latex;
   char PG_command [1000];
   char ejercicio_texto[10];
   PGresult *res,*res_aux;
   char hilera_antes [9000], hilera_despues [9000];
   char alfileres [6];
   long double media, discriminacion;

   sprintf (PG_command,"SELECT texto_ejercicio from bd_ejercicios where codigo_ejercicio = '%s'",
            ejercicio);
   res = PQEXEC(DATABASE, PG_command);
   strcpy (ejercicio_texto,PQgetvalue (res, 0, 0));  

   Archivo_Latex = fopen ("EX3030.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");
   fprintf (Archivo_Latex, "\\title{Pregunta %s.%d}\n", ejercicio, secuencia);
   fprintf (Archivo_Latex, "\\maketitle\n\n");

   fprintf (Archivo_Latex, "\\rule{8.75cm}{5pt}\n\n");

   sprintf (PG_command,"SELECT * from bd_texto_preguntas, bd_estadisticas_preguntas where codigo_consecutivo_ejercicio = '%s' and secuencia_pregunta = %d and codigo_unico_pregunta = pregunta", 
            ejercicio_texto, secuencia);
   res = PQEXEC(DATABASE, PG_command);

   sprintf (PG_command,"SELECT usa_header, texto_header from bd_texto_ejercicios where consecutivo_texto = '%s'", ejercicio_texto);
   res_aux = PQEXEC(DATABASE, PG_command);

   fprintf (Archivo_Latex, "\\begin{questions}\n");

   fprintf (Archivo_Latex, "\n\n");

   if (*PQgetvalue (res_aux, 0, 0) == 't')
      {
       fprintf (Archivo_Latex,"\\question ");
       sprintf (hilera_antes,"%s \n %s", PQgetvalue (res_aux, 0, 1), PQgetvalue (res, 0, 8));
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
      }
   else
      {
       fprintf (Archivo_Latex,"\\question ");
       sprintf (hilera_antes,"%s", PQgetvalue (res, 0, 8));
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
      }

   strcpy (alfileres, PQgetvalue (res, 0, 15));

   fprintf (Archivo_Latex, "\\begin{answers}\n");

   fprintf(Archivo_Latex, "\\item ");
   if (alfileres [0] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   sprintf (hilera_antes," %s", PQgetvalue (res, 0, 9));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   if (*(PQgetvalue (res, 0, 14)) == 'A')
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   fprintf (Archivo_Latex, "\n\n");

   fprintf(Archivo_Latex, "\\item ");
   if (alfileres [1] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   sprintf (hilera_antes," %s", PQgetvalue (res, 0, 10));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   if (*(PQgetvalue (res, 0, 14)) == 'B')
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   fprintf (Archivo_Latex, "\n\n");

   fprintf(Archivo_Latex, "\\item ");
   if (alfileres [2] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   sprintf (hilera_antes," %s", PQgetvalue (res, 0, 11));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   if (*(PQgetvalue (res, 0, 14)) == 'C')
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   fprintf (Archivo_Latex, "\n\n");

   fprintf(Archivo_Latex, "\\item ");
   if (alfileres [3] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   sprintf (hilera_antes," %s", PQgetvalue (res, 0, 12));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   if (*(PQgetvalue (res, 0, 14)) == 'D')
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   fprintf (Archivo_Latex, "\n\n");

   fprintf(Archivo_Latex, "\\item ");
   if (alfileres [4] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   sprintf (hilera_antes," %s", PQgetvalue (res, 0, 13));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   if (*(PQgetvalue (res, 0, 14)) == 'E')
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\end{answers}\n");

   fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\begin{center}");
   media          = (long double) atof(PQgetvalue (res, 0, 26));
   discriminacion = (long double) atof(PQgetvalue (res, 0, 28));
   fprintf (Archivo_Latex,"\\framebox[6.0cm][l]{Dificultad: \\textbf{%6.4Lf} - Discriminaci\\'on: \\textbf{%6.4Lf}}\n\n", media, discriminacion);
   fprintf (Archivo_Latex, "\\end{center}");

   fprintf (Archivo_Latex, "\\end{questions}\n\n");

   fprintf (Archivo_Latex, "\\rule{8.75cm}{5pt}\n\n");

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX3030", 1, NULL, 0.0, 0.0, NULL, NULL);
}

void Imprime_Pre_Examen ()
{
  int i, k;
  long int j;
  FILE * Archivo_Latex;
  char hilera_antes [2000], hilera_despues [2000];
  char codigo[10];
  gchar *materia, *materia_descripcion, *descripcion, *esquema, *esquema_descripcion, *hilera;
  int actual, N_preguntas_ejercicio;
  char ejercicio_actual[6];
  PGresult *res_header, *res_aux;
  char PG_command [2000];
  long double media, desviacion;

   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show         (window4);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   j = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
   sprintf (codigo, "%05ld", j);

   descripcion         = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion)        , 0, -1);
   materia             = gtk_editable_get_chars(GTK_EDITABLE(EN_materia)            , 0, -1);
   materia_descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_materia_descripcion), 0, -1);
   esquema             = gtk_editable_get_chars(GTK_EDITABLE(EN_esquema)            , 0, -1);
   esquema_descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_esquema_descripcion), 0, -1);

   hilera              = gtk_editable_get_chars(GTK_EDITABLE(EN_media)            , 0, -1);
   media = (long double) atof(hilera);
   g_free(hilera);

   hilera              = gtk_editable_get_chars(GTK_EDITABLE(EN_desviacion)            , 0, -1);
   desviacion          = (long double) atof(hilera);
   g_free(hilera);

   Archivo_Latex = fopen ("EX3030.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{Pre Examen}\n");
   fprintf (Archivo_Latex, "\\SetWatermarkLightness{0.7}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");

   sprintf (hilera_antes, "\\title{%s\\\\(Pre Examen %s - Esquema %s)\\\\%s}\n", materia_descripcion, codigo, esquema, descripcion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\\maketitle\n\n");
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.05);
   while (gtk_events_pending ()) gtk_main_iteration ();

   actual = 0;
   strcpy (ejercicio_actual, Pre_Examen[actual].ejercicio);
   N_preguntas_ejercicio = 0;
   for (i = actual; (i < Numero_Preguntas) && (strcmp(ejercicio_actual, Pre_Examen[i].ejercicio) == 0); i++) N_preguntas_ejercicio++;

   if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

   if (Pre_Examen[actual].Header)
      {
       sprintf (PG_command,"SELECT texto_header from BD_ejercicios, BD_texto_ejercicios where codigo_ejercicio = '%s' and consecutivo_texto = texto_ejercicio", Pre_Examen[actual].ejercicio);
       res_header = PQEXEC(DATABASE, PG_command);
       if (N_preguntas_ejercicio > 1)
	  {
	   if (N_preguntas_ejercicio == 2)
	      fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
	   else
	      fprintf (Archivo_Latex, "\\textbf{Las preguntas %d a %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	   strcpy (hilera_antes, PQgetvalue (res_header, 0, 0));
           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf (Archivo_Latex, "%s\n", hilera_despues);
           fprintf (Archivo_Latex, "\\rule{8.9cm}{1pt}\n");
           fprintf (Archivo_Latex, "\\begin{questions}\n");
	  }
       else
	  {
           fprintf (Archivo_Latex, "\\begin{questions}\n");
	   Imprime_Pregunta_Pre_Examen (Pre_Examen[actual].ejercicio,Pre_Examen[actual].secuencia, Archivo_Latex, PQgetvalue (res_header,0,0));
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
	Imprime_Pregunta_Pre_Examen (Pre_Examen[actual+i].ejercicio,Pre_Examen[actual+i].secuencia, Archivo_Latex, " ");
       }

   if (N_preguntas_ejercicio > 1)
      fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

   actual += N_preguntas_ejercicio;
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.05+((long double) actual/Numero_Preguntas * 0.75));
   while (gtk_events_pending ()) gtk_main_iteration ();

   while (actual < Numero_Preguntas)
         {
          strcpy (ejercicio_actual, Pre_Examen[actual].ejercicio);
          N_preguntas_ejercicio = 0;
          for (i = actual; (i < Numero_Preguntas) && (strcmp(ejercicio_actual, Pre_Examen[i].ejercicio) == 0); i++) N_preguntas_ejercicio++;

          if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

          if (Pre_Examen[actual].Header)
             {
              sprintf (PG_command,"SELECT texto_header from BD_ejercicios, BD_texto_ejercicios where codigo_ejercicio = '%s' and consecutivo_texto = texto_ejercicio", Pre_Examen[actual].ejercicio);
              res_header = PQEXEC(DATABASE, PG_command);
              if (N_preguntas_ejercicio > 1)
	         {
	          if (N_preguntas_ejercicio == 2)
	             fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
	          else
	             fprintf (Archivo_Latex, "\\textbf{Las preguntas %d a %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	          strcpy (hilera_antes, PQgetvalue (res_header, 0, 0));
                  hilera_LATEX (hilera_antes, hilera_despues);
                  fprintf (Archivo_Latex, "%s\n", hilera_despues);
                  fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
	         }
              else
	         {
	          Imprime_Pregunta_Pre_Examen (Pre_Examen[actual].ejercicio,Pre_Examen[actual].secuencia, Archivo_Latex, PQgetvalue (res_header,0,0));
                  actual++;
                  N_preguntas_ejercicio = 0;
	         }
             }

          for (i=0; i < N_preguntas_ejercicio; i++)
              {
	       Imprime_Pregunta_Pre_Examen (Pre_Examen[actual+i].ejercicio,Pre_Examen[actual+i].secuencia, Archivo_Latex, " ");
              }

          if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

          actual += N_preguntas_ejercicio;
          gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.05+((long double) actual/Numero_Preguntas * 0.75));
          while (gtk_events_pending ()) gtk_main_iteration ();
         }

   fprintf (Archivo_Latex, "\\end{questions}\n");
   fprintf (Archivo_Latex, "\\onecolumn\n");
   fprintf (Archivo_Latex, "\\section*{An\\'{a}lisis}\n");

   Grafico_y_Tabla (Archivo_Latex, media, desviacion, Numero_Preguntas);

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.82);
   while (gtk_events_pending ()) gtk_main_iteration ();

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX3030", 1, PB_reporte, 0.82, 0.17, NULL, NULL);
   system ("rm EX3030*.pdf");

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 1.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   gtk_widget_hide (window4);
   gtk_widget_set_sensitive(window1, 1);

   g_free(descripcion);
   g_free(materia);
   g_free(materia_descripcion);
   g_free(esquema);
   g_free(esquema_descripcion);
}

void Imprime_Pregunta_Pre_Examen (char * ejercicio, int secuencia, FILE * Archivo_Latex,char * prefijo)
{
   char hilera_antes [4000], hilera_despues [4000];
   PGresult * res_aux;
   char PG_command[2000];
   long double media, varianza, desviacion;

   sprintf (PG_command,"SELECT * from BD_texto_preguntas, BD_estadisticas_preguntas, BD_ejercicios where codigo_ejercicio = '%s' and texto_ejercicio = codigo_consecutivo_ejercicio and secuencia_pregunta = %d and pregunta = codigo_unico_pregunta",
            ejercicio, secuencia);
   res_aux = PQEXEC(DATABASE, PG_command);

   sprintf (hilera_antes,"\\question %s\n%s", prefijo, PQgetvalue (res_aux, 0, 8));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   fprintf (Archivo_Latex, "\\begin{answers}\n");

   fprintf (Archivo_Latex, "\\item ");
   strcpy (hilera_antes, PQgetvalue (res_aux, 0, 9));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item ");
   strcpy (hilera_antes, PQgetvalue (res_aux, 0, 10));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item ");
   strcpy (hilera_antes, PQgetvalue (res_aux, 0, 11));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item ");
   strcpy (hilera_antes, PQgetvalue (res_aux, 0, 12));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item ");
   strcpy (hilera_antes, PQgetvalue (res_aux, 0, 13));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\end{answers}\n");

   if (atoi(PQgetvalue (res_aux, 0, 22)))
      {
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Creada: %s/%s/%s - \\'{U}ltimo Uso: %s/%s/%s - %s %s}\n\n",    PQgetvalue (res_aux, 0, 6), PQgetvalue (res_aux, 0, 5), PQgetvalue (res_aux, 0, 4),
                PQgetvalue (res_aux, 0, 19), PQgetvalue (res_aux, 0, 18), PQgetvalue (res_aux, 0, 17), PQgetvalue (res_aux, 0, 22), strcmp("1",PQgetvalue (res_aux, 0, 22))==0?"vez":"veces");
      }
   else
      {
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Creada: %s/%s/%s - \\textbf{No ha sido usada}}\n\n",    PQgetvalue (res_aux, 0, 6), PQgetvalue (res_aux, 0, 5), PQgetvalue (res_aux, 0, 4));
      }

   media      = (long double) atof(PQgetvalue (res_aux, 0, 26));
   varianza   = (long double) atof(PQgetvalue (res_aux, 0, 27));
   desviacion = sqrt(varianza);
   fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Media: %6.4Lf - Varianza: %6.4Lf - Desviaci\\'{o}n: %6.4Lf}\n\n", media, varianza, desviacion);

   if (atoi(PQgetvalue (res_aux, 0, 22)))
      {
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{\\'{I}ndice de Discriminaci\\'{o}n ($r_{pb}$): %6.4Lf}\n\n", (long double)atof(PQgetvalue (res_aux, 0, 28)));
      }

   fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
   fprintf (Archivo_Latex, "\n\n");

   PQclear(res_aux);
}

void Graba_Pre_Examen ()
{
  long int k;
  int i;
  char codigo[10];
  gchar * descripcion, *esquema;
  char PG_command [2000];
  long double media, desviacion, alfa, Rpb;
  int frecuencia;
  PGresult *res;

  k = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
  sprintf (codigo, "%05ld", k);

  descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion)     , 0, -1);
  esquema     = gtk_editable_get_chars(GTK_EDITABLE(EN_esquema)         , 0, -1);

  media       = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_media)      , 0, -1));
  desviacion  = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_desviacion) , 0, -1));
  frecuencia  = atoi(gtk_editable_get_chars(GTK_EDITABLE(EN_estudiantes), 0, -1));
  alfa        = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_alfa)       , 0, -1));
  Rpb         = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_Rpb)        , 0, -1));

  /* start a transaction block */
  res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

  /* Al borrar el Pre-examen, se borran sus líneas también por el ON DELETE CASCADE */
  sprintf (PG_command,"DELETE from EX_pre_examenes where codigo_Pre_Examen = '%.5s'", codigo);
  res = PQEXEC(DATABASE, PG_command); PQclear(res);

  sprintf (PG_command,"INSERT into EX_pre_examenes values ('%.5s','%.5s','%.200s',%Lf, %Lf, %d, %Lf, %Lf, %Lf)", 
	    codigo, esquema, descripcion, media, desviacion, frecuencia, novedad_pre_examen, alfa, Rpb);
  res = PQEXEC(DATABASE, PG_command); PQclear(res);

  for (i=0; i < Numero_Preguntas;i++)
      {
       sprintf (PG_command,"INSERT into EX_pre_examenes_lineas values ('%.5s','%.6s','%.6s',%d)", 
		 codigo, Pre_Examen [i].codigo, Pre_Examen [i].ejercicio, Pre_Examen [i].secuencia); 
       res = PQEXEC(DATABASE, PG_command); PQclear(res);
      }

  /* end the transaction */
  res = PQEXEC(DATABASE, "END"); PQclear(res);

  g_free (esquema);
  g_free (descripcion);
  
  carga_parametros_EXAMINER (&parametros, DATABASE);
  if (parametros.report_update)
     {
      gtk_widget_set_sensitive(window1, 0);
      gtk_widget_show (window3);

      if (parametros.timeout)
         {
          while (gtk_events_pending ()) gtk_main_iteration ();
          catNap(parametros.timeout);
          gtk_widget_hide (window3);
          gtk_widget_set_sensitive(window1, 1);
         }
     }

  Init_Fields ();
}

void Cambia_Pregunta ()
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k) Despliega_Pregunta (k);
}

void Despliega_Pregunta (int k)
{
  char hilera[20];
  PGresult *Q_descripcion;
  char PG_command [2000];
  gchar *materia;
  
  gtk_entry_set_text(GTK_ENTRY(EN_ejercicio_viejo), Pre_Examen[k-1].ejercicio);
  sprintf(hilera, "%d", Pre_Examen[k-1].secuencia);
  gtk_entry_set_text(GTK_ENTRY(EN_secuencia_viejo), hilera);
  gtk_entry_set_text(GTK_ENTRY(EN_tema_viejo),    Pre_Examen[k-1].tema);
  gtk_entry_set_text(GTK_ENTRY(EN_subtema_viejo), Pre_Examen[k-1].subtema);

  sprintf (hilera,"%d", Pre_Examen[k-1].frecuencia);
  gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_viejo), hilera);
  sprintf (hilera,"%7.4Lf", Pre_Examen[k-1].media);
  gtk_entry_set_text(GTK_ENTRY(EN_media_viejo), hilera);
  sprintf (hilera,"%7.4Lf", (long double) sqrt(Pre_Examen[k-1].varianza));
  gtk_entry_set_text(GTK_ENTRY(EN_desviacion_vieja), hilera);
  sprintf (hilera,"%7.4Lf", Pre_Examen[k-1].alfa);
  gtk_entry_set_text(GTK_ENTRY(EN_alfa_viejo), hilera);
  sprintf (hilera,"%7.4Lf", Pre_Examen[k-1].Rpb);
  gtk_entry_set_text(GTK_ENTRY(EN_Rpb_viejo), hilera);

  gtk_text_buffer_set_text(buffer_TV_pregunta,      Pre_Examen[k-1].texto_pregunta, -1);
  
  materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);
  
  sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          ' order by orden", materia, Pre_Examen[k-1].tema);
  Q_descripcion = PQEXEC(DATABASE, PG_command);
  gtk_entry_set_text(GTK_ENTRY(EN_tema_descripcion_viejo), PQgetvalue (Q_descripcion, 0, 0));
  PQclear(Q_descripcion);

  sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s' order by orden", materia, Pre_Examen[k-1].tema, Pre_Examen[k-1].subtema);
  Q_descripcion = PQEXEC(DATABASE, PG_command);
  gtk_entry_set_text(GTK_ENTRY(EN_subtema_descripcion_viejo), PQgetvalue (Q_descripcion, 0, 0));
  PQclear(Q_descripcion);
  
  g_free (materia);
}

void Update_Dials()
{
#define DIAS_MAXIMO       730
  int i;
  char hilera[200];
  time_t curtime;
  struct tm *loctime;
  int month, year, day;
  int Total_Dias, Dias_sin_usar, uso;

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

  uso = 0;
  media = varianza = desviacion = probabilidad = alfa = novedad_pre_examen = Rpb =0.0;

  Total_Dias = 0;
  for (i=0; i < Numero_Preguntas; i++)
      {
       media    += Pre_Examen [i].media;
       varianza += Pre_Examen [i].varianza;
       uso      += Pre_Examen [i].frecuencia;
       alfa     += Pre_Examen [i].alfa*(Pre_Examen [i].frecuencia+1);
       Rpb      += Pre_Examen [i].Rpb*(Pre_Examen [i].frecuencia+1);

       if (Pre_Examen[i].Day)
	  {
           Dias_sin_usar = ((                  year - Pre_Examen[i].Year) * 365 +
             	            (Dias_Acumulados[month] - Dias_Acumulados[Pre_Examen[i].Month]) +
		            (                  day  - Pre_Examen[i].Day));
	   if (Dias_sin_usar > DIAS_MAXIMO) Dias_sin_usar = DIAS_MAXIMO;
	  }
       else
	  Dias_sin_usar = DIAS_MAXIMO;

       Total_Dias  += Dias_sin_usar;
      }

  if (Numero_Preguntas)
     {
      media               = media/Numero_Preguntas * 100;
      varianza            = varianza/Numero_Preguntas * 100;
      novedad_pre_examen  = (long double)Total_Dias/Numero_Preguntas;
      alfa                = alfa/(uso + Numero_Preguntas);
      Rpb                 = Rpb/(uso + Numero_Preguntas);
     }

  novedad = novedad_pre_examen / DIAS_MAXIMO;
  novedad = MINIMO (1.0, novedad);

  desviacion = sqrt(varianza);

  if (desviacion > 0.0) probabilidad = CDF(67.5, media, desviacion);

  sprintf (hilera,"%d", uso);
  gtk_entry_set_text(GTK_ENTRY(EN_estudiantes), hilera);
  sprintf (hilera,"%7.3Lf", media);
  gtk_entry_set_text(GTK_ENTRY(EN_media), hilera);
  sprintf (hilera,"%7.3Lf", desviacion);
  gtk_entry_set_text(GTK_ENTRY(EN_desviacion), hilera);
  sprintf (hilera,"%7.3Lf", alfa);
  gtk_entry_set_text(GTK_ENTRY(EN_alfa), hilera);
  sprintf (hilera,"%7.3Lf", Rpb);
  gtk_entry_set_text(GTK_ENTRY(EN_Rpb), hilera);

  gtk_widget_queue_draw(DA_dif);
  gtk_widget_queue_draw(DA_disc);
  gtk_widget_queue_draw(DA_dias);
  gtk_widget_queue_draw(DA_Cron);
}

long double CDF (long double X, long double Media, long double Desv)
{
  return (0.5*(1 + (long double)erf((X-Media)/(Desv*M_SQRT2))));
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

void Grafico_y_Tabla(FILE * Archivo_Latex, long double media, long double desviacion, int N_preguntas)
{
    fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
    fprintf (Archivo_Latex, "\\centering\n");
    fprintf (Archivo_Latex, "\\begin{minipage}[c]{0.25\\textwidth}\n");
    fprintf (Archivo_Latex, "\\centering\n");

    Tabla_Probabilidades   (Archivo_Latex, media, desviacion, N_preguntas);
    fprintf (Archivo_Latex, "\\end{minipage}\n");

    Prepara_Grafico_Normal (Archivo_Latex, media, desviacion, N_preguntas);

    fprintf (Archivo_Latex, "\\begin{minipage}[c]{0.71\\textwidth}\n");
    fprintf (Archivo_Latex, "\\includegraphics[scale=1.0]{EX3030-n.pdf}\n");
    fprintf (Archivo_Latex, "\\end{minipage}\n");
    fprintf (Archivo_Latex, "\\end{figure}\n");
}

void Prepara_Grafico_Normal(FILE * Archivo_Latex, long double media, long double desviacion, int N)
{
    FILE * Archivo_gnuplot;
    char Hilera_Antes [2000], Hilera_Despues [2000];
    char comando[1000];

    Archivo_gnuplot = fopen ("EX3030-n.gp","w");

    fprintf (Archivo_gnuplot, "set term postscript eps enhanced \"Times\" 12\n");
    fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
    fprintf (Archivo_gnuplot, "set size 1.1, 0.8\n");
    fprintf (Archivo_gnuplot, "set grid xtics\n");

    fprintf (Archivo_gnuplot, "set output \"EX3030-n.eps\"\n");
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
        fprintf (Archivo_gnuplot, "%s", Hilera_Despues);
       }

    fprintf (Archivo_gnuplot, "plot normal (x, %Lf, %Lf) with filledcurve above x1=67.50 notitle, normal (x, %Lf, %Lf) with lines linetype 1 lw 7 lc 3 notitle",
             media, desviacion,media, desviacion);

    fclose (Archivo_gnuplot);

    sprintf(comando, "%s EX3030-n.gp", parametros.gnuplot);
    system (comando);

    sprintf(comando, "mv EX3030-n.gp %s", parametros.ruta_gnuplot);
    system (comando);

    sprintf(comando, "%s EX3030-n.eps", parametros.epstopdf);
    system (comando);

    system ("rm EX3030-n.eps");
}

void Tabla_Probabilidades (FILE * Archivo_Latex, long double media, long double desviacion, int N)
{
   fprintf (Archivo_Latex, "\\rowcolors{1}{white}{green}\n");
   fprintf (Archivo_Latex, "\\begin{tabular}[b]{|r|r|}\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Nota} & \\textbf{Probabilidad} \\\\\n");
   fprintf (Archivo_Latex, "\\hline\\hline\n");

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

void Update_dificultad()
{
   long double p, r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   p = CDF( 67.5, media, desviacion);
   r = (1.0 - p) * PI;
   cos_r = cos(r);
   sin_r = sin(r);

   image = cairo_image_surface_create_from_png(".imagenes/dialdifex.png");
     
   cr = gdk_cairo_create(DA_dif->window );
  
   //   cairo_set_source_rgb(cr, 0.45, 0.45, 0.45);
   cairo_set_source_rgb(cr, 0.11, 0.11, 0.11);
     
   cairo_rectangle(cr, 0, 0, DA_WIDTH, DA_HEIGHT);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

   cairo_set_font_size(cr, 8);
   cairo_move_to(cr, 2, 8);
   cairo_show_text(cr, "Dificultad");  

   cairo_set_font_size(cr, 9);
   
   cairo_save(cr); 
   cairo_move_to(cr, 19, 48 + DA_BEGIN);
   cairo_rotate(cr, -0.94248); 
   cairo_show_text(cr, "FÁCIL"); 
   cairo_restore(cr);
   
   cairo_save(cr); 
   cairo_move_to(cr, 52, 14 + DA_BEGIN);
   cairo_rotate(cr, -0.31416); 
   cairo_show_text(cr, "JUSTO"); 
   cairo_restore(cr);
   
   cairo_save(cr); 
   cairo_move_to(cr, 101, 7 + DA_BEGIN);
   cairo_rotate(cr, 0.31416); 
   cairo_show_text(cr, "DURO"); 
   cairo_restore(cr);

   cairo_save(cr); 
   cairo_move_to(cr, 139, 21 + DA_BEGIN);
   cairo_rotate(cr, 0.94248); 
   cairo_show_text(cr, "SEVERO"); 
   cairo_restore(cr);

   cairo_stroke_preserve(cr);

   cairo_set_source_surface(cr, image, 7, DA_BEGIN);
   cairo_paint( cr );

   cairo_set_line_width (cr, 3);

// shadow
   cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

   x = round(cos_r * (ARROW_BODY - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_BODY - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * (ARROW_LENGTH - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);
   
// cuerpo de la aguja
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
      
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_arc( cr, DA_MIDDLE, CERO, 2, 0, 2 * G_PI );
   x = round(cos_r * ARROW_BODY - sin_r * 0);
   y = round(sin_r * ARROW_BODY + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * ARROW_LENGTH - sin_r * 0);
   y = round(sin_r * ARROW_LENGTH + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * ARROW_WIDTH);
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * ARROW_WIDTH);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * (0 - ARROW_WIDTH));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * (0 - ARROW_WIDTH));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);

   cairo_destroy( cr );
}

void Update_novedad()
{
   long double r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   r = (1.0 - novedad) * PI;
   cos_r = cos(r);
   sin_r = sin(r);

   image = cairo_image_surface_create_from_png(".imagenes/dialdias.png");
     
   cr = gdk_cairo_create(DA_dias->window );
  
   //   cairo_set_source_rgb(cr, 0.45, 0.45, 0.45);
   cairo_set_source_rgb(cr, 0.11, 0.11, 0.11);
     
   cairo_rectangle(cr, 0, 0, DA_WIDTH, DA_HEIGHT);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

   cairo_set_font_size(cr, 8);
   cairo_move_to(cr, 2, 8);
   cairo_show_text(cr, "Novedad");  

   cairo_set_font_size(cr, 9);
   
   cairo_save(cr); 
   cairo_move_to(cr, 11, 69 + DA_BEGIN);
   cairo_rotate(cr, -1.25664); 
   cairo_show_text(cr, "MUY"); 
   cairo_restore(cr);
   
   cairo_save(cr); 
   cairo_move_to(cr, 18, 44 + DA_BEGIN);
   cairo_rotate(cr, -0.78540); 
   cairo_show_text(cr, "CONOCIDO"); 
   cairo_restore(cr);
   
   cairo_save(cr); 
   cairo_move_to(cr, 110, 5 + DA_BEGIN);
   cairo_rotate(cr, 0.47124); 
   cairo_show_text(cr, "NUEVO"); 
   cairo_restore(cr);

   cairo_stroke_preserve(cr);

   cairo_set_source_surface(cr, image, 7, DA_BEGIN);
   cairo_paint( cr );

   cairo_set_line_width (cr, 3);

// shadow
   cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

   x = round(cos_r * (ARROW_BODY - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_BODY - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * (ARROW_LENGTH - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);
   
// cuerpo de la aguja
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
     
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_arc( cr, DA_MIDDLE, CERO, 2, 0, 2 * G_PI );
   x = round(cos_r * ARROW_BODY - sin_r * 0);
   y = round(sin_r * ARROW_BODY + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * ARROW_LENGTH - sin_r * 0);
   y = round(sin_r * ARROW_LENGTH + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * ARROW_WIDTH);
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * ARROW_WIDTH);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * (0 - ARROW_WIDTH));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * (0 - ARROW_WIDTH));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);

   cairo_destroy( cr );
}

void Update_alfa()
{
   long double r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   r = (1.0 - alfa) * PI;
   cos_r = cos(r);
   sin_r = sin(r);

   image = cairo_image_surface_create_from_png(".imagenes/dialcronbach.png");
     
   cr = gdk_cairo_create(DA_Cron->window );
  
   //   cairo_set_source_rgb(cr, 0.45, 0.45, 0.45);
   cairo_set_source_rgb(cr, 0.11, 0.11, 0.11);
     
   cairo_rectangle(cr, 0, 0, DA_WIDTH, DA_HEIGHT);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

   cairo_set_font_size(cr, 8);
   cairo_move_to(cr, 2, 8);
   cairo_show_text(cr, "Alfa de Cronbach");  

   cairo_set_font_size(cr, 8);
   
   cairo_save(cr); 
   cairo_move_to(cr, 139, 17 + DA_BEGIN);
   cairo_rotate(cr, -0.94248); 
   cairo_show_text(cr, "0.70"); 
   cairo_restore(cr);

   cairo_save(cr); 
   cairo_move_to(cr, 162, 47 + DA_BEGIN);
   cairo_rotate(cr, -0.47124); 
   cairo_show_text(cr, "0.85"); 
   cairo_restore(cr);

   cairo_stroke_preserve(cr);

   cairo_set_source_surface(cr, image, 7, DA_BEGIN);
   cairo_paint( cr );

   cairo_set_line_width (cr, 3);

// shadow
   cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

   x = round(cos_r * (ARROW_BODY - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_BODY - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * (ARROW_LENGTH - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);
   
// cuerpo de la aguja
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
     
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_arc( cr, DA_MIDDLE, CERO, 2, 0, 2 * G_PI );
   x = round(cos_r * ARROW_BODY - sin_r * 0);
   y = round(sin_r * ARROW_BODY + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * ARROW_LENGTH - sin_r * 0);
   y = round(sin_r * ARROW_LENGTH + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * ARROW_WIDTH);
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * ARROW_WIDTH);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * (0 - ARROW_WIDTH));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * (0 - ARROW_WIDTH));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);

   cairo_destroy( cr );
}

void Update_rpb()
{
   long double r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   r = (1.0 - ((1.0 + Rpb)/2.0)) * PI;
   cos_r = cos(r);
   sin_r = sin(r);

   image = cairo_image_surface_create_from_png(".imagenes/dialdiscex.png");
     
   cr = gdk_cairo_create(DA_disc->window );
  
   //   cairo_set_source_rgb(cr, 0.45, 0.45, 0.45);
   cairo_set_source_rgb(cr, 0.11, 0.11, 0.11);
     
   cairo_rectangle(cr, 0, 0, DA_WIDTH, DA_HEIGHT);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

   cairo_set_font_size(cr, 8);
   cairo_move_to(cr, 2, 8);
   cairo_show_text(cr, "Discriminación");  

   cairo_set_font_size(cr, 8);
   
   cairo_move_to(cr, DA_MIDDLE - 8, 2 + DA_BEGIN);
   cairo_show_text(cr, "0.0"); 

   cairo_save(cr); 
   cairo_move_to(cr, 128, 11 + DA_BEGIN);
   cairo_rotate(cr, -1.09956); 
   cairo_show_text(cr, "0.3"); 
   cairo_restore(cr);

   cairo_stroke_preserve(cr);

   cairo_set_source_surface(cr, image, 7, DA_BEGIN);
   cairo_paint( cr );

   cairo_set_line_width (cr, 3);

// shadow
   cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

   x = round(cos_r * (ARROW_BODY - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_BODY - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * (ARROW_LENGTH - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);
   
// cuerpo de la aguja
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_arc( cr, DA_MIDDLE, CERO, 2, 0, 2 * G_PI );
   x = round(cos_r * ARROW_BODY - sin_r * 0);
   y = round(sin_r * ARROW_BODY + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * ARROW_LENGTH - sin_r * 0);
   y = round(sin_r * ARROW_LENGTH + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * ARROW_WIDTH);
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * ARROW_WIDTH);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * (0 - ARROW_WIDTH));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * (0 - ARROW_WIDTH));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);

   cairo_destroy( cr );
}

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex3030_destroy (GtkWidget *widget, gpointer user_data) 
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

void on_SC_preguntas_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_Pregunta ();
}

void on_BN_delete_clicked(GtkWidget *widget, gpointer user_data)
{
  Borra_Pregunta ();
}

void on_BN_add_clicked(GtkWidget *widget, gpointer user_data)
{
  Agrega_Pregunta ();
}

void on_BN_print_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Pre_Examen ();
}

void on_BN_save_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Pre_Examen ();
}

void on_BN_OK_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  Init_Fields ();
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

void on_BN_print_actual_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Pregunta_actual ();
}

void on_BN_print_nuevo_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Pregunta_nueva ();
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

void on_SC_preguntas_nuevas_value_changed(GtkWidget *widget, gpointer user_data)
{
  long int k;

  k = (long int) gtk_range_get_value (GTK_RANGE(SC_preguntas_nuevas));

  if (k) Cambio_Preguntas_Nuevas ();
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window3);
  gtk_widget_set_sensitive(window1, 1);
}

void on_DA_dif_expose(GtkWidget *widget, gpointer user_data)
{
  Update_dificultad ();
}

void on_DA_disc_expose(GtkWidget *widget, gpointer user_data)
{
  Update_rpb ();
}

void on_DA_Cron_expose(GtkWidget *widget, gpointer user_data)
{
  Update_alfa ();
}

void on_DA_dias_expose(GtkWidget *widget, gpointer user_data)
{
  Update_novedad ();
}
