/*
Compilar con:

cc -o EX EX.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX:                                    */
/*                                         */
/*    Menú Principal                       */
/*    The Examiner 0.1                     */
/*    6 de Enero 2011                      */
/*    Autor: Francisco J. Torres-Rojas     */
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    26 de Noviembre 2011                 */
/*-----------------------------------------*/
/*    Verifica que se haya ejecutado       */
/*    programa de configuración antes de   */
/*    ejecutar cualquier programa          */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    25 de Junio del 2013                 */
/*******************************************/
#include <stdlib.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include "examiner.h"

/******************/
/* Postgres stuff */
/******************/
PGconn	 *DATABASE;

/*************/
/* GTK stuff */
/*************/
GtkBuilder*builder; 
GError    * error = NULL;

GtkWidget *window1 = NULL;
GtkWidget *window2 = NULL;
GtkWidget *window3 = NULL;
GtkWidget *window4 = NULL;
GtkWidget *window5 = NULL;
GtkWidget *window6 = NULL;
GtkWidget *window7 = NULL;
GtkWidget *window8 = NULL;
GtkWidget *window9 = NULL;
GtkWidget *window10 = NULL;
GtkWidget *window11 = NULL;
GtkWidget *EN_materias      = NULL;
GtkWidget *EN_autores       = NULL;
GtkWidget *EN_ejercicios    = NULL;
GtkWidget *EN_preguntas     = NULL;
GtkWidget *EN_esquemas      = NULL;
GtkWidget *EN_pre_examenes  = NULL;
GtkWidget *EN_examenes      = NULL;
GtkWidget *EN_evaluaciones  = NULL;
GtkWidget *EN_respaldo      = NULL;
GtkWidget *BN_refresh       = NULL;
GtkWidget *FR_no_disponible = NULL;
GtkWidget *FR_no_materias   = NULL;
GtkWidget *FR_no_autores    = NULL;
GtkWidget *FR_no_ejercicios = NULL;
GtkWidget *FR_no_preguntas  = NULL;
GtkWidget *FR_no_esquemas   = NULL;
GtkWidget *FR_no_configurado= NULL;

GtkWidget *FR_archivo       = NULL;
GtkImage  *IM_archivo       = NULL;
GtkWidget *BN_exit          = NULL;
GtkWidget *BN_parametros    = NULL;
GtkWidget *BN_backup        = NULL;
GtkWidget *BN_upgrade       = NULL;

GtkWidget *FR_datos1        = NULL;
GtkImage  *IM_datos1        = NULL;
GtkWidget *BN_materias      = NULL;
GtkWidget *BN_objetivos     = NULL;
GtkWidget *BN_autores       = NULL;
GtkWidget *BN_ejercicios    = NULL;
GtkWidget *BN_preguntas     = NULL;
GtkWidget *BN_recorrer      = NULL;
GtkWidget *BN_copias        = NULL;
GtkWidget *BN_ligas         = NULL;

GtkWidget *FR_examenes      = NULL;
GtkImage  *IM_examenes      = NULL;
GtkWidget *BN_esquemas      = NULL;
GtkWidget *BN_sorteo        = NULL;
GtkWidget *BN_preexamenes   = NULL;
GtkWidget *BN_refinar       = NULL;
GtkWidget *BN_examen        = NULL;
GtkWidget *BN_final         = NULL;

GtkWidget *FR_revision      = NULL;
GtkImage  *IM_revision      = NULL;
GtkWidget *BN_revision       = NULL;
GtkWidget *BN_analisis      = NULL;
GtkWidget *BN_actualizar    = NULL;

GtkWidget *FR_reportes        = NULL;
GtkImage  *IM_reportes        = NULL;
GtkWidget *BN_listamaterias   = NULL;
GtkWidget *BN_listaejercicios = NULL;
GtkWidget *BN_listaanalisis   = NULL;
GtkWidget *BN_cruce           = NULL;
GtkWidget *BN_listatodos      = NULL;

GtkWidget *FR_limpieza           = NULL;
GtkImage  *IM_limpieza           = NULL;
GtkWidget *BN_borra_examenes     = NULL;
GtkWidget *BN_borra_pre_examenes = NULL;
GtkWidget *BN_borra_esquemas     = NULL;

GtkImage  *IM_background    = NULL;
GtkLabel  *LB_version       = NULL;

