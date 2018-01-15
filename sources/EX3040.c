/*
Compilar con

cc -o EX3040 EX3040.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic

*/
/*******************************************/
/*  EX3040:                                */
/*                                         */
/*    Convierte un Preexamen en Examen     */
/*    The Examiner 0.0                     */
/*    21 de Enero 2011                     */
/*                                         */
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Se cambia distribución de campos     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    28 de Febrero del 2012               */
/*******************************************/
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <libpq-fe.h>
#include <locale.h>
#include "examiner.h"

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define MAXIMO_VERSIONES         20
#define CODIGO_SIZE              6
#define EJERCICIO_SIZE           7
#define PREGUNTA_SIZE            7
#define EXAMEN_SIZE              6
#define TEXTO_PREGUNTA_SIZE      4097

#define POR_SECUENCIA            0
#define POR_DIFICULTAD_ASC       1
#define POR_DIFICULTAD_DES       2
#define max(a,b) (a)>(b)?(a):(b)
struct timeval T;

struct timezone {
             int     tz_minuteswest; /* of Greenwich */
             int     tz_dsttime;     /* type of dst correction to apply */
     } Z;

struct PREGUNTA
{
  char ejercicio [EJERCICIO_SIZE];
  int  secuencia;
  char codigo    [PREGUNTA_SIZE];
  int  alfiler;
  int  Posicion_Ejercicio;
  char alfileres_opciones [6];
  int  correcta;
  int  orden [5];
  int  Header;
  char texto_pregunta [TEXTO_PREGUNTA_SIZE];
  char tema           [CODIGO_TEMA_SIZE    + 1];
  char subtema        [CODIGO_SUBTEMA_SIZE + 1];
  int N_estudiantes;
  long double media;
  long double desviacion;
  long double alfa;
  long double Rpb;
};

struct EJERCICIO
{
  char ejercicio      [EJERCICIO_SIZE];
  char tema           [CODIGO_TEMA_SIZE    + 1];
  int  orden_tema;
  char subtema        [CODIGO_SUBTEMA_SIZE + 1];
  int  orden_subtema;
  long double dificultad;
  int  primer_pregunta;
  int  alfiler;
};

#define MAX_PREGUNTAS 1000

int Numero_Preguntas = 0;
struct PREGUNTA   Preguntas_Base      [MAX_PREGUNTAS];

int Numero_Ejercicios = 0;
struct EJERCICIO  Ejercicios_Base     [MAX_PREGUNTAS];

int N_versiones = 0;
struct EJERCICIO * * Ejercicios_Version = NULL;
struct PREGUNTA  * * Preguntas_Version  = NULL;
int Numero_Temas = 0;

int N_autores = 0;

long double prediccion_media;
long double prediccion_desviacion;
int         Usos;
long double prediccion_alfa;
long double prediccion_Rpb;

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

GtkWidget *EB_pre2ex              = NULL;
GtkWidget *FR_pre2ex              = NULL;

GtkSpinButton *SP_pre_examen      = NULL;
GtkWidget *EN_descripcion         = NULL;
GtkWidget *EN_esquema             = NULL;
GtkWidget *EN_esquema_descripcion = NULL;
GtkWidget *EN_materia             = NULL;
GtkWidget *EN_materia_descripcion = NULL;
GtkWidget *EN_N_preguntas         = NULL;
GtkWidget *EN_N_ejercicios        = NULL;
GtkWidget *EN_N_temas             = NULL;
GtkWidget *SC_N_versiones         = NULL;
GtkComboBox *CB_profesor;

GtkWidget *EN_version              [MAXIMO_VERSIONES];
GtkComboBox *CB_orden              [MAXIMO_VERSIONES];
GtkToggleButton *CK_opciones       [MAXIMO_VERSIONES];

GtkWidget *EN_ejercicio           = NULL;
GtkWidget *EN_secuencia           = NULL;
GtkWidget *EN_tema                = NULL;
GtkWidget *EN_subtema             = NULL;

GtkWidget *EN_examen              = NULL;
GtkWidget *EN_examen_descripcion  = NULL;

GtkWidget *EB_orden_manual        = NULL;

GtkWidget *FR_versiones           = NULL;
GtkWidget *FR_orden_manual        = NULL;
GtkWidget *FR_pregunta_actual     = NULL;
GtkWidget *FR_botones             = NULL;

GtkWidget *SC_preguntas           = NULL;
GtkWidget *SC_ejercicios          = NULL;
GtkWidget *SC_posicion            = NULL;

GtkToggleButton *CK_alfiler_global = NULL;
GtkImage  *IM_alfiler_destino      = NULL;

GtkTextView *TV_pregunta            = NULL;

GtkWidget *BN_save                = NULL;
GtkWidget *BN_swap                = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_terminar            = NULL;

GtkTextBuffer * buffer_TV_pregunta;

