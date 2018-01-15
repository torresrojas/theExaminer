/*
Compile with:

cc -o EX1010 EX1010.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX1010:                                */
/*                                         */
/*    Backup/Restore                       */
/*    The Examiner 0.2                     */
/*    June, 17th 2013                      */
/*    Author: Francisco J. Torres-Rojas    */
/*******************************************/
#include <stdlib.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <locale.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
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

GtkWidget       *EB_backup          = NULL;
GtkWidget       *FR_backup          = NULL;
GtkToggleButton *CK_datos_backup    = NULL;
GtkToggleButton *CK_software_backup = NULL;
GtkWidget       *EN_datos_backup    = NULL;
GtkFileChooser  *FC_datos_backup    = NULL;
GtkWidget       *EN_software_backup = NULL;
GtkFileChooser  *FC_software_backup = NULL;
GtkWidget       *BN_backup          = NULL;

GtkWidget *EB_datos_restore          = NULL;
GtkWidget *FR_datos_restore          = NULL;
GtkToggleButton *CK_datos_restore    = NULL;
GtkWidget       *EN_datos_restore    = NULL;
GtkFileChooser  *FC_datos_restore    = NULL;
GtkComboBox     *CB_datos_restore    = NULL;
GtkWidget *BN_datos_restore          = NULL;

GtkWidget *EB_software_restore       = NULL;
GtkWidget *FR_software_restore       = NULL;
GtkToggleButton *CK_software_restore = NULL;
GtkWidget       *EN_software_restore = NULL;
GtkFileChooser  *FC_software_restore = NULL;
GtkComboBox     *CB_software_restore = NULL;
GtkWidget *BN_software_restore       = NULL;

GtkWidget *FR_SR                       = NULL;
GtkWidget *BN_confirm_software_restore = NULL;
GtkWidget *BN_cancel_software_restore  = NULL;

GtkWidget *FR_DR                       = NULL;
GtkWidget *BN_confirm_datos_restore    = NULL;
GtkWidget *BN_cancel_datos_restore     = NULL;

GtkWidget *FR_sin_datos                = NULL;
GtkWidget *BN_ok                       = NULL;

GtkWidget *FR_botones             = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_terminar            = NULL;

GtkLabel  *LB_software_mensaje    = NULL;
GtkLabel  *LB_datos_mensaje       = NULL;

GtkWidget *EB_backup_ready        = NULL;
GtkWidget *FR_backup_ready        = NULL;
GtkWidget *BN_backup_ready        = NULL;
GtkWidget *EB_restore_ready       = NULL;
GtkWidget *FR_restore_ready       = NULL;
GtkWidget *BN_restore_ready       = NULL;
GtkWidget *EB_haciendo_backup     = NULL;
GtkWidget *FR_haciendo_backup     = NULL;
GtkWidget *EB_restaurando         = NULL;
GtkWidget *FR_restaurando         = NULL;

GtkWidget *PB_backup              = NULL;
GtkWidget *PB_restore             = NULL;

/**************/
/* Prototipos */
/**************/
void Connect_widgets                               ();
void Data_Backup                                   (gchar * Directorio_datos);
void Datos_Restore                                 ();
void Execute_Backup                                ();
int Filter_file_name                               (char *filename, char * type, char * Directorio);
void Fin_de_Programa                               (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                                   (GtkWidget *widget, gpointer user_data);
void Init_Fields                                   ();
void Interface_Coloring                            ();
void Load_datos_restore                            ();
void Load_software_restore                         ();
int main                                           (int argc, char *argv[]);
void on_BN_backup_clicked                          (GtkWidget *widget, gpointer user_data);
void on_BN_cancel_datos_restore_clicked            (GtkWidget *widget, gpointer user_data);
void on_BN_cancel_software_restore_clicked         (GtkWidget *widget, gpointer user_data);
void on_BN_confirm_datos_restore_clicked           (GtkWidget *widget, gpointer user_data);
void on_BN_confirm_software_restore_clicked        (GtkWidget *widget, gpointer user_data);
void on_BN_datos_restore_clicked                   (GtkWidget *widget, gpointer user_data);
void on_BN_software_restore_clicked                (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked                        (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked                            (GtkWidget *widget, gpointer user_data);
void on_BN_ok_clicked                              (GtkWidget *widget, gpointer user_data);
void on_CK_datos_backup_toggled                    (GtkWidget *widget, gpointer user_data);
void on_CK_datos_restore_toggled                   (GtkWidget *widget, gpointer user_data);
void on_CK_software_backup_toggled                 (GtkWidget *widget, gpointer user_data);
void on_CK_software_restore_toggled                (GtkWidget *widget, gpointer user_data);
void on_FC_datos_restore_current_folder_changed    (GtkWidget *widget, gpointer user_data);
void on_FC_software_restore_current_folder_changed (GtkWidget *widget, gpointer user_data);
void on_WN_ex1010_destroy                          (GtkWidget *widget, gpointer user_data);
void Read_Only_Fields                              ();
void size_to_hilera                                (long int file_size, char * hilera_size);
void Software_Backup                               (gchar * Directorio_software);
void Software_Restore                              ();
void Toggle_datos_backup                           (GtkWidget *widget, gpointer user_data);
void Toggle_datos_restore                          (GtkWidget *widget, gpointer user_data);
void Toggle_software_backup                        (GtkWidget *widget, gpointer user_data);
void Toggle_software_restore                       (GtkWidget *widget, gpointer user_data);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;
int N_files_datos    = 0;
int N_files_software = 0;
#define NAME_SIZE   28
#define MAX_BACKUPS 200
char * months[] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Setiembre", "Octubre", "Noviembre", "Diciembre"};
char datos_restore    [MAX_BACKUPS][NAME_SIZE];
char software_restore [MAX_BACKUPS][NAME_SIZE];
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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX1010.glade", &error))
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

	  /* Displays main window */
          gtk_widget_show(window1);  

          gtk_file_chooser_set_current_folder (FC_datos_backup,     parametros.ruta_backup);
          gtk_file_chooser_set_current_folder (FC_software_backup,  parametros.ruta_backup);
          gtk_file_chooser_set_current_folder (FC_datos_restore,    parametros.ruta_backup);
          gtk_file_chooser_set_current_folder (FC_software_restore, parametros.ruta_backup);

          Init_Fields ();              

          /* start the event loop */
          gtk_main ();
	 }
     }

  return 0;
}

