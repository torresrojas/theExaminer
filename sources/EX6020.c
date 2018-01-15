/*
Compilar con

cc -o EX6020 EX6020.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic

*/
/*******************************************/
/*  EX6020:                                */
/*                                         */
/*    Elimina Esquemas Viejos              */
/*                                         */
/*    The Examiner 0.4                     */
/*    20 de Junio 2014                     */
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

struct LINEA_ESQUEMA
{
  char tema    [CODIGO_TEMA_SIZE    + 1];
  char subtema [CODIGO_SUBTEMA_SIZE + 1];
  int  cantidad;
  int  maximo;
  int  inicio;
};

int Lineas_Esquema;
int Preguntas_Pre_Examen;
int Total_Preguntas;
long double Total_Examenes;

struct LINEA_ESQUEMA * Esquema    = NULL;

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

GtkWidget *FR_hay_preexamenes     = NULL;
GtkWidget *BN_hay_preexamenes     = NULL;

GtkToggleButton *CK_activar_borrado   = NULL;

GtkSpinButton *SP_esquema         = NULL;
GtkWidget *EN_esquema_descripcion = NULL;
GtkWidget *EN_materia             = NULL;
GtkWidget *EN_materia_descripcion = NULL;
GtkWidget *EN_preguntas           = NULL;
GtkWidget *EN_examenes            = NULL;

GtkWidget *FR_botones                = NULL;

GtkWidget *BN_delete              = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_terminar            = NULL;
GtkWidget *EB_ready              = NULL;
GtkWidget *FR_ready              = NULL;

GtkWidget *BN_ok                  = NULL;

