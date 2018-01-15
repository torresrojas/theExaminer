/*
Compilar con:

cc -o EX2000 EX2000.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX2000:                                */
/*                                         */
/*    Actualización Archivo de Materias    */
/*    The Examiner 0.0                     */
/*    26 de Junio 2011                     */
/*    Autor: Francisco J. Torres-Rojas     */
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    27 de Noviembre 2011                 */
/*******************************************/
#include <stdlib.h>
#include <gtk/gtk.h>
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

GtkWidget *EN_materia             = NULL;
GtkWidget *EN_tema                = NULL;
GtkWidget *EN_subtema             = NULL;
GtkWidget *EN_descripcion_materia = NULL;
GtkWidget *EN_descripcion_tema    = NULL;
GtkWidget *EN_descripcion_subtema = NULL;
GtkWidget *SC_orden_tema          = NULL;
GtkWidget *SC_orden_subtema       = NULL;
GtkWidget *LB_tema                = NULL;
GtkWidget *LB_subtema             = NULL;
GtkWidget *LB_orden_tema          = NULL;
GtkWidget *LB_orden_subtema       = NULL;
GtkWidget *FR_botones             = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_save                = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_delete              = NULL;
GtkWidget *BN_terminar            = NULL;
GtkComboBox *CB_materia           = NULL;
GtkComboBox *CB_tema              = NULL;
GtkComboBox *CB_subtema           = NULL;

GtkWidget *EB_reporte             = NULL;
GtkWidget *FR_reporte             = NULL;
GtkWidget *PB_reporte             = NULL;

GtkWidget *FR_update              = NULL;
GtkWidget *EB_update              = NULL;
GtkWidget *BN_ok                  = NULL;

/**************/
/* Prototipos */
/**************/
void Acomoda_Temas                      (int orden_tema);
void Acomoda_Subtemas                   (int orden_subtema);
void Borra_Registro                     (GtkWidget *widget, gpointer user_data);
void Cambio_Descripcion_Materia         (GtkWidget *widget, gpointer user_data);
void Cambio_Descripcion_Subtema         (GtkWidget *widget, gpointer user_data);
void Cambio_Descripcion_Tema            (GtkWidget *widget, gpointer user_data);
void Cambio_Materia                     (GtkWidget *widget, gpointer user_data);
void Cambio_Materia_Combo               (GtkWidget *widget, gpointer user_data);
void Cambio_Subtema                     (GtkWidget *widget, gpointer user_data);
void Cambio_Subtema_Combo               (GtkWidget *widget, gpointer user_data);
void Cambio_Tema                        (GtkWidget *widget, gpointer user_data);
void Cambio_Tema_Combo                  (GtkWidget *widget, gpointer user_data);
void Carga_Subtemas                     (char * materia, char * tema);
void Carga_Temas                        (char * materia);
void Connect_Widgets                    ();
void Fin_de_Programa                    (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                        (GtkWidget *widget, gpointer user_data);
void Graba_Registro                     (GtkWidget *widget, gpointer user_data);
void Imprime_listado                    (GtkWidget *widget, gpointer user_data);
void Init_Fields                        ();
void Interface_Coloring                 ();
void Materia_ya_existe                  (char * materia);
void on_BN_delete_clicked               (GtkWidget *widget, gpointer user_data);
void on_BN_print_clicked                (GtkWidget *widget, gpointer user_data);
void on_BN_save_clicked                 (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked             (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked                 (GtkWidget *widget, gpointer user_data);
void on_CB_materia_changed              (GtkWidget *widget, gpointer user_data);
void on_CB_subtema_changed              (GtkWidget *widget, gpointer user_data);
void on_CB_tema_changed                 (GtkWidget *widget, gpointer user_data);
void on_EN_descripcion_materia_activate (GtkWidget *widget, gpointer user_data);
void on_EN_descripcion_subtema_activate (GtkWidget *widget, gpointer user_data);
void on_EN_descripcion_tema_activate    (GtkWidget *widget, gpointer user_data);
void on_EN_materia_activate             (GtkWidget *widget, gpointer user_data);
void on_EN_subtema_activate             (GtkWidget *widget, gpointer user_data);
void on_EN_tema_activate                (GtkWidget *widget, gpointer user_data);
void on_WN_ex2000_destroy               (GtkWidget *widget, gpointer user_data);
void Quita_espacios                     (char * hilera);
void Subtema_ya_existe                  (char * materia, char * tema, char * subtema);
void Tema_ya_existe                     (char * materia, char * tema);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;
#define SOLO_MATERIA         0
#define MATERIA_TEMA         1
#define MATERIA_TEMA_SUBTEMA 2

int REGISTRO_YA_EXISTE = 0;
int MODO_GRABACION     = SOLO_MATERIA;

int N_materias=0;
int N_temas   =0;
int N_subtemas=0;
int orden_tema_previo, orden_subtema_previo;
/*----------------------------------------------------------------------------*/

/***************/
/* M A I N ( ) */
/***************/
int main(int argc, char *argv[]) 
{
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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX2000.glade", &error))
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

	  /* Displays main window */
          gtk_widget_show (window1);  
          Init_Fields ();              

          /* start the event loop */
          gtk_main ();
	 }
     }

  return 0;
}

void Connect_Widgets()
{
  window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex2000"));
  window2                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));
  window3                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_reporte"));

  CB_materia             = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_materia"));
  EN_materia             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia"));
  EN_descripcion_materia = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion_materia"));

  LB_tema                = GTK_WIDGET (gtk_builder_get_object (builder, "LB_tema"));
  CB_tema                = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_tema"));
  EN_tema                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_tema"));
  EN_descripcion_tema    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion_tema"));
  LB_orden_tema          = GTK_WIDGET (gtk_builder_get_object (builder, "LB_orden_tema"));
  SC_orden_tema          = GTK_WIDGET (gtk_builder_get_object (builder, "SC_orden_tema"));

  LB_subtema             = GTK_WIDGET (gtk_builder_get_object (builder, "LB_subtema"));
  CB_subtema             = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_subtema"));
  EN_subtema             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_subtema"));
  EN_descripcion_subtema = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion_subtema"));
  LB_orden_subtema       = GTK_WIDGET (gtk_builder_get_object (builder, "LB_orden_subtema"));
  SC_orden_subtema       = GTK_WIDGET (gtk_builder_get_object (builder, "SC_orden_subtema"));

  FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));

  BN_print               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_save                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
  BN_delete              = GTK_WIDGET (gtk_builder_get_object (builder, "BN_delete"));
  BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_undo                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));

  EB_reporte         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_reporte"));
  FR_reporte         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reporte"));
  PB_reporte         = GTK_WIDGET (gtk_builder_get_object (builder, "PB_reporte"));

  EB_update         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
  FR_update         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));
  BN_ok             = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_botones,  GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,               GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,                GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_delete,              GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,                GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,                  GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_delete,   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,       GTK_STATE_ACTIVE, &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(EN_materia,             GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_tema   ,             GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_subtema,             GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_descripcion_materia, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_descripcion_tema   , GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_descripcion_subtema, GTK_STATE_NORMAL, &color);

  gdk_color_parse (PROCESSING_FR, &color);
  gtk_widget_modify_bg(FR_reporte,             GTK_STATE_NORMAL, &color);
  gdk_color_parse (PROCESSING_WINDOW, &color);
  gtk_widget_modify_bg(EB_reporte,              GTK_STATE_NORMAL, &color);
  gdk_color_parse (PROCESSING_PB, &color);
  gtk_widget_modify_bg(GTK_WIDGET(PB_reporte),  GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,  GTK_STATE_NORMAL,   &color);
  gdk_color_parse (FINISHED_WORK_FR, &color);
  gtk_widget_modify_bg(FR_update, GTK_STATE_NORMAL, &color);
}

