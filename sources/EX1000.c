/*
Compilar con:

cc -o EX1000 EX1000.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX1000:                                */
/*                                         */
/*    Actualización de Parámetros          */
/*    The Examiner 0.4                     */
/*    15 de Octubre 2012                   */
/*    Autor: Francisco J. Torres-Rojas     */
/*-----------------------------------------*/
/*  Se despliega un menú con un botón por  */
/*  cada elemento configurable. A su vez,  */
/*  hay una interfaz independiente para    */
/*  cada uno de ellos.                     */
/*  Se actualiza al final el archivo de    */
/*  parámetros.                            */
/*******************************************/
#include <stdlib.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <locale.h>
#include <string.h>
#include "examiner.h"

/***************************/
/* Postgres stuff          */
/***************************/
PGconn	 *DATABASE;

/*************/
/* GTK stuff */
/*************/
GtkBuilder*builder; 
GError    * error = NULL;

GtkWidget *window1                = NULL;
GtkWidget *window2                = NULL;
GtkWidget *window3                = NULL;
GtkWidget *window4                = NULL;
GtkWidget *window5                = NULL;
GtkWidget *window6                = NULL;
GtkWidget *window7                = NULL;
GtkWidget *window8                = NULL;
GtkWidget *window9                = NULL;

GtkWidget *EB_cambia_interfaz     = NULL;
GtkWidget *FR_cambia_interfaz     = NULL;
GtkWidget *BN_interfaz            = NULL;

GtkWidget *EB_cambia_directorios  = NULL;
GtkWidget *FR_cambia_directorios  = NULL;
GtkWidget *BN_directorios         = NULL;

GtkWidget *EB_cambia_programas  = NULL;
GtkWidget *FR_cambia_programas  = NULL;
GtkWidget *BN_programas         = NULL;

GtkWidget *EB_cambia_datos  = NULL;
GtkWidget *FR_cambia_datos  = NULL;
GtkWidget *BN_datos         = NULL;

GtkWidget *EB_cambia_latex  = NULL;
GtkWidget *FR_cambia_latex  = NULL;
GtkWidget *BN_latex         = NULL;

GtkWidget *EB_cambia_beamer = NULL;
GtkWidget *FR_cambia_beamer = NULL;
GtkWidget *BN_beamer        = NULL;

GtkWidget *EB_programas           = NULL;
GtkWidget *FR_programas           = NULL;
GtkWidget *EN_latex_2_pdf         = NULL;
GtkWidget *EN_despliega_pdf       = NULL;
GtkWidget *EN_epstopdf            = NULL;
GtkWidget *EN_gnuplot             = NULL;
GtkWidget *EN_xterm               = NULL;

GtkWidget *EB_datos               = NULL;
GtkWidget *FR_datos               = NULL;
GtkWidget *EN_institucion         = NULL;
GtkWidget *EN_escuela             = NULL;
GtkWidget *EN_programa            = NULL;

GtkWidget     *FR_instrucciones        = NULL;
GtkTextView   *TV_instrucciones        = NULL;
GtkTextBuffer *buffer_TV_instrucciones = NULL;
GtkWidget     *FR_juramento            = NULL;
GtkTextView   *TV_juramento            = NULL;
GtkTextBuffer *buffer_TV_juramento     = NULL;

GtkWidget *EB_directorios           = NULL;
GtkWidget *FR_directorios           = NULL;
GtkWidget *EN_examenes              = NULL;
GtkFileChooser * FC_examenes        = NULL;
GtkWidget *EN_reportes              = NULL;
GtkFileChooser * FC_reportes        = NULL;
GtkWidget *EN_latex                 = NULL;
GtkFileChooser * FC_latex           = NULL;
GtkWidget *EN_gnuplot_path          = NULL;
GtkFileChooser * FC_gnuplot         = NULL;
GtkWidget *EN_backup                = NULL;
GtkFileChooser * FC_backup          = NULL;
GtkToggleButton *CK_figuras         = NULL;
GtkWidget *EN_figuras               = NULL;
GtkFileChooser * FC_figuras         = NULL;

GtkWidget *EB_latex                 = NULL;
GtkWidget *FR_latex                 = NULL;
GtkWidget *FR_packages              = NULL;
GtkTextView   *TV_packages          = NULL;
GtkTextBuffer *buffer_TV_packages   = NULL;

GtkWidget *EB_interfaz            = NULL;
GtkWidget *FR_interfaz            = NULL;
GtkWidget *EB_interaccion         = NULL;
GtkWidget *FR_interaccion         = NULL;
GtkWidget *FR_EX                  = NULL;
GtkWidget *FR_EXT                 = NULL;
GtkWidget *FR_EXZ                 = NULL;
GtkWidget *FR_EXO                 = NULL;
GtkToggleButton *RB_EX            = NULL;
GtkToggleButton *RB_EXT           = NULL;
GtkToggleButton *RB_EXZ           = NULL;
GtkToggleButton *RB_EXO           = NULL;
GtkToggleButton *CK_aleatorio     = NULL;
GtkWidget *EB_decoracion          = NULL;
GtkWidget *FR_decoracion          = NULL;
GtkWidget *FR_background          = NULL;
GtkWidget *SC_decoracion          = NULL;
GtkImage  *IM_decoracion          = NULL;
GtkWidget *EB_reportes            = NULL;
GtkWidget *FR_reportes            = NULL;
GtkToggleButton *CK_reportes      = NULL;
GtkWidget *EB_actualizacion       = NULL;
GtkWidget *FR_actualizacion       = NULL;
GtkToggleButton *CK_actualizacion = NULL;
GtkToggleButton *CK_timeout       = NULL;
GtkWidget *SC_timeout             = NULL;

