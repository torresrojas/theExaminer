/*

Compilar con:

cc -o EX2030 EX2030.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX2030:                                */
/*                                         */
/*    Actualización Archivo de Ejercicios  */
/*    The Examiner 0.2                     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    28 de Junio del 2011                 */
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    12 de Diciembre 2011                 */
/*******************************************/
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <time.h>
#include <libpq-fe.h>
#include <locale.h>
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


GtkSpinButton *SP_codigo          = NULL;
GtkWidget *SC_codigo              = NULL;
GtkWidget *BN_ok                  = NULL;

GtkWidget *EB_ejercicio           = NULL;
GtkWidget *FR_ejercicio           = NULL;

GtkComboBox *CB_materia           = NULL;
GtkComboBox *CB_tema              = NULL;
GtkComboBox *CB_subtema           = NULL;
GtkComboBox *CB_autor             = NULL;
GtkComboBox *CB_Bloom             = NULL;
GtkWidget *EN_descripcion         = NULL;
GtkToggleButton * TG_objetivo [8];
GtkToggleButton *CK_reutilizable  = NULL;
GtkWidget *FR_header              = NULL;
GtkToggleButton *CK_header        = NULL;
GtkTextView *TV_header            = NULL;
GtkWidget *BN_extend_header       = NULL;

GtkWidget *FR_botones             = NULL;
GtkWidget *BN_save                = NULL;
GtkWidget *BN_print               = NULL;

GtkWidget *BN_undo                = NULL;
GtkWidget *BN_delete              = NULL;
GtkWidget *BN_terminar            = NULL;

GtkTextBuffer * buffer_TV_header;
GtkWidget *BN_ok_1                = NULL;
GtkWidget *BN_ok_2                = NULL;

GtkWidget *FR_update              = NULL;
GtkWidget *EB_update              = NULL;
GtkWidget *FR_delete              = NULL;
GtkWidget *EB_delete              = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Activa_Objetivos                      ();
void Borra_Registro                        (GtkWidget *widget, gpointer user_data);
void Cambio_Autor                          (GtkWidget *widget, gpointer user_data);
void Cambio_Codigo                         (GtkWidget *widget, gpointer user_data);
void Cambio_Descripcion                    (GtkWidget *widget, gpointer user_data);
void Cambio_Materia                        (GtkWidget *widget, gpointer user_data);
void Cambio_Subtema                        (GtkWidget *widget, gpointer user_data);
void Cambio_Tema                           (GtkWidget *widget, gpointer user_data);
void Carga_Autor                           (char * autor);
void Carga_materias_temas_subtemas_autores ();
void Carga_MTS                             (char * materia, char * tema, char * subtema);
void Carga_Registro                        (PGresult *res);
void Connect_widgets                       ();
void Fin_de_Programa                       (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                           (GtkWidget *widget, gpointer user_data);
void Graba_Registro                        (GtkWidget *widget, gpointer user_data);
void Imprime_Encabezado                    (GtkWidget *widget, gpointer user_data);
void Init_Fields                           ();
void Interface_Coloring                    ();
void on_BN_delete_clicked                  (GtkWidget *widget, gpointer user_data);
void on_BN_print_clicked                   (GtkWidget *widget, gpointer user_data);
void on_BN_save_clicked                    (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked                (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked                    (GtkWidget *widget, gpointer user_data);
void on_CB_materia_changed                 (GtkWidget *widget, gpointer user_data);
void on_CB_subtema_changed                 (GtkWidget *widget, gpointer user_data);
void on_CB_tema_changed                    (GtkWidget *widget, gpointer user_data);
void on_EN_descripcion_activate            (GtkWidget *widget, gpointer user_data);
void on_WN_ex2030_destroy                  (GtkWidget *widget, gpointer user_data);
void Procesa_Ejercicio                     (GtkWidget *widget, gpointer user_data);
void Toggle_header                         (GtkWidget *widget, gpointer user_data);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define CODIGO_SIZE              7
#define AUTOR_SIZE               11

int N_materias = 0;
int N_temas    = 0;
int N_subtemas = 0;
int N_autores  = 0;
int N_objetivos= 0;

int Materia_default;
int Tema_default;
int Subtema_default;
int Autor_default;

int REGISTRO_YA_EXISTE   = 0;

char Codigo_texto   [7];
char Ultimo_texto   [7];
char Codigo_Materia [CODIGO_MATERIA_SIZE + 1];
char Codigo_Tema    [CODIGO_TEMA_SIZE    + 1];
char Codigo_Subtema [CODIGO_SUBTEMA_SIZE + 1];
char Codigo_Persona [AUTOR_SIZE];
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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX2030.glade", &error))
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
	     Connect_widgets ();

	     /* The magic of color...  */
	     Interface_Coloring ();

	     /* Displays main window */
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
  int i;
  char hilera[50];

  window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex2030"));
  window2                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));
  window3                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_delete"));

  SP_codigo              = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_codigo"));
  SC_codigo              = GTK_WIDGET (gtk_builder_get_object (builder, "SC_codigo"));
  BN_ok                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));

  EB_ejercicio      = GTK_WIDGET (gtk_builder_get_object (builder, "EB_ejercicio"));
  FR_ejercicio      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_ejercicio"));

  CB_materia             = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_materia"));
  CB_tema                = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_tema"));
  CB_subtema             = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_subtema"));
  CB_autor               = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_autor"));
  CB_Bloom               = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_Bloom"));
  EN_descripcion         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));

  for (i=0;i<8;i++)
      {
       sprintf (hilera,"TG_objetivo%1d", i+1);
       TG_objetivo [i] = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, hilera));
      }

  CK_reutilizable = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_reutilizable"));
  CK_header       = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_header"));
  TV_header       = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_header"));
  FR_header       = GTK_WIDGET (gtk_builder_get_object (builder, "FR_header"));
  BN_extend_header= GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_header"));
  FR_botones      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  BN_print        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_save         = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
  BN_delete       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_delete"));
  BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_undo                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_ok_1                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok_1"));
  EB_update         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
  FR_update         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));
  BN_ok_2                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok_2"));
  EB_delete         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_delete"));
  FR_delete         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_delete"));

  buffer_TV_header = gtk_text_view_get_buffer(TV_header);
}

