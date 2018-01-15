/*
Compilar con

cc -o EX6010 EX6010.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic

*/
/*******************************************/
/*  EX6010:                                */
/*                                         */
/*    Elimina Prexámenes viejos            */
/*    The Examiner 0.4                     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    16 de Junio 2014                     */
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

#define MINIMO(a,b) ((a)<(b))?(a):(b)

#define CODIGO_SIZE              7
#define EJERCICIO_SIZE           7
#define PREGUNTA_SIZE            7
#define TEXTO_PREGUNTA_SIZE      4097

struct PREGUNTA
{
  char ejercicio [EJERCICIO_SIZE];
  int  secuencia;
  char codigo    [PREGUNTA_SIZE];
  char tema      [CODIGO_TEMA_SIZE    + 1];
  char subtema   [CODIGO_SUBTEMA_SIZE + 1];
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

GtkWidget *FR_hay_examenes        = NULL;
GtkWidget *BN_hay_examenes        = NULL;

GtkWidget *FR_pre_examen          = NULL;
GtkWidget *FR_medidas             = NULL;
GtkWidget *FR_botones             = NULL;

GtkSpinButton *SP_codigo          = NULL;
GtkWidget *EN_descripcion         = NULL;
GtkWidget *EN_materia             = NULL;
GtkWidget *EN_materia_descripcion = NULL;
GtkWidget *EN_esquema             = NULL;
GtkWidget *EN_esquema_descripcion = NULL;
GtkWidget *EN_preguntas           = NULL;

GtkToggleButton *CK_activar_borrado   = NULL;
GtkWidget *EB_borrado                 = NULL;
GtkWidget *FR_borrado                 = NULL;
GtkToggleButton *CK_borrar_preexamen  = NULL;
GtkToggleButton *CK_borrar_esquema    = NULL;

GtkWidget *BN_delete           = NULL;
GtkWidget *FR_verifica_borrado = NULL;
GtkLabel  *LB_advertencia      = NULL;
GtkLabel  *LB_operacion        = NULL;
GtkWidget *BN_confirma_borrado = NULL;
GtkWidget *BN_cancela_borrado  = NULL;

GtkProgressBar *PB_probabilidad   = NULL;
GtkProgressBar *PB_novedad        = NULL;
GtkWidget *EN_estudiantes         = NULL;
GtkWidget *EN_media               = NULL;
GtkWidget *EN_desviacion          = NULL;
GtkWidget *EN_alfa                = NULL;
GtkWidget *EN_Rpb                 = NULL;

GtkWidget *BN_undo                = NULL;
GtkWidget *BN_terminar            = NULL;

GtkWidget *EB_ready = NULL;
GtkWidget *FR_ready = NULL;
GtkWidget *BN_ok    = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Advertencia_borrado            ();
void Borra_Datos                    ();
void Calcula_Barras                 ();
void Cambio_Codigo                  ();
void Carga_Pre_Examen               (PGresult * res, gchar * codigo);
long double CDF                     (long double X, long double Media, long double Desv);
void Extrae_codigo                  (char * hilera, char * codigo);
void Fin_de_Programa                (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                    (GtkWidget *widget, gpointer user_data);
void Init_Fields                    ();
int main                            (int argc, char *argv[]);
void on_BN_cancela_borrado_clicked  (GtkWidget *widget, gpointer user_data);
void on_BN_confirma_borrado_clicked (GtkWidget *widget, gpointer user_data);
void on_BN_delete_clicked           (GtkWidget *widget, gpointer user_data);
void on_BN_hay_examenes_clicked     (GtkWidget *widget, gpointer user_data);
void on_BN_ok_clicked               (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked         (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked             (GtkWidget *widget, gpointer user_data);
void on_CK_activar_borrado_toggled  (GtkWidget *widget, gpointer user_data);
void on_CK_borrar_preexamen_toggled (GtkWidget *widget, gpointer user_data);
void on_SP_codigo_activate          (GtkWidget *widget, gpointer user_data);
void on_WN_ex6010_destroy           (GtkWidget *widget, gpointer user_data);

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX6010.glade", &error))
         {
          g_warning ("%s\n", error->message);
          g_error_free (error);
         }
      else
	 {
	  /* Conecta funciones con eventos de la interfaz */
          gtk_builder_connect_signals (builder, NULL);

	  /* Crea conexión con los campos de la interfaz a ser controlados */
          window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex6010"));
          window2                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_hay_examenes"));
          window3                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_verifica_borrado"));
          window4                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ready"));

          FR_hay_examenes        = GTK_WIDGET (gtk_builder_get_object (builder, "FR_hay_examenes"));
          BN_hay_examenes        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_hay_examenes"));

          FR_pre_examen          = GTK_WIDGET (gtk_builder_get_object (builder, "FR_pre_examen"));
          FR_medidas             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_medidas"));
          FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));

          SP_codigo              = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_codigo"));

          EN_descripcion         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));
          EN_materia             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia"));
          EN_materia_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia_descripcion"));
          EN_esquema             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema"));
          EN_esquema_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema_descripcion"));
          EN_preguntas           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_preguntas"));


