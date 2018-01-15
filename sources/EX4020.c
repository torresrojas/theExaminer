/*
Compilar con

cc -o EX4020 EX4020.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic

*/
/*******************************************/
/*  EX4020:                                */
/*                                         */
/*    Actualiza Base de Datos de Preguntas */
/*    con la información de un EXAMEN ya   */
/*    ejecutado.                           */
/*                                         */
/*    The Examiner 0.2                     */
/*    22 de Julio del 2012                 */
/*    Autor: Francisco J. Torres-Rojas     */        
/*                                         */
/*******************************************/
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libpq-fe.h>
#include <locale.h>
#include "examiner.h"

/***************************/
/* Globales y Definiciones */
/***************************/
#define EXAMEN_SIZE           6
#define DESCRIPCION_SIZE      201
#define CODIGO_EJERCICIO_SIZE 7
#define CODIGO_PREGUNTA_SIZE  7
#define CODIGO_VERSION_SIZE   5
struct PARAMETROS_EXAMINER parametros;

int N_preguntas   = 0;
int N_versiones   = 0;
int N_estudiantes = 0;

int Year_examen;
int Month_examen;
int Day_examen;
long double Media_examen;
long double Suma_cuadrados_examenes;
long double Alfa_examen;

struct VERSION
{
  char codigo [CODIGO_VERSION_SIZE];
  struct PREGUNTA_VERSION
  {
    char codigo [CODIGO_PREGUNTA_SIZE];
    char respuesta;
    int  orden_version [5];
  } * preguntas;
} * versiones = NULL;

struct PREGUNTA
{
  char ejercicio [CODIGO_EJERCICIO_SIZE];
  int  secuencia;
  char pregunta  [CODIGO_PREGUNTA_SIZE];
  long int  buenos;
  long int  malos;
  long int  acumulado_opciones      [5];
  long double suma_seleccion   [5];
  long double media_ex_1       [5];
  long double media_ex_0       [5];
  long double Rpb_opcion       [5];

  char correcta;
  long double previo;
  long double desviacion;
  long double porcentaje;
  long double suma_buenos;
  long double suma_malos;
  long double Rpb;
  long double alfa_sin;

  int  ajuste;
  int  correctas_nuevas[5];
  int  actualizar;
};

struct PREGUNTA * preguntas            = NULL;

/***************************/
/* Postgres stuff          */
/***************************/
PGconn	 *DATABASE;

/*************/
/* GTK stuff */
/*************/
GtkBuilder*builder; 
GError    * error;

GtkWidget *window1                   = NULL;
GtkWidget *window2                   = NULL;
GtkWidget *window3                   = NULL;

GtkWidget *EB_actualizacion          = NULL;
GtkWidget *FR_actualizacion          = NULL;

GtkSpinButton *SP_examen             = NULL;
GtkWidget *EN_descripcion            = NULL;
GtkWidget *EN_pre_examen             = NULL;
GtkWidget *EN_pre_examen_descripcion = NULL;
GtkWidget *EN_esquema                = NULL;
GtkWidget *EN_esquema_descripcion    = NULL;
GtkWidget *EN_materia                = NULL;
GtkWidget *EN_materia_descripcion    = NULL;
GtkWidget *EN_institucion            = NULL;
GtkWidget *EN_escuela                = NULL;
GtkWidget *EN_programa               = NULL;
GtkWidget *EN_profesor               = NULL;
GtkWidget *EN_fecha                  = NULL;
GtkWidget *EN_N_preguntas            = NULL;
GtkWidget *EN_N_versiones            = NULL;
GtkWidget *EN_N_estudiantes          = NULL;

GtkWidget *EB_prediccion             = NULL;
GtkWidget *EB_real                   = NULL;
GtkWidget *FR_prediccion             = NULL;
GtkWidget *FR_real                   = NULL;
GtkWidget *FR_ajustes                = NULL;
GtkWidget *FR_botones                = NULL;
GtkWidget *FR_actualizar             = NULL;
GtkWidget *FR_procesado              = NULL;

GtkWidget *EN_media_prediccion       = NULL;
GtkWidget *EN_desviacion_prediccion  = NULL;
GtkWidget *EN_alfa_prediccion        = NULL;
GtkWidget *EN_Rpb_prediccion         = NULL;

GtkWidget *EN_media_real             = NULL;
GtkWidget *EN_desviacion_real        = NULL;
GtkWidget *EN_alfa_real              = NULL;
GtkWidget *EN_Rpb_real               = NULL;

GtkWidget *BN_undo                = NULL;
GtkWidget *BN_save                = NULL;
GtkWidget *BN_terminar            = NULL;
GtkWidget *BN_confirm_update      = NULL;
GtkWidget *BN_cancelar            = NULL;
GtkWidget *BN_ok                  = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Actualice_Estadisticas        (int buena, char * ejercicio, int secuencia, long double Nota, char respuesta, int opcion_original[5]);
void Actualiza_Datos               ();
void Actualiza_Porcentajes         ();
void Calcula_estadisticas_examen   ();
void Calcula_Medias_y_RPB          (long int F_total_previo, long int F_previo, long int F_total_examen,  long int F_examen, 
                                    long double   media_1_previo, long double   media_0_previo,
                                    long double   media_1_examen, long double   media_0_examen,
                                    long double * media_1_nuevo,  long double * media_0_nuevo,
			            long double   desviacion_nueva, 
                                    long double * point_biserial);
