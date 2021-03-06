/*
Compilar con

cc -o EX3050 EX3050.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic

*/
/*******************************************/
/*  EX3050:                                */
/*                                         */
/*    Imprime versiones de un examen       */
/*    The Examiner 0.0                     */
/*    7 de Agosto 2011                     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*                                         */
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    10 de Marzo del 2012                 */
/*******************************************/
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
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
/* Postgres stuff          */
/***************************/
PGconn	 *DATABASE;
PGresult *res, *res_aux;

/*************/
/* GTK stuff */
/*************/
GtkBuilder      *builder; 
GError          * error;
GtkWidget       *window1                   = NULL;
GtkWidget       *window2                   = NULL;
GtkWidget       *window3                   = NULL;
GtkWidget       *window4                   = NULL;

GtkWidget *EB_examen   = NULL;
GtkWidget *FR_examen   = NULL;

GtkSpinButton   *SP_examen                 = NULL;
GtkWidget       *EN_descripcion            = NULL;
GtkWidget       *EN_materia                = NULL;
GtkWidget       *EN_materia_descripcion    = NULL;
GtkWidget       *EN_version                = NULL;
GtkToggleButton *CK_instrucciones          = NULL;
GtkWidget       *FR_instrucciones          = NULL;
GtkTextView     *TV_instrucciones          = NULL;
GtkTextBuffer   *buffer_TV_instrucciones   = NULL;
GtkToggleButton *CK_juramento              = NULL;
GtkWidget       *FR_juramento              = NULL;
GtkTextView     *TV_juramento              = NULL;
GtkTextBuffer   *buffer_TV_juramento       = NULL;
GtkToggleButton *CK_watermark              = NULL;
GtkWidget       *LB_watermark              = NULL;
GtkFileChooser  *FC_watermark              = NULL;
GtkToggleButton *CK_apendice               = NULL;
GtkWidget       *LB_apendice               = NULL;
GtkFileChooser  *FC_apendice               = NULL;
GtkWidget       *FR_botones                = NULL;
GtkWidget       *FR_print                  = NULL;
GtkWidget       *EN_institucion            = NULL;
GtkWidget       *EN_escuela                = NULL;
GtkWidget       *EN_programa               = NULL;
GtkComboBox     *CB_profesor;
GtkWidget       *EN_size                   = NULL;
GtkToggleButton *RB_1_columna              = NULL;
GtkToggleButton *RB_2_columna              = NULL;
GtkToggleButton *RB_escribir               = NULL;
GtkToggleButton *RB_relleno                = NULL;
GtkToggleButton *RB_arriba                 = NULL;
GtkToggleButton *RB_abajo                  = NULL;
GtkWidget       *SC_size                   = NULL;
GtkWidget       *SC_versiones              = NULL;
GtkCalendar     *CAL_fecha                 = NULL;
GtkWidget       *BN_undo                   = NULL;
GtkWidget       *BN_print                  = NULL;
GtkWidget       *BN_print_multi            = NULL;
GtkWidget       *BN_terminar               = NULL;
GtkWidget       *BN_ok                     = NULL;
GtkWidget       *BN_ok1                    = NULL;
GtkWidget       *BN_extend_juramento       = NULL;
GtkWidget       *BN_extend_instrucciones   = NULL;

GtkWidget *FR_destino_reporte     = NULL;
GtkWidget *EB_destino_reporte     = NULL;
GtkLabel  *LB_destino_reporte     = NULL;
GtkLabel  *LB_destino_LATEX       = NULL;
GtkLabel  *LB_solucion            = NULL;
GtkLabel  *LB_solucion_LATEX      = NULL;

GtkWidget *FR_destinos_reporte    = NULL;
GtkWidget *EB_destinos_reporte    = NULL;
GtkLabel  *LB_destinos_reporte    = NULL;
GtkLabel  *LB_destinos_LATEX      = NULL;
GtkLabel  *LB_soluciones          = NULL;
GtkLabel  *LB_soluciones_LATEX    = NULL;

GtkWidget *EB_reporte             = NULL;
GtkWidget *FR_reporte             = NULL;
GtkWidget *PB_reporte             = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Activa_widgets_Inicio ();
void Agregar_Instrucciones (FILE * Archivo_Latex);
void Agregar_Juramento     (FILE * Archivo_Latex);
void Arma_nombres (int k);
void Caja_Letras (FILE * Archivo_Latex);
void Caja_Nota(FILE * Archivo_Latex);
void Caja_Opciones (FILE * Archivo_Latex);
void Cambia_Size ();
void Cambia_Version ();
void Cambio_Descripcion(GtkWidget *widget, gpointer user_data);
void Cambio_Escuela(GtkWidget *widget, gpointer user_data);
void Cambio_Examen();
void Cambio_Institucion(GtkWidget *widget, gpointer user_data);
void Cambio_Programa(GtkWidget *widget, gpointer user_data);
void Carga_Profesor (char * profesor);
void Codifica_Informacion (FILE * Archivo_Latex);
void Connect_Widgets();
void Desactiva_widgets_Inicio ();
void Establece_Directorios (char * Directorio_Enunciados, char * Directorio_Enunciados_Latex,
                            char * Directorio_Soluciones, char * Directorio_Soluciones_Latex,
                            gchar * materia, int year, int month, int day);
void Extrae_codigo (char * hilera, char * codigo, int N);
void Fin_de_Programa (GtkWidget *widget, gpointer user_data);
void Fin_Ventana (GtkWidget *widget, gpointer user_data);
void grafico_frecuencias(FILE * Archivo_Latex, int * frecuencias);
void Headers_Footers (FILE * Archivo_Latex, int day, int month, int year,
                      char * descripcion, char * materia_descripcion, char * escuela, char * institucion, char * programa);
void Imprime_Enunciado (char * Directorio_Enunciados, char * Directorio_Enunciados_Latex, char * ruta_destino, char * ruta_latex, long double base, long double size);
void Imprime_pregunta (int i, PGresult * res, FILE * Archivo_Latex,char * prefijo, int N_columnas);
void Imprime_Solucion (char * Directorio_Soluciones, char * Directorio_Soluciones_Latex, char * ruta_destino, char * ruta_latex, long double base, long double size);
void Imprime_solo_una_version ();
void Imprime_todas_las_Versiones ();
void Imprime_Version_Actual (char * Directorio_Enunciados, char * Directorio_Enunciados_Latex, char * Directorio_Soluciones, char * Directorio_Soluciones_Latex, char * ruta_enunciado, char * ruta_enunciado_LATEX, char * ruta_solucion, char * ruta_solucion_LATEX, long double base, long double size);
void Init_Fields();
void Interface_Coloring ();
void Invisible_widgets ();
int main(int argc, char *argv[]) ;
void on_EN_descripcion_activate(GtkWidget *widget, gpointer user_data);
void on_EN_escuela_activate(GtkWidget *widget, gpointer user_data);
void on_EN_institucion_activate(GtkWidget *widget, gpointer user_data);
void on_EN_programa_activate(GtkWidget *widget, gpointer user_data);
void on_BN_print_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_print_multi_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked(GtkWidget *widget, gpointer user_data);
void on_CK_apendice_toggled (GtkWidget *widget, gpointer user_data);
void on_CK_instrucciones_toggled (GtkWidget *widget, gpointer user_data);
void on_CK_juramento_toggled (GtkWidget *widget, gpointer user_data);
void on_CK_watermark_toggled (GtkWidget *widget, gpointer user_data);
void on_SC_size_value_changed(GtkWidget *widget, gpointer user_data);
void on_SC_versiones_value_changed(GtkWidget *widget, gpointer user_data);
void on_SP_examen_activate(GtkWidget *widget, gpointer user_data);
void on_WN_ex3050_destroy (GtkWidget *widget, gpointer user_data);
int  Pone_Circulos (FILE * Archivo_Latex, int j);
void Pone_Letras (FILE * Archivo_Latex, int j);
void Primera_Pagina_Examen (FILE * Archivo_Latex, char * codigo, char * version, char * institucion,
                            char * escuela, char * materia_descripcion, char * descripcion, int day, int month, int year);
void Primera_Pagina_Examen_CV (FILE * Archivo_Latex, char * codigo, char * version, char * institucion,
                               char * escuela, char * materia_descripcion, char * descripcion, int day, int month, int year);
void Primera_Pagina_Examen_Escribir (FILE * Archivo_Latex, char * codigo, char * version, char * institucion,
				     char * escuela, char * materia_descripcion, char * descripcion, int day, int month, int year);
void Quita_espacios (char * hilera);
void Read_Only_Fields ();
void Toggle_apendice(GtkWidget *widget, gpointer user_data);
void Toggle_instrucciones(GtkWidget *widget, gpointer user_data);
void Toggle_juramento(GtkWidget *widget, gpointer user_data);

/***************************/
/* Globales y Definiciones */
/***************************/
//#define MIN(a,b) (((a) > (b))?(b):(a))
struct PARAMETROS_EXAMINER parametros;

#define EXAMEN_SIZE              6
#define DESCRIPCION_SIZE         201

int Numero_Preguntas = 0;
int N_versiones      = 0;
int N_autores        = 0;

#define LATEX_SIZES 10
char * Latex_Sizes[LATEX_SIZES] = {"\\tiny", "\\scriptsize", "\\footnotesize", "\\small","\\normalsize","\\large","\\Large","\\LARGE","\\huge","\\Huge "};

char Enunciado[30], Solucion[30];

/*----------------------------------------------------------------------------*/
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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX3050.glade", &error))
         {
          g_warning ("%s\n", error->message);
          g_error_free (error);
         }
      else
	if (!Connect_Latex_Window (builder, &error, parametros))
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
            gtk_main();
	   }
     }

  return 0;
}