          EB_borrado                = GTK_WIDGET (gtk_builder_get_object (builder, "EB_borrado"));
          FR_borrado                = GTK_WIDGET (gtk_builder_get_object (builder, "FR_borrado"));
          CK_activar_borrado        = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_activar_borrado"));
          CK_borrar_preexamen       = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_borrar_preexamen"));
          CK_borrar_esquema         = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_borrar_esquema"));

          PB_probabilidad        = (GtkProgressBar *) GTK_WIDGET (gtk_builder_get_object (builder, "PB_probabilidad"));
          PB_novedad             = (GtkProgressBar *) GTK_WIDGET (gtk_builder_get_object (builder, "PB_novedad"));
          EN_estudiantes         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_estudiantes"));
          EN_media               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media"));
          EN_desviacion          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion"));
          EN_alfa                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa"));
          EN_Rpb                 = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb"));

          FR_verifica_borrado = GTK_WIDGET (gtk_builder_get_object (builder, "FR_verifica_borrado"));
          LB_operacion        = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_operacion"));
          BN_confirma_borrado = GTK_WIDGET (gtk_builder_get_object (builder, "BN_confirma_borrado"));
          BN_cancela_borrado  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_cancela_borrado"));

          BN_delete      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_delete"));
          BN_undo        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
          BN_terminar    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));

          EB_ready = GTK_WIDGET (gtk_builder_get_object (builder, "EB_ready"));
          FR_ready = GTK_WIDGET (gtk_builder_get_object (builder, "FR_ready"));
          BN_ok    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));

	  /* Muchos campos son READ ONLY */
          gtk_widget_set_can_focus(EN_descripcion        , FALSE);
          gtk_widget_set_can_focus(EN_materia            , FALSE);
          gtk_widget_set_can_focus(EN_materia_descripcion, FALSE);
          gtk_widget_set_can_focus(EN_esquema            , FALSE);
          gtk_widget_set_can_focus(EN_esquema_descripcion, FALSE);
          gtk_widget_set_can_focus(EN_preguntas          , FALSE);
 
          gtk_widget_set_can_focus(EN_estudiantes        , FALSE);
          gtk_widget_set_can_focus(EN_media              , FALSE);
          gtk_widget_set_can_focus(EN_desviacion         , FALSE);
          gtk_widget_set_can_focus(EN_alfa               , FALSE);
          gtk_widget_set_can_focus(EN_Rpb                , FALSE);

	  /* Un poco de color */

          gdk_color_parse ("pale goldenrod", &color);
          gtk_widget_modify_bg(window1,        GTK_STATE_NORMAL, &color);

          gdk_color_parse ("dark goldenrod", &color);
          gtk_widget_modify_bg(GTK_WIDGET(SP_codigo), GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(FR_pre_examen,         GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(FR_botones,            GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(BN_undo,               GTK_STATE_ACTIVE, &color);
          gtk_widget_modify_bg(BN_terminar,           GTK_STATE_ACTIVE, &color);
          gtk_widget_modify_bg(BN_delete,             GTK_STATE_ACTIVE, &color);

          gdk_color_parse ("dark red", &color);
          gtk_widget_modify_bg(BN_cancela_borrado,   GTK_STATE_ACTIVE, &color);

          gdk_color_parse ("light goldenrod", &color);
          gtk_widget_modify_bg(BN_undo,        GTK_STATE_PRELIGHT, &color);
          gtk_widget_modify_bg(BN_terminar,    GTK_STATE_PRELIGHT, &color);
          gtk_widget_modify_bg(BN_delete,      GTK_STATE_PRELIGHT, &color);
          gtk_widget_modify_bg(FR_medidas,     GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(EN_estudiantes, GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(EN_media,       GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(EN_desviacion,  GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(EN_alfa,        GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(EN_Rpb,         GTK_STATE_NORMAL, &color);

          gdk_color_parse ("red", &color);
          gtk_widget_modify_bg(BN_cancela_borrado, GTK_STATE_PRELIGHT,&color);
          gtk_widget_modify_bg(FR_borrado,        GTK_STATE_NORMAL,  &color);

          gdk_color_parse ("green", &color);
          gtk_widget_modify_bg(FR_hay_examenes, GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(BN_confirma_borrado, GTK_STATE_PRELIGHT, &color);
          gtk_widget_modify_bg(FR_ready,            GTK_STATE_NORMAL,   &color);
          gtk_widget_modify_bg(BN_ok,               GTK_STATE_PRELIGHT, &color);

          gdk_color_parse ("dark green", &color);
          gtk_widget_modify_bg(window2,         GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(EB_ready,            GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(BN_hay_examenes, GTK_STATE_ACTIVE, &color);
          gtk_widget_modify_bg(BN_ok,           GTK_STATE_ACTIVE, &color);
          gtk_widget_modify_bg(BN_confirma_borrado, GTK_STATE_ACTIVE, &color);

          gdk_color_parse ("green", &color);
          gtk_widget_modify_bg(BN_hay_examenes, GTK_STATE_PRELIGHT, &color);
          gtk_widget_modify_bg(BN_ok,           GTK_STATE_PRELIGHT, &color);

          gdk_color_parse ("gold", &color);
          gtk_widget_modify_bg(GTK_WIDGET(PB_probabilidad), GTK_STATE_PRELIGHT, &color);
          gtk_widget_modify_bg(GTK_WIDGET(PB_novedad)     , GTK_STATE_PRELIGHT, &color);

          gdk_color_parse ("black", &color);
          gtk_widget_modify_bg(FR_verifica_borrado, GTK_STATE_NORMAL, &color);

          gdk_color_parse ("white", &color);
          gtk_widget_modify_bg(EB_borrado, GTK_STATE_NORMAL, &color);

          gdk_color_parse ("yellow", &color);
          gtk_widget_modify_bg(window3, GTK_STATE_NORMAL, &color);

          gtk_widget_show (window1);

          Init_Fields ();

          /* start the event loop */
          gtk_main();
	 }
     }

  return 0;
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
      { /* Solo hay un preexamen */
       gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo), 0);
       gtk_spin_button_set_range (SP_codigo, 0.0, (long double) Last);
      }
   gtk_spin_button_set_value (SP_codigo, Last);

   Numero_Preguntas = 0;
   sprintf (hilera,"%d", Numero_Preguntas);
   gtk_entry_set_text(GTK_ENTRY(EN_preguntas),        hilera);

   gtk_entry_set_text(GTK_ENTRY(EN_descripcion)        , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia)            , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema)            , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion), "\0");

   Calcula_Barras ();

   gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo) , 1);
   gtk_widget_set_sensitive(EN_descripcion        , 1);

   gtk_widget_set_sensitive(FR_pre_examen         , 0);
   gtk_widget_set_sensitive(EN_materia            , 0);
   gtk_widget_set_sensitive(EN_materia_descripcion, 0);
   gtk_widget_set_sensitive(EN_esquema            , 0);
   gtk_widget_set_sensitive(EN_esquema_descripcion, 0);
   gtk_widget_set_sensitive(EN_preguntas          , 0);

   gtk_widget_set_sensitive(BN_delete, 0);
   gtk_widget_set_sensitive(BN_undo,   1);
 
   gtk_toggle_button_set_active(CK_activar_borrado, FALSE);
   gtk_widget_set_sensitive(GTK_WIDGET(CK_activar_borrado), 0);
   gtk_widget_set_sensitive(FR_borrado               , 0);
   gtk_toggle_button_set_active(CK_borrar_preexamen, FALSE);
   gtk_widget_set_sensitive(GTK_WIDGET(CK_borrar_preexamen), 1);
   gtk_toggle_button_set_active(CK_borrar_esquema, FALSE);
   gtk_widget_set_sensitive(GTK_WIDGET(CK_borrar_esquema), 0);

   gtk_widget_set_sensitive(BN_delete, 0);

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
	 { /* Aún hay exámenes derivados de preexamen - no se puede borrar */
          gtk_widget_set_sensitive(window1, 0);
          gtk_widget_show         (window2);
          gtk_widget_grab_focus   (GTK_WIDGET(SP_codigo));
         }
      else
	 {
          Carga_Pre_Examen (res, codigo);
          gtk_widget_set_sensitive(GTK_WIDGET(GTK_WIDGET(SP_codigo)), 0);
          gtk_widget_set_sensitive(GTK_WIDGET(CK_activar_borrado), 1);
          gtk_widget_grab_focus   (BN_undo);
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
  PGresult *res_aux;

  gtk_widget_set_sensitive(EN_descripcion        , 1);
  gtk_widget_set_sensitive(EN_materia            , 1);
  gtk_widget_set_sensitive(EN_materia_descripcion, 1);
  gtk_widget_set_sensitive(EN_esquema            , 1);
  gtk_widget_set_sensitive(EN_esquema_descripcion, 1);
  gtk_widget_set_sensitive(EN_preguntas          , 1);
  gtk_widget_set_sensitive(EN_media              , 1);
  gtk_widget_set_sensitive(EN_desviacion         , 1);

  gtk_entry_set_text(GTK_ENTRY(EN_descripcion),         PQgetvalue (res, 0, 2));
  gtk_entry_set_text(GTK_ENTRY(EN_materia),             PQgetvalue (res, 0, 4));
  gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), PQgetvalue (res, 0, 5));
  gtk_entry_set_text(GTK_ENTRY(EN_esquema),             PQgetvalue (res, 0, 1));
  gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion), PQgetvalue (res, 0, 3));

  sprintf (PG_command,"SELECT ejercicio, secuencia, codigo_pregunta, texto_pregunta, dificultad, varianza_dificultad, usa_header, frecuencia, ultimo_uso_year, ultimo_uso_month, ultimo_uso_day, cronbach_alpha_media_con, point_biserial, tema, subtema from EX_pre_examenes_lineas, BD_ejercicios, BD_texto_ejercicios, BD_texto_preguntas, BD_estadisticas_preguntas where Pre_Examen = '%s' and codigo_pregunta = codigo_unico_pregunta and codigo_consecutivo_ejercicio = consecutivo_texto and codigo_unico_pregunta = pregunta and consecutivo_texto = texto_ejercicio and materia = '%s' order by ejercicio, secuencia_pregunta", codigo, PQgetvalue (res, 0, 4));
  res_aux = PQEXEC(DATABASE, PG_command);

  Numero_Preguntas = PQntuples(res_aux);
  sprintf (hilera,"%d", Numero_Preguntas);
  gtk_entry_set_text(GTK_ENTRY(EN_preguntas), hilera);

  for (i=0;i<Numero_Preguntas;i++)
      {
       strcpy (Pre_Examen[i].ejercicio     , PQgetvalue (res_aux, i, 0));
       Pre_Examen[i].secuencia =        atoi(PQgetvalue (res_aux, i, 1));
       strcpy (Pre_Examen[i].codigo        , PQgetvalue (res_aux, i, 2));
       strcpy (Pre_Examen[i].texto_pregunta, PQgetvalue (res_aux, i, 3));
       Pre_Examen[i].media    =         atof(PQgetvalue (res_aux, i, 4));
       Pre_Examen[i].varianza =         atof(PQgetvalue (res_aux, i, 5));
       if (*PQgetvalue (res_aux, i, 6) == 't') 
          Pre_Examen[i].Header = 1; 
       else 
          Pre_Examen[i].Header = 0; 
       Pre_Examen[i].frecuencia =       atoi(PQgetvalue (res_aux, i, 7));
       Pre_Examen[i].Year       =       atoi(PQgetvalue (res_aux, i, 8));
       Pre_Examen[i].Month      =       atoi(PQgetvalue (res_aux, i, 9));
       Pre_Examen[i].Day        =       atoi(PQgetvalue (res_aux, i, 10));
       Pre_Examen[i].alfa       =       atof(PQgetvalue (res_aux, i, 11));
       Pre_Examen[i].Rpb        =       atof(PQgetvalue (res_aux, i, 12));
       strcpy (Pre_Examen[i].tema          , PQgetvalue (res_aux, i, 13));
       strcpy (Pre_Examen[i].subtema       , PQgetvalue (res_aux, i, 14));
      }

  PQclear(res_aux);

  Calcula_Barras ();

  if (Numero_Preguntas) gtk_widget_set_sensitive(FR_pre_examen, 1);
}