void Cambio_Examen                 ();
void Carga_preguntas_examen        ();
void Connect_Widgets();
void Construye_versiones           (char * examen);
void Fin_de_Programa               (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                   (GtkWidget *widget, gpointer user_data);
void Inicializa_Tabla_estadisticas ();
void Init_Fields                   ();
void Interface_Coloring ();
void on_BN_cancelar_clicked        (GtkWidget *widget, gpointer user_data);
void on_BN_ok_clicked              (GtkWidget *widget, gpointer user_data);
void on_BN_save_clicked            (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked        (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked            (GtkWidget *widget, gpointer user_data);
void on_WN_ex4020_destroy          (GtkWidget *widget, gpointer user_data) ;
void Read_Only_Fields ();

/*----------------------------------------------------------------------------*/
/***************/
/* M A I N ( ) */
/***************/
int main(int argc, char *argv[]) 
{
  char hilera[100];
  int i;
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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX4020.glade", &error))
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
	  
          /* Most fields are READ ONLY */
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

void Connect_Widgets()
{
  window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex4020"));
  window2                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_actualizar"));
  window3                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_procesado"));

  EB_actualizacion = GTK_WIDGET (gtk_builder_get_object (builder, "EB_actualizacion"));
  FR_actualizacion = GTK_WIDGET (gtk_builder_get_object (builder, "FR_actualizacion"));

  SP_examen          = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_examen"));
  EN_descripcion            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));
  EN_pre_examen             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_pre_examen"));
  EN_pre_examen_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_pre_examen_descripcion"));
  EN_esquema                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema"));
  EN_esquema_descripcion    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema_descripcion"));
  EN_materia                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia"));
  EN_materia_descripcion    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia_descripcion"));
  EN_fecha                  = GTK_WIDGET (gtk_builder_get_object (builder, "EN_fecha"));
  EN_N_preguntas            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_N_preguntas"));
  EN_N_versiones            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_N_versiones"));
  EN_N_estudiantes          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_N_estudiantes"));

  EN_institucion            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_institucion"));
  EN_escuela                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_escuela"));
  EN_programa               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_programa"));
  EN_profesor               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_profesor"));

  EB_prediccion             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_prediccion"));
  FR_prediccion             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_prediccion"));
  EN_media_prediccion       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_prediccion"));
  EN_desviacion_prediccion  = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion_prediccion"));
  EN_alfa_prediccion        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_prediccion"));
  EN_Rpb_prediccion         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb_prediccion"));

  EB_real                   = GTK_WIDGET (gtk_builder_get_object (builder, "EB_real"));
  FR_real                   = GTK_WIDGET (gtk_builder_get_object (builder, "FR_real"));
  EN_media_real             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_real"));
  EN_desviacion_real        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion_real"));
  EN_alfa_real              = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_real"));
  EN_Rpb_real               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb_real"));

  FR_botones                = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));           
  FR_actualizar             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_actualizar"));
  FR_procesado              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_procesado"));

  BN_save                   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
  BN_undo                   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_terminar               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_confirm_update         = GTK_WIDGET (gtk_builder_get_object (builder, "BN_confirm_update"));
  BN_cancelar               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_cancelar"));
  BN_ok                     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_descripcion           , FALSE);
  gtk_widget_set_can_focus(EN_pre_examen            , FALSE);
  gtk_widget_set_can_focus(EN_pre_examen_descripcion, FALSE);
  gtk_widget_set_can_focus(EN_esquema               , FALSE);
  gtk_widget_set_can_focus(EN_esquema_descripcion   , FALSE);
  gtk_widget_set_can_focus(EN_materia               , FALSE);
  gtk_widget_set_can_focus(EN_materia_descripcion   , FALSE);
  gtk_widget_set_can_focus(EN_fecha                 , FALSE);
  gtk_widget_set_can_focus(EN_N_preguntas           , FALSE);
  gtk_widget_set_can_focus(EN_N_versiones           , FALSE);
  gtk_widget_set_can_focus(EN_N_estudiantes         , FALSE);
  gtk_widget_set_can_focus(EN_profesor              , FALSE);
  gtk_widget_set_can_focus(EN_institucion           , FALSE);
  gtk_widget_set_can_focus(EN_escuela               , FALSE);
  gtk_widget_set_can_focus(EN_programa              , FALSE);
  gtk_widget_set_can_focus(EN_media_prediccion      , FALSE);
  gtk_widget_set_can_focus(EN_desviacion_prediccion , FALSE);
  gtk_widget_set_can_focus(EN_alfa_prediccion       , FALSE);
  gtk_widget_set_can_focus(EN_Rpb_prediccion        , FALSE);
  gtk_widget_set_can_focus(EN_media_real            , FALSE);
  gtk_widget_set_can_focus(EN_desviacion_real       , FALSE);
  gtk_widget_set_can_focus(EN_alfa_real             , FALSE);
  gtk_widget_set_can_focus(EN_Rpb_real              , FALSE);
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (IMPORTANT_WINDOW, &color);
  gtk_widget_modify_bg(window2,  GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(window3,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (IMPORTANT_FR, &color);
  gtk_widget_modify_bg(FR_actualizar, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_procesado,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_actualizacion,   GTK_STATE_NORMAL,   &color);

  gdk_color_parse (SPECIAL_AREA_1, &color);
  gtk_widget_modify_bg(EB_prediccion,  GTK_STATE_NORMAL,   &color);  

  gdk_color_parse (SPECIAL_AREA_2, &color);
  gtk_widget_modify_bg(EB_real,             GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_examen), GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_botones,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_prediccion, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_real,       GTK_STATE_NORMAL,   &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,           GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,           GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_confirm_update, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,             GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_cancelar,       GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,           GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,           GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar,       GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_confirm_update, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,             GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_cancelar,       GTK_STATE_ACTIVE, &color);
}

void Init_Fields()
{
   char Examen [EXAMEN_SIZE];
   char hilera [100];
   int i, Last;
   PGresult *res;

   /* Recupera el examen con código más alto */
   res = PQEXEC(DATABASE, "SELECT codigo_examen from EX_examenes order by codigo_examen DESC limit 1");
   Last = 0;
   if (PQntuples(res))
      {
       Last = atoi (PQgetvalue (res, 0, 0));
      }
   PQclear(res);

   if (Last > 1)
      {
       gtk_widget_set_sensitive(GTK_WIDGET(SP_examen), 1);
       gtk_spin_button_set_range (SP_examen, 1.0, (long double) Last);
      }
   else
      { /* Solo hay un examen */
       gtk_widget_set_sensitive(GTK_WIDGET(SP_examen), 0);
       gtk_spin_button_set_range (SP_examen, 0.0, (long double) Last);
      }
   gtk_spin_button_set_value (SP_examen, Last);

#if 0
   /* start a transaction block */
   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);
   res = PQEXEC(DATABASE, "DECLARE ultimo_examen CURSOR FOR select codigo_examen from EX_examenes order by codigo_examen DESC"); PQclear(res);
   res = PQEXEC(DATABASE, "FETCH 1 in ultimo_examen");
   if (PQntuples(res))
      strcpy (Examen,PQgetvalue (res, 0, 0));
   else
      sprintf (Examen,"00001");

   PQclear(res);
   res = PQEXEC(DATABASE, "CLOSE ultimo_examen");  PQclear(res);

   /* end the transaction */
   res = PQEXEC(DATABASE, "END"); PQclear(res);

   gtk_entry_set_text(GTK_ENTRY(EN_examen), Examen);
#endif

   if (preguntas)            free (preguntas);
   preguntas            = NULL;

   if (versiones)
      {
       for (i=0;i<N_versiones;i++) free (versiones[i].preguntas);

       free (versiones);
       versiones            = NULL;
      }

   gtk_entry_set_text(GTK_ENTRY(EN_fecha), "\0");
   N_preguntas = N_versiones = N_estudiantes = 0;
   sprintf (hilera,"%7d", N_preguntas);
   gtk_entry_set_text(GTK_ENTRY(EN_N_preguntas),   hilera);
   gtk_entry_set_text(GTK_ENTRY(EN_N_versiones),   hilera);
   gtk_entry_set_text(GTK_ENTRY(EN_N_estudiantes), hilera);

   gtk_entry_set_text(GTK_ENTRY(EN_descripcion),            "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_pre_examen),             "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_pre_examen_descripcion), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema),                "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion),    "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia),                "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion),    "\0");
 
   gtk_entry_set_text(GTK_ENTRY(EN_institucion), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_escuela),     "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_programa),    "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_profesor),    "\0");

   gtk_widget_set_sensitive(GTK_WIDGET(SP_examen), 1);
   gtk_widget_set_sensitive(EN_pre_examen            , 0);
   gtk_widget_set_sensitive(EN_pre_examen_descripcion, 0);
   gtk_widget_set_sensitive(EN_esquema               , 0);
   gtk_widget_set_sensitive(EN_esquema_descripcion   , 0);
   gtk_widget_set_sensitive(EN_materia               , 0);
   gtk_widget_set_sensitive(EN_materia_descripcion   , 0);
   gtk_widget_set_sensitive(EN_fecha                 , 0);
   gtk_widget_set_sensitive(EN_N_preguntas           , 0);
   gtk_widget_set_sensitive(EN_N_versiones           , 0);
   gtk_widget_set_sensitive(EN_N_estudiantes         , 0);

   gtk_widget_set_sensitive(EN_institucion           , 0);
   gtk_widget_set_sensitive(EN_escuela               , 0);
   gtk_widget_set_sensitive(EN_programa              , 0);
   gtk_widget_set_sensitive(EN_profesor              , 0);
   gtk_widget_set_sensitive(EN_descripcion           , 0);

   gtk_widget_set_sensitive(FR_prediccion            , 0);
   gtk_widget_set_sensitive(FR_real                  , 0);

   gtk_widget_set_sensitive(BN_save,        0);
   gtk_widget_set_sensitive(BN_undo,        1);

   if (Last > 1)
      gtk_widget_grab_focus (GTK_WIDGET(SP_examen));
   else
      Cambio_Examen ();
}