GtkWidget *EB_beamer              = NULL;
GtkWidget *FR_beamer              = NULL;
GtkComboBox *CB_estilo            = NULL;
GtkComboBox *CB_color             = NULL;
GtkComboBox *CB_font              = NULL;
GtkComboBox *CB_size              = NULL;
GtkComboBox *CB_aspecto           = NULL;

GtkWidget *FR_botones             = NULL;
GtkWidget *BN_save                = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_terminar            = NULL;
GtkWidget *BN_ok                  = NULL;

GtkWidget *BN_ok1                 = NULL;
GtkWidget *BN_ok2                 = NULL;
GtkWidget *BN_ok3                 = NULL;
GtkWidget *BN_ok4                 = NULL;
GtkWidget *BN_ok5                 = NULL;
GtkWidget *BN_ok6                 = NULL;
GtkWidget *BN_ok7                 = NULL;

GtkWidget *FR_update              = NULL;
GtkWidget *EB_update              = NULL;
GtkWidget *FR_destino_reporte     = NULL;
GtkWidget *EB_destino_reporte     = NULL;
GtkLabel  *LB_destino_reporte     = NULL;
GtkLabel  *LB_destino_LATEX       = NULL;

/**************/
/* Prototipos */
/**************/
void Actualiza_Parametros   (GtkWidget *widget, gpointer user_data);
void Cambia_Actualizacion   ();
void Cambia_Aleatorio       ();
void Cambia_Decoracion      ();
void Cambia_Interaccion     ();
void Cambia_Timeout         ();
void Connect_widgets        ();
void Fin_de_Programa        (GtkWidget *widget, gpointer user_data);
void Fin_Ventana            (GtkWidget *widget, gpointer user_data);
void Init_Fields            ();
void Interface_Coloring     ();
void on_WN_ex1000_destroy   (GtkWidget *widget, gpointer user_data);
void on_BN_save_clicked     (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked     (GtkWidget *widget, gpointer user_data);
void on_CK_actualizacion_toggled(GtkWidget *widget, gpointer user_data);
void Read_Only_Fields       ();


/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX1000.glade", &error))
         {
          g_warning ("%s\n", error->message);
          g_error_free (error);
         }
      else
	 {
	  /* Conecta funciones con eventos de la interfaz */
          gtk_builder_connect_signals (builder, NULL);

	  /* Crea conexión con los campos de la interfaz a ser controlados */
	  Connect_widgets ();

	  /* Algunos campos son READ ONLY */
	  Read_Only_Fields ();

	  /* Un poco de color...  */
	  Interface_Coloring ();

          /* Despliega ventana principal e inicializa todos los campos */
          gtk_widget_show (window1);  
          Init_Fields ();              

          /* start the event loop */
          gtk_main ();
	 }
     }

  return 0;
}

