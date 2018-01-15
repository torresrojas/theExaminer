/*
Compilar con

cc -o EX3020 EX3020.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic

*/
/*******************************************/
/*  EX3020:                                */
/*                                         */
/*    Genera un Pre-examen a partir        */
/*    de un Esquema predefinido.           */
/*                                         */
/*    The Examiner 0.0                     */
/*    24 de Julio 2011                     */
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Se cambia distribución de campos     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    12 de Febrero del 2012               */
/*-----------------------------------------*/
/*    Se incluye manejo de parámetros de   */
/*    sorteo de preguntas, se leen las     */
/*    ecuaciones lineales asociadas a      */
/*    dificultad, discriminacion, novedad  */
/*    y usos de cada pregunta.             */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    22 de Setiembre del 2012             */
/*******************************************/
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
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
GError    * error;

GtkWidget *window1                = NULL;
GtkWidget *window2                = NULL;
GtkWidget *window3                = NULL;
GtkWidget *window4                = NULL;
GtkWidget *FR_preexamenusado      = NULL;

GtkSpinButton *SP_codigo          = NULL;
GtkSpinButton *SP_esquema         = NULL;
GtkWidget *EN_descripcion         = NULL;
GtkWidget *EN_esquema_descripcion = NULL;
GtkWidget *EN_materia             = NULL;
GtkWidget *EN_materia_descripcion = NULL;
GtkWidget *EN_preguntas           = NULL;
GtkWidget *EN_examenes            = NULL;

GtkWidget *EB_A      = NULL;
GtkWidget *FR_A      = NULL;
GtkWidget *FR_dif_A  = NULL;
GtkWidget *FR_disc_A = NULL;
GtkWidget *FR_Cron_A = NULL;
GtkWidget *FR_dias_A = NULL;
GtkWidget *DA_dif_A  = NULL;
GtkWidget *DA_disc_A = NULL;
GtkWidget *DA_Cron_A = NULL;
GtkWidget *DA_dias_A = NULL;

GtkWidget *EB_B      = NULL;
GtkWidget *FR_B      = NULL;
GtkWidget *FR_dif_B  = NULL;
GtkWidget *FR_disc_B = NULL;
GtkWidget *FR_Cron_B = NULL;
GtkWidget *FR_dias_B = NULL;
GtkWidget *DA_dif_B  = NULL;
GtkWidget *DA_disc_B = NULL;
GtkWidget *DA_Cron_B = NULL;
GtkWidget *DA_dias_B = NULL;

GtkWidget *EB_C      = NULL;
GtkWidget *FR_C      = NULL;
GtkWidget *FR_dif_C  = NULL;
GtkWidget *FR_disc_C = NULL;
GtkWidget *FR_Cron_C = NULL;
GtkWidget *FR_dias_C = NULL;
GtkWidget *DA_dif_C  = NULL;
GtkWidget *DA_disc_C = NULL;
GtkWidget *DA_Cron_C = NULL;
GtkWidget *DA_dias_C = NULL;

GtkWidget *EB_D      = NULL;
GtkWidget *FR_D      = NULL;
GtkWidget *FR_dif_D  = NULL;
GtkWidget *FR_disc_D = NULL;
GtkWidget *FR_Cron_D = NULL;
GtkWidget *FR_dias_D = NULL;
GtkWidget *DA_dif_D  = NULL;
GtkWidget *DA_disc_D = NULL;
GtkWidget *DA_Cron_D = NULL;
GtkWidget *DA_dias_D = NULL;

GtkImage  *IM_A                      = NULL;
GtkImage  *IM_B                      = NULL;
GtkImage  *IM_C                      = NULL;
GtkImage  *IM_D                      = NULL;

GtkWidget *FR_botones                = NULL;
GtkWidget *FR_botones_A              = NULL;
GtkWidget *FR_botones_B              = NULL;
GtkWidget *FR_botones_C              = NULL;
GtkWidget *FR_botones_D              = NULL;

GtkWidget *EN_estudiantes_A       = NULL;
GtkWidget *EN_estudiantes_B       = NULL;
GtkWidget *EN_estudiantes_C       = NULL;
GtkWidget *EN_estudiantes_D       = NULL;
GtkWidget *EN_media_A             = NULL;
GtkWidget *EN_media_B             = NULL;
GtkWidget *EN_media_C             = NULL;
GtkWidget *EN_media_D             = NULL;
GtkWidget *EN_desviacion_A        = NULL;
GtkWidget *EN_desviacion_B        = NULL;
GtkWidget *EN_desviacion_C        = NULL;
GtkWidget *EN_desviacion_D        = NULL;
GtkWidget *EN_alfa_A              = NULL;
GtkWidget *EN_alfa_B              = NULL;
GtkWidget *EN_alfa_C              = NULL;
GtkWidget *EN_alfa_D              = NULL;
GtkWidget *EN_Rpb_A               = NULL;
GtkWidget *EN_Rpb_B               = NULL;
GtkWidget *EN_Rpb_C               = NULL;
GtkWidget *EN_Rpb_D               = NULL;
GtkWidget *BN_print_A             = NULL;
GtkWidget *BN_print_B             = NULL;
GtkWidget *BN_print_C             = NULL;
GtkWidget *BN_print_D             = NULL;
GtkWidget *BN_save_A              = NULL;
GtkWidget *BN_save_B              = NULL;
GtkWidget *BN_save_C              = NULL;
GtkWidget *BN_save_D              = NULL;
GtkWidget *BN_freeze_A            = NULL;
GtkWidget *BN_freeze_B            = NULL;
GtkWidget *BN_freeze_C            = NULL;
GtkWidget *BN_freeze_D            = NULL;
GtkWidget *BN_generate            = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_terminar            = NULL;
GtkWidget *EB_update              = NULL;
GtkWidget *FR_update              = NULL;
GtkWidget *BN_ok                  = NULL;

GtkWidget *EB_reporte             = NULL;
GtkWidget *FR_reporte             = NULL;
GtkWidget *PB_reporte             = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Activar_Campos              ();
void Arma_Pre_Examenes           ();
int  asigna_boletos              (int j);
void Calcular_Total_Examenes     ();
void Cambio_Codigo               ();
void Cambia_Descripcion          (GtkWidget *widget, gpointer user_data);
void Cambio_Esquema              (GtkWidget *widget, gpointer user_data);
void Carga_Esquema               (PGresult *res);
void Carga_Pre_Examen            (PGresult *res);
void Carga_Preguntas             (char * materia);
long double CDF                  (long double X, long double Media, long double Desv);
long double combinaciones        (int k, int n);
void Connect_widgets             ();
void Fin_de_Programa             (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                 (GtkWidget *widget, gpointer user_data);
void Graba_Descripcion           ();
void Graba_Pre_Examen            (int * Pre_Examen, long double media, long double desviacion, int frecuencia, long double novedad, long double alfa, long double rpb);
void Grafico_y_Tabla             (FILE * Archivo_Latex, long double media, long double desviacion, int N_preguntas);
void Imprime_pregunta            (char * ejercicio, int secuencia, FILE * Archivo_Latex,char * prefijo);
void Imprime_Reporte             (char letra, int * Pre_Examen, long double media, long double desviacion);
void Imprime_Tabla               (int * Pre_Examen, long double media, long double desviacion);
void Init_Fields                 ();
void Interface_Coloring          ();
void Llena_Pre_Examen            (int * Pre_Examen, long double * media, long double * desviacion, int * frecuencia, long double * novedad, long double * alfa, long double * Rpb);
void on_BN_freeze_A_clicked      (GtkWidget *widget, gpointer user_data);
void on_BN_freeze_B_clicked      (GtkWidget *widget, gpointer user_data);
void on_BN_freeze_C_clicked      (GtkWidget *widget, gpointer user_data);
void on_BN_freeze_D_clicked      (GtkWidget *widget, gpointer user_data);
void on_BN_generate_clicked      (GtkWidget *widget, gpointer user_data);
void on_BN_OK_clicked            (GtkWidget *widget, gpointer user_data);
void on_BN_print_A_clicked       (GtkWidget *widget, gpointer user_data);
void on_BN_print_B_clicked       (GtkWidget *widget, gpointer user_data);
void on_BN_print_C_clicked       (GtkWidget *widget, gpointer user_data);
void on_BN_print_D_clicked       (GtkWidget *widget, gpointer user_data);
void on_BN_save_A_clicked        (GtkWidget *widget, gpointer user_data);
void on_BN_save_B_clicked        (GtkWidget *widget, gpointer user_data);
void on_BN_save_C_clicked        (GtkWidget *widget, gpointer user_data);
void on_BN_save_D_clicked        (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked      (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked          (GtkWidget *widget, gpointer user_data);
void on_EN_descripcion_activate  (GtkWidget *widget, gpointer user_data);
void on_SP_codigo_activate       (GtkWidget *widget, gpointer user_data);
void on_WN_ex3020_destroy        (GtkWidget *widget, gpointer user_data);
void Ordena_Pre_Examen           (int * Pre_Examen);
void Prepara_Grafico_Normal      (FILE * Archivo_Latex, long double media, long double desviacion, int N);
void Read_Only_Fields            ();
void Tabla_Probabilidades        (FILE * Archivo_Latex, long double media, long double desviacion, int N);

void Update_dificultad          (long double media, long double desviacion, GtkWidget * DA_dif, int activo);
void Update_novedad             (long double dias, GtkWidget * DA_dias, int activo);
void Update_alfa                (long double alfa, GtkWidget * DA_alfa, int activo);
void Update_rpb                 (long double rpb, GtkWidget * DA_disc, int activo);

void on_DA_dif_A_expose         (GtkWidget *widget, gpointer user_data);
void on_DA_disc_A_expose        (GtkWidget *widget, gpointer user_data);
void on_DA_Cron_A_expose        (GtkWidget *widget, gpointer user_data);
void on_DA_dias_A_expose        (GtkWidget *widget, gpointer user_data);

void on_DA_dif_B_expose         (GtkWidget *widget, gpointer user_data);
void on_DA_disc_B_expose        (GtkWidget *widget, gpointer user_data);
void on_DA_Cron_B_expose        (GtkWidget *widget, gpointer user_data);
void on_DA_dias_B_expose        (GtkWidget *widget, gpointer user_data);

void on_DA_dif_C_expose         (GtkWidget *widget, gpointer user_data);
void on_DA_disc_C_expose        (GtkWidget *widget, gpointer user_data);
void on_DA_Cron_C_expose        (GtkWidget *widget, gpointer user_data);
void on_DA_dias_C_expose        (GtkWidget *widget, gpointer user_data);

void on_DA_dif_D_expose         (GtkWidget *widget, gpointer user_data);
void on_DA_disc_D_expose        (GtkWidget *widget, gpointer user_data);
void on_DA_Cron_D_expose        (GtkWidget *widget, gpointer user_data);
void on_DA_dias_D_expose        (GtkWidget *widget, gpointer user_data);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define MINIMO(a,b) ((a)<(b))?(a):(b)
#define DIAS_MAXIMO       730 /* 2 años o 4 semestres */
#define PI 3.141592654
#define DA_HEIGHT 95
#define DA_WIDTH  181
#define DA_MIDDLE (DA_WIDTH / 2)
#define DA_BEGIN   5
#define CERO (DA_BEGIN + 84)
#define ARROW_LENGTH 68
#define ARROW_SIZE   16
#define ARROW_WIDTH  10
#define ARROW_BODY (ARROW_LENGTH - ARROW_SIZE + 2)
#define SHADOW_OFFSET 4

struct PREGUNTA
{
  char        ejercicio [6];
  int         secuencia;
  char        tema    [CODIGO_TEMA_SIZE    + 1];
  char        subtema [CODIGO_SUBTEMA_SIZE + 1];
  long double media;
  long double varianza;
  long double alfa;
  long double Rpb;
  int         estado;
  int         boletos_original;
  int         boletos;
  int         total_boletos;
  int         frecuencia;
  int         cantidad;
  int         N_total;
  int         Year;
  int         Month;
  int         Day;
  int         dias_sin_usar;
  int         Header;
  char        codigo_unico[6];
};

struct LINEA_ESQUEMA
{
  char tema    [CODIGO_TEMA_SIZE    + 1];
  char subtema [CODIGO_SUBTEMA_SIZE + 1];
  int  cantidad;
  int  maximo;
  int  inicio;
  int  total_boletos_original;
  int  total_boletos;
};

long double media_A, varianza_A, desviacion_A, novedad_A, alfa_A, Rpb_A;
long double media_B, varianza_B, desviacion_B, novedad_B, alfa_B, Rpb_B;
long double media_C, varianza_C, desviacion_C, novedad_C, alfa_C, Rpb_C;
long double media_D, varianza_D, desviacion_D, novedad_D, alfa_D, Rpb_D;
int frecuencia_A, frecuencia_B, frecuencia_C, frecuencia_D;


int Lineas_Esquema;
int Preguntas_Pre_Examen;
int Total_Preguntas;
int Pre_Examenes_Congelados=0;
long double Total_Examenes;

struct LINEA_ESQUEMA * Esquema    = NULL;
struct PREGUNTA      * Preguntas  = NULL;
int                  * Pre_Examen_A = NULL;
int                  * Pre_Examen_B = NULL;
int                  * Pre_Examen_C = NULL;
int                  * Pre_Examen_D = NULL;

int PREEXAMEN_EXISTE;
int A_activo, B_activo, C_activo, D_activo;

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX3020.glade", &error))
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

          Init_Fields ();
          gtk_widget_show (window1);

          /* start the event loop */
          gtk_main();
	 }
     }

  return 0;
}

