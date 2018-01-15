/*
Compilar con:

cc -o EX2010 EX2010.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX2010:                                */
/*                                         */
/*    Actualización Objetivos de Cursos    */
/*    The Examiner 0.1                     */
/*    3 de Diciembre del 2011              */
/*    Autor: Francisco J. Torres-Rojas     */
/*                                         */
/*******************************************/
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
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
GError    * error = NULL;
GtkWidget *window1                = NULL;
GtkWidget *window2                = NULL;
GtkComboBox *CB_materia             = NULL;
GtkWidget *EB_objetivos = NULL;
GtkWidget *FR_objetivos = NULL;

GtkWidget *FR_general             = NULL;
GtkWidget *FR_especifico1         = NULL;
GtkWidget *FR_especifico2         = NULL;
GtkWidget *FR_especifico3         = NULL;
GtkWidget *FR_especifico4         = NULL;
GtkWidget *FR_especifico5         = NULL;
GtkWidget *FR_especifico6         = NULL;
GtkWidget *FR_especifico7         = NULL;
GtkWidget *FR_especifico8         = NULL;
GtkTextView *TV_general           = NULL;
GtkTextView *TV_especifico1       = NULL;
GtkTextView *TV_especifico2       = NULL;
GtkTextView *TV_especifico3       = NULL;
GtkTextView *TV_especifico4       = NULL;
GtkTextView *TV_especifico5       = NULL;
GtkTextView *TV_especifico6       = NULL;
GtkTextView *TV_especifico7       = NULL;
GtkTextView *TV_especifico8       = NULL;
GtkWidget *FR_botones             = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_save                = NULL;
GtkWidget *BN_delete              = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_terminar            = NULL;
GtkWidget *BN_ok                  = NULL;

GtkTextBuffer * buffer_TV_general;
GtkTextBuffer * buffer_TV_especifico1;
GtkTextBuffer * buffer_TV_especifico2;
GtkTextBuffer * buffer_TV_especifico3;
GtkTextBuffer * buffer_TV_especifico4;
GtkTextBuffer * buffer_TV_especifico5;
GtkTextBuffer * buffer_TV_especifico6;
GtkTextBuffer * buffer_TV_especifico7;
GtkTextBuffer * buffer_TV_especifico8;
GtkWidget *FR_update              = NULL;
GtkWidget *EB_update              = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Init_Fields();
void Interface_Coloring ();
void Cambio_Materia_Combo(GtkWidget *widget, gpointer user_data);
void Connect_widgets();
void Graba_Registro (GtkWidget *widget, gpointer user_data);
void prepara_hilera_postgres (unsigned char * antes, unsigned char * despues);
void Borra_Registro (GtkWidget *widget, gpointer user_data);
void Imprime_Objetivos (GtkWidget *widget, gpointer user_data);
void Fin_de_Programa (GtkWidget *widget, gpointer user_data);
void Fin_Ventana (GtkWidget *widget, gpointer user_data);
void on_WN_ex2010_destroy (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data);
void on_CB_materia_changed(GtkWidget *widget, gpointer user_data);
void on_BN_save_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_delete_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_print_clicked(GtkWidget *widget, gpointer user_data);

int  Acomoda_objetivos (char especifico_corregida [8][4097]);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

int REGISTRO_YA_EXISTE   = 0;
int N_materias = 0;

char codigo_materia[CODIGO_MATERIA_SIZE + 1];

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX2010.glade", &error))
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
          gtk_main();
	 }
     }

  return 0;
}