void Connect_widgets()
{
  window1 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex1000"));
  window2 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_interfaz"));
  window3 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_directorios"));
  window4 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_programas"));
  window5 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_datos"));
  window6 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_latex"));
  window7 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_beamer"));
  window8 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));
  window9 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_destino_reporte"));

  EB_cambia_interfaz = GTK_WIDGET (gtk_builder_get_object (builder, "EB_cambia_interfaz"));
  FR_cambia_interfaz = GTK_WIDGET (gtk_builder_get_object (builder, "FR_cambia_interfaz"));
  BN_interfaz        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_interfaz"));

  EB_cambia_directorios = GTK_WIDGET (gtk_builder_get_object (builder, "EB_cambia_directorios"));
  FR_cambia_directorios = GTK_WIDGET (gtk_builder_get_object (builder, "FR_cambia_directorios"));
  BN_directorios        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_directorios"));

  EB_cambia_programas = GTK_WIDGET (gtk_builder_get_object (builder, "EB_cambia_programas"));
  FR_cambia_programas = GTK_WIDGET (gtk_builder_get_object (builder, "FR_cambia_programas"));
  BN_programas        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_programas"));

  EB_cambia_datos = GTK_WIDGET (gtk_builder_get_object (builder, "EB_cambia_datos"));
  FR_cambia_datos = GTK_WIDGET (gtk_builder_get_object (builder, "FR_cambia_datos"));
  BN_datos        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_datos"));

  EB_cambia_latex = GTK_WIDGET (gtk_builder_get_object (builder, "EB_cambia_latex"));
  FR_cambia_latex = GTK_WIDGET (gtk_builder_get_object (builder, "FR_cambia_latex"));
  BN_latex        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_latex"));

  EB_cambia_beamer = GTK_WIDGET (gtk_builder_get_object (builder, "EB_cambia_beamer"));
  FR_cambia_beamer = GTK_WIDGET (gtk_builder_get_object (builder, "FR_cambia_beamer"));
  BN_beamer        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_beamer"));

  EB_interfaz    = GTK_WIDGET (gtk_builder_get_object (builder, "EB_interfaz"));
  FR_interfaz    = GTK_WIDGET (gtk_builder_get_object (builder, "FR_interfaz"));
  EB_interaccion = GTK_WIDGET (gtk_builder_get_object (builder, "EB_interaccion"));
  FR_interaccion = GTK_WIDGET (gtk_builder_get_object (builder, "FR_interaccion"));
  FR_EX          = GTK_WIDGET (gtk_builder_get_object (builder, "FR_EX"));
  FR_EXT         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_EXT"));
  FR_EXZ         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_EXZ"));
  FR_EXO         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_EXO"));
  RB_EX          = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_EX"));
  RB_EXT         = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_EXT"));
  RB_EXO         = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_EXO"));
  RB_EXZ         = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_EXZ"));
  CK_aleatorio   = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_aleatorio"));
  EB_decoracion  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_decoracion"));
  FR_decoracion  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_decoracion"));
  FR_background  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_background"));
  SC_decoracion  = GTK_WIDGET (gtk_builder_get_object (builder, "SC_decoracion"));
  IM_decoracion  = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_decoracion"));
  EB_reportes    = GTK_WIDGET (gtk_builder_get_object (builder, "EB_reportes"));
  FR_reportes    = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reportes"));
  CK_reportes = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_reportes"));
  EB_actualizacion = GTK_WIDGET (gtk_builder_get_object (builder, "EB_actualizacion"));
  FR_actualizacion = GTK_WIDGET (gtk_builder_get_object (builder, "FR_actualizacion"));
  CK_actualizacion = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_actualizacion"));
  CK_timeout       = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_timeout"));
  SC_timeout       = GTK_WIDGET (gtk_builder_get_object (builder, "SC_timeout"));

  EB_programas     = GTK_WIDGET (gtk_builder_get_object (builder, "EB_programas"));
  FR_programas     = GTK_WIDGET (gtk_builder_get_object (builder, "FR_programas"));
  EN_latex_2_pdf   = GTK_WIDGET (gtk_builder_get_object (builder, "EN_latex_2_pdf"));
  EN_despliega_pdf = GTK_WIDGET (gtk_builder_get_object (builder, "EN_despliega_pdf"));
  EN_epstopdf      = GTK_WIDGET (gtk_builder_get_object (builder, "EN_epstopdf"));
  EN_gnuplot       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_gnuplot"));
  EN_xterm         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_xterm"));

  EB_datos       = GTK_WIDGET (gtk_builder_get_object (builder, "EB_datos"));
  FR_datos       = GTK_WIDGET (gtk_builder_get_object (builder, "FR_datos"));
  EN_institucion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_institucion"));
  EN_escuela     = GTK_WIDGET (gtk_builder_get_object (builder, "EN_escuela"));
  EN_programa    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_programa"));

  FR_instrucciones        = GTK_WIDGET (gtk_builder_get_object (builder, "FR_instrucciones"));
  TV_instrucciones        = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_instrucciones"));
  FR_juramento            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_juramento"));
  TV_juramento            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_juramento"));
  buffer_TV_instrucciones = gtk_text_view_get_buffer(TV_instrucciones);
  buffer_TV_juramento     = gtk_text_view_get_buffer(TV_juramento);

  EB_directorios  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_directorios"));
  FR_directorios  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_directorios"));
  EN_examenes     = GTK_WIDGET (gtk_builder_get_object (builder, "EN_examenes"));
  FC_examenes     = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_examenes"));
  EN_latex        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_latex"));
  FC_latex        = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_latex"));
  EN_reportes     = GTK_WIDGET (gtk_builder_get_object (builder, "EN_reportes"));
  FC_reportes     = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_reportes"));
  EN_gnuplot_path = GTK_WIDGET (gtk_builder_get_object (builder, "EN_gnuplot_path"));
  FC_gnuplot      = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_gnuplot"));
  EN_backup       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_backup"));
  FC_backup       = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_backup"));
  CK_figuras      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_figuras"));
  EN_figuras      = GTK_WIDGET (gtk_builder_get_object (builder, "EN_figuras"));
  FC_figuras      = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_figuras"));

  EB_latex           = GTK_WIDGET (gtk_builder_get_object (builder, "EB_latex"));
  FR_latex           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_latex"));
  FR_packages        = GTK_WIDGET (gtk_builder_get_object (builder, "FR_packages"));
  TV_packages        = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_packages"));
  buffer_TV_packages = gtk_text_view_get_buffer(TV_packages);

  EB_beamer  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_beamer"));
  FR_beamer  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_beamer"));
  CB_estilo  = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_estilo"));
  CB_color   = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_color"));
  CB_font    = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_font"));
  CB_size    = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_size"));
  CB_aspecto = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_aspecto"));

  FR_botones  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  BN_save     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
  BN_print    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_undo     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_terminar = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_ok       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
  BN_ok1      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok1"));
  BN_ok2      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok2"));
  BN_ok3      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok3"));
  BN_ok4      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok4"));
  BN_ok5      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok5"));
  BN_ok6      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok6"));
  BN_ok7      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok7"));

  EB_update = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
  FR_update = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));

  EB_destino_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "EB_destino_reporte"));
  FR_destino_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "FR_destino_reporte"));
  LB_destino_reporte = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_destino_reporte"));
  LB_destino_LATEX   = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_destino_LATEX"));
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_examenes,     FALSE);
  gtk_widget_set_can_focus(EN_reportes,     FALSE);
  gtk_widget_set_can_focus(EN_latex,        FALSE);
  gtk_widget_set_can_focus(EN_gnuplot_path, FALSE);
  gtk_widget_set_can_focus(EN_backup,       FALSE);
  gtk_widget_set_can_focus(EN_figuras,      FALSE);
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_interfaz,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_directorios, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_datos,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_programas,   GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_latex,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_beamer,      GTK_STATE_NORMAL, &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_interaccion,   GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_decoracion,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_reportes,      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_actualizacion, GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_cambia_interfaz,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_cambia_directorios, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_cambia_programas,   GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_cambia_datos,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_cambia_latex,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_cambia_beamer,      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_interaccion,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_decoracion,         GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_reportes,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_actualizacion,      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_EX,                 GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_EXT,                GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_EXZ,                GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_EXO,                GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones,            GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_interfaz,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_background,         GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_directorios,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_datos,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_programas,          GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_latex,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_beamer,             GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_instrucciones,      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_juramento,          GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_packages,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_update,             GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_destino_reporte,    GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(EN_examenes,      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_reportes,      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_latex,         GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_gnuplot_path,  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_backup,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_figuras,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_institucion,   GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_escuela,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_programa,      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_latex_2_pdf,   GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_despliega_pdf, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_epstopdf,      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_gnuplot,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_xterm,         GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,       GTK_STATE_ACTIVE, &color);

  gdk_color_parse (SPECIAL_BUTTON_NORMAL, &color);
  gtk_widget_modify_bg(BN_interfaz,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(BN_directorios, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(BN_programas,   GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(BN_datos,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(BN_latex,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(BN_beamer,      GTK_STATE_NORMAL, &color);

  gdk_color_parse (SPECIAL_BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_interfaz,    GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_directorios, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_programas,   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_datos,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_latex,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_beamer,      GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok1,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok2,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok3,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok4,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok5,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok6,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok7,         GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (SPECIAL_BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_interfaz,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_directorios, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_programas,   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_datos,       GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_latex,       GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_beamer,      GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,          GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok1,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok2,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok3,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok4,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok5,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok6,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok7,         GTK_STATE_ACTIVE, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,          GTK_STATE_NORMAL, &color);

  gdk_color_parse (SPECIAL_WINDOW, &color);
  gtk_widget_modify_bg(EB_destino_reporte, GTK_STATE_NORMAL, &color);
}

void Init_Fields()
{
   int i;
   char hilera[210];
   PGresult *res;

   gtk_entry_set_text  (GTK_ENTRY(EN_latex_2_pdf)  , parametros.latex_2_pdf);
   gtk_entry_set_text  (GTK_ENTRY(EN_despliega_pdf), parametros.despliega_pdf);
   gtk_entry_set_text  (GTK_ENTRY(EN_epstopdf),      parametros.epstopdf);
   gtk_entry_set_text  (GTK_ENTRY(EN_gnuplot),       parametros.gnuplot);
   gtk_entry_set_text  (GTK_ENTRY(EN_xterm),         parametros.xterm);

   gtk_entry_set_text  (GTK_ENTRY(EN_institucion),   parametros.Institucion);
   gtk_entry_set_text  (GTK_ENTRY(EN_escuela),       parametros.Escuela);
   gtk_entry_set_text  (GTK_ENTRY(EN_programa),      parametros.Programa);
   gtk_text_buffer_set_text(buffer_TV_instrucciones, parametros.Instrucciones, -1);
   gtk_text_buffer_set_text(buffer_TV_juramento,     parametros.Juramento, -1);
   gtk_text_buffer_set_text(buffer_TV_packages,      parametros.Paquetes, -1);

   gtk_file_chooser_set_current_folder (FC_examenes, parametros.ruta_examenes);
   gtk_file_chooser_set_current_folder (FC_reportes, parametros.ruta_reportes);
   gtk_file_chooser_set_current_folder (FC_latex,    parametros.ruta_latex);
   gtk_file_chooser_set_current_folder (FC_gnuplot,  parametros.ruta_gnuplot);
   gtk_file_chooser_set_current_folder (FC_backup,   parametros.ruta_backup);
   gtk_file_chooser_set_current_folder (FC_figuras,  parametros.ruta_figuras);

   if (parametros.usa_figuras)
      {
       gtk_toggle_button_set_active(CK_figuras, TRUE);
       gtk_widget_set_sensitive(GTK_WIDGET(FC_figuras), 1);
      }
   else
      {
       gtk_toggle_button_set_active(CK_figuras, FALSE);
       gtk_widget_set_sensitive(GTK_WIDGET(FC_figuras), 0);
      }

   if (parametros.programa == 0) 
      gtk_toggle_button_set_active(RB_EX, TRUE);
   else
      if (parametros.programa == 1) 
         gtk_toggle_button_set_active(RB_EXT, TRUE);
      else
         if (parametros.programa == 2) 
            gtk_toggle_button_set_active(RB_EXZ, TRUE);
	 else
            gtk_toggle_button_set_active(RB_EXO, TRUE);
   Cambia_Interaccion ();

   if (parametros.decoracion == 0)
      {
       gtk_toggle_button_set_active(CK_aleatorio, TRUE);
       gtk_range_set_value (GTK_RANGE(SC_decoracion),  (gdouble) 1.0);
      }
   else
      {
       gtk_toggle_button_set_active(CK_aleatorio, FALSE);
       gtk_range_set_value (GTK_RANGE(SC_decoracion),  (gdouble) parametros.decoracion);
      }
   Cambia_Decoracion ();
   Cambia_Aleatorio  ();

   if (parametros.despliega_reportes == 1)
      gtk_toggle_button_set_active(CK_reportes, TRUE);
   else
      gtk_toggle_button_set_active(CK_reportes, FALSE);

   if (parametros.report_update == 1)
      gtk_toggle_button_set_active(CK_actualizacion, TRUE);
   else
      gtk_toggle_button_set_active(CK_actualizacion, FALSE);

   if (parametros.timeout == 0)
      {
       gtk_toggle_button_set_active(CK_timeout, FALSE);
       gtk_range_set_value (GTK_RANGE(SC_timeout),  (gdouble) 1.0);
      }
   else
      {
       gtk_toggle_button_set_active(CK_timeout, TRUE);
       gtk_range_set_value (GTK_RANGE(SC_timeout),  (gdouble) parametros.timeout);
      }

   gtk_combo_box_set_active    (CB_estilo,  parametros.Beamer_Estilo);
   gtk_combo_box_set_active    (CB_color,   parametros.Beamer_Color);
   gtk_combo_box_set_active    (CB_font,    parametros.Beamer_Font);
   gtk_combo_box_set_active    (CB_size,    parametros.Beamer_Size);
   gtk_combo_box_set_active    (CB_aspecto, parametros.Beamer_Aspecto);
}

void Toggle_usa_figuras(GtkWidget *widget, gpointer user_data)
{
  if (gtk_toggle_button_get_active(CK_figuras))
     gtk_widget_set_sensitive(GTK_WIDGET(FC_figuras), 1);
  else
     gtk_widget_set_sensitive(GTK_WIDGET(FC_figuras), 0);
}

void Imprime_listado (GtkWidget *widget, gpointer user_data)
{
   FILE * Archivo_Latex;
   char  hilera_antes [8000], hilera_despues [8000];
   gchar *Estilo, *Color, *Font, *Size, *Aspecto;
   char ruta_destino[3000];
   char ruta_latex[3000];

   Archivo_Latex = fopen ("EX1000.tex","w");
   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, twocolumn, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");

   fprintf (Archivo_Latex, "\\begin{document}\n");

   sprintf (hilera_antes, "\\title{Parámetros de Ejecución de \\textit{The Examiner}}\n");
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\\maketitle\n\n");

   fprintf (Archivo_Latex, "\\section{Directorios}\n");

   fprintf (Archivo_Latex, "\\subsection*{Ex\\'amenes}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\begin{verbatim}\n");
   fprintf (Archivo_Latex, "%s\n", parametros.ruta_examenes);
   fprintf (Archivo_Latex, "\\end{verbatim}\n");
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Reportes}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\begin{verbatim}\n");
   fprintf (Archivo_Latex, "%s\n", parametros.ruta_reportes);
   fprintf (Archivo_Latex, "\\end{verbatim}\n");
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Fuentes de Latex}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\begin{verbatim}\n");
   fprintf (Archivo_Latex, "%s\n", parametros.ruta_latex);
   fprintf (Archivo_Latex, "\\end{verbatim}\n");
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Fuentes de \\texttt{gnuplot}}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\begin{verbatim}\n");
   fprintf (Archivo_Latex, "%s\n", parametros.ruta_gnuplot);
   fprintf (Archivo_Latex, "\\end{verbatim}\n");
   fprintf (Archivo_Latex, "\\end{framed}\n");


   fprintf (Archivo_Latex, "\\subsection*{Respaldos}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\begin{verbatim}\n");
   fprintf (Archivo_Latex, "%s\n", parametros.ruta_backup);
   fprintf (Archivo_Latex, "\\end{verbatim}\n");
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Figuras}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\begin{verbatim}\n");
   fprintf (Archivo_Latex, "%s\n", parametros.ruta_figuras);
   fprintf (Archivo_Latex, "\\end{verbatim}\n");
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\section{Programas}\n");
   fprintf (Archivo_Latex, "\\subsection*{Convertir de Latex a PDF}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\texttt{%s}\n", parametros.latex_2_pdf);
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Despliegue de PDFs}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\texttt{%s}\n", parametros.despliega_pdf);
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Convertir de EPS a PDF}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\texttt{%s}\n", parametros.epstopdf);
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Comando \\texttt{gnuplot}}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\texttt{%s}\n", parametros.gnuplot);
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Despliegue de Consola}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\texttt{%s}\n", parametros.xterm);
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\section{Datos}\n");
   fprintf (Archivo_Latex, "\\subsection*{Instituci\\'on}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   hilera_LATEX (parametros.Institucion, hilera_despues);
   fprintf (Archivo_Latex, "\\textbf{%s}\n", hilera_despues);
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Escuela}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   hilera_LATEX (parametros.Escuela, hilera_despues);
   fprintf (Archivo_Latex, "\\textbf{%s}\n", hilera_despues);
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Programa}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   hilera_LATEX (parametros.Programa, hilera_despues);
   fprintf (Archivo_Latex, "\\textbf{%s}\n", hilera_despues);
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Instrucciones}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   hilera_LATEX (parametros.Instrucciones, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Juramento}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   hilera_LATEX (parametros.Juramento, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\section{C\\'odigo \\LaTeX}\n");
   fprintf (Archivo_Latex, "\\subsection*{Packages est\\'andar}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\begin{verbatim}\n");
   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\\end{verbatim}\n");
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\subsection*{Packages agregados}\n");
   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "\\begin{verbatim}\n");
   fprintf (Archivo_Latex, "%s\n", parametros.Paquetes);
   fprintf (Archivo_Latex, "\\end{verbatim}\n");
   fprintf (Archivo_Latex, "\\end{framed}\n");

   fprintf (Archivo_Latex, "\\section{Par\\'ametros de Beamer}\n");
   Estilo  = gtk_combo_box_get_active_text(CB_estilo);
   Color   = gtk_combo_box_get_active_text(CB_color);
   Font    = gtk_combo_box_get_active_text(CB_font);
   Size    = gtk_combo_box_get_active_text(CB_size);
   Aspecto = gtk_combo_box_get_active_text(CB_aspecto);

   fprintf (Archivo_Latex, "\\begin{framed}\n");
   fprintf (Archivo_Latex, "Estilo = \\texttt{%s}\n\n", Estilo);
   fprintf (Archivo_Latex, "Color = \\texttt{%s}\n\n", Color);
   fprintf (Archivo_Latex, "Font = \\texttt{%s}\n\n", Font);
   fprintf (Archivo_Latex, "Size = \\texttt{%s}\n\n", Size);
   fprintf (Archivo_Latex, "Aspecto = \\texttt{%s}\n\n", Aspecto);
   fprintf (Archivo_Latex, "\\end{framed}\n");

   g_free (Estilo);
   g_free (Color);
   g_free (Font);
   g_free (Size);
   g_free (Aspecto);

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX1000", 1, NULL, 0.0, 0.0, ruta_destino, ruta_latex);

   if (parametros.despliega_reportes)
      {
       sprintf (hilera_antes,"<span foreground=\"yellow\">%s</span>", ruta_destino); 
       gtk_label_set_markup(LB_destino_reporte, hilera_antes);
       sprintf (hilera_antes,"<span foreground=\"yellow\">%s</span>", ruta_latex); 
       gtk_label_set_markup(LB_destino_LATEX,   hilera_antes);

       gtk_widget_set_sensitive(window1, 0);
       gtk_widget_show (window9);
      }
}