void Connect_widgets()
{
  window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex1010"));
  window2                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_datos_restore"));
  window3                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_software_restore"));
  window4                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_sin_datos"));
  window5                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_backup_ready"));
  window6                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_restore_ready"));
  window7                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_haciendo_backup"));
  window8                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_restaurando"));

  EB_backup              = GTK_WIDGET (gtk_builder_get_object (builder, "EB_backup"));
  FR_backup              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_backup"));
  CK_datos_backup        = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_datos_backup"));
  CK_software_backup     = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_software_backup"));
  EN_datos_backup        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_datos_backup"));
  FC_datos_backup        = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_datos_backup"));
  EN_software_backup     = GTK_WIDGET (gtk_builder_get_object (builder, "EN_software_backup"));
  FC_software_backup     = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_software_backup"));
  BN_backup              = GTK_WIDGET (gtk_builder_get_object (builder, "BN_backup"));

  EB_datos_restore       = GTK_WIDGET (gtk_builder_get_object (builder, "EB_datos_restore"));
  FR_datos_restore       = GTK_WIDGET (gtk_builder_get_object (builder, "FR_datos_restore"));
  CK_datos_restore       = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_datos_restore"));
  EN_datos_restore       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_datos_restore"));
  FC_datos_restore       = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_datos_restore"));
  CB_datos_restore       = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_datos_restore"));
  BN_datos_restore       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_datos_restore"));

  EB_software_restore    = GTK_WIDGET (gtk_builder_get_object (builder, "EB_software_restore"));
  FR_software_restore    = GTK_WIDGET (gtk_builder_get_object (builder, "FR_software_restore"));
  CK_software_restore    = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_software_restore"));
  EN_software_restore    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_software_restore"));
  FC_software_restore    = (GtkFileChooser *) GTK_WIDGET (gtk_builder_get_object (builder, "FC_software_restore"));
  CB_software_restore    = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_software_restore"));
  BN_software_restore    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_software_restore"));

  FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  BN_undo                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));

  FR_SR                       = GTK_WIDGET (gtk_builder_get_object (builder, "FR_SR"));
  BN_confirm_software_restore = GTK_WIDGET (gtk_builder_get_object (builder, "BN_confirm_software_restore"));
  BN_cancel_software_restore  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_cancel_software_restore"));

  FR_DR                       = GTK_WIDGET (gtk_builder_get_object (builder, "FR_DR"));
  BN_confirm_datos_restore    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_confirm_datos_restore"));
  BN_cancel_datos_restore     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_cancel_datos_restore"));

  FR_sin_datos           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_sin_datos"));
  BN_ok                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));

  EB_backup_ready        = GTK_WIDGET (gtk_builder_get_object (builder, "EB_backup_ready"));
  FR_backup_ready        = GTK_WIDGET (gtk_builder_get_object (builder, "FR_backup_ready"));
  BN_backup_ready        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_backup_ready"));

  EB_restore_ready        = GTK_WIDGET (gtk_builder_get_object (builder, "EB_restore_ready"));
  FR_restore_ready        = GTK_WIDGET (gtk_builder_get_object (builder, "FR_restore_ready"));
  BN_restore_ready        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_restore_ready"));

  LB_software_mensaje    = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_software_mensaje"));
  LB_datos_mensaje       = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_datos_mensaje"));

  EB_haciendo_backup     = GTK_WIDGET (gtk_builder_get_object (builder, "EB_haciendo_backup"));
  FR_haciendo_backup     = GTK_WIDGET (gtk_builder_get_object (builder, "FR_haciendo_backup"));
  EB_restaurando         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_restaurando"));
  FR_restaurando         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_restaurando"));
  PB_backup     = GTK_WIDGET (gtk_builder_get_object (builder, "PB_backup"));
  PB_restore    = GTK_WIDGET (gtk_builder_get_object (builder, "PB_restore"));
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_datos_backup,     FALSE);
  gtk_widget_set_can_focus(EN_software_backup,  FALSE);
  gtk_widget_set_can_focus(EN_datos_restore,    FALSE);
  gtk_widget_set_can_focus(EN_software_restore, FALSE);
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_backup,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_datos_restore,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_software_restore, GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_botones,          GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_datos_restore,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_software_restore, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_backup,           GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(EN_software_restore, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_datos_restore,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_datos_backup,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_software_backup,  GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,             GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_backup,           GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_datos_restore,    GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_software_restore, GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,             GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_backup,           GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_datos_restore,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_software_restore, GTK_STATE_ACTIVE, &color);

  gdk_color_parse (PROCESSING_FR, &color);
  gtk_widget_modify_bg(FR_haciendo_backup,  GTK_STATE_NORMAL, &color);  
  gtk_widget_modify_bg(FR_restaurando,      GTK_STATE_NORMAL, &color);  

  gdk_color_parse (PROCESSING_WINDOW, &color);
  gtk_widget_modify_bg(EB_haciendo_backup, GTK_STATE_NORMAL,   &color);  
  gtk_widget_modify_bg(EB_restaurando,     GTK_STATE_NORMAL,   &color);  

  gdk_color_parse (PROCESSING_PB, &color);
  gtk_widget_modify_bg(GTK_WIDGET(PB_backup),  GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(GTK_WIDGET(PB_restore), GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (IMPORTANT_WINDOW, &color);
  gtk_widget_modify_bg(window2,                  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(window3,                  GTK_STATE_NORMAL, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_backup_ready, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_restore_ready, GTK_STATE_NORMAL, &color);

  gdk_color_parse ("dark green", &color);
  gtk_widget_modify_bg(BN_backup_ready, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_restore_ready, GTK_STATE_ACTIVE, &color);

  gdk_color_parse ("green", &color);
  gtk_widget_modify_bg(BN_ok,                      GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_confirm_software_restore,GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_confirm_datos_restore,   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(FR_backup_ready, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(BN_backup_ready, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(FR_restore_ready, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(BN_restore_ready, GTK_STATE_PRELIGHT, &color);

  gdk_color_parse ("red", &color);
  gtk_widget_modify_bg(BN_cancel_software_restore, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_cancel_datos_restore,    GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(FR_sin_datos,               GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_SR,                      GTK_STATE_NORMAL, &color);

  gdk_color_parse ("black", &color);
  gtk_widget_modify_bg(FR_DR,                      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_SR,                      GTK_STATE_NORMAL, &color);

}

void Init_Fields()
{
   gtk_widget_set_sensitive     (GTK_WIDGET(EN_datos_backup),    0);
   gtk_widget_set_sensitive     (GTK_WIDGET(EN_software_backup), 0);
   gtk_widget_set_sensitive     (GTK_WIDGET(FC_datos_backup),    0);
   gtk_widget_set_sensitive     (GTK_WIDGET(FC_software_backup), 0);
   gtk_widget_set_sensitive     (GTK_WIDGET(BN_backup),          0);

   gtk_toggle_button_set_active (CK_datos_backup,    FALSE);
   gtk_toggle_button_set_active (CK_software_backup, FALSE);

   gtk_widget_set_sensitive     (GTK_WIDGET(EN_datos_restore),    0);
   gtk_widget_set_sensitive     (GTK_WIDGET(FC_datos_restore),    0);
   gtk_widget_set_sensitive     (GTK_WIDGET(CB_datos_restore),    0);
   gtk_widget_set_sensitive     (GTK_WIDGET(BN_datos_restore),    0);

   gtk_widget_set_sensitive     (GTK_WIDGET(EN_software_restore), 0);
   gtk_widget_set_sensitive     (GTK_WIDGET(FC_software_restore), 0);
   gtk_widget_set_sensitive     (GTK_WIDGET(CB_software_restore), 0);
   gtk_widget_set_sensitive     (GTK_WIDGET(BN_software_restore), 0);

   gtk_toggle_button_set_active (CK_datos_restore,    FALSE);
   gtk_toggle_button_set_active (CK_software_restore, FALSE);

}

void Load_datos_restore()
{
  int i, j;
  DIR * pdir;
  struct dirent * files;
  char hilera[200];
  char buffer[NAME_SIZE];
  gchar * Directorio;
  struct stat file_data;
  long int file_size;
  char ruta_archivo[2000];
  char hilera_size[100];

  gtk_combo_box_set_active (CB_datos_restore, -1);
  for (i=0; i < N_files_datos; i++) gtk_combo_box_remove_text (CB_datos_restore, 0);

  Directorio = gtk_file_chooser_get_filename(FC_datos_restore);

  N_files_datos = 0;
  pdir = opendir(Directorio);
  if (pdir)
     {
      while((files=readdir(pdir))!=NULL)
           {
	     if (Filter_file_name(files->d_name,"data.", Directorio))
	       {
		strncpy (datos_restore[N_files_datos], files->d_name, NAME_SIZE-1);

		N_files_datos++;
	       }
	   }

      for (j=N_files_datos; j > 0; j--)
	  {
	   for (i=0; i < j-1; i++)
	       {
		if (strcmp(datos_restore[i], datos_restore[i+1]) < 0)
		   {
		    strcpy(buffer            , datos_restore[i]);
		    strcpy(datos_restore[i  ], datos_restore[i+1]);
		    strcpy(datos_restore[i+1], buffer);
		   }
	       }
	  }

      for (i=0; i<N_files_datos; i++)
	  {
	   sprintf(ruta_archivo, "%s/%s/EX.data.pg", Directorio, datos_restore[i]); 
	   if (!stat(ruta_archivo,&file_data))
  	      file_size = file_data.st_size;
	   else
	      file_size = 0;

	   sprintf(ruta_archivo, "%s/%s/EX.figures.tgz", Directorio, datos_restore[i]); 
	   if (!stat(ruta_archivo,&file_data))
  	      file_size += file_data.st_size;

	   size_to_hilera(file_size, hilera_size);

	   sprintf (hilera,"%d de %s del %d a las %02d:%02d:%02d - %s",atoi(datos_restore[i]+16), months[atoi(datos_restore[i]+13)-1], atoi(datos_restore[i]+8),
		    atoi(datos_restore[i]+19),atoi(datos_restore[i]+22),atoi(datos_restore[i]+25), hilera_size);
           gtk_combo_box_append_text(CB_datos_restore, hilera);
	  }

      if (N_files_datos) gtk_combo_box_set_active (CB_datos_restore, 0);
     }

  g_free(Directorio);
}

void size_to_hilera(long int file_size, char * hilera_size)
{
  if (file_size < 1024)
     sprintf (hilera_size, "%ld bytes", file_size);
  else
    if (file_size < (1024 * 1024))
       sprintf (hilera_size, "%.2Lf Kb", (long double)file_size/1024.0);
    else
       sprintf (hilera_size, "%.2Lf Mb", (long double)file_size/(1024.0*1024.0));
}

void Load_software_restore()
{
  int i, j;
  DIR * pdir;
  struct dirent * files;
  char hilera[100];
  char buffer[NAME_SIZE];
  gchar * Directorio;
  struct stat file_data;
  long int file_size;
  char ruta_archivo[2000];
  char hilera_size[100];

  gtk_combo_box_set_active (CB_software_restore, -1);
  for (i=0; i < N_files_software; i++) gtk_combo_box_remove_text (CB_software_restore, 0);

  Directorio = gtk_file_chooser_get_filename(FC_software_restore);

  N_files_software = 0;
  pdir = opendir(Directorio);
  if (pdir)
     {
      while((files=readdir(pdir))!=NULL)
           {
	     if (Filter_file_name(files->d_name,"soft.", Directorio))
	       {
		strncpy (software_restore[N_files_software], files->d_name, NAME_SIZE-1);
		N_files_software++;
	       }
	   }

      for (j=N_files_software; j > 0; j--)
	  {
	   for (i=0; i < j-1; i++)
	       {
		if (strcmp(software_restore[i], software_restore[i+1]) < 0)
		   {
		    strcpy(buffer               , software_restore[i]);
		    strcpy(software_restore[i  ], software_restore[i+1]);
		    strcpy(software_restore[i+1], buffer);
		   }
	       }
	  }
      for (i=0; i<N_files_software; i++)
	  {
	   sprintf(ruta_archivo, "%s/%s/EX.software.tgz", Directorio, software_restore[i]); 
	   if (!stat(ruta_archivo,&file_data))
  	      file_size = file_data.st_size;
	   else
	      file_size = 0;
	   size_to_hilera(file_size, hilera_size);

	   sprintf (hilera,"%d de %s del %d a las %02d:%02d:%02d - %s",atoi(software_restore[i]+16), months[atoi(software_restore[i]+13)-1], atoi(software_restore[i]+8),
		    atoi(software_restore[i]+19),atoi(software_restore[i]+22),atoi(software_restore[i]+25), hilera_size);
           gtk_combo_box_append_text(CB_software_restore, hilera);
	  }

      if (N_files_software) gtk_combo_box_set_active (CB_software_restore, 0);
     }

  g_free(Directorio);
}

void Toggle_datos_backup(GtkWidget *widget, gpointer user_data)
{
  if (gtk_toggle_button_get_active(CK_datos_backup))
     {
      gtk_widget_set_sensitive (GTK_WIDGET(EN_datos_backup),     1);
      gtk_widget_set_sensitive (GTK_WIDGET(FC_datos_backup),     1);
      gtk_widget_set_sensitive (GTK_WIDGET(BN_backup),           1);
      gtk_widget_set_sensitive (GTK_WIDGET(FR_datos_restore),    0);
      gtk_widget_set_sensitive (GTK_WIDGET(FR_software_restore), 0);
     }
  else
     {
      gtk_widget_set_sensitive (GTK_WIDGET(EN_datos_backup),       0);
      gtk_widget_set_sensitive (GTK_WIDGET(FC_datos_backup),       0);
      if (!gtk_toggle_button_get_active(CK_software_backup))
	 {
          gtk_widget_set_sensitive (GTK_WIDGET(BN_backup),           0);
          gtk_widget_set_sensitive (GTK_WIDGET(FR_datos_restore),    1);
          gtk_widget_set_sensitive (GTK_WIDGET(FR_software_restore), 1);
	 }
     }
}

void Toggle_software_backup(GtkWidget *widget, gpointer user_data)
{
  if (gtk_toggle_button_get_active(CK_software_backup))
     {
      gtk_widget_set_sensitive (GTK_WIDGET(EN_software_backup),  1);
      gtk_widget_set_sensitive (GTK_WIDGET(FC_software_backup),  1);
      gtk_widget_set_sensitive (GTK_WIDGET(BN_backup),           1);
      gtk_widget_set_sensitive (GTK_WIDGET(FR_datos_restore),    0);
      gtk_widget_set_sensitive (GTK_WIDGET(FR_software_restore), 0);
     }
  else
     {
      gtk_widget_set_sensitive (GTK_WIDGET(EN_software_backup),      0);
      gtk_widget_set_sensitive (GTK_WIDGET(FC_software_backup),      0);
      if (!gtk_toggle_button_get_active(CK_datos_backup))
	 {
          gtk_widget_set_sensitive (GTK_WIDGET(BN_backup),           0);
          gtk_widget_set_sensitive (GTK_WIDGET(FR_datos_restore),    1);
          gtk_widget_set_sensitive (GTK_WIDGET(FR_software_restore), 1);
	 }
     }
}

void Toggle_datos_restore(GtkWidget *widget, gpointer user_data)
{
  if (gtk_toggle_button_get_active(CK_datos_restore))
     {
      gtk_widget_set_sensitive (GTK_WIDGET(EN_datos_restore),    1);
      gtk_widget_set_sensitive (GTK_WIDGET(FC_datos_restore),    1);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_datos_restore),    1);
      if (N_files_datos) gtk_widget_set_sensitive (GTK_WIDGET(BN_datos_restore),    1);
      gtk_widget_set_sensitive (GTK_WIDGET(FR_backup),           0);
      gtk_widget_set_sensitive (GTK_WIDGET(FR_software_restore), 0);
     }
  else
     {
      gtk_widget_set_sensitive (GTK_WIDGET(EN_datos_restore), 0);
      gtk_widget_set_sensitive (GTK_WIDGET(FC_datos_restore), 0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_datos_restore), 0);
      if (!gtk_toggle_button_get_active(CK_software_restore))
	 {
          gtk_widget_set_sensitive (GTK_WIDGET(BN_datos_restore),    0);
          gtk_widget_set_sensitive (GTK_WIDGET(FR_backup),           1);
          gtk_widget_set_sensitive (GTK_WIDGET(FR_software_restore), 1);
	 }
     }
}

void Toggle_software_restore(GtkWidget *widget, gpointer user_data)
{
  if (gtk_toggle_button_get_active(CK_software_restore))
     {
      gtk_widget_set_sensitive (GTK_WIDGET(EN_software_restore), 1);
      gtk_widget_set_sensitive (GTK_WIDGET(FC_software_restore), 1);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_software_restore), 1);
      if (N_files_software) gtk_widget_set_sensitive (GTK_WIDGET(BN_software_restore), 1);
      gtk_widget_set_sensitive (GTK_WIDGET(FR_backup),           0);
      gtk_widget_set_sensitive (GTK_WIDGET(FR_datos_restore),    0);
     }
  else
     {
      gtk_widget_set_sensitive (GTK_WIDGET(EN_software_restore), 0);
      gtk_widget_set_sensitive (GTK_WIDGET(FC_software_restore), 0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_software_restore), 0);
      if (!gtk_toggle_button_get_active(CK_datos_restore))
	 {
          gtk_widget_set_sensitive (GTK_WIDGET(BN_software_restore), 0);
          gtk_widget_set_sensitive (GTK_WIDGET(FR_backup),           1);
          gtk_widget_set_sensitive (GTK_WIDGET(FR_datos_restore),    1);
	 }
     }
}

void Execute_Backup ()
{
   gchar * Directorio_datos;
   gchar * Directorio_software;
   int i;

   gtk_widget_set_sensitive(window1, 0);

   gtk_widget_show         (window7);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Directorio_datos    = gtk_file_chooser_get_filename(FC_datos_backup);
   Directorio_software = gtk_file_chooser_get_filename(FC_software_backup);

   if (gtk_toggle_button_get_active(CK_datos_backup))    Data_Backup     (Directorio_datos);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.40);
   while (gtk_events_pending ()) gtk_main_iteration ();
   if (gtk_toggle_button_get_active(CK_software_backup)) Software_Backup (Directorio_software);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.80);
   while (gtk_events_pending ()) gtk_main_iteration ();

   g_free (Directorio_datos);
   g_free (Directorio_software);

   Load_datos_restore    ();
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.90);
   while (gtk_events_pending ()) gtk_main_iteration ();
   Load_software_restore ();

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 1.00);
   while (gtk_events_pending ()) gtk_main_iteration ();

   gtk_widget_hide (window7);

   carga_parametros_EXAMINER (&parametros, DATABASE);
   if (parametros.report_update)
      {
       gtk_widget_show (window5);

       if (parametros.timeout)
	  {
           while (gtk_events_pending ()) gtk_main_iteration ();
           catNap(parametros.timeout);
           gtk_widget_hide (window5);
           gtk_widget_set_sensitive(window1, 1);
	   Init_Fields();
	  }
      }
   else
      gtk_widget_set_sensitive(window1, 1);
}