void Cambio_Examen()
{
  char examen[10];
  char hilera      [200];
  char Descripcion [2*DESCRIPCION_SIZE]         = "*** Examen no está registrado o ya fue procesado ***";
  char PG_command  [4000];
  long double media_prediccion, desviacion_prediccion, alfa_prediccion, Rpb_prediccion;
  PGresult *res, *res_aux, *res_aux_2;
  int i, j, k;

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  sprintf (PG_command,"SELECT codigo_examen, descripcion, pre_examen, profesor, EX_examenes.materia, institucion, escuela, programa, esquema, descripcion_pre_examen, descripcion_esquema, descripcion_materia, nombre, prediccion_media, prediccion_desviacion, prediccion_alfa, prediccion_Rpb, Year, month, day, ejecutado from EX_examenes, EX_pre_examenes, EX_esquemas, BD_materias, BD_personas where codigo_examen = '%s' and pre_examen = codigo_pre_examen and esquema = codigo_esquema and EX_examenes.materia = codigo_materia and codigo_tema = '          ' and codigo_subtema = '          ' and profesor = codigo_persona", examen);

  res = PQEXEC(DATABASE, PG_command);

  if (PQntuples(res))
     {
      if (*PQgetvalue (res, 0, 20) == 'f')
	 {
          gtk_widget_set_sensitive(GTK_WIDGET(SP_examen), 0);

          gtk_widget_set_sensitive(EN_pre_examen            , 1);
          gtk_widget_set_sensitive(EN_pre_examen_descripcion, 1);
          gtk_widget_set_sensitive(EN_esquema               , 1);
          gtk_widget_set_sensitive(EN_esquema_descripcion   , 1);
          gtk_widget_set_sensitive(EN_materia               , 1);
          gtk_widget_set_sensitive(EN_materia_descripcion   , 1);
          gtk_widget_set_sensitive(EN_fecha                 , 1);
          gtk_widget_set_sensitive(EN_N_preguntas           , 1);
          gtk_widget_set_sensitive(EN_N_versiones           , 1);
          gtk_widget_set_sensitive(EN_N_estudiantes         , 1);
          gtk_widget_set_sensitive(EN_institucion           , 1);
          gtk_widget_set_sensitive(EN_escuela               , 1);
          gtk_widget_set_sensitive(EN_programa              , 1);
          gtk_widget_set_sensitive(EN_profesor              , 1);
          gtk_widget_set_sensitive(EN_descripcion           , 1);

          gtk_widget_set_sensitive(FR_prediccion            , 1);
          gtk_widget_set_sensitive(FR_real                  , 1);
          gtk_widget_set_sensitive(BN_save,        1);

          sprintf (PG_command,"SELECT DISTINCT version from EX_examenes_preguntas where examen = '%s' order by version", examen);
          res_aux = PQEXEC(DATABASE, PG_command);
          N_versiones = PQntuples(res_aux);
          PQclear(res_aux);

          sprintf (PG_command,"SELECT * from EX_examenes_preguntas where examen = '%s'", examen);
          res_aux = PQEXEC(DATABASE, PG_command);
          N_preguntas = PQntuples(res_aux);
          N_preguntas = N_preguntas/N_versiones;
          PQclear(res_aux);

          strcpy (Descripcion,                                     PQgetvalue (res, 0, 1));
          gtk_entry_set_text(GTK_ENTRY(EN_pre_examen),             PQgetvalue (res, 0, 2));
          gtk_entry_set_text(GTK_ENTRY(EN_pre_examen_descripcion), PQgetvalue (res, 0, 9));
          gtk_entry_set_text(GTK_ENTRY(EN_esquema),                PQgetvalue (res, 0, 8));
          gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion),    PQgetvalue (res, 0, 10));
          gtk_entry_set_text(GTK_ENTRY(EN_profesor),               PQgetvalue (res, 0, 12));
          gtk_entry_set_text(GTK_ENTRY(EN_materia),                PQgetvalue (res, 0, 4));
          gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion),    PQgetvalue (res, 0, 11));
          gtk_entry_set_text(GTK_ENTRY(EN_institucion),            PQgetvalue (res, 0, 5));
          gtk_entry_set_text(GTK_ENTRY(EN_escuela),                PQgetvalue (res, 0, 6));
          gtk_entry_set_text(GTK_ENTRY(EN_programa),               PQgetvalue (res, 0, 7));

          sprintf (hilera,"%7d", N_preguntas);
          gtk_entry_set_text(GTK_ENTRY(EN_N_preguntas), hilera);
          sprintf (hilera,"%7d", N_versiones);
          gtk_entry_set_text(GTK_ENTRY(EN_N_versiones), hilera);

          media_prediccion = atof (PQgetvalue (res, 0, 13));
          sprintf (hilera,"%8.3Lf", media_prediccion);
          gtk_entry_set_text(GTK_ENTRY(EN_media_prediccion), hilera);

          desviacion_prediccion = atof (PQgetvalue (res, 0, 14));
          sprintf (hilera,"%8.3Lf", desviacion_prediccion);
          gtk_entry_set_text(GTK_ENTRY(EN_desviacion_prediccion), hilera);

          alfa_prediccion = atof (PQgetvalue (res, 0, 15));
          sprintf (hilera,"%7.4Lf", alfa_prediccion);
          gtk_entry_set_text(GTK_ENTRY(EN_alfa_prediccion), hilera);

          Rpb_prediccion = atof (PQgetvalue (res, 0, 16));
          sprintf (hilera,"%7.4Lf", Rpb_prediccion);
          gtk_entry_set_text(GTK_ENTRY(EN_Rpb_prediccion), hilera);

          Year_examen  = atoi(PQgetvalue (res, 0, 17));
          Month_examen = atoi(PQgetvalue (res, 0, 18));
          Day_examen   = atoi(PQgetvalue (res, 0, 19));

          sprintf (hilera,"%02d/%02d/%02d", Day_examen, Month_examen, Year_examen);
          gtk_entry_set_text(GTK_ENTRY(EN_fecha), hilera);

          preguntas            = (struct PREGUNTA *) malloc ((sizeof (struct PREGUNTA) * N_preguntas));

          Construye_versiones           (examen);

          Carga_preguntas_examen        ();

          Actualiza_Porcentajes         ();

          Inicializa_Tabla_estadisticas ();
          Calcula_estadisticas_examen   ();

          gtk_widget_grab_focus   (BN_undo);
	 }
      else
	 {
          gtk_widget_set_sensitive(window1, 0);
          gtk_widget_show (window3);
	 }
     }
  else
     {
      gtk_widget_grab_focus (GTK_WIDGET(SP_examen));
     }

  PQclear(res);

  gtk_entry_set_text(GTK_ENTRY(EN_descripcion), Descripcion);
}