void Connect_Widgets()
{
  window1 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex3050"));
  window2 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_destino_reporte"));
  window3 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_destinos_reporte"));
  window4 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_reporte"));

  EB_examen = GTK_WIDGET (gtk_builder_get_object (builder, "EB_examen"));
  FR_examen = GTK_WIDGET (gtk_builder_get_object (builder, "FR_examen"));

  SP_examen                 = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_examen"));
  EN_descripcion            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));
  EN_materia                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia"));
  EN_materia_descripcion    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia_descripcion"));
  EN_version                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_version"));
  EN_institucion            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_institucion"));
  EN_escuela                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_escuela"));
  EN_programa               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_programa"));
  CB_profesor               = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder,  "CB_profesor"));
  EN_size                   = GTK_WIDGET (gtk_builder_get_object (builder, "EN_size"));
  CK_instrucciones          = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_instrucciones"));
  FR_instrucciones          = GTK_WIDGET (gtk_builder_get_object (builder, "FR_instrucciones"));
  TV_instrucciones          = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_instrucciones"));
  CK_juramento              = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_juramento"));
  FR_juramento              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_juramento"));
  TV_juramento              = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_juramento"));
  CK_watermark              = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_watermark"));
  LB_watermark              = GTK_WIDGET (gtk_builder_get_object (builder, "LB_watermark"));
  FC_watermark              = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_watermark"));
  CK_apendice               = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_apendice"));
  LB_apendice               = GTK_WIDGET (gtk_builder_get_object (builder, "LB_apendice"));
  FC_apendice               = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_apendice"));
  RB_1_columna              = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_1_columna"));
  RB_2_columna              = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_2_columna"));
  RB_escribir               = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_escribir"));
  RB_relleno                = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_relleno"));
  RB_arriba                 = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_arriba"));
  RB_abajo                  = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_abajo"));
  FR_botones                = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  FR_print                  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_print"));
  CAL_fecha                 = (GtkCalendar *) GTK_WIDGET (gtk_builder_get_object (builder, "CAL_fecha"));
  SC_size                   = GTK_WIDGET (gtk_builder_get_object (builder, "SC_size"));
  SC_versiones              = GTK_WIDGET (gtk_builder_get_object (builder, "SC_versiones"));
  BN_print_multi            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print_multi"));
  BN_print                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_undo                   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_terminar               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_ok                     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
  BN_ok1                    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok1"));
  BN_extend_juramento       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_juramento"));
  BN_extend_instrucciones   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_instrucciones"));
  buffer_TV_instrucciones   = gtk_text_view_get_buffer(TV_instrucciones);
  buffer_TV_juramento       = gtk_text_view_get_buffer(TV_juramento);

  EB_destino_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "EB_destino_reporte"));
  FR_destino_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "FR_destino_reporte"));
  LB_destino_reporte = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_destino_reporte"));
  LB_destino_LATEX   = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_destino_LATEX"));
  LB_solucion        = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_solucion"));
  LB_solucion_LATEX  = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_solucion_LATEX"));

  EB_destinos_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "EB_destinos_reporte"));
  FR_destinos_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "FR_destinos_reporte"));
  LB_destinos_reporte = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_destinos_reporte"));
  LB_destinos_LATEX   = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_destinos_LATEX"));
  LB_soluciones       = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_soluciones"));
  LB_soluciones_LATEX = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_soluciones_LATEX"));

  EB_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "EB_reporte"));
  FR_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reporte"));
  PB_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "PB_reporte"));
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_materia               , FALSE);
  gtk_widget_set_can_focus(EN_materia_descripcion   , FALSE);
  gtk_widget_set_can_focus(EN_version               , FALSE);
  gtk_widget_set_can_focus(EN_size                  , FALSE);
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,        GTK_STATE_NORMAL, &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_examen, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_examen, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_instrucciones, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_juramento,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_print,     GTK_STATE_NORMAL, &color);

  gdk_color_parse (SPECIAL_AREA_1, &color);
  gtk_widget_modify_bg(GTK_WIDGET(CAL_fecha), GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_examen), GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_descripcion, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_institucion, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_escuela,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_programa,    GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print_multi, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,    GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok1,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_juramento,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_instrucciones, GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,       GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print_multi, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,          GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok1,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_juramento,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_instrucciones, GTK_STATE_ACTIVE, &color);

  gdk_color_parse ("white", &color);
  gtk_widget_modify_fg(GTK_WIDGET(CAL_fecha), GTK_STATE_NORMAL, &color);

  gdk_color_parse (IMPORTANT_WINDOW, &color);
  gtk_widget_modify_bg(EB_destino_reporte,  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_destinos_reporte, GTK_STATE_NORMAL, &color);

  gdk_color_parse (IMPORTANT_FR, &color);
  gtk_widget_modify_bg(FR_destino_reporte,    GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_destinos_reporte,   GTK_STATE_NORMAL,   &color);


  gdk_color_parse (PROCESSING_WINDOW, &color);
  gtk_widget_modify_bg(EB_reporte,  GTK_STATE_NORMAL,   &color); 

  gdk_color_parse (PROCESSING_FR, &color);
  gtk_widget_modify_bg(FR_reporte,            GTK_STATE_NORMAL, &color);

  gdk_color_parse (PROCESSING_PB, &color);
  gtk_widget_modify_bg(GTK_WIDGET(PB_reporte),  GTK_STATE_PRELIGHT, &color);
}

void Init_Fields()
{
   char Examen [EXAMEN_SIZE];
   char hilera [100];
   time_t curtime;
   struct tm *loctime;
   int i, Last, month, year, day;

   /* Get the current time.  */
   curtime = time (NULL);
   /* Convert it to local time representation.  */
   loctime = localtime (&curtime);

   /* Recupera el Examen con código más alto */
   res = PQEXEC(DATABASE, "SELECT codigo_examen from EX_examenes order by codigo_examen DESC limit 1");
   Last = 0;
   if (PQntuples(res)) Last = atoi (PQgetvalue (res, 0, 0));
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

   Numero_Preguntas = N_versiones = 0;
   sprintf (hilera,"%d", Numero_Preguntas);

   gtk_entry_set_text(GTK_ENTRY(EN_descripcion),            "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia),                "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion),    "\0");

   gtk_range_set_range (GTK_RANGE(SC_versiones),  (gdouble) 0.0, (gdouble) 1.0);
   gtk_range_set_value (GTK_RANGE(SC_versiones),  (gdouble) 0.0);

   gtk_range_set_range (GTK_RANGE(SC_size),  (gdouble) -4.0, (gdouble) 5.0);
   gtk_range_set_value (GTK_RANGE(SC_size),  (gdouble) 0.0);
 
   gtk_entry_set_text(GTK_ENTRY(EN_institucion), parametros.Institucion);
   gtk_entry_set_text(GTK_ENTRY(EN_escuela),     parametros.Escuela);

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

   gtk_entry_set_text(GTK_ENTRY(EN_programa),    parametros.Programa);
   gtk_entry_set_text(GTK_ENTRY(EN_version),     "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_size),        "\0");

   gtk_toggle_button_set_active(CK_instrucciones, TRUE);
   gtk_text_buffer_set_text(buffer_TV_instrucciones, parametros.Instrucciones, -1);

   gtk_toggle_button_set_active(CK_juramento,     TRUE);
   gtk_text_buffer_set_text(buffer_TV_juramento, parametros.Juramento, -1);

   gtk_toggle_button_set_active(CK_watermark,     FALSE);

   strftime (hilera, 100, "%m", loctime);
   month = atoi(hilera)-1;
   strftime (hilera, 100, "%Y", loctime);
   year  = atoi(hilera);
   strftime (hilera, 100, "%d", loctime);
   day   = atoi(hilera);

   gtk_calendar_select_month(CAL_fecha, month, year);
   gtk_calendar_select_day  (CAL_fecha, day );
   gtk_toggle_button_set_active(RB_2_columna, TRUE);
   gtk_toggle_button_set_active(RB_escribir,  TRUE);
   gtk_toggle_button_set_active(RB_abajo,     TRUE);

   Desactiva_widgets_Inicio ();
   Activa_widgets_Inicio    ();
   Invisible_widgets        ();

   /* Initial focus */
   if (Last > 1)
      gtk_widget_grab_focus (GTK_WIDGET(SP_examen));
   else
      Cambio_Examen ();
}

void Desactiva_widgets_Inicio ()
{
   gtk_widget_set_sensitive(EN_materia                  , 0);
   gtk_widget_set_sensitive(EN_materia_descripcion      , 0);
   gtk_widget_set_sensitive(EN_version                  , 0);
   gtk_widget_set_sensitive(EN_size                     , 0);
   gtk_widget_set_sensitive(EN_institucion              , 0);
   gtk_widget_set_sensitive(EN_escuela                  , 0);
   gtk_widget_set_sensitive(EN_programa                 , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CB_profesor)     , 0);
   gtk_widget_set_sensitive(EN_descripcion              , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CAL_fecha)       , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CK_instrucciones), 0);
   gtk_widget_set_sensitive(FR_instrucciones            , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CK_juramento)    , 0);
   gtk_widget_set_sensitive(FR_juramento                , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CK_watermark)    , 0);
   gtk_widget_set_sensitive(LB_watermark                , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CK_apendice)     , 0);
   gtk_widget_set_sensitive(LB_apendice                 , 0);
   gtk_widget_set_sensitive(SC_size                     , 0);
   gtk_widget_set_sensitive(SC_versiones                , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(RB_1_columna)    , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(RB_2_columna)    , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(RB_escribir)     , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(RB_relleno)      , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(RB_arriba)       , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(RB_abajo)        , 0);
   gtk_widget_set_sensitive(BN_print_multi              , 0);
   gtk_widget_set_sensitive(BN_print                    , 0);
   gtk_widget_set_sensitive(BN_extend_juramento         , 0);
   gtk_widget_set_sensitive(BN_extend_instrucciones     , 0);
}

void Activa_widgets_Inicio ()
{
  gtk_widget_set_sensitive(GTK_WIDGET(SP_examen), 1);
  gtk_widget_set_sensitive(BN_undo              , 1);
}