void Data_Backup(gchar * Directorio_datos)
{
   time_t curtime;
   struct tm *loctime;
   DIR * pdir;
   char Directorio_actual[1000];
   char Directorio[1000];
   char comando[2000];
   char PG_command [1000];
   PGresult * res;

   /* Get the current time.  */
   curtime = time (NULL);
   /* Convert it to local time representation.  */
   loctime = localtime (&curtime);

   sprintf (Directorio, "%s/EX.data.%4d.%02d.%02d.%02d.%02d.%02d",
           Directorio_datos,
           1900 + loctime->tm_year, loctime->tm_mon + 1, loctime->tm_mday,
	   loctime->tm_hour, loctime->tm_min, loctime->tm_sec);

   sprintf (PG_command,"UPDATE pa_parametros SET hilera_parametro=E'%2d/%2d/%4d' where codigo_parametro = 'DATEBUP'",loctime->tm_mday,loctime->tm_mon + 1,1900 + loctime->tm_year); 
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   pdir = opendir(Directorio);
   while (pdir)
         {
          closedir(pdir);

          /* Get the current time.  */
          curtime = time (NULL);
          /* Convert it to local time representation.  */
          loctime = localtime (&curtime);

          sprintf (Directorio, "%s/EX.data.%4d.%02d.%02d.%02d.%02d.%02d",
                   Directorio_datos,
                   1900 + loctime->tm_year, loctime->tm_mon + 1, loctime->tm_mday,
	           loctime->tm_hour, loctime->tm_min, loctime->tm_sec);

          pdir = opendir(Directorio);
         }

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.05);
   while (gtk_events_pending ()) gtk_main_iteration ();

   sprintf (comando, "mkdir '%s'", Directorio);
   system  (comando);

   getcwd(Directorio_actual, 1000);
   chdir(Directorio);

   if (parametros.usa_figuras)
      {
       system ("mkdir FIGURES");

       sprintf (comando, "cp '%s'/* FIGURES >/dev/null 2>&1", parametros.ruta_figuras);
       system  (comando);
       gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.10);
       while (gtk_events_pending ()) gtk_main_iteration ();

       system ("tar cfz EX.figures.tgz FIGURES");

       system ("rm -rf FIGURES");
       gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.20);
       while (gtk_events_pending ()) gtk_main_iteration ();
      }

   system ("pg_dump -O -c EXAMENES  -f EX.data.pg");

   chdir(Directorio_actual);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.35);
   while (gtk_events_pending ()) gtk_main_iteration ();
}