GtkWidget *FR_update              = NULL;
GtkWidget *EB_update              = NULL;
GtkWidget *BN_ok                  = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Baraja_ejercicios                 (struct EJERCICIO * Ejercicios);
void Baraja_preguntas                  (struct EJERCICIO * Ejercicios, struct PREGUNTA * Preguntas);
void Cambia_Alfiler                    ();
void Cambia_Destino                    ();
void Cambia_Ejercicio                  ();
void Cambia_Numero_Versiones           ();
void Cambia_orden_opciones             (struct PREGUNTA * Preguntas);
void Cambia_Pregunta                   ();
void Cambio_Codigo                     ();
void Cambio_Examen_Descripcion         (GtkWidget *widget, gpointer user_data);
void Carga_Preguntas                   (struct EJERCICIO * Ejercicios, struct PREGUNTA * Preguntas, int orden);
void Carga_Profesor                    (char * profesor);
void Cambio_Profesor                   (GtkWidget *widget, gpointer user_data);
void Connect_widgets();
void Extrae_codigo                     (char * hilera, char * codigo);
void Fin_de_Programa                   (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                       (GtkWidget *widget, gpointer user_data);
void Graba_Examen                      ();
void Init_Fields                       ();
void Intercambia_Ejercicios            ();
void Interface_Coloring ();
void on_BN_save_clicked                (GtkWidget *widget, gpointer user_data);
void on_BN_swap_clicked                (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked            (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked                (GtkWidget *widget, gpointer user_data);
void on_CK_alfiler_global_toggled      (GtkWidget *widget, gpointer user_data);
void on_EN_examen_descripcion_activate (GtkWidget *widget, gpointer user_data);
void on_SC_ejercicios_value_changed    (GtkWidget *widget, gpointer user_data);
void on_SC_N_versiones_value_changed   (GtkWidget *widget, gpointer user_data);
void on_SC_posicion_value_changed      (GtkWidget *widget, gpointer user_data);
void on_SC_preguntas_value_changed     (GtkWidget *widget, gpointer user_data);
void on_WN_ex3040_destroy              (GtkWidget *widget, gpointer user_data) ;
void Orden_Aleatorio                   (struct PREGUNTA * preguntas, int Barajar_opciones);
void Orden_Dificultad_A                (struct PREGUNTA * preguntas, int Barajar_opciones);
void Orden_Dificultad_D                (struct PREGUNTA * preguntas, int Barajar_opciones);
void Orden_Manual                      (struct PREGUNTA * preguntas, int Barajar_opciones);
void Orden_Tema                        (struct PREGUNTA * preguntas, int Barajar_opciones);
void Orden_Tema_Subtema                (struct PREGUNTA * preguntas, int Barajar_opciones);
void Read_Only_Fields ();
/*----------------------------------------------------------------------------*/
/***************/
/* M A I N ( ) */
/***************/
int main(int argc, char *argv[]) 
{
  gettimeofday(&T, &Z);
  srand (T.tv_sec);

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX3040.glade", &error))
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
  char hilera[100];
  int i;
  
  window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex3040"));
  window2                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));

  EB_pre2ex = GTK_WIDGET (gtk_builder_get_object (builder, "EB_pre2ex"));
  FR_pre2ex = GTK_WIDGET (gtk_builder_get_object (builder, "FR_pre2ex"));

  SP_pre_examen          = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_pre_examen"));
  EN_descripcion         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));
  EN_esquema             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema"));
  EN_esquema_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema_descripcion"));
  EN_materia             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia"));
  EN_materia_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia_descripcion"));
  EN_N_preguntas         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_N_preguntas"));
  EN_N_ejercicios        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_N_ejercicios"));
  EN_N_temas             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_N_temas"));
  EN_examen              = GTK_WIDGET (gtk_builder_get_object (builder, "EN_examen"));
  EN_examen_descripcion  = GTK_WIDGET (gtk_builder_get_object (builder, "EN_examen_descripcion"));

  CB_profesor            = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder,  "CB_profesor"));

  SC_N_versiones         = GTK_WIDGET (gtk_builder_get_object (builder, "SC_N_versiones"));

  for (i=1; i <= MAXIMO_VERSIONES; i++)
      {
       sprintf (hilera, "EN_version%02d", i);
       EN_version [i-1] = GTK_WIDGET (gtk_builder_get_object (builder,  hilera));
       gtk_widget_set_can_focus(EN_version[i-1], FALSE); /* Campo READ-ONLY */

       sprintf (hilera, "CB_orden%02d", i);
       CB_orden [i-1] = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder,  hilera));

       sprintf (hilera, "CK_opciones%02d", i);
       CK_opciones [i-1] = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder,  hilera));
      }

  EB_orden_manual        = GTK_WIDGET (gtk_builder_get_object (builder, "EB_orden_manual"));

  FR_versiones           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_versiones"));
  FR_orden_manual        = GTK_WIDGET (gtk_builder_get_object (builder, "FR_orden_manual"));
  FR_pregunta_actual     = GTK_WIDGET (gtk_builder_get_object (builder, "FR_pregunta_actual"));
  FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));

  EN_ejercicio           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_ejercicio"));
  EN_secuencia           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_secuencia"));
  EN_tema                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_tema"));
  EN_subtema             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_subtema"));
  CK_alfiler_global      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_alfiler_global"));
  IM_alfiler_destino     = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_alfiler_destino"));

  SC_preguntas           = GTK_WIDGET (gtk_builder_get_object (builder, "SC_preguntas"));
  SC_ejercicios          = GTK_WIDGET (gtk_builder_get_object (builder, "SC_ejercicios"));
  SC_posicion            = GTK_WIDGET (gtk_builder_get_object (builder, "SC_posicion"));

  TV_pregunta            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_pregunta"));
  buffer_TV_pregunta     = gtk_text_view_get_buffer(TV_pregunta);

  BN_save        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
  BN_swap        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_swap"));
  BN_terminar    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_undo        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));

  EB_update      = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
  FR_update      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));
  BN_ok          = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_descripcion        , FALSE);
  gtk_widget_set_can_focus(EN_esquema            , FALSE);
  gtk_widget_set_can_focus(EN_esquema_descripcion, FALSE);
  gtk_widget_set_can_focus(EN_materia            , FALSE);
  gtk_widget_set_can_focus(EN_materia_descripcion, FALSE);
  gtk_widget_set_can_focus(EN_examen             , FALSE);

  gtk_widget_set_can_focus(EN_N_preguntas        , FALSE);
  gtk_widget_set_can_focus(EN_N_ejercicios       , FALSE);
  gtk_widget_set_can_focus(EN_N_temas            , FALSE);

  gtk_widget_set_can_focus(EN_ejercicio           , FALSE);
  gtk_widget_set_can_focus(EN_secuencia           , FALSE);
  gtk_widget_set_can_focus(EN_tema                , FALSE);
  gtk_widget_set_can_focus(EN_subtema             , FALSE);
  gtk_widget_set_can_focus(GTK_WIDGET(TV_pregunta), FALSE);
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,        GTK_STATE_NORMAL, &color);

  gdk_color_parse (SECONDARY_AREA, &color);
  gtk_widget_modify_bg(EB_pre2ex, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (SPECIAL_AREA_1, &color);
  gtk_widget_modify_bg(EB_orden_manual, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_botones,         GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_versiones,          GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_orden_manual,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_pregunta_actual, GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_pre_examen), GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_examen, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_examen_descripcion, GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,                  GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_swap, GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,                  GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_swap, GTK_STATE_ACTIVE, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,              GTK_STATE_NORMAL, &color);

  gdk_color_parse (FINISHED_WORK_FR, &color);
  gtk_widget_modify_bg(FR_update,              GTK_STATE_NORMAL, &color);
}