void Construye_versiones (char * examen)
{
  int i, j, k;
  char PG_command [3000];
  PGresult *res, *res_aux;
  int respuesta_original;
  int opcion_1,opcion_2,opcion_3,opcion_4,opcion_5;

  sprintf (PG_command,"SELECT DISTINCT version from EX_examenes_preguntas where examen = '%s' order by version", examen);
  res = PQEXEC(DATABASE, PG_command);
  N_versiones = PQntuples(res);
  versiones = (struct VERSION *) malloc (sizeof(struct VERSION) * N_versiones);

  for (i=0;i<N_versiones;i++)
      {
       strcpy (versiones[i].codigo, PQgetvalue (res, i, 0));

       sprintf (PG_command,"SELECT respuesta, opcion_1, opcion_2, opcion_3, opcion_4, opcion_5, codigo_pregunta from ex_examenes_preguntas, bd_texto_preguntas where examen = '%.5s' and version = '%.4s' and codigo_pregunta = codigo_unico_pregunta order by posicion", examen, PQgetvalue (res, i, 0));
       res_aux = PQEXEC(DATABASE, PG_command);

       N_preguntas = PQntuples(res_aux);

       versiones[i].preguntas = (struct PREGUNTA_VERSION *)  malloc(sizeof(struct PREGUNTA_VERSION)   * (N_preguntas));

       for (j=0; j<N_preguntas; j++)
	   {
  	    respuesta_original =     *PQgetvalue (res_aux, j, 0) - 'A';
	    opcion_1           = atoi(PQgetvalue (res_aux, j, 1));
	    opcion_2           = atoi(PQgetvalue (res_aux, j, 2));
	    opcion_3           = atoi(PQgetvalue (res_aux, j, 3));
	    opcion_4           = atoi(PQgetvalue (res_aux, j, 4));
	    opcion_5           = atoi(PQgetvalue (res_aux, j, 5));

	    if (respuesta_original == opcion_1) versiones[i].preguntas [j].respuesta = 'A';
	    if (respuesta_original == opcion_2) versiones[i].preguntas [j].respuesta = 'B';
	    if (respuesta_original == opcion_3) versiones[i].preguntas [j].respuesta = 'C';
	    if (respuesta_original == opcion_4) versiones[i].preguntas [j].respuesta = 'D';
	    if (respuesta_original == opcion_5) versiones[i].preguntas [j].respuesta = 'E';

	    versiones[i].preguntas [j].orden_version [0] = opcion_1;
	    versiones[i].preguntas [j].orden_version [1] = opcion_2;
	    versiones[i].preguntas [j].orden_version [2] = opcion_3;
	    versiones[i].preguntas [j].orden_version [3] = opcion_4;
	    versiones[i].preguntas [j].orden_version [4] = opcion_5;

	    strcpy (versiones[i].preguntas [j].codigo, PQgetvalue (res_aux, j, 6));
	   }

       PQclear(res_aux);
      }

  PQclear(res);
}

void Actualiza_Porcentajes ()
{ /* En caso que se haya actualizado la respuesta correcta de alguna pregunta */
  char PG_command          [2000];
  PGresult *res, *res_aux;
  int i, j, k;
  int N_estudiantes, N_correctas;
  long double Porcentaje;
  char examen[10];

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  sprintf (PG_command,"SELECT estudiante, version, respuestas from EX_examenes_respuestas where examen = '%s' order by estudiante", examen);
  res = PQEXEC(DATABASE, PG_command);
  N_estudiantes = PQntuples(res);

  for (i=0; i<N_estudiantes; i++)
      {
       for (j=0; (j < N_versiones) && strcmp (versiones[j].codigo, PQgetvalue(res, i, 1)) != 0; j++);

       N_correctas = 0;
       for (k=0; k<N_preguntas; k++)
	   {
	    if (versiones[j].preguntas [k].respuesta == PQgetvalue (res, i, 2)[k]) N_correctas++;
	   }
       Porcentaje = (long double) N_correctas/N_preguntas * 100.0;

       sprintf (PG_command,"UPDATE ex_examenes_respuestas set correctas = %d, porcentaje = %Lf where examen = '%s' and estudiante = %d", 
		N_correctas, Porcentaje,
	        examen, i+1);
       res_aux = PQEXEC(DATABASE, PG_command);
      }
}

void Carga_preguntas_examen ()
{
  int i, j, k;
  char PG_command  [3000];
  PGresult *res, *res_aux;
  gchar *materia;
  char examen[10];
  struct PREGUNTA temporal;

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);
  materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);

  sprintf (PG_command,"SELECT ejercicio, secuencia, codigo_pregunta, dificultad, BD_texto_preguntas.respuesta from EX_examenes_preguntas, BD_ejercicios, BD_texto_preguntas, BD_estadisticas_preguntas, BD_texto_ejercicios where examen = '%s' and codigo_ejercicio = ejercicio and materia = '%s' and codigo_pregunta = pregunta and codigo_unico_pregunta = pregunta and consecutivo_texto = texto_ejercicio order by version, tema, subtema, ejercicio, secuencia", examen, materia);
  res = PQEXEC(DATABASE, PG_command);

  for (i=0;i<N_preguntas;i++) /* Solo se carga la primer version */
      {
       strcpy (preguntas[i].ejercicio, PQgetvalue (res, i, 0));
       preguntas[i].secuencia =   atoi(PQgetvalue (res, i, 1));
       strcpy (preguntas[i].pregunta,  PQgetvalue (res, i, 2));
       preguntas[i].previo      = atof(PQgetvalue (res, i, 3));
       preguntas[i].correcta    =     *PQgetvalue (res, i, 4);

       preguntas[i].desviacion  = sqrt(preguntas[i].previo * (1.0 - preguntas[i].previo));

       preguntas[i].ajuste               = 0;
       preguntas[i].correctas_nuevas [0] = 0;
       preguntas[i].correctas_nuevas [1] = 0;
       preguntas[i].correctas_nuevas [2] = 0;
       preguntas[i].correctas_nuevas [3] = 0;
       preguntas[i].correctas_nuevas [4] = 0;
       preguntas[i].actualizar           = 1;

       sprintf (PG_command,"SELECT * from EX_examenes_ajustes where examen = '%s' and codigo_pregunta = '%s'", examen, PQgetvalue (res, i, 2));
       res_aux = PQEXEC(DATABASE, PG_command);
       if (PQntuples(res_aux))
	  {
           preguntas[i].ajuste               = atoi(PQgetvalue (res_aux, 0, 3));
           preguntas[i].correctas_nuevas [0] = atoi(PQgetvalue (res_aux, 0, 4));
           preguntas[i].correctas_nuevas [1] = atoi(PQgetvalue (res_aux, 0, 5));
           preguntas[i].correctas_nuevas [2] = atoi(PQgetvalue (res_aux, 0, 6));
           preguntas[i].correctas_nuevas [3] = atoi(PQgetvalue (res_aux, 0, 7));
           preguntas[i].correctas_nuevas [4] = atoi(PQgetvalue (res_aux, 0, 8));
           preguntas[i].actualizar           = atoi(PQgetvalue (res_aux, 0, 9));
	  }
       PQclear(res_aux);
      }
  PQclear(res);

  g_free (materia);
}

void Inicializa_Tabla_estadisticas ()
{
  int i, j;

  for (i=0;i<N_preguntas;i++)
      {
       preguntas[i].buenos      = 0;
       preguntas[i].malos       = 0;
       for (j=0;j<5;j++)
	   {
            preguntas[i].acumulado_opciones [j] = 0;
            preguntas[i].suma_seleccion     [j] = 0.0;
            preguntas[i].Rpb_opcion         [j] = 0.0;
	   }

       preguntas[i].suma_buenos = 0.0;
       preguntas[i].suma_malos  = 0.0;
      }
}