void Actualiza_Parametros (GtkWidget *widget, gpointer user_data)
{
   int interaccion, decoracion, disp_reportes, actualizacion, timeout;
   gchar * latex_2_pdf, * despliega_pdf, * epstopdf, * gnuplot, * xterm;
   gchar * institucion, * escuela, * programa;
   gchar * examenes, * figuras, * reportes, * latex, * gnuplot_path, *backup;
   gchar *instrucciones, *juramento, *packages;
   char  *instrucciones_corregidas, *juramento_corregido, *packages_corregido;
   int Beamer_Estilo, Beamer_Color, Beamer_Font, Beamer_Size, Beamer_Aspecto;
   GtkTextIter start;
   GtkTextIter end;
   char PG_command [2000];
   PGresult *res;

   res = PQEXEC(DATABASE, "BEGIN");
   PQclear(res);

   /* INTERFAZ */
   if (gtk_toggle_button_get_active(RB_EX))  
      interaccion = 0;
   else
      if (gtk_toggle_button_get_active(RB_EXT))
 	 interaccion = 1;
      else
         if (gtk_toggle_button_get_active(RB_EXZ))
	    interaccion = 2;
	 else
	    interaccion = 3;

   if (gtk_toggle_button_get_active(CK_aleatorio))
      decoracion = 0;
   else
      decoracion = (int) gtk_range_get_value (GTK_RANGE(SC_decoracion));
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro=E'PROGRAMA=%d DECORACION=%d' where codigo_parametro = 'INTER'", interaccion, decoracion);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   if (gtk_toggle_button_get_active(CK_reportes))
      disp_reportes = 1;
   else
      disp_reportes = 0;
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%d' where codigo_parametro = 'DISPREP'", disp_reportes);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   if (gtk_toggle_button_get_active(CK_actualizacion))
      {
       actualizacion = 1;
       if (gtk_toggle_button_get_active(CK_timeout))
          timeout = (int) gtk_range_get_value (GTK_RANGE(SC_timeout));
       else
          timeout = 0;
      }
   else
      {
       actualizacion = 0;
       timeout = 0;
      }
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro=E'REPORT_UPDATE=%d TIMEOUT=%d' where codigo_parametro = 'UPDATE'", actualizacion, timeout);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   /* PROGRAMAS */
   latex_2_pdf = gtk_editable_get_chars(GTK_EDITABLE(EN_latex_2_pdf), 0, -1);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'LATEXPDF'", latex_2_pdf);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (latex_2_pdf);

   despliega_pdf = gtk_editable_get_chars(GTK_EDITABLE(EN_despliega_pdf), 0, -1);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'DISPPDF'", despliega_pdf);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (despliega_pdf);

   epstopdf = gtk_editable_get_chars(GTK_EDITABLE(EN_epstopdf), 0, -1);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'EPSTOPDF'", epstopdf);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (epstopdf);

   gnuplot = gtk_editable_get_chars(GTK_EDITABLE(EN_gnuplot), 0, -1);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'GNUPLOT'", gnuplot);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (gnuplot);

   xterm = gtk_editable_get_chars(GTK_EDITABLE(EN_xterm), 0, -1);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'XTERM'", xterm);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (xterm);

   /* DATOS */
   institucion = gtk_editable_get_chars(GTK_EDITABLE(EN_institucion), 0, -1);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'INSTITU'", institucion);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (institucion);

   escuela = gtk_editable_get_chars(GTK_EDITABLE(EN_escuela), 0, -1);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'ESCUELA'", escuela);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (escuela);

   programa = gtk_editable_get_chars(GTK_EDITABLE(EN_programa), 0, -1);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'PROGRAM'", programa);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (programa);

   gtk_text_buffer_get_start_iter (buffer_TV_instrucciones, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_instrucciones, &end);
   instrucciones = gtk_text_buffer_get_text (buffer_TV_instrucciones, &start, &end, FALSE);
   instrucciones_corregidas = (char *) malloc (sizeof(char) * strlen(instrucciones)*2);
   hilera_POSTGRES (instrucciones, instrucciones_corregidas);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro=E'%s' where codigo_parametro = 'INSTRUC'", instrucciones_corregidas);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   free   (instrucciones_corregidas);
   g_free (instrucciones);

   gtk_text_buffer_get_start_iter (buffer_TV_juramento, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_juramento, &end);
   juramento = gtk_text_buffer_get_text (buffer_TV_juramento, &start, &end, FALSE);
   juramento_corregido = (char *) malloc (sizeof(char) * strlen(juramento)*2);
   hilera_POSTGRES (juramento, juramento_corregido);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro=E'%s' where codigo_parametro = 'JURAM'", juramento_corregido);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   free   (juramento_corregido);
   g_free (juramento);

   /* PAQUETES DE LATEX */
   gtk_text_buffer_get_start_iter (buffer_TV_packages, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_packages, &end);
   packages = gtk_text_buffer_get_text (buffer_TV_packages, &start, &end, FALSE);
   packages_corregido = (char *) malloc (sizeof(char) * strlen(packages)*2);
   hilera_POSTGRES (packages, packages_corregido);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro=E'%s' where codigo_parametro = 'PACKA'", packages_corregido);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   free   (packages_corregido);
   g_free (packages);

   /* BEAMER */
   Beamer_Estilo  = gtk_combo_box_get_active(CB_estilo);
   Beamer_Color   = gtk_combo_box_get_active(CB_color);
   Beamer_Font    = gtk_combo_box_get_active(CB_font);
   Beamer_Size    = gtk_combo_box_get_active(CB_size);
   Beamer_Aspecto = gtk_combo_box_get_active(CB_aspecto);
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro=E'ESTILO=%d COLOR=%d FONT=%d SIZE=%d ASPECTO=%d' where codigo_parametro = 'BEAMER'", Beamer_Estilo, Beamer_Color, Beamer_Font, Beamer_Size, Beamer_Aspecto);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   /* DIRECTORIOS */
   examenes =  gtk_editable_get_chars(GTK_EDITABLE(EN_examenes), 0, -1);   
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'EXAMEN'", examenes);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (examenes);

   figuras =  gtk_editable_get_chars(GTK_EDITABLE(EN_figuras), 0, -1);   
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'FIGURES'", figuras);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (figuras);

   if (gtk_toggle_button_get_active(CK_figuras))
      res = PQEXEC(DATABASE, "UPDATE pa_parametros SET hilera_parametro='1' where codigo_parametro = 'USAFIG'");
   else
      res = PQEXEC(DATABASE, "UPDATE pa_parametros SET hilera_parametro='0' where codigo_parametro = 'USAFIG'");
   PQclear(res);

   reportes =  gtk_editable_get_chars(GTK_EDITABLE(EN_reportes), 0, -1);   
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'REPORTES'", reportes);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (reportes);

   latex =  gtk_editable_get_chars(GTK_EDITABLE(EN_latex), 0, -1);   
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'LATEX'", latex);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (latex);

   gnuplot_path =  gtk_editable_get_chars(GTK_EDITABLE(EN_gnuplot_path), 0, -1);   
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'PATHGNUP'", gnuplot_path);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (gnuplot_path);

   backup =  gtk_editable_get_chars(GTK_EDITABLE(EN_backup), 0, -1);   
   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro='%s' where codigo_parametro = 'BACKUP'", backup);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);
   g_free (backup);

   res = PQEXEC(DATABASE, "UPDATE pa_parametros SET hilera_parametro='1' where codigo_parametro = 'CONFIG'");
   PQclear(res);

   res = PQEXEC(DATABASE, "END");
   PQclear(res);

   carga_parametros_EXAMINER (&parametros, DATABASE);
   if (parametros.report_update)
      {
       gtk_widget_set_sensitive(window1, 0);
       gtk_widget_show (window8);

       if (parametros.timeout)
	  {
           while (gtk_events_pending ()) gtk_main_iteration ();
           catNap(parametros.timeout);
           gtk_widget_hide (window8);
           gtk_widget_set_sensitive(window1, 1);
	  }
      }
}