void Init_Fields()
{
   int  i, k;
   char hilera         [600];
   char text           [2];
   char codigo_version [5];
   char *simbolos = ".+-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz";
   int N, Last;
   PGresult *res;

   N= strlen(simbolos);

   /* Recupera el pre-examen con código más alto */
   res = PQEXEC(DATABASE, "SELECT codigo_pre_examen from EX_pre_examenes order by codigo_pre_examen DESC limit 1");
   Last = 0;
   if (PQntuples(res))
      {
       Last = atoi (PQgetvalue (res, 0, 0));
      }
   PQclear(res);

   if (Last > 1)
      {
       gtk_widget_set_sensitive(GTK_WIDGET(SP_pre_examen), 1);
       gtk_spin_button_set_range (SP_pre_examen, 1.0, (long double) Last);
      }
   else
      { /* Primer pre-examen que se registra */
       gtk_widget_set_sensitive(GTK_WIDGET(SP_pre_examen), 0);
       gtk_spin_button_set_range (SP_pre_examen, 0.0, (long double) Last);
      }
   gtk_spin_button_set_value (SP_pre_examen, Last);

   Numero_Preguntas = Numero_Ejercicios = 0;
   sprintf (hilera,"%11d", Numero_Preguntas);
   gtk_entry_set_text(GTK_ENTRY(EN_N_preguntas),  hilera);
   gtk_entry_set_text(GTK_ENTRY(EN_N_ejercicios), hilera);
   gtk_entry_set_text(GTK_ENTRY(EN_N_temas),      hilera);

   gtk_entry_set_text(GTK_ENTRY(EN_descripcion),         "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema),             "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia),             "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_examen),              "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_examen_descripcion),  "*** Descripción de Examen ***");

   gtk_combo_box_set_active (CB_profesor, -1);
   for (i=0; i < N_autores; i++) gtk_combo_box_remove_text (CB_profesor, 0);
   res = PQEXEC(DATABASE,"SELECT * from bd_personas order by codigo_persona"); 
   N_autores = PQntuples(res);
   for (i=0; i < N_autores; i++)
       {
	sprintf (hilera,"%s %s",PQgetvalue (res, i, 0),PQgetvalue (res, i, 1));
        gtk_combo_box_append_text(CB_profesor, hilera);
       }
   PQclear(res);

   res = PQEXEC(DATABASE, "SELECT profesor from EX_examenes order by codigo_examen DESC limit 1");
   if (PQntuples(res))
      {
       Carga_Profesor (PQgetvalue (res, 0, 0));
      }
   else
      {
       gtk_combo_box_set_active (CB_profesor, 0);
      }
   PQclear(res);

   gtk_range_set_range (GTK_RANGE(SC_preguntas),  (gdouble) 0.0, (gdouble) 1.0);
   gtk_range_set_value (GTK_RANGE(SC_preguntas),  (gdouble) 0.0);
   gtk_range_set_range (GTK_RANGE(SC_ejercicios), (gdouble) 0.0, (gdouble) 1.0);
   gtk_range_set_value (GTK_RANGE(SC_ejercicios), (gdouble) 0.0);
   gtk_range_set_range (GTK_RANGE(SC_posicion),   (gdouble) 0.0, (gdouble) 1.0);
   gtk_range_set_value (GTK_RANGE(SC_posicion),   (gdouble) 0.0);

   gtk_image_set_from_file (IM_alfiler_destino, ".imagenes/empty.png");
   gtk_toggle_button_set_active(CK_alfiler_global, FALSE);

   gtk_entry_set_text(GTK_ENTRY(EN_ejercicio),   "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_secuencia),   "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_tema),        "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_subtema),     "\0");

   gtk_text_buffer_set_text(buffer_TV_pregunta, "\0", -1);

   for (i=0;i<MAXIMO_VERSIONES;i++)
       {
        k = (int) (N * (rand() / (RAND_MAX + 1.0))); codigo_version[0] = simbolos[k];
        k = (int) (N * (rand() / (RAND_MAX + 1.0))); codigo_version[1] = simbolos[k];
        k = (int) (N * (rand() / (RAND_MAX + 1.0))); codigo_version[2] = simbolos[k];
        k = (int) (N * (rand() / (RAND_MAX + 1.0))); codigo_version[3] = simbolos[k];
	codigo_version [4] = '\0';

        gtk_entry_set_text(GTK_ENTRY(EN_version [i]), codigo_version);
        gtk_combo_box_set_active    (CB_orden   [i], 0);
        gtk_toggle_button_set_active(CK_opciones[i], TRUE);
     
        gtk_widget_set_visible (EN_version             [i] , FALSE);
        gtk_widget_set_visible (GTK_WIDGET(CB_orden    [i]), FALSE);
        gtk_widget_set_visible (GTK_WIDGET(CK_opciones [i]), FALSE);
       }

   N_versiones = 0;
   gtk_range_set_value (GTK_RANGE(SC_N_versiones), (gdouble) 0.0);

   gtk_widget_set_sensitive(GTK_WIDGET(SP_pre_examen), 1);
   gtk_widget_set_sensitive(EN_descripcion        , 0);
   gtk_widget_set_sensitive(EN_esquema            , 0);
   gtk_widget_set_sensitive(EN_esquema_descripcion, 0);
   gtk_widget_set_sensitive(EN_materia            , 0);
   gtk_widget_set_sensitive(EN_materia_descripcion, 0);
   gtk_widget_set_sensitive(EN_N_preguntas        , 0);
   gtk_widget_set_sensitive(EN_N_ejercicios       , 0);
   gtk_widget_set_sensitive(EN_N_temas            , 0);
   gtk_widget_set_sensitive(FR_versiones          , 0);
   gtk_widget_set_sensitive(FR_orden_manual       , 0);
   gtk_widget_set_sensitive(EN_examen             , 0);
   gtk_widget_set_sensitive(EN_examen_descripcion , 0);
   gtk_widget_set_sensitive(SC_N_versiones        , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CB_profesor), 0);

   gtk_widget_set_sensitive(BN_save,    0);
   gtk_widget_set_sensitive(BN_swap,    0);

   if (Last > 1)
      gtk_widget_grab_focus (GTK_WIDGET(SP_pre_examen));
   else
      Cambio_Codigo ();
}

void Carga_Profesor (char * profesor)
{
  int i;
  PGresult *res_autores;
  char PG_command[1000];

  sprintf (PG_command,"SELECT * from bd_personas where codigo_persona <= '%s' order by codigo_persona", profesor); 
  res_autores = PQEXEC(DATABASE, PG_command);
  i = PQntuples(res_autores);
  PQclear(res_autores);
  gtk_combo_box_set_active (CB_profesor, i-1);
}