void Invisible_widgets ()
{
  gtk_widget_set_visible (GTK_WIDGET(FC_watermark),     FALSE);
  gtk_widget_set_visible (GTK_WIDGET(FC_apendice),      FALSE);
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

void Cambio_Examen()
{
  long int k;
  char examen [20];
  char hilera      [200];
  char Descripcion [2*DESCRIPCION_SIZE]         = "*** Examen no está registrado ***";
  char PG_command  [2000];
  guint year, month, day;

  k = (long int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05ld", k);

  sprintf (PG_command,"SELECT codigo_examen, descripcion, pre_examen, profesor, EX_examenes.materia, institucion, escuela, programa, Year, Month, Day, tiene_instrucciones, instrucciones, tiene_juramento, juramento, esquema, descripcion_pre_examen, descripcion_esquema, descripcion_materia from EX_examenes, EX_pre_examenes, EX_esquemas, BD_materias, BD_personas where codigo_examen = '%s' and pre_examen = codigo_pre_examen and esquema = codigo_esquema and EX_examenes.materia = codigo_materia and codigo_tema = '          ' and codigo_subtema = '          ' and profesor = codigo_persona", examen);

  res = PQEXEC(DATABASE, PG_command);

  if (PQntuples(res))
     {
      gtk_widget_set_sensitive(GTK_WIDGET(SP_examen), 0);

      gtk_widget_set_sensitive(EN_materia               , 1);
      gtk_widget_set_sensitive(EN_materia_descripcion   , 1);
      gtk_widget_set_sensitive(EN_institucion           , 1);
      gtk_widget_set_sensitive(EN_escuela               , 1);
      gtk_widget_set_sensitive(EN_programa              , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(CB_profesor)  , 1);
      gtk_widget_set_sensitive(EN_descripcion           , 1);
      gtk_widget_set_sensitive(EN_version               , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(CAL_fecha)    , 1);
      gtk_widget_set_sensitive(EN_size                  , 1);
      gtk_widget_set_sensitive(FR_instrucciones         , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(CK_instrucciones), 1);
      gtk_widget_set_sensitive(GTK_WIDGET(CK_juramento)    , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(CK_watermark)    , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(CK_apendice)     , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(RB_1_columna)    , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(RB_2_columna)    , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(RB_escribir)     , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(RB_relleno)      , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(RB_arriba)       , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(RB_abajo)        , 1);

      gtk_widget_set_sensitive(BN_print,       1);
      gtk_widget_set_sensitive(BN_print_multi, 1);
      gtk_widget_set_sensitive(BN_extend_juramento         , 1);
      gtk_widget_set_sensitive(BN_extend_instrucciones     , 1);

      sprintf (PG_command,"SELECT DISTINCT version from EX_examenes_preguntas where examen = '%s'", examen);
      res_aux = PQEXEC(DATABASE, PG_command);
      N_versiones = PQntuples(res_aux);
      PQclear(res_aux);

      sprintf (PG_command,"SELECT * from EX_examenes_preguntas where examen = '%s'", examen);
      res_aux = PQEXEC(DATABASE, PG_command);
      Numero_Preguntas = PQntuples(res_aux);
      Numero_Preguntas = Numero_Preguntas/N_versiones;
      PQclear(res_aux);

      strcpy (Descripcion,                                     PQgetvalue (res, 0, 1));

      Carga_Profesor (PQgetvalue (res, 0, 3));

      gtk_entry_set_text(GTK_ENTRY(EN_materia),                PQgetvalue (res, 0, 4));
      gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion),    PQgetvalue (res, 0, 18));
      gtk_entry_set_text(GTK_ENTRY(EN_institucion),            PQgetvalue (res, 0, 5));
      gtk_entry_set_text(GTK_ENTRY(EN_escuela),                PQgetvalue (res, 0, 6));
      gtk_entry_set_text(GTK_ENTRY(EN_programa),               PQgetvalue (res, 0, 7));
      gtk_widget_set_sensitive(SC_size, 1);
      gtk_range_set_value (GTK_RANGE(SC_size), (gdouble) -5.0);
      gtk_range_set_value (GTK_RANGE(SC_size), (gdouble)  0.0);
         
      gtk_toggle_button_set_active(RB_2_columna, TRUE);
      gtk_toggle_button_set_active(RB_escribir,  TRUE);
      gtk_toggle_button_set_active(RB_abajo,     TRUE);

      year = atoi(PQgetvalue (res, 0, 8));
      month= atoi(PQgetvalue (res, 0, 9));
      day  = atoi(PQgetvalue (res, 0, 10));

      gtk_calendar_select_month(CAL_fecha, month-1, year);
      gtk_calendar_select_day  (CAL_fecha, day );

      if (*PQgetvalue (res, 0, 11) == 't') 
         gtk_toggle_button_set_active(CK_instrucciones, TRUE);
      else
         gtk_toggle_button_set_active(CK_instrucciones, FALSE);

      if (gtk_toggle_button_get_active(CK_instrucciones))
         {
          gtk_text_buffer_set_text(buffer_TV_instrucciones, PQgetvalue (res, 0, 12), -1);
          gtk_widget_set_sensitive(FR_instrucciones, 1);
         }

      if (*PQgetvalue (res, 0, 13) == 't') 
         gtk_toggle_button_set_active(CK_juramento, TRUE);
      else
         gtk_toggle_button_set_active(CK_juramento, FALSE);

      if (gtk_toggle_button_get_active(CK_juramento))
         {
          gtk_text_buffer_set_text(buffer_TV_juramento, PQgetvalue (res, 0, 14), -1);
          gtk_widget_set_sensitive(FR_juramento, 1);
         }

      if (N_versiones > 1)
	 {
          gtk_widget_set_sensitive(SC_versiones       , 1);
          gtk_range_set_range (GTK_RANGE(SC_versiones), (gdouble) 1.0, (gdouble) N_versiones);
          gtk_range_set_value (GTK_RANGE(SC_versiones), (gdouble) 1.0);
	 }
      else
	 {
	  Arma_nombres (1);
	 }

      gtk_widget_grab_focus   (EN_descripcion);
     }

  PQclear(res);

  gtk_entry_set_text(GTK_ENTRY(EN_descripcion), Descripcion);
}

void Toggle_watermark(GtkWidget *widget, gpointer user_data)
{
  if (gtk_toggle_button_get_active(CK_watermark))
     {
      gtk_widget_set_sensitive(LB_watermark, 1);
      gtk_widget_set_visible  (GTK_WIDGET(FC_watermark), TRUE);
      gtk_widget_grab_focus   (GTK_WIDGET(FC_watermark));
     }
  else
     {
      gtk_widget_set_sensitive(LB_watermark, 0);
      gtk_widget_set_visible  (GTK_WIDGET(FC_watermark), FALSE);
      gtk_widget_grab_focus   (EN_descripcion);
     }
}

void Toggle_apendice(GtkWidget *widget, gpointer user_data)
{
  if (gtk_toggle_button_get_active(CK_apendice))
     {
      gtk_widget_set_sensitive(LB_apendice, 1);
      gtk_widget_set_visible  (GTK_WIDGET(FC_apendice), TRUE);
      gtk_widget_grab_focus   (GTK_WIDGET(FC_apendice));
     }
  else
     {
      gtk_widget_set_sensitive(LB_apendice, 0);
      gtk_widget_set_visible  (GTK_WIDGET(FC_apendice), FALSE);
      gtk_widget_grab_focus   (EN_descripcion);
     }
}

void Toggle_instrucciones(GtkWidget *widget, gpointer user_data)
{
  if (gtk_toggle_button_get_active(CK_instrucciones))
     {
      gtk_text_buffer_set_text(buffer_TV_instrucciones, parametros.Instrucciones, -1);
      gtk_widget_set_sensitive(FR_instrucciones, 1);
      gtk_widget_grab_focus   (GTK_WIDGET(TV_instrucciones));
     }
  else
     {
      gtk_text_buffer_set_text(buffer_TV_instrucciones, "\0", -1);
      gtk_widget_set_sensitive(FR_instrucciones, 0);
      gtk_widget_grab_focus   (EN_descripcion);
     }
}

void Toggle_juramento(GtkWidget *widget, gpointer user_data)
{
  if (gtk_toggle_button_get_active(CK_juramento))
     {
      gtk_text_buffer_set_text(buffer_TV_juramento, parametros.Juramento, -1);
      gtk_widget_set_sensitive(FR_juramento, 1);
      gtk_widget_grab_focus   (GTK_WIDGET(TV_juramento));
     }
  else
     {
      gtk_text_buffer_set_text(buffer_TV_juramento, "\0", -1);
      gtk_widget_set_sensitive(FR_juramento, 0);
      gtk_widget_grab_focus   (EN_descripcion);
     }
}

void Imprime_todas_las_Versiones ()
{
  int i;
  int version_actual;
  char Directorio_Enunciados       [2000];
  char Directorio_Enunciados_Latex [2000];
  char Directorio_Soluciones       [2000];
  char Directorio_Soluciones_Latex [2000];
  char ruta_enunciado              [2000];
  char ruta_enunciado_LATEX        [2000];
  char ruta_solucion               [2000];
  char ruta_solucion_LATEX         [2000];
  char hilera[3000];

  gtk_widget_set_sensitive(window1, 0);
  gtk_widget_show         (window4);
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.0);
  while (gtk_events_pending ()) gtk_main_iteration ();

  version_actual = (int) gtk_range_get_value (GTK_RANGE(SC_versiones));

  for (i=0;i<N_versiones;i++)
      {
       gtk_range_set_value (GTK_RANGE(SC_versiones),  (gdouble) (i+1));
       Imprime_Version_Actual (Directorio_Enunciados, Directorio_Enunciados_Latex, Directorio_Soluciones, Directorio_Soluciones_Latex, ruta_enunciado, ruta_enunciado_LATEX, ruta_solucion, ruta_solucion_LATEX, i *((long double) 1.0/(long double)N_versiones), ((long double) 1.0/(long double)N_versiones));
      }
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 1.0);
  while (gtk_events_pending ()) gtk_main_iteration ();
  gtk_widget_hide (window4);
  gtk_widget_set_sensitive(window1, 1);

  gtk_range_set_value (GTK_RANGE(SC_versiones),  (gdouble) version_actual);

  if (parametros.despliega_reportes)
     {
      sprintf (hilera,"<span foreground=\"black\">%s</span>", Directorio_Enunciados); 
      gtk_label_set_markup(LB_destinos_reporte, hilera);
      sprintf (hilera,"<span foreground=\"black\">%s</span>", Directorio_Enunciados_Latex); 
      gtk_label_set_markup(LB_destinos_LATEX,   hilera);
      sprintf (hilera,"<span foreground=\"black\">%s</span>", Directorio_Soluciones); 
      gtk_label_set_markup(LB_soluciones, hilera);
      sprintf (hilera,"<span foreground=\"black\">%s</span>", Directorio_Soluciones_Latex); 
      gtk_label_set_markup(LB_soluciones_LATEX, hilera);

      gtk_widget_set_sensitive(window1, 0);
      gtk_widget_show (window3);
     }
}

void Imprime_solo_una_version ()
{
  char Directorio_Enunciados       [2000];
  char Directorio_Enunciados_Latex [2000];
  char Directorio_Soluciones       [2000];
  char Directorio_Soluciones_Latex [2000];
  char ruta_enunciado              [2000];
  char ruta_enunciado_LATEX        [2000];
  char ruta_solucion               [2000];
  char ruta_solucion_LATEX         [2000];
  char hilera[3000];

  gtk_widget_set_sensitive(window1, 0);
  gtk_widget_show         (window4);
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.0);
  while (gtk_events_pending ()) gtk_main_iteration ();

  Imprime_Version_Actual (Directorio_Enunciados, Directorio_Enunciados_Latex, Directorio_Soluciones, Directorio_Soluciones_Latex, ruta_enunciado, ruta_enunciado_LATEX, ruta_solucion, ruta_solucion_LATEX, 0.0, 1.0);

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 1.0);
  while (gtk_events_pending ()) gtk_main_iteration ();
  gtk_widget_hide (window4);
  gtk_widget_set_sensitive(window1, 1);

  if (parametros.despliega_reportes)
     {
      sprintf (hilera,"<span foreground=\"black\">%s</span>", ruta_enunciado); 
      gtk_label_set_markup(LB_destino_reporte, hilera);
      sprintf (hilera,"<span foreground=\"black\">%s</span>", ruta_enunciado_LATEX); 
      gtk_label_set_markup(LB_destino_LATEX,   hilera);
      sprintf (hilera,"<span foreground=\"black\">%s</span>", ruta_solucion); 
      gtk_label_set_markup(LB_solucion, hilera);
      sprintf (hilera,"<span foreground=\"black\">%s</span>", ruta_solucion_LATEX); 
      gtk_label_set_markup(LB_solucion_LATEX, hilera);

      gtk_widget_set_sensitive(window1, 0);
      gtk_widget_show (window2);
     }
}