void Cambia_Decoracion ()
{
  int k;
  char hilera[100];

  k = (int) gtk_range_get_value (GTK_RANGE(SC_decoracion));

  sprintf (hilera,".imagenes/EX-BG-%02d-x.jpg", k);
  gtk_image_set_from_file (IM_decoracion, hilera);
}

void Cambia_Interaccion ()
{
  gtk_widget_set_sensitive(FR_EX, 0);
  gtk_widget_set_sensitive(FR_EXT, 0);
  gtk_widget_set_sensitive(FR_EXZ, 0);
  gtk_widget_set_sensitive(FR_EXO, 0);

  if (gtk_toggle_button_get_active(RB_EX))  gtk_widget_set_sensitive(FR_EX,  1);
  if (gtk_toggle_button_get_active(RB_EXT)) gtk_widget_set_sensitive(FR_EXT, 1);
  if (gtk_toggle_button_get_active(RB_EXZ)) gtk_widget_set_sensitive(FR_EXZ, 1);
  if (gtk_toggle_button_get_active(RB_EXO)) gtk_widget_set_sensitive(FR_EXO, 1);
}

void Cambia_Aleatorio ()
{
  if (gtk_toggle_button_get_active(CK_aleatorio))
     {
      gtk_widget_set_sensitive(SC_decoracion, 0);
      gtk_widget_set_sensitive(FR_background, 0);
     }
  else
     {
      gtk_widget_set_sensitive(SC_decoracion, 1);
      gtk_widget_set_sensitive(FR_background, 1);
     }
}