/**************/
/* Prototipos */
/**************/
void Asigna_version           ();
void Connect_Widgets          ();
void Fin_de_Programa          (GtkWidget *widget, gpointer user_data);
void Init_Fields              ();
void Interface_Coloring       ();
void on_AB_ex_response        (GtkWidget *widget, gpointer user_data);
void on_BN_OK_clicked         (GtkWidget *widget, gpointer user_data);
void on_BN_refresh_clicked    (GtkWidget *widget, gpointer user_data);
void on_MN_EX1000_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX1010_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX2000_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX2010_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX2020_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX2030_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX2040_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX2050_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX3000_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX3010_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX3020_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX3030_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX3040_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX3050_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX4000_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX4010_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX4020_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX5000_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX5010_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX5020_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX5030_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX5040_activate    (GtkWidget *widget, gpointer user_data);
void on_MN_EX6_about_activate (GtkWidget *widget, gpointer user_data);
void on_WN_ex_destroy         (GtkWidget *widget, gpointer user_data);
void on_MN_salir_activate     (GtkWidget *widget, gpointer user_data);
void Read_Only_Fields         ();

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;
int N_materias;
int N_personas;
int N_ejercicios;
int N_preguntas;
int N_esquemas;
int N_pre_examenes;
int N_examenes;
int N_evaluaciones;
char decoration_color[200];

/***************/
/* M A I N ( ) */
/***************/
int main(int argc, char *argv[]) 
{
  struct timeval T;
  struct timezone {
                   int     tz_minuteswest; /* of Greenwich */
                   int     tz_dsttime;     /* type of dst correction to apply */
                  } Z;
  int k;

  gettimeofday(&T, &Z);
  srand (T.tv_usec);

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX-T.glade", &error))
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

	  /* Some fields are READ ONLY */
	  Read_Only_Fields ();

	  /* The magic of color...  */
	  Interface_Coloring ();

          gtk_widget_show (window1);  
          Init_Fields ();              

          /* start the event loop */
          gtk_main ();
	 }
     }

   return 0;
}

void Connect_Widgets ()
{
  window1           = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex"));
  window2           = GTK_WIDGET (gtk_builder_get_object (builder, "AB_ex"));
  window3           = GTK_WIDGET (gtk_builder_get_object (builder, "WN_EX_no_disponible"));
  window4           = GTK_WIDGET (gtk_builder_get_object (builder, "WN_no_materias"));
  window5           = GTK_WIDGET (gtk_builder_get_object (builder, "WN_no_autores"));
  window6           = GTK_WIDGET (gtk_builder_get_object (builder, "WN_no_ejercicios"));
  window7           = GTK_WIDGET (gtk_builder_get_object (builder, "WN_no_preguntas"));
  window8           = GTK_WIDGET (gtk_builder_get_object (builder, "WN_no_esquemas"));
  window9           = GTK_WIDGET (gtk_builder_get_object (builder, "WN_no_pre_examenes"));
  window10          = GTK_WIDGET (gtk_builder_get_object (builder, "WN_no_examenes"));
  window11          = GTK_WIDGET (gtk_builder_get_object (builder, "WN_no_configurado"));

  EN_materias       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materias"));
  EN_autores        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_autores"));
  EN_ejercicios     = GTK_WIDGET (gtk_builder_get_object (builder, "EN_ejercicios"));
  EN_preguntas      = GTK_WIDGET (gtk_builder_get_object (builder, "EN_preguntas"));
  EN_esquemas       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquemas"));
  EN_pre_examenes   = GTK_WIDGET (gtk_builder_get_object (builder, "EN_pre_examenes"));
  EN_examenes       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_examenes"));
  EN_evaluaciones   = GTK_WIDGET (gtk_builder_get_object (builder, "EN_evaluaciones"));
  EN_respaldo       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_respaldo"));
  BN_refresh        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_refresh"));
  FR_no_disponible  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_no_disponible"));
  FR_no_materias    = GTK_WIDGET (gtk_builder_get_object (builder, "FR_no_materias"));
  FR_no_autores     = GTK_WIDGET (gtk_builder_get_object (builder, "FR_no_autores"));
  FR_no_ejercicios  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_no_ejercicios"));
  FR_no_preguntas   = GTK_WIDGET (gtk_builder_get_object (builder, "FR_no_preguntas"));
  FR_no_esquemas    = GTK_WIDGET (gtk_builder_get_object (builder, "FR_no_esquemas"));
  FR_no_configurado = GTK_WIDGET (gtk_builder_get_object (builder, "FR_no_configurado"));

  FR_archivo       = GTK_WIDGET (gtk_builder_get_object (builder, "FR_archivo"));
  IM_archivo       = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_archivo"));
  BN_exit          = GTK_WIDGET (gtk_builder_get_object (builder, "BN_exit"));
  BN_parametros    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_parametros"));
  BN_backup        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_backup"));
  BN_upgrade       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_upgrade"));

  FR_datos1        = GTK_WIDGET (gtk_builder_get_object (builder, "FR_datos1"));
  IM_datos1        = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_datos1"));
  BN_materias      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_materias"));
  BN_objetivos     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_objetivos"));
  BN_autores       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_autores"));
  BN_ejercicios    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ejercicios"));
  BN_preguntas     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_preguntas"));
  BN_recorrer      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_recorrer"));
  BN_copias        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_copias"));
  BN_ligas         = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ligas"));

  FR_examenes      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_examenes"));
  IM_examenes      = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_examenes"));
  BN_esquemas      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_esquemas"));
  BN_sorteo        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_sorteo"));
  BN_preexamenes   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_preexamenes"));
  BN_refinar       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_refinar"));
  BN_examen        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_examen"));
  BN_final         = GTK_WIDGET (gtk_builder_get_object (builder, "BN_final"));

  FR_revision      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_revision"));
  IM_revision      = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_revision"));
  BN_revision      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_revision"));
  BN_analisis      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_analisis"));
  BN_actualizar    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_actualizar"));

  FR_reportes        = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reportes"));
  IM_reportes        = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_reportes"));
  BN_listamaterias   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_listamaterias"));
  BN_listaejercicios = GTK_WIDGET (gtk_builder_get_object (builder, "BN_listaejercicios"));
  BN_listaanalisis   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_listaanalisis"));
  BN_cruce           = GTK_WIDGET (gtk_builder_get_object (builder, "BN_cruce"));
  BN_listatodos      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_listatodos"));

  FR_limpieza            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_limpieza"));
  IM_limpieza            = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_limpieza"));
  BN_borra_examenes      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_borra_examenes"));
  BN_borra_pre_examenes  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_borra_pre_examenes"));
  BN_borra_esquemas      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_borra_esquemas"));

  IM_background     = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_background"));
  LB_version        = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_version"));
}