void Connect_widgets()
{
  window1              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex3020"));
  window2              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex3020_usado"));
  window3              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));
  window4              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_reporte"));

  SP_codigo              = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_codigo"));
  SP_esquema             = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_esquema"));
  EN_descripcion         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));
  EN_esquema_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema_descripcion"));
  EN_materia             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia"));
  EN_materia_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia_descripcion"));
  EN_preguntas           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_preguntas"));
  EN_examenes            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_examenes"));

  EB_A      = GTK_WIDGET (gtk_builder_get_object (builder, "EB_A"));
  FR_A      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_A"));
  FR_dif_A  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dif_A"));
  FR_disc_A = GTK_WIDGET (gtk_builder_get_object (builder, "FR_disc_A"));
  FR_Cron_A = GTK_WIDGET (gtk_builder_get_object (builder, "FR_Cron_A"));
  FR_dias_A = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dias_A"));
  DA_dif_A  = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dif_A"));
  DA_disc_A = GTK_WIDGET (gtk_builder_get_object (builder, "DA_disc_A"));
  DA_Cron_A = GTK_WIDGET (gtk_builder_get_object (builder, "DA_Cron_A"));
  DA_dias_A = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dias_A"));
  
  EB_B      = GTK_WIDGET (gtk_builder_get_object (builder, "EB_B"));
  FR_B      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_B"));
  FR_dif_B  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dif_B"));
  FR_disc_B = GTK_WIDGET (gtk_builder_get_object (builder, "FR_disc_B"));
  FR_Cron_B = GTK_WIDGET (gtk_builder_get_object (builder, "FR_Cron_B"));
  FR_dias_B = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dias_B"));
  DA_dif_B  = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dif_B"));
  DA_disc_B = GTK_WIDGET (gtk_builder_get_object (builder, "DA_disc_B"));
  DA_Cron_B = GTK_WIDGET (gtk_builder_get_object (builder, "DA_Cron_B"));
  DA_dias_B = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dias_B"));

  EB_C      = GTK_WIDGET (gtk_builder_get_object (builder, "EB_C"));
  FR_C      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_C"));
  FR_dif_C  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dif_C"));
  FR_disc_C = GTK_WIDGET (gtk_builder_get_object (builder, "FR_disc_C"));
  FR_Cron_C = GTK_WIDGET (gtk_builder_get_object (builder, "FR_Cron_C"));
  FR_dias_C = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dias_C"));
  DA_dif_C  = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dif_C"));
  DA_disc_C = GTK_WIDGET (gtk_builder_get_object (builder, "DA_disc_C"));
  DA_Cron_C = GTK_WIDGET (gtk_builder_get_object (builder, "DA_Cron_C"));
  DA_dias_C = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dias_C"));

  EB_D      = GTK_WIDGET (gtk_builder_get_object (builder, "EB_D"));
  FR_D      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_D"));
  FR_dif_D  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dif_D"));
  FR_disc_D = GTK_WIDGET (gtk_builder_get_object (builder, "FR_disc_D"));
  FR_Cron_D = GTK_WIDGET (gtk_builder_get_object (builder, "FR_Cron_D"));
  FR_dias_D = GTK_WIDGET (gtk_builder_get_object (builder, "FR_dias_D"));
  DA_dif_D  = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dif_D"));
  DA_disc_D = GTK_WIDGET (gtk_builder_get_object (builder, "DA_disc_D"));
  DA_Cron_D = GTK_WIDGET (gtk_builder_get_object (builder, "DA_Cron_D"));
  DA_dias_D = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dias_D"));

  IM_A                   = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_A"));
  IM_B                   = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_B"));
  IM_C                   = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_C"));
  IM_D                   = (GtkImage  *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_D"));

  FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  FR_botones_A           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones_A"));
  FR_botones_B           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones_B"));
  FR_botones_C           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones_C"));
  FR_botones_D           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones_D"));

  EN_estudiantes_A               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_estudiantes_A"));
  EN_estudiantes_B               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_estudiantes_B"));
  EN_estudiantes_C               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_estudiantes_C"));
  EN_estudiantes_D               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_estudiantes_D"));

  EN_media_A             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_A"));
  EN_media_B             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_B"));
  EN_media_C             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_C"));
  EN_media_D             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_D"));

  EN_desviacion_A        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion_A"));
  EN_desviacion_B        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion_B"));
  EN_desviacion_C        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion_C"));
  EN_desviacion_D        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion_D"));

  EN_alfa_A              = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_A"));
  EN_alfa_B              = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_B"));
  EN_alfa_C              = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_C"));
  EN_alfa_D              = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_D"));

  EN_Rpb_A               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb_A"));
  EN_Rpb_B               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb_B"));
  EN_Rpb_C               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb_C"));
  EN_Rpb_D               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb_D"));

  BN_print_A             = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print_A"));
  BN_print_B             = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print_B"));
  BN_print_C             = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print_C"));
  BN_print_D             = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print_D"));
  BN_save_A              = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save_A"));
  BN_save_B              = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save_B"));
  BN_save_C              = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save_C"));
  BN_save_D              = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save_D"));
  BN_freeze_A            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_freeze_A"));
  BN_freeze_B            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_freeze_B"));
  BN_freeze_C            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_freeze_C"));
  BN_freeze_D            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_freeze_D"));

  BN_generate            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_generate"));
  BN_undo                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));

  FR_preexamenusado      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_preexamenusado"));
  EN_examenes            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_examenes"));
  EB_update              = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
  FR_update              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));
  BN_ok                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));

  EB_reporte         = GTK_WIDGET (gtk_builder_get_object (builder, "EB_reporte"));
  FR_reporte         = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reporte"));
  PB_reporte         = GTK_WIDGET (gtk_builder_get_object (builder, "PB_reporte"));

  g_signal_connect( G_OBJECT( DA_dif_A),  "expose-event", G_CALLBACK( on_DA_dif_A_expose  ), NULL);
  g_signal_connect( G_OBJECT( DA_disc_A), "expose-event", G_CALLBACK( on_DA_disc_A_expose ), NULL);
  g_signal_connect( G_OBJECT( DA_Cron_A), "expose-event", G_CALLBACK( on_DA_Cron_A_expose ), NULL);
  g_signal_connect( G_OBJECT( DA_dias_A), "expose-event", G_CALLBACK( on_DA_dias_A_expose ), NULL);

  g_signal_connect( G_OBJECT( DA_dif_B),  "expose-event", G_CALLBACK( on_DA_dif_B_expose  ), NULL);
  g_signal_connect( G_OBJECT( DA_disc_B), "expose-event", G_CALLBACK( on_DA_disc_B_expose ), NULL);
  g_signal_connect( G_OBJECT( DA_Cron_B), "expose-event", G_CALLBACK( on_DA_Cron_B_expose ), NULL);
  g_signal_connect( G_OBJECT( DA_dias_B), "expose-event", G_CALLBACK( on_DA_dias_B_expose ), NULL);

  g_signal_connect( G_OBJECT( DA_dif_C),  "expose-event", G_CALLBACK( on_DA_dif_C_expose  ), NULL);
  g_signal_connect( G_OBJECT( DA_disc_C), "expose-event", G_CALLBACK( on_DA_disc_C_expose ), NULL);
  g_signal_connect( G_OBJECT( DA_Cron_C), "expose-event", G_CALLBACK( on_DA_Cron_C_expose ), NULL);
  g_signal_connect( G_OBJECT( DA_dias_C), "expose-event", G_CALLBACK( on_DA_dias_C_expose ), NULL);

  g_signal_connect( G_OBJECT( DA_dif_D),  "expose-event", G_CALLBACK( on_DA_dif_D_expose  ), NULL);
  g_signal_connect( G_OBJECT( DA_disc_D), "expose-event", G_CALLBACK( on_DA_disc_D_expose ), NULL);
  g_signal_connect( G_OBJECT( DA_Cron_D), "expose-event", G_CALLBACK( on_DA_Cron_D_expose ), NULL);
  g_signal_connect( G_OBJECT( DA_dias_D), "expose-event", G_CALLBACK( on_DA_dias_D_expose ), NULL);
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_esquema_descripcion, FALSE);
  gtk_widget_set_can_focus(EN_materia,             FALSE);
  gtk_widget_set_can_focus(EN_materia_descripcion, FALSE);
  gtk_widget_set_can_focus(EN_preguntas,           FALSE);
  gtk_widget_set_can_focus(EN_examenes,            FALSE);

  gtk_widget_set_can_focus(EN_estudiantes_A,  FALSE);
  gtk_widget_set_can_focus(EN_estudiantes_B,  FALSE);
  gtk_widget_set_can_focus(EN_estudiantes_C,  FALSE);
  gtk_widget_set_can_focus(EN_estudiantes_D,  FALSE);
  gtk_widget_set_can_focus(EN_media_A  ,      FALSE);
  gtk_widget_set_can_focus(EN_media_B  ,      FALSE);
  gtk_widget_set_can_focus(EN_media_C  ,      FALSE);
  gtk_widget_set_can_focus(EN_media_D  ,      FALSE);
  gtk_widget_set_can_focus(EN_desviacion_A ,  FALSE);
  gtk_widget_set_can_focus(EN_desviacion_B ,  FALSE);
  gtk_widget_set_can_focus(EN_desviacion_C ,  FALSE);
  gtk_widget_set_can_focus(EN_desviacion_D ,  FALSE);
  gtk_widget_set_can_focus(EN_alfa_A  ,       FALSE);
  gtk_widget_set_can_focus(EN_alfa_B  ,       FALSE);
  gtk_widget_set_can_focus(EN_alfa_C  ,       FALSE);
  gtk_widget_set_can_focus(EN_alfa_D  ,       FALSE);
  gtk_widget_set_can_focus(EN_Rpb_A  ,        FALSE);
  gtk_widget_set_can_focus(EN_Rpb_B  ,        FALSE);
  gtk_widget_set_can_focus(EN_Rpb_C  ,        FALSE);
  gtk_widget_set_can_focus(EN_Rpb_D  ,        FALSE);
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (IMPORTANT_WINDOW, &color);
  gtk_widget_modify_bg(window2, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (IMPORTANT_FR, &color);
  gtk_widget_modify_bg(FR_preexamenusado,     GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_botones,  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones_A,GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones_B,GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones_C,GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones_D,GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(EN_descripcion,        GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_codigo),  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_esquema), GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_generate, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save_A,   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save_B,   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save_C,   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save_D,   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_freeze_A, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_freeze_B, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_freeze_C, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_freeze_D, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print_A,  GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print_B,  GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print_C,  GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print_D,  GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,       GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_generate, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save_A,   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save_B,   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save_C,   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save_D,   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_freeze_A, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_freeze_B, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_freeze_C, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_freeze_D, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print_A,  GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print_B,  GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print_C,  GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print_D,  GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,       GTK_STATE_ACTIVE, &color);

  gdk_color_parse (PROCESSING_WINDOW, &color);
  gtk_widget_modify_bg(EB_reporte,  GTK_STATE_NORMAL,   &color); 

  gdk_color_parse (PROCESSING_PB, &color);
  gtk_widget_modify_bg(GTK_WIDGET(PB_reporte),  GTK_STATE_PRELIGHT, &color);
  
  gdk_color_parse (PROCESSING_FR, &color);
  gtk_widget_modify_bg(FR_reporte,  GTK_STATE_NORMAL, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,              GTK_STATE_NORMAL, &color);

  gdk_color_parse (FINISHED_WORK_FR, &color);
  gtk_widget_modify_bg(FR_update,              GTK_STATE_NORMAL, &color);
}