void Init_Fields()
{
   int i;
   char hilera[210];
   PGresult *res;

   gtk_entry_set_text(GTK_ENTRY(EN_materia),             "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_tema),                "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_subtema),             "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_descripcion_materia), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_descripcion_tema)   , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_descripcion_subtema), "\0");

   gtk_widget_set_visible (GTK_WIDGET(CB_materia),             TRUE);
   gtk_widget_set_visible (EN_materia,             FALSE);
   gtk_widget_set_visible (EN_descripcion_materia, FALSE);

   gtk_widget_set_visible (GTK_WIDGET(CB_tema),                FALSE);
   gtk_widget_set_visible (LB_tema,                FALSE);
   gtk_widget_set_visible (EN_tema,                FALSE);
   gtk_widget_set_visible (EN_descripcion_tema,    FALSE);
   gtk_widget_set_visible (LB_orden_tema,          FALSE);
   gtk_widget_set_visible (SC_orden_tema,          FALSE);

   gtk_widget_set_visible (GTK_WIDGET(CB_subtema),             FALSE);
   gtk_widget_set_visible (LB_subtema,             FALSE);
   gtk_widget_set_visible (EN_subtema,             FALSE);
   gtk_widget_set_visible (EN_descripcion_subtema, FALSE);
   gtk_widget_set_visible (LB_orden_subtema,       FALSE);
   gtk_widget_set_visible (SC_orden_subtema,       FALSE);

   gtk_combo_box_set_active (CB_materia, -1);
   for (i=0; i < N_materias; i++) gtk_combo_box_remove_text (CB_materia, 0);

   gtk_combo_box_set_active (CB_tema, -1);
   for (i=0; i < N_temas; i++) gtk_combo_box_remove_text (CB_tema, 0);

   gtk_combo_box_set_active (CB_subtema, -1);
   for (i=0; i < N_subtemas; i++) gtk_combo_box_remove_text (CB_subtema, 0);

   gtk_combo_box_append_text(CB_materia, "*** Materia Nueva ***");
   res = PQEXEC(DATABASE,"SELECT codigo_materia, descripcion_materia from bd_materias where codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_materia"); 
   N_materias = PQntuples(res);

   for (i=0; i < N_materias; i++)
       {
	sprintf (hilera,"%s %s", PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
        gtk_combo_box_append_text(CB_materia, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_materia, -1);
   N_materias++;

   gtk_widget_set_can_focus(EN_materia            , FALSE);
   gtk_widget_set_can_focus(EN_tema               , FALSE);
   gtk_widget_set_can_focus(EN_subtema            , FALSE);
   gtk_widget_set_can_focus(EN_descripcion_materia, FALSE);
   gtk_widget_set_can_focus(EN_descripcion_tema,    FALSE);
   gtk_widget_set_can_focus(EN_descripcion_subtema, FALSE);

   gtk_widget_set_sensitive(EN_materia            , 0);
   gtk_widget_set_sensitive(EN_descripcion_materia, 0);
   gtk_widget_set_sensitive(EN_tema               , 0);
   gtk_widget_set_sensitive(EN_descripcion_tema   , 0);
   gtk_widget_set_sensitive(EN_subtema            , 0);
   gtk_widget_set_sensitive(EN_descripcion_subtema, 0);

   if (N_materias)
      gtk_widget_set_sensitive(BN_print,   1);
   else
      gtk_widget_set_sensitive(BN_print,   0);

   gtk_widget_set_sensitive(BN_save,   0);
   gtk_widget_set_sensitive(BN_undo,   1);
   gtk_widget_set_sensitive(BN_delete, 0);

   REGISTRO_YA_EXISTE = 0;
   MODO_GRABACION = SOLO_MATERIA;

   gtk_widget_set_sensitive(GTK_WIDGET(CB_materia), 1);
   gtk_widget_set_sensitive(GTK_WIDGET(CB_tema),    0);
   gtk_widget_set_sensitive(GTK_WIDGET(CB_subtema), 0);
   gtk_widget_grab_focus   (GTK_WIDGET(CB_materia));
}

void Cambio_Materia_Combo(GtkWidget *widget, gpointer user_data)
{
  int i, j, k;
  char codigo_materia     [CODIGO_MATERIA_SIZE      + 1];
  char descripcion_materia[DESCRIPCION_MATERIA_SIZE + 1];
  gchar *materia;

  MODO_GRABACION = SOLO_MATERIA;

  k = gtk_combo_box_get_active(CB_materia);

  gtk_widget_set_visible (EN_materia,             TRUE);
  gtk_widget_set_visible (EN_descripcion_materia, TRUE);
  gtk_widget_set_visible (GTK_WIDGET(CB_materia), FALSE);

  if (k == 0)
     { /* Materia nueva */
      gtk_widget_set_sensitive(GTK_WIDGET(CB_materia), 0);
      gtk_widget_set_sensitive(EN_materia, 1);
      gtk_widget_set_can_focus(EN_materia, TRUE);
      gtk_widget_grab_focus   (EN_materia);
     }
  else
     {
      gtk_widget_set_sensitive(BN_save,   1);

      materia = gtk_combo_box_get_active_text(CB_materia);

      i=0;
      while (materia[i] != ' ') 
	    {
	     codigo_materia[i] = materia[i];
	     i++;
	    }
      codigo_materia[i]='\0';

      while (materia[i] == ' ') i++; 

      j=0;
      while (materia[i+j])
	    {
	     descripcion_materia[j] = materia[i+j];
	     j++;
	    }
      descripcion_materia[j]='\0';

      gtk_widget_set_sensitive(GTK_WIDGET(CB_materia), 0);
      gtk_entry_set_text(GTK_ENTRY(EN_materia),             codigo_materia);
      gtk_entry_set_text(GTK_ENTRY(EN_descripcion_materia), descripcion_materia);

      gtk_widget_set_sensitive(EN_materia, 1);
      gtk_widget_set_sensitive(EN_descripcion_materia, 1);

      Materia_ya_existe (codigo_materia);

      g_free (materia);
     }
}

void Cambio_Materia(GtkWidget *widget, gpointer user_data)
{
  gchar *materia = NULL;
  char PG_command[200];
  PGresult *res;

  materia = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
  Quita_espacios(materia);
  gtk_entry_set_text(GTK_ENTRY(EN_materia),materia);

  if (materia[0])
     {
      MODO_GRABACION = SOLO_MATERIA;
      gtk_widget_set_can_focus(EN_materia    , FALSE);
      gtk_widget_set_sensitive(BN_save,   1);

      sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", materia);
      res = PQEXEC(DATABASE, PG_command);
      if (PQntuples(res))
	 {
          gtk_entry_set_text(GTK_ENTRY(EN_descripcion_materia),PQgetvalue (res, 0, 3));

	  Materia_ya_existe (materia);
	 }
      else
	{ /* MATERIA NO EXISTE */
	  REGISTRO_YA_EXISTE = 0;

          gtk_widget_set_sensitive(BN_delete, 0);

          gtk_widget_set_sensitive(EN_tema               , 0);
          gtk_widget_set_sensitive(EN_subtema            , 0);
          gtk_widget_set_sensitive(EN_descripcion_materia, 1);
          gtk_widget_set_can_focus(EN_descripcion_materia, TRUE);

          gtk_widget_grab_focus (EN_descripcion_materia);
	 }
      PQclear(res);
     }

  g_free (materia);
}

void Materia_ya_existe (char * materia)
{
  char PG_command[200];
  PGresult *res;
  PGresult *res_aux;

  REGISTRO_YA_EXISTE = 1;

  gtk_widget_set_sensitive(BN_delete, 1);

  sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema!= '          '", materia);
  res = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res)) 
     gtk_widget_set_sensitive(BN_delete, 0);
  else
     {
      sprintf (PG_command,"SELECT * from EX_esquemas where materia = '%s'", materia);
      res = PQEXEC(DATABASE, PG_command);
      if (PQntuples(res)) 
         gtk_widget_set_sensitive(BN_delete, 0);
      else
	 {
          sprintf (PG_command,"SELECT * from bd_ejercicios where materia = '%s'", materia);
          res = PQEXEC(DATABASE, PG_command);
          if (PQntuples(res)) 
             gtk_widget_set_sensitive(BN_delete, 0);
	  else
             sprintf (PG_command,"SELECT * from bd_objetivos where materia = '%s'", materia);
             res = PQEXEC(DATABASE, PG_command);
             if (PQntuples(res)) 
                gtk_widget_set_sensitive(BN_delete, 0);

	 }
     }
  PQclear(res);

  gtk_widget_set_sensitive(EN_descripcion_materia, 1);
  gtk_widget_set_can_focus(EN_descripcion_materia, TRUE);

  Carga_Temas (materia);

  gtk_widget_set_visible  (LB_tema, TRUE);
  gtk_widget_set_visible  (GTK_WIDGET(CB_tema), TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(CB_tema), 1);
  gtk_widget_grab_focus   (GTK_WIDGET(CB_tema));
}