void Imprime_Version_Actual (char * Directorio_Enunciados, char * Directorio_Enunciados_Latex, char * Directorio_Soluciones, char * Directorio_Soluciones_Latex, char * ruta_enunciado, char * ruta_enunciado_LATEX, char * ruta_solucion, char * ruta_solucion_LATEX, long double base, long double size)
{
#define CODIGO_PROFESOR_SIZE 10
  long int k;
  GtkTextIter start;
  GtkTextIter end;
  char PG_command [10000];
  gchar *materia, * institucion, *escuela, *programa, *descripcion, *profesor;
  char examen[10];
  gchar *instrucciones, *juramento;
  char  *instrucciones_corregidas, *juramento_corregido;
  int year, month, day;
  char Codigo_Profesor [20];

  k = (long int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05ld", k);

  materia             = gtk_editable_get_chars(GTK_EDITABLE(EN_materia)            , 0, -1);
  institucion         = gtk_editable_get_chars(GTK_EDITABLE(EN_institucion)        , 0, -1);
  escuela             = gtk_editable_get_chars(GTK_EDITABLE(EN_escuela)            , 0, -1);
  programa            = gtk_editable_get_chars(GTK_EDITABLE(EN_programa)           , 0, -1);
  descripcion         = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion)        , 0, -1);
  gtk_calendar_get_date(CAL_fecha,&year,&month,&day); 
  month++;

  profesor = gtk_combo_box_get_active_text(CB_profesor);
  Extrae_codigo (profesor, Codigo_Profesor, CODIGO_PROFESOR_SIZE);

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.1));
  while (gtk_events_pending ()) gtk_main_iteration ();

  gtk_text_buffer_get_start_iter (buffer_TV_instrucciones, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_instrucciones, &end);
  instrucciones = gtk_text_buffer_get_text (buffer_TV_instrucciones, &start, &end, FALSE);
  instrucciones_corregidas = (char *) malloc (sizeof(char) * strlen(instrucciones)*2);
  hilera_POSTGRES (instrucciones, instrucciones_corregidas);

  gtk_text_buffer_get_start_iter (buffer_TV_juramento, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_juramento, &end);
  juramento = gtk_text_buffer_get_text (buffer_TV_juramento, &start, &end, FALSE);
  juramento_corregido = (char *) malloc (sizeof(char) * strlen(juramento)*2);
  hilera_POSTGRES (juramento, juramento_corregido);

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.15));
  while (gtk_events_pending ()) gtk_main_iteration ();

  sprintf (PG_command,"UPDATE ex_examenes SET institucion='%s', escuela='%s', programa ='%s', profesor = '%s', descripcion='%s', Year=%d, Month=%d, Day=%d, tiene_instrucciones = %s, instrucciones=E'%s', tiene_juramento=%s, juramento = E'%s' where codigo_examen = '%s'", 
           institucion, escuela, programa, Codigo_Profesor, descripcion, year, month, day,
           gtk_toggle_button_get_active(CK_instrucciones)?"TRUE":"FALSE",
           gtk_toggle_button_get_active(CK_instrucciones)?instrucciones_corregidas:" ",
           gtk_toggle_button_get_active(CK_juramento)    ?"TRUE":"FALSE",
           gtk_toggle_button_get_active(CK_juramento)    ?juramento_corregido:" ", examen);
  res = PQEXEC(DATABASE, PG_command); PQclear(res);

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.2));
  while (gtk_events_pending ()) gtk_main_iteration ();

  Establece_Directorios (Directorio_Enunciados, Directorio_Enunciados_Latex,
                         Directorio_Soluciones, Directorio_Soluciones_Latex,
                         materia, year, month, day);

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.22));
  while (gtk_events_pending ()) gtk_main_iteration ();

  Imprime_Enunciado (Directorio_Enunciados, Directorio_Enunciados_Latex, ruta_enunciado, ruta_enunciado_LATEX, base + (size * 0.22), (size * 0.58) );

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.8));
  while (gtk_events_pending ()) gtk_main_iteration ();

  Imprime_Solucion  (Directorio_Soluciones, Directorio_Soluciones_Latex, ruta_solucion, ruta_solucion_LATEX,base + (size * 0.8), (size * 0.1));

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.9));
  while (gtk_events_pending ()) gtk_main_iteration ();

  g_free(materia);
  g_free(institucion);
  g_free(escuela);
  g_free(programa);
  g_free(descripcion);
  g_free(instrucciones);
  free(instrucciones_corregidas);
  g_free(juramento);
  free(juramento_corregido);

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.95));
  while (gtk_events_pending ()) gtk_main_iteration ();
}

void Extrae_codigo (char * hilera, char * codigo, int N)
{
  int i;

  i = 0;

  while (hilera[i] != ' ') codigo [i] = hilera[i++];

  while (i < (N-1)) codigo[i++] = ' ';

  codigo[i] = '\0';
}

void Imprime_Enunciado (char * Directorio_Enunciados, char * Directorio_Enunciados_Latex, char * ruta_destino, char * ruta_latex, long double base, long double size)
{
  long int k;
  int i;
  gchar * font_size;

  FILE * Archivo_Latex;
  char PG_command [2000];
  char hilera_antes [3000], hilera_despues [3000];
  gchar *materia_descripcion, *descripcion, *version, *institucion, *escuela, *programa;
  char codigo[10];
  char Nombre_Archivo_Latex[100];
  char Comando[200];
  char ejercicio_actual[7];
  int N_preguntas, N_preguntas_ejercicio;
  int actual;
  int year, month, day;
  int color_barra = 0;
  int N_columnas;

   k = (long int) gtk_spin_button_get_value_as_int (SP_examen);
   sprintf (codigo, "%05ld", k);

   materia_descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_materia_descripcion), 0, -1);
   descripcion         = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion)        , 0, -1);
   institucion         = gtk_editable_get_chars(GTK_EDITABLE(EN_institucion)        , 0, -1);
   escuela             = gtk_editable_get_chars(GTK_EDITABLE(EN_escuela)            , 0, -1);
   programa            = gtk_editable_get_chars(GTK_EDITABLE(EN_programa)           , 0, -1);
   version             = gtk_editable_get_chars(GTK_EDITABLE(EN_version)            , 0, -1);
   gtk_calendar_get_date(CAL_fecha,&year,&month,&day); 
   month++;

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.05));
   while (gtk_events_pending ()) gtk_main_iteration ();

   sprintf (Nombre_Archivo_Latex,"%s.tex",Enunciado);
   Archivo_Latex = fopen (Nombre_Archivo_Latex,"w");

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.1));
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, twoside, onecolumn]{EXAMINER}\n");

   fprintf (Archivo_Latex, "\\PassOptionsToPackage{firstpage}{draftwatermark}\n");
   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);
   fprintf (Archivo_Latex, "\\newcommand*\\circled[1]{\\tikz[baseline=(char.base)]{\\node[shape=circle,draw,very thick, inner sep=0.0pt, minimum size=10.4pt] (char) {{\\color{gray}\\textsf{\\scriptsize{#1}}}};}}\n");
   fprintf (Archivo_Latex, "\\newcommand*\\circledblack[1]{\\tikz[baseline=(char.base)]{\\node[shape=circle,draw,fill=black,very thick, inner sep=0.0pt, minimum size=10.4pt] (char) {{\\color{yellow}\\textit{#1}}};}}\n");
   fprintf (Archivo_Latex, "\\newcommand*\\circledoption[1]{\\tikz[baseline=(char.base)]{\\node[shape=circle,draw,inner sep=0.0pt, minimum size=0.35cm] (char) {\\textit{#1}};}}\n");

   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);
   fprintf (Archivo_Latex, "\\pagestyle{fancy}\n");

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.12));
   while (gtk_events_pending ()) gtk_main_iteration ();

   if (gtk_toggle_button_get_active(CK_watermark))
      {
       if (gtk_file_chooser_get_filename(FC_watermark))
	  {
           fprintf (Archivo_Latex, "\\SetWatermarkAngle{0}\n");
           fprintf (Archivo_Latex, "\\SetWatermarkText{\\includegraphics{%s}}\n", gtk_file_chooser_get_filename(FC_watermark));
	  }
       else
          fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
      }
   else
      fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");

   Headers_Footers (Archivo_Latex, day, month, year, descripcion, materia_descripcion, escuela, institucion, programa);
  
   fprintf (Archivo_Latex, "\\begin{document}\n");
   fprintf (Archivo_Latex, "\n\n");

   Primera_Pagina_Examen (Archivo_Latex, codigo, version, institucion, escuela, materia_descripcion, descripcion, day, month, year);

   if (gtk_toggle_button_get_active(RB_1_columna))
      {
       N_columnas = 1;
       fprintf (Archivo_Latex, "\\onecolumn\n");
      }
   else
      {
       N_columnas = 2;
       fprintf (Archivo_Latex, "\\twocolumn\n");
      }

   fprintf (Archivo_Latex, "\\clearpage\n");

   font_size = gtk_editable_get_chars(GTK_EDITABLE(EN_size), 0, -1);
   fprintf (Archivo_Latex, "{%s\n", font_size);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.15));
   while (gtk_events_pending ()) gtk_main_iteration ();

   sprintf (PG_command,"SELECT texto_pregunta, texto_opcion_A, texto_opcion_B, texto_opcion_C, texto_opcion_D, texto_opcion_E, Opcion_1, Opcion_2, Opcion_3, Opcion_4, Opcion_5, ejercicio, usa_header, texto_header from bd_texto_preguntas, bd_texto_ejercicios, ex_examenes_preguntas where examen = '%.5s' and version = '%.4s' and codigo_pregunta = codigo_unico_pregunta and codigo_consecutivo_ejercicio = consecutivo_texto order by posicion", codigo, version);
   res = PQEXEC(DATABASE, PG_command);

   N_preguntas = PQntuples(res);
   actual = 0;

   strcpy (ejercicio_actual, PQgetvalue (res, actual, 11));
   N_preguntas_ejercicio = 0;
   for (i = actual; (i < N_preguntas) && (strcmp(ejercicio_actual, PQgetvalue (res, i, 11)) == 0); i++) N_preguntas_ejercicio++;
   if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n{\\color{%s}\\rule{8.1cm}{5pt}}\n\n",color_barra?"green":"blue");

   if (*PQgetvalue (res, actual, 12) == 't')
      {
       if (N_preguntas_ejercicio > 1)
          {
           if (N_preguntas_ejercicio == 2)
              fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
           else
              fprintf (Archivo_Latex, "\\textbf{Las preguntas de la %d a la %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	   strcpy (hilera_antes, PQgetvalue (res, actual, 13));
           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf (Archivo_Latex, "%s\n", hilera_despues);
           fprintf (Archivo_Latex, "\\rule{%scm}{1pt}\n", N_columnas==1? "18":"8.9");
           fprintf (Archivo_Latex, "\\begin{questions}\n");
	  }
       else
          {
           fprintf (Archivo_Latex, "\\begin{questions}\n");
           Imprime_pregunta (actual, res, Archivo_Latex, PQgetvalue (res, actual, 13), N_columnas);
           actual++;
           N_preguntas_ejercicio = 0;
          }
      }
   else
      {
       fprintf (Archivo_Latex, "\\begin{questions}\n");
      }

   for (i=0; i < N_preguntas_ejercicio; i++) Imprime_pregunta (actual+i, res, Archivo_Latex," ", N_columnas);

   if (N_preguntas_ejercicio > 1)
      {
       fprintf (Archivo_Latex, "\n\n{\\color{%s}\\rule{%scm}{5pt}}\n\n",color_barra?"green":"blue",N_columnas==1?"16.2":"8.1");
       color_barra = 1 - color_barra;
      }

   actual += N_preguntas_ejercicio;

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.15) + (size * 0.6 *((long double) actual/(long double) N_preguntas)));
   while (gtk_events_pending ()) gtk_main_iteration ();

   while (actual < N_preguntas)
         {
          strcpy (ejercicio_actual, PQgetvalue (res, actual, 11));
          N_preguntas_ejercicio = 0;

          for (i = actual; (i < N_preguntas) && (strcmp(ejercicio_actual, PQgetvalue (res, i, 11)) == 0); i++) N_preguntas_ejercicio++;

          if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n{\\color{%s} \\rule{%scm}{5pt}}\n\n",color_barra?"green":"blue",N_columnas==1?"16.2":"8.1");

          if (*PQgetvalue (res, actual, 12) == 't')
	     {
              if (N_preguntas_ejercicio > 1)
	         {
  	          if (N_preguntas_ejercicio == 2)
	             fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
	          else
	             fprintf (Archivo_Latex, "\\textbf{Las preguntas de la %d a la %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	          strcpy (hilera_antes, PQgetvalue (res, actual, 13));
                  hilera_LATEX (hilera_antes, hilera_despues);
                  fprintf (Archivo_Latex, "%s\n", hilera_despues);

                  fprintf (Archivo_Latex, "\\rule{%scm}{1pt}\n",N_columnas==1?"16":"8");
		 }
	      else
		 {
		  Imprime_pregunta (actual, res, Archivo_Latex, PQgetvalue(res,actual, 13), N_columnas);
		  N_preguntas_ejercicio = 0;
		  actual++;
		 }
	     }

	  for (i=0; i < N_preguntas_ejercicio; i++) Imprime_pregunta (actual+i, res, Archivo_Latex, " ", N_columnas);

          if (N_preguntas_ejercicio > 1) 
	     {
	       fprintf (Archivo_Latex, "\n\n{\\color{%s} \\rule{%scm}{5pt}}\n\n",color_barra?"green":"blue",N_columnas==1?"16.2":"8.1");
	      color_barra = 1 - color_barra;
	     }

          actual += N_preguntas_ejercicio;

          gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.15) + (size * 0.6 *((long double) actual/(long double) N_preguntas)));
          while (gtk_events_pending ()) gtk_main_iteration ();
         }

   PQclear(res);

   fprintf (Archivo_Latex, "\\end{questions}\n");

   if (gtk_toggle_button_get_active(CK_apendice))
      {
       if (gtk_file_chooser_get_filename(FC_apendice))
	  {
           fprintf (Archivo_Latex, "\\clearpage\n");
	   fprintf (Archivo_Latex, "\\input{%s}\n",gtk_file_chooser_get_filename(FC_apendice));
	  }
      }
   fprintf (Archivo_Latex, "}\n");

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.75));
   while (gtk_events_pending ()) gtk_main_iteration ();

   latex_2_pdf (&parametros, Directorio_Enunciados, Directorio_Enunciados_Latex, Enunciado, 1,PB_reporte, base + (size * 0.75), (size * 0.25), ruta_destino, ruta_latex);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 1.0));
   while (gtk_events_pending ()) gtk_main_iteration ();

   g_free(materia_descripcion);
   g_free(descripcion);
   g_free(institucion);
   g_free(escuela);
   g_free(programa);
   g_free(font_size);
}