void Interface_Coloring ()
{
  GdkColor color;
  int i;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_ejercicio, GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_botones,  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_ejercicio,  GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_codigo), GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_descripcion, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_header,      GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_header, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,      GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_delete,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok_1,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok_2,          GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_header, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar,      GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,          GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_delete,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,          GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok_1,          GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok_2,          GTK_STATE_ACTIVE, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_delete,              GTK_STATE_NORMAL, &color);

  gdk_color_parse (FINISHED_WORK_FR    , &color);
  gtk_widget_modify_bg(FR_delete,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_update,              GTK_STATE_NORMAL, &color);

  gdk_color_parse (SPECIAL_BUTTON_PRELIGHT, &color);
  for (i=0;i<8;i++) gtk_widget_modify_bg(GTK_WIDGET(TG_objetivo[i]),GTK_STATE_PRELIGHT, &color);
  gdk_color_parse (SPECIAL_BUTTON_ACTIVE, &color);
  for (i=0;i<8;i++) gtk_widget_modify_bg(GTK_WIDGET(TG_objetivo[i]),GTK_STATE_ACTIVE, &color);
}

void Init_Fields()
{
   int  i, Last;
   char Codigo [CODIGO_SIZE];
   char text   [2];
   char hilera [500];
   PGresult *res;
   gchar * materia;

   Carga_materias_temas_subtemas_autores ();

   /* Recupera el ejercicio con código más alto - supuestamente este fue el último en ser ingresado */
   res = PQEXEC(DATABASE, "SELECT codigo_ejercicio, materia, tema, subtema, texto_ejercicio, consecutivo_texto, autor from BD_ejercicios, BD_texto_ejercicios where consecutivo_texto = texto_ejercicio order by codigo_ejercicio DESC limit 1");

   if (PQntuples(res))
      {
       Last = atoi (PQgetvalue (res, 0, 0));
       sprintf (Codigo,"%06d",Last+1);

       Carga_MTS   (PQgetvalue (res, 0, 1), PQgetvalue (res, 0, 2), PQgetvalue (res, 0, 3));
       Carga_Autor (PQgetvalue (res, 0, 6));

       Materia_default = gtk_combo_box_get_active(CB_materia);
       Tema_default    = gtk_combo_box_get_active(CB_tema);
       Subtema_default = gtk_combo_box_get_active(CB_subtema);
       Autor_default   = gtk_combo_box_get_active(CB_autor);
      }
   else
      {
       Last = 0;
       sprintf (Codigo,       "000001");

       gtk_combo_box_set_active (CB_materia, 0);
       gtk_combo_box_set_active (CB_autor,   0);
      }
   PQclear(res);

   materia = gtk_combo_box_get_active_text(CB_materia);
   i=0;
   while (materia[i] != ' ') 
         {
          Codigo_Materia[i] = materia[i];
          i++;
         }
   Codigo_Materia[i]='\0';
   g_free(materia);

   Last++; /* Siguiente ejercicio */

   if (Last > 1)
      {
       gtk_widget_set_sensitive(GTK_WIDGET(SC_codigo), 1);
       gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo), 1);
       gtk_range_set_range (GTK_RANGE(SC_codigo),  (gdouble) 1.0, (gdouble) Last);
       gtk_spin_button_set_range (SP_codigo, 1.0, (long double) Last);
      }
   else
      { /* Primer ejercicio que se registra */
       gtk_widget_set_sensitive(GTK_WIDGET(SC_codigo), 0);
       gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo), 0);
       gtk_range_set_range (GTK_RANGE(SC_codigo),  (gdouble) 0.0, (gdouble) Last);
       gtk_spin_button_set_range (SP_codigo, 0.0, (long double) Last);
      }
   gtk_range_set_value (GTK_RANGE(SC_codigo),  (gdouble) 0);
   gtk_range_set_value (GTK_RANGE(SC_codigo),  (gdouble) Last);

   res = PQEXEC(DATABASE, "SELECT consecutivo_texto from BD_texto_ejercicios order by consecutivo_texto DESC limit 1");
   if (PQntuples(res))
      {
       Last = atoi (PQgetvalue (res, 0, 0));
       sprintf (Codigo_texto,"%06d",Last+1);
       sprintf (Ultimo_texto,"%06d",Last+1);
      }
   else
      {
       sprintf (Codigo_texto, "000001");
       sprintf (Ultimo_texto, "000001");
      }
   PQclear(res);

   N_objetivos = 0;

   gtk_widget_set_sensitive(BN_ok, 1);

   gtk_combo_box_set_active (CB_Bloom, 0);

   for (i=0;i<8;i++) gtk_toggle_button_set_active(TG_objetivo[i]   , FALSE);     

   gtk_toggle_button_set_active(CK_reutilizable, TRUE);
   gtk_toggle_button_set_active(CK_header      , FALSE);
 
   gtk_widget_set_sensitive(GTK_WIDGET(CB_materia), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CB_tema)   , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CB_subtema), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CB_autor)  , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CB_Bloom)  , 0);

   for (i=0;i<8;i++) gtk_widget_set_sensitive(GTK_WIDGET(TG_objetivo[i]), 0);

   gtk_widget_set_sensitive(GTK_WIDGET(CK_reutilizable), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(CK_header)      , 0);

   text[0] = '\0';
   gtk_text_buffer_set_text(buffer_TV_header, text, -1);
   gtk_widget_set_sensitive(BN_extend_header,   0);

   gtk_widget_set_sensitive(BN_save,   0);
   gtk_widget_set_sensitive(BN_print,  0);
   gtk_widget_set_sensitive(BN_undo,   1);
   gtk_widget_set_sensitive(BN_delete, 0);

   REGISTRO_YA_EXISTE = 0;

   gtk_widget_set_can_focus(EN_descripcion,        FALSE);
   gtk_widget_set_can_focus(GTK_WIDGET(TV_header), FALSE);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_header), 0);

   gtk_widget_grab_focus (GTK_WIDGET(SP_codigo));
}