void Carga_Temas (char * materia)
{
   int i;
   char hilera[210];
   char PG_command[1000];
   PGresult *res;

   gtk_combo_box_set_active (CB_tema, -1);
   for (i=0; i<N_temas; i++)
       {
	gtk_combo_box_remove_text (CB_tema, 0);
       }

   gtk_combo_box_append_text(CB_tema, "*** Tema Nuevo ***");
   sprintf (PG_command,"SELECT codigo_tema, descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_subtema = '          ' and codigo_tema != '          ' order by orden, codigo_tema", materia); 
   res = PQEXEC(DATABASE, PG_command);

   N_temas = PQntuples(res);
   for (i=0; i < N_temas; i++)
       {
	sprintf (hilera,"%s %s", PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
        gtk_combo_box_append_text(CB_tema, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_tema, -1);
   N_temas++;
}

void Cambio_Tema_Combo(GtkWidget *widget, gpointer user_data)
{
  int i, j, k;
  char codigo_tema      [CODIGO_TEMA_SIZE         + 1];
  char descripcion_tema [DESCRIPCION_MATERIA_SIZE + 1];
  gchar *materia;
  gchar *tema;
  char PG_command[1000];
  PGresult *res;

  MODO_GRABACION = MATERIA_TEMA;
  gtk_widget_set_visible (EN_tema,             TRUE);
  gtk_widget_set_visible (EN_descripcion_tema, TRUE);
  gtk_widget_set_visible (GTK_WIDGET(CB_tema), FALSE);

  k = gtk_combo_box_get_active(CB_tema);

  if (k == 0)
     { /* Tema nuevo */
      orden_tema_previo = N_temas;
      if (N_temas > 1)
	 {
          gtk_widget_set_visible (LB_orden_tema,       TRUE);
          gtk_widget_set_visible (SC_orden_tema,       TRUE);
          gtk_range_set_range (GTK_RANGE(SC_orden_tema), (gdouble) 1.0, (gdouble)N_temas);
          gtk_range_set_value (GTK_RANGE(SC_orden_tema), (gdouble)               N_temas);
	 }
      else
	 {
	  gtk_range_set_range (GTK_RANGE(SC_orden_tema), (gdouble) 1.0, (gdouble)2);
          gtk_range_set_value (GTK_RANGE(SC_orden_tema), (gdouble)               1);
	 }

      gtk_widget_set_sensitive(BN_save,   0);

      gtk_widget_set_sensitive(GTK_WIDGET(CB_tema), 0);
      gtk_widget_set_sensitive(EN_tema, 1);
      gtk_widget_set_can_focus(EN_tema, TRUE);
      gtk_widget_grab_focus   (EN_tema);
     }
  else
     {
      materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);
      tema    = gtk_combo_box_get_active_text(CB_tema);

      i=0;
      while (tema[i] != ' ') 
	    {
	     codigo_tema[i] = tema[i];
	     i++;
	    }
      codigo_tema[i]='\0';

      while (tema[i] == ' ') i++; 

      j=0;
      while (tema[i+j])
	    {
	     descripcion_tema[j] = tema[i+j];
	     j++;
	    }
      descripcion_tema[j]='\0';

      gtk_widget_set_sensitive(GTK_WIDGET(CB_tema), 0);
      gtk_entry_set_text(GTK_ENTRY(EN_tema),             codigo_tema);
      gtk_entry_set_text(GTK_ENTRY(EN_descripcion_tema), descripcion_tema);

      gtk_widget_set_sensitive(EN_tema, 1);
      gtk_widget_set_sensitive(EN_descripcion_tema, 1);

      Tema_ya_existe (materia, codigo_tema);

      g_free (tema);
     }
}

void Cambio_Tema(GtkWidget *widget, gpointer user_data)
{
  gchar *materia, *tema = NULL;
  char PG_command[600];
  PGresult *res;

  materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia)    , 0, -1);
  tema    = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
  Quita_espacios(tema);
  gtk_entry_set_text(GTK_ENTRY(EN_tema),tema);

  if (tema[0])
     {
      MODO_GRABACION = MATERIA_TEMA;
      gtk_widget_set_can_focus(EN_tema, FALSE);
      gtk_widget_set_sensitive(BN_save,   1);

      sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", materia, tema);
      res = PQEXEC(DATABASE, PG_command);
      if (PQntuples(res))
	 {
          gtk_entry_set_text(GTK_ENTRY(EN_descripcion_tema),PQgetvalue (res, 0, 3));

          Tema_ya_existe (materia, tema);
	 }
      else
	 {
	  REGISTRO_YA_EXISTE = 0;
          gtk_widget_set_sensitive(BN_delete, 0);


          gtk_widget_set_sensitive(EN_descripcion_tema, 1);
          gtk_widget_set_can_focus(EN_descripcion_tema, TRUE);
          gtk_widget_grab_focus   (EN_descripcion_tema);
	 }
      PQclear(res);
     }
  else
     {
      gtk_widget_grab_focus (EN_descripcion_materia);
     }

  g_free (tema);
  g_free (materia);
}