void Establece_Directorios (char * Directorio_Enunciados, char * Directorio_Enunciados_Latex,
                            char * Directorio_Soluciones, char * Directorio_Soluciones_Latex,
                            gchar * materia, int year, int month, int day)
{
   DIR * pdir;
   char Directorio_materia       [600];
   char Directorio_materia_fecha [600];
   int i, n;

   sprintf (Directorio_materia,"%s/%s",parametros.ruta_examenes, materia);
   n = strlen(Directorio_materia);
   for (i=n-1; Directorio_materia[i]==' ';i--);
   Directorio_materia[i+1]='\0';

   sprintf (Directorio_materia_fecha,   "%s/%d-%02d-%02d", Directorio_materia,year,month,day);
   sprintf (Directorio_Enunciados,      "%s/Enunciados",Directorio_materia_fecha);
   sprintf (Directorio_Enunciados_Latex,"%s/LATEX",Directorio_Enunciados);
   sprintf (Directorio_Soluciones,      "%s/Soluciones",Directorio_materia_fecha);
   sprintf (Directorio_Soluciones_Latex,"%s/LATEX",Directorio_Soluciones);

   pdir = opendir(Directorio_materia);
   if (!pdir)
      {
       mkdir(Directorio_materia,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      }
   else
      closedir(pdir);

   pdir = opendir(Directorio_materia_fecha);
   if (!pdir)
      {
       mkdir(Directorio_materia_fecha,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      }
   else
      closedir(pdir);

   pdir = opendir(Directorio_Enunciados);
   if (!pdir)
      {
       mkdir(Directorio_Enunciados,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      }
   else
      closedir(pdir);

   pdir = opendir(Directorio_Enunciados_Latex);
   if (!pdir)
      {
       mkdir(Directorio_Enunciados_Latex,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      }
   else
      closedir(pdir);

   pdir = opendir(Directorio_Soluciones);
   if (!pdir)
      {
       mkdir(Directorio_Soluciones,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      }
   else
      closedir(pdir);

   pdir = opendir(Directorio_Soluciones_Latex);
   if (!pdir)
      {
       mkdir(Directorio_Soluciones_Latex,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      }
   else
      closedir(pdir);
}

void Headers_Footers (FILE * Archivo_Latex, int day, int month, int year,
                      char * descripcion, char * materia_descripcion, char * escuela, char * institucion, char * programa)
{
   char hilera_antes[2000], hilera_despues[2000];

   fprintf (Archivo_Latex, "\\fancyhead{}\n");
   fprintf (Archivo_Latex, "\\fancyhead[LO, RE]{%d/%d/%d}\n", day, month, year);
   fprintf (Archivo_Latex, "\\fancyhead[LE,RO]{\\thepage}\n");
   sprintf (hilera_antes,"\\fancyhead[CE,CO]{\\textbf{%s - %s}}", descripcion, materia_descripcion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   fprintf  (Archivo_Latex, "\\fancyfoot{}\n");
   sprintf  (hilera_antes,"\\fancyfoot[CE,CO]{%s}",           escuela);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   sprintf  (hilera_antes,"\\fancyfoot[LE,LO]{\\textbf{%s}}", institucion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   sprintf  (hilera_antes,"\\fancyfoot[RE,RO]{%s}",           programa);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   fprintf (Archivo_Latex, "\\renewcommand{\\headrulewidth}{0.4pt}\n");
   fprintf (Archivo_Latex, "\\renewcommand{\\footrulewidth}{0.4pt}\n");
}

void Primera_Pagina_Examen (FILE * Archivo_Latex, char * codigo, char * version, char * institucion,
                            char * escuela, char * materia_descripcion, char * descripcion, int day, int month, int year)
{
  if (gtk_toggle_button_get_active(RB_escribir))
     Primera_Pagina_Examen_Escribir (Archivo_Latex, codigo, version, institucion, escuela, materia_descripcion, descripcion, day, month, year);
  else
     Primera_Pagina_Examen_CV (Archivo_Latex, codigo, version, institucion, escuela, materia_descripcion, descripcion, day, month, year);
}

void Primera_Pagina_Examen_CV (FILE * Archivo_Latex, char * codigo, char * version, char * institucion,
                               char * escuela, char * materia_descripcion, char * descripcion, int day, int month, int year)
{
   int i, j, k, N_lineas;
   char hilera_antes [2000], hilera_despues[2000];
   int left, right;

   sprintf (hilera_antes, "\\renewcommand{\\leftmark}{%s-%s - %s - %s}", codigo, version, institucion, escuela);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes, "\\title{\\textsf{%s}\\\\\\textsf{%s} - \\textsf{%d/%d/%d}}", materia_descripcion,descripcion, day, month, year);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex,"%s\n", hilera_despues);

   fprintf (Archivo_Latex, "\\maketitle\n");

   fprintf (Archivo_Latex, "\\begin{tabular}{r p{8cm} r p{3cm}}\n");
   fprintf (Archivo_Latex, "Nombre : & & Carnet:& \\\\ \\cline{2-2} \\cline{4-4} \\\\ \n");
   fprintf (Archivo_Latex, "\\end{tabular}\n\n\n");

   if (gtk_toggle_button_get_active(RB_arriba))
      {
       Caja_Opciones (Archivo_Latex);
       Agregar_Instrucciones (Archivo_Latex);
       Agregar_Juramento     (Archivo_Latex);
       fprintf (Archivo_Latex, "\\vfill\n");
       Caja_Nota(Archivo_Latex);
      }
   else
      {
       Agregar_Instrucciones (Archivo_Latex);
       Agregar_Juramento     (Archivo_Latex);
       fprintf (Archivo_Latex, "\\vfill\n");
       Caja_Opciones (Archivo_Latex);
       Caja_Nota(Archivo_Latex);
      }
}

void Caja_Opciones (FILE * Archivo_Latex)
{
   int i, j, N_lineas;

   fprintf (Archivo_Latex, "\\renewcommand{\\arraystretch}{1.1}\n");
   /*                                                         #    A    B    C    D    E            #    A    B    C    D    E            #    A    B    C    D    E            #    A    B    C    D    E            #    A    B    C    D    E            #    A    B    C    D    E     */
   fprintf (Archivo_Latex, "\\begin{tabularx}{\\textwidth}{@{}r@{ }c@{ }c@{ }c@{ }c@{ }c@{    }c@{ }r@{ }c@{ }c@{ }c@{ }c@{ }c@{    }c@{ }r@{ }c@{ }c@{ }c@{ }c@{ }c@{    }c@{ }r@{ }c@{ }c@{ }c@{ }c@{ }c@{    }c@{ }r@{ }c@{ }c@{ }c@{ }c@{ }c@{    }c@{ }r@{ }c@{ }c@{ }c@{ }c@{ }c@{ }}\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\multicolumn{2}{c}{} \\\\ [-1.9ex]\n");
   fprintf (Archivo_Latex, "\\multicolumn{41}{c}{\\cellcolor{white}\\Large{\\textsf{\\textbf{Respuestas}}}} \\\\ \n");
   fprintf (Archivo_Latex, "\\multicolumn{41}{c}{} \\\\ [-1.9ex]\n");

   N_lineas = Numero_Preguntas / 6;
   if ((N_lineas * 6) < Numero_Preguntas) N_lineas++;

   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\multicolumn{41}{c}{} \\\\ [-1.9ex]\n");
   j = 1;
   for (i=0;i < N_lineas; i++)
       {
	j = Pone_Circulos (Archivo_Latex, j);
        fprintf (Archivo_Latex, "\\multicolumn{41}{c}{} \\\\ [-2.51ex]\n");
       }
   fprintf (Archivo_Latex, "\\multicolumn{2}{c}{} \\\\ [-1.9ex]\n");
   fprintf (Archivo_Latex, "\\hline\n");
   Codifica_Informacion (Archivo_Latex);
   fprintf (Archivo_Latex, "\\multicolumn{2}{c}{} \\\\ [-1.9ex]\n");
   fprintf (Archivo_Latex, "\\multicolumn{2}{c}{} \\\\ [-1.9ex]\n");

   fprintf (Archivo_Latex, "\\end{tabularx}\n\n\n");
   fprintf (Archivo_Latex, "\\renewcommand{\\arraystretch}{1.0}\n\n\n\n");
}

#if 0
void Pone_Letras (FILE * Archivo_Latex, int j)
{
  int k;

	k = j;
        fprintf (Archivo_Latex, " & \\textsf{\\scriptsize{{\\color{gray} A}}} & \\textsf{\\scriptsize{{\\color{gray} B}}}& \\textsf{\\scriptsize{{\\color{gray} C}}}& \\textsf{\\scriptsize{{\\color{gray} D}}}& \\textsf{\\scriptsize{{\\color{gray} E}}}& & ");
	k++;
	if (k > Numero_Preguntas)
	   fprintf (Archivo_Latex, "\\\\[-1.4ex]\n");
	else
	   {
            fprintf (Archivo_Latex, " & \\textsf{\\scriptsize{{\\color{gray} A}}} & \\textsf{\\scriptsize{{\\color{gray} B}}}& \\textsf{\\scriptsize{{\\color{gray} C}}}& \\textsf{\\scriptsize{{\\color{gray} D}}}& \\textsf{\\scriptsize{{\\color{gray} E}}}& & ");
  	    k++;
	    if (k > Numero_Preguntas)
	       fprintf (Archivo_Latex, "\\\\[-1.4ex]\n");
	    else
	       {
                fprintf (Archivo_Latex, " & \\textsf{\\scriptsize{{\\color{gray} A}}} & \\textsf{\\scriptsize{{\\color{gray} B}}}& \\textsf{\\scriptsize{{\\color{gray} C}}}& \\textsf{\\scriptsize{{\\color{gray} D}}}& \\textsf{\\scriptsize{{\\color{gray} E}}}& & ");
  	        k++;
	        if (k > Numero_Preguntas)
	           fprintf (Archivo_Latex, "\\\\[-1.4ex]\n");
		else
		   {
                    fprintf (Archivo_Latex, " & \\textsf{\\scriptsize{{\\color{gray} A}}} & \\textsf{\\scriptsize{{\\color{gray} B}}}& \\textsf{\\scriptsize{{\\color{gray} C}}}& \\textsf{\\scriptsize{{\\color{gray} D}}}& \\textsf{\\scriptsize{{\\color{gray} E}}}& & ");
  	            k++;
	            if (k > Numero_Preguntas)
	               fprintf (Archivo_Latex, "\\\\[-1.4ex]\n");
		    else
		       {
                        fprintf (Archivo_Latex, " & \\textsf{\\scriptsize{{\\color{gray} A}}} & \\textsf{\\scriptsize{{\\color{gray} B}}}& \\textsf{\\scriptsize{{\\color{gray} C}}}& \\textsf{\\scriptsize{{\\color{gray} D}}}& \\textsf{\\scriptsize{{\\color{gray} E}}}& & ");
  	                k++;
	                if (k > Numero_Preguntas)
	                   fprintf (Archivo_Latex, "\\\\[-1.4ex]\n");
			else
                           fprintf (Archivo_Latex, " & \\textsf{\\scriptsize{{\\color{gray} A}}} & \\textsf{\\scriptsize{{\\color{gray} B}}}& \\textsf{\\scriptsize{{\\color{gray} C}}}& \\textsf{\\scriptsize{{\\color{gray} D}}}& \\textsf{\\scriptsize{{\\color{gray} E}}}\\\\[-1.4ex]\n");
		       }
		   }
	       }
	   }
}
#endif

int Pone_Circulos (FILE * Archivo_Latex, int j)
{
  int k;
  k = j;
        fprintf (Archivo_Latex, "\\scriptsize{\\textsf{{\\color{gray} %d}}} &\\circled{A} &\\circled{B} &\\circled{C} & \\circled{D} & \\circled{E}& &", k++);
	if (k > Numero_Preguntas)
	   fprintf (Archivo_Latex, "\\\\\n");
	else
	   {
            fprintf (Archivo_Latex, "\\scriptsize{\\textsf{{\\color{gray} %d}}} &\\circled{A} &\\circled{B} &\\circled{C} & \\circled{D} & \\circled{E}& &", k++);
	    if (k > Numero_Preguntas)
	       fprintf (Archivo_Latex, "\\\\\n");
	    else
	       {
                fprintf (Archivo_Latex, "\\scriptsize{\\textsf{{\\color{gray} %d}}} &\\circled{A} &\\circled{B} &\\circled{C} & \\circled{D} & \\circled{E}& &", k++);
  	        if (k > Numero_Preguntas)
	           fprintf (Archivo_Latex, "\\\\\n");
		else
		   {
                    fprintf (Archivo_Latex, "\\scriptsize{\\textsf{{\\color{gray} %d}}} &\\circled{A} &\\circled{B} &\\circled{C} & \\circled{D} & \\circled{E}& &", k++);
    	            if (k > Numero_Preguntas)
	               fprintf (Archivo_Latex, "\\\\\n");
		    else
		       {
                        fprintf (Archivo_Latex, "\\scriptsize{\\textsf{{\\color{gray} %d}}} &\\circled{A} &\\circled{B} &\\circled{C} & \\circled{D} & \\circled{E}& &", k++);
    	                if (k > Numero_Preguntas)
	                   fprintf (Archivo_Latex, "\\\\\n");
			else
                           fprintf (Archivo_Latex, "\\scriptsize{\\textsf{{\\color{gray} %d}}} &\\circled{A} &\\circled{B} &\\circled{C} & \\circled{D} & \\circled{E}\\\\\n", k++);
		       }
		   }
	       }
	   }

  return (k);
}

void Codifica_Informacion (FILE * Archivo_Latex)
{
  long int k;
  int i;
  int N_bits;
  int random;
  long unsigned int codigo;
  int A,B,C,D,E,F;

  codigo = Numero_Preguntas;
  codigo = (codigo << 5);
  k = (int) gtk_range_get_value (GTK_RANGE(SC_versiones));
  codigo += k;
  codigo = (codigo << 15);
  k = (long int) gtk_spin_button_get_value_as_int (SP_examen);
  codigo += k;
  codigo = (codigo << 3); + MIN (7, random);
  random = (int) (7 * (rand() / (RAND_MAX + 1.0)));
  codigo += random;

  /* Codifica en 6 bloques de 5 bits */
  fprintf (Archivo_Latex, " &");
  A = B = C = D = E = F = 0;
  for (i=0;i<5; i++)
      {
       if (codigo & 0x01)
          fprintf (Archivo_Latex, "\\circledblack{} &");
       else
          fprintf (Archivo_Latex, "\\circled     {} &");
       A = A *2 + (codigo & 0x01);
       codigo >>= 1;
      }
  fprintf (Archivo_Latex, " & &");
  for (i=0;i<5; i++)
      {
       if (codigo & 0x01)
          fprintf (Archivo_Latex, "\\circledblack{} &");
       else
          fprintf (Archivo_Latex, "\\circled     {} &");
       B = B * 2 + (codigo & 0x01);
       codigo >>= 1;
      }
  fprintf (Archivo_Latex, " & &");
  for (i=0;i<5; i++)
      {
       if (codigo & 0x01)
          fprintf (Archivo_Latex, "\\circledblack{} &");
       else
          fprintf (Archivo_Latex, "\\circled     {} &");
       C = C *2 + (codigo & 0x01);
       codigo >>= 1;
      }
  fprintf (Archivo_Latex, " & &");
  for (i=0;i<5; i++)
      {
       if (codigo & 0x01)
          fprintf (Archivo_Latex, "\\circledblack{} &");
       else
          fprintf (Archivo_Latex, "\\circled     {} &");
       D = D * 2 + (codigo & 0x01);
       codigo >>= 1;
      }

  fprintf (Archivo_Latex, " & &");
  for (i=0;i<5; i++)
      {
       if (codigo & 0x01)
          fprintf (Archivo_Latex, "\\circledblack{} &");
       else
          fprintf (Archivo_Latex, "\\circled     {} &");
       E = E *2 + (codigo & 0x01);
       codigo >>= 1;
      }

  fprintf (Archivo_Latex, " & &");
  for (i=0;i<4; i++)
      {
       if (codigo & 0x01)
          fprintf (Archivo_Latex, "\\circledblack{} &");
       else
          fprintf (Archivo_Latex, "\\circled     {} &");
       F = F *2 + (codigo & 0x01);
       codigo >>= 1;
      }

  if (codigo & 0x01)
     fprintf (Archivo_Latex, "\\circledblack{}");
  else
     fprintf (Archivo_Latex, "\\circled     {}");
  F = F *2 + (codigo & 0x01);

  fprintf (Archivo_Latex, "\\\\\n");
}

void revisa(int A, int B, int C, int D, int E, int F)
{
  unsigned long int X, Y;
  int random, examen, version, preguntas;

  random   = ((A & 0x10)? 1    : 0) + ((A & 0x08)?    2 : 0) + ((A & 0x04)? 4     : 0);
  examen   =                                                                             ((A & 0x02)?    1 : 0) + ((A & 0x01)?    2 : 0) +
             ((B & 0x10)? 4    : 0) + ((B & 0x08)?    8 : 0) + ((B & 0x04)?    16 : 0) + ((B & 0x02)?   32 : 0) + ((B & 0x01)?   64 : 0) +
             ((C & 0x10)? 128  : 0) + ((C & 0x08)?  256 : 0) + ((C & 0x04)?   512 : 0) + ((C & 0x02)? 1024 : 0) + ((C & 0x01)? 2048 : 0) +
             ((D & 0x10)? 4096 : 0) + ((D & 0x08)? 8192 : 0) + ((D & 0x04)? 16384 : 0);
  version   =                                                                             ((D & 0x02)?    1 : 0) + ((D & 0x01)?    2 : 0) +
             ((E & 0x10)? 4    : 0) + ((E & 0x08)?    8 : 0) + ((E & 0x04)?    16 : 0);
  preguntas =                                                                             ((E & 0x02)?    1 : 0) + ((E & 0x01)?    2 : 0) +
             ((F & 0x10)? 4    : 0) + ((F & 0x08)?    8 : 0) + ((F & 0x04)?    16 : 0) + ((F & 0x02)?   32 : 0) + ((F & 0x01)?   64 : 0);
}

void Primera_Pagina_Examen_Escribir (FILE * Archivo_Latex, char * codigo, char * version, char * institucion,
                            char * escuela, char * materia_descripcion, char * descripcion, int day, int month, int year)
{
   int i, j, N_lineas;
   char hilera_antes [2000], hilera_despues[2000];

   sprintf (hilera_antes, "\\renewcommand{\\leftmark}{%s-%s - %s - %s}", codigo, version, institucion, escuela);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes, "\\title{\\textsf{%s}\\\\\\textsf{%s} - \\textsf{%d/%d/%d}}", materia_descripcion,descripcion, day, month, year);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex,"%s\n", hilera_despues);

   fprintf (Archivo_Latex, "\\maketitle\n");

   fprintf (Archivo_Latex, "\\begin{tabular}{r p{8cm} r p{3cm}}\n");
   fprintf (Archivo_Latex, "Nombre : & & Carnet:& \\\\ \\cline{2-2} \\cline{4-4} \\\\ \n");
   fprintf (Archivo_Latex, "\\end{tabular}\n\n\n");

   if (gtk_toggle_button_get_active(RB_arriba))
      {
       Caja_Letras (Archivo_Latex);
       Agregar_Instrucciones (Archivo_Latex);
       Agregar_Juramento     (Archivo_Latex);
       fprintf (Archivo_Latex, "\\vfill\n");
       Caja_Nota(Archivo_Latex);
      }
   else
      {
       Agregar_Instrucciones (Archivo_Latex);
       Agregar_Juramento     (Archivo_Latex);
       fprintf (Archivo_Latex, "\\vfill\n");
       Caja_Letras (Archivo_Latex);
       Caja_Nota(Archivo_Latex);
      }
}

void Agregar_Instrucciones (FILE * Archivo_Latex)
{
   gchar *Instrucciones;
   char  *Instrucciones_corregidas;
   GtkTextIter start;
   GtkTextIter end;

   if (gtk_toggle_button_get_active(CK_instrucciones))
      {
       fprintf (Archivo_Latex, "\\textbf{Instrucciones:} ");

       gtk_text_buffer_get_start_iter (buffer_TV_instrucciones, &start);
       gtk_text_buffer_get_end_iter   (buffer_TV_instrucciones, &end);
       Instrucciones = gtk_text_buffer_get_text (buffer_TV_instrucciones, &start, &end, FALSE);
       Instrucciones_corregidas = (char *) malloc (sizeof(char) * strlen(Instrucciones)*2);
       hilera_LATEX (Instrucciones, Instrucciones_corregidas);
       fprintf (Archivo_Latex, "%s\n\n\n\n", Instrucciones_corregidas);
       fprintf (Archivo_Latex, "\\vspace{2mm}\n");
       g_free(Instrucciones);
       free(Instrucciones_corregidas);
      }

}

void Agregar_Juramento (FILE * Archivo_Latex)
{
   gchar *Juramento;
   char  *Juramento_corregido;
   GtkTextIter start;
   GtkTextIter end;

   if (gtk_toggle_button_get_active(CK_juramento))
      {
       fprintf (Archivo_Latex, "\\begin{center}\n");
       fprintf (Archivo_Latex, "\\line(1, 0){300}\n\n");
       fprintf (Archivo_Latex, "\\end{center}\n");

       gtk_text_buffer_get_start_iter (buffer_TV_juramento, &start);
       gtk_text_buffer_get_end_iter   (buffer_TV_juramento, &end);
       Juramento = gtk_text_buffer_get_text (buffer_TV_juramento, &start, &end, FALSE);
       Juramento_corregido = (char *) malloc (sizeof(char) * strlen(Juramento)*2);
       hilera_LATEX (Juramento, Juramento_corregido);
       fprintf (Archivo_Latex, "\n\n\\textit{%s}\n\n\n\n", Juramento_corregido);
       fprintf (Archivo_Latex, "\\vspace{4mm}\n");
       fprintf (Archivo_Latex, "\\hfill \\line(1, 0){107}\n\n");
       fprintf (Archivo_Latex, "\\hspace{5.5in} \\textbf{Firma}\n\n");
       fprintf (Archivo_Latex, "\\vspace{4mm}\n");
       g_free(Juramento);
       free(Juramento_corregido);
      }
}

void Caja_Letras (FILE * Archivo_Latex)
{
   int i, j, N_lineas;

   fprintf (Archivo_Latex, "\\vspace{2mm}\n");
   fprintf (Archivo_Latex, "\\renewcommand{\\arraystretch}{2.0}\n");
   fprintf (Archivo_Latex, "\\begin{tabularx}{\\textwidth}{|r|X||r|X||r|X|c|r|X||r|X||r|X|c|r|X||r|X||r|X|c|r|X||r|X||r|X|}\n");

   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\multicolumn{27}{|c|}{\\cellcolor{white}\\LARGE{Respuestas}} \\\\ \n");
   fprintf (Archivo_Latex, "\\hline\n");

   fprintf (Archivo_Latex, "\\multicolumn{27}{c}{} \\\\ [-4ex]\n");
   fprintf (Archivo_Latex, "\\hhline{------~------~------~------}\n");

   N_lineas = Numero_Preguntas / 12;
   if ((N_lineas * 12) < Numero_Preguntas) N_lineas++;

   for (j=0; j < N_lineas; j++)
       {
        for (i=0;i<11;i++)
  	    {
	     if ((j*12+i+1)<=Numero_Preguntas)
	        {
		  if ((j+1)%2)
		    {
                     fprintf (Archivo_Latex, "\\cellcolor{lime}\\textbf{\\small{%d}} &\\cellcolor{lime} &", j*12 + i+1);
		    }
		 else
		    {
                     fprintf (Archivo_Latex, "\\cellcolor{white}\\textbf{\\small{%d}} &\\cellcolor{white} &", j*12 + i+1);
		    }
	        }
	     else
	        {
                 fprintf (Archivo_Latex, "\\cellcolor[gray]{0.2} & \\cellcolor[gray]{0.2}&");
	        }
	     if ((i == 2) || (i == 5) || (i == 8) ) fprintf (Archivo_Latex, " &");
	    }

	if ((j*12+i+1)<=Numero_Preguntas)
	   {
	    if ((j+1)%2)
               fprintf (Archivo_Latex, "\\cellcolor{lime}\\textbf{\\small{%d}} &\\cellcolor{lime} \\\\ \n", j*12 + i+1);
	    else
               fprintf (Archivo_Latex, "\\cellcolor{white}\\textbf{\\small{%d}} &\\cellcolor{white} \\\\ \n", j*12 + i+1);
	   }
	else
           fprintf (Archivo_Latex, "\\cellcolor[gray]{0.2}  & \\cellcolor[gray]{0.2}\\\\ \n");

        fprintf (Archivo_Latex, "\\cline{1-6} \\cline{8-13} \\cline{15-20} \\cline{22-27}\n");
        if (j < N_lineas-1)
 	   {
            fprintf (Archivo_Latex, "\\multicolumn{27}{c}{} \\\\ [-4ex]\n");
            fprintf (Archivo_Latex, "\\hhline{------~------~------~------}\n");
	   }
       }

   fprintf (Archivo_Latex, "\\multicolumn{27}{c}{} \\\\ [-4ex]\n");
   fprintf (Archivo_Latex, "\\multicolumn{27}{c}{} \\\\ [-4ex]\n");
   fprintf (Archivo_Latex, "\\end{tabularx}\n\n\n");

   fprintf (Archivo_Latex, "\\renewcommand{\\arraystretch}{1.0}\n\n\n\n");
}

void Caja_Nota(FILE * Archivo_Latex)
{
   fprintf (Archivo_Latex, "\\renewcommand{\\arraystretch}{2.8}\n");
   fprintf (Archivo_Latex, "\\begin{tabularx}{\\textwidth}{|l|l||l|X|l|}\n");
   fprintf (Archivo_Latex, "\\cline{1-3} \\cline{5-5}\n");
   fprintf (Archivo_Latex, "\\textsf{Correctas:} \\_\\_\\_\\_\\_\\_\\_\\_ \\textsf{de} \\textbf{\\textsf{%d}} & \\textsf{Porcentaje:} \\_\\_\\_\\_\\_\\_\\_\\_& \\textsf{Ajuste:} \\_\\_\\_\\_\\_\\_\\_\\_ de \\_\\_\\_\\_\\_\\_\\_\\_& &\\huge{\\textbf{\\textsf{Nota}}}: \\_\\_\\_\\_\\_\\_\\_\\_ \\\\ \n \n", Numero_Preguntas);
   fprintf (Archivo_Latex, "\\cline{1-3} \\cline{5-5}\n");
   /*
   fprintf (Archivo_Latex, "\\multicolumn{5}{c}{} \\\\ [-4.5ex]\n");
   */
   fprintf (Archivo_Latex, "\\end{tabularx}\n\n\n");
   fprintf (Archivo_Latex, "\\renewcommand{\\arraystretch}{1.0}\n\n\n\n");
}

void Imprime_pregunta (int i, PGresult * res, FILE * Archivo_Latex,char * prefijo, int N_columnas)
{
   char hilera_antes [4000], hilera_despues [4000];

   sprintf (hilera_antes,"\\question %s\n%s", prefijo, PQgetvalue (res, i, 0));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   fprintf (Archivo_Latex, "\\begin{answers}\n");

   fprintf (Archivo_Latex, "\\item[\\circledoption{a}] ");
   strcpy (hilera_antes, PQgetvalue (res, i, atoi(PQgetvalue(res, i, 6)) + 1));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item[\\circledoption{b}] ");
   strcpy (hilera_antes, PQgetvalue (res, i, atoi(PQgetvalue(res, i, 7)) + 1));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item[\\circledoption{c}] ");
   strcpy (hilera_antes, PQgetvalue (res, i, atoi(PQgetvalue(res, i, 8)) + 1));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item[\\circledoption{d}] ");
   strcpy (hilera_antes, PQgetvalue (res, i, atoi(PQgetvalue(res, i, 9)) + 1));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item[\\circledoption{e}] ");
   strcpy (hilera_antes, PQgetvalue (res, i, atoi(PQgetvalue(res, i, 10)) + 1));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\end{answers}\n");

   fprintf (Archivo_Latex, "\\rule{%scm}{1pt}\n",N_columnas==1?"16":"8");
   fprintf (Archivo_Latex, "\n\n");
}

void Imprime_Solucion (char * Directorio_Soluciones, char * Directorio_Soluciones_Latex, char * ruta_destino, char * ruta_latex, long double base, long double size)
{
  long int codigo_examen;
  int i,j, k, N_lineas;
  int N_respuestas;
  int respuesta_original;
  int opcion_1,opcion_2,opcion_3,opcion_4,opcion_5;

  FILE * Archivo_Latex;
  char PG_command [800];
  char hilera_antes [2000], hilera_despues [2000];
  gchar *materia, *materia_descripcion, *descripcion, *version, *institucion, *escuela, *programa;
  char codigo[10];
  char Nombre_Archivo_Latex[100];
  char Comando[200];
  int year, month, day;
  int frecuencias [5];
  DIR * pdir;
  char respuestas [2000];

   codigo_examen = (long int) gtk_spin_button_get_value_as_int (SP_examen);
   sprintf (codigo, "%05ld", codigo_examen);

   materia             = gtk_editable_get_chars(GTK_EDITABLE(EN_materia)            , 0, -1);
   materia_descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_materia_descripcion), 0, -1);
   descripcion         = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion)        , 0, -1);
   institucion         = gtk_editable_get_chars(GTK_EDITABLE(EN_institucion)        , 0, -1);
   escuela             = gtk_editable_get_chars(GTK_EDITABLE(EN_escuela)            , 0, -1);
   programa            = gtk_editable_get_chars(GTK_EDITABLE(EN_programa)           , 0, -1);
   version             = gtk_editable_get_chars(GTK_EDITABLE(EN_version)            , 0, -1);
   gtk_calendar_get_date(CAL_fecha,&year,&month,&day); 
   month++;

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.05));
   while (gtk_events_pending ()) gtk_main_iteration ();

   sprintf (Nombre_Archivo_Latex,"%s.tex",Solucion);

   Archivo_Latex = fopen (Nombre_Archivo_Latex,"w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, onecolumn, twoside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   if (gtk_toggle_button_get_active(CK_watermark))
      {
       if (gtk_file_chooser_get_filename(FC_watermark))
	  {
           fprintf (Archivo_Latex, "\\SetWatermarkAngle{0}\n");
           fprintf (Archivo_Latex, "\\SetWatermarkText{\\includegraphics{%s}}\n", gtk_file_chooser_get_filename(FC_watermark));
	  }
       else
          fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
      }
   else
      fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.1));
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\begin{document}\n");
   fprintf (Archivo_Latex, "\n\n");

   sprintf (hilera_antes, "\\title{%s\\\\Solución a %s - %d/%d/%d\\\\Versión %s\n}", materia_descripcion,descripcion, day, month, year, version);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex,"%s\n", hilera_despues);

   sprintf (hilera_antes, "\\renewcommand{\\leftmark}{%s - %s}", institucion, escuela);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes, "\\renewcommand{\\rightmark}{%s - %s - %d/%d/%d}", programa, materia_descripcion, day, month, year);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   fprintf (Archivo_Latex, "\\maketitle\n");

   fprintf (Archivo_Latex, "\\renewcommand{\\arraystretch}{2.0}\n");

   fprintf (Archivo_Latex, "\\begin{tabularx}{\\textwidth}{|");

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.15));
   while (gtk_events_pending ()) gtk_main_iteration ();

   for (i=0;i<4;i++)
       {
        fprintf (Archivo_Latex, "r|X||r|X||r|X|");
	if (i < 3) fprintf (Archivo_Latex, "c|");
       }
   fprintf (Archivo_Latex, "}\n");

   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\multicolumn{27}{|c|}{\\cellcolor{white}\\huge{Respuestas}} \\\\ \n");
   fprintf (Archivo_Latex, "\\hline\n");

   fprintf (Archivo_Latex, "\\multicolumn{27}{c}{} \\\\ [-4ex]\n");
   fprintf (Archivo_Latex, "\\hhline{------~------~------~------}\n");

   for (i=0;i<5;i++) frecuencias[i] = 0;

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.2));
   while (gtk_events_pending ()) gtk_main_iteration ();

   sprintf (PG_command,"SELECT respuesta, opcion_1, opcion_2, opcion_3, opcion_4, opcion_5 from ex_examenes_preguntas, bd_texto_preguntas where examen = '%.5s' and version = '%.4s' and codigo_pregunta = codigo_unico_pregunta order by posicion", codigo, version);
   res = PQEXEC(DATABASE, PG_command);
   N_respuestas = PQntuples(res);
   for (i=0; i<N_respuestas;i++)
       {
	respuesta_original = *PQgetvalue (res,i, 0) - 'A';
	opcion_1           = atoi(PQgetvalue (res,i, 1));
	opcion_2           = atoi(PQgetvalue (res,i, 2));
	opcion_3           = atoi(PQgetvalue (res,i, 3));
	opcion_4           = atoi(PQgetvalue (res,i, 4));
	opcion_5           = atoi(PQgetvalue (res,i, 5));

	if (respuesta_original == opcion_1) respuestas[i] = 'A';
	if (respuesta_original == opcion_2) respuestas[i] = 'B';
	if (respuesta_original == opcion_3) respuestas[i] = 'C';
	if (respuesta_original == opcion_4) respuestas[i] = 'D';
	if (respuesta_original == opcion_5) respuestas[i] = 'E';
       }
   PQclear(res);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.3));
   while (gtk_events_pending ()) gtk_main_iteration ();

   N_lineas = Numero_Preguntas / 12;
   if ((N_lineas * 12) < Numero_Preguntas) N_lineas++;

   for (j=0; j < N_lineas; j++)
       {
        for (i=0;i<11;i++)
  	    {
	     k=j*12+i+1;
	     if (k <= Numero_Preguntas)
	        {
		 if ((j+1)%2)
		    fprintf (Archivo_Latex, "\\cellcolor{lime}\\small{\\textbf{%d}} & \\cellcolor{lime}\\large{\\texttt{%c}} &", k, respuestas[k-1]);
		 else
		    fprintf (Archivo_Latex, "\\cellcolor{white}\\small{\\textbf{%d}} & \\cellcolor{white}\\large{\\texttt{%c}} &", k, respuestas[k-1]);

		 frecuencias [respuestas[k-1] - 'A']++;
	        }
	     else
                fprintf (Archivo_Latex, "\\cellcolor[gray]{0.2} & \\cellcolor[gray]{0.2}&");
	     if ((i == 2) || (i == 5) || (i == 8) ) fprintf (Archivo_Latex, " &");
	    }

        k=j*12+i+1;
	if (k <= Numero_Preguntas)
	   {
	    if ((j+1)%2) 
  	       fprintf (Archivo_Latex, "\\cellcolor{lime}\\small{\\textbf{%d}} &  \\cellcolor{lime}\\large{\\texttt{%c}} \\\\ \n", k, respuestas[k-1]);
	    else
  	       fprintf (Archivo_Latex, "\\cellcolor{white}\\small{\\textbf{%d}} &  \\cellcolor{white}\\large{\\texttt{%c}} \\\\ \n", k, respuestas[k-1]);
	    frecuencias [respuestas[k-1] - 'A']++;
	   }
	else
           fprintf (Archivo_Latex, "\\cellcolor[gray]{0.2}  & \\cellcolor[gray]{0.2}\\\\ \n");

        fprintf (Archivo_Latex, "\\hhline{------~------~------~------}\n");
        if (j < N_lineas-1)
 	   {
            fprintf (Archivo_Latex, "\\multicolumn{27}{c}{} \\\\ [-4ex]\n");
            fprintf (Archivo_Latex, "\\hhline{------~------~------~------}\n");
	   }
       }

   fprintf (Archivo_Latex, "\\end{tabularx}\n");

   fprintf (Archivo_Latex, "\\renewcommand{\\arraystretch}{1.0}\n\n\n\n");

   grafico_frecuencias (Archivo_Latex, frecuencias);

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 0.5));
   while (gtk_events_pending ()) gtk_main_iteration ();

   latex_2_pdf (&parametros, Directorio_Soluciones, Directorio_Soluciones_Latex, Solucion, 1, PB_reporte, base + (size * 0.5), (size * 0.45), ruta_destino, ruta_latex);

   system ("rm EX3050x.pdf");

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), base + (size * 1.0));
   while (gtk_events_pending ()) gtk_main_iteration ();

   g_free(materia);
   g_free(materia_descripcion);
   g_free(descripcion);
}