void Software_Backup(gchar * Directorio_software)
{
   time_t curtime;
   struct tm *loctime;
   DIR * pdir;
   char Directorio_actual[1000];
   char Directorio[1000];
   char comando[1000];

   /* Get the current time.  */
   curtime = time (NULL);
   /* Convert it to local time representation.  */
   loctime = localtime (&curtime);

   sprintf (Directorio, "%s/EX.soft.%4d.%02d.%02d.%02d.%02d.%02d",
           Directorio_software,
           1900 + loctime->tm_year, loctime->tm_mon + 1, loctime->tm_mday,
	   loctime->tm_hour, loctime->tm_min, loctime->tm_sec);

   pdir = opendir(Directorio);
   while (pdir)
         {
          closedir(pdir);

          /* Get the current time.  */
          curtime = time (NULL);
          /* Convert it to local time representation.  */
          loctime = localtime (&curtime);

          sprintf (Directorio, "%s/EX.soft.%4d.%02d.%02d.%02d.%02d.%02d",
                   Directorio_software,
                   1900 + loctime->tm_year, loctime->tm_mon + 1, loctime->tm_mday,
	           loctime->tm_hour, loctime->tm_min, loctime->tm_sec);

          pdir = opendir(Directorio);
         }
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.42);
   while (gtk_events_pending ()) gtk_main_iteration ();

   sprintf (comando, "mkdir '%s'", Directorio);
   system  (comando);
   sprintf (comando, "mkdir '%s/software'", Directorio);
   system  (comando);

   sprintf (comando, "cp EX '%s/software'", Directorio);
   system  (comando);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.55);
   while (gtk_events_pending ()) gtk_main_iteration ();

   sprintf (comando, "cp EXAMINER.cls '%s/software'", Directorio);
   system  (comando);
   sprintf (comando, "cp -r sources '%s/software/sources'", Directorio);
   system  (comando);
   sprintf (comando, "cp -r bin '%s/software/bin'", Directorio);
   system  (comando);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.65);
   while (gtk_events_pending ()) gtk_main_iteration ();
   sprintf (comando, "cp -r .interfaces '%s/software/.interfaces'", Directorio);
   system  (comando);
   sprintf (comando, "cp -r .imagenes '%s/software/.imagenes'", Directorio);
   system  (comando);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.7);
   while (gtk_events_pending ()) gtk_main_iteration ();

   getcwd(Directorio_actual, 1000);
   chdir(Directorio);

   system ("tar cfz EX.software.tgz software");
   system ("rm -rf software");
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_backup), 0.75);
   while (gtk_events_pending ()) gtk_main_iteration ();

   chdir(Directorio_actual);
}