void Connect_widgets()
{
  window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex2010"));
  window2                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));


  CB_materia             = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_materia"));

  EB_objetivos           = GTK_WIDGET (gtk_builder_get_object (builder, "EB_objetivos"));
  FR_objetivos           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_objetivos"));
  FR_general             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_general"));
  FR_especifico1         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_especifico1"));
  FR_especifico2         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_especifico2"));
  FR_especifico3         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_especifico3"));
  FR_especifico4         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_especifico4"));
  FR_especifico5         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_especifico5"));
  FR_especifico6         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_especifico6"));
  FR_especifico7         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_especifico7"));
  FR_especifico8         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_especifico8"));
  TV_general             = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_general"));
  TV_especifico1         = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_especifico1"));
  TV_especifico2         = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_especifico2"));
  TV_especifico3         = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_especifico3"));
  TV_especifico4         = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_especifico4"));
  TV_especifico5         = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_especifico5"));
  TV_especifico6         = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_especifico6"));
  TV_especifico7         = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_especifico7"));
  TV_especifico8         = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_especifico8"));
  FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  BN_print               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_save                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
  BN_delete              = GTK_WIDGET (gtk_builder_get_object (builder, "BN_delete"));
  BN_undo                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_ok                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
  EB_update         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
  FR_update         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));

  /* Links buffers to text windows */
  buffer_TV_general     = gtk_text_view_get_buffer(TV_general);
  buffer_TV_especifico1 = gtk_text_view_get_buffer(TV_especifico1);
  buffer_TV_especifico2 = gtk_text_view_get_buffer(TV_especifico2);
  buffer_TV_especifico3 = gtk_text_view_get_buffer(TV_especifico3);
  buffer_TV_especifico4 = gtk_text_view_get_buffer(TV_especifico4);
  buffer_TV_especifico5 = gtk_text_view_get_buffer(TV_especifico5);
  buffer_TV_especifico6 = gtk_text_view_get_buffer(TV_especifico6);
  buffer_TV_especifico7 = gtk_text_view_get_buffer(TV_especifico7);
  buffer_TV_especifico8 = gtk_text_view_get_buffer(TV_especifico8);
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_objetivos,           GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_objetivos,   GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones,     GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(FR_general,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_especifico1, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_especifico2, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_especifico3, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_especifico4, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_especifico5, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_especifico6, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_especifico7, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_especifico8, GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,               GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,                GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_delete,              GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,                GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,                  GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_delete,   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,                  GTK_STATE_ACTIVE, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,              GTK_STATE_NORMAL, &color);
  gdk_color_parse (FINISHED_WORK_FR    , &color);
  gtk_widget_modify_bg(FR_update,              GTK_STATE_NORMAL, &color);
}

void Init_Fields()
{
   int i;
   char hilera[210];
   char text       [2];


   gtk_combo_box_set_active (CB_materia, -1);
   for (i=0; i < N_materias; i++) gtk_combo_box_remove_text (CB_materia, 0);

   res = PQEXEC(DATABASE,"SELECT codigo_materia, descripcion_materia from bd_materias where codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_materia"); 
   N_materias = PQntuples(res);

   for (i=0; i < N_materias; i++)
       {
	sprintf (hilera,"%s %s", PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
        gtk_combo_box_append_text(CB_materia, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_materia, -1);

   text[0] = '\0';
   gtk_text_buffer_set_text(buffer_TV_general    , text, -1);
   gtk_text_buffer_set_text(buffer_TV_especifico1, text, -1);
   gtk_text_buffer_set_text(buffer_TV_especifico2, text, -1);
   gtk_text_buffer_set_text(buffer_TV_especifico3, text, -1);
   gtk_text_buffer_set_text(buffer_TV_especifico4, text, -1);
   gtk_text_buffer_set_text(buffer_TV_especifico5, text, -1);
   gtk_text_buffer_set_text(buffer_TV_especifico6, text, -1);
   gtk_text_buffer_set_text(buffer_TV_especifico7, text, -1);
   gtk_text_buffer_set_text(buffer_TV_especifico8, text, -1);

   gtk_widget_set_sensitive(GTK_WIDGET(TV_general)   , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico1), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico2), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico3), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico4), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico5), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico6), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico7), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico8), 0);

   REGISTRO_YA_EXISTE = 0;

   gtk_widget_set_sensitive(BN_print,  0);
   gtk_widget_set_sensitive(BN_save,   0);
   gtk_widget_set_sensitive(BN_undo,   1);
   gtk_widget_set_sensitive(BN_delete, 0);

   gtk_widget_set_sensitive(GTK_WIDGET(CB_materia), 1);
   gtk_widget_grab_focus   (GTK_WIDGET(CB_materia));
}