void grafico_frecuencias(FILE * Archivo_Latex, int * frecuencias)
{
   int i;
   FILE * Archivo_Datos, *Archivo_gnuplot;
   char comando [200];

   Archivo_Datos   = fopen ("EX3050.dat","w");
   for (i=0; i < 5; i++) fprintf(Archivo_Datos,"%3d\n", frecuencias[i]);
   fclose (Archivo_Datos);

   Archivo_gnuplot = fopen ("EX3050.gp","w");
   
   fprintf (Archivo_gnuplot, "set term postscript eps enhanced \"Times\" 12\n");
   fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
   fprintf (Archivo_gnuplot, "set size 1.25, 1.0\n");
   fprintf (Archivo_gnuplot, "set grid ytics\n");
   fprintf (Archivo_gnuplot, "set output \"EX3050x.eps\"\n");

   fprintf (Archivo_gnuplot, "set ylabel \"Cantidad de Preguntas\"\n");
   fprintf (Archivo_gnuplot, "set yrange [0:]\n");

   fprintf (Archivo_gnuplot, "set xrange [-1:5]\n");
   fprintf (Archivo_gnuplot, "set xtics 0, 1\n");

   fprintf (Archivo_gnuplot, "set xtics (");
   for (i=0; i < 5; i++)
       {
	fprintf (Archivo_gnuplot,"\"%c\" %d", 'A' + i, i);
        if (i == 4)
           fprintf (Archivo_gnuplot,")\n ");
        else
           fprintf (Archivo_gnuplot,", ");
       }

   fprintf (Archivo_gnuplot, "set style fill solid 1.0 border -1\n");
   fprintf (Archivo_gnuplot, "set boxwidth 0.25\n");

   fprintf (Archivo_gnuplot, "plot \"EX3050.dat\" using 1 with boxes fill lw 3 lc 3 lt 1 notitle");

   fclose (Archivo_gnuplot);

   sprintf(comando, "%s EX3050.gp", parametros.gnuplot);
   system (comando);

   sprintf(comando, "mv EX3050.gp %s", parametros.ruta_gnuplot);
   system (comando);

   sprintf(comando, "%s EX3050x.eps", parametros.epstopdf);
   system (comando);

   system ("rm EX3050x.eps");
   system ("rm EX3050.dat");

   fprintf (Archivo_Latex, "\\begin{figure}[b]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=1.0]{EX3050x.pdf}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n");
}