void Tema_ya_existe(char * materia, char * tema)
{
   char PG_command[1000];
   PGresult *res;

   REGISTRO_YA_EXISTE = 1;

   gtk_widget_set_sensitive(BN_delete, 1);

   sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema= '%s' and codigo_subtema != '          '", materia,tema);
   res = PQEXEC(DATABASE, PG_command);
   if (PQntuples(res))
     {
      gtk_widget_set_sensitive(BN_delete, 0);
     }
   else
      {
       sprintf (PG_command,"SELECT * from EX_esquemas, EX_esquemas_lineas where materia = '%s' and codigo_esquema = esquema and tema = '%s'", materia,tema);
       res = PQEXEC(DATABASE, PG_command);
       if (PQntuples(res))
	 {
          gtk_widget_set_sensitive(BN_delete, 0);
	 }
       else
 	  {
	   sprintf (PG_command,"SELECT * from bd_ejercicios where materia = '%s' and tema = '%s'", materia, tema);
           res = PQEXEC(DATABASE, PG_command);
           if (PQntuples(res))
	     {
              gtk_widget_set_sensitive(BN_delete, 0);
	     }
	  }
      }
   PQclear(res);

   gtk_widget_set_sensitive(EN_descripcion_tema, 1);
   gtk_widget_set_can_focus(EN_descripcion_tema, TRUE);

   sprintf (PG_command,"SELECT orden from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", materia, tema);
   res = PQEXEC(DATABASE, PG_command);
   orden_tema_previo = atoi(PQgetvalue (res, 0, 0));

   if (N_temas > 2)
      {
       gtk_widget_set_visible (LB_orden_tema,       TRUE);
       gtk_widget_set_visible (SC_orden_tema,       TRUE);
       gtk_range_set_range (GTK_RANGE(SC_orden_tema), (gdouble) 1.0, (gdouble)N_temas-1);
       gtk_range_set_value (GTK_RANGE(SC_orden_tema), (gdouble)               orden_tema_previo);
      }
   else
      {
       gtk_range_set_range (GTK_RANGE(SC_orden_tema), (gdouble) 1.0, (gdouble)2);
       gtk_range_set_value (GTK_RANGE(SC_orden_tema), (gdouble)               1);
      }

   Carga_Subtemas (materia, tema);

   gtk_widget_set_visible  (LB_subtema, TRUE);
   gtk_widget_set_visible  (GTK_WIDGET(CB_subtema), TRUE);
   gtk_widget_set_sensitive(GTK_WIDGET(CB_subtema), 1);
   gtk_widget_grab_focus   (GTK_WIDGET(CB_subtema));
}