void Cambio_Codigo()
{
  long int k;
  int i;
  char codigo[10];
  char hilera [50];
  char Descripcion [100] = "Preexamen no está registrado o está vacío";
  char PG_command [2000];
  char Codigo_Examen[6];
  PGresult *res, *res_aux;

  k = (long int) gtk_spin_button_get_value_as_int (SP_pre_examen);
  sprintf (codigo, "%05ld", k);

  sprintf (PG_command,"SELECT * from EX_pre_examenes, EX_esquemas, BD_materias where codigo_pre_examen = '%s' and esquema = codigo_esquema and materia = codigo_materia and codigo_tema = '          ' and codigo_subtema = '          '", codigo);
  res = PQEXEC(DATABASE, PG_command);

  sprintf (PG_command,"SELECT pre_examen from EX_pre_examenes_lineas where pre_examen = '%s'", codigo);
  res_aux = PQEXEC(DATABASE, PG_command);
  Numero_Preguntas = PQntuples(res_aux);
  PQclear(res_aux);

  if (PQntuples(res) && Numero_Preguntas)
     { /* Registro ya existe... */
      res_aux = PQEXEC(DATABASE, "BEGIN"); PQclear(res_aux);
      res_aux = PQEXEC(DATABASE, "DECLARE ultimo_examen CURSOR FOR select codigo_examen from EX_examenes order by codigo_examen DESC"); PQclear(res_aux);
      res_aux = PQEXEC(DATABASE, "FETCH 1 in ultimo_examen");
      if (PQntuples(res_aux))
         {
          strcpy (Codigo_Examen,PQgetvalue (res_aux, 0, 0));
          i = atoi(Codigo_Examen);
          i++;
          sprintf (Codigo_Examen,"%05d",i);
         }
      else
         sprintf (Codigo_Examen,"00001");
      PQclear(res_aux);
      res_aux = PQEXEC(DATABASE, "CLOSE ultimo_examen");  PQclear(res_aux);
      /* end the transaction */
      res_aux = PQEXEC(DATABASE, "END"); PQclear(res_aux);

      gtk_entry_set_text(GTK_ENTRY(EN_examen), Codigo_Examen);

      gtk_widget_set_sensitive(EN_descripcion        , 1);
      gtk_widget_set_sensitive(EN_esquema            , 1);
      gtk_widget_set_sensitive(EN_esquema_descripcion, 1);
      gtk_widget_set_sensitive(EN_materia            , 1);
      gtk_widget_set_sensitive(EN_materia_descripcion, 1);
      gtk_widget_set_sensitive(EN_ejercicio          , 1);
      gtk_widget_set_sensitive(EN_N_preguntas        , 1);
      gtk_widget_set_sensitive(EN_N_ejercicios       , 1);
      gtk_widget_set_sensitive(EN_N_temas            , 1);
      gtk_widget_set_sensitive(EN_examen             , 1);
      gtk_widget_set_sensitive(EN_examen_descripcion , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(CB_profesor), 1);

      gtk_entry_set_text(GTK_ENTRY(EN_esquema), PQgetvalue (res, 0, 1));
      strcpy (Descripcion,                      PQgetvalue (res, 0, 2));
      prediccion_media                   = atof(PQgetvalue (res, 0, 3));
      prediccion_desviacion              = atof(PQgetvalue (res, 0, 4));
      Usos                               = atoi(PQgetvalue (res, 0, 5));
      prediccion_alfa                    = atof(PQgetvalue (res, 0, 7));
      prediccion_Rpb                     = atof(PQgetvalue (res, 0, 8));

      gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion), PQgetvalue (res, 0, 14));
      gtk_entry_set_text(GTK_ENTRY(EN_materia),             PQgetvalue (res, 0, 10));
      gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), PQgetvalue (res, 0, 18));

      sprintf (hilera,"%11d", Numero_Preguntas);
      gtk_entry_set_text(GTK_ENTRY(EN_N_preguntas), hilera);
      sprintf (PG_command,"SELECT DISTINCT ejercicio from EX_pre_examenes_lineas where pre_examen = '%s' order by ejercicio", codigo);
      res_aux = PQEXEC(DATABASE, PG_command);
      Numero_Ejercicios = PQntuples(res_aux);
      sprintf (hilera,"%11d", Numero_Ejercicios);
      gtk_entry_set_text(GTK_ENTRY(EN_N_ejercicios), hilera);
      for (i=0; i < Numero_Ejercicios; i++)
	  {
	   strcpy (Ejercicios_Base [i].ejercicio,PQgetvalue (res_aux, i, 0));
	   Ejercicios_Base[i].alfiler = 0;
	  }
      PQclear(res_aux);

      sprintf (PG_command,"SELECT DISTINCT tema from EX_pre_examenes_lineas, BD_texto_preguntas, BD_ejercicios where pre_examen = '%s' and codigo_pregunta = codigo_unico_pregunta and codigo_consecutivo_ejercicio = texto_ejercicio and materia = '%s'", codigo, PQgetvalue (res, 0, 10));
      res_aux = PQEXEC(DATABASE, PG_command);
      Numero_Temas = PQntuples(res_aux);
      sprintf (hilera,"%11d", Numero_Temas);
      gtk_entry_set_text(GTK_ENTRY(EN_N_temas), hilera);

      Carga_Preguntas (Ejercicios_Base, Preguntas_Base, POR_SECUENCIA);

      gtk_range_set_range (GTK_RANGE(SC_preguntas),  (gdouble) 1.0, (gdouble) Numero_Preguntas);
      gtk_range_set_range (GTK_RANGE(SC_ejercicios), (gdouble) 1.0, (gdouble) Numero_Ejercicios);
      gtk_range_set_range (GTK_RANGE(SC_posicion),   (gdouble) 1.0, (gdouble) Numero_Ejercicios);
      gtk_range_set_value (GTK_RANGE(SC_preguntas),  (gdouble) 1.0);

      gtk_widget_set_sensitive(FR_orden_manual, 1);
      gtk_widget_set_sensitive(FR_versiones,    1);
      gtk_widget_set_sensitive(SC_N_versiones,  1);

      N_versiones = 1;
      gtk_range_set_value (GTK_RANGE(SC_N_versiones), (gdouble) 1.0);
      Cambia_Numero_Versiones ();

      gtk_widget_set_sensitive(BN_save, 1);

      gtk_widget_set_sensitive(GTK_WIDGET(SP_pre_examen), 0);
      gtk_widget_grab_focus   (EN_examen_descripcion);
     }
  else
     {
      gtk_widget_grab_focus   (GTK_WIDGET(SP_pre_examen));
     }

  PQclear(res);

  gtk_entry_set_text(GTK_ENTRY(EN_descripcion), Descripcion);
}