void Carga_materias_temas_subtemas_autores ()
{
   int i;
   PGresult *res;
   char hilera[100];

   gtk_combo_box_set_active (CB_materia, -1);
   for (i=0; i < N_materias; i++)
       {
	gtk_combo_box_remove_text (CB_materia, 0);
       }

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

   gtk_combo_box_set_active (CB_autor, -1);
   for (i=0; i<N_autores; i++)
       {
	gtk_combo_box_remove_text (CB_autor, 0);
       }

   res = PQEXEC(DATABASE,"SELECT codigo_materia, descripcion_materia from bd_materias where codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_materia"); 
   N_materias = PQntuples(res);
   for (i=0; i < N_materias; i++)
       {
	sprintf (hilera,"%s %s",PQgetvalue (res, i, 0),PQgetvalue (res, i, 1));
        gtk_combo_box_append_text(CB_materia, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_materia, -1);

   res = PQEXEC(DATABASE,"SELECT * from bd_personas order by codigo_persona"); 
   N_autores = PQntuples(res);
   for (i=0; i < N_autores; i++)
       {
	sprintf (hilera,"%s %s",PQgetvalue (res, i, 0),PQgetvalue (res, i, 1));
        gtk_combo_box_append_text(CB_autor, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_autor, -1);
}

void Cambia_Codigo ()
{
  long int i, k;
  char codigo[10];
  char PG_command [1000];
  char text   [2];
  PGresult *res;

  k = (long int) gtk_range_get_value (GTK_RANGE(SC_codigo));
  sprintf (codigo, "%06ld", k);
  gtk_spin_button_set_value (SP_codigo, k);

  sprintf (PG_command,"SELECT * from BD_ejercicios where codigo_ejercicio = '%s'", codigo);
  res = PQEXEC(DATABASE, PG_command);

  if (PQntuples(res))
     {
      REGISTRO_YA_EXISTE = 1;

      Carga_Registro (res);
     }
  else
     {      
      REGISTRO_YA_EXISTE = 0;

      gtk_entry_set_text(GTK_ENTRY(EN_descripcion), "* EJERCICIO NUEVO *");
      gtk_widget_set_sensitive(BN_delete, 0);

      gtk_combo_box_set_active (CB_materia, Materia_default);
      gtk_combo_box_set_active (CB_tema, Tema_default);
      gtk_combo_box_set_active (CB_subtema, Subtema_default);
      gtk_combo_box_set_active (CB_autor, Autor_default);
      gtk_toggle_button_set_active(CK_reutilizable, TRUE);
      gtk_toggle_button_set_active(CK_header,       FALSE);
      for (i=0;i<N_objetivos;i++)
	  {
           gtk_toggle_button_set_active(TG_objetivo[i], FALSE);
           gtk_widget_set_sensitive(GTK_WIDGET(TG_objetivo[i]), 0);
	  }

      strcpy (Codigo_texto, Ultimo_texto);
      text[0] = '\0';
      gtk_text_buffer_set_text(buffer_TV_header, text, -1);
     }

  PQclear(res);
}

void Procesa_Ejercicio (GtkWidget *widget, gpointer user_data)
{
  int i;
  long int k;
  char PG_command [1000];
  PGresult *res, *res_aux;
  char codigo [CODIGO_SIZE];
  int contador_referencias;

  k = (long int) gtk_range_get_value (GTK_RANGE(SC_codigo));
  sprintf (codigo, "%06ld", k);

  gtk_widget_set_sensitive(GTK_WIDGET(SC_codigo), 0);
  gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo), 0);
  gtk_widget_set_sensitive(BN_ok, 0);

  gtk_widget_set_can_focus(EN_descripcion,        TRUE);
  gtk_widget_set_can_focus(GTK_WIDGET(TV_header), TRUE);

  gtk_widget_set_sensitive(GTK_WIDGET(CB_materia),      1);
  gtk_widget_set_sensitive(GTK_WIDGET(CB_tema),         1);
  gtk_widget_set_sensitive(GTK_WIDGET(CB_subtema),      1);
  gtk_widget_set_sensitive(GTK_WIDGET(CB_autor),        1);
  gtk_widget_set_sensitive(GTK_WIDGET(CB_Bloom),        1);
  gtk_widget_set_sensitive(GTK_WIDGET(CK_reutilizable), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(CK_header),       1);

  if (gtk_toggle_button_get_active(CK_header))
     {
      gtk_widget_set_sensitive(GTK_WIDGET(TV_header), 1);
      gtk_widget_set_sensitive(BN_extend_header,  1);
      gtk_widget_set_sensitive(BN_print, 1);
     }
  else
     {
      gtk_widget_set_sensitive(GTK_WIDGET(TV_header), 0);
      gtk_widget_set_sensitive(BN_extend_header,  0);
      gtk_widget_set_sensitive(BN_print,  0);
     }

  gtk_widget_set_sensitive(BN_delete,  0);

  if (REGISTRO_YA_EXISTE)
     {
      sprintf (PG_command,"SELECT ejercicio from ex_pre_examenes_lineas where ejercicio = '%s'", codigo);
      res = PQEXEC(DATABASE, PG_command);

      if (!PQntuples(res)) /* Ejercicio no aparece en ningún pre-examen, ni examen */
         {
          sprintf (PG_command,"SELECT contador_referencias from bd_texto_ejercicios where consecutivo_texto = '%s'", Codigo_texto);
          res_aux = PQEXEC(DATABASE, PG_command);
          contador_referencias = atoi(PQgetvalue (res_aux, 0, 0));
          PQclear(res_aux);

	  if (contador_referencias > 1)
	    /* Si las preguntas son compartidas con otros ejercicios,
               este puede ser borrado */
             gtk_widget_set_sensitive(BN_delete, 1);
	  else
	     { 
              sprintf (PG_command,"SELECT * from bd_texto_preguntas where codigo_consecutivo_ejercicio = '%s'", Codigo_texto);
              res_aux = PQEXEC(DATABASE, PG_command);
	      /* Si solo queda un ejercicio, las preguntas deben borrarse primero */
              if (!PQntuples(res_aux)) gtk_widget_set_sensitive(BN_delete, 1);
              PQclear(res_aux);
	     }
         }
     }

  Activa_Objetivos ();
  gtk_widget_grab_focus   (GTK_WIDGET(EN_descripcion));
  gtk_widget_set_sensitive(BN_save,   1);
}