void Cambia_Size ()
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_size));

  gtk_entry_set_text(GTK_ENTRY(EN_size), Latex_Sizes[k+4]);
}

void Cambia_Version ()
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_versiones));

  Arma_nombres (k);
}

void Arma_nombres (int k)
{
  long int j;
  char PG_command [1000];
  gchar * materia;
  char examen[10];

  j = (long int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05ld", j);

  materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);

  sprintf (PG_command,"SELECT DISTINCT version from EX_examenes_preguntas where examen = '%s' order by version", examen);
  res_aux = PQEXEC(DATABASE, PG_command);

  if (PQntuples(res_aux) && (k > 0))
     {
      gtk_entry_set_text(GTK_ENTRY(EN_version), PQgetvalue (res_aux, k-1, 0));

      sprintf (Enunciado,"%s-%02d-%s",materia, k,PQgetvalue (res_aux, k-1, 0));
      Quita_espacios(Enunciado);
      sprintf (Solucion,"%s-sol",Enunciado);
     }

  PQclear(res_aux);

  g_free (materia);
}

void Quita_espacios (char * hilera)
{
  int i,j;

  i = j = 0;
  while (hilera[i])
        {
	 if (hilera[i] != ' ') hilera[j++] = hilera[i];
	 i++;
        }
  hilera[j] = '\0';
}