void Software_Restore()
{
   int k;
   char path[4000];
   char comando[1000];
   time_t curtime;
   struct tm *loctime;
   DIR * pdir;
   char Directorio[1000];
   gchar * Directorio_software;

   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show         (window8);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 0.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Directorio_software = gtk_file_chooser_get_filename(FC_software_restore);

   /* Get the current time.  */
   curtime = time (NULL);
   /* Convert it to local time representation.  */
   loctime = localtime (&curtime);

   sprintf (Directorio, "TMP-RESTORE-SOFT.%4d.%02d.%02d.%02d.%02d.%02d",
           1900 + loctime->tm_year, loctime->tm_mon + 1, loctime->tm_mday,
	   loctime->tm_hour, loctime->tm_min, loctime->tm_sec);

   pdir = opendir(Directorio);
   while (pdir)
         {
          closedir(pdir);

          /* Get the current time.  */
          curtime = time (NULL);
          /* Convert it to local time representation.  */
          loctime = localtime (&curtime);

          sprintf (Directorio, "TMP-RESTORE-SOFT.%4d.%02d.%02d.%02d.%02d.%02d",
                   1900 + loctime->tm_year, loctime->tm_mon + 1, loctime->tm_mday,
	           loctime->tm_hour, loctime->tm_min, loctime->tm_sec);

          pdir = opendir(Directorio);
         }
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 0.05);
   while (gtk_events_pending ()) gtk_main_iteration ();

   sprintf (comando, "mkdir '%s'", Directorio);
   system  (comando);

   k = gtk_combo_box_get_active(CB_software_restore);
   strcpy (path, Directorio_software);
   strcat (path,"/");
   strcat (path, software_restore[k]);
   strcat (path,"/");
   strcat (path, "EX.software.tgz");

   if (existe_archivo(path))
      {
       sprintf (comando, "cp '%s' '%s'", path, Directorio);
       system  (comando);
       chdir   (Directorio);
       system  ("tar xfz EX.software.tgz");
       gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 0.15);
       while (gtk_events_pending ()) gtk_main_iteration ();

       system  ("rm -f ../EX");
       system  ("cp software/EX ..");
       system  ("cp software/EXAMINER.cls ..");
       system  ("rm -r ../bin");
       system  ("mkdir ../bin");
       system  ("cp software/bin/* ../bin");
       gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 0.45);
       while (gtk_events_pending ()) gtk_main_iteration ();

       system  ("rm -r ../.imagenes");
       system  ("mkdir ../.imagenes");
       system  ("cp software/.imagenes/* ../.imagenes");
       gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 0.75);
       while (gtk_events_pending ()) gtk_main_iteration ();

       system  ("rm -r ../.interfaces");
       system  ("mkdir ../.interfaces");
       system  ("cp software/.interfaces/* ../.interfaces");
       system  ("rm -r ../sources");
       system  ("mkdir ../sources");
       system  ("cp software/sources/* ../sources");
       chdir   ("..");
       gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 0.95);
       while (gtk_events_pending ()) gtk_main_iteration ();
      }
   else
      {
       gtk_widget_hide (window8);
       gtk_widget_set_sensitive (window1, 0);
       gtk_widget_show          (window4);
      }

   sprintf (comando, "rm -r '%s'", Directorio);
   system  (comando);

   g_free (Directorio_software);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 1.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   gtk_widget_hide (window8);

   carga_parametros_EXAMINER (&parametros, DATABASE);
   if (parametros.report_update)
      {
       gtk_widget_show (window6);

       if (parametros.timeout)
	  {
           while (gtk_events_pending ()) gtk_main_iteration ();
           catNap(parametros.timeout);
           gtk_widget_hide (window6);
           gtk_widget_set_sensitive(window1, 1);
	   Init_Fields();
	  }
      }
   else
      gtk_widget_set_sensitive(window1, 1);
}