void Activa_Objetivos()
{
  int i;
  char PG_command[1000];
  PGresult *res_aux;

  for (i=0;i<8;i++) gtk_widget_set_sensitive(GTK_WIDGET(TG_objetivo[i]), 0);

  sprintf (PG_command,"SELECT N_objetivos from bd_objetivos where materia = '%s'", Codigo_Materia); 

  res_aux = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res_aux))
     {
      N_objetivos = atoi(PQgetvalue (res_aux, 0, 0));
      for (i=0;i<N_objetivos;i++) gtk_widget_set_sensitive(GTK_WIDGET(TG_objetivo[i]), 1);
     }
  else
     N_objetivos = 0;
     
  PQclear(res_aux);
}

void Carga_Registro (PGresult *res)
{
  int i,j,k;
  char PG_command [1000];
  PGresult *res_aux, *res_autores;

  Carga_MTS   (PQgetvalue (res, 0, 1), PQgetvalue (res, 0, 2), PQgetvalue (res, 0, 3));
 
  if (*PQgetvalue (res, 0, 7) == 't') 
     gtk_toggle_button_set_active(CK_reutilizable, TRUE);
  else
     gtk_toggle_button_set_active(CK_reutilizable, FALSE);

  strcpy (Codigo_texto, PQgetvalue (res, 0, 9));

  for (i=0;i<8;i++)
      {
       if (*PQgetvalue (res, 0, 10+i) == '1') 
          gtk_toggle_button_set_active(TG_objetivo[i], TRUE);
       else
          gtk_toggle_button_set_active(TG_objetivo[i], FALSE);
      }

  sprintf (PG_command,"SELECT * from bd_texto_ejercicios where consecutivo_texto = '%s'", Codigo_texto);
  res_aux = PQEXEC(DATABASE, PG_command);
  gtk_entry_set_text(GTK_ENTRY(EN_descripcion), PQgetvalue (res_aux, 0, 1));

  Carga_Autor (PQgetvalue (res_aux, 0, 3));

  i=atoi( PQgetvalue (res, 0, 18)); 
  gtk_combo_box_set_active (CB_Bloom, i);

  if (*PQgetvalue (res_aux, 0, 4) == 't') 
     gtk_toggle_button_set_active(CK_header, TRUE);
  else
     gtk_toggle_button_set_active(CK_header, FALSE);

  if (gtk_toggle_button_get_active(CK_header))
     {
      gtk_text_buffer_set_text(buffer_TV_header, PQgetvalue (res_aux, 0, 5), -1);
      gtk_widget_set_sensitive(GTK_WIDGET(TV_header), 1);
      gtk_widget_set_sensitive(BN_extend_header,  1);
     }

  PQclear(res_aux);

  for (i=0;i<N_objetivos;i++) gtk_widget_set_sensitive(GTK_WIDGET(TG_objetivo[i]), 0);
  gtk_widget_set_sensitive(BN_extend_header,  0);
  gtk_widget_set_sensitive(BN_print,  0);
  gtk_widget_set_sensitive(BN_delete,  0);
}