void Carga_Preguntas (struct EJERCICIO * Ejercicios, struct PREGUNTA * Preguntas, int orden)
{
  long int k;
  int i,j;
  char codigo[10];
  gchar * materia;
  char PG_command [2000];
  long double dificultad_ejercicio;

  k = (long int) gtk_spin_button_get_value_as_int (SP_pre_examen);
  sprintf (codigo, "%05ld", k);

  materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);

  Numero_Preguntas = 0;

  for (i=0; i < Numero_Ejercicios; i++)
      {
       if (orden == POR_SECUENCIA)
	  sprintf (PG_command,"SELECT ejercicio, secuencia, codigo_pregunta, texto_pregunta, alfileres_opciones, respuesta, usa_header, N_estudiantes, dificultad, varianza_dificultad, cronbach_alpha_media_con, point_biserial, tema, subtema from EX_pre_examenes_lineas, BD_texto_preguntas, BD_estadisticas_preguntas, BD_texto_ejercicios, BD_ejercicios where pre_examen = '%s' and codigo_pregunta = codigo_unico_pregunta and ejercicio = '%s' and codigo_consecutivo_ejercicio = consecutivo_texto and pregunta = codigo_unico_pregunta and texto_ejercicio = consecutivo_texto and materia = '%s' order by secuencia", codigo, Ejercicios[i].ejercicio, materia);
       else
	  if (orden == POR_DIFICULTAD_ASC)
	     sprintf (PG_command,"SELECT ejercicio, secuencia, codigo_pregunta, texto_pregunta, alfileres_opciones, respuesta, usa_header, N_estudiantes, dificultad, varianza_dificultad, cronbach_alpha_media_con, point_biserial, tema, subtema from EX_pre_examenes_lineas, BD_texto_preguntas, BD_estadisticas_preguntas, BD_texto_ejercicios, BD_ejercicios where pre_examen = '%s' and codigo_pregunta = codigo_unico_pregunta and ejercicio = '%s' and codigo_consecutivo_ejercicio = consecutivo_texto and pregunta = codigo_unico_pregunta and texto_ejercicio = consecutivo_texto and materia = '%s' order by dificultad DESC", codigo, Ejercicios[i].ejercicio, materia);
	  else
	     if (orden == POR_DIFICULTAD_DES)
	        sprintf (PG_command,"SELECT ejercicio, secuencia, codigo_pregunta, texto_pregunta, alfileres_opciones, respuesta, usa_header, N_estudiantes, dificultad, varianza_dificultad, cronbach_alpha_media_con, point_biserial, tema, subtema from EX_pre_examenes_lineas, BD_texto_preguntas, BD_estadisticas_preguntas, BD_texto_ejercicios, BD_ejercicios where pre_examen = '%s' and codigo_pregunta = codigo_unico_pregunta and ejercicio = '%s' and codigo_consecutivo_ejercicio = consecutivo_texto and pregunta = codigo_unico_pregunta and texto_ejercicio = consecutivo_texto and materia = '%s' order by dificultad", codigo, Ejercicios[i].ejercicio, materia);

       res_aux = PQEXEC(DATABASE, PG_command);

       k = PQntuples(res_aux);

       dificultad_ejercicio = 0.0;

       Ejercicios[i].primer_pregunta = Numero_Preguntas+1;
       for (j=0;j<k;j++)
 	   {
	    strcpy (Preguntas[Numero_Preguntas].ejercicio         , PQgetvalue (res_aux, j, 0));
	    Preguntas[Numero_Preguntas].secuencia            = atoi(PQgetvalue (res_aux, j, 1));
	    strcpy (Preguntas[Numero_Preguntas].codigo            , PQgetvalue (res_aux, j, 2));
	    strcpy (Preguntas[Numero_Preguntas].texto_pregunta    , PQgetvalue (res_aux, j, 3));
	    strcpy (Preguntas[Numero_Preguntas].alfileres_opciones, PQgetvalue (res_aux, j, 4));
	            Preguntas[Numero_Preguntas].correcta =         *PQgetvalue (res_aux, j, 5) - 'A';
	            Preguntas[Numero_Preguntas].Posicion_Ejercicio = i+1;
	            Preguntas[Numero_Preguntas].orden [0] = 0;
	            Preguntas[Numero_Preguntas].orden [1] = 1;
	            Preguntas[Numero_Preguntas].orden [2] = 2;
	            Preguntas[Numero_Preguntas].orden [3] = 3;
	            Preguntas[Numero_Preguntas].orden [4] = 4;

	    Preguntas[Numero_Preguntas].Header                 = *PQgetvalue (res_aux, j, 6) == 't';
	    Preguntas[Numero_Preguntas].N_estudiantes      = atoi(PQgetvalue (res_aux, j, 7));
	    Preguntas[Numero_Preguntas].media              = atof(PQgetvalue (res_aux, j, 8));
	    Preguntas[Numero_Preguntas].desviacion    = sqrt(atof(PQgetvalue (res_aux, j, 9)));
	    Preguntas[Numero_Preguntas].alfa               = atof(PQgetvalue (res_aux, j, 10));
	    Preguntas[Numero_Preguntas].Rpb                = atof(PQgetvalue (res_aux, j, 11));
	    strcpy (Preguntas[Numero_Preguntas].tema            , PQgetvalue (res_aux, j, 12));
	    strcpy (Preguntas[Numero_Preguntas].subtema         , PQgetvalue (res_aux, j, 13));

	    dificultad_ejercicio += Preguntas[Numero_Preguntas].media;

            Numero_Preguntas++;
	   }

       PQclear(res_aux);
       
       Ejercicios[i].dificultad = dificultad_ejercicio / k;
       strcpy (Ejercicios[i].tema   , Preguntas[Ejercicios[i].primer_pregunta-1]. tema);
       strcpy (Ejercicios[i].subtema, Preguntas[Ejercicios[i].primer_pregunta-1]. subtema);


       sprintf (PG_command,"SELECT orden from BD_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", materia, Ejercicios[i].tema);
       res_aux = PQEXEC(DATABASE, PG_command);
       Ejercicios[i].orden_tema = atoi(PQgetvalue (res_aux, 0, 0));
       PQclear(res_aux);

       sprintf (PG_command,"SELECT orden from BD_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", materia, Ejercicios[i].tema, Ejercicios[i].subtema);
       res_aux = PQEXEC(DATABASE, PG_command);
       Ejercicios[i].orden_subtema = atoi(PQgetvalue (res_aux, 0, 0));
       PQclear(res_aux);
      }

  g_free (materia);
}