void Init_Fields()
{
   GdkColor color;
   int  Last, Last_esquema;
   char Codigo[6];
   char esquema[6];

   media_A = varianza_A = desviacion_A = novedad_A = alfa_A = Rpb_A = 0.0;
   media_B = varianza_B = desviacion_B = novedad_B = alfa_B = Rpb_B = 0.0;
   media_C = varianza_C = desviacion_C = novedad_C = alfa_C = Rpb_C = 0.0;
   media_D = varianza_D = desviacion_D = novedad_D = alfa_D = Rpb_D = 0.0;
   frecuencia_A = frecuencia_B = frecuencia_C = frecuencia_D = 0;

   gtk_widget_set_visible (GTK_WIDGET(EB_A), TRUE);
   gtk_widget_set_visible (GTK_WIDGET(EB_B), TRUE);
   gtk_widget_set_visible (GTK_WIDGET(EB_C), TRUE);
   gtk_widget_set_visible (GTK_WIDGET(EB_D), TRUE);

   gdk_color_parse (SECONDARY_AREA, &color);
   gtk_widget_modify_bg(EB_A, GTK_STATE_NORMAL,   &color);
   gtk_widget_modify_bg(EB_B, GTK_STATE_NORMAL,   &color);
   gtk_widget_modify_bg(EB_C, GTK_STATE_NORMAL,   &color);
   gtk_widget_modify_bg(EB_D, GTK_STATE_NORMAL,   &color);

   gdk_color_parse (STANDARD_FRAME, &color);
   gtk_widget_modify_bg(FR_A,      GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_dif_A,  GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_disc_A, GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_Cron_A, GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_dias_A, GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_B,      GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_dif_B,  GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_disc_B, GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_Cron_B, GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_dias_B, GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_C,      GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_dif_C,  GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_disc_C, GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_Cron_C, GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_dias_C, GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_D,      GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_dif_D,  GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_disc_D, GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_Cron_D, GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(FR_dias_D, GTK_STATE_NORMAL, &color);

   gdk_color_parse (STANDARD_ENTRY, &color);
   gtk_widget_modify_bg(EN_estudiantes_A  , GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_estudiantes_B  , GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_estudiantes_C  , GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_estudiantes_D  , GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_media_A  ,       GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_media_B  ,       GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_media_C  ,       GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_media_D  ,       GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_desviacion_A ,   GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_desviacion_B ,   GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_desviacion_C ,   GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_desviacion_D ,   GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_alfa_A  ,        GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_alfa_B  ,        GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_alfa_C  ,        GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_alfa_D  ,        GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_Rpb_A  ,         GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_Rpb_B  ,         GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_Rpb_C  ,         GTK_STATE_NORMAL, &color);
   gtk_widget_modify_bg(EN_Rpb_D  ,         GTK_STATE_NORMAL, &color);

   gtk_widget_set_sensitive(BN_print_A,  0);
   gtk_widget_set_sensitive(BN_save_A,   0);
   gtk_widget_set_sensitive(BN_freeze_A, 0);
   gtk_widget_set_sensitive(BN_print_B,  0);
   gtk_widget_set_sensitive(BN_save_B,   0);
   gtk_widget_set_sensitive(BN_freeze_B, 0);
   gtk_widget_set_sensitive(BN_print_C,  0);
   gtk_widget_set_sensitive(BN_save_C,   0);
   gtk_widget_set_sensitive(BN_freeze_C, 0);
   gtk_widget_set_sensitive(BN_print_D,  0);
   gtk_widget_set_sensitive(BN_save_D,   0);
   gtk_widget_set_sensitive(BN_freeze_D, 0);

   A_activo = B_activo = C_activo = D_activo = 1;

   /* Recupera el pre-examen con código más alto y lo incrementa */

   res = PQEXEC(DATABASE, "SELECT codigo_pre_examen from EX_pre_examenes order by codigo_pre_examen DESC limit 1");

   Last = 0;
   if (PQntuples(res))
      {
       Last = atoi (PQgetvalue (res, 0, 0));
      }
   PQclear(res);

   Last++;
   sprintf (Codigo,"%05d",Last);

   if (Last > 1)
      {
       gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo), 1);
       gtk_spin_button_set_range (SP_codigo, 1.0, (long double) Last);
      }
   else
      { /* Primer pre-examen que se registra */
       gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo), 0);
       gtk_spin_button_set_range (SP_codigo, 0.0, (long double) Last);
      }
   gtk_spin_button_set_value (SP_codigo, Last);

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
      { /* Primer pre-examen que se registra */
       gtk_widget_set_sensitive(GTK_WIDGET(SP_esquema), 0);
       gtk_spin_button_set_range (SP_esquema, 0.0, (long double) Last_esquema);
      }
   gtk_spin_button_set_value (SP_esquema, Last_esquema);

   gtk_entry_set_text(GTK_ENTRY(EN_descripcion),         "*** Descripción de Preexamen ***");
   gtk_entry_set_text(GTK_ENTRY(EN_materia),             "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion), "\0");

   gtk_entry_set_text(GTK_ENTRY(EN_preguntas      ), "0");
   gtk_entry_set_text(GTK_ENTRY(EN_examenes       ), "0");

   gtk_widget_set_sensitive(EN_descripcion        , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(SP_esquema), 0);
   gtk_widget_set_sensitive(EN_esquema_descripcion, 0);
   gtk_widget_set_sensitive(EN_materia,             0);
   gtk_widget_set_sensitive(EN_materia_descripcion, 0);

   gtk_widget_set_sensitive(EN_preguntas,           0);
   gtk_widget_set_sensitive(EN_examenes,            0);

   gtk_widget_set_sensitive(FR_A, 0);
   gtk_widget_set_sensitive(FR_B, 0);
   gtk_widget_set_sensitive(FR_C, 0);
   gtk_widget_set_sensitive(FR_D, 0);

   gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_A), "           0");
   gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_B), "           0");
   gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_C), "           0");
   gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_D), "           0");

   gtk_entry_set_text(GTK_ENTRY(EN_media_A), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_media_B), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_media_C), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_media_D), "        0.0");

   gtk_entry_set_text(GTK_ENTRY(EN_desviacion_A), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_desviacion_B), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_desviacion_C), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_desviacion_D), "        0.0");

   gtk_entry_set_text(GTK_ENTRY(EN_alfa_A), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_alfa_B), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_alfa_C), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_alfa_D), "        0.0");

   gtk_entry_set_text(GTK_ENTRY(EN_Rpb_A), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_Rpb_B), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_Rpb_C), "        0.0");
   gtk_entry_set_text(GTK_ENTRY(EN_Rpb_D), "        0.0");

   gtk_widget_set_sensitive(BN_generate,0);
   gtk_widget_set_sensitive(BN_undo,    1);

   if (Esquema)      free (Esquema);
   if (Pre_Examen_A) free (Pre_Examen_A);
   if (Pre_Examen_B) free (Pre_Examen_B);
   if (Pre_Examen_C) free (Pre_Examen_C);
   if (Pre_Examen_D) free (Pre_Examen_D);
   if (Preguntas)    free (Preguntas);

   Esquema      = NULL;
   Pre_Examen_A = Pre_Examen_B = Pre_Examen_C = Pre_Examen_D = NULL;
   Preguntas    = NULL;

   Preguntas_Pre_Examen = 0;
   Total_Preguntas    = 0;

   PREEXAMEN_EXISTE = 0;

   if (Last > 1)
      gtk_widget_grab_focus (GTK_WIDGET(SP_codigo));
   else
      on_SP_codigo_activate(NULL, NULL);
}

void Cambio_Codigo()
{
  long int k;
  char codigo [10];
  char PG_command [1000];
  PGresult *res;

  k = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
  sprintf (codigo, "%05ld", k);

  sprintf (PG_command,"SELECT * from EX_pre_examenes where codigo_Pre_Examen = '%s'", codigo);

  gtk_widget_set_sensitive(GTK_WIDGET(SP_codigo), 0);
  res = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res))
     { /* Registro ya existe... */
      Carga_Pre_Examen (res);
      gtk_widget_grab_focus   (BN_generate);
     }
  else
     {
      gtk_widget_set_sensitive(EN_descripcion, 1);
      gtk_widget_grab_focus   (EN_descripcion);
     }

  PQclear(res);
}

void Carga_Pre_Examen (PGresult *res)
{
  long int k;
  GdkColor color;
  int frecuencia;
  long double media, desviacion, probabilidad, novedad, alfa, Rpb;
  char hilera[50];
  char esquema[6];
  char PG_command [1000];
  PGresult *res_aux;

  k = (long int) atoi (PQgetvalue(res, 0, 1)); 
  gtk_spin_button_set_value (SP_esquema, k);

  gtk_entry_set_text(GTK_ENTRY(EN_descripcion),PQgetvalue(res, 0, 2)); 
  strcpy (esquema, PQgetvalue(res, 0, 1)); 

  media_A      = media        = atof (PQgetvalue(res, 0, 3));
  desviacion_A = desviacion   = atof (PQgetvalue(res, 0, 4));
  frecuencia_A = frecuencia   = atoi (PQgetvalue(res, 0, 5));
  novedad_A    = novedad      = atof (PQgetvalue(res, 0, 6));
  alfa_A       = alfa         = atof (PQgetvalue(res, 0, 7));
  Rpb_A        = Rpb          = atof (PQgetvalue(res, 0, 8));
  probabilidad = CDF( 67.5, media, desviacion);

  novedad    = novedad/DIAS_MAXIMO;
  novedad    = MINIMO (1.0, novedad);

  sprintf (hilera,"%9d", frecuencia);
  gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_A), hilera);
  sprintf (hilera,"%7.3Lf", media);
  gtk_entry_set_text(GTK_ENTRY(EN_media_A), hilera);
  sprintf (hilera,"%7.3Lf", desviacion);
  gtk_entry_set_text(GTK_ENTRY(EN_desviacion_A), hilera);
  sprintf (hilera,"%7.3Lf", alfa);
  gtk_entry_set_text(GTK_ENTRY(EN_alfa_A), hilera);
  sprintf (hilera,"%7.3Lf", Rpb);
  gtk_entry_set_text(GTK_ENTRY(EN_Rpb_A), hilera);

  sprintf (PG_command,"SELECT * from EX_esquemas where codigo_esquema = '%.5s'", esquema);
  res_aux = PQEXEC(DATABASE, PG_command);
  Carga_Esquema (res_aux);
  PQclear(res_aux);

  PREEXAMEN_EXISTE = 1;

  A_activo = 0;
  
  gtk_widget_set_visible (GTK_WIDGET(EB_B),     FALSE);
  gtk_widget_set_visible (GTK_WIDGET(EB_C),     FALSE);
  gtk_widget_set_visible (GTK_WIDGET(EB_D),     FALSE);

  gdk_color_parse (FROZEN_COLOR, &color);
  gtk_widget_modify_bg(FR_A,    GTK_STATE_NORMAL, &color);

  /* Pre examen ya fue grabado - no se puede borrar ni editar - solo descripción */
  gtk_widget_set_sensitive(window1,         0);
  gtk_widget_show (window2);
  gtk_widget_set_sensitive(GTK_WIDGET(SP_esquema), 0);
  gtk_widget_set_sensitive(EN_descripcion,         1);
  gtk_widget_set_sensitive(EN_materia,             1);
  gtk_widget_set_sensitive(EN_materia_descripcion, 1);
  gtk_widget_set_sensitive(EN_esquema_descripcion, 1);
  gtk_widget_set_sensitive(EN_preguntas,           1);
  gtk_widget_set_sensitive(EN_examenes,            1);
  gtk_widget_set_sensitive(FR_A,      1);

  gtk_widget_grab_focus   (EN_descripcion);

  gtk_widget_set_sensitive(BN_save_A,  1);
}

void Cambia_Descripcion(GtkWidget *widget, gpointer user_data)
{
  if (PREEXAMEN_EXISTE)
     {
      gtk_widget_grab_focus    (BN_generate);
     }
  else
     {
      gtk_widget_set_sensitive (GTK_WIDGET(SP_esquema), 1);
      gtk_widget_grab_focus    (GTK_WIDGET(SP_esquema));
     }
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
     {
      Carga_Esquema (res);
      if (Preguntas_Pre_Examen)
	 {
	  Activar_Campos ();
	 }
      else
	 {
          gtk_widget_grab_focus    (GTK_WIDGET(SP_esquema));
	 }
     }
  else
     {
      gtk_widget_grab_focus    (GTK_WIDGET(SP_esquema));
     }

  PQclear(res);
}