void Carga_MTS (char * materia, char * tema, char * subtema)
{
  int i,j,k;
  PGresult *res_aux;
  char PG_command[1000];

  sprintf (PG_command,"SELECT * from bd_materias where codigo_materia <= '%s' and codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_materia",
                      materia); 
  res_aux = PQEXEC(DATABASE, PG_command);
  i = PQntuples(res_aux);
  PQclear(res_aux);

  sprintf (PG_command,"SELECT orden from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", 
	              materia, tema);
  res_aux = PQEXEC(DATABASE, PG_command);
  j = atoi (PQgetvalue (res_aux, 0, 0));
  PQclear(res_aux);

  sprintf (PG_command,"SELECT orden from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'",
	              materia, tema, subtema);
  res_aux = PQEXEC(DATABASE, PG_command);
  k = atoi (PQgetvalue (res_aux, 0, 0));
  PQclear(res_aux);

  gtk_combo_box_set_active (CB_materia, -1);
  gtk_combo_box_set_active (CB_materia, i-1);
  gtk_combo_box_set_active (CB_tema,    -1);
  gtk_combo_box_set_active (CB_tema,    j-1);
  gtk_combo_box_set_active (CB_subtema, -1);
  gtk_combo_box_set_active (CB_subtema, k-1);
}