void Graba_Examen ()
{
   long int k;
   int i,j, modo;
   struct PREGUNTA   Preguntas  [MAX_PREGUNTAS];
   /*
   char respuestas [MAX_PREGUNTAS+1];
   */
   gchar * Codigo_Examen;
   char  Pre_Examen[10];
   gchar * Materia;
   gchar * Descripcion;
   gchar * Version;
   gchar * profesor;
   char  *instrucciones_corregidas, *juramento_corregido;
   int     year, month, day;

   char PG_command [5000];
   time_t curtime;
   struct tm *loctime;
   char hilera[20];
   char Codigo_profesor [10];

   Codigo_Examen = gtk_editable_get_chars(GTK_EDITABLE(EN_examen)            , 0, -1);
   k = (long int) gtk_spin_button_get_value_as_int (SP_pre_examen);
   sprintf (Pre_Examen, "%05ld", k);

   Materia       = gtk_editable_get_chars(GTK_EDITABLE(EN_materia)           , 0, -1);
   Descripcion   = gtk_editable_get_chars(GTK_EDITABLE(EN_examen_descripcion), 0, -1);

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

   profesor = gtk_combo_box_get_active_text(CB_profesor);
   Extrae_codigo (profesor, Codigo_profesor);

   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);
   instrucciones_corregidas = (char *) malloc (sizeof(char) * strlen(parametros.Instrucciones)*2);
   juramento_corregido = (char *) malloc (sizeof(char) * strlen(parametros.Juramento)*2);
   hilera_POSTGRES (parametros.Instrucciones, instrucciones_corregidas);
   hilera_POSTGRES (parametros.Juramento,     juramento_corregido);

   sprintf (PG_command,"INSERT into EX_examenes values ('%.5s','%s','%.5s','%.10s','%s','%s','%s','%s',%d,%d,%d,%d,%d,%Lf,%Lf,%Lf,%Lf,0.0,0.0,0.0,0.0,TRUE,E'%s',TRUE,E'%s',FALSE)", 
	    Codigo_Examen, 
	    Descripcion,
            Pre_Examen,
	    Codigo_profesor,
            Materia,
	    parametros.Institucion, 
	    parametros.Escuela, 
	    parametros.Programa, 
            year, month, day,
            N_versiones,
	    Usos, 
            prediccion_media, prediccion_desviacion, prediccion_alfa, prediccion_Rpb,
	    instrucciones_corregidas, 
	    juramento_corregido);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   free(instrucciones_corregidas);
   free(juramento_corregido);

   for (i=0; i < N_versiones;i++)
       {

        Version = gtk_editable_get_chars(GTK_EDITABLE(EN_version [i]), 0, -1);
	modo = gtk_combo_box_get_active(CB_orden[i]);

	switch (modo)
	       {
	        case 0: /* Orden Aleatorio */
		        Orden_Aleatorio (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 1: /* Por dificultad ascendente de los ejercicios */
		        Orden_Dificultad_A (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 2: /* Por dificultad descendente de los ejercicios */
		        Orden_Dificultad_D (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 3: /* POR TEMA. Orden Aleatorio interno*/
		        Orden_Tema (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 4: /* Orden Aleatorio */
		        Orden_Aleatorio (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 5: /* Orden Aleatorio */
		        Orden_Aleatorio (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 6: /* Por TEMA y SUBTEMA. Orden Aleatorio interno */
		        Orden_Tema_Subtema (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 7: /* Orden Aleatorio */
		        Orden_Aleatorio (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 8: /* Orden Aleatorio */
		        Orden_Aleatorio (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 9: /* Orden Aleatorio */
		        Orden_Aleatorio (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 10: /* Orden Aleatorio */
		        Orden_Aleatorio (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 11: /* Orden Aleatorio */
		        Orden_Aleatorio (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 12: /* Orden Aleatorio */
		        Orden_Aleatorio (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 13: /* Orden Aleatorio */
		        Orden_Aleatorio (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;

	        case 14: /* Copiar el Orden Manual indicado por el usuario */
		        Orden_Manual (Preguntas, gtk_toggle_button_get_active (CK_opciones[i]));
		        break;
	       };

        sprintf (PG_command,"INSERT into EX_versiones values ('%.5s','%.4s',%d,%s)", 
	         Codigo_Examen,
                 Version,
		 modo,
                 gtk_toggle_button_get_active (CK_opciones[i])?"TRUE":"FALSE");

        res = PQEXEC(DATABASE, PG_command); PQclear(res);

        for (j=0; j < Numero_Preguntas;j++)
	    {
             sprintf (PG_command,"INSERT into EX_examenes_Preguntas values ('%.5s','%.4s',%d,'%.6s',%d,'%.6s',%d,%d,%d,%d,%d)", 
		      Codigo_Examen, Version, j+1, 
                      Preguntas[j].ejercicio,
                      Preguntas[j].secuencia,
                      Preguntas[j].codigo,
                      Preguntas[j].orden[0],Preguntas[j].orden[1],Preguntas[j].orden[2],Preguntas[j].orden[3],Preguntas[j].orden[4]);
             res = PQEXEC(DATABASE, PG_command); PQclear(res);
            }

        g_free (Version);
       }

  res = PQEXEC(DATABASE, "END"); PQclear(res);

  g_free (Codigo_Examen);
  g_free (Materia);
  g_free (Descripcion);
  g_free (profesor);
  
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

void Orden_Aleatorio (struct PREGUNTA * preguntas, int Barajar_opciones)
{
  struct EJERCICIO  Ejercicios [MAX_PREGUNTAS], ejer_aux;
  int i;

  Baraja_ejercicios (Ejercicios);
  Carga_Preguntas   (Ejercicios, preguntas, POR_SECUENCIA);
  Baraja_preguntas  (Ejercicios, preguntas);

  if (Barajar_opciones) Cambia_orden_opciones (preguntas);
}

void Orden_Dificultad_A (struct PREGUNTA * preguntas, int Barajar_opciones)
{
  struct EJERCICIO Ejercicios [MAX_PREGUNTAS], ejer_aux;
  struct EJERCICIO temporal;
  int i,j;

  Baraja_ejercicios (Ejercicios); /* Así los empates por dificultad quedan aleatoriamente */

  for (i=Numero_Ejercicios; i >= 0; i--)
      for (j=0; j < (i-1); j++)
	  {
	   if (Ejercicios[j].dificultad < Ejercicios[j+1].dificultad) /* Entre más bajo más difícil */
	      {
	       temporal         = Ejercicios[j+1];
               Ejercicios [j+1] = Ejercicios[j];
               Ejercicios [j]   = temporal;
	      }
	  }

  Carga_Preguntas (Ejercicios, preguntas, POR_DIFICULTAD_ASC);

  if (Barajar_opciones) Cambia_orden_opciones (preguntas);
}

void Orden_Dificultad_D (struct PREGUNTA * preguntas, int Barajar_opciones)
{
  struct EJERCICIO Ejercicios [MAX_PREGUNTAS], ejer_aux;
  struct EJERCICIO temporal;
  int i,j;

  Baraja_ejercicios (Ejercicios); /* Así los empates por dificultad quedan aleatoriamente */

  for (i=Numero_Ejercicios; i >= 0; i--)
      for (j=0; j < (i-1); j++)
	  {
	   if (Ejercicios[j].dificultad > Ejercicios[j+1].dificultad) /* Entre más alto más fácil */
	      {
	       temporal         = Ejercicios[j+1];
               Ejercicios [j+1] = Ejercicios[j];
               Ejercicios [j]   = temporal;
	      }
	  }

  Carga_Preguntas (Ejercicios, preguntas, POR_DIFICULTAD_DES);

  if (Barajar_opciones) Cambia_orden_opciones (preguntas);
}

void Orden_Tema (struct PREGUNTA * preguntas, int Barajar_opciones)
{
  struct EJERCICIO Ejercicios [MAX_PREGUNTAS], ejer_aux;
  struct EJERCICIO temporal;
  int i, j;

  Baraja_ejercicios (Ejercicios); /* Así los ejercicios del mismo TEMA se ordenan aleatoriamente */
  for (i=Numero_Ejercicios; i >= 0; i--)
      for (j=0; j < (i-1); j++)
	  {
	   if (Ejercicios[j].orden_tema > Ejercicios[j+1].orden_tema)
	      {
	       temporal         = Ejercicios[j+1];
               Ejercicios [j+1] = Ejercicios[j];
               Ejercicios [j]   = temporal;
	      }
	  }

  Carga_Preguntas   (Ejercicios, preguntas, POR_SECUENCIA);
  Baraja_preguntas  (Ejercicios, preguntas);

  if (Barajar_opciones) Cambia_orden_opciones (preguntas);
}

void Orden_Tema_Subtema (struct PREGUNTA * preguntas, int Barajar_opciones)
{
  struct EJERCICIO Ejercicios [MAX_PREGUNTAS], ejer_aux;
  struct EJERCICIO temporal;
  int i, j;

  Baraja_ejercicios (Ejercicios); /* Así los ejercicios del mismo TEMA-SUBTEMA se ordenan aleatoriamente */
  for (i=Numero_Ejercicios; i >= 0; i--)
      for (j=0; j < (i-1); j++)
	  {
	    if ((Ejercicios[j].orden_tema > Ejercicios[j+1].orden_tema) ||
		((Ejercicios[j].orden_tema == Ejercicios[j+1].orden_tema) && (Ejercicios[j].orden_subtema > Ejercicios[j+1].orden_subtema)))
	      {
	       temporal         = Ejercicios[j+1];
               Ejercicios [j+1] = Ejercicios[j];
               Ejercicios [j]   = temporal;
	      }
	  }

  Carga_Preguntas   (Ejercicios, preguntas, POR_SECUENCIA);
  Baraja_preguntas  (Ejercicios, preguntas);

  if (Barajar_opciones) Cambia_orden_opciones (preguntas);
}

void Orden_Manual (struct PREGUNTA * preguntas, int Barajar_opciones)
{
  int i;

  for (i=0; i < Numero_Preguntas; i++)
      {
       preguntas [i] = Preguntas_Base [i];
      }

  if (Barajar_opciones) Cambia_orden_opciones (preguntas);
}

void Baraja_ejercicios (struct EJERCICIO * Ejercicios)
{
  struct EJERCICIO  ejer_aux;
  int i, j, k;

  for (i=0; i < Numero_Ejercicios; i++) Ejercicios [i] = Ejercicios_Base [i];

  k = Numero_Ejercicios-1;
  for (i=0; i<Numero_Ejercicios; i++)
      {
       if (!Ejercicios [k].alfiler)
	  {
	   j = (int) (Numero_Ejercicios * (rand() / (RAND_MAX + 1.0)));
	   j = (j == Numero_Ejercicios)? Numero_Ejercicios-1:j;
	   while (Ejercicios[j].alfiler)
	         {
  		  j = (int) (Numero_Ejercicios * (rand() / (RAND_MAX + 1.0)));
	          j = (j == Numero_Ejercicios)? Numero_Ejercicios-1:j;
		 }
  	   ejer_aux = Ejercicios [k];
	   Ejercicios [k] = Ejercicios [j];
	   Ejercicios [j] = ejer_aux;
	  }

       k--;
      }
}

void Baraja_preguntas (struct EJERCICIO * Ejercicios, struct PREGUNTA * Preguntas)
{
  int i, j, k, m;
  struct PREGUNTA preg_aux;

  for (i=0; i<Numero_Ejercicios; i++)
      {
       k = (i < (Numero_Ejercicios-1))? Ejercicios[i+1].primer_pregunta - Ejercicios[i].primer_pregunta : Numero_Preguntas - Ejercicios[i].primer_pregunta + 1;
       if (k > 1)
          {
	   for (j=0;j<k;j++)
	       {
	        if (!Preguntas [j + Ejercicios[i].primer_pregunta-1].alfiler)
		   {
 	            m = (int) (k * (rand() / (RAND_MAX + 1.0)));
	            m = (m == k)? k-1:m;
	            while (Preguntas [m + Ejercicios[i].primer_pregunta-1].alfiler)
		          {
	                   m = (int) (k * (rand() / (RAND_MAX + 1.0)));
	                   m = (m == k)? k-1:m;
		          }
          	    preg_aux = Preguntas [m + Ejercicios[i].primer_pregunta-1];
	            Preguntas [m + Ejercicios[i].primer_pregunta-1] = Preguntas [j + Ejercicios[i].primer_pregunta-1];
	            Preguntas [j + Ejercicios[i].primer_pregunta-1] = preg_aux;
	           }
	       }
	  }
      }
}

void Cambia_orden_opciones (struct PREGUNTA * Preguntas)
{
  int i, j, k, m, temp;
  int boletos, acumulados, N_opciones;
  int reservados[5];
  int correcta_previa;
  char hilera[20];
  char opciones_barajadas[5];

  for (i=0;i<5;i++) reservados [i] = 0;
  for (i=0; i < Numero_Preguntas; i++)
      { /* Cuántas correctas tienen alfiler? */
       strcpy (hilera, Preguntas[i].alfileres_opciones);
       correcta_previa = Preguntas [i].correcta;
       if (hilera[correcta_previa] == '1') reservados[correcta_previa]++;
      }

  N_opciones = max(5, Numero_Preguntas);
  N_opciones = N_opciones + (N_opciones * 0.1);
  N_opciones = round(N_opciones/5.0);

  boletos = 0;
  for (i=0;i<5;i++)
      {
       reservados[i] = max (0, N_opciones - reservados[i]);
       boletos += reservados[i];
      }
	
  for (i=0; i < Numero_Preguntas; i++)
      {
       for (j=0; j < 5; j++) opciones_barajadas [j] = j;

       strcpy (hilera, Preguntas[i].alfileres_opciones);
       correcta_previa = Preguntas [i].correcta;

       if (hilera[correcta_previa] == '0')
          {
           m = (int) (boletos * (rand() / (RAND_MAX + 1.0)))+1;
           k = 0;
	   acumulados = reservados[0];
	   while (acumulados < m) acumulados += reservados[++k];

	   while (hilera[k] == '1')
	         {
      	          m = (int) (boletos * (rand() / (RAND_MAX + 1.0)))+1;
	          k = 0;
	          acumulados = reservados[0];
	          while (acumulados < m) acumulados += reservados[++k];
	         }

	   opciones_barajadas [k] = correcta_previa;
	   opciones_barajadas [correcta_previa] = k;
	   correcta_previa = k;
	   hilera [correcta_previa] = '1';
	  }

       k = 4;
       for (j=0; j<5; j++)
    	   {
	    if (hilera[k] == '0')
	       {
	        m = (int) (5 * (rand() / (RAND_MAX + 1.0)));
	        while (hilera[m] == '1') m = (int) (5 * (rand() / (RAND_MAX + 1.0)));

	        temp                   = opciones_barajadas [k];
	        opciones_barajadas [k] = opciones_barajadas [m];
	        opciones_barajadas [m] = temp;
	       }
    	        
            k--;
	   }

       for (j=0;j<5;j++) Preguntas[i].orden[j] = opciones_barajadas[j];

       Preguntas [i].correcta = correcta_previa;
      }
}

void Cambia_Numero_Versiones ()
{
  int i, j, k;

  if (N_versiones)
     {
      k = (int) gtk_range_get_value (GTK_RANGE(SC_N_versiones));

      for (i=0; i<k; i++)
          {
           gtk_widget_set_visible (EN_version             [i] , TRUE);
           gtk_widget_set_visible (GTK_WIDGET(CB_orden    [i]), TRUE);
           gtk_widget_set_visible (GTK_WIDGET(CK_opciones [i]), TRUE);
          }

      for (i=k; i < MAXIMO_VERSIONES; i++)
          {
           gtk_combo_box_set_active    (CB_orden   [i], 0);
           gtk_toggle_button_set_active(CK_opciones[i], TRUE);

           gtk_widget_set_visible (EN_version             [i] , FALSE);
           gtk_widget_set_visible (GTK_WIDGET(CB_orden    [i]), FALSE);
           gtk_widget_set_visible (GTK_WIDGET(CK_opciones [i]), FALSE);
          }

      N_versiones = k;
     }
}

void Cambia_Pregunta ()
{
  int k, j;
  char hilera [20];

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k)
     {
      if (Preguntas_Base[k-1].Posicion_Ejercicio != j) gtk_range_set_value (GTK_RANGE(SC_ejercicios), (gdouble) Preguntas_Base[k-1].Posicion_Ejercicio);
      j = (int) gtk_range_get_value (GTK_RANGE(SC_ejercicios));

      gtk_entry_set_text(GTK_ENTRY(EN_ejercicio), Preguntas_Base[k-1].ejercicio);
      sprintf (hilera,"%d", Preguntas_Base[k-1].secuencia);
      gtk_entry_set_text(GTK_ENTRY(EN_secuencia), hilera);
      gtk_entry_set_text(GTK_ENTRY(EN_tema), Preguntas_Base[k-1].tema);
      gtk_entry_set_text(GTK_ENTRY(EN_subtema), Preguntas_Base[k-1].subtema);

      gtk_text_buffer_set_text(buffer_TV_pregunta, Preguntas_Base[k-1].texto_pregunta, -1);

#if 0
      if (!Ejercicios_Base[j-1].alfiler)
	 {
	   /*
          gtk_toggle_button_set_active(CK_alfiler_global, FALSE);
	  Ejercicios_Base[j-1].alfiler = 0;
	   */
	 }
      else
	 {
	   /*
          gtk_toggle_button_set_active(CK_alfiler_global, TRUE);
	  Ejercicios_Base[j-1].alfiler = 1;
	   */
	 }
#endif
     }
}

void Cambia_Ejercicio ()
{
  int k, j;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_ejercicios));
  j = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k)
     {
      if (Preguntas_Base[j-1].Posicion_Ejercicio != k) gtk_range_set_value (GTK_RANGE(SC_preguntas), (gdouble) Ejercicios_Base[k-1].primer_pregunta);
      if (!Ejercicios_Base[k-1].alfiler)
	 {
          gtk_toggle_button_set_active(CK_alfiler_global, FALSE);
	 }
      else
	 {
          gtk_toggle_button_set_active(CK_alfiler_global, TRUE);
	 }

      gtk_range_set_value (GTK_RANGE(SC_posicion), (gdouble) k);
      gtk_widget_set_sensitive(BN_swap,   0);
     }
}

void Cambia_Destino ()
{
  int k, j;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_posicion));
  j = (int) gtk_range_get_value (GTK_RANGE(SC_ejercicios));

  if (Ejercicios_Base[k-1].alfiler)
     gtk_image_set_from_file (IM_alfiler_destino, ".imagenes/pin2.png");
  else
     gtk_image_set_from_file (IM_alfiler_destino, ".imagenes/empty.png");

  if ((k == j) || Ejercicios_Base[j-1].alfiler || Ejercicios_Base[k-1].alfiler)
     {
      gtk_widget_set_sensitive(BN_swap,   0);
     }
  else
     {
      gtk_widget_set_sensitive(BN_swap,   1);
     }
}

void Cambia_Alfiler ()
{
  int k, j;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_ejercicios));
  j = (int) gtk_range_get_value (GTK_RANGE(SC_posicion));

  if (k)
     {
      if (gtk_toggle_button_get_active(CK_alfiler_global))
	 {
   	  Ejercicios_Base[k-1].alfiler = 1;
          gtk_image_set_from_file (IM_alfiler_destino, ".imagenes/pin2.png");
	 }
      else
	 {
  	  Ejercicios_Base[k-1].alfiler = 0;
          gtk_image_set_from_file (IM_alfiler_destino, ".imagenes/empty.png");
	 }

      if (!Ejercicios_Base[k-1].alfiler)
	 {
          if (!Ejercicios_Base[j-1].alfiler) gtk_widget_set_sensitive(BN_swap, 1);
	 }
      else
	 {
          gtk_range_set_value (GTK_RANGE(SC_posicion), (gdouble) k);
	 }
     }
}

void Intercambia_Ejercicios ()
{
  int j, k;
  struct EJERCICIO aux;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_ejercicios));
  j = (int) gtk_range_get_value (GTK_RANGE(SC_posicion));

  aux                   = Ejercicios_Base [k-1];
  Ejercicios_Base [k-1] = Ejercicios_Base [j-1]; 
  Ejercicios_Base [j-1] = aux;

  Carga_Preguntas (Ejercicios_Base, Preguntas_Base, POR_SECUENCIA);

  Ejercicios_Base[j-1].alfiler = 1;
  gtk_range_set_value (GTK_RANGE(SC_preguntas), (gdouble) Ejercicios_Base[j-1].primer_pregunta);
}

void Cambio_Profesor(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_examen_descripcion);
}

void Cambio_Examen_Descripcion(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_descripcion);
}

void Extrae_codigo (char * hilera, char * codigo)
{
  int i;

  i = 0;

  while (hilera[i] != ' ') codigo [i] = hilera[i++];

  codigo[i] = '\0';
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
void on_WN_ex3040_destroy (GtkWidget *widget, gpointer user_data) 
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

void on_BN_swap_clicked(GtkWidget *widget, gpointer user_data)
{
  Intercambia_Ejercicios ();
}

void on_BN_save_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Examen ();
}

void on_EN_examen_descripcion_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Examen_Descripcion (widget, user_data);
}

void on_SC_preguntas_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_Pregunta ();
}

void on_SC_ejercicios_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_Ejercicio ();
}

void on_SC_posicion_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_Destino ();
}

void on_SC_N_versiones_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_Numero_Versiones ();
}

void on_CK_alfiler_global_toggled (GtkWidget *widget, gpointer user_data)
{
  Cambia_Alfiler ();
}

void on_SP_pre_examen_activate(GtkWidget *widget, gpointer user_data)
{
  long int k;
  gchar * codigo;

  codigo = gtk_editable_get_chars(GTK_EDITABLE(SP_pre_examen), 0, -1);
  k = atoi(codigo);
  g_free(codigo);

  gtk_spin_button_set_value (SP_pre_examen, k);
  Cambio_Codigo ();
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
}