void Cambio_Institucion(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_escuela);
}

void Cambio_Escuela(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_programa);
}

void Cambio_Programa(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_descripcion);
}

void Cambio_Descripcion(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_institucion);
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
void on_WN_ex3050_destroy (GtkWidget *widget, gpointer user_data) 
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

void on_BN_print_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_solo_una_version ();
}

void on_BN_print_multi_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_todas_las_Versiones ();
}

void on_EN_institucion_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Institucion (widget, user_data);
}

void on_EN_escuela_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Escuela (widget, user_data);
}

void on_EN_programa_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Programa (widget, user_data);
}

void on_EN_descripcion_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Descripcion (widget, user_data);
}

void on_SC_versiones_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_Version ();
}

void on_SC_size_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_Size ();
}

void on_CK_instrucciones_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_instrucciones (widget, user_data);
}

void on_CK_juramento_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_juramento (widget, user_data);
}

void on_CK_apendice_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_apendice (widget, user_data);
}

void on_CK_watermark_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_watermark (widget, user_data);
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

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_ok1_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window3);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_extend_juramento_clicked(GtkWidget *widget, gpointer user_data)
{
  char window_title [100] = "Código LATEX de Juramento";
  char root[100] = "JURA";

  Display_Latex_Window (window_title, root, buffer_TV_juramento, window1);
}

void on_BN_extend_instrucciones_clicked(GtkWidget *widget, gpointer user_data)
{
  char window_title [100] = "Código LATEX de Instrucciones";
  char root[100] = "INST";

  Display_Latex_Window (window_title, root, buffer_TV_instrucciones, window1);
}