void Cambia_Actualizacion ()
{
  if (gtk_toggle_button_get_active(CK_actualizacion))
     {
      gtk_widget_set_sensitive(GTK_WIDGET(CK_timeout), 1);
     }
  else
     {
      gtk_widget_set_sensitive(GTK_WIDGET(CK_timeout), 0);
      gtk_toggle_button_set_active(CK_timeout, FALSE);
     }

  Cambia_Timeout ();
}

void Cambia_Timeout ()
{
  if (gtk_toggle_button_get_active(CK_timeout))
     {
      gtk_widget_set_sensitive(SC_timeout, 1);
     }
  else
     {
      gtk_widget_set_sensitive(SC_timeout, 0);
      gtk_range_set_value (GTK_RANGE(SC_timeout),  (gdouble) 1.0);
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

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex1000_destroy (GtkWidget *widget, gpointer user_data) 
{
  Fin_Ventana (widget, user_data);
}

void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data)
{
  Fin_de_Programa (widget, user_data);
}

void on_BN_undo_clicked(GtkWidget *widget, gpointer user_data)
{
  carga_parametros_EXAMINER (&parametros, DATABASE);
  Init_Fields ();
}

void on_BN_save_clicked(GtkWidget *widget, gpointer user_data)
{
  Actualiza_Parametros (widget, user_data);
}

void on_BN_print_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_listado (widget, user_data);
}