void Activar_Campos ()
{
  int i;

  gtk_widget_set_sensitive(GTK_WIDGET(SP_esquema), 0);

  gtk_widget_set_sensitive(EN_descripcion,         1);
  gtk_widget_set_sensitive(EN_esquema_descripcion, 1);
  gtk_widget_set_sensitive(EN_materia,             1);
  gtk_widget_set_sensitive(EN_materia_descripcion, 1);
  gtk_widget_set_sensitive(EN_preguntas,           1);
  gtk_widget_set_sensitive(EN_examenes,            1);

  gtk_widget_set_sensitive(FR_A,      1);
  gtk_widget_set_sensitive(FR_B,      1);
  gtk_widget_set_sensitive(FR_C,      1);
  gtk_widget_set_sensitive(FR_D,      1);

  gtk_widget_set_sensitive (BN_generate,1);
  gtk_widget_grab_focus    (BN_generate);
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

  Total_Preguntas      = 0;
  Preguntas_Pre_Examen = 0;
  Lineas_Esquema       = 0;
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

	       Total_Preguntas      += Esquema[Lineas_Esquema].maximo;
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
      Carga_Preguntas (PQgetvalue(res,0,1));
      Pre_Examen_A = (int *) malloc(sizeof(int) * Preguntas_Pre_Examen);
      Pre_Examen_B = (int *) malloc(sizeof(int) * Preguntas_Pre_Examen);
      Pre_Examen_C = (int *) malloc(sizeof(int) * Preguntas_Pre_Examen);
      Pre_Examen_D = (int *) malloc(sizeof(int) * Preguntas_Pre_Examen);
     }

  sprintf (hilera,"%10d",Preguntas_Pre_Examen);
  gtk_entry_set_text(GTK_ENTRY(EN_preguntas), hilera);

  if (Total_Examenes < millon)
     sprintf (hilera,"%15.0Lf",Total_Examenes);
  else
     if (Total_Examenes < millardo)
        sprintf (hilera,"%14.2Lf millones ",Total_Examenes/1000000.0);
     else
        if (Total_Examenes < billon)
	   sprintf (hilera,"%14.2Lf millardos ",Total_Examenes/1000000000.0);
	else
           if (Total_Examenes < trillon)
  	      sprintf (hilera,"%14.2Lf billones ",Total_Examenes/1000000000000.0);
	   else
	      if (Total_Examenes < millontrillon)
	         sprintf (hilera,"%14.2Lf trillones ",Total_Examenes/1000000000000000000.0);
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

void Carga_Preguntas (char * materia)
{
  PGresult *res_aux, *res_aux2;
  char PG_command [3000];
  int i, j, maximo;
  char hilera[50];
  int pregunta_actual;
  int inicio_actual, total_boletos;
  int month, year, day;
  time_t curtime;
  struct tm *loctime;
  int cantidad;

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

  Preguntas = (struct PREGUNTA *) malloc(sizeof(struct PREGUNTA) * Total_Preguntas);
  pregunta_actual = 0;
  for (i=0;i< Lineas_Esquema;i++)
      {
       if (strcmp (Esquema[i].subtema,CODIGO_VACIO) == 0)
          sprintf (PG_command,"SELECT codigo_ejercicio, secuencia_pregunta, dificultad, varianza_dificultad, frecuencia, ultimo_uso_year, ultimo_uso_month, ultimo_uso_day, usa_header, codigo_unico_pregunta, point_biserial, cronbach_alpha_media_con from  bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios, bd_texto_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and texto_ejercicio = consecutivo_texto and materia = '%s' and tema = '%s' and automatico and pregunta = codigo_unico_pregunta order by codigo_ejercicio, secuencia_pregunta",materia, Esquema[i].tema);
       else
	  sprintf (PG_command,"SELECT codigo_ejercicio, secuencia_pregunta, dificultad, varianza_dificultad, frecuencia, ultimo_uso_year, ultimo_uso_month, ultimo_uso_day, usa_header, codigo_unico_pregunta, point_biserial, cronbach_alpha_media_con from  bd_texto_preguntas, bd_estadisticas_preguntas, bd_ejercicios, bd_texto_ejercicios where texto_ejercicio = codigo_consecutivo_ejercicio and texto_ejercicio = consecutivo_texto and materia = '%s' and tema = '%s' and subtema = '%s' and automatico and pregunta = codigo_unico_pregunta order by codigo_ejercicio, secuencia_pregunta",materia, Esquema[i].tema, Esquema[i].subtema);

       inicio_actual = Esquema[i].inicio;
       total_boletos = 0;

       res_aux = PQEXEC(DATABASE, PG_command);

       for (j=0; j<PQntuples(res_aux); j++)
	   {
	    strcpy(Preguntas[inicio_actual + j].ejercicio, PQgetvalue(res_aux,j,0));
	    Preguntas[inicio_actual + j].secuencia = atoi (PQgetvalue(res_aux,j,1));
	    strcpy(Preguntas[inicio_actual + j].tema,    Esquema[i].tema);
	    strcpy(Preguntas[inicio_actual + j].subtema, Esquema[i].subtema);

	    Preguntas[inicio_actual + j].media     = atof (PQgetvalue(res_aux,j,2));
	    Preguntas[inicio_actual + j].varianza  = atof (PQgetvalue(res_aux,j,3));
	    Preguntas[inicio_actual + j].Rpb       = atof (PQgetvalue(res_aux,j,10));
	    Preguntas[inicio_actual + j].alfa      = atof (PQgetvalue(res_aux,j,11));
	    Preguntas[inicio_actual + j].estado    = 0;


	    Preguntas[inicio_actual + j].frecuencia= atoi (PQgetvalue(res_aux,j,4));
	    Preguntas[inicio_actual + j].cantidad  = Esquema[i].cantidad;
	    Preguntas[inicio_actual + j].N_total   = PQntuples(res_aux);
	    Preguntas[inicio_actual + j].Year      = atoi (PQgetvalue(res_aux,j,5));
	    Preguntas[inicio_actual + j].Month     = atoi (PQgetvalue(res_aux,j,6));
	    Preguntas[inicio_actual + j].Day       = atoi (PQgetvalue(res_aux,j,7));

            if (Preguntas[inicio_actual + j].Day && Preguntas[inicio_actual + j].frecuencia)
	       {
		Preguntas[inicio_actual + j].dias_sin_usar = days_between(Preguntas[inicio_actual + j].Day,
                                                                          Preguntas[inicio_actual + j].Month,
									  Preguntas[inicio_actual + j].Year,
									  day, month, year);
	        if (Preguntas[inicio_actual + j].dias_sin_usar > DIAS_MAXIMO) Preguntas[inicio_actual + j].dias_sin_usar = DIAS_MAXIMO;
	       }
            else
	       Preguntas[inicio_actual + j].dias_sin_usar = DIAS_MAXIMO;

	    if (*PQgetvalue(res_aux,j,8) == 't')
	       Preguntas[inicio_actual + j].Header = 1;
	    else
	       Preguntas[inicio_actual + j].Header = 0;

	    strcpy(Preguntas[inicio_actual + j].codigo_unico, PQgetvalue(res_aux,j,9));

	    Preguntas[inicio_actual + j].boletos_original = asigna_boletos (inicio_actual + j);
	    Preguntas[inicio_actual + j].boletos          = Preguntas[inicio_actual + j].boletos_original;

	    total_boletos += Preguntas[inicio_actual + j].boletos;
	   }

       Esquema [i].total_boletos_original = total_boletos;
       Esquema [i].total_boletos          = Esquema [i].total_boletos_original;
       for (j=0; j<PQntuples(res_aux); j++)
	   {
	    Preguntas[inicio_actual + j].total_boletos = total_boletos;
	   }
       PQclear(res_aux);
      }
}

int asigna_boletos (int j)
{
  int boletos;
  long double p, Rpb;
  int dias, estudiantes;

  p   = Preguntas[j].media;
  Rpb = Preguntas[j].Rpb;
  if (Rpb < 0.0) Rpb = 0.0;
  dias = Preguntas[j].dias_sin_usar;
  if (dias > DIAS_MAXIMO) dias = DIAS_MAXIMO;
  estudiantes = Preguntas[j].frecuencia;
  if (estudiantes > 200) estudiantes = 200;

  boletos = parametros.boletos_base     +
            parametros.b_dificultad     +
            parametros.b_discriminacion +
            parametros.b_novedad        +
            parametros.b_usos           +
            (int) round(parametros.m_dificultad     * p    +
                        parametros.m_discriminacion * Rpb  +
                        parametros.m_novedad        * dias +
                        parametros.m_usos           * estudiantes);
  return (boletos);
}

void Arma_Pre_Examenes()
{
  long double Probabilidad, Frecuencia, Novedad;
  char hilera[50];

  if (A_activo)
     {
      Llena_Pre_Examen  (Pre_Examen_A, &media_A, &desviacion_A, &frecuencia_A, &novedad_A, &alfa_A, &Rpb_A);
      Ordena_Pre_Examen (Pre_Examen_A);

      sprintf (hilera,"%9d", frecuencia_A);
      gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_A), hilera);
      sprintf (hilera,"%7.3Lf", media_A);
      gtk_entry_set_text(GTK_ENTRY(EN_media_A), hilera);
      sprintf (hilera,"%7.3Lf", desviacion_A);
      gtk_entry_set_text(GTK_ENTRY(EN_desviacion_A), hilera);
      sprintf (hilera,"%7.3Lf", alfa_A);
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_A), hilera);
      sprintf (hilera,"%7.3Lf", Rpb_A);
      gtk_entry_set_text(GTK_ENTRY(EN_Rpb_A), hilera);

      Novedad    = novedad_A/DIAS_MAXIMO;
      Novedad    = MINIMO (1.0, Novedad);
      Probabilidad = CDF( 67.5, media_A, desviacion_A);
      
      gtk_widget_queue_draw(DA_dif_A);
      gtk_widget_queue_draw(DA_disc_A);
      gtk_widget_queue_draw(DA_dias_A);
      gtk_widget_queue_draw(DA_Cron_A);

      gtk_widget_set_sensitive(BN_print_A,  1);
      gtk_widget_set_sensitive(BN_save_A,   1);
      gtk_widget_set_sensitive(BN_freeze_A, 1);
     }

  if (B_activo)
     {
      Llena_Pre_Examen  (Pre_Examen_B, &media_B, &desviacion_B, &frecuencia_B, &novedad_B, &alfa_B, &Rpb_B);
      Ordena_Pre_Examen (Pre_Examen_B);

      sprintf (hilera,"%9d", frecuencia_B);
      gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_B), hilera);
      sprintf (hilera,"%7.3Lf", media_B);
      gtk_entry_set_text(GTK_ENTRY(EN_media_B), hilera);
      sprintf (hilera,"%7.3Lf", desviacion_B);
      gtk_entry_set_text(GTK_ENTRY(EN_desviacion_B), hilera);
      sprintf (hilera,"%7.3Lf", alfa_B);
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_B), hilera);
      sprintf (hilera,"%7.3Lf", Rpb_B);
      gtk_entry_set_text(GTK_ENTRY(EN_Rpb_B), hilera);

      Novedad    = novedad_B/DIAS_MAXIMO;
      Novedad    = MINIMO (1.0, Novedad);
      Probabilidad = CDF( 67.5, media_B, desviacion_B);

      gtk_widget_queue_draw(DA_dif_B);
      gtk_widget_queue_draw(DA_disc_B);
      gtk_widget_queue_draw(DA_dias_B);
      gtk_widget_queue_draw(DA_Cron_B);

      gtk_widget_set_sensitive(BN_print_B,  1);
      gtk_widget_set_sensitive(BN_save_B,   1);
      gtk_widget_set_sensitive(BN_freeze_B, 1);
     }

  if (C_activo)
     {
      Llena_Pre_Examen  (Pre_Examen_C, &media_C, &desviacion_C, &frecuencia_C, &novedad_C, &alfa_C, &Rpb_C);
      Ordena_Pre_Examen (Pre_Examen_C);

      sprintf (hilera,"%9d", frecuencia_C);
      gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_C), hilera);
      sprintf (hilera,"%7.3Lf", media_C);
      gtk_entry_set_text(GTK_ENTRY(EN_media_C), hilera);
      sprintf (hilera,"%7.3Lf", desviacion_C);
      gtk_entry_set_text(GTK_ENTRY(EN_desviacion_C), hilera);
      sprintf (hilera,"%7.3Lf", alfa_C);
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_C), hilera);
      sprintf (hilera,"%7.3Lf", Rpb_C);
      gtk_entry_set_text(GTK_ENTRY(EN_Rpb_C), hilera);

      Novedad    = novedad_C/DIAS_MAXIMO;
      Novedad    = MINIMO (1.0, Novedad);
      Probabilidad = CDF( 67.5, media_C, desviacion_C);

      gtk_widget_queue_draw(DA_dif_C);
      gtk_widget_queue_draw(DA_disc_C);
      gtk_widget_queue_draw(DA_dias_C);
      gtk_widget_queue_draw(DA_Cron_C);

      gtk_widget_set_sensitive(BN_print_C,  1);
      gtk_widget_set_sensitive(BN_save_C,   1);
      gtk_widget_set_sensitive(BN_freeze_C, 1);
     }

  if (D_activo)
     {
      Llena_Pre_Examen  (Pre_Examen_D, &media_D, &desviacion_D, &frecuencia_D, &novedad_D, &alfa_D, &Rpb_D);
      Ordena_Pre_Examen (Pre_Examen_D);

      sprintf (hilera,"%9d", frecuencia_D);
      gtk_entry_set_text(GTK_ENTRY(EN_estudiantes_D), hilera);
      sprintf (hilera,"%7.3Lf", media_D);
      gtk_entry_set_text(GTK_ENTRY(EN_media_D), hilera);
      sprintf (hilera,"%7.3Lf", desviacion_D);
      gtk_entry_set_text(GTK_ENTRY(EN_desviacion_D), hilera);
      sprintf (hilera,"%7.3Lf", alfa_D);
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_D), hilera);
      sprintf (hilera,"%7.3Lf", Rpb_D);
      gtk_entry_set_text(GTK_ENTRY(EN_Rpb_D), hilera);

      Novedad    = novedad_D/DIAS_MAXIMO;
      Novedad    = MINIMO (1.0, Novedad);
      Probabilidad = CDF( 67.5, media_D, desviacion_D);

      gtk_widget_queue_draw(DA_dif_D);
      gtk_widget_queue_draw(DA_disc_D);
      gtk_widget_queue_draw(DA_dias_D);
      gtk_widget_queue_draw(DA_Cron_D);

      gtk_widget_set_sensitive(BN_print_D,  1);
      gtk_widget_set_sensitive(BN_save_D,   1);
      gtk_widget_set_sensitive(BN_freeze_D, 1);
     }
}