void Calcula_estadisticas_examen ()
{
  int i,j, k, N, version_actual;
  int buenas, malas;
  char PG_command  [3000];
  PGresult *res, *res_aux, *res_aux_2;
  char examen[10];
  char hilera[100];
  long double media, diferencia, varianza, desviacion, varianza_pregunta, desviacion_pregunta, suma_varianzas, alfa, Rpb;
  long double media_buenos, media_malos;
  long double media_sin, varianza_sin, alfa_sin;
  long double nota;

  struct PREGUNTA temporal;
  int opcion_original [5];

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  sprintf (PG_command,"SELECT porcentaje, version, respuestas, estudiante from EX_examenes_respuestas where examen = '%s' order by estudiante", examen);
  res = PQEXEC(DATABASE, PG_command);
  N_estudiantes = PQntuples(res);

  sprintf (hilera,"%7d", N_estudiantes);
  gtk_entry_set_text(GTK_ENTRY(EN_N_estudiantes), hilera);

  if (N_estudiantes <= 1)
     {
      gtk_widget_set_sensitive(FR_real,    0);
      gtk_widget_set_sensitive(BN_save,    0);
     }
  else
     {
      media                   = 0.0;
      Suma_cuadrados_examenes = 0.0;
      for (i=0;i<N_estudiantes;i++)
	  {
	   nota = atof(PQgetvalue (res, i, 0));

	   media += nota;
	   Suma_cuadrados_examenes += (nota * nota);
	  }

      media /= N_estudiantes;
      sprintf (hilera,"%8.3Lf", media);
      gtk_entry_set_text(GTK_ENTRY(EN_media_real), hilera);
      Media_examen = media;

      varianza = 0.0;
      for (i=0;i<N_estudiantes;i++)
	  {
	   diferencia = media - atof(PQgetvalue (res, i, 0));
	   varianza += (diferencia * diferencia);
	  }

      varianza /= N_estudiantes;

      desviacion = sqrt(varianza);
      sprintf (hilera,"%8.3Lf", desviacion);
      gtk_entry_set_text(GTK_ENTRY(EN_desviacion_real), hilera);

      for (i=0; i<N_estudiantes; i++)
	  {
           sprintf (PG_command,"SELECT ejercicio, secuencia, opcion_1, opcion_2, opcion_3, opcion_4, opcion_5 from EX_examenes_preguntas where examen = '%s' and version = '%s' order by posicion", examen, PQgetvalue (res, i, 1));
           res_aux = PQEXEC(DATABASE, PG_command);
           N = PQntuples(res_aux);

           for (version_actual=0; (version_actual < N_versiones) && strcmp (versiones[version_actual].codigo, PQgetvalue(res, i, 1)) != 0; version_actual++);

	   buenas = malas = 0;
	   for (j=0; j < N_preguntas; j++)
	       {
  	        opcion_original [0] =atoi(PQgetvalue (res_aux, j, 2));
	        opcion_original [1] =atoi(PQgetvalue (res_aux, j, 3));
	        opcion_original [2] =atoi(PQgetvalue (res_aux, j, 4));
	        opcion_original [3] =atoi(PQgetvalue (res_aux, j, 5));
	        opcion_original [4] =atoi(PQgetvalue (res_aux, j, 6));

		if (versiones[version_actual].preguntas[j].respuesta == PQgetvalue (res, i, 2)[j])
		   {
		    buenas++;
		   }
		else
		   {
		    malas++;
		   }

		Actualice_Estadisticas (versiones[version_actual].preguntas[j].respuesta == PQgetvalue (res, i, 2)[j],
                                        PQgetvalue (res_aux, j, 0), atoi(PQgetvalue (res_aux, j, 1)), (long double)atof(PQgetvalue (res, i, 0)),
					PQgetvalue (res, i, 2)[j], opcion_original);
	       }

           PQclear(res_aux);
	  }

      Rpb = 0.0;
      suma_varianzas = 0.0;
      for (i=0; i <N_preguntas; i++)
	  {
  	   preguntas[i].porcentaje = (long double)preguntas[i].buenos/N_estudiantes;

           varianza_pregunta = preguntas[i].porcentaje * (1.0 - preguntas[i].porcentaje);
	   desviacion_pregunta = sqrt(varianza_pregunta);
           suma_varianzas   += varianza_pregunta;

	   media_buenos = 0.0;
	   if (preguntas[i].buenos) media_buenos = preguntas[i].suma_buenos/preguntas[i].buenos;

	   media_malos = 0.0;
	   if (preguntas[i].malos)  media_malos  = preguntas[i].suma_malos/preguntas[i].malos;

	   preguntas[i].Rpb  = (media_buenos - media_malos) * (desviacion_pregunta/desviacion);
	   Rpb += preguntas[i].Rpb;

 	   for (j=0;j<5;j++)
	       {
	        if (preguntas[i].acumulado_opciones[j] == 0)
		   {
		    preguntas[i].media_ex_1 [j] = 0.0;
		   }
	        else
	           {
		    preguntas[i].media_ex_1 [j] = preguntas[i].suma_seleccion[j]/preguntas[i].acumulado_opciones[j];
	           }

	        if (preguntas[i].acumulado_opciones[j] == N_estudiantes)
		   {
		    preguntas[i].media_ex_0 [j] = 0.0;
		   }
	        else
	           {
		    preguntas[i].media_ex_0 [j] = (preguntas[i].suma_buenos + preguntas[i].suma_malos - preguntas[i].suma_seleccion[j])/(N_estudiantes-preguntas[i].acumulado_opciones[j]);
	           }
	       }

	  }

      alfa = ((long double) N_preguntas/ (long double) (N_preguntas - 1)) * (1.0 - (suma_varianzas/varianza));

      sprintf (hilera,"%8.3Lf", alfa);
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_real), hilera);
      Alfa_examen = alfa;

      Rpb /= N_preguntas;
      sprintf (hilera,"%8.3Lf", Rpb);
      gtk_entry_set_text(GTK_ENTRY(EN_Rpb_real), hilera);

      for (i=0; i <N_preguntas; i++)
	  {
	   media_sin = 0.0;

	   for (j=0; j<N_estudiantes;j++)
	       {/* calcular media ignorando pregunta i */
                sprintf (PG_command,"SELECT version from EX_versiones where examen = '%s' and version <= '%s'", examen, PQgetvalue(res, j, 1));
                res_aux_2 = PQEXEC(DATABASE, PG_command);
                version_actual = PQntuples(res_aux_2)-1;
                PQclear(res_aux_2);

		N = 0;
		for (k=0; k<N_preguntas;k++) N += (versiones[version_actual].preguntas[k].respuesta == PQgetvalue (res, j, 2)[k]);
		N -= (versiones[version_actual].preguntas[i].respuesta == PQgetvalue (res, j, 2)[i]);

		media_sin += ((long double) N * 100.0/ (N_preguntas-1));
	       }

	   media_sin /= N_estudiantes;

	   varianza_sin = 0.0;

	   for (j=0; j<N_estudiantes;j++)
	       {/* calcular varianza ignorando pregunta i */
                sprintf (PG_command,"SELECT version from EX_versiones where examen = '%s' and version <= '%s'", examen, PQgetvalue(res, j, 1));
                res_aux_2 = PQEXEC(DATABASE, PG_command);
                version_actual = PQntuples(res_aux_2)-1;
                PQclear(res_aux_2);

		N = 0;
		for (k=0; k<N_preguntas;k++) N += (versiones[version_actual].preguntas[k].respuesta == PQgetvalue (res, j, 2)[k]);
		N -= (versiones[version_actual].preguntas[i].respuesta == PQgetvalue (res, j, 2)[i]);

		varianza_sin += ((media_sin - ((long double) N * 100.0/ (N_preguntas-1))) * (media_sin - ((long double) N * 100.0/ (N_preguntas-1))));
	       }

	   varianza_sin /= N_estudiantes;
           varianza_pregunta = (((long double)preguntas[i].buenos/N_estudiantes)*((long double)preguntas[i].malos/N_estudiantes));

           preguntas[i].alfa_sin = ((long double) (N_preguntas-1)/ (long double) (N_preguntas - 2)) * (1.0 - ((suma_varianzas-varianza_pregunta)/varianza_sin));
	  }
     }

  PQclear(res);
}