void Read_Only_Fields         ()
{
  gtk_widget_set_can_focus(EN_materias  ,   FALSE);
  gtk_widget_set_can_focus(EN_autores   ,   FALSE);
  gtk_widget_set_can_focus(EN_ejercicios,   FALSE);
  gtk_widget_set_can_focus(EN_preguntas ,   FALSE);
  gtk_widget_set_can_focus(EN_esquemas  ,   FALSE);
  gtk_widget_set_can_focus(EN_pre_examenes, FALSE);
  gtk_widget_set_can_focus(EN_examenes,     FALSE);
  gtk_widget_set_can_focus(EN_evaluaciones, FALSE);
  gtk_widget_set_can_focus(EN_respaldo,     FALSE);
}

void Interface_Coloring ()
{
  GdkRGBA color;

  Main_decoration(IM_background, decoration_color, parametros.decoracion);
  Asigna_version (decoration_color);

  gdk_rgba_parse (&color, decoration_color);
  gtk_widget_override_background_color(EN_materias,     GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_autores,      GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_ejercicios,   GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_preguntas,    GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_esquemas,     GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_pre_examenes, GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_examenes,     GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_evaluaciones, GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_respaldo,     GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(BN_refresh,      GTK_STATE_NORMAL, &color);

  gdk_rgba_parse (&color, STANDARD_FRAME);
  gtk_widget_override_background_color(FR_no_disponible, GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(FR_no_materias,   GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(FR_no_autores,    GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(FR_no_ejercicios, GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(FR_no_preguntas,  GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(FR_no_esquemas,   GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(FR_no_configurado,GTK_STATE_NORMAL, &color);
  gtk_widget_override_color(FR_archivo,       GTK_STATE_NORMAL, &color);
  gtk_widget_override_color(FR_datos1,        GTK_STATE_NORMAL, &color);
  gtk_widget_override_color(FR_examenes,      GTK_STATE_NORMAL, &color);
  gtk_widget_override_color(FR_revision,      GTK_STATE_NORMAL, &color);
  gtk_widget_override_color(FR_reportes,      GTK_STATE_NORMAL, &color);
  gtk_widget_override_color(FR_limpieza,      GTK_STATE_NORMAL, &color);

  gdk_rgba_parse (&color, "dark goldenrod");
  gtk_widget_override_color(BN_exit,               GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_parametros,         GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_backup,             GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_upgrade,            GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_materias,           GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_objetivos,          GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_autores,            GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_ejercicios,         GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_preguntas,          GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_recorrer,           GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_copias,             GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_ligas,              GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_esquemas,           GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_sorteo,             GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_preexamenes,        GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_refinar,            GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_examen,             GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_final,              GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_revision,           GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_analisis,           GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_actualizar,         GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_listamaterias,      GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_listaejercicios,    GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_listaanalisis,      GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_cruce,              GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_listatodos,         GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_borra_examenes,     GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_borra_pre_examenes, GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_borra_esquemas,     GTK_STATE_ACTIVE, &color);
  gtk_widget_override_background_color(BN_refresh,            GTK_STATE_ACTIVE, &color);

  gdk_rgba_parse (&color, BUTTON_PRELIGHT);
  gtk_widget_override_color(BN_exit,               GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_parametros,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_backup,             GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_upgrade,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_materias,           GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_objetivos,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_autores,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_ejercicios,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_preguntas,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_recorrer,           GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_copias,             GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_ligas,              GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_esquemas,           GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_sorteo,             GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_preexamenes,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_refinar,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_examen,             GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_final,              GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_revision,           GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_analisis,           GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_actualizar,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_listamaterias,      GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_listaejercicios,    GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_listaanalisis,      GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_cruce,              GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_listatodos,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_borra_examenes,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_borra_pre_examenes, GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_borra_esquemas,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_override_background_color(BN_refresh,            GTK_STATE_PRELIGHT, &color);

  gdk_rgba_parse (&color, "red");
}

/************************************/
/* Conteos de principales elementos */
/************************************/
void Init_Fields()
{
  PGresult *res, *res_aux;
  char PG_command[1000];
  char Hilera[200];
  int k;

  res = PQEXEC(DATABASE, "SELECT * from BD_materias where codigo_tema = '     ' and codigo_subtema = '     '");
  N_materias = PQntuples(res);

  if (N_materias == 1)
     sprintf (Hilera,"%7d Materia",  N_materias);
  else
     sprintf (Hilera,"%7d Materias", N_materias);
  gtk_entry_set_text(GTK_ENTRY(EN_materias), Hilera);
  PQclear(res);

  res = PQEXEC(DATABASE, "SELECT * from BD_personas");
  N_personas = PQntuples(res);

  if (N_personas == 1)
     sprintf (Hilera,"%7d Autor",   N_personas);
  else
     sprintf (Hilera,"%7d Autores", N_personas);
  gtk_entry_set_text(GTK_ENTRY(EN_autores), Hilera);
  PQclear(res);

  res = PQEXEC(DATABASE, "SELECT * from BD_ejercicios");
  N_ejercicios = PQntuples(res);

  if (N_ejercicios == 1)
     sprintf (Hilera,"%5d Ejercicio",  N_ejercicios);
  else
     sprintf (Hilera,"%5d Ejercicios", N_ejercicios);
  gtk_entry_set_text(GTK_ENTRY(EN_ejercicios), Hilera);
  PQclear(res);

  res = PQEXEC(DATABASE, "SELECT * from BD_texto_preguntas");
  N_preguntas = PQntuples(res);

  if (N_preguntas == 1)
     sprintf (Hilera,"%5d Pregunta", N_preguntas);
  else
     sprintf (Hilera,"%5d Preguntas", N_preguntas);
  gtk_entry_set_text(GTK_ENTRY(EN_preguntas), Hilera);
  PQclear(res);

  res = PQEXEC(DATABASE, "SELECT * from EX_esquemas");
  N_esquemas = PQntuples(res);

  if (N_esquemas == 1)
     sprintf (Hilera,"%7d Esquema",  N_esquemas);
  else
     sprintf (Hilera,"%7d Esquemas", N_esquemas);
  gtk_entry_set_text(GTK_ENTRY(EN_esquemas), Hilera);
  PQclear(res);

  res = PQEXEC(DATABASE, "SELECT * from EX_pre_examenes");
  N_pre_examenes = PQntuples(res);

  if (PQntuples(res) == 1)
     sprintf (Hilera,"%7d Preexamen",   N_pre_examenes);
  else
     sprintf (Hilera,"%7d Preexámenes", N_pre_examenes);
  gtk_entry_set_text(GTK_ENTRY(EN_pre_examenes), Hilera);
  PQclear(res);

  res = PQEXEC(DATABASE, "SELECT * from EX_examenes");
  N_examenes = PQntuples(res);

  if (PQntuples(res) == 1)
    sprintf (Hilera,"%7d Examen",   N_examenes);
  else
    sprintf (Hilera,"%7d Exámenes", N_examenes);
  gtk_entry_set_text(GTK_ENTRY(EN_examenes), Hilera);
  PQclear(res);

  res = PQEXEC(DATABASE, "SELECT * from EX_examenes_respuestas");
  N_evaluaciones = PQntuples(res);

  if (PQntuples(res) == 1)
    sprintf (Hilera,"%7d Evaluación",   N_evaluaciones);
  else
    sprintf (Hilera,"%7d Evaluaciones",  N_evaluaciones);
  gtk_entry_set_text(GTK_ENTRY(EN_evaluaciones), Hilera);
  PQclear(res);

  sprintf (Hilera,"Respaldo %s",parametros.fecha_respaldo);
  gtk_entry_set_text(GTK_ENTRY(EN_respaldo), Hilera);

  carga_parametros_EXAMINER (&parametros, DATABASE);
  if (!parametros.configurado)
     {
      gtk_widget_set_sensitive (window1, 0);
      gtk_widget_show          (window11);
      }
}

void Fin_de_Programa (GtkWidget *widget, gpointer user_data)
{
   gtk_main_quit ();
   exit (0);
}

void Asigna_version (char * color)
{
   int subversion = 0;
   char version[100];

   sprintf (version,"<span font_desc=\"Georgia 30\" foreground=\"%s\">The Examiner 0.5.%1d</span>", color, subversion);
   gtk_label_set_markup(LB_version,version);
}

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex_destroy (GtkWidget *widget, gpointer user_data) 
{
  Fin_de_Programa (widget, user_data);
}

void on_MN_salir_activate(GtkWidget *widget, gpointer user_data)
{
  Fin_de_Programa (widget, user_data);
}

/******************/
/*  Menú ARCHIVO  */
/******************/

/* PARAMETROS */
void on_MN_EX1000_activate(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   system ("bin/EX1000 &");  /* El programa es ejecutado como un proceso independiente */
}

/* RESPALDOS */
void on_MN_EX1010_activate(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   system ("bin/EX1010 &");  /* El programa es ejecutado como un proceso independiente */
}

/* UPGRADE */
void on_MN_EX1020_activate(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   system ("bin/EX1020 &");  /* El programa es ejecutado como un proceso independiente */
}

/******************/
/*  Menú DATOS    */
/******************/

/* MATERIAS */
void on_MN_EX2000_activate(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   system ("bin/EX2000 &"); /* El programa es ejecutado como un proceso independiente */
}

/* OBJETIVOS */
void on_MN_EX2010_activate(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();

   if (!N_materias)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window4); /* No hay materias */
      }
   else
      {
       system ("bin/EX2010 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* AUTORES Y PROFESORES */
void on_MN_EX2020_activate(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   system ("bin/EX2020 &");  /* El programa es ejecutado como un proceso independiente */
}

/* EJERCICIOS */
void on_MN_EX2030_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_set_sensitive(window1,  0);
   gtk_widget_show (window3); /* Opción no disponible */
#endif

   Init_Fields ();

   if (!N_materias)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window4); /* No hay materias */
      }
   else
      {
       if (!N_personas)
          {
           gtk_widget_set_sensitive(window1,  0);
           gtk_widget_show (window5); /* No hay autores */
          }
       else
	  {
           system ("bin/EX2030 &");  /* El programa es ejecutado como un proceso independiente */
	  }
      }
}

/* PREGUNTAS POR EJERCICIO */
void on_MN_EX2040_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_ejercicios)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window6); /* No hay ejercicios */
      }
   else
      {
       system ("bin/EX2040 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* RECORRER POR PREGUNTAS */
void on_MN_EX2050_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_preguntas)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window7); /* No hay preguntas */
      }
   else
      {
       system ("bin/EX2050 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* COPIAS DE EJERCICIOS */
void on_MN_EX2060_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_ejercicios)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window6); /* No hay ejercicios */
      }
   else
      {
       system ("bin/EX2060 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* LIGAS DE EJERCICIOS */
void on_MN_EX2070_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_ejercicios)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window6); /* No hay ejercicios */
      }
   else
      {
       system ("bin/EX2070 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/******************/
/*  Menú EXAMENES */
/******************/

/* ESQUEMAS */
void on_MN_EX3000_activate(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();

   if (!N_preguntas)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window7); /* No hay preguntas */
      }
   else
      {
       system ("bin/EX3000 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* CAMBIA PARÁMETROS DE SORTEO */
void on_MN_EX3010_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   system ("bin/EX3010 &");  /* El programa es ejecutado como un proceso independiente */
}

/* PRE EXAMENES */
void on_MN_EX3020_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_esquemas)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window8); /* No hay esquemas */
      }
   else
      {
       system ("bin/EX3020 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* REFINAMIENTO DE PREEXAMENES */
void on_MN_EX3030_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_pre_examenes)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window9); /* No hay pre exámenes */
      }
   else
      {
       system ("bin/EX3030 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* PREEXAMENES A EXAMENES */
void on_MN_EX3040_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();
   if (!N_pre_examenes)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window9); /* No hay pre exámenes */
      }
   else
      {
       system ("bin/EX3040 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* IMPRIME EXAMENES */
void on_MN_EX3050_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_examenes)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window10); /* No hay exámenes */
      }
   else
      {
       system ("bin/EX3050 &");  /* El programa es ejecutado como un proceso independiente */
      }
}