void Carga_Autor (char * autor)
{
  int i;
  PGresult *res_autores;
  char PG_command[1000];

  sprintf (PG_command,"SELECT * from bd_personas where codigo_persona <= '%s' order by codigo_persona", autor); 
  res_autores = PQEXEC(DATABASE, PG_command);
  i = PQntuples(res_autores);
  PQclear(res_autores);
  gtk_combo_box_set_active (CB_autor, i-1);
}

void Cambio_Materia(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *materia;
  char PG_command  [1000];
  char hilera[500];
  PGresult *res;

  materia = gtk_combo_box_get_active_text(CB_materia);

  i=0;
  while (materia[i] != ' ') 
        {
         Codigo_Materia[i] = materia[i];
	 i++;
	}
  Codigo_Materia[i]='\0';

  gtk_combo_box_set_active (CB_tema, -1);
  for (i=0; i<N_temas; i++) gtk_combo_box_remove_text (CB_tema, 0);

  sprintf (PG_command,"SELECT codigo_tema, descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema != '          ' and codigo_subtema = '          ' order by orden", Codigo_Materia); 
  res = PQEXEC(DATABASE, PG_command);
  N_temas = PQntuples(res);
  for (i=0; i < N_temas; i++)
      {
       sprintf(hilera,"%s %s", PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
       gtk_combo_box_append_text(CB_tema, hilera);
      }
  PQclear(res);

  for (i=0;i<8;i++) gtk_toggle_button_set_active(TG_objetivo[i], FALSE);
  for (i=0;i<8;i++) gtk_widget_set_sensitive(GTK_WIDGET(TG_objetivo[i]), 0);

  sprintf (PG_command,"SELECT N_objetivos from bd_objetivos where materia = '%s'", Codigo_Materia); 
  res = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res))
     {
      N_objetivos = atoi(PQgetvalue (res, 0, 0));

      for (i=0;i<N_objetivos;i++) gtk_widget_set_sensitive(GTK_WIDGET(TG_objetivo[i]), 1);
     }
  else
     N_objetivos = 0;
     
  PQclear(res);

  gtk_combo_box_set_active (CB_tema, 0);

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
  i=0;
  while (tema[i] != ' ') 
        {
         Codigo_Tema[i] = tema[i];
	 i++;
	}
  Codigo_Tema[i]='\0';

  for (i=0; i<N_subtemas; i++) gtk_combo_box_remove_text (CB_subtema, 0);

  sprintf (PG_command,"SELECT codigo_subtema, descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' order by orden",
                      Codigo_Materia, Codigo_Tema); 
  res = PQEXEC(DATABASE, PG_command);
  N_subtemas = PQntuples(res);

  for (i=0; i < N_subtemas; i++)
      {
       sprintf (hilera,"%s %s", PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
       gtk_combo_box_append_text(CB_subtema, hilera);
      }
  PQclear(res);

  gtk_combo_box_set_active (CB_subtema, 0);

  g_free (tema);
}

void Cambio_Subtema(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *subtema = NULL;
  char PG_command  [1000];

  subtema = gtk_combo_box_get_active_text(CB_subtema);

  i=0;
  while (subtema[i] != ' ') 
        {
         Codigo_Subtema[i] = subtema[i];
	 i++;
	}
  Codigo_Subtema[i]='\0';

  g_free (subtema);
}

void Cambio_Autor(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *autor = NULL;
  char PG_command  [1000];

  autor = gtk_combo_box_get_active_text(CB_autor);
  i=0;
  while (autor[i] != ' ') 
        {
         Codigo_Persona[i] = autor[i];
	 i++;
	}
  Codigo_Persona[i]='\0';

  g_free (autor);
}

void Cambio_Descripcion(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus   (EN_descripcion);
}

void Toggle_header(GtkWidget *widget, gpointer user_data)
{
  char text[2];

  if (gtk_toggle_button_get_active(CK_header))
     {
      gtk_widget_set_sensitive(GTK_WIDGET(TV_header), 1);
      gtk_widget_set_sensitive(BN_print,  1);
      gtk_widget_set_sensitive(BN_extend_header,  1);
     }
  else
     {
      text[0] = '\0';
      gtk_text_buffer_set_text(buffer_TV_header, text, -1);
      gtk_widget_set_sensitive(GTK_WIDGET(TV_header), 0);
      gtk_widget_set_sensitive(BN_print,  0);
      gtk_widget_set_sensitive(BN_extend_header,  0);
     }
}