void Extrae_codigo (char * hilera, char * codigo)
{
  int i;

  i = 0;

  while (hilera[i] != ' ') codigo [i] = hilera[i++];

  codigo[i] = '\0';
}

void Calcula_Barras()
{
#define DIAS_MAXIMO       730
  int i;
  long double media, varianza, desviacion, probabilidad, alfa, Rpb;
  long double novedad_proporcional;
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

  novedad_proporcional = novedad_pre_examen / DIAS_MAXIMO;
  novedad_proporcional = MINIMO (1.0, novedad_proporcional);

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

  gtk_progress_bar_set_fraction (PB_probabilidad,(gdouble) probabilidad);
  gtk_progress_bar_set_fraction (PB_novedad     ,(gdouble) novedad_proporcional);
}

long double CDF (long double X, long double Media, long double Desv)
{
  return (0.5*(1 + (long double)erf((X-Media)/(Desv*M_SQRT2))));
}

void Advertencia_borrado ()
{
  int operacion;
  int estado;
  char * mensajes_operacion[] =
    {"<b>No se indicó que información\nse desea eliminar...</b>",
     "<b>¿Desea borrar este Preexamen?</b>",
     "***",
     "<b>¿Desea borrar este Preexamen\n y el Esquema asociado?</b>",
    };

  operacion = 0;
  if (gtk_toggle_button_get_active(CK_borrar_preexamen))  operacion += 1;
  if (gtk_toggle_button_get_active(CK_borrar_esquema))    operacion += 2;
  gtk_label_set_markup(LB_operacion, mensajes_operacion[operacion]);

  gtk_widget_set_sensitive (window1, 0);
  if (!operacion)
     gtk_widget_set_sensitive(BN_confirma_borrado, 0);
  else
     gtk_widget_set_sensitive(BN_confirma_borrado, 1);

  gtk_widget_show          (window3);
  gtk_widget_grab_focus    (BN_cancela_borrado);
}