/*******************************************/
/* Hace una rifa entre todas las preguntas */
/* candidatas pata armar el Pre_Examen de e- */
/* xamen. Cada pregunta tiene cierto núme- */
/* ro de boletos. Las características que  */
/* se quieren estimular reciben más bole-  */
/* tos.                                    */
/*******************************************/
void Llena_Pre_Examen (int * Pre_Examen, long double * media, long double * desviacion, int * frecuencia, long double * novedad, long double * alfa, long double * Rpb)
{
  int i, j, k, i0, boletos;
  int Seleccionadas_Linea;
  int ganador, acumulado;
  int escogida;
  long double varianza;
  time_t curtime;
  struct tm *loctime;
  int month, year, day;
  char hilera[50];
  long Total_Dias, Dias_sin_usar;
  long N_muestras, Total_muestras;

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

  escogida = 0;
  *frecuencia = 0;
  for (i=0; i < Total_Preguntas; i++) 
      {
       Preguntas [i].estado  = 0;
       Preguntas [i].boletos = Preguntas [i].boletos_original;
      }

  for (i=0; i < Lineas_Esquema; i++)
      {
       k = Esquema [i].cantidad;
       boletos = Esquema[i].total_boletos;
           
       Seleccionadas_Linea = 0;

       if (k == Esquema[i].maximo)
	  { /* Todas las preguntas asociadas a este tema/subtema deben seleccionarse */
	   j = Esquema[i].inicio;
	   while (Seleccionadas_Linea < k)
	         {
		  Pre_Examen[escogida] = j;
		  escogida++;
		  Preguntas[j].estado = 1;
		  Seleccionadas_Linea++;
		  j++;
	         }
	  }
       else
	  {
           while (Seleccionadas_Linea < k)
	         {
		  ganador = 1 + (int) (boletos * (rand() / (RAND_MAX + 1.0))); /* Esta es la rifa :D :D */
	          j = Esquema[i].inicio;
	          acumulado = Preguntas[j].boletos;
	          while (acumulado < ganador)
		        {
		         j++;
		         acumulado += Preguntas[j].boletos;
		        }
	          if (Preguntas[j].estado == 0)
		     {
		      Pre_Examen[escogida] = j;
		      escogida++;
		      Preguntas[j].estado = 1;
		      Seleccionadas_Linea++;

		      /* El ganador pierde sus boletos para siguiente rifa */
                      boletos -= Preguntas[j].boletos;
                      Preguntas[j].boletos = 0;

		      for (i0=0; i0<Esquema[i].maximo; i0++)
			  {
			   if ((strcmp(Preguntas[j].ejercicio, Preguntas[i0+Esquema[i].inicio].ejercicio) == 0) &&
			       (Preguntas[i0+Esquema[i].inicio].boletos != 0))
			      {
			       Preguntas[i0+Esquema[i].inicio].boletos += parametros.boletos_ejercicio;
			       boletos                                 += parametros.boletos_ejercicio;
			      }
			  }
		     }
	         }
	  }
      }

  Total_muestras = 0;
  Total_Dias     = 0;
  *frecuencia    = 0;
  *media = varianza = *desviacion = *alfa = *Rpb = 0.0;
  for (i=0;i<escogida;i++)
      {
       j = Pre_Examen[i];

       N_muestras    = Preguntas[j].frecuencia+1;

       *frecuencia  += Preguntas[j].frecuencia;
       *media       += Preguntas[j].media;
       varianza     += Preguntas[j].varianza;
       *alfa        += (Preguntas[j].alfa     * N_muestras);
       *Rpb         += (Preguntas[j].Rpb      * N_muestras);

       Total_Dias     += Preguntas[j].dias_sin_usar;

       Total_muestras += N_muestras;
      }

  /* Se escalan la media y varianza por tema para que sea un número entre 0 y 100 */
  *media    =  *media * (100.0/Preguntas_Pre_Examen);
  varianza =  varianza * (100.0/Preguntas_Pre_Examen) * (100.0/Preguntas_Pre_Examen);
  *desviacion = sqrt (varianza);

  *novedad    = ((long double) Total_Dias/ (long double) Preguntas_Pre_Examen);
  *alfa       = *alfa/Total_muestras;
  *Rpb        = *Rpb/Total_muestras;
}

void Ordena_Pre_Examen (int * Pre_Examen)
{
  int i, j, k;

  for (i= Preguntas_Pre_Examen; i >= 0; i--)
      for (j=0; j < (i-1); j++)
	  {
	   if (Pre_Examen[j] > Pre_Examen[j+1])
	          {
	           k              = Pre_Examen [j+1];
                   Pre_Examen [j+1] = Pre_Examen [j];
                   Pre_Examen [j]   = k;
	          }
          }
}

void Imprime_Reporte (char letra, int * Pre_Examen, long double media, long double desviacion)
{
   long int j;
   int i, k;
   FILE * Archivo_Latex;
   char hilera_antes [10000], hilera_despues [10000];
   char codigo[10];
   char esquema[10];
   gchar *materia, *materia_descripcion, *descripcion, *esquema_descripcion;
   struct PREGUNTA_PRE_EXAMEN 
          {
           char ejercicio[6];
           int  secuencia;
 	   int  Header;
          };
   struct PREGUNTA_PRE_EXAMEN * Preguntas_Impresion;
   int actual, N_preguntas_ejercicio;
   char ejercicio_actual[6];
   PGresult *res_header, *res_aux;
   char PG_command [3000];

   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show         (window4);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Preguntas_Impresion = (struct PREGUNTA_PRE_EXAMEN *) malloc (sizeof(struct PREGUNTA_PRE_EXAMEN) * Preguntas_Pre_Examen);
   for (i=0;i<Preguntas_Pre_Examen;i++)
       {
	k = Pre_Examen[i];
	strcpy (Preguntas_Impresion [i].ejercicio, Preguntas[k].ejercicio);
        Preguntas_Impresion [i].secuencia = Preguntas[k].secuencia;
        Preguntas_Impresion [i].Header    = Preguntas[k].Header;
       }
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.1);
   while (gtk_events_pending ()) gtk_main_iteration ();

   j = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
   sprintf (codigo, "%05ld", j);
   j = (long int) gtk_spin_button_get_value_as_int (SP_esquema);
   sprintf (esquema, "%05ld", j);

   descripcion         = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion)        , 0, -1);
   materia             = gtk_editable_get_chars(GTK_EDITABLE(EN_materia)            , 0, -1);
   materia_descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_materia_descripcion), 0, -1);
   esquema_descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_esquema_descripcion), 0, -1);

   Archivo_Latex = fopen ("EX3020.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{Pre Examen}\n");
   fprintf (Archivo_Latex, "\\SetWatermarkLightness{0.7}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);


   fprintf (Archivo_Latex, "\\begin{document}\n");

   sprintf (hilera_antes, "\\title{%s\\\\(Pre Examen %s %c - Esquema %s)\\\\%s}\n", materia_descripcion, codigo, letra, esquema, descripcion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\\maketitle\n\n");
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.15);
   while (gtk_events_pending ()) gtk_main_iteration ();

   actual = 0;
   strcpy (ejercicio_actual, Preguntas_Impresion[actual].ejercicio);
   N_preguntas_ejercicio = 0;
   for (i = actual; (i < Preguntas_Pre_Examen) && (strcmp(ejercicio_actual, Preguntas_Impresion[i].ejercicio) == 0); i++) N_preguntas_ejercicio++;

   if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.17);
   while (gtk_events_pending ()) gtk_main_iteration ();

   if (Preguntas_Impresion[actual].Header)
      {
       sprintf (PG_command,"SELECT texto_header from BD_ejercicios, BD_texto_ejercicios where codigo_ejercicio = '%s' and consecutivo_texto = texto_ejercicio", Preguntas_Impresion[actual].ejercicio);

       res_header = PQEXEC(DATABASE, PG_command);

       if (N_preguntas_ejercicio > 1)
	  {
	   if (N_preguntas_ejercicio == 2)
	      fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
	   else
	      fprintf (Archivo_Latex, "\\textbf{Las preguntas %d a %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	   strcpy (hilera_antes, PQgetvalue (res_header, 0, 0));
           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf (Archivo_Latex, "%s\n", hilera_despues);
           fprintf (Archivo_Latex, "\\rule{8.9cm}{1pt}\n");
           fprintf (Archivo_Latex, "\\begin{questions}\n");
	  }
       else
	  {
           fprintf (Archivo_Latex, "\\begin{questions}\n");
	   Imprime_pregunta (Preguntas_Impresion[actual].ejercicio,Preguntas_Impresion[actual].secuencia, Archivo_Latex, PQgetvalue (res_header,0,0));
           actual++;
           N_preguntas_ejercicio = 0;
	  }
      }
   else
      {
       fprintf (Archivo_Latex, "\\begin{questions}\n");
      }

   for (i=0; i < N_preguntas_ejercicio; i++)
       {
	Imprime_pregunta (Preguntas_Impresion[actual+i].ejercicio,Preguntas_Impresion[actual+i].secuencia, Archivo_Latex, " ");
       }

   if (N_preguntas_ejercicio > 1)
      fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

   actual += N_preguntas_ejercicio;
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.17 + ((long double) actual/Preguntas_Pre_Examen)*0.6);
   while (gtk_events_pending ()) gtk_main_iteration ();

   while (actual < Preguntas_Pre_Examen)
         {
          strcpy (ejercicio_actual, Preguntas_Impresion[actual].ejercicio);
          N_preguntas_ejercicio = 0;
          for (i = actual; (i < Preguntas_Pre_Examen) && (strcmp(ejercicio_actual, Preguntas_Impresion[i].ejercicio) == 0); i++) N_preguntas_ejercicio++;

          if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

          if (Preguntas_Impresion[actual].Header)
             {
              sprintf (PG_command,"SELECT texto_header from BD_ejercicios, BD_texto_ejercicios where codigo_ejercicio = '%s' and consecutivo_texto = texto_ejercicio", 
                                  Preguntas_Impresion[actual].ejercicio);

              res_header = PQEXEC(DATABASE, PG_command);

              if (N_preguntas_ejercicio > 1)
	         {
	          if (N_preguntas_ejercicio == 2)
	             fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
	          else
	             fprintf (Archivo_Latex, "\\textbf{Las preguntas %d a %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	          strcpy (hilera_antes, PQgetvalue (res_header, 0, 0));
                  hilera_LATEX (hilera_antes, hilera_despues);
                  fprintf (Archivo_Latex, "%s\n", hilera_despues);
                  fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
	         }
              else
	         {
	          Imprime_pregunta (Preguntas_Impresion[actual].ejercicio,Preguntas_Impresion[actual].secuencia, Archivo_Latex, PQgetvalue (res_header,0,0));
                  actual++;
                  N_preguntas_ejercicio = 0;
	         }
             }

          for (i=0; i < N_preguntas_ejercicio; i++)
              {
	       Imprime_pregunta (Preguntas_Impresion[actual+i].ejercicio,Preguntas_Impresion[actual+i].secuencia, Archivo_Latex, " ");
              }

          if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

          actual += N_preguntas_ejercicio;
          gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.17 + ((long double) actual/Preguntas_Pre_Examen)*0.6);
          while (gtk_events_pending ()) gtk_main_iteration ();
         }

   fprintf (Archivo_Latex, "\\end{questions}\n");
   fprintf (Archivo_Latex, "\\onecolumn\n");
   fprintf (Archivo_Latex, "\\section*{An\\'{a}lisis}\n");

   Grafico_y_Tabla (Archivo_Latex, media, desviacion, Preguntas_Pre_Examen);

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX3020", 1, PB_reporte, 0.77, 0.2, NULL, NULL);
   system ("rm EX3020*.pdf"); /* queda un archivo por borrar :) */

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 1.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   gtk_widget_hide (window4);
   gtk_widget_set_sensitive(window1, 1);

   free (Preguntas_Impresion);

   g_free(descripcion);
   g_free(materia);
   g_free(materia_descripcion);
   g_free(esquema_descripcion);
}