void Graba_Registro (GtkWidget *widget, gpointer user_data)
{
   long int k;
   GtkTextIter start;
   GtkTextIter end;
   char codigo [CODIGO_SIZE];
   gchar *descripcion, *text_header;
   char * header_corregido;
   char PG_command [9000], hilera[20];
   long int Last;
   time_t curtime;
   struct tm *loctime;
   int month, year, day;
   PGresult *res, *res_aux;

   /* Get the current time.  */
   curtime = time (NULL);
   /* Convert it to local time representation.  */
   loctime = localtime (&curtime);

   k = (long int) gtk_range_get_value (GTK_RANGE(SC_codigo));
   sprintf (codigo, "%06ld", k);
   descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion), 0, -1);

   header_corregido = NULL;
   if (gtk_toggle_button_get_active(CK_header))
      {
       gtk_text_buffer_get_start_iter (buffer_TV_header, &start);
       gtk_text_buffer_get_end_iter   (buffer_TV_header, &end);
       text_header      = gtk_text_buffer_get_text (buffer_TV_header, &start, &end, FALSE);
       header_corregido = (char *) malloc (sizeof(char) * strlen(text_header)*2);
       hilera_POSTGRES (text_header, header_corregido);
      }
 
   if (REGISTRO_YA_EXISTE)
      { /* Ejercicio y Texto de Ejercicio ya existen */
       res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

       sprintf (PG_command,"UPDATE bd_ejercicios SET Materia='%s', Tema='%s', Subtema='%s', automatico=%s, objetivo1='%s', objetivo2='%s', objetivo3='%s', objetivo4='%s', objetivo5='%s', objetivo6='%s', objetivo7='%s', objetivo8='%s', Bloom = %d where codigo_ejercicio = '%.6s'", 
		Codigo_Materia, Codigo_Tema, Codigo_Subtema, gtk_toggle_button_get_active(CK_reutilizable)?"TRUE":"FALSE", 
                gtk_toggle_button_get_active(TG_objetivo[0])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[1])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[2])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[3])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[4])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[5])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[6])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[7])?"1":"0",
		gtk_combo_box_get_active(CB_Bloom),
                codigo);
       res = PQEXEC(DATABASE, PG_command); PQclear(res);

       sprintf (PG_command,"UPDATE bd_texto_ejercicios SET descripcion_ejercicio='%s', usa_header=%s, autor='%s', texto_header = E'%s' where consecutivo_texto = '%.6s'", 
		descripcion, gtk_toggle_button_get_active(CK_header)?"TRUE":"FALSE", Codigo_Persona, gtk_toggle_button_get_active(CK_header)?header_corregido:" " ,Codigo_texto);
       res = PQEXEC(DATABASE, PG_command); PQclear(res);

       res = PQEXEC(DATABASE, "END"); PQclear(res);
      }
   else
      { /* Ejercicio y Texto de Ejercicio Nuevos */
       res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

       sprintf (PG_command,"INSERT into bd_texto_ejercicios values ('%.6s','%s', 1, '%.10s', %s,E'%s')", 
		Codigo_texto, descripcion, Codigo_Persona, gtk_toggle_button_get_active(CK_header)?"TRUE":"FALSE",  gtk_toggle_button_get_active(CK_header)? header_corregido: " ");

       res = PQEXEC(DATABASE, PG_command); PQclear(res);

       strftime (hilera, 100, "%m", loctime);
       month = atoi(hilera);
       strftime (hilera, 100, "%Y", loctime);
       year  = atoi(hilera);
       strftime (hilera, 100, "%d", loctime);
       day   = atoi(hilera);

       sprintf (PG_command,"INSERT into bd_ejercicios values ('%.6s','%s','%s','%s',%d,%d,%d, %s,' ','%.6s','%.1s','%.1s','%.1s','%.1s','%.1s','%.1s','%.1s','%.1s',%d)", 
		codigo,Codigo_Materia,Codigo_Tema,Codigo_Subtema, year, month, day,gtk_toggle_button_get_active(CK_reutilizable)?"TRUE":"FALSE", Codigo_texto,
                gtk_toggle_button_get_active(TG_objetivo[0])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[1])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[2])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[3])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[4])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[5])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[6])?"1":"0",
                gtk_toggle_button_get_active(TG_objetivo[7])?"1":"0",
                gtk_combo_box_get_active(CB_Bloom));

       res = PQEXEC(DATABASE, PG_command); PQclear(res);

       REGISTRO_YA_EXISTE = 1;
       res = PQEXEC(DATABASE, "END"); PQclear(res);
      }

  g_free (descripcion);

  if (gtk_toggle_button_get_active(CK_header))
     {
      g_free(text_header);
      free (header_corregido);
     }

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