void Datos_Restore()
{
   int k;
   char path_figures [4000];
   char path_datos   [4000];
   char comando[1000];
   time_t curtime;
   struct tm *loctime;
   DIR * pdir;
   char Directorio[1000];
   gchar * Directorio_datos;
   char Directorio_actual[1000];

   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show         (window8);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 0.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Directorio_datos = gtk_file_chooser_get_filename(FC_datos_restore);

   /* Get the current time.  */
   curtime = time (NULL);
   /* Convert it to local time representation.  */
   loctime = localtime (&curtime);

   sprintf (Directorio, "TMP-RESTORE-DATA.%4d.%02d.%02d.%02d.%02d.%02d",
           1900 + loctime->tm_year, loctime->tm_mon + 1, loctime->tm_mday,
	   loctime->tm_hour, loctime->tm_min, loctime->tm_sec);

   pdir = opendir(Directorio);
   while (pdir)
         {
          closedir(pdir);

          /* Get the current time.  */
          curtime = time (NULL);
          /* Convert it to local time representation.  */
          loctime = localtime (&curtime);

          sprintf (Directorio, "TMP-RESTORE-DATA.%4d.%02d.%02d.%02d.%02d.%02d",
                   1900 + loctime->tm_year, loctime->tm_mon + 1, loctime->tm_mday,
	           loctime->tm_hour, loctime->tm_min, loctime->tm_sec);

          pdir = opendir(Directorio);
         }
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 0.05);
   while (gtk_events_pending ()) gtk_main_iteration ();

   sprintf (comando, "mkdir '%s'", Directorio);
   system  (comando);

   k = gtk_combo_box_get_active(CB_datos_restore);

   strcpy (path_datos, Directorio_datos);
   strcat (path_datos,"/");
   strcat (path_datos, datos_restore[k]);
   strcat (path_datos,"/");
   strcat (path_datos, "EX.data.pg");

   strcpy (path_figures, Directorio_datos);
   strcat (path_figures,"/");
   strcat (path_figures, datos_restore[k]);
   strcat (path_figures,"/");
   strcat (path_figures, "EX.figures.tgz");

   if (existe_archivo(path_datos))
      {
       chdir   (Directorio);

       if (parametros.usa_figuras && (existe_archivo(path_figures)))
          {
	   sprintf (comando, "cp '%s' .", path_figures);
           system  (comando);

           system  ("tar xfz EX.figures.tgz");
           gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 0.25);
           while (gtk_events_pending ()) gtk_main_iteration ();

           pdir = opendir("FIGURES");
           if (pdir)
              {
               closedir(pdir);

	       sprintf (comando, "cp FIGURES/* '%s' >/dev/null 2>&1", parametros.ruta_figuras);
               system  (comando);
	      }
           gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 0.55);
           while (gtk_events_pending ()) gtk_main_iteration ();
	  }

       sprintf (comando, "cp '%s' .", path_datos);
       system  (comando);

       system  ("psql -d EXAMENES <EX.data.pg >/dev/null 2>&1");
       gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 0.95);
       while (gtk_events_pending ()) gtk_main_iteration ();

       chdir ("..");
      }
   else
      {
       gtk_widget_hide (window8);
       gtk_widget_set_sensitive (window1, 0);
       gtk_widget_show          (window4);
      }

   sprintf (comando, "rm -r '%s'", Directorio);
   system  (comando);

   g_free (Directorio_datos);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_restore), 1.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   gtk_widget_hide (window8);

   carga_parametros_EXAMINER (&parametros, DATABASE);
   if (parametros.report_update)
      {
       gtk_widget_show (window6);

       if (parametros.timeout)
	  {
           while (gtk_events_pending ()) gtk_main_iteration ();
           catNap(parametros.timeout);
           gtk_widget_hide (window6);
           gtk_widget_set_sensitive(window1, 1);
	   Init_Fields();
	  }
      }
   else
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