/******************/
/*  Menú REVISIÓN */
/******************/
/* GRABA RESPUESTAS CON NOTA PRELIMINAR */
void on_MN_EX4000_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_examenes)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window10); /* No hay exámenes */
      }
   else
      {
       system ("bin/EX4000 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* ANALIZA EXAMENES */
void on_MN_EX4010_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_examenes)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window10); /* No hay exámenes */
      }
   else
      {
       system ("bin/EX4010 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* ACTUALIZA BASE DE DATOS */
void on_MN_EX4020_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_examenes)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window10); /* No hay exámenes */
      }
   else
      {
       system ("bin/EX4020 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* BORRA RESPUESTAS VIEJAS */
void on_MN_EX4030_activate(GtkWidget *widget, gpointer user_data)
{
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);

#if 0
   Init_Fields ();

   if (!N_examenes)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window10); /* No hay exámenes */
      }
   else
      {
       system ("bin/EX4030 &");  /* El programa es ejecutado como un proceso independiente */
      }
#endif
}

/******************/
/*  Menú REPORTES */
/******************/

/* LISTA DE MATERIAS */
void on_MN_EX5000_activate(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();

   if (!N_materias)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window4); /* No hay materias */
      }
   else
      {
       system ("bin/EX5000 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* LISTA DE EJERCICIOS */
void on_MN_EX5010_activate(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();

   if (!N_materias)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window4); /* No hay materias */
      }
   else
      {
       system ("bin/EX5010 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* ANALISIS DE BASE DE DATOS */
void on_MN_EX5020_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();
   if (!N_preguntas)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window7); /* No hay preguntas */
      }
   else
      {
       system ("bin/EX5020 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* Análisis de 2 variables cruzadas - Lista de preguntas */
void on_MN_EX5030_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_preguntas)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window7); /* No hay preguntas */
      }
   else
      {
       system ("bin/EX5030 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* LISTADO DE ESQUEMAS, PREEXAMENES y EXAMENES */
void on_MN_EX5040_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();
   system ("bin/EX5040 &");  /* El programa es ejecutado como un proceso independiente */
}

/******************/
/*  Menú LIMPIEZA */
/******************/
/* BORRA EXAMENES VIEJOS */
void on_MN_EX6000_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_examenes)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window10); /* No hay exámenes */
      }
   else
      {
       system ("bin/EX6000 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* BORRA PREEXAMENES VIEJOS */
void on_MN_EX6010_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_pre_examenes)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window9); /* No hay preexámenes */
      }
   else
      {
       system ("bin/EX6010 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/* BORRA ESQUEMAS VIEJOS */
void on_MN_EX6020_activate(GtkWidget *widget, gpointer user_data)
{
#if 0
   gtk_widget_show (window3); /* Opción no disponible */
   gtk_widget_set_sensitive(window1,  0);
#endif

   Init_Fields ();

   if (!N_esquemas)
      {
       gtk_widget_set_sensitive(window1,  0);
       gtk_widget_show (window8); /* No hay esquemas */
      }
   else
      {
       system ("bin/EX6020 &");  /* El programa es ejecutado como un proceso independiente */
      }
}

/******************/
/*  Menú AYUDA    */
/******************/
void on_MN_EX6_about_activate(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   gtk_widget_show (window2);
}

void on_AB_ex_response(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   gtk_widget_hide (window2);
}

/**********************************/
/*  Botón de Opción no disponible */
/**********************************/
void on_BN_OK_clicked(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   gtk_widget_hide (window3);
   gtk_widget_set_sensitive(window1,  1);
}

/*****************************/
/*  Botón de NO hay materias */
/*****************************/
void on_BN_OK1_clicked(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   gtk_widget_hide (window4);
   gtk_widget_set_sensitive(window1,  1);
}

/*****************************/
/*  Botón de NO hay autores  */
/*****************************/
void on_BN_OK2_clicked(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   gtk_widget_hide (window5);
   gtk_widget_set_sensitive(window1,  1);
}

/*******************************/
/*  Botón de NO hay ejercicios */
/*******************************/
void on_BN_OK3_clicked(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   gtk_widget_hide (window6);
   gtk_widget_set_sensitive(window1,  1);
}

/*******************************/
/*  Botón de NO hay preguntas */
/*******************************/
void on_BN_OK4_clicked(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   gtk_widget_hide (window7);
   gtk_widget_set_sensitive(window1,  1);
}

/*******************************/
/*  Botón de NO hay esquemas */
/*******************************/
void on_BN_OK5_clicked(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   gtk_widget_hide (window8);
   gtk_widget_set_sensitive(window1,  1);
}

/********************************/
/*  Botón de NO hay preexamenes */
/********************************/
void on_BN_OK6_clicked(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   gtk_widget_hide (window9);
   gtk_widget_set_sensitive(window1,  1);
}

/********************************/
/*  Botón de NO hay examenes */
/********************************/
void on_BN_OK7_clicked(GtkWidget *widget, gpointer user_data)
{
   Init_Fields ();
   gtk_widget_hide (window10);
   gtk_widget_set_sensitive(window1,  1);
}

/********************************/
/*  Botón de NO configuración   */
/********************************/
void on_BN_OK8_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (GTK_WIDGET(window11));

  system ("bin/EX1000");

  gtk_widget_set_sensitive (window1, 1);

  Init_Fields ();
}

/**************************************/
/*  Botón para refrescar Estadísticas */
/**************************************/
void on_BN_refresh_clicked(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color;

  Main_decoration(IM_background, decoration_color, parametros.decoracion);
  Asigna_version (decoration_color);

  gdk_rgba_parse (&color, decoration_color);
  gtk_widget_override_background_color(EN_materias,     GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_autores,      GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_ejercicios,   GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_preguntas,    GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_esquemas,     GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_pre_examenes, GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_examenes,     GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_evaluaciones, GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(EN_respaldo,     GTK_STATE_NORMAL, &color);
  gtk_widget_override_background_color(BN_refresh,      GTK_STATE_NORMAL, &color);

  Init_Fields ();
}

void on_FR_archivo_enter(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color;
  /*
  gtk_widget_set_sensitive (FR_datos1,   0);
  gtk_widget_set_sensitive (FR_examenes, 0);
  gtk_widget_set_sensitive (FR_revision, 0);
  gtk_widget_set_sensitive (FR_reportes, 0);
  */

  gdk_rgba_parse (&color, decoration_color);
  gtk_widget_override_background_color(FR_archivo, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_archivo, ".imagenes/gold.jpg");
}

void on_FR_archivo_leave(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color = {0.0, 0.0, 0.0, 0.0};
  /*
  gtk_widget_set_sensitive (FR_datos1,   1);
  gtk_widget_set_sensitive (FR_examenes, 1);
  gtk_widget_set_sensitive (FR_revision, 1);
  gtk_widget_set_sensitive (FR_reportes, 1);
  */
  //gdk_rgba_parse (&color, "gold");
  gtk_widget_override_background_color(FR_archivo, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_archivo, ".imagenes/empty.png");
}

void on_FR_datos1_enter(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color;
  /*
  gtk_widget_set_sensitive (FR_archivo,  0);
  gtk_widget_set_sensitive (FR_examenes, 0);
  gtk_widget_set_sensitive (FR_revision, 0);
  gtk_widget_set_sensitive (FR_reportes, 0);
  */

  gdk_rgba_parse (&color, decoration_color);
  gtk_widget_override_background_color(FR_datos1, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_datos1, ".imagenes/gold.jpg");
}

void on_FR_datos1_leave(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color = {0.0, 0.0, 0.0, 0.0};
  /*
  gtk_widget_set_sensitive (FR_archivo,  1);
  gtk_widget_set_sensitive (FR_examenes, 1);
  gtk_widget_set_sensitive (FR_revision, 1);
  gtk_widget_set_sensitive (FR_reportes, 1);
  */
  gtk_widget_override_background_color(FR_datos1, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_datos1, ".imagenes/empty.png");
}

void on_FR_examenes_enter(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color;
  /*
  gtk_widget_set_sensitive (FR_archivo,  0);
  gtk_widget_set_sensitive (FR_datos1,   0);
  gtk_widget_set_sensitive (FR_examenes, 0);
  gtk_widget_set_sensitive (FR_revision, 0);
  gtk_widget_set_sensitive (FR_reportes, 0);
  */

  gdk_rgba_parse (&color, decoration_color);
  gtk_widget_override_background_color(FR_examenes, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_examenes, ".imagenes/gold.jpg");
}

void on_FR_examenes_leave(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color = {0.0, 0.0, 0.0, 0.0};
  /*
  gtk_widget_set_sensitive (FR_archivo,  1);
  gtk_widget_set_sensitive (FR_datos1,   1);
  gtk_widget_set_sensitive (FR_examenes, 1);
  gtk_widget_set_sensitive (FR_revision, 1);
  gtk_widget_set_sensitive (FR_reportes, 1);
  */

  gtk_widget_override_background_color(FR_examenes, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_examenes, ".imagenes/empty.png");
}

void on_FR_revision_enter(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color;
  /*
  gtk_widget_set_sensitive (FR_archivo,  0);
  gtk_widget_set_sensitive (FR_datos1,   0);
  gtk_widget_set_sensitive (FR_examenes, 0);
  gtk_widget_set_sensitive (FR_revision, 0);
  gtk_widget_set_sensitive (FR_reportes, 0);
  */

  gdk_rgba_parse (&color, decoration_color);
  gtk_widget_override_background_color(FR_revision, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_revision, ".imagenes/gold.jpg");
}

void on_FR_revision_leave(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color = {0.0, 0.0, 0.0, 0.0};
  /*
  gtk_widget_set_sensitive (FR_archivo,  1);
  gtk_widget_set_sensitive (FR_datos1,   1);
  gtk_widget_set_sensitive (FR_examenes, 1);
  gtk_widget_set_sensitive (FR_revision, 1);
  gtk_widget_set_sensitive (FR_reportes, 1);
  */
  gtk_widget_override_background_color(FR_revision, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_revision, ".imagenes/empty.png");
}

void on_FR_reportes_enter(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color;
  /*
  gtk_widget_set_sensitive (FR_archivo,  0);
  gtk_widget_set_sensitive (FR_datos1,   0);
  gtk_widget_set_sensitive (FR_examenes, 0);
  gtk_widget_set_sensitive (FR_revision, 0);
  */

  gdk_rgba_parse (&color, decoration_color);
  gtk_widget_override_background_color(FR_reportes, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_reportes, ".imagenes/gold.jpg");
}

void on_FR_reportes_leave(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color = {0.0, 0.0, 0.0, 0.0};
  /*
  gtk_widget_set_sensitive (FR_archivo,  1);
  gtk_widget_set_sensitive (FR_datos1,   1);
  gtk_widget_set_sensitive (FR_examenes, 1);
  gtk_widget_set_sensitive (FR_revision, 1);
  */
  gtk_widget_override_background_color(FR_reportes, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_reportes, ".imagenes/empty.png");
}

void on_FR_limpieza_enter(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color;
  /*
  gtk_widget_set_sensitive (FR_archivo,  0);
  gtk_widget_set_sensitive (FR_datos1,   0);
  gtk_widget_set_sensitive (FR_examenes, 0);
  gtk_widget_set_sensitive (FR_revision, 0);
  */

  gdk_rgba_parse (&color, decoration_color);
  gtk_widget_override_background_color(FR_limpieza, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_limpieza, ".imagenes/gold.jpg");
}

void on_FR_limpieza_leave(GtkWidget *widget, gpointer user_data)
{
  GdkRGBA color = {0.0, 0.0, 0.0, 0.0};
    /*
  gtk_widget_set_sensitive (FR_archivo,  1);
  gtk_widget_set_sensitive (FR_datos1,   1);
  gtk_widget_set_sensitive (FR_examenes, 1);
  gtk_widget_set_sensitive (FR_revision, 1);
  */
  gtk_widget_override_background_color(FR_limpieza, GTK_STATE_NORMAL, &color);
  //gtk_image_set_from_file (IM_limpieza, ".imagenes/empty.png");
}