GtkWidget *FR_verifica_borrado = NULL;
GtkWidget *BN_confirma_borrado = NULL;
GtkWidget *BN_cancela_borrado  = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Activar_Campos                 ();
void Borra_Datos                    ();
void Calcular_Total_Examenes        ();
void Cambio_Esquema                 (GtkWidget *widget, gpointer user_data);
void Carga_Esquema                  (PGresult *res);
long double combinaciones           (int k, int n);
void Fin_de_Programa                (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                    (GtkWidget *widget, gpointer user_data);
void Init_Fields                    ();
int main                            (int argc, char *argv[]);
void on_BN_cancela_borrado_clicked  (GtkWidget *widget, gpointer user_data);
void on_BN_confirma_borrado_clicked (GtkWidget *widget, gpointer user_data);
void on_BN_delete_clicked           (GtkWidget *widget, gpointer user_data);
void on_BN_hay_preexamenes_clicked  (GtkWidget *widget, gpointer user_data);
void on_BN_ok_clicked               (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked         (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked             (GtkWidget *widget, gpointer user_data);
void on_CK_activar_borrado_toggled  (GtkWidget *widget, gpointer user_data);
void on_SP_esquema_activate         (GtkWidget *widget, gpointer user_data);
void on_WN_ex6020_destroy           (GtkWidget *widget, gpointer user_data);

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
  GdkColor color;

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX6020.glade", &error))
         {
          g_warning ("%s\n", error->message);
          g_error_free (error);
         }
      else
	 {
	  /* Conecta funciones con eventos de la interfaz */
          gtk_builder_connect_signals (builder, NULL);

	  /* Crea conexión con los campos de la interfaz a ser controlados */
          window1              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex6020"));
          window2              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_hay_preexamenes"));
          window3                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_verifica_borrado"));
          window4                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ready"));

          FR_hay_preexamenes   = GTK_WIDGET (gtk_builder_get_object (builder, "FR_hay_preexamenes"));
          BN_hay_preexamenes   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_hay_preexamenes"));

          SP_esquema             = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_esquema"));
          EN_esquema_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema_descripcion"));
          EN_materia             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia"));
          EN_materia_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia_descripcion"));
          EN_preguntas           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_preguntas"));
          EN_examenes            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_examenes"));
          CK_activar_borrado        = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_activar_borrado"));
          FR_verifica_borrado = GTK_WIDGET (gtk_builder_get_object (builder, "FR_verifica_borrado"));
          BN_confirma_borrado = GTK_WIDGET (gtk_builder_get_object (builder, "BN_confirma_borrado"));
          BN_cancela_borrado  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_cancela_borrado"));

          FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));

          BN_delete              = GTK_WIDGET (gtk_builder_get_object (builder, "BN_delete"));
          BN_undo                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
          BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));

          EN_examenes            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_examenes"));
	  EB_ready              = GTK_WIDGET (gtk_builder_get_object (builder, "EB_ready"));
	  FR_ready              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_ready"));
          BN_ok    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));

	  /* Muchos campos son READ ONLY */
          gtk_widget_set_can_focus(EN_esquema_descripcion, FALSE);
          gtk_widget_set_can_focus(EN_materia,             FALSE);
          gtk_widget_set_can_focus(EN_materia_descripcion, FALSE);
          gtk_widget_set_can_focus(EN_preguntas,           FALSE);
          gtk_widget_set_can_focus(EN_examenes,            FALSE);

	  /* Un poco de color */
          gdk_color_parse ("pale goldenrod", &color);
          gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

          gdk_color_parse ("red", &color);
          gtk_widget_modify_bg(BN_cancela_borrado, GTK_STATE_PRELIGHT,&color);
          gdk_color_parse ("dark red", &color);
          gtk_widget_modify_bg(BN_cancela_borrado,   GTK_STATE_ACTIVE, &color);

          gdk_color_parse ("dark goldenrod", &color);
          gtk_widget_modify_bg(GTK_WIDGET(SP_esquema), GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(FR_botones,  GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(BN_terminar, GTK_STATE_ACTIVE, &color);
          gtk_widget_modify_bg(BN_delete,   GTK_STATE_ACTIVE, &color);
          gtk_widget_modify_bg(BN_undo,     GTK_STATE_ACTIVE, &color);

          gdk_color_parse ("light goldenrod", &color);
          gtk_widget_modify_bg(BN_terminar, GTK_STATE_PRELIGHT, &color);
          gtk_widget_modify_bg(BN_delete,   GTK_STATE_PRELIGHT, &color);
          gtk_widget_modify_bg(BN_undo,     GTK_STATE_PRELIGHT, &color);

          gdk_color_parse ("dark green", &color);
          gtk_widget_modify_bg(window2,            GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(BN_hay_preexamenes, GTK_STATE_ACTIVE, &color);
          gtk_widget_modify_bg(EB_ready,          GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(BN_confirma_borrado, GTK_STATE_ACTIVE, &color);
          gtk_widget_modify_bg(BN_ok,           GTK_STATE_ACTIVE, &color);

          gdk_color_parse ("green", &color);
          gtk_widget_modify_bg(FR_hay_preexamenes,  GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(BN_hay_preexamenes,  GTK_STATE_PRELIGHT, &color);
          gtk_widget_modify_bg(BN_confirma_borrado, GTK_STATE_PRELIGHT, &color);
          gtk_widget_modify_bg(FR_ready,           GTK_STATE_NORMAL, &color);
          gtk_widget_modify_bg(BN_ok,               GTK_STATE_PRELIGHT, &color);

          gdk_color_parse ("black", &color);
          gtk_widget_modify_bg(FR_verifica_borrado, GTK_STATE_NORMAL, &color);

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
   int  Last_esquema;
   char esquema[6];

   /* Recupera el esquema con código más alto */

   res = PQEXEC(DATABASE, "SELECT codigo_esquema from EX_esquemas order by codigo_esquema DESC limit 1");

   Last_esquema = 0;
   if (PQntuples(res))
      {
       Last_esquema = atoi (PQgetvalue (res, 0, 0));
      }
   PQclear(res);

   sprintf (esquema,"%05d",Last_esquema);

   if (Last_esquema > 1)
      {
       gtk_widget_set_sensitive(GTK_WIDGET(SP_esquema), 1);
       gtk_spin_button_set_range (SP_esquema, 1.0, (long double) Last_esquema);
      }
   else
      { /* Solo hay un esquema */
       gtk_widget_set_sensitive(GTK_WIDGET(SP_esquema), 0);
       gtk_spin_button_set_range (SP_esquema, 0.0, (long double) Last_esquema);
      }
   gtk_spin_button_set_value (SP_esquema, Last_esquema);

   gtk_entry_set_text(GTK_ENTRY(EN_materia),             "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion), "\0");

   gtk_entry_set_text(GTK_ENTRY(EN_preguntas      ), "0");
   gtk_entry_set_text(GTK_ENTRY(EN_examenes       ), "0");

   gtk_widget_set_sensitive(GTK_WIDGET(SP_esquema), 1);
   gtk_widget_set_sensitive(EN_esquema_descripcion, 0);
   gtk_widget_set_sensitive(EN_materia,             0);
   gtk_widget_set_sensitive(EN_materia_descripcion, 0);

   gtk_widget_set_sensitive(EN_preguntas,           0);
   gtk_widget_set_sensitive(EN_examenes,            0);

   gtk_widget_set_sensitive(BN_delete,  0);
   gtk_widget_set_sensitive(BN_undo,    1);
   gtk_toggle_button_set_active(CK_activar_borrado, FALSE);
   gtk_widget_set_sensitive(GTK_WIDGET(CK_activar_borrado), 0);

   if (Esquema) free (Esquema);
   Esquema      = NULL;

   Preguntas_Pre_Examen = 0;
   Total_Preguntas    = 0;
}

void Cambio_Esquema(GtkWidget *widget, gpointer user_data)
{
  long int k;
  char esquema [10];
  char PG_command [1000];
  PGresult *res, *res_aux;

  k = (long int) gtk_spin_button_get_value_as_int (SP_esquema);
  sprintf (esquema, "%05ld", k);
  sprintf (PG_command,"SELECT * from EX_esquemas where codigo_esquema = '%.5s'", esquema);
  res = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res))
     {/* Esquema existe */
      sprintf (PG_command,"SELECT * from EX_pre_examenes where esquema = '%.5s'", esquema);
      res_aux = PQEXEC(DATABASE, PG_command);
      if (PQntuples(res_aux))
	 { /* Aún hay preexámenes derivados de esquema - no se puede borrar */
          gtk_widget_set_sensitive(window1, 0);
          gtk_widget_show         (window2);
          gtk_widget_grab_focus   (GTK_WIDGET(BN_hay_preexamenes));
         }
      else
	 {
          Carga_Esquema (res);
	  Activar_Campos ();
          gtk_widget_set_sensitive(GTK_WIDGET(CK_activar_borrado), 1);
          gtk_widget_grab_focus   (GTK_WIDGET(BN_undo));
	 }
     }
  else
     {
      gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion),"**** Esquema no está registrado ****"); 
      gtk_widget_grab_focus    (GTK_WIDGET(SP_esquema));
     }

  PQclear(res);
}

void Activar_Campos ()
{
  gtk_widget_set_sensitive(GTK_WIDGET(SP_esquema), 0);
  gtk_widget_set_sensitive(EN_esquema_descripcion, 1);
  gtk_widget_set_sensitive(EN_materia,             1);
  gtk_widget_set_sensitive(EN_materia_descripcion, 1);
  gtk_widget_set_sensitive(EN_preguntas,           1);
  gtk_widget_set_sensitive(EN_examenes,            1);
}

void Carga_Esquema (PGresult *res)
{
  PGresult *res_lineas_esquema, *res_materia, *res_preguntas;
  char PG_command [2000];
  int i, maximo;
  char hilera[50];
  long double millon        =1000000L;
  long double millardo      =1000000000L;
  long double billon        =1000000000000L;
  long double trillon       =1000000000000000000L;
  long double millontrillon =1e24L;

  gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion), PQgetvalue(res, 0, 5));
  gtk_entry_set_text(GTK_ENTRY(EN_materia),             PQgetvalue(res, 0, 1));
  sprintf (PG_command,"SELECT * from bd_materias where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", PQgetvalue(res, 0, 1));
  res_materia = PQEXEC(DATABASE, PG_command);
  gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), PQgetvalue(res_materia, 0, 3));
  PQclear(res_materia);

  sprintf (PG_command,"SELECT * from EX_esquemas_lineas where esquema = '%s' order by tema,subtema", PQgetvalue(res, 0, 0));
  res_lineas_esquema = PQEXEC(DATABASE, PG_command);

  Esquema = NULL;
  if (PQntuples(res_lineas_esquema))
     {
      Esquema = (struct LINEA_ESQUEMA *) malloc(sizeof(struct LINEA_ESQUEMA) * PQntuples(res_lineas_esquema));
     }

  Total_Preguntas    = 0;
  Preguntas_Pre_Examen = 0;
  Lineas_Esquema      = 0;
  for (i=0;i<PQntuples(res_lineas_esquema);i++)
      {
       sprintf (PG_command,"SELECT * from bd_texto_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and materia = '%s' and tema = '%s' and automatico",
                           PQgetvalue(res,0,1), PQgetvalue(res_lineas_esquema, i, 1));
       res_preguntas = PQEXEC(DATABASE, PG_command);
       maximo = PQntuples(res_preguntas);
       PQclear(res_preguntas);

       if (maximo)
	  {
           if (strcmp( PQgetvalue(res_lineas_esquema, i, 2),CODIGO_VACIO) == 0)
	      { /* NO se usa subtema y sabemos que hay preguntas del tema */
               strcpy (Esquema[Lineas_Esquema].tema,    PQgetvalue(res_lineas_esquema, i, 1));
               strcpy (Esquema[Lineas_Esquema].subtema, CODIGO_VACIO);

               Esquema[Lineas_Esquema].cantidad  = MINIMO(atof(PQgetvalue(res_lineas_esquema, i, 3)),maximo);
               Esquema[Lineas_Esquema].maximo    = maximo;
               Esquema[Lineas_Esquema].inicio    = Total_Preguntas;

	       Total_Preguntas    += Esquema[Lineas_Esquema].maximo;
	       Preguntas_Pre_Examen += Esquema[Lineas_Esquema].cantidad;
	       Lineas_Esquema++;
	      }
	   else
	     { /* Se usa subtema */
 	       sprintf (PG_command,"SELECT * from bd_texto_preguntas, bd_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and materia = '%s' and tema = '%s' and subtema = '%s' and automatico",
                        PQgetvalue(res,0,1), PQgetvalue(res_lineas_esquema, i, 1),PQgetvalue(res_lineas_esquema, i, 2));
               res_preguntas = PQEXEC(DATABASE, PG_command);
               maximo = PQntuples(res_preguntas);
               PQclear(res_preguntas);
	       if (maximo)
		  { /* Hay preguntas */
                   strcpy (Esquema[Lineas_Esquema].tema,    PQgetvalue(res_lineas_esquema, i, 1));
                   strcpy (Esquema[Lineas_Esquema].subtema, PQgetvalue(res_lineas_esquema, i, 2));

                   Esquema[Lineas_Esquema].cantidad = MINIMO(atof(PQgetvalue(res_lineas_esquema, i, 3)),maximo);
                   Esquema[Lineas_Esquema].maximo   = maximo;
                   Esquema[Lineas_Esquema].inicio   = Total_Preguntas;

  	           Total_Preguntas    += Esquema[Lineas_Esquema].maximo;
  	           Preguntas_Pre_Examen += Esquema[Lineas_Esquema].cantidad;
	           Lineas_Esquema++;
		  }
	      }
	  }
      }

  if (Preguntas_Pre_Examen)
     {
      Calcular_Total_Examenes ();
     }

  sprintf (hilera,"%10d",Preguntas_Pre_Examen);
  gtk_entry_set_text(GTK_ENTRY(EN_preguntas), hilera);

  if (Total_Examenes < millon)
     sprintf (hilera,"%Lf",Total_Examenes);
  else
     if (Total_Examenes < millardo)
        sprintf (hilera,"%.2Lf millones ",Total_Examenes/1000000.0);
     else
        if (Total_Examenes < billon)
	   sprintf (hilera,"%.2Lf millardos ",Total_Examenes/1000000000.0);
	else
           if (Total_Examenes < trillon)
  	      sprintf (hilera,"%.2Lf billones ",Total_Examenes/1000000000000.0);
	   else
	      if (Total_Examenes < millontrillon)
	         sprintf (hilera,"%.2Lf trillones ",Total_Examenes/1000000000000000000.0);
              else
	         sprintf (hilera,"%1.9Le",Total_Examenes);

  gtk_entry_set_text(GTK_ENTRY(EN_examenes), hilera);

  PQclear(res_lineas_esquema);
}