void Imprime_pregunta (char * ejercicio, int secuencia, FILE * Archivo_Latex,char * prefijo)
{
   char hilera_antes [4000], hilera_despues [4000];
   PGresult * res_aux;
   char PG_command[2000];
   long double media, varianza, desviacion;

   sprintf (PG_command,"SELECT * from BD_texto_preguntas, BD_estadisticas_preguntas, BD_ejercicios where codigo_ejercicio = '%s' and texto_ejercicio = codigo_consecutivo_ejercicio and secuencia_pregunta = %d and pregunta = codigo_unico_pregunta",
            ejercicio, secuencia);
   res_aux = PQEXEC(DATABASE, PG_command);

   sprintf (hilera_antes,"\\question %s\n%s", prefijo, PQgetvalue (res_aux, 0, 8));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   fprintf (Archivo_Latex, "\\begin{answers}\n");

   fprintf (Archivo_Latex, "\\item ");
   strcpy (hilera_antes, PQgetvalue (res_aux, 0, 9));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item ");
   strcpy (hilera_antes, PQgetvalue (res_aux, 0, 10));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item ");
   strcpy (hilera_antes, PQgetvalue (res_aux, 0, 11));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item ");
   strcpy (hilera_antes, PQgetvalue (res_aux, 0, 12));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\item ");
   strcpy (hilera_antes, PQgetvalue (res_aux, 0, 13));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\end{answers}\n");

   if (atoi(PQgetvalue (res_aux, 0, 22)))
      {
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Creada: %s/%s/%s - \\'{U}ltimo Uso: %s/%s/%s - %s %s}\n\n",    PQgetvalue (res_aux, 0, 6), PQgetvalue (res_aux, 0, 5), PQgetvalue (res_aux, 0, 4),
                PQgetvalue (res_aux, 0, 19), PQgetvalue (res_aux, 0, 18), PQgetvalue (res_aux, 0, 17), PQgetvalue (res_aux, 0, 22), strcmp("1",PQgetvalue (res_aux, 0, 22))==0?"vez":"veces");
      }
   else
      {
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Creada: %s/%s/%s - \\textbf{No ha sido usada}}\n\n",    PQgetvalue (res_aux, 0, 6), PQgetvalue (res_aux, 0, 5), PQgetvalue (res_aux, 0, 4));
      }


   media      = (long double) atof(PQgetvalue (res_aux, 0, 26));
   varianza   = (long double) atof(PQgetvalue (res_aux, 0, 27));
   desviacion = sqrt(varianza);
   fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Media: %6.4Lf - Varianza: %6.4Lf - Desviaci\\'{o}n: %6.4Lf}\n\n", media, varianza, desviacion);

   if (atoi(PQgetvalue (res_aux, 0, 22)))
      {
       fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{\\'{I}ndice de Discriminaci\\'{o}n ($r_{pb}$): %6.4Lf}\n\n", (long double)atof(PQgetvalue (res_aux, 0, 28)));
      }

   fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
   fprintf (Archivo_Latex, "\n\n");
}

void Imprime_Tabla (int * Pre_Examen, long double media, long double desviacion)
{
   long int j;
   int i, k, color_actual;
   FILE * Archivo_Latex;
   char hilera_antes [10000], hilera_despues [10000];
   char codigo[10];
   char esquema[10];
   gchar *materia, *materia_descripcion, *descripcion, *esquema_descripcion;
   struct PREGUNTA_PRE_EXAMEN 
          {
           char ejercicio[6];
           int  secuencia;
           char tema    [CODIGO_TEMA_SIZE    + 1];
           char subtema [CODIGO_SUBTEMA_SIZE + 1];
	   long double Rpb;
	   long double media;
           int  dias_sin_usar;
	   int  frecuencia;
	   int  cantidad;
	   int  N_total;
 	   int  Header;
	   int  boletos;
	   int  total_boletos;
          };
   struct PREGUNTA_PRE_EXAMEN * Preguntas_Impresion;
   int actual, N_preguntas_ejercicio;
   char ejercicio_actual[6];
   PGresult *res_header, *res_aux;
   char PG_command [3000];
   char tema_previo    [CODIGO_TEMA_SIZE    + 1];
   char subtema_previo [CODIGO_SUBTEMA_SIZE + 1];
   long double probabilidad, probabilidad_individual;
   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show         (window4);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Preguntas_Impresion = (struct PREGUNTA_PRE_EXAMEN *) malloc (sizeof(struct PREGUNTA_PRE_EXAMEN) * Preguntas_Pre_Examen);
   for (i=0;i<Preguntas_Pre_Examen;i++)
       {
	k = Pre_Examen[i];
	strcpy (Preguntas_Impresion [i].ejercicio, Preguntas[k].ejercicio);
        Preguntas_Impresion [i].secuencia        = Preguntas[k].secuencia;
	strcpy (Preguntas_Impresion [i].tema,      Preguntas[k].tema);
	strcpy (Preguntas_Impresion [i].subtema,   Preguntas[k].subtema);
        Preguntas_Impresion [i].Rpb              = Preguntas[k].Rpb;
        Preguntas_Impresion [i].media            = Preguntas[k].media;
        Preguntas_Impresion [i].dias_sin_usar    = Preguntas[k].dias_sin_usar;
        Preguntas_Impresion [i].frecuencia       = Preguntas[k].frecuencia;
        Preguntas_Impresion [i].cantidad         = Preguntas[k].cantidad;
        Preguntas_Impresion [i].N_total          = Preguntas[k].N_total;
        Preguntas_Impresion [i].Header           = Preguntas[k].Header;
        Preguntas_Impresion [i].boletos          = Preguntas[k].boletos_original;
        Preguntas_Impresion [i].total_boletos    = Preguntas[k].total_boletos;
       }
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.1);
   while (gtk_events_pending ()) gtk_main_iteration ();

   j = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
   sprintf (codigo, "%05ld", j);
   j = (long int) gtk_spin_button_get_value_as_int (SP_esquema);
   sprintf (esquema, "%05ld", j);

   descripcion         = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion)        , 0, -1);
   materia             = gtk_editable_get_chars(GTK_EDITABLE(EN_materia)            , 0, -1);
   materia_descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_materia_descripcion), 0, -1);
   esquema_descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_esquema_descripcion), 0, -1);

   Archivo_Latex = fopen ("EX3020-tabla.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, onecolumn, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);
   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf (Archivo_Latex, "\\SetWatermarkLightness{0.7}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");

   sprintf (hilera_antes, "\\title{%s\\\\(Pre Examen %s - Esquema %s)\\\\%s}\n", materia_descripcion, codigo, esquema, descripcion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\\maketitle\n\n");
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.15);
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\begin{center}\n");
   fprintf (Archivo_Latex, "\\begin{longtable}{|r|c|c|c|c|c|c|c|c|c|c|}\n");
   fprintf (Archivo_Latex, "\\caption*{\\textbf{Preguntas de Preexamen}}\\\\\n");

   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{\\#} & \\textbf{Tema} & \\textbf{Subtema} &\\textbf{Pregunta} & \\textbf{$p$} & \\textbf{$r_{pb}$} &\\textbf{D\\'ias}&\\textbf{Usos}&\\textbf{Selecci\\'on}&\\textbf{Boletos} & \\textbf{Probabilidad}  \\\\ \\hline \\hline \\hline \\hline \\hline \n");
   fprintf (Archivo_Latex, "\\endfirsthead\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{\\#} & \\textbf{Tema} & \\textbf{Subtema} &\\textbf{Pregunta} & \\textbf{$p$} & \\textbf{$r_{pb}$} &\\textbf{D\\'ias}&\\textbf{Usos}&\\textbf{Selecci\\'on}&\\textbf{Boletos} & \\textbf{Probabilidad}  \\\\ \\hline \\hline \\hline \\hline \\hline \n");
   fprintf (Archivo_Latex, "\\endhead\n");
   tema_previo[0]    = '\0';
   subtema_previo[0] = '\0';
   color_actual      = 1;
   for (i=0; i< Preguntas_Pre_Examen; i++)
       {
	if ((strcmp(tema_previo,   Preguntas_Impresion[i].tema)    != 0) ||
	    (strcmp(subtema_previo,Preguntas_Impresion[i].subtema) != 0))
	   {
	    fprintf (Archivo_Latex, "\\hline\n");
	    color_actual = 1 - color_actual;
	    strcpy(tema_previo,    Preguntas_Impresion[i].tema);
	    strcpy(subtema_previo, Preguntas_Impresion[i].subtema);
	   }

	if (color_actual) fprintf (Archivo_Latex,"\\rowcolor{cyan}\n");

	probabilidad_individual = (long double)Preguntas_Impresion[i].boletos/(long double) Preguntas_Impresion[i].total_boletos;
	probabilidad = 1.0;
	for (k=0; k < Preguntas_Impresion[i].cantidad; k++) probabilidad = probabilidad * (1 - probabilidad_individual);
	probabilidad = 1.0 - probabilidad;
 
	sprintf (hilera_antes,"%d & %s & %s &  %s-%d &%5.4Lf & %5.4Lf & %d & %d &%d de %d & %d/%d & %5.4Lf (%5.4Lf)  \\\\ \\hline",
		 i+1,Preguntas_Impresion[i].tema, Preguntas_Impresion[i].subtema,
                     Preguntas_Impresion[i].ejercicio,Preguntas_Impresion[i].secuencia,
		     Preguntas_Impresion[i].media,
		     Preguntas_Impresion[i].Rpb,
		     Preguntas_Impresion[i].dias_sin_usar,
		     Preguntas_Impresion[i].frecuencia,
		     Preguntas_Impresion[i].cantidad, Preguntas_Impresion[i].N_total,
                     Preguntas_Impresion[i].boletos, Preguntas_Impresion[i].total_boletos,
		     probabilidad, probabilidad_individual);

        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s\n", hilera_despues);
       }

   fprintf (Archivo_Latex, "\\hline\n");

   fprintf (Archivo_Latex, "\\end{longtable}\n");
   fprintf (Archivo_Latex, "\\end{center}\n");

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX3020-tabla", 1, PB_reporte, 0.77, 0.2, NULL, NULL);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 1.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   gtk_widget_hide (window4);
   gtk_widget_set_sensitive(window1, 1);

   free (Preguntas_Impresion);

   g_free(descripcion);
   g_free(materia);
   g_free(materia_descripcion);
   g_free(esquema_descripcion);
}

long double CDF (long double X, long double Media, long double Desv)
{
  return (0.5*(1 + (long double)erf((X-Media)/(Desv*M_SQRT2))));
}

void Graba_Pre_Examen (int * Pre_Examen, long double media, long double desviacion, int frecuencia, long double novedad, long double alfa, long double rpb)
{
   long int j;
   int i, k;
   char codigo[10], esquema[10];
   gchar *descripcion;
   char PG_command [2000];
   PGresult *res, *res_aux;

   j = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
   sprintf (codigo, "%05ld", j);
   j = (long int) gtk_spin_button_get_value_as_int (SP_esquema);
   sprintf (esquema, "%05ld", j);

   descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion), 0, -1);

   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

   /* Al borrar el Pre-examen, se borran sus líneas también por el ON DELETE CASCADE */
   sprintf (PG_command,"DELETE from EX_pre_examenes where codigo_Pre_Examen = '%.5s'", codigo);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   sprintf (PG_command,"INSERT into EX_pre_examenes values ('%.5s','%.5s','%.200s',%Lf, %Lf, %d, %Lf, %Lf, %Lf)", 
	    codigo, esquema, descripcion, media, desviacion, frecuencia, novedad, alfa, rpb);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   for (i=0;i<Preguntas_Pre_Examen;i++)
       {
	k = Pre_Examen [i];
        sprintf (PG_command,"INSERT into EX_pre_examenes_lineas values ('%.5s','%.6s','%.6s',%d)", 
		 codigo, Preguntas [k].codigo_unico, Preguntas [k].ejercicio, Preguntas [k].secuencia); 
        res = PQEXEC(DATABASE, PG_command); PQclear(res);
       }

   res = PQEXEC(DATABASE, "END"); PQclear(res);

   g_free (descripcion);

   Imprime_Tabla (Pre_Examen, media, desviacion);
   
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

void Graba_Descripcion ()
{
  long int j;
  char codigo[10];
  gchar * descripcion;
  char PG_command [2000];
  PGresult *res, *res_aux;

  j = (long int) gtk_spin_button_get_value_as_int (SP_codigo);
  sprintf (codigo, "%05ld", j);

  descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion) , 0, -1);

  sprintf (PG_command,"UPDATE EX_pre_examenes set descripcion_Pre_Examen = '%s' where codigo_Pre_Examen = '%.5s'", descripcion, codigo);
  res = PQEXEC(DATABASE, PG_command); PQclear(res);

  g_free (descripcion);
  gtk_widget_set_sensitive(window1, 0);
  gtk_widget_show (window3);

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