int Filter_file_name (char *filename, char * type, char * Directorio)
{
  int result;
  char path[2000];
  struct stat entrystat;

  strcpy (path, Directorio);
  strcat (path, "/");
  strcat (path, filename);
  stat(path, &entrystat);
  /*           1         2       */
  /* 012345678901234567890123456 */
  /* EX.data.2013.06.20.17.55.13 */
  result = 0;
  if ((S_ISDIR(entrystat.st_mode))        &&
      (strncmp(filename  ,"EX.",3) == 0)  &&
      (strncmp(filename+3, type,5) == 0)  &&
      (*(filename+8)  >= '0')             &&
      (*(filename+8)  <= '9')             &&
      (*(filename+9)  >= '0')             &&
      (*(filename+9)  <= '9')             &&
      (*(filename+10) >= '0')             &&
      (*(filename+10) <= '9')             &&
      (*(filename+11) >= '0')             &&
      (*(filename+11) <= '9')             &&
      (*(filename+12) == '.')             &&
      (*(filename+13) >= '0')             &&
      (*(filename+13) <= '1')             &&
      (*(filename+14) >= '0')             &&
      (*(filename+14) <= '9')             &&
      (atoi(filename+13) <= 12)           &&
      (*(filename+15) == '.')             &&
      (*(filename+16) >= '0')             &&
      (*(filename+16) <= '3')             &&
      (*(filename+17) >= '0')             &&
      (*(filename+17) <= '9')             &&
      (atoi(filename+16) <= 31)           &&
      (*(filename+18) == '.')             &&
      (*(filename+19) >= '0')             &&
      (*(filename+19) <= '2')             &&
      (*(filename+20) >= '0')             &&
      (*(filename+20) <= '9')             &&
      (atoi(filename+19) <= 23)           &&
      (*(filename+21) == '.')             &&
      (*(filename+22) >= '0')             &&
      (*(filename+22) <= '5')             &&
      (*(filename+23) >= '0')             &&
      (*(filename+23) <= '9')             &&
      (atoi(filename+22) <= 59)           &&
      (*(filename+24) == '.')             &&
      (*(filename+25) >= '0')             &&
      (*(filename+25) <= '5')             &&
      (*(filename+26) >= '0')             &&
      (*(filename+26) <= '9')             &&
      (atoi(filename+25) <= 59)           &&
      (*(filename+27) == '\0'))
     result = 1;

  return(result);
}

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex1010_destroy (GtkWidget *widget, gpointer user_data) 
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
  gtk_file_chooser_set_current_folder (FC_datos_backup, parametros.ruta_backup);
  gtk_file_chooser_set_current_folder (FC_software_backup, parametros.ruta_backup);
  gtk_file_chooser_set_current_folder (FC_datos_restore, parametros.ruta_backup);
  gtk_file_chooser_set_current_folder (FC_software_restore, parametros.ruta_backup);
  Init_Fields ();
}