void Borra_Datos ()
{
  int k;
  PGresult *res;
  char PG_command [4000], preexamen[6];
  gchar *esquema;

  k = (int) gtk_spin_button_get_value_as_int (SP_codigo);
  sprintf (preexamen, "%05d", k);
  esquema    = gtk_editable_get_chars(GTK_EDITABLE(EN_esquema),    0, -1);

  res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

  if (gtk_toggle_button_get_active(CK_borrar_preexamen))
     {
      sprintf (PG_command, "DELETE FROM EX_pre_examenes where codigo_pre_examen = '%s'", preexamen);
      res = PQEXEC(DATABASE, PG_command);
      PQclear(res);
      /* Por ON DELETE CASCADE se borran también EX_pre_examenes_lineas */
     }

  if (gtk_toggle_button_get_active(CK_borrar_esquema))
     {
      sprintf (PG_command, "DELETE FROM EX_esquemas where codigo_esquema = '%s'", esquema);
      res = PQEXEC(DATABASE, PG_command);
      PQclear(res);
      /* Por ON DELETE CASCADE se borran también EX_esquemas_lineas */
     }

   res = PQEXEC(DATABASE, "END"); PQclear(res);

   g_free (esquema);

   Init_Fields ();

   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show (window4);
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
void on_WN_ex6010_destroy (GtkWidget *widget, gpointer user_data) 
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

void on_BN_delete_clicked(GtkWidget *widget, gpointer user_data)
{
  Advertencia_borrado ();
}

void on_BN_cancela_borrado_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window3);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (BN_undo);
}