void on_CK_figuras_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_usa_figuras(widget, user_data);
}

void on_FC_examenes_current_folder_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * examenes;

  examenes = gtk_file_chooser_get_filename(FC_examenes);
  gtk_entry_set_text  (GTK_ENTRY(EN_examenes), examenes);
  g_free (examenes);
}

void on_FC_reportes_current_folder_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * reportes;

  reportes = gtk_file_chooser_get_filename(FC_reportes);
  gtk_entry_set_text  (GTK_ENTRY(EN_reportes), reportes);
  g_free (reportes);
}

void on_FC_latex_current_folder_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * latex;

  latex = gtk_file_chooser_get_filename(FC_latex);
  gtk_entry_set_text  (GTK_ENTRY(EN_latex), latex);
  g_free (latex);
}

void on_FC_gnuplot_current_folder_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * gnuplot;

  gnuplot = gtk_file_chooser_get_filename(FC_gnuplot);
  gtk_entry_set_text  (GTK_ENTRY(EN_gnuplot_path), gnuplot);
  g_free (gnuplot);
}

void on_FC_backup_current_folder_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * backup;

  backup = gtk_file_chooser_get_filename(FC_backup);
  gtk_entry_set_text  (GTK_ENTRY(EN_backup), backup);
  g_free (backup);
}