void Carga_Subtemas (char * materia, char * tema)
{
   int i;
   char hilera[210];
   char PG_command[1000];
   PGresult *res;

   gtk_combo_box_set_active (CB_subtema, -1);
   for (i=0; i<N_subtemas; i++)
       {
	gtk_combo_box_remove_text (CB_subtema, 0);
       }

   gtk_combo_box_append_text(CB_subtema, "*** Subtema Nuevo ***");
   sprintf (PG_command,"SELECT codigo_subtema, descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' order by orden, codigo_subtema", materia, tema); 
   res = PQEXEC(DATABASE, PG_command);
   N_subtemas = PQntuples(res); 

   for (i=0; i < N_subtemas; i++)
       {
	sprintf (hilera,"%s %s", PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
        gtk_combo_box_append_text(CB_subtema, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_subtema, -1);
   N_subtemas++;
}

void Cambio_Subtema_Combo(GtkWidget *widget, gpointer user_data)
{
  int i, j, k;
  char codigo_subtema      [CODIGO_SUBTEMA_SIZE      + 1];
  char descripcion_subtema [DESCRIPCION_MATERIA_SIZE + 1];
  gchar *materia;
  gchar *tema;
  gchar *subtema;

  MODO_GRABACION = MATERIA_TEMA_SUBTEMA;
  gtk_widget_set_visible (EN_subtema,             TRUE);
  gtk_widget_set_visible (EN_descripcion_subtema, TRUE);
  gtk_widget_set_visible (GTK_WIDGET(CB_subtema), FALSE);

  k = gtk_combo_box_get_active(CB_subtema);

  if (k == 0)
     { /* Subtema nuevo */
      orden_subtema_previo = N_subtemas;
      if (N_subtemas > 1)
         {
          gtk_widget_set_visible (LB_orden_subtema,       TRUE);
          gtk_widget_set_visible (SC_orden_subtema,       TRUE);
          gtk_range_set_range (GTK_RANGE(SC_orden_subtema), (gdouble) 1.0, (gdouble)N_subtemas);
          gtk_range_set_value (GTK_RANGE(SC_orden_subtema), (gdouble)               N_subtemas);
	 }
      else
	 {
	  gtk_range_set_range (GTK_RANGE(SC_orden_subtema), (gdouble) 1.0, (gdouble)2);
          gtk_range_set_value (GTK_RANGE(SC_orden_subtema), (gdouble)               1);
	 }

      gtk_widget_set_sensitive(BN_save,   0);

      gtk_widget_set_sensitive(GTK_WIDGET(CB_subtema), 0);
      gtk_widget_set_sensitive(EN_subtema, 1);
      gtk_widget_set_can_focus(EN_subtema, TRUE);
      gtk_widget_grab_focus   (EN_subtema);
     }
  else
     {
      materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);
      tema    = gtk_editable_get_chars(GTK_EDITABLE(EN_tema)   , 0, -1);
      subtema    = gtk_combo_box_get_active_text(CB_subtema);

      i=0;
      while (subtema[i] != ' ') 
	    {
	     codigo_subtema[i] = subtema[i];
	     i++;
	    }
      codigo_subtema[i]='\0';

      while (subtema[i] == ' ') i++; 

      j=0;
      while (subtema[i+j])
	    {
	     descripcion_subtema[j] = subtema[i+j];
	     j++;
	    }
      descripcion_subtema[j]='\0';

      gtk_widget_set_sensitive(GTK_WIDGET(CB_subtema), 0);
      gtk_entry_set_text(GTK_ENTRY(EN_subtema),             codigo_subtema);
      gtk_entry_set_text(GTK_ENTRY(EN_descripcion_subtema), descripcion_subtema);

      gtk_widget_set_sensitive(EN_subtema, 1);
      gtk_widget_set_sensitive(EN_descripcion_subtema, 1);

      Subtema_ya_existe (materia, tema, codigo_subtema);

      g_free (materia);
      g_free (tema);
      g_free (subtema);
     }
}

void Cambio_Subtema(GtkWidget *widget, gpointer user_data)
{
  gchar *materia, *tema, *subtema = NULL;
  char PG_command[200];
  PGresult *res;

  materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia) , 0, -1);
  tema    = gtk_editable_get_chars(GTK_EDITABLE(EN_tema)    , 0, -1);
  subtema = gtk_editable_get_chars(GTK_EDITABLE(widget)     , 0, -1);
  Quita_espacios(subtema);
  gtk_entry_set_text(GTK_ENTRY(EN_subtema),subtema);

  if (subtema[0])
     {
      MODO_GRABACION = MATERIA_TEMA_SUBTEMA;

      gtk_widget_set_sensitive(BN_save,   1);

      gtk_widget_set_can_focus(EN_subtema       , FALSE);

      sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", materia, tema, subtema);
      res = PQEXEC(DATABASE, PG_command);
      if (PQntuples(res))
         { /* Registro ya existe... */
          gtk_entry_set_text(GTK_ENTRY(EN_descripcion_subtema),PQgetvalue (res, 0, 3));

          Subtema_ya_existe (materia, tema, subtema);
	 }
      else
	 {
          REGISTRO_YA_EXISTE = 0;
          gtk_widget_set_sensitive(BN_delete, 0);

          gtk_widget_set_sensitive    (EN_descripcion_subtema, 1);
          gtk_widget_set_can_focus    (EN_descripcion_subtema, TRUE);
          gtk_widget_grab_focus       (EN_descripcion_subtema);
	 }
      PQclear(res);
     }
  else
     {
      gtk_widget_grab_focus (EN_descripcion_materia);
     }

  g_free (materia);
  g_free (tema);
  g_free (subtema);
}