void Calcular_Total_Examenes ()
{
  long int    i;
  long double k;

  Total_Examenes = 1.0;
  for (i=0;i<Lineas_Esquema;i++)
      {
       k = combinaciones (Esquema[i].cantidad, Esquema[i].maximo);
       Total_Examenes *= k;
      }
}

long double combinaciones (int k, int n)
{
  long double i;
  long double kfactorial = 1;
  long double N=1;

  for (i=1.0    ; i<=k; i++) kfactorial *= i;
  for (i=n-k+1  ; i<=n; i++) N          *= i;
  return (N/kfactorial);
}

void Fin_de_Programa (GtkWidget *widget, gpointer user_data)
{
   /* close the connection to the database and cleanup */
   PQfinish(DATABASE);
   gtk_main_quit ();
   exit (0);
}

void Borra_Datos ()
{
  int k;
  PGresult *res;
  char PG_command [4000], esquema[6];

  k = (int) gtk_spin_button_get_value_as_int (SP_esquema);
  sprintf (esquema, "%05d", k);

  res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

  sprintf (PG_command, "DELETE FROM EX_esquemas where codigo_esquema = '%s'", esquema);
  res = PQEXEC(DATABASE, PG_command);
  PQclear(res);
  /* Por ON DELETE CASCADE se borran también EX_esquemas_lineas */

  res = PQEXEC(DATABASE, "END"); PQclear(res);

  Init_Fields ();

  gtk_widget_set_sensitive(window1, 0);
  gtk_widget_show (window4);
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
void on_WN_ex6020_destroy (GtkWidget *widget, gpointer user_data) 
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
  gtk_widget_set_sensitive (window1, 0);
  gtk_widget_show          (window3);
  gtk_widget_grab_focus    (BN_cancela_borrado);
}

void on_SP_esquema_activate(GtkWidget *widget, gpointer user_data)
{
  long int k;
  gchar * esquema;

  esquema = gtk_editable_get_chars(GTK_EDITABLE(SP_esquema), 0, -1);
  k = atoi(esquema);
  g_free(esquema);

  gtk_spin_button_set_value (SP_esquema, k);
  Cambio_Esquema (widget, user_data);
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window4);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus    (GTK_WIDGET(SP_esquema));
}

void on_BN_hay_preexamenes_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  Init_Fields ();
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (GTK_WIDGET(SP_esquema));
}

void on_CK_activar_borrado_toggled (GtkWidget *widget, gpointer user_data)
{
  if (gtk_toggle_button_get_active(CK_activar_borrado))
     gtk_widget_set_sensitive(BN_delete,  1);
  else
     gtk_widget_set_sensitive(BN_delete,  0);
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