void Borra_Registro (GtkWidget *widget, gpointer user_data)
{
   long int k;
   char PG_command [500];
   char codigo[CODIGO_SIZE];
   int contador_referencias;
   PGresult *res, *res_aux;

   k = (long int) gtk_range_get_value (GTK_RANGE(SC_codigo));
   sprintf (codigo, "%06ld", k);

   sprintf (PG_command,"SELECT contador_referencias from bd_texto_ejercicios where consecutivo_texto = '%s'", Codigo_texto);
   res_aux = PQEXEC(DATABASE, PG_command);
   contador_referencias = atoi(PQgetvalue (res_aux, 0, 0));
   PQclear(res_aux);

   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

   sprintf (PG_command,"DELETE from bd_ejercicios where codigo_ejercicio = '%.6s'", codigo);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   contador_referencias--;

   if (!contador_referencias)
      {
       sprintf (PG_command,"DELETE from bd_texto_ejercicios where consecutivo_texto = '%.6s'", Codigo_texto);
       res = PQEXEC(DATABASE, PG_command); PQclear(res);
      }
   else
      {
       sprintf (PG_command,"UPDATE bd_texto_ejercicios SET contador_referencias = %d where consecutivo_texto = '%.6s'",contador_referencias, Codigo_texto);
       res = PQEXEC(DATABASE, PG_command); PQclear(res);
      }

   res = PQEXEC(DATABASE, "END"); PQclear(res);

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

void Imprime_Encabezado (GtkWidget *widget, gpointer user_data)
{
   long int k;
   FILE * Archivo_Latex;
   GtkTextIter start;
   GtkTextIter end;
   char codigo[CODIGO_SIZE];
   gchar *text_header;
   char  *header_corregido;
   char hilera_antes [3000], hilera_despues [3000];
   char Correcta;

   k = (long int) gtk_range_get_value (GTK_RANGE(SC_codigo));
   sprintf (codigo, "%06ld", k);

   Archivo_Latex = fopen ("EX2030.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");
   fprintf (Archivo_Latex, "\\title{Encabezado Ejercicio %s}\n", codigo);
   fprintf (Archivo_Latex, "\\maketitle\n\n");

   gtk_text_buffer_get_start_iter (buffer_TV_header, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_header, &end);
   text_header      = gtk_text_buffer_get_text (buffer_TV_header, &start, &end, FALSE);
   header_corregido = (char *) malloc (sizeof(char) * strlen(text_header)*2);
   hilera_LATEX (text_header, header_corregido);
   fprintf (Archivo_Latex, "%s\n\n", header_corregido);

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX2030", 1, NULL, 0.0, 0.0, NULL, NULL);
}

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex2030_destroy (GtkWidget *widget, gpointer user_data) 
{
  Fin_Ventana (widget, user_data);
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  Procesa_Ejercicio (widget, user_data);
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
  Graba_Registro (widget, user_data);
}

void on_BN_print_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Encabezado (widget, user_data);
}

void on_BN_delete_clicked(GtkWidget *widget, gpointer user_data)
{
  Borra_Registro (widget, user_data);
}

void on_EN_descripcion_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Descripcion (widget, user_data);
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

void on_CB_autor_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * autor;

  autor = gtk_combo_box_get_active_text(CB_autor);

  if (autor)
     {
      g_free (autor);
      Cambio_Autor (widget, user_data);
     }
  else
     g_free (autor);
}

void on_CK_header_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_header (widget, user_data);
}

void on_SC_codigo_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_Codigo ();
}

void on_SP_codigo_value_changed(GtkWidget *widget, gpointer user_data)
{
  long int k;

  k = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
  gtk_range_set_value (GTK_RANGE(SC_codigo), (gdouble) k);
}

void on_SP_codigo_activate(GtkWidget *widget, gpointer user_data)
{
  long int k;
  gchar * codigo;

  codigo = gtk_editable_get_chars(GTK_EDITABLE(SP_codigo), 0, -1);
  k = atoi(codigo);
  g_free(codigo);

  gtk_range_set_value (GTK_RANGE(SC_codigo), (gdouble) k);
  Procesa_Ejercicio (widget, user_data);
}

void on_BN_ok_1_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_ok_2_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window3);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_extend_header_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * ejercicio;
  char window_title [100];
  char root[100];

  ejercicio = gtk_editable_get_chars(GTK_EDITABLE(SP_codigo), 0, -1);
  sprintf (window_title, "Código LATEX de Encabezado de Ejercicio %s", ejercicio);
  sprintf (root, "%s-H", ejercicio);

  Display_Latex_Window (window_title, root, buffer_TV_header, window1);
}