void Subtema_ya_existe (char * materia, char * tema, char * subtema)
{
   char PG_command[500];
   PGresult *res;
 
   REGISTRO_YA_EXISTE = 1;

   gtk_widget_set_sensitive(BN_delete, 1);

   sprintf (PG_command,"SELECT * from bd_ejercicios where materia = '%s' and tema = '%s' and subtema = '%s'", materia, tema, subtema);
   res  = PQEXEC(DATABASE, PG_command);
   if (PQntuples(res)) gtk_widget_set_sensitive(BN_delete, 0);
   PQclear(res);

   sprintf (PG_command,"SELECT orden from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", materia, tema, subtema);
   res = PQEXEC(DATABASE, PG_command);
   orden_subtema_previo = atoi(PQgetvalue (res, 0, 0));

   if (N_subtemas > 2)
      {
       gtk_widget_set_visible (LB_orden_subtema,       TRUE);
       gtk_widget_set_visible (SC_orden_subtema,       TRUE);
       gtk_range_set_range (GTK_RANGE(SC_orden_subtema), (gdouble) 1.0, (gdouble)N_subtemas-1);
       gtk_range_set_value (GTK_RANGE(SC_orden_subtema), (gdouble)               orden_subtema_previo);
      }
   else
      {
       gtk_range_set_range (GTK_RANGE(SC_orden_subtema), (gdouble) 1.0, (gdouble)2);
       gtk_range_set_value (GTK_RANGE(SC_orden_subtema), (gdouble)               1);
      }

   gtk_widget_set_sensitive    (EN_descripcion_subtema, 1);
   gtk_widget_set_can_focus    (EN_descripcion_subtema, TRUE);
   gtk_widget_grab_focus       (EN_descripcion_subtema);
}

void Graba_Registro (GtkWidget *widget, gpointer user_data)
{
   gchar *materia, *descripcion_materia, *descripcion_tema, *descripcion_subtema, *tema, *subtema;
   int orden_tema, orden_subtema;
   char PG_command [500];
   PGresult *res;

   materia             = gtk_editable_get_chars(GTK_EDITABLE(EN_materia)            , 0, -1);
   tema                = gtk_editable_get_chars(GTK_EDITABLE(EN_tema)               , 0, -1);
   subtema             = gtk_editable_get_chars(GTK_EDITABLE(EN_subtema)            , 0, -1);
   descripcion_materia = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion_materia), 0, -1);
   descripcion_tema    = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion_tema)   , 0, -1);
   descripcion_subtema = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion_subtema), 0, -1);
   orden_tema          = gtk_range_get_value (GTK_RANGE(SC_orden_tema));
   orden_subtema       = gtk_range_get_value (GTK_RANGE(SC_orden_subtema));

   if (MODO_GRABACION == SOLO_MATERIA)
      {
       if (REGISTRO_YA_EXISTE)
          {
           sprintf (PG_command,"UPDATE bd_materias SET descripcion_materia='%s' where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", 
		                descripcion_materia, materia);
           res = PQEXEC(DATABASE, PG_command); PQclear(res);
	  }
       else
	  {
           sprintf (PG_command,"INSERT into bd_materias values ('%s','          ','          ','%s',0)", 
	 	                materia, descripcion_materia);
           res = PQEXEC(DATABASE, PG_command); PQclear(res);
	  }

       Materia_ya_existe (materia);
      }
   else
      {
       if (MODO_GRABACION == MATERIA_TEMA)
          {
           sprintf (PG_command,"UPDATE bd_materias SET descripcion_materia='%s' where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", 
		                descripcion_materia, materia);
           res = PQEXEC(DATABASE, PG_command); PQclear(res);

	   if (orden_tema != orden_tema_previo) Acomoda_Temas (orden_tema);

           if (REGISTRO_YA_EXISTE)
              {
               sprintf (PG_command,"UPDATE bd_materias SET descripcion_materia='%s', orden=%d where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", 
			descripcion_tema, orden_tema, materia, tema);
               res = PQEXEC(DATABASE, PG_command); PQclear(res);
	      }
           else
	      {
               sprintf (PG_command,"INSERT into bd_materias values ('%s','%s','          ','%s', %d)", 
			materia, tema, descripcion_tema, orden_tema);
               res = PQEXEC(DATABASE, PG_command); PQclear(res);
	       N_temas++;
	      }
           Tema_ya_existe (materia, tema);
          }
       else
          {
           sprintf (PG_command,"UPDATE bd_materias SET descripcion_materia='%s' where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", 
		                descripcion_materia, materia);
           res = PQEXEC(DATABASE, PG_command); PQclear(res);

	   if (orden_tema != orden_tema_previo) Acomoda_Temas (orden_tema);

           sprintf (PG_command,"UPDATE bd_materias SET descripcion_materia='%s', orden = %d where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", 
		    descripcion_tema, orden_tema, materia, tema);
           res = PQEXEC(DATABASE, PG_command); PQclear(res);

	   if (orden_subtema != orden_subtema_previo) Acomoda_Subtemas (orden_subtema);

           if (REGISTRO_YA_EXISTE)
              {
               sprintf (PG_command,"UPDATE bd_materias SET descripcion_materia='%s', orden=%d  where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", 
			descripcion_subtema, orden_subtema, materia, tema, subtema);
               res = PQEXEC(DATABASE, PG_command); PQclear(res);
	      }
           else
	      {
               sprintf (PG_command,"INSERT into bd_materias values ('%s','%s','%s','%s',%d)", 
			materia, tema, subtema, descripcion_subtema, orden_subtema);
               res = PQEXEC(DATABASE, PG_command); PQclear(res);
	      }

           gtk_entry_set_text(GTK_ENTRY(EN_subtema),             "\0");
           gtk_entry_set_text(GTK_ENTRY(EN_descripcion_subtema), "\0");
           gtk_widget_set_visible (LB_orden_subtema,       FALSE);
           gtk_widget_set_visible (SC_orden_subtema,       FALSE);
           gtk_widget_set_visible (EN_subtema,             FALSE);
           gtk_widget_set_visible (EN_descripcion_subtema, FALSE);
           Tema_ya_existe (materia, tema);
          }
      }

  g_free (materia);
  g_free (tema);
  g_free (subtema);
  g_free (descripcion_materia);
  g_free (descripcion_tema);
  g_free (descripcion_subtema);

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
   char PG_command [500];
   PGresult *res;
   gchar *materia,*tema,*subtema;

   materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);
   tema    = gtk_editable_get_chars(GTK_EDITABLE(EN_tema)  , 0, -1);
   subtema = gtk_editable_get_chars(GTK_EDITABLE(EN_subtema), 0, -1);

   sprintf (PG_command,"DELETE from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", materia, tema[0]?tema:CODIGO_VACIO, subtema[0]?subtema:CODIGO_VACIO);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   if (subtema [0]) 
      Acomoda_Subtemas (N_subtemas);
   else
      if (tema [0])
         Acomoda_Temas (N_temas);

   g_free (materia);
   g_free (tema);
   g_free (subtema);
   Init_Fields ();

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