void Grafico_y_Tabla(FILE * Archivo_Latex, long double media, long double desviacion, int N_preguntas)
{
    fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
    fprintf (Archivo_Latex, "\\centering\n");
    fprintf (Archivo_Latex, "\\begin{minipage}[c]{0.25\\textwidth}\n");
    fprintf (Archivo_Latex, "\\centering\n");

    Tabla_Probabilidades   (Archivo_Latex, media, desviacion, N_preguntas);
    fprintf (Archivo_Latex, "\\end{minipage}\n");

    Prepara_Grafico_Normal (Archivo_Latex, media, desviacion, N_preguntas);

    fprintf (Archivo_Latex, "\\begin{minipage}[c]{0.71\\textwidth}\n");
    fprintf (Archivo_Latex, "\\includegraphics[scale=1.0]{EX3020-n.pdf}\n");
    fprintf (Archivo_Latex, "\\end{minipage}\n");
    fprintf (Archivo_Latex, "\\end{figure}\n");
}

void Prepara_Grafico_Normal(FILE * Archivo_Latex, long double media, long double desviacion, int N)
{
    FILE * Archivo_gnuplot;
    char Hilera_Antes [2000], Hilera_Despues [2000];
    char comando[1000];

    Archivo_gnuplot = fopen ("EX3020-n.gp","w");

    fprintf (Archivo_gnuplot, "set term postscript eps enhanced \"Times\" 12\n");
    fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
    fprintf (Archivo_gnuplot, "set size 1.1, 0.8\n");
    fprintf (Archivo_gnuplot, "set grid xtics\n");

    fprintf (Archivo_gnuplot, "set output \"EX3020-n.eps\"\n");
    sprintf (Hilera_Antes,"set xlabel \"Media = %Lf - Desviación = %Lf\"", media, desviacion);
    hilera_GNUPLOT (Hilera_Antes, Hilera_Despues);
    fprintf (Archivo_gnuplot, "%s\n", Hilera_Despues);
    hilera_GNUPLOT ("set ylabel \"Proporción\"\n", Hilera_Despues);
    fprintf (Archivo_gnuplot, "%s", Hilera_Despues);

    fprintf (Archivo_gnuplot, "set xrange [%Lf:%Lf]\n", media - 3*desviacion, media + 3*desviacion);
    fprintf (Archivo_gnuplot, "load \".scripts/stat.inc\"\n");
    fprintf (Archivo_gnuplot, "set style fill pattern 8 border 2\n");

    if ((media+1.7*desviacion) >= 67.5)
       {
        fprintf (Archivo_gnuplot, "set arrow from %Lf, 0.03 to %Lf, 0.01 lw 6 lt 1 lc 2 front\n", media+2.4*desviacion, media+1.7*desviacion);
        sprintf (Hilera_Antes, "set label \"%Lf\" at %Lf, 0.032 front\n", 1.0-CDF(67.5, media, desviacion), media+2.4*desviacion);
        hilera_GNUPLOT (Hilera_Antes, Hilera_Despues);
        fprintf (Archivo_gnuplot,"%s", Hilera_Despues);
       }

    fprintf (Archivo_gnuplot, "plot normal (x, %Lf, %Lf) with filledcurve above x1=67.50 notitle, normal (x, %Lf, %Lf) with lines linetype 1 lw 7 lc 3 notitle",
             media, desviacion,media, desviacion);

    fclose (Archivo_gnuplot);

    sprintf(comando, "%s EX3020-n.gp", parametros.gnuplot);
    system (comando);

    sprintf(comando, "mv EX3020-n.gp %s", parametros.ruta_gnuplot);
    system (comando);

    sprintf(comando, "%s EX3020-n.eps", parametros.epstopdf);
    system (comando);

    system ("rm EX3020-n.eps");

}

void Tabla_Probabilidades (FILE * Archivo_Latex, long double media, long double desviacion, int N)
{
   fprintf (Archivo_Latex, "\\rowcolors{1}{white}{green}\n");
   fprintf (Archivo_Latex, "\\begin{tabular}[b]{|r|r|}\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Nota} & \\textbf{Probabilidad} \\\\\n");
   fprintf (Archivo_Latex, "\\hline\\hline\n");

   fprintf (Archivo_Latex, "67.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 67.5, media, desviacion));
   fprintf (Archivo_Latex, "70.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 70.0, media, desviacion));
   fprintf (Archivo_Latex, "72.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 72.5, media, desviacion));
   fprintf (Archivo_Latex, "75.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 75.0, media, desviacion));
   fprintf (Archivo_Latex, "77.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 77.5, media, desviacion));
   fprintf (Archivo_Latex, "80.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 80.0, media, desviacion));
   fprintf (Archivo_Latex, "82.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 82.5, media, desviacion));
   fprintf (Archivo_Latex, "85.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 85.0, media, desviacion));
   fprintf (Archivo_Latex, "87.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 87.5, media, desviacion));
   fprintf (Archivo_Latex, "90.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 90.0, media, desviacion));
   fprintf (Archivo_Latex, "92.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 92.5, media, desviacion));
   fprintf (Archivo_Latex, "95.00 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 95.0, media, desviacion));
   fprintf (Archivo_Latex, "97.50 & %Lf\\\\ \\hline\n\n", 1.0-CDF( 97.5, media, desviacion));
   fprintf (Archivo_Latex, "100.00 & %Lf\\\\ \\hline\n\n",1.0-CDF(100.0, media, desviacion));

   fprintf (Archivo_Latex, "\\hline\n");

   fprintf (Archivo_Latex, "\\end{tabular}\n");
}

void Update_dificultad(long double media, long double desviacion, GtkWidget * DA_dif, int activo)
{
   long double p, r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   p = CDF( 67.5, media, desviacion);
   r = (1.0 - p) * PI;
   cos_r = cos(r);
   sin_r = sin(r);

   if (activo)
      image = cairo_image_surface_create_from_png(".imagenes/dialdifex.png");
   else
      image = cairo_image_surface_create_from_png(".imagenes/dialdifexF.png");
     
   cr = gdk_cairo_create(DA_dif->window );
  
   //   cairo_set_source_rgb(cr, 0.45, 0.45, 0.45);
   if (activo)
      cairo_set_source_rgb(cr, 0.11, 0.11, 0.11);
   else
      cairo_set_source_rgb(cr, 0.098, 0.6039, 0.74902);
     
   cairo_rectangle(cr, 0, 0, DA_WIDTH, DA_HEIGHT);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

   cairo_set_font_size(cr, 8);
   cairo_move_to(cr, 2, 8);
   cairo_show_text(cr, "Dificultad");  

   cairo_set_font_size(cr, 9);
   
   cairo_save(cr); 
   cairo_move_to(cr, 19, 48 + DA_BEGIN);
   cairo_rotate(cr, -0.94248); 
   cairo_show_text(cr, "FÁCIL"); 
   cairo_restore(cr);
   
   cairo_save(cr); 
   cairo_move_to(cr, 52, 14 + DA_BEGIN);
   cairo_rotate(cr, -0.31416); 
   cairo_show_text(cr, "JUSTO"); 
   cairo_restore(cr);
   
   cairo_save(cr); 
   cairo_move_to(cr, 101, 7 + DA_BEGIN);
   cairo_rotate(cr, 0.31416); 
   cairo_show_text(cr, "DURO"); 
   cairo_restore(cr);

   cairo_save(cr); 
   cairo_move_to(cr, 139, 21 + DA_BEGIN);
   cairo_rotate(cr, 0.94248); 
   cairo_show_text(cr, "SEVERO"); 
   cairo_restore(cr);

   cairo_stroke_preserve(cr);

   cairo_set_source_surface(cr, image, 7, DA_BEGIN);
   cairo_paint( cr );

   cairo_set_line_width (cr, 3);

// shadow
   cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

   x = round(cos_r * (ARROW_BODY - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_BODY - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * (ARROW_LENGTH - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);
   
// cuerpo de la aguja
   if (activo)
      cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   else
      cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
      
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_arc( cr, DA_MIDDLE, CERO, 2, 0, 2 * G_PI );
   x = round(cos_r * ARROW_BODY - sin_r * 0);
   y = round(sin_r * ARROW_BODY + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * ARROW_LENGTH - sin_r * 0);
   y = round(sin_r * ARROW_LENGTH + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * ARROW_WIDTH);
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * ARROW_WIDTH);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * (0 - ARROW_WIDTH));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * (0 - ARROW_WIDTH));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);

   cairo_destroy( cr );
}

void Update_novedad(long double dias, GtkWidget * DA_dias, int activo)
{
   long double p, r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   p  = dias/DIAS_MAXIMO;
   p = MINIMO (1.0, p);
   r = (1.0 - p) * PI;
   cos_r = cos(r);
   sin_r = sin(r);

   if (activo)
      image = cairo_image_surface_create_from_png(".imagenes/dialdias.png");
   else
      image = cairo_image_surface_create_from_png(".imagenes/dialdiasF.png");
     
   cr = gdk_cairo_create(DA_dias->window );
  
   //   cairo_set_source_rgb(cr, 0.45, 0.45, 0.45);
   if (activo)
      cairo_set_source_rgb(cr, 0.11, 0.11, 0.11);
   else
      cairo_set_source_rgb(cr, 0.098, 0.6039, 0.74902);
     
   cairo_rectangle(cr, 0, 0, DA_WIDTH, DA_HEIGHT);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

   cairo_set_font_size(cr, 8);
   cairo_move_to(cr, 2, 8);
   cairo_show_text(cr, "Novedad");  

   cairo_set_font_size(cr, 9);
   
   cairo_save(cr); 
   cairo_move_to(cr, 11, 69 + DA_BEGIN);
   cairo_rotate(cr, -1.25664); 
   cairo_show_text(cr, "MUY"); 
   cairo_restore(cr);
   
   cairo_save(cr); 
   cairo_move_to(cr, 18, 44 + DA_BEGIN);
   cairo_rotate(cr, -0.78540); 
   cairo_show_text(cr, "CONOCIDO"); 
   cairo_restore(cr);
   
   cairo_save(cr); 
   cairo_move_to(cr, 110, 5 + DA_BEGIN);
   cairo_rotate(cr, 0.47124); 
   cairo_show_text(cr, "NUEVO"); 
   cairo_restore(cr);

   cairo_stroke_preserve(cr);

   cairo_set_source_surface(cr, image, 7, DA_BEGIN);
   cairo_paint( cr );

   cairo_set_line_width (cr, 3);

// shadow
   cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

   x = round(cos_r * (ARROW_BODY - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_BODY - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * (ARROW_LENGTH - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);
   
// cuerpo de la aguja
   if (activo)
      cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   else
      cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
     
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_arc( cr, DA_MIDDLE, CERO, 2, 0, 2 * G_PI );
   x = round(cos_r * ARROW_BODY - sin_r * 0);
   y = round(sin_r * ARROW_BODY + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * ARROW_LENGTH - sin_r * 0);
   y = round(sin_r * ARROW_LENGTH + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * ARROW_WIDTH);
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * ARROW_WIDTH);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * (0 - ARROW_WIDTH));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * (0 - ARROW_WIDTH));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);

   cairo_destroy( cr );
}

void Update_alfa(long double alfa, GtkWidget * DA_alfa, int activo)
{
   long double r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   r = (1.0 - alfa) * PI;
   cos_r = cos(r);
   sin_r = sin(r);

   if (activo)
      image = cairo_image_surface_create_from_png(".imagenes/dialcronbach.png");
   else
      image = cairo_image_surface_create_from_png(".imagenes/dialcronbachF.png");
     
   cr = gdk_cairo_create(DA_alfa->window );
  
   //   cairo_set_source_rgb(cr, 0.45, 0.45, 0.45);
   if (activo)
      cairo_set_source_rgb(cr, 0.11, 0.11, 0.11);
   else
      cairo_set_source_rgb(cr, 0.098, 0.6039, 0.74902);
     
   cairo_rectangle(cr, 0, 0, DA_WIDTH, DA_HEIGHT);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

   cairo_set_font_size(cr, 8);
   cairo_move_to(cr, 2, 8);
   cairo_show_text(cr, "Alfa de Cronbach");  

   cairo_set_font_size(cr, 8);
   
   cairo_save(cr); 
   cairo_move_to(cr, 139, 17 + DA_BEGIN);
   cairo_rotate(cr, -0.94248); 
   cairo_show_text(cr, "0.70"); 
   cairo_restore(cr);

   cairo_save(cr); 
   cairo_move_to(cr, 162, 47 + DA_BEGIN);
   cairo_rotate(cr, -0.47124); 
   cairo_show_text(cr, "0.85"); 
   cairo_restore(cr);

   cairo_stroke_preserve(cr);

   cairo_set_source_surface(cr, image, 7, DA_BEGIN);
   cairo_paint( cr );

   cairo_set_line_width (cr, 3);

// shadow
   cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

   x = round(cos_r * (ARROW_BODY - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_BODY - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * (ARROW_LENGTH - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);
   
// cuerpo de la aguja
   if (activo)
      cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   else
      cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
     
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_arc( cr, DA_MIDDLE, CERO, 2, 0, 2 * G_PI );
   x = round(cos_r * ARROW_BODY - sin_r * 0);
   y = round(sin_r * ARROW_BODY + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * ARROW_LENGTH - sin_r * 0);
   y = round(sin_r * ARROW_LENGTH + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * ARROW_WIDTH);
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * ARROW_WIDTH);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * (0 - ARROW_WIDTH));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * (0 - ARROW_WIDTH));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);

   cairo_destroy( cr );
}

void Update_rpb(long double rpb, GtkWidget * DA_disc, int activo)
{
   long double r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   r = (1.0 - ((1.0 + rpb)/2.0)) * PI;
   cos_r = cos(r);
   sin_r = sin(r);

   if (activo)
      image = cairo_image_surface_create_from_png(".imagenes/dialdiscex.png");
   else
      image = cairo_image_surface_create_from_png(".imagenes/dialdiscexF.png");
     
   cr = gdk_cairo_create(DA_disc->window );
  
   //   cairo_set_source_rgb(cr, 0.45, 0.45, 0.45);
   if (activo)
      cairo_set_source_rgb(cr, 0.11, 0.11, 0.11);
   else
      cairo_set_source_rgb(cr, 0.098, 0.6039, 0.74902);
     
   cairo_rectangle(cr, 0, 0, DA_WIDTH, DA_HEIGHT);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

   cairo_set_font_size(cr, 8);
   cairo_move_to(cr, 2, 8);
   cairo_show_text(cr, "Discriminación");  

   cairo_set_font_size(cr, 8);
   
   cairo_move_to(cr, DA_MIDDLE - 8, 2 + DA_BEGIN);
   cairo_show_text(cr, "0.0"); 

   cairo_save(cr); 
   cairo_move_to(cr, 128, 11 + DA_BEGIN);
   cairo_rotate(cr, -1.09956); 
   cairo_show_text(cr, "0.3"); 
   cairo_restore(cr);

   cairo_stroke_preserve(cr);

   cairo_set_source_surface(cr, image, 7, DA_BEGIN);
   cairo_paint( cr );

   cairo_set_line_width (cr, 3);

// shadow
   cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

   x = round(cos_r * (ARROW_BODY - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_BODY - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * (ARROW_LENGTH - SHADOW_OFFSET) - sin_r * (0 - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - SHADOW_OFFSET) + cos_r * (0 - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) - sin_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE - SHADOW_OFFSET) + cos_r * (0 - ARROW_WIDTH - SHADOW_OFFSET));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);
   
// cuerpo de la aguja
   if (activo)
      cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   else
      cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_arc( cr, DA_MIDDLE, CERO, 2, 0, 2 * G_PI );
   x = round(cos_r * ARROW_BODY - sin_r * 0);
   y = round(sin_r * ARROW_BODY + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, DA_MIDDLE, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   x = round(cos_r * ARROW_LENGTH - sin_r * 0);
   y = round(sin_r * ARROW_LENGTH + cos_r * 0);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * ARROW_WIDTH);
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * ARROW_WIDTH);
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   x = round(cos_r * (ARROW_LENGTH - ARROW_SIZE) - sin_r * (0 - ARROW_WIDTH));
   y = round(sin_r * (ARROW_LENGTH - ARROW_SIZE) + cos_r * (0 - ARROW_WIDTH));
   x = x + DA_MIDDLE;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   cairo_fill(cr);

   cairo_destroy( cr );
}

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex3020_destroy (GtkWidget *widget, gpointer user_data) 
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

void on_EN_descripcion_activate(GtkWidget *widget, gpointer user_data)
{
  Cambia_Descripcion (widget, user_data);
}

void on_BN_generate_clicked(GtkWidget *widget, gpointer user_data)
{
  Arma_Pre_Examenes ();
}

void on_BN_print_A_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Reporte ('1', Pre_Examen_A, media_A, desviacion_A);
}

void on_BN_print_B_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Reporte ('2', Pre_Examen_B, media_B, desviacion_B);
}

void on_BN_print_C_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Reporte ('3', Pre_Examen_C, media_C, desviacion_C);
}

void on_BN_print_D_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Reporte ('4', Pre_Examen_D, media_D, desviacion_D);
}