void on_FC_figuras_current_folder_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * figuras;

  figuras = gtk_file_chooser_get_filename(FC_figuras);
  gtk_entry_set_text  (GTK_ENTRY(EN_figuras), figuras);
  g_free (figuras);
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window8);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_interfaz_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_set_sensitive(window1, 0);

  gtk_widget_show (window2);  
}

void on_BN_directorios_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_set_sensitive(window1, 0);

  gtk_widget_show (window3);  
}

void on_BN_programas_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_set_sensitive(window1, 0);

  gtk_widget_show (window4);  
}

void on_BN_datos_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_set_sensitive(window1, 0);

  gtk_widget_show (window5);  
}

void on_BN_latex_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_set_sensitive(window1, 0);

  gtk_widget_show (window6);  
}

void on_BN_beamer_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_set_sensitive(window1, 0);

  gtk_widget_show (window7);  
}

void on_BN_ok1_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window3);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_ok2_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window4);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_ok3_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window5);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_ok4_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window6);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_ok5_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_ok6_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window7);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_ok7_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window9);
  gtk_widget_set_sensitive(window1, 1);
}

void on_SC_decoracion_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_Decoracion ();
}

void on_RB_interaccion_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambia_Interaccion ();
}

void on_CK_aleatorio_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambia_Aleatorio ();
}

void on_CK_actualizacion_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambia_Actualizacion ();
}

void on_CK_timeout_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambia_Timeout ();
}