void Actualice_Estadisticas (int buena, char * ejercicio, int secuencia, long double Nota, char respuesta, int opcion_original[5])
{
  int i, j;
  int found;

  for (i=0; (i < N_preguntas) && !((strcmp(ejercicio,preguntas[i].ejercicio)==0) && (preguntas[i].secuencia == secuencia)) ; i++);

  if (buena)
     {
      preguntas[i].buenos++;
      preguntas[i].suma_buenos += Nota;
     }
  else
     {
      preguntas[i].malos++;
      preguntas[i].suma_malos += Nota;
     }

  if ((respuesta >= 'A' )  && (respuesta <= 'E'))
     {
      preguntas[i].acumulado_opciones [opcion_original[respuesta-'A']]++;
      preguntas[i].suma_seleccion     [opcion_original[respuesta-'A']] += Nota;
     } 
}

void Actualiza_Datos ()
{
   int i, k;
   char PG_command [4000];
   PGresult *res, *res_aux;

   /* Estadísticas del Examen */
   gchar * cantidad;
   long double Media_Examen;
   long double Desviacion_Examen;
   long double Alfa_Examen;
   long double Rpb_Examen;

   /* Datos previos tomados de Base de Datos */
   int         Year_previo, Month_previo, Day_previo;
   int         N_examenes_previo;
   char        codigo_examen_previo [EXAMEN_SIZE];
   long double media_previo;
   int         N_estudiantes_previo;
   long double Suma_cuadrados_previo;
   long double Media_correctos_previo;
   long double Media_incorrectos_previo;
   long int    N_correctos_previo;
   long int    N_incorrectos_previo;
   long int    Frecuencia_previo;
   long int    Frecuencia_1_previo;
   long int    Frecuencia_2_previo;
   long int    Frecuencia_3_previo;
   long int    Frecuencia_4_previo;
   long int    Frecuencia_5_previo;
   long double Cronbach_con_previo;
   long double Cronbach_sin_previo;
   long int    N_Cronbach_previo;
   long double media_ex_1_1_previo, media_ex_1_0_previo;
   long double media_ex_2_1_previo, media_ex_2_0_previo;
   long double media_ex_3_1_previo, media_ex_3_0_previo;
   long double media_ex_4_1_previo, media_ex_4_0_previo;
   long double media_ex_5_1_previo, media_ex_5_0_previo;

   /* Datos actuales tomados del Examen */
   char examen_actual[10];

   /* Datos que serán grabados en Base de Datos */
   int         Year_nuevo, Month_nuevo, Day_nuevo;
   char        codigo_examen_nuevo [EXAMEN_SIZE];
   int         N_examenes_nuevo;
   int         N_estudiantes_nuevo;
   long double media_nuevo;
   long double Suma_cuadrados_nuevo;
   long double Varianza_nuevo;
   long double Desviacion_nuevo;
   long double Dificultad_nuevo;
   long double Rpb_nuevo;
   long double Media_correctos_nuevo;
   long double Media_incorrectos_nuevo;
   long int    N_correctos_nuevo;
   long int    N_incorrectos_nuevo;
   long double Cronbach_con_nuevo;
   long double Cronbach_sin_nuevo;
   long int    N_Cronbach_nuevo;
   long int    Frecuencia_nuevo;
   long int    Frecuencia_1_nuevo;
   long int    Frecuencia_2_nuevo;
   long int    Frecuencia_3_nuevo;
   long int    Frecuencia_4_nuevo;
   long int    Frecuencia_5_nuevo;
   long double media_ex_1_1_nuevo, media_ex_1_0_nuevo, point_biserial_1_nuevo;
   long double media_ex_2_1_nuevo, media_ex_2_0_nuevo, point_biserial_2_nuevo;
   long double media_ex_3_1_nuevo, media_ex_3_0_nuevo, point_biserial_3_nuevo;
   long double media_ex_4_1_nuevo, media_ex_4_0_nuevo, point_biserial_4_nuevo;
   long double media_ex_5_1_nuevo, media_ex_5_0_nuevo, point_biserial_5_nuevo;

   k = (int) gtk_spin_button_get_value_as_int (SP_examen);
   sprintf (examen_actual, "%05d", k);

   cantidad      = gtk_editable_get_chars(GTK_EDITABLE(EN_media_real), 0, -1);
   Media_Examen = atof(cantidad);
   g_free (cantidad);

   cantidad      = gtk_editable_get_chars(GTK_EDITABLE(EN_desviacion_real), 0, -1);
   Desviacion_Examen = atof(cantidad);
   g_free (cantidad);

   cantidad      = gtk_editable_get_chars(GTK_EDITABLE(EN_alfa_real), 0, -1);
   Alfa_Examen = atof(cantidad);
   g_free (cantidad);

   cantidad      = gtk_editable_get_chars(GTK_EDITABLE(EN_Rpb_real), 0, -1);
   Rpb_Examen = atof(cantidad);
   g_free (cantidad);

   /* start a transaction block */
   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

   for (i=0; i < N_preguntas; i++)
       {
	if (preguntas[i].actualizar)
	   {
 	    sprintf (PG_command,"SELECT * from BD_Estadisticas_Preguntas where pregunta = '%s'", preguntas[i].pregunta);
            res = PQEXEC(DATABASE, PG_command);

	    Year_previo              = atoi(PQgetvalue (res, 0, 1));
	    Month_previo             = atoi(PQgetvalue (res, 0, 2));
	    Day_previo               = atoi(PQgetvalue (res, 0, 3));
	    strcpy   (codigo_examen_previo, PQgetvalue (res, 0, 4));
	    N_examenes_previo        = atoi(PQgetvalue (res, 0, 5));
	    N_estudiantes_previo     = atoi(PQgetvalue (res, 0, 6));
	    media_previo             = atof(PQgetvalue (res, 0, 7));
	    Suma_cuadrados_previo    = atof(PQgetvalue (res, 0, 8));
	    Media_correctos_previo   = atof(PQgetvalue (res, 0, 13));
	    Media_incorrectos_previo = atof(PQgetvalue (res, 0, 14));
	    N_correctos_previo       = atoi(PQgetvalue (res, 0, 15));
	    N_incorrectos_previo     = atoi(PQgetvalue (res, 0, 16));
	    Cronbach_con_previo      = atof(PQgetvalue (res, 0, 17));
	    Cronbach_sin_previo      = atof(PQgetvalue (res, 0, 18));
	    N_Cronbach_previo        = atoi(PQgetvalue (res, 0, 19));
	    Frecuencia_previo        = atoi(PQgetvalue (res, 0, 20));
	    Frecuencia_1_previo      = atoi(PQgetvalue (res, 0, 21));
	    Frecuencia_2_previo      = atoi(PQgetvalue (res, 0, 22));
	    Frecuencia_3_previo      = atoi(PQgetvalue (res, 0, 23));
	    Frecuencia_4_previo      = atoi(PQgetvalue (res, 0, 24));
	    Frecuencia_5_previo      = atoi(PQgetvalue (res, 0, 25));
	    media_ex_1_1_previo      = atof(PQgetvalue (res, 0, 26));
	    media_ex_1_0_previo      = atof(PQgetvalue (res, 0, 27));
	    media_ex_2_1_previo      = atof(PQgetvalue (res, 0, 28));
	    media_ex_2_0_previo      = atof(PQgetvalue (res, 0, 29));
	    media_ex_3_1_previo      = atof(PQgetvalue (res, 0, 30));
	    media_ex_3_0_previo      = atof(PQgetvalue (res, 0, 31));
	    media_ex_4_1_previo      = atof(PQgetvalue (res, 0, 32));
	    media_ex_4_0_previo      = atof(PQgetvalue (res, 0, 33));
	    media_ex_5_1_previo      = atof(PQgetvalue (res, 0, 34));
	    media_ex_5_0_previo      = atof(PQgetvalue (res, 0, 35));

	    Frecuencia_1_nuevo = Frecuencia_1_previo + preguntas[i].acumulado_opciones [0];
	    Frecuencia_2_nuevo = Frecuencia_2_previo + preguntas[i].acumulado_opciones [1];
	    Frecuencia_3_nuevo = Frecuencia_3_previo + preguntas[i].acumulado_opciones [2];
	    Frecuencia_4_nuevo = Frecuencia_4_previo + preguntas[i].acumulado_opciones [3];
	    Frecuencia_5_nuevo = Frecuencia_5_previo + preguntas[i].acumulado_opciones [4];
	    Frecuencia_nuevo = Frecuencia_1_nuevo + Frecuencia_2_nuevo + Frecuencia_3_nuevo + Frecuencia_4_nuevo + Frecuencia_5_nuevo;
	    N_estudiantes_nuevo = N_estudiantes + N_estudiantes_previo;
	    /* Frecuencia_nuevo y N_estudiantes_nuevo podrían ser diferentes si hubiera respuestas en blanco */

	    media_nuevo = (media_previo * N_estudiantes_previo + Media_examen * N_estudiantes) / N_estudiantes_nuevo;
	    Suma_cuadrados_nuevo = Suma_cuadrados_previo + Suma_cuadrados_examenes;
	    Varianza_nuevo       = Suma_cuadrados_nuevo/N_estudiantes_nuevo - media_nuevo * media_nuevo;
	    Desviacion_nuevo     = sqrt(Varianza_nuevo);

	    Calcula_Medias_y_RPB (Frecuencia_previo, Frecuencia_1_previo, N_estudiantes, preguntas[i].acumulado_opciones [0],
                                  media_ex_1_1_previo, media_ex_1_0_previo,
				  preguntas[i].media_ex_1[0], preguntas[i].media_ex_0[0],
				  &media_ex_1_1_nuevo,&media_ex_1_0_nuevo,
				  Desviacion_nuevo,&point_biserial_1_nuevo);
	    Calcula_Medias_y_RPB (Frecuencia_previo, Frecuencia_2_previo, N_estudiantes, preguntas[i].acumulado_opciones [1],
                                  media_ex_2_1_previo, media_ex_2_0_previo,
				  preguntas[i].media_ex_1[1], preguntas[i].media_ex_0[1],
				  &media_ex_2_1_nuevo,&media_ex_2_0_nuevo,
				  Desviacion_nuevo,&point_biserial_2_nuevo);
	    Calcula_Medias_y_RPB (Frecuencia_previo, Frecuencia_3_previo, N_estudiantes, preguntas[i].acumulado_opciones [2],
                                  media_ex_3_1_previo, media_ex_3_0_previo,
				  preguntas[i].media_ex_1[2], preguntas[i].media_ex_0[2],
				  &media_ex_3_1_nuevo,&media_ex_3_0_nuevo,
				  Desviacion_nuevo,&point_biserial_3_nuevo);
	    Calcula_Medias_y_RPB (Frecuencia_previo, Frecuencia_4_previo, N_estudiantes, preguntas[i].acumulado_opciones [3],
                                  media_ex_4_1_previo, media_ex_4_0_previo,
				  preguntas[i].media_ex_1[3], preguntas[i].media_ex_0[3],
				  &media_ex_4_1_nuevo,&media_ex_4_0_nuevo,
				  Desviacion_nuevo,&point_biserial_4_nuevo);
	    Calcula_Medias_y_RPB (Frecuencia_previo, Frecuencia_5_previo, N_estudiantes, preguntas[i].acumulado_opciones [4],
                                  media_ex_5_1_previo, media_ex_5_0_previo,
				  preguntas[i].media_ex_1[4], preguntas[i].media_ex_0[4],
				  &media_ex_5_1_nuevo,&media_ex_5_0_nuevo,
				  Desviacion_nuevo,&point_biserial_5_nuevo);

	    N_examenes_nuevo = N_examenes_previo + 1;

	    Year_nuevo  = Year_previo;
	    Month_nuevo = Month_previo;
	    Day_nuevo   = Day_previo;
	    strcpy (codigo_examen_nuevo, codigo_examen_previo);
	    if ((Year_examen > Year_previo) || 
                ((Year_previo == Year_examen) && (Month_examen > Month_previo)) || 
                ((Year_previo == Year_examen) && (Month_previo == Month_examen) && (Day_examen >= Day_previo)))
	       {
		Year_nuevo  = Year_examen;
		Month_nuevo = Month_examen;
		Day_nuevo   = Day_examen;
		strcpy (codigo_examen_nuevo, examen_actual);
	       }

	    if      (preguntas[i].correcta == 'A')
	            {
                     Dificultad_nuevo        = (long double) Frecuencia_1_nuevo / Frecuencia_nuevo;
		     N_correctos_nuevo       = Frecuencia_1_nuevo;
		     N_incorrectos_nuevo     = Frecuencia_nuevo - Frecuencia_1_nuevo;
  	             Media_correctos_nuevo   = media_ex_1_1_nuevo;
  	             Media_incorrectos_nuevo = media_ex_1_0_nuevo;
		     Rpb_nuevo               = point_biserial_1_nuevo;
	            }
	    else if (preguntas[i].correcta == 'B')
	            {
                     Dificultad_nuevo = (long double) Frecuencia_2_nuevo / Frecuencia_nuevo;
		     N_correctos_nuevo       = Frecuencia_2_nuevo;
		     N_incorrectos_nuevo     = Frecuencia_nuevo - Frecuencia_2_nuevo;
  	             Media_correctos_nuevo   = media_ex_2_1_nuevo;
  	             Media_incorrectos_nuevo = media_ex_2_0_nuevo;
		     Rpb_nuevo               = point_biserial_2_nuevo;
	            }
	    else if (preguntas[i].correcta == 'C')
	            {
                     Dificultad_nuevo = (long double) Frecuencia_3_nuevo / Frecuencia_nuevo;
		     N_correctos_nuevo       = Frecuencia_3_nuevo;
		     N_incorrectos_nuevo     = Frecuencia_nuevo - Frecuencia_3_nuevo;
  	             Media_correctos_nuevo   = media_ex_3_1_nuevo;
  	             Media_incorrectos_nuevo = media_ex_3_0_nuevo;
		     Rpb_nuevo               = point_biserial_3_nuevo;
	            }
	    else if (preguntas[i].correcta == 'D')
	            {
                     Dificultad_nuevo = (long double) Frecuencia_4_nuevo / Frecuencia_nuevo;
		     N_correctos_nuevo       = Frecuencia_4_nuevo;
		     N_incorrectos_nuevo     = Frecuencia_nuevo - Frecuencia_4_nuevo;
  	             Media_correctos_nuevo   = media_ex_4_1_nuevo;
  	             Media_incorrectos_nuevo = media_ex_4_0_nuevo;
		     Rpb_nuevo               = point_biserial_4_nuevo;
	            }
	    else if (preguntas[i].correcta == 'E')
	            {
                     Dificultad_nuevo = (long double) Frecuencia_5_nuevo / Frecuencia_nuevo;
		     N_correctos_nuevo       = Frecuencia_5_nuevo;
		     N_incorrectos_nuevo     = Frecuencia_nuevo - Frecuencia_5_nuevo;
  	             Media_correctos_nuevo   = media_ex_5_1_nuevo;
  	             Media_incorrectos_nuevo = media_ex_5_0_nuevo;
		     Rpb_nuevo               = point_biserial_5_nuevo;
	            }

	    N_Cronbach_nuevo = N_Cronbach_previo + N_preguntas;
	    Cronbach_con_nuevo = ((Cronbach_con_previo * N_Cronbach_previo)+(Alfa_examen*N_preguntas))/N_Cronbach_nuevo;
	    Cronbach_sin_nuevo = ((Cronbach_sin_previo * N_Cronbach_previo)+(preguntas[i].alfa_sin*N_preguntas))/N_Cronbach_nuevo;

 	    sprintf (PG_command,"UPDATE BD_Estadisticas_Preguntas set ultimo_uso_year = %d, ultimo_uso_month = %d, ultimo_uso_day = %d, ultimo_examen = '%s', N_examenes = %d, N_estudiantes = %d, media_examenes = %Lf, suma_cuadrados_examenes = %Lf, varianza_examenes=%Lf, dificultad=%Lf, varianza_dificultad = %Lf, point_biserial = %Lf, media_correctos = %Lf, media_incorrectos=%Lf, N_correctos=%ld, N_incorrectos=%ld, cronbach_alpha_media_con=%Lf, cronbach_alpha_media_sin=%Lf, ponderacion_cronbach=%ld, frecuencia=%ld, frecuencia_1 = %ld, frecuencia_2=%ld, frecuencia_3=%ld, frecuencia_4=%ld, frecuencia_5=%ld, media_ex_1_1 = %Lf, media_ex_1_0 = %Lf, media_ex_2_1 = %Lf, media_ex_2_0 = %Lf, media_ex_3_1 = %Lf, media_ex_3_0 = %Lf, media_ex_4_1 = %Lf, media_ex_4_0 = %Lf, media_ex_5_1 = %Lf, media_ex_5_0 = %Lf, point_biserial_1=%Lf, point_biserial_2=%Lf, point_biserial_3=%Lf, point_biserial_4=%Lf, point_biserial_5=%Lf where pregunta = '%s'",
		     Year_nuevo, Month_nuevo, Day_nuevo, 
                     codigo_examen_nuevo,
                     N_examenes_nuevo,
                     N_estudiantes_nuevo,
                     media_nuevo,
		     Suma_cuadrados_nuevo,
		     Varianza_nuevo,
		     Dificultad_nuevo,
		     (1.0 - Dificultad_nuevo) * Dificultad_nuevo,
		     Rpb_nuevo,
		     Media_correctos_nuevo,
		     Media_incorrectos_nuevo,
		     N_correctos_nuevo,
		     N_incorrectos_nuevo,
		     Cronbach_con_nuevo,
		     Cronbach_sin_nuevo,
		     N_Cronbach_nuevo,
		     Frecuencia_nuevo,
		     Frecuencia_1_nuevo,
		     Frecuencia_2_nuevo,
		     Frecuencia_3_nuevo,
		     Frecuencia_4_nuevo,
		     Frecuencia_5_nuevo,
		     media_ex_1_1_nuevo,
		     media_ex_1_0_nuevo,
		     media_ex_2_1_nuevo,
		     media_ex_2_0_nuevo,
		     media_ex_3_1_nuevo,
		     media_ex_3_0_nuevo,
		     media_ex_4_1_nuevo,
		     media_ex_4_0_nuevo,
		     media_ex_5_1_nuevo,
		     media_ex_5_0_nuevo,
		     point_biserial_1_nuevo,
		     point_biserial_2_nuevo,
		     point_biserial_3_nuevo,
		     point_biserial_4_nuevo,
		     point_biserial_5_nuevo,
                     preguntas[i].pregunta);

            res_aux = PQEXEC(DATABASE, PG_command); PQclear(res_aux);
            PQclear(res);
	   }
       }

   sprintf (PG_command, "UPDATE ex_examenes set ejecutado=TRUE, real_media= %Lf, real_desviacion=%Lf, real_alfa=%Lf, real_rpb=%Lf where codigo_examen = '%s'",
	    Media_examen, Desviacion_Examen, Alfa_Examen, Rpb_Examen, examen_actual);

   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   /* end the transaction */
   res = PQEXEC(DATABASE, "END"); PQclear(res);

   Init_Fields ();
}