void Acomoda_Temas(int orden_tema)
{
   int i,j, N;
   char PG_command [500];
   PGresult *res, *res_aux;
   gchar *materia, * tema;

   /* start a transaction block */
   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

   materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);
   tema    = gtk_editable_get_chars(GTK_EDITABLE(EN_tema), 0, -1);

   sprintf (PG_command,"SELECT codigo_tema from bd_materias where codigo_materia = '%s' and codigo_tema != '          ' and codigo_tema != '%s' and codigo_subtema = '          ' and orden <= %d order by orden",
            materia, tema, orden_tema);
   res = PQEXEC(DATABASE, PG_command);
   N = PQntuples(res);
   for (i=0;i<N;i++)
       {
	sprintf (PG_command,"UPDATE bd_materias SET orden=%d  where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '",
                     i+1, materia, PQgetvalue (res, i, 0));
        res_aux = PQEXEC(DATABASE, PG_command); PQclear(res_aux);
       }
   PQclear(res);

   sprintf (PG_command,"SELECT codigo_tema from bd_materias where codigo_materia = '%s' and codigo_tema != '          ' and codigo_tema != '%s' and codigo_subtema = '          ' and orden >= %d order by orden",
            materia, tema, orden_tema);
   res = PQEXEC(DATABASE, PG_command);
   N = PQntuples(res);
   for (i=0;i<N;i++)
       {
	sprintf (PG_command,"UPDATE bd_materias SET orden=%d  where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '",
                     orden_tema + i + 1, materia, PQgetvalue (res, i, 0));
        res_aux = PQEXEC(DATABASE, PG_command); PQclear(res_aux);
       }
   PQclear(res);

   g_free (materia);
   g_free (tema);

   /* end the transaction */
   res = PQEXEC(DATABASE, "END"); PQclear(res);
}

void Acomoda_Subtemas(int orden_subtema)
{
   int i,j, N;
   char PG_command [500];
   PGresult *res, *res_aux;
   gchar *materia, *tema, *subtema;

   /* start a transaction block */
   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

   materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);
   tema    = gtk_editable_get_chars(GTK_EDITABLE(EN_tema),    0, -1);
   subtema = gtk_editable_get_chars(GTK_EDITABLE(EN_subtema), 0, -1);

   sprintf (PG_command,"SELECT codigo_subtema from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '%s' and codigo_subtema != '          ' and orden <= %d order by orden",
            materia, tema, subtema, orden_subtema);
   res = PQEXEC(DATABASE, PG_command);
   N = PQntuples(res);
   for (i=0;i<N;i++)
       {
	sprintf (PG_command,"UPDATE bd_materias SET orden=%d  where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'",
		 i+1, materia, tema, PQgetvalue (res, i, 0));
        res_aux = PQEXEC(DATABASE, PG_command); PQclear(res_aux);
       }
   PQclear(res);

   sprintf (PG_command,"SELECT codigo_subtema from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '%s' and codigo_subtema != '          ' and orden >= %d order by orden",
            materia, tema, subtema, orden_subtema);
   res = PQEXEC(DATABASE, PG_command);
   N = PQntuples(res);
   for (i=0;i<N;i++)
       {
	sprintf (PG_command,"UPDATE bd_materias SET orden=%d  where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'",
		 orden_subtema+i+1, materia, tema, PQgetvalue (res, i, 0));
        res_aux = PQEXEC(DATABASE, PG_command); PQclear(res_aux);
       }
   PQclear(res);

   g_free (materia);
   g_free (tema);
   g_free (subtema);

   /* end the transaction */
   res = PQEXEC(DATABASE, "END"); PQclear(res);
}

void Cambio_Descripcion_Materia(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_descripcion_materia);
}

void Cambio_Descripcion_Tema(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_descripcion_tema);
}

void Cambio_Descripcion_Subtema(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_descripcion_subtema);
}