void on_BN_save_A_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Pre_Examen (Pre_Examen_A, media_A, desviacion_A, frecuencia_A, novedad_A, alfa_A, Rpb_A);
}

void on_BN_save_B_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Pre_Examen (Pre_Examen_B, media_B, desviacion_B, frecuencia_B, novedad_B, alfa_B, Rpb_B);
}

void on_BN_save_C_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Pre_Examen (Pre_Examen_C, media_C, desviacion_C, frecuencia_C, novedad_C, alfa_C, Rpb_C);
}

void on_BN_save_D_clicked(GtkWidget *widget, gpointer user_data)
{
  if (PREEXAMEN_EXISTE)
     Graba_Descripcion ();
  else
    Graba_Pre_Examen (Pre_Examen_D, media_D, desviacion_D, frecuencia_D, novedad_D, alfa_D, Rpb_D);
}

void on_BN_freeze_A_clicked(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  A_activo = 1 - A_activo;

  if (A_activo)
     {
      gdk_color_parse (STANDARD_FRAME, &color);
      gtk_widget_modify_bg(FR_A,    GTK_STATE_NORMAL, &color);

      gdk_color_parse (STANDARD_ENTRY, &color);
      gtk_widget_modify_bg(EN_estudiantes_A  , GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_media_A  ,       GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_desviacion_A ,   GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_alfa_A  ,        GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_Rpb_A  ,         GTK_STATE_NORMAL, &color);
      gtk_image_set_from_file (IM_A, ".imagenes/empty.png");
     }
  else
     {
      gdk_color_parse (FROZEN_COLOR, &color);
      gtk_widget_modify_bg(FR_A,    GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_estudiantes_A  , GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_media_A  ,       GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_desviacion_A ,   GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_alfa_A  ,        GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_Rpb_A  ,         GTK_STATE_NORMAL, &color);
      gtk_image_set_from_file (IM_A, ".imagenes/BB2.jpg");
     }

  gtk_widget_queue_draw(DA_dif_A);
  gtk_widget_queue_draw(DA_disc_A);
  gtk_widget_queue_draw(DA_Cron_A);
  gtk_widget_queue_draw(DA_dias_A);
}

void on_BN_freeze_B_clicked(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  B_activo = 1 - B_activo;

  if (B_activo)
     {
      gdk_color_parse (STANDARD_FRAME, &color);
      gtk_widget_modify_bg(FR_B,    GTK_STATE_NORMAL, &color);

      gdk_color_parse (STANDARD_ENTRY, &color);
      gtk_widget_modify_bg(EN_estudiantes_B  , GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_media_B  ,       GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_desviacion_B ,   GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_alfa_B  ,        GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_Rpb_B  ,         GTK_STATE_NORMAL, &color);
      gtk_image_set_from_file (IM_B, ".imagenes/empty.png");
     }
  else
     {
      gdk_color_parse (FROZEN_COLOR, &color);
      gtk_widget_modify_bg(FR_B,    GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_estudiantes_B  , GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_media_B  ,       GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_desviacion_B ,   GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_alfa_B  ,        GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_Rpb_B  ,         GTK_STATE_NORMAL, &color);
      gtk_image_set_from_file (IM_B, ".imagenes/BB2.jpg");
     }

  gtk_widget_queue_draw(DA_dif_B);
  gtk_widget_queue_draw(DA_disc_B);
  gtk_widget_queue_draw(DA_Cron_B);
  gtk_widget_queue_draw(DA_dias_B);

}

void on_BN_freeze_C_clicked(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  C_activo = 1 - C_activo;

  if (C_activo)
     {
      gdk_color_parse (STANDARD_FRAME, &color);
      gtk_widget_modify_bg(FR_C,    GTK_STATE_NORMAL, &color);

      gdk_color_parse (STANDARD_ENTRY, &color);
      gtk_widget_modify_bg(EN_estudiantes_C  , GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_media_C  ,       GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_desviacion_C ,   GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_alfa_C  ,        GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_Rpb_C  ,         GTK_STATE_NORMAL, &color);
      gtk_image_set_from_file (IM_C, ".imagenes/empty.png");
     }
  else
     {
      gdk_color_parse (FROZEN_COLOR, &color);
      gtk_widget_modify_bg(FR_C,    GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_estudiantes_C  , GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_media_C  ,       GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_desviacion_C ,   GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_alfa_C  ,        GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_Rpb_C  ,         GTK_STATE_NORMAL, &color);
      gtk_image_set_from_file (IM_C, ".imagenes/BB2.jpg");
     }

  gtk_widget_queue_draw(DA_dif_C);
  gtk_widget_queue_draw(DA_disc_C);
  gtk_widget_queue_draw(DA_Cron_C);
  gtk_widget_queue_draw(DA_dias_C);
}

void on_BN_freeze_D_clicked(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  D_activo = 1 - D_activo;

  if (D_activo)
     {
      gdk_color_parse (STANDARD_FRAME, &color);
      gtk_widget_modify_bg(FR_D,    GTK_STATE_NORMAL, &color);

      gdk_color_parse (STANDARD_ENTRY, &color);
      gtk_widget_modify_bg(EN_estudiantes_D  , GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_media_D  ,       GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_desviacion_D ,   GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_alfa_D  ,        GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_Rpb_D  ,         GTK_STATE_NORMAL, &color);
      gtk_image_set_from_file (IM_D, ".imagenes/empty.png");
     }
  else
     {
      gdk_color_parse (FROZEN_COLOR, &color);
      gtk_widget_modify_bg(FR_D,    GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_estudiantes_D  , GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_media_D  ,       GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_desviacion_D ,   GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_alfa_D  ,        GTK_STATE_NORMAL, &color);
      gtk_widget_modify_bg(EN_Rpb_D  ,         GTK_STATE_NORMAL, &color);
      gtk_image_set_from_file (IM_D, ".imagenes/BB2.jpg");
     }
  
  gtk_widget_queue_draw(DA_dif_D);
  gtk_widget_queue_draw(DA_disc_D);
  gtk_widget_queue_draw(DA_Cron_D);
  gtk_widget_queue_draw(DA_dias_D);
}

void on_BN_OK_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (EN_descripcion);
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
  gtk_widget_hide (window3);
  gtk_widget_set_sensitive(window1, 1);
}

void on_DA_dif_A_expose(GtkWidget *widget, gpointer user_data)
{
  Update_dificultad (media_A, desviacion_A, DA_dif_A, A_activo);
}

void on_DA_disc_A_expose(GtkWidget *widget, gpointer user_data)
{
  Update_rpb (Rpb_A, DA_disc_A, A_activo);
}

void on_DA_Cron_A_expose(GtkWidget *widget, gpointer user_data)
{
  Update_alfa (alfa_A, DA_Cron_A, A_activo);
}

void on_DA_dias_A_expose(GtkWidget *widget, gpointer user_data)
{
  Update_novedad (novedad_A, DA_dias_A, A_activo);
}

void on_DA_dif_B_expose(GtkWidget *widget, gpointer user_data)
{
  Update_dificultad (media_B, desviacion_B, DA_dif_B, B_activo);
}

void on_DA_disc_B_expose(GtkWidget *widget, gpointer user_data)
{
  Update_rpb (Rpb_B, DA_disc_B, B_activo);
}

void on_DA_Cron_B_expose(GtkWidget *widget, gpointer user_data)
{
  Update_alfa (alfa_B, DA_Cron_B, B_activo);
}

void on_DA_dias_B_expose(GtkWidget *widget, gpointer user_data)
{
  Update_novedad (novedad_B, DA_dias_B, B_activo);
}

void on_DA_dif_C_expose(GtkWidget *widget, gpointer user_data)
{
  Update_dificultad (media_C, desviacion_C, DA_dif_C, C_activo);
}

void on_DA_disc_C_expose(GtkWidget *widget, gpointer user_data)
{
  Update_rpb (Rpb_C, DA_disc_C, C_activo);
}

void on_DA_Cron_C_expose(GtkWidget *widget, gpointer user_data)
{
  Update_alfa (alfa_C, DA_Cron_C, C_activo);
}

void on_DA_dias_C_expose(GtkWidget *widget, gpointer user_data)
{
  Update_novedad (novedad_C, DA_dias_C, C_activo);
}

void on_DA_dif_D_expose(GtkWidget *widget, gpointer user_data)
{
  Update_dificultad (media_D, desviacion_D, DA_dif_D, D_activo);
}

void on_DA_disc_D_expose(GtkWidget *widget, gpointer user_data)
{
  Update_rpb (Rpb_D, DA_disc_D, D_activo);
}

void on_DA_Cron_D_expose(GtkWidget *widget, gpointer user_data)
{
  Update_alfa (alfa_D, DA_Cron_D, D_activo);
}

void on_DA_dias_D_expose(GtkWidget *widget, gpointer user_data)
{
  Update_novedad (novedad_D, DA_dias_D, D_activo);
}