void Calcula_Medias_y_RPB(long int F_total_previo, long int F_previo, long int F_total_examen,  long int F_examen, 
                          long double   media_1_previo, long double   media_0_previo,
                          long double   media_1_examen, long double   media_0_examen,
                          long double * media_1_nuevo,  long double * media_0_nuevo,
			  long double   desviacion_nueva, 
                          long double * point_biserial)
{
   long int N_1_nuevo,  N_0_nuevo;
   long double dificultad;
   long double desviacion;

   N_1_nuevo  = F_examen + F_previo;
   N_0_nuevo  = F_total_examen + F_total_previo - F_examen - F_previo;

   dificultad = (long double) (F_examen + F_previo) / (F_total_examen + F_total_previo);
   desviacion = sqrt(dificultad * (1 - dificultad));

   if (N_1_nuevo)
      *media_1_nuevo = ((F_previo * media_1_previo) + (F_examen * media_1_examen))/ N_1_nuevo;
   else
      *media_1_nuevo = 0.0;

   if (N_0_nuevo)
      *media_0_nuevo = (((F_total_previo - F_previo) * media_0_previo) + ((F_total_examen - F_examen) * media_0_examen)) / N_0_nuevo;
   else
      *media_0_nuevo = 0.0;

   if (!N_1_nuevo || !N_0_nuevo)
      *point_biserial = 0.0;
   else
      *point_biserial = (*media_1_nuevo - *media_0_nuevo)*(desviacion/desviacion_nueva);
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
void on_WN_ex4020_destroy (GtkWidget *widget, gpointer user_data) 
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
  gtk_widget_set_sensitive (window1, 0);
  gtk_widget_show          (window2);
  gtk_widget_grab_focus    (BN_cancelar);
}

void on_BN_cancelar_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (BN_undo);
}

void on_BN_confirm_update_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
  Actualiza_Datos();
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window3);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (GTK_WIDGET(SP_examen));
}

void on_SP_examen_activate(GtkWidget *widget, gpointer user_data)
{
  long int k;
  gchar * codigo;

  codigo = gtk_editable_get_chars(GTK_EDITABLE(SP_examen), 0, -1);
  k = atoi(codigo);
  g_free(codigo);

  gtk_spin_button_set_value (SP_examen, k);
  Cambio_Examen ();
}