void Imprime_listado (GtkWidget *widget, gpointer user_data)
{
   FILE * Archivo_Latex;
   gchar *materia, *tema, *materia_descripcion, *tema_descripcion;
   int i,j,k;
   int N_materias, N_temas, N_subtemas;
   char hilera_antes [2000], hilera_despues [2000];
   char PG_command [2000];
   PGresult *res_materia, *res_tema, *res_subtema, *res_update;

   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show         (window3);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   materia             = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);
   tema                = gtk_editable_get_chars(GTK_EDITABLE(EN_tema),    0, -1);
   materia_descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion_materia)    , 0, -1);
   tema_descripcion    = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion_tema), 0, -1);

   Archivo_Latex = fopen ("EX2000.tex","w");
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.05);
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\documentclass[9pt, journal,oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");

   if (materia[0] == '\0')
      {
       fprintf (Archivo_Latex, "\\title{Materias, Temas y Subtemas}\n");
       fprintf (Archivo_Latex, "\\maketitle\n\n");

       sprintf (PG_command,"SELECT * from bd_materias where codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_materia");
       res_materia = PQEXEC(DATABASE, PG_command);
       N_materias = PQntuples(res_materia);
       for (i=0; i<N_materias; i++)
           {
	    sprintf (hilera_antes,"\\section*{\\textbf{%s (%s)}}", PQgetvalue (res_materia, i, 3),PQgetvalue (res_materia, i, 0));
            hilera_LATEX (hilera_antes, hilera_despues);
            fprintf (Archivo_Latex, "%s\n\n", hilera_despues);

            sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema != '          ' and codigo_subtema = '          ' order by orden, codigo_tema",PQgetvalue (res_materia, i, 0));
            res_tema = PQEXEC(DATABASE, PG_command);
            N_temas = PQntuples(res_tema);
            if (N_temas) fprintf (Archivo_Latex, "\\begin{enumerate}\n");
            for (j=0; j<N_temas; j++)
	        {
                 sprintf (PG_command,"UPDATE bd_materias set orden=%d where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '",
			  j+1,
                          PQgetvalue (res_materia, i, 0),
                          PQgetvalue (res_tema,    j, 1));
                 res_update = PQEXEC(DATABASE, PG_command);
                 PQclear(res_update);

   	         sprintf (hilera_antes,"\\item \\textbf{%s (%s)}", PQgetvalue (res_tema, j, 3),PQgetvalue (res_tema, j, 1));
                 hilera_LATEX (hilera_antes, hilera_despues);
                 fprintf (Archivo_Latex, "%s\n\n", hilera_despues);

                 sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' order by orden",
			              PQgetvalue (res_materia, i, 0), PQgetvalue (res_tema, j, 1));
                 res_subtema = PQEXEC(DATABASE, PG_command);
                 N_subtemas = PQntuples(res_subtema);
                 if (N_subtemas) fprintf (Archivo_Latex, "\\begin{enumerate}\n");
                 for (k=0; k<N_subtemas; k++)
		     {
                      sprintf (PG_command,"UPDATE bd_materias set orden=%d where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'",
			       k+1,
                               PQgetvalue (res_materia, i, 0),
                               PQgetvalue (res_tema,    j, 1),
                               PQgetvalue (res_subtema, k, 2));
                      res_update = PQEXEC(DATABASE, PG_command);
                      PQclear(res_update);

     	              sprintf (hilera_antes,"\\item %s (%s)", PQgetvalue (res_subtema, k, 3),PQgetvalue (res_subtema, k, 2));
                      hilera_LATEX (hilera_antes, hilera_despues);
                      fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
		     }
                 PQclear(res_subtema);
                 if (N_subtemas) fprintf (Archivo_Latex, "\\end{enumerate}\n");
	        }
            PQclear(res_tema);
            if (N_temas) fprintf (Archivo_Latex, "\\end{enumerate}\n");

            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.05 + (long double) i/N_materias * 0.65);
            while (gtk_events_pending ()) gtk_main_iteration ();
           }
       PQclear(res_materia);
      }
   else
      {
       if (tema[0] == '\0')
	  {
	   sprintf (hilera_antes, "\\title{Temas y Subtemas de %s (%s)}\n", materia_descripcion, materia);
           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf (Archivo_Latex, "%s", hilera_despues);
           fprintf (Archivo_Latex, "\\maketitle\n\n");

           sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema != '          ' and codigo_subtema = '          ' order by orden",materia);
           res_tema = PQEXEC(DATABASE, PG_command);
           N_temas = PQntuples(res_tema);
           if (N_temas) fprintf (Archivo_Latex, "\\begin{enumerate}\n");
           for (j=0; j<N_temas; j++)
	        {
   	         sprintf (hilera_antes,"\\item \\textbf{%s (%s)}", PQgetvalue (res_tema, j, 3),PQgetvalue (res_tema, j, 1));
                 hilera_LATEX (hilera_antes, hilera_despues);
                 fprintf (Archivo_Latex, "%s\n\n", hilera_despues);

                 sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' order by orden",
			              materia, PQgetvalue (res_tema, j, 1));
                 res_subtema = PQEXEC(DATABASE, PG_command);
                 N_subtemas = PQntuples(res_subtema);
                 if (N_subtemas) fprintf (Archivo_Latex, "\\begin{enumerate}\n");
                 for (k=0; k<N_subtemas; k++)
		     {
     	              sprintf (hilera_antes,"\\item %s (%s)", PQgetvalue (res_subtema, k, 3),PQgetvalue (res_subtema, k, 2));
                      hilera_LATEX (hilera_antes, hilera_despues);
                      fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
		     }
                 PQclear(res_subtema);
                 if (N_subtemas) fprintf (Archivo_Latex, "\\end{enumerate}\n");
	        }
           PQclear(res_tema);
           if (N_temas) fprintf (Archivo_Latex, "\\end{enumerate}\n");
	  }
       else
	  {
	   sprintf (hilera_antes, "\\title{Subtemas de %s (%s) \\\\%s (%s)}\n", tema_descripcion, tema, materia_descripcion, materia);
           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf (Archivo_Latex, "%s", hilera_despues);
           fprintf (Archivo_Latex, "\\maketitle\n\n");

           sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' order by orden",
	 	              materia, tema);
           res_subtema = PQEXEC(DATABASE, PG_command);
           N_subtemas = PQntuples(res_subtema);
           if (N_subtemas) fprintf (Archivo_Latex, "\\begin{enumerate}\n");
           for (k=0; k<N_subtemas; k++)
	       {
     	        sprintf (hilera_antes,"\\item %s (%s)", PQgetvalue (res_subtema, k, 3),PQgetvalue (res_subtema, k, 2));
                hilera_LATEX (hilera_antes, hilera_despues);
                fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
	       }
           PQclear(res_subtema);
           if (N_subtemas) fprintf (Archivo_Latex, "\\end{enumerate}\n");
	  }
      }

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.7);
   while (gtk_events_pending ()) gtk_main_iteration ();

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX2000", 1, PB_reporte, 0.7, 0.25, NULL, NULL);

   g_free (materia);
   g_free (materia_descripcion);
   g_free (tema);
   g_free (tema_descripcion);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 1.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   gtk_widget_hide (window3);
   gtk_widget_set_sensitive(window1, 1);
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

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex2000_destroy (GtkWidget *widget, gpointer user_data) 
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
  Imprime_listado (widget, user_data);
}

void on_BN_save_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Registro (widget, user_data);
}

void on_BN_delete_clicked(GtkWidget *widget, gpointer user_data)
{
  Borra_Registro (widget, user_data);
}

void on_EN_materia_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Materia (widget, user_data);
}

void on_EN_descripcion_materia_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Descripcion_Materia (widget, user_data);
}

void on_EN_descripcion_tema_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Descripcion_Tema (widget, user_data);
}

void on_EN_descripcion_subtema_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Descripcion_Subtema (widget, user_data);
}

void on_EN_tema_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Tema (widget, user_data);
}

void on_EN_subtema_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Subtema (widget, user_data);
}

void on_CB_materia_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * materia;

  materia = gtk_combo_box_get_active_text(CB_materia);

  if (materia)
     {
      g_free (materia);
      Cambio_Materia_Combo (widget, user_data);
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
      Cambio_Tema_Combo (widget, user_data);
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
      Cambio_Subtema_Combo (widget, user_data);
     }
  else
     g_free (subtema);
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
}