void on_BN_confirma_borrado_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window3);

  Borra_Datos();
}

void on_BN_hay_examenes_clicked(GtkWidget *widget, gpointer user_data)
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

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window4);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (GTK_WIDGET(SP_codigo));
}

void on_CK_activar_borrado_toggled (GtkWidget *widget, gpointer user_data)
{
  if (gtk_toggle_button_get_active(CK_activar_borrado))
     {
      gtk_widget_set_sensitive(FR_borrado, 1);
      gtk_widget_set_sensitive(BN_delete,  1);
     }
  else
     {
      gtk_widget_set_sensitive(FR_borrado, 0);
      gtk_widget_set_sensitive(BN_delete,  0);
     }
}

void on_CK_borrar_preexamen_toggled (GtkWidget *widget, gpointer user_data)
{
  PGresult *res;
  char PG_command[1000];
  gchar * esquema;
  int k;

  k = (int) gtk_spin_button_get_value_as_int (SP_codigo);
  esquema    = gtk_editable_get_chars(GTK_EDITABLE(EN_esquema),    0, -1);

  if (gtk_toggle_button_get_active(CK_borrar_preexamen))
     {
      sprintf (PG_command,"SELECT codigo_pre_examen from EX_pre_examenes where esquema = '%s' and codigo_pre_examen != '%05d'", esquema, k);
      res = PQEXEC(DATABASE, PG_command);
      if (!PQntuples(res))
         gtk_widget_set_sensitive(GTK_WIDGET(CK_borrar_esquema), 1);
      else
         gtk_widget_set_sensitive(GTK_WIDGET(CK_borrar_esquema), 0);
      PQclear(res);
     }
  else
     {
      gtk_toggle_button_set_active(CK_borrar_esquema, FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(CK_borrar_esquema),   0);
     }

  g_free (esquema);
}