void Cambio_Materia_Combo(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *materia;
  char PG_command  [1000];
  PGresult *res, *res_aux;

  gtk_widget_set_sensitive(GTK_WIDGET(CB_materia), 0);
  gtk_widget_set_sensitive(BN_save,    1);

  materia = gtk_combo_box_get_active_text(CB_materia);

  i=0;
  while (materia[i] != ' ') 
        {
         codigo_materia[i] = materia[i];
         i++;
        }
  codigo_materia[i]='\0';

  sprintf (PG_command,"SELECT * from BD_objetivos where materia = '%s'", codigo_materia);
  res = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res))
     {
      REGISTRO_YA_EXISTE = 1;
      gtk_text_buffer_set_text(buffer_TV_general,     PQgetvalue (res, 0, 1), -1);
      gtk_text_buffer_set_text(buffer_TV_especifico1, PQgetvalue (res, 0, 3), -1);
      gtk_text_buffer_set_text(buffer_TV_especifico2, PQgetvalue (res, 0, 4), -1);
      gtk_text_buffer_set_text(buffer_TV_especifico3, PQgetvalue (res, 0, 5), -1);
      gtk_text_buffer_set_text(buffer_TV_especifico4, PQgetvalue (res, 0, 6), -1);
      gtk_text_buffer_set_text(buffer_TV_especifico5, PQgetvalue (res, 0, 7), -1);
      gtk_text_buffer_set_text(buffer_TV_especifico6, PQgetvalue (res, 0, 8), -1);
      gtk_text_buffer_set_text(buffer_TV_especifico7, PQgetvalue (res, 0, 9), -1);
      gtk_text_buffer_set_text(buffer_TV_especifico8, PQgetvalue (res, 0, 10), -1);
      gtk_widget_set_sensitive(BN_print,  1);
      gtk_widget_set_sensitive(BN_delete, 1);
     }
  else
     {
      REGISTRO_YA_EXISTE = 0;
      gtk_widget_set_sensitive(BN_delete, 0);
     }

  gtk_widget_set_sensitive(GTK_WIDGET(TV_general)    , 1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico1), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico2), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico3), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico4), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico5), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico6), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico7), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_especifico8), 1);

  gtk_widget_grab_focus   (GTK_WIDGET(TV_general));

  gtk_widget_set_sensitive(BN_save,   1);

  g_free (materia);
}