void on_BN_backup_clicked(GtkWidget *widget, gpointer user_data)
{
  Execute_Backup ();
}

void on_BN_datos_restore_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * fecha;
  char hilera[800];

  fecha = gtk_combo_box_get_active_text(CB_datos_restore);
  sprintf(hilera, "¿Desea restaurar la Base de Datos completa al estado del\n<b>%s</b>?", fecha);
  gtk_label_set_markup(LB_datos_mensaje, hilera);
  gtk_widget_set_sensitive (window1, 0);
  gtk_widget_show          (window2);

  g_free(fecha);
}

void on_BN_software_restore_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * fecha;
  char hilera[800];

  fecha = gtk_combo_box_get_active_text(CB_software_restore);
  sprintf(hilera, "¿Desea restaurar el software al estado del\n<b>%s</b>?", fecha);
  gtk_label_set_markup(LB_software_mensaje, hilera);
  gtk_widget_set_sensitive (window1, 0);
  gtk_widget_show          (window3);

  g_free(fecha);
}

void on_CK_datos_backup_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_datos_backup (widget, user_data);
}

void on_CK_software_backup_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_software_backup (widget, user_data);
}

void on_CK_datos_restore_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_datos_restore (widget, user_data);
}

void on_CK_software_restore_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_software_restore (widget, user_data);
}

void on_FC_datos_restore_current_folder_changed (GtkWidget *widget, gpointer user_data)
{
  gchar * datos_restore;

  datos_restore = gtk_file_chooser_get_filename(FC_datos_restore);
  gtk_entry_set_text  (GTK_ENTRY(EN_datos_restore), datos_restore);
  g_free (datos_restore);

  Load_datos_restore();
  if (gtk_toggle_button_get_active(CK_datos_restore))
     {
      if (N_files_datos) 
         gtk_widget_set_sensitive (GTK_WIDGET(BN_datos_restore), 1);
      else
         gtk_widget_set_sensitive (GTK_WIDGET(BN_datos_restore), 0);
     }
}

void on_FC_software_restore_current_folder_changed (GtkWidget *widget, gpointer user_data)
{
  gchar * software_restore;

  software_restore = gtk_file_chooser_get_filename(FC_software_restore);
  gtk_entry_set_text  (GTK_ENTRY(EN_software_restore), software_restore);
  g_free (software_restore);

  Load_software_restore();
  if (gtk_toggle_button_get_active(CK_software_restore))
     {
      if (N_files_software) 
         gtk_widget_set_sensitive (GTK_WIDGET(BN_software_restore), 1);
      else
         gtk_widget_set_sensitive (GTK_WIDGET(BN_software_restore), 0);
     }
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window4);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_backup_ready_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window5);
  gtk_widget_set_sensitive(window1, 1);

  Init_Fields           ();
}

void on_BN_restore_ready_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window6);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_cancel_software_restore_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window3);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (BN_undo);
}

void on_BN_cancel_datos_restore_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (BN_undo);
}

void on_BN_confirm_software_restore_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window3);
  Software_Restore ();

  Load_datos_restore    ();
  Load_software_restore ();
  Init_Fields           ();
}

void on_BN_confirm_datos_restore_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  Datos_Restore ();

  Load_datos_restore    ();
  Load_software_restore ();
  Init_Fields           ();
}

void on_FC_datos_backup_current_folder_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * datos_backup;

  datos_backup = gtk_file_chooser_get_filename(FC_datos_backup);
  gtk_entry_set_text  (GTK_ENTRY(EN_datos_backup), datos_backup);
  g_free (datos_backup);
}

void on_FC_software_backup_current_folder_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * software_backup;

  software_backup = gtk_file_chooser_get_filename(FC_software_backup);
  gtk_entry_set_text  (GTK_ENTRY(EN_software_backup), software_backup);
  g_free (software_backup);
}