void Graba_Registro (GtkWidget *widget, gpointer user_data)
{
  int i, N_objetivos;
  char PG_command [40000];

  GtkTextIter start;
  GtkTextIter end;
  gchar *general, *especifico1, *especifico2, *especifico3, *especifico4, *especifico5, *especifico6, *especifico7, *especifico8;

  char *general_corregida;
  char especifico_corregida [8][4097];

  gtk_text_buffer_get_start_iter (buffer_TV_general, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_general, &end);
  general = gtk_text_buffer_get_text (buffer_TV_general, &start, &end, FALSE);
  general_corregida = (char *) malloc (sizeof(char) * strlen(general)*2);
  prepara_hilera_postgres (general, general_corregida);

  gtk_text_buffer_get_start_iter (buffer_TV_especifico1, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_especifico1, &end);
  especifico1 = gtk_text_buffer_get_text (buffer_TV_especifico1, &start, &end, FALSE);

  prepara_hilera_postgres (especifico1, especifico_corregida [0]);

  gtk_text_buffer_get_start_iter (buffer_TV_especifico2, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_especifico2, &end);
  especifico2 = gtk_text_buffer_get_text (buffer_TV_especifico2, &start, &end, FALSE);

  prepara_hilera_postgres (especifico2, especifico_corregida [1]);

  gtk_text_buffer_get_start_iter (buffer_TV_especifico3, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_especifico3, &end);
  especifico3 = gtk_text_buffer_get_text (buffer_TV_especifico3, &start, &end, FALSE);

  prepara_hilera_postgres (especifico3, especifico_corregida [2]);

  gtk_text_buffer_get_start_iter (buffer_TV_especifico4, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_especifico4, &end);
  especifico4 = gtk_text_buffer_get_text (buffer_TV_especifico4, &start, &end, FALSE);

  prepara_hilera_postgres (especifico4, especifico_corregida [3]);

  gtk_text_buffer_get_start_iter (buffer_TV_especifico5, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_especifico5, &end);
  especifico5 = gtk_text_buffer_get_text (buffer_TV_especifico5, &start, &end, FALSE);

  prepara_hilera_postgres (especifico5, especifico_corregida[4]);

  gtk_text_buffer_get_start_iter (buffer_TV_especifico6, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_especifico6, &end);
  especifico6 = gtk_text_buffer_get_text (buffer_TV_especifico6, &start, &end, FALSE);

  prepara_hilera_postgres (especifico6, especifico_corregida[5]);

  gtk_text_buffer_get_start_iter (buffer_TV_especifico7, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_especifico7, &end);
  especifico7 = gtk_text_buffer_get_text (buffer_TV_especifico7, &start, &end, FALSE);

  prepara_hilera_postgres (especifico7, especifico_corregida[6]);

  gtk_text_buffer_get_start_iter (buffer_TV_especifico8, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_especifico8, &end);
  especifico8 = gtk_text_buffer_get_text (buffer_TV_especifico8, &start, &end, FALSE);

  prepara_hilera_postgres (especifico8, especifico_corregida[7]);

  N_objetivos = Acomoda_objetivos (especifico_corregida);

  if (REGISTRO_YA_EXISTE)
     {
      sprintf (PG_command,"UPDATE bd_objetivos SET objetivo_general=E'%s', N_objetivos=%d, objetivo_especifico1=E'%s',objetivo_especifico2=E'%s',objetivo_especifico3=E'%s',objetivo_especifico4=E'%s',objetivo_especifico5=E'%s',objetivo_especifico6=E'%s',objetivo_especifico7=E'%s',objetivo_especifico8=E'%s' where materia = '%s'", 
               general_corregida, N_objetivos, 
                                  especifico_corregida[0],especifico_corregida[1],especifico_corregida[2],especifico_corregida[3],
	                          especifico_corregida[4],especifico_corregida[5],especifico_corregida[6],especifico_corregida[7],
               codigo_materia);
      res = PQEXEC(DATABASE, PG_command); PQclear(res);
     }
  else
     {
      sprintf (PG_command,"INSERT into bd_objetivos values ('%s',E'%s',%d,E'%s',E'%s',E'%s',E'%s',E'%s',E'%s',E'%s',E'%s')",
	       codigo_materia,
               general_corregida, N_objetivos,
                                  especifico_corregida[0],especifico_corregida[1],especifico_corregida[2],especifico_corregida[3],
       	                          especifico_corregida[4],especifico_corregida[5],especifico_corregida[6],especifico_corregida[7]);

      res = PQEXEC(DATABASE, PG_command); PQclear(res);
     }
 
  g_free (general);
  g_free (especifico1);
  g_free (especifico2);
  g_free (especifico3);
  g_free (especifico4);
  g_free (especifico5);
  g_free (especifico6);
  g_free (especifico7);
  g_free (especifico8);

  free (general_corregida);

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

int  Acomoda_objetivos (char especifico_corregida [8][4097])
{
  int i,j,k, N_objetivos;

  i=0;
  N_objetivos = 0;
  while (i < 8)
        {
         if (especifico_corregida[i][0] == '\0')
	    {
	     j = i + 1;
             while ((j < 8) && especifico_corregida[j][0] == '\0') j++;
             if (j < 8)
	        {
		 k = 0;
                 while (especifico_corregida [i][k] = especifico_corregida[j][k++]);
                 especifico_corregida[j][0] = '\0';
	        }
	    }

         if (especifico_corregida[i][0] != '\0') N_objetivos++;
	 i++;
        }

  return (N_objetivos);
}

void prepara_hilera_postgres (unsigned char * antes, unsigned char * despues)
{
  while (*antes)
        {
         if (*antes == '\\')
	    {
             *despues++ = '\\';
             *despues++ = *(antes++); 
	    }
         else
            if (*antes == '\'')
	       {
                *despues++ = '\'';
                *despues++ = *(antes++); 
	       }
            else
               *despues++ = *(antes++); 
        }
  *despues = '\0';
}

void Borra_Registro (GtkWidget *widget, gpointer user_data)
{
   char PG_command [1000];

   sprintf (PG_command,"DELETE from bd_objetivos where materia = '%s'", codigo_materia);
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

   Init_Fields ();
}

void Imprime_Objetivos (GtkWidget *widget, gpointer user_data)
{
   FILE * Archivo_Latex;
   char PG_command [500];
   char hilera_antes [9000], hilera_despues [9000];
   GtkTextIter start;
   GtkTextIter end;
   gchar *objetivo_general;
   gchar *objetivo_especifico;
 
   Archivo_Latex = fopen ("EX2010.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[10pt,journal, oneside, onecolumn]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");

   sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '     ' and codigo_subtema = '     '", codigo_materia);
   res = PQEXEC(DATABASE, PG_command);
   if (PQntuples(res))
      {
       sprintf (hilera_antes, "\\title{%s}", PQgetvalue (res, 0, 0));
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n", hilera_despues);
      }
   else
      fprintf (Archivo_Latex, "\\title{%s}\n", codigo_materia);

   fprintf (Archivo_Latex, "\\maketitle\n\n");
   PQclear(res);

   fprintf (Archivo_Latex, "\\subsection*{\\large{\\textbf{Objetivo General}}}\n\n");

   fprintf (Archivo_Latex, "\\begin{itemize}\n");
   gtk_text_buffer_get_start_iter (buffer_TV_general, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_general, &end);
   objetivo_general = gtk_text_buffer_get_text (buffer_TV_general, &start, &end, FALSE);
   hilera_LATEX (objetivo_general, hilera_despues);
   fprintf (Archivo_Latex, "\\item %s\n\n\n", hilera_despues);
   fprintf (Archivo_Latex, "\\end{itemize}\n");
   g_free (objetivo_general);

   fprintf (Archivo_Latex, "\\subsection*{\\large{\\textbf{Objetivos Espec\\'{i}ficos}}}\n\n");

   fprintf (Archivo_Latex, "\\begin{enumerate}\n");

   gtk_text_buffer_get_start_iter (buffer_TV_especifico1, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_especifico1, &end);
   objetivo_especifico = gtk_text_buffer_get_text (buffer_TV_especifico1, &start, &end, FALSE);
   if (objetivo_especifico[0] != '\0')
      {
       hilera_LATEX (objetivo_especifico, hilera_despues);
       fprintf (Archivo_Latex, "\\item %s\n\n\n", hilera_despues);
      }
   g_free (objetivo_especifico);

   gtk_text_buffer_get_start_iter (buffer_TV_especifico2, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_especifico2, &end);
   objetivo_especifico = gtk_text_buffer_get_text (buffer_TV_especifico2, &start, &end, FALSE);
   if (objetivo_especifico[0] != '\0')
      {
       hilera_LATEX (objetivo_especifico, hilera_despues);
       fprintf (Archivo_Latex, "\\item %s\n\n\n", hilera_despues);
      }
   g_free (objetivo_especifico);

   gtk_text_buffer_get_start_iter (buffer_TV_especifico3, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_especifico3, &end);
   objetivo_especifico = gtk_text_buffer_get_text (buffer_TV_especifico3, &start, &end, FALSE);
   if (objetivo_especifico[0] != '\0')
      {
       hilera_LATEX (objetivo_especifico, hilera_despues);
       fprintf (Archivo_Latex, "\\item %s\n\n\n", hilera_despues);
      }
   g_free (objetivo_especifico);

   gtk_text_buffer_get_start_iter (buffer_TV_especifico4, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_especifico4, &end);
   objetivo_especifico = gtk_text_buffer_get_text (buffer_TV_especifico4, &start, &end, FALSE);
   if (objetivo_especifico[0] != '\0')
      {
       hilera_LATEX (objetivo_especifico, hilera_despues);
       fprintf (Archivo_Latex, "\\item %s\n\n\n", hilera_despues);
      }
   g_free (objetivo_especifico);

   gtk_text_buffer_get_start_iter (buffer_TV_especifico5, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_especifico5, &end);
   objetivo_especifico = gtk_text_buffer_get_text (buffer_TV_especifico5, &start, &end, FALSE);
   if (objetivo_especifico[0] != '\0')
      {
       hilera_LATEX (objetivo_especifico, hilera_despues);
       fprintf (Archivo_Latex, "\\item %s\n\n\n", hilera_despues);
      }
   g_free (objetivo_especifico);

   gtk_text_buffer_get_start_iter (buffer_TV_especifico6, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_especifico6, &end);
   objetivo_especifico = gtk_text_buffer_get_text (buffer_TV_especifico6, &start, &end, FALSE);
   if (objetivo_especifico[0] != '\0')
      {
       hilera_LATEX (objetivo_especifico, hilera_despues);
       fprintf (Archivo_Latex, "\\item %s\n\n\n", hilera_despues);
      }
   g_free (objetivo_especifico);

   gtk_text_buffer_get_start_iter (buffer_TV_especifico7, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_especifico7, &end);
   objetivo_especifico = gtk_text_buffer_get_text (buffer_TV_especifico7, &start, &end, FALSE);
   if (objetivo_especifico[0] != '\0')
      {
       hilera_LATEX (objetivo_especifico, hilera_despues);
       fprintf (Archivo_Latex, "\\item %s\n\n\n", hilera_despues);
      }
   g_free (objetivo_especifico);

   gtk_text_buffer_get_start_iter (buffer_TV_especifico8, &start);
   gtk_text_buffer_get_end_iter   (buffer_TV_especifico8, &end);
   objetivo_especifico = gtk_text_buffer_get_text (buffer_TV_especifico8, &start, &end, FALSE);
   if (objetivo_especifico[0] != '\0')
      {
       hilera_LATEX (objetivo_especifico, hilera_despues);
       fprintf (Archivo_Latex, "\\item %s\n\n\n", hilera_despues);
      }
   g_free (objetivo_especifico);

   fprintf (Archivo_Latex, "\\end{enumerate}\n");

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX2010", 1, NULL, 0.0, 0.0, NULL, NULL);
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
void on_WN_ex2010_destroy (GtkWidget *widget, gpointer user_data) 
{
  Fin_Ventana (widget, user_data);
}

void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data)
{
  Fin_de_Programa (widget, user_data);
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

void on_BN_save_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Registro (widget, user_data);
}

void on_BN_undo_clicked(GtkWidget *widget, gpointer user_data)
{
  Init_Fields ();
}

void on_BN_delete_clicked(GtkWidget *widget, gpointer user_data)
{
  Borra_Registro (widget, user_data);
}

void on_BN_print_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Objetivos (widget, user_data);
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
}
