/*

Para compilar:

cc -o EX2040 EX2040.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX2040:                                */
/*                                         */
/*    Actualización Archivo de Preguntas   */
/*    The Examiner 0.0                     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    30 de Junio del 2011                 */
/*                                         */
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    17 de Diciembre 2011                 */
/*-----------------------------------------*/
/*    Nuevo formato de la Base de Datos    */
/*    Cambios a la interfaz                */
/*    Autor: Francisco J. Torres-Rojas     */
/*    11 de Enero 2012                     */
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
/* Postgres stuff          */
/***************************/
PGconn	 *DATABASE;

/*************/
/* GTK stuff */
/*************/
#define MAX_TOTAL_BOLETOS 60

GtkBuilder*builder; 
GError    * error = NULL;

GtkWidget *window1                = NULL;
GtkWidget *window2                = NULL;
GtkWidget *window3                = NULL;
GtkWidget *window4                = NULL;
GtkWidget *window5                = NULL;
GtkWidget *window6                = NULL;

GList *focus_chain;

GtkSpinButton *SP_ejercicio       = NULL;
GtkWidget *SC_ejercicio           = NULL;
GtkWidget *EN_descripcion         = NULL;
GtkWidget *EN_materia_descripcion = NULL;
GtkWidget *EN_tema_descripcion    = NULL;
GtkWidget *EN_subtema_descripcion = NULL;
GtkWidget *EN_preguntas           = NULL;
GtkComboBox *CB_pregunta          = NULL;

GtkWidget *EB_preguntas           = NULL;
GtkWidget *FR_preguntas           = NULL;
GtkWidget *EB_opciones            = NULL;
GtkWidget *FR_opciones            = NULL;

GtkTextView *TV_pregunta        = NULL;
GtkWidget   *BN_extend_question = NULL;
GtkTextView *TV_opcion_A        = NULL;
GtkWidget   *BN_extend_A        = NULL;
GtkTextView *TV_opcion_B        = NULL;
GtkWidget   *BN_extend_B        = NULL;
GtkTextView *TV_opcion_C        = NULL;
GtkWidget   *BN_extend_C        = NULL;
GtkTextView *TV_opcion_D        = NULL;
GtkWidget   *BN_extend_D        = NULL;
GtkTextView *TV_opcion_E        = NULL;
GtkWidget   *BN_extend_E        = NULL;

GtkWidget *EB_opcionA                = NULL;
GtkWidget *EB_opcionB                = NULL;
GtkWidget *EB_opcionC                = NULL;
GtkWidget *EB_opcionD                = NULL;
GtkWidget *EB_opcionE                = NULL;
GtkToggleButton *RB_opcion_A         = NULL;
GtkToggleButton *RB_opcion_B         = NULL;
GtkToggleButton *RB_opcion_C         = NULL;
GtkToggleButton *RB_opcion_D         = NULL;
GtkToggleButton *RB_opcion_E         = NULL;
GtkToggleButton *CK_alfiler_opcion_A = NULL;
GtkImage  *IM_alfiler_opcion_A       = NULL;
GtkToggleButton *CK_alfiler_opcion_B = NULL;
GtkImage  *IM_alfiler_opcion_B       = NULL;
GtkToggleButton *CK_alfiler_opcion_C = NULL;
GtkImage  *IM_alfiler_opcion_C       = NULL;
GtkToggleButton *CK_alfiler_opcion_D = NULL;
GtkImage  *IM_alfiler_opcion_D       = NULL;
GtkToggleButton *CK_alfiler_opcion_E = NULL;
GtkImage  *IM_alfiler_opcion_E    = NULL;
GtkWidget *EB_pregunta            = NULL;
GtkWidget *FR_pregunta            = NULL;
GtkWidget *FR_texto_pregunta      = NULL;
GtkWidget *FR_opcionA             = NULL;
GtkWidget *FR_texto_A             = NULL;
GtkWidget *FR_opcionB             = NULL;
GtkWidget *FR_texto_B             = NULL;
GtkWidget *FR_opcionC             = NULL;
GtkWidget *FR_texto_C             = NULL;
GtkWidget *FR_opcionD             = NULL;
GtkWidget *FR_texto_D             = NULL;
GtkWidget *FR_opcionE             = NULL;
GtkWidget *FR_texto_E             = NULL;
GtkWidget *EN_rpb_A               = NULL;
GtkWidget *EN_rpb_B               = NULL;
GtkWidget *EN_rpb_C               = NULL;
GtkWidget *EN_rpb_D               = NULL;
GtkWidget *EN_rpb_E               = NULL;
GtkWidget *EN_porcentaje_A        = NULL;
GtkWidget *EN_porcentaje_B        = NULL;
GtkWidget *EN_porcentaje_C        = NULL;
GtkWidget *EN_porcentaje_D        = NULL;
GtkWidget *EN_porcentaje_E        = NULL;
GtkImage  *IM_BG_A                = NULL;
GtkImage  *IM_BG_B                = NULL;
GtkImage  *IM_BG_C                = NULL;
GtkImage  *IM_BG_D                = NULL;
GtkImage  *IM_BG_E                = NULL;
GtkImage  *IM_smile_A             = NULL;
GtkImage  *IM_smile_B             = NULL;
GtkImage  *IM_smile_C             = NULL;
GtkImage  *IM_smile_D             = NULL;
GtkImage  *IM_smile_E             = NULL;
GtkImage  *IM_littlesmile_A       = NULL;
GtkImage  *IM_littlesmile_B       = NULL;
GtkImage  *IM_littlesmile_C       = NULL;
GtkImage  *IM_littlesmile_D       = NULL;
GtkImage  *IM_littlesmile_E       = NULL;
GtkImage  *IM_littlealfiler_A     = NULL;
GtkImage  *IM_littlealfiler_B     = NULL;
GtkImage  *IM_littlealfiler_C     = NULL;
GtkImage  *IM_littlealfiler_D     = NULL;
GtkImage  *IM_littlealfiler_E     = NULL;
GtkWidget *PB_A                   = NULL;
GtkWidget *PB_B                   = NULL;
GtkWidget *PB_C                   = NULL;
GtkWidget *PB_D                   = NULL;
GtkWidget *PB_E                   = NULL;
GtkWidget *EN_stat_estudiantes         = NULL;
GtkWidget *EN_examenes            = NULL;
GtkWidget *EN_ultimo_uso          = NULL;
GtkWidget *EN_creacion            = NULL;
GtkWidget *EN_media_examenes      = NULL;
GtkWidget *EN_varianza_examenes   = NULL;
GtkWidget *EN_media_correctos     = NULL;
GtkWidget *EN_media_incorrectos   = NULL;
GtkWidget *EN_alfa_cronbach_con   = NULL;
GtkWidget *EN_alfa_cronbach_sin   = NULL;
GtkWidget *SC_dificultad          = NULL;
GtkWidget *EN_dificultad          = NULL;
GtkWidget *EN_varianza            = NULL;
GtkWidget *EN_stat_discriminacion      = NULL;
GtkWidget *FR_borrar              = NULL;

GtkTextBuffer * buffer_TV_pregunta;
GtkTextBuffer * buffer_TV_opcion_A;
GtkTextBuffer * buffer_TV_opcion_B;
GtkTextBuffer * buffer_TV_opcion_C;
GtkTextBuffer * buffer_TV_opcion_D;
GtkTextBuffer * buffer_TV_opcion_E;

GtkWidget *BN_ok                  = NULL;

GtkWidget *EB_estadisticas           = NULL;

GtkWidget *EB_boletos                = NULL;
GtkWidget *FR_boletos                = NULL;
GtkWidget *EN_stat_dificultad           = NULL;
GtkWidget *EN_stat_dias                 = NULL;
GtkWidget *EN_boletos_dificultad     = NULL;
GtkWidget *EN_boletos_discriminacion = NULL;
GtkWidget *EN_boletos_dias           = NULL;
GtkWidget *EN_boletos_estudiantes    = NULL;
GtkWidget *EN_boletos_base           = NULL;
GtkWidget *EN_boletos_total          = NULL;
GtkWidget *EB_boletos_dificultad     [MAX_TOTAL_BOLETOS];
GtkWidget *FR_boletos_dificultad     [MAX_TOTAL_BOLETOS];
GtkWidget *EB_boletos_discriminacion [MAX_TOTAL_BOLETOS];
GtkWidget *FR_boletos_discriminacion [MAX_TOTAL_BOLETOS];
GtkWidget *EB_boletos_dias           [MAX_TOTAL_BOLETOS];
GtkWidget *FR_boletos_dias           [MAX_TOTAL_BOLETOS];
GtkWidget *EB_boletos_estudiantes    [MAX_TOTAL_BOLETOS];
GtkWidget *FR_boletos_estudiantes    [MAX_TOTAL_BOLETOS];
GtkWidget *EB_boletos_base           [MAX_TOTAL_BOLETOS];
GtkWidget *FR_boletos_base           [MAX_TOTAL_BOLETOS];

GtkWidget *FR_G_dificultad        = NULL;
GtkWidget *FR_G_discriminacion    = NULL;
GtkWidget *DA_dificultad          = NULL;
GtkWidget *DA_discriminacion      = NULL;

GtkWidget *FR_botones             = NULL;
GtkWidget *BN_save                = NULL;
GtkWidget *BN_delete              = NULL;
GtkWidget *BN_copy                = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_terminar            = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_up                  = NULL;
GtkWidget *BN_estadisticas        = NULL;

GtkWidget *BN_confirm_delete      = NULL;
GtkWidget *BN_cancelar            = NULL;
GtkWidget *BN_ok_1                = NULL;
GtkWidget *BN_ok_2                = NULL;
GtkWidget *BN_ok_3                = NULL;

GtkWidget *FR_update              = NULL;
GtkWidget *EB_update              = NULL;
GtkWidget *FR_delete              = NULL;
GtkWidget *EB_delete              = NULL;
GtkWidget *EB_copy                = NULL;
GtkWidget *FR_copy                = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Actualiza_Varianza             ();
void Borra_Registro                 (GtkWidget *widget, gpointer user_data);
void Cambio_Ejercicio               ();
void Cambio_Pregunta_Combo          (GtkWidget *widget, gpointer user_data);
void Carga_Estadisticas             (char * pregunta);
void Carga_Pregunta                 (PGresult *res);
void Clear_Pregunta                 ();
void Codigo_Pregunta_Nuevo          (char * Codigo_Pregunta);
void Copia_Pregunta                 (GtkWidget *widget, gpointer user_data);
void corrige_hilera                 (unsigned char * antes, unsigned char * despues);
void Connect_widgets                ();
int  Count_questions                (char * ejercicio);
void Despliega_Boletos              ();
void Fin_de_Programa                (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                    (GtkWidget *widget, gpointer user_data);
void Genera_comando_actualizacion   (char * PG_command, char respuesta, char * Codigo_pregunta);
void Graba_Registro                 (GtkWidget *widget, gpointer user_data);
void Imprime_Ejercicio              (GtkWidget *widget, gpointer user_data);
void Init_Ejercicio                 ();
void Init_Pregunta                  ();
void Interface_Coloring             ();
void on_BN_delete_clicked           (GtkWidget *widget, gpointer user_data);
void on_BN_estadisticas_clicked     (GtkWidget *widget, gpointer user_data);
void on_BN_print_clicked            (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked         (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked             (GtkWidget *widget, gpointer user_data);
void on_BN_up_clicked               (GtkWidget *widget, gpointer user_data);
void on_BN_save_clicked             (GtkWidget *widget, gpointer user_data);
void on_CB_pregunta_changed         (GtkWidget *widget, gpointer user_data);
void on_CK_alfiler_opcion_A_toggled (GtkWidget *widget, gpointer user_data);
void on_CK_alfiler_opcion_B_toggled (GtkWidget *widget, gpointer user_data);
void on_CK_alfiler_opcion_C_toggled (GtkWidget *widget, gpointer user_data);
void on_CK_alfiler_opcion_D_toggled (GtkWidget *widget, gpointer user_data);
void on_CK_alfiler_opcion_E_toggled (GtkWidget *widget, gpointer user_data);
void on_RB_opcion_A_toggled         (GtkWidget *widget, gpointer user_data);
void on_RB_opcion_B_toggled         (GtkWidget *widget, gpointer user_data);
void on_RB_opcion_C_toggled         (GtkWidget *widget, gpointer user_data);
void on_RB_opcion_D_toggled         (GtkWidget *widget, gpointer user_data);
void on_RB_opcion_E_toggled         (GtkWidget *widget, gpointer user_data);
void on_SC_dificultad_value_changed (GtkWidget *widget, gpointer user_data);
void on_WN_ex2040_destroy           (GtkWidget *widget, gpointer user_data);
void Print_Latex_Buffer ();
void Read_Only_Fields ();
void Reiniciar_Ejercicio            ();
void Reiniciar_Pregunta             ();
void Toggle_alfiler_opcion_A        (GtkWidget *widget, gpointer user_data);
void Toggle_alfiler_opcion_B        (GtkWidget *widget, gpointer user_data);
void Toggle_alfiler_opcion_C        (GtkWidget *widget, gpointer user_data);
void Toggle_alfiler_opcion_D        (GtkWidget *widget, gpointer user_data);
void Toggle_alfiler_opcion_E        (GtkWidget *widget, gpointer user_data);
void Toggle_opcion_A                (GtkWidget *widget, gpointer user_data);
void Toggle_opcion_B                (GtkWidget *widget, gpointer user_data);
void Toggle_opcion_C                (GtkWidget *widget, gpointer user_data);
void Toggle_opcion_D                (GtkWidget *widget, gpointer user_data);
void Toggle_opcion_E                (GtkWidget *widget, gpointer user_data);
void on_DA_dificultad_expose        (GtkWidget *widget, gpointer user_data);
void on_DA_discriminacion_expose    (GtkWidget *widget, gpointer user_data);
void update_N_questions             (char *ejercicio);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define EJERCICIO_SIZE           7
#define CODIGO_SIZE              7
#define PI 3.141592654
#define DA_HEIGHT 143
#define DA_WIDTH  236
#define DA_BEGIN   25
#define CERO (DA_BEGIN + 105)

int stats_displayed=0;
int REGISTRO_YA_EXISTE = 0;
int PREGUNTA_USADA     = 0;  
int Alfiler_Opcion_A = 0;
int Alfiler_Opcion_B = 0;
int Alfiler_Opcion_C = 0;
int Alfiler_Opcion_D = 0;
int Alfiler_Opcion_E = 0;
int Opcion_A_correcta = 1;
int Opcion_B_correcta = 0;
int Opcion_C_correcta = 0;
int Opcion_D_correcta = 0;
int Opcion_E_correcta = 0;

char Codigo_texto    [EJERCICIO_SIZE]; /* Codigo del texto del ejercicio - podría estar compartido por diversos ejercicios de distintas materias */
char Codigo_Pregunta [CODIGO_SIZE];    /* Codigo único de pregunta */
int  N_preguntas=0;

/*----------------------------------------------------------------------------*/

/***************/
/* M A I N ( ) */
/***************/

int main(int argc, char *argv[]) 
{
  GdkColor color;
  int i;

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX2040.glade", &error))
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

	     /* Some fields are READ ONLY */
	     Read_Only_Fields ();

	     /* The magic of color...  */
	     Interface_Coloring ();

             gtk_widget_show (window1);

             Init_Ejercicio ();

             /* start the event loop */
             gtk_main();
	    }
     }

  return 0;
}

void Connect_widgets()
{
   int i;
   char hilera[100];

   window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex2040"));
   window2                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_borrar"));
   window3                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));
   window4                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_delete"));
   window5                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_boletos"));
   window6                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_copy"));

   EB_preguntas           = GTK_WIDGET (gtk_builder_get_object (builder, "EB_preguntas"));
   FR_preguntas           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_preguntas"));

   EB_opciones           = GTK_WIDGET (gtk_builder_get_object (builder, "EB_opciones"));
   FR_opciones           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_opciones"));

   SP_ejercicio           = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_ejercicio"));
   SC_ejercicio           = GTK_WIDGET (gtk_builder_get_object (builder, "SC_ejercicio"));
   EN_descripcion         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));
   EN_materia_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia_descripcion"));
   EN_tema_descripcion    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_tema_descripcion"));
   EN_subtema_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_subtema_descripcion"));
   EN_preguntas           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_preguntas"));
   CB_pregunta            = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_pregunta"));

   EB_pregunta            = GTK_WIDGET (gtk_builder_get_object (builder, "EB_pregunta"));
   FR_pregunta            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_pregunta"));
   FR_texto_pregunta      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_texto_pregunta"));

   EB_opcionA             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_opcionA"));
   EB_opcionB             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_opcionB"));
   EB_opcionC             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_opcionC"));
   EB_opcionD             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_opcionD"));
   EB_opcionE             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_opcionE"));

   FR_opcionA             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_opcionA"));
   FR_texto_A             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_texto_A"));
   FR_opcionB             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_opcionB"));
   FR_texto_B             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_texto_B"));
   FR_opcionC             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_opcionC"));
   FR_texto_C             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_texto_C"));
   FR_opcionD             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_opcionD"));
   FR_texto_D             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_texto_D"));
   FR_opcionE             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_opcionE"));
   FR_texto_E             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_texto_E"));
   IM_BG_A                = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_BG_A"));
   IM_BG_B                = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_BG_B"));
   IM_BG_C                = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_BG_C"));
   IM_BG_D                = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_BG_D"));
   IM_BG_E                = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_BG_E"));
   IM_smile_A             = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile_A"));
   IM_smile_B             = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile_B"));
   IM_smile_C             = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile_C"));
   IM_smile_D             = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile_D"));
   IM_smile_E             = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile_E"));
   IM_littlesmile_A       = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_littlesmile_A"));
   IM_littlesmile_B       = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_littlesmile_B"));
   IM_littlesmile_C       = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_littlesmile_C"));
   IM_littlesmile_D       = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_littlesmile_D"));
   IM_littlesmile_E       = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_littlesmile_E"));
   TV_pregunta            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_pregunta"));
   BN_extend_question     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_question"));
   TV_opcion_A            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_opcion_A"));
   BN_extend_A            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_A"));
   TV_opcion_B            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_opcion_B"));
   BN_extend_B            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_B"));
   TV_opcion_C            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_opcion_C"));
   BN_extend_C            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_C"));
   TV_opcion_D            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_opcion_D"));
   BN_extend_D            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_D"));
   TV_opcion_E            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_opcion_E"));
   BN_extend_E            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_E"));

   RB_opcion_A            = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_opcion_A"));
   RB_opcion_B            = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_opcion_B"));
   RB_opcion_C            = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_opcion_C"));
   RB_opcion_D            = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_opcion_D"));
   RB_opcion_E            = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_opcion_E"));
   CK_alfiler_opcion_A    = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_alfiler_opcion_A"));
   IM_alfiler_opcion_A    = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_alfiler_opcion_A"));
   CK_alfiler_opcion_B    = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_alfiler_opcion_B"));
   IM_alfiler_opcion_B    = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_alfiler_opcion_B"));
   CK_alfiler_opcion_C    = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_alfiler_opcion_C"));
   IM_alfiler_opcion_C    = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_alfiler_opcion_C"));
   CK_alfiler_opcion_D    = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_alfiler_opcion_D"));
   IM_alfiler_opcion_D    = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_alfiler_opcion_D"));
   CK_alfiler_opcion_E    = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_alfiler_opcion_E"));
   IM_alfiler_opcion_E    = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_alfiler_opcion_E"));
   IM_littlealfiler_A    = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_littlealfiler_A"));
   IM_littlealfiler_B    = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_littlealfiler_B"));
   IM_littlealfiler_C    = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_littlealfiler_C"));
   IM_littlealfiler_D    = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_littlealfiler_D"));
   IM_littlealfiler_E    = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_littlealfiler_E"));
   EN_rpb_A               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_rpb_A"));
   EN_rpb_B               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_rpb_B"));
   EN_rpb_C               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_rpb_C"));
   EN_rpb_D               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_rpb_D"));
   EN_rpb_E               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_rpb_E"));
   EN_porcentaje_A        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_porcentaje_A"));
   EN_porcentaje_B        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_porcentaje_B"));
   EN_porcentaje_C        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_porcentaje_C"));
   EN_porcentaje_D        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_porcentaje_D"));
   EN_porcentaje_E        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_porcentaje_E"));
   PB_A                   = GTK_WIDGET (gtk_builder_get_object (builder, "PB_A"));
   PB_B                   = GTK_WIDGET (gtk_builder_get_object (builder, "PB_B"));
   PB_C                   = GTK_WIDGET (gtk_builder_get_object (builder, "PB_C"));
   PB_D                   = GTK_WIDGET (gtk_builder_get_object (builder, "PB_D"));
   PB_E                   = GTK_WIDGET (gtk_builder_get_object (builder, "PB_E"));
   EN_stat_estudiantes         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_stat_estudiantes"));
   EN_examenes            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_examenes"));
   EN_ultimo_uso          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_ultimo_uso"));
   EN_creacion            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_creacion"));
   EN_media_examenes      = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_examenes"));
   EN_varianza_examenes   = GTK_WIDGET (gtk_builder_get_object (builder, "EN_varianza_examenes"));
   EN_media_correctos     = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_correctos"));
   EN_media_incorrectos   = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_incorrectos"));
   EN_alfa_cronbach_con   = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_cronbach_con"));
   EN_alfa_cronbach_sin   = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_cronbach_sin"));
   SC_dificultad          = GTK_WIDGET (gtk_builder_get_object (builder, "SC_dificultad"));
   EN_dificultad          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_dificultad"));
   EN_varianza            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_varianza"));
   EN_stat_discriminacion      = GTK_WIDGET (gtk_builder_get_object (builder, "EN_stat_discriminacion"));
   BN_ok                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
   EB_estadisticas        = GTK_WIDGET (gtk_builder_get_object (builder, "EB_estadisticas"));
   EB_boletos             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_boletos"));
   FR_boletos             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_boletos"));
   EN_stat_dificultad        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_stat_dificultad"));
   EN_stat_dias              = GTK_WIDGET (gtk_builder_get_object (builder, "EN_stat_dias"));
   EN_boletos_dificultad     = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_dificultad"));
   EN_boletos_discriminacion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_discriminacion"));
   EN_boletos_dias           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_dias"));
   EN_boletos_estudiantes    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_estudiantes"));
   EN_boletos_base           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_base"));
   EN_boletos_total          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_total"));
   FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
   BN_save                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
   BN_delete              = GTK_WIDGET (gtk_builder_get_object (builder, "BN_delete"));
   BN_copy                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_copy"));
   BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
   BN_undo                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
   BN_print               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
   BN_up                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_up"));
   BN_estadisticas        = GTK_WIDGET (gtk_builder_get_object (builder, "BN_estadisticas"));

   FR_G_dificultad     = GTK_WIDGET (gtk_builder_get_object (builder, "FR_G_dificultad"));
   FR_G_discriminacion = GTK_WIDGET (gtk_builder_get_object (builder, "FR_G_discriminacion"));
   DA_dificultad       = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dificultad"));
   DA_discriminacion   = GTK_WIDGET (gtk_builder_get_object (builder, "DA_discriminacion"));

   FR_borrar              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_borrar"));
   BN_cancelar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_cancelar"));
   BN_confirm_delete      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_confirm_delete"));
   BN_ok_1                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok_1"));
   EB_update              = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
   FR_update              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));
   BN_ok_2                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok_2"));
   EB_delete              = GTK_WIDGET (gtk_builder_get_object (builder, "EB_delete"));
   FR_delete              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_delete"));
   EB_copy                = GTK_WIDGET (gtk_builder_get_object (builder, "EB_copy"));
   FR_copy                = GTK_WIDGET (gtk_builder_get_object (builder, "FR_copy"));
   BN_ok_3                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok_3"));

   for (i=0; i< MAX_TOTAL_BOLETOS; i++)
       {
        sprintf (hilera, "EB_dif_%02d", i+1);
        EB_boletos_dificultad[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));
        sprintf (hilera, "FR_dif_%02d", i+1);
        FR_boletos_dificultad[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));

        sprintf (hilera, "EB_disc_%02d", i+1);
        EB_boletos_discriminacion[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));
        sprintf (hilera, "FR_disc_%02d", i+1);
        FR_boletos_discriminacion[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));

        sprintf (hilera, "EB_dias_%02d", i+1);
        EB_boletos_dias[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));
        sprintf (hilera, "FR_dias_%02d", i+1);
        FR_boletos_dias[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));

        sprintf (hilera, "EB_dias_%02d", i+1);
        EB_boletos_dias[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));
        sprintf (hilera, "FR_dias_%02d", i+1);
        FR_boletos_dias[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));

        sprintf (hilera, "EB_est_%02d", i+1);
        EB_boletos_estudiantes[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));
        sprintf (hilera, "FR_est_%02d", i+1);
        FR_boletos_estudiantes[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));

        sprintf (hilera, "EB_base_%02d", i+1);
        EB_boletos_base[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));
        sprintf (hilera, "FR_base_%02d", i+1);
        FR_boletos_base[i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));
       }

  focus_chain = NULL;
  focus_chain = g_list_append( focus_chain, TV_pregunta);
  focus_chain = g_list_append( focus_chain, TV_opcion_A);
  focus_chain = g_list_append( focus_chain, TV_opcion_B);
  focus_chain = g_list_append( focus_chain, TV_opcion_C);
  focus_chain = g_list_append( focus_chain, TV_opcion_D);
  focus_chain = g_list_append( focus_chain, TV_opcion_E);

  gtk_container_set_focus_chain( GTK_CONTAINER(FR_pregunta), focus_chain );
  gtk_container_set_focus_chain( GTK_CONTAINER(FR_opcionA),  focus_chain );
  gtk_container_set_focus_chain( GTK_CONTAINER(FR_opcionB),  focus_chain );
  gtk_container_set_focus_chain( GTK_CONTAINER(FR_opcionC),  focus_chain );
  gtk_container_set_focus_chain( GTK_CONTAINER(FR_opcionD),  focus_chain );
  gtk_container_set_focus_chain( GTK_CONTAINER(FR_opcionE),  focus_chain );

  /* Establece buffers para pregunta y opciones */
  buffer_TV_pregunta = gtk_text_view_get_buffer(TV_pregunta);
  buffer_TV_opcion_A = gtk_text_view_get_buffer(TV_opcion_A);
  buffer_TV_opcion_B = gtk_text_view_get_buffer(TV_opcion_B);
  buffer_TV_opcion_C = gtk_text_view_get_buffer(TV_opcion_C);
  buffer_TV_opcion_D = gtk_text_view_get_buffer(TV_opcion_D);
  buffer_TV_opcion_E = gtk_text_view_get_buffer(TV_opcion_E);

  gtk_text_view_set_accepts_tab (TV_pregunta, FALSE);
  gtk_text_view_set_accepts_tab (TV_opcion_A, FALSE);
  gtk_text_view_set_accepts_tab (TV_opcion_B, FALSE);
  gtk_text_view_set_accepts_tab (TV_opcion_C, FALSE);
  gtk_text_view_set_accepts_tab (TV_opcion_D, FALSE);
  gtk_text_view_set_accepts_tab (TV_opcion_E, FALSE);

  g_signal_connect( G_OBJECT( DA_dificultad    ), "expose-event", G_CALLBACK( on_DA_dificultad_expose ),     NULL);
  g_signal_connect( G_OBJECT( DA_discriminacion), "expose-event", G_CALLBACK( on_DA_discriminacion_expose ),     NULL);
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_descripcion        , FALSE);
  gtk_widget_set_can_focus(EN_materia_descripcion, FALSE);
  gtk_widget_set_can_focus(EN_tema_descripcion   , FALSE);
  gtk_widget_set_can_focus(EN_subtema_descripcion, FALSE);
  gtk_widget_set_can_focus(EN_preguntas          , FALSE);
  gtk_widget_set_can_focus(EN_varianza           , FALSE);
  gtk_widget_set_can_focus(EN_rpb_A              , FALSE);
  gtk_widget_set_can_focus(EN_rpb_B              , FALSE);
  gtk_widget_set_can_focus(EN_rpb_C              , FALSE);
  gtk_widget_set_can_focus(EN_rpb_D              , FALSE);
  gtk_widget_set_can_focus(EN_rpb_E              , FALSE);
  gtk_widget_set_can_focus(EN_porcentaje_A       , FALSE);
  gtk_widget_set_can_focus(EN_porcentaje_B       , FALSE);
  gtk_widget_set_can_focus(EN_porcentaje_C       , FALSE);
  gtk_widget_set_can_focus(EN_porcentaje_D       , FALSE);
  gtk_widget_set_can_focus(EN_porcentaje_E       , FALSE);
  gtk_widget_set_can_focus(PB_A                  , FALSE);
  gtk_widget_set_can_focus(PB_B                  , FALSE);
  gtk_widget_set_can_focus(PB_C                  , FALSE);
  gtk_widget_set_can_focus(PB_D                  , FALSE);
  gtk_widget_set_can_focus(PB_E                  , FALSE);
  gtk_widget_set_can_focus(EN_stat_estudiantes   , FALSE);
  gtk_widget_set_can_focus(EN_examenes           , FALSE);
  gtk_widget_set_can_focus(EN_ultimo_uso         , FALSE);
  gtk_widget_set_can_focus(EN_creacion           , FALSE);
  gtk_widget_set_can_focus(EN_media_examenes     , FALSE);
  gtk_widget_set_can_focus(EN_varianza_examenes  , FALSE);
  gtk_widget_set_can_focus(EN_media_correctos    , FALSE);
  gtk_widget_set_can_focus(EN_media_incorrectos  , FALSE);
  gtk_widget_set_can_focus(EN_alfa_cronbach_con  , FALSE);
  gtk_widget_set_can_focus(EN_alfa_cronbach_sin  , FALSE);
  gtk_widget_set_can_focus(EN_stat_discriminacion, FALSE);
  gtk_widget_set_can_focus(EN_dificultad         , FALSE);
  gtk_widget_set_can_focus(EN_stat_dificultad    , FALSE);
  gtk_widget_set_can_focus(EN_stat_dias          , FALSE);
  gtk_widget_set_can_focus(EN_boletos_dias       , FALSE);
  gtk_widget_set_can_focus(EN_boletos_estudiantes, FALSE);
  gtk_widget_set_can_focus(EN_boletos_dificultad , FALSE);
  gtk_widget_set_can_focus(EN_boletos_discriminacion, FALSE);
  gtk_widget_set_can_focus(EN_boletos_base,           FALSE);
  gtk_widget_set_can_focus(EN_boletos_total,          FALSE);
}

void Interface_Coloring ()
{
  GdkColor color;
  int i;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_preguntas, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (SECONDARY_AREA, &color);
  gtk_widget_modify_bg(EB_pregunta,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_opciones,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_estadisticas, GTK_STATE_NORMAL, &color);

  gdk_color_parse (IMPORTANT_WINDOW, &color);
  gtk_widget_modify_bg(window2, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_pregunta,  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_preguntas, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_boletos,   GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones,   GTK_STATE_NORMAL, &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(FR_texto_pregunta,         GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_descripcion,            GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_ejercicio),  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_boletos_dias,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_boletos_dificultad,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_boletos_discriminacion, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_boletos_estudiantes,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_boletos_base,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_boletos_total,          GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_up,              GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,           GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_delete,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_copy,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_estadisticas,    GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok_1,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok_2,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,              GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_confirm_delete,  GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_question, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_A,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_B,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_C,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_D,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_E,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_cancelar,        GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,            GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_up,              GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,           GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_delete,          GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_copy,            GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_estadisticas,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,            GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok_1,            GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok_2,            GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,              GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_confirm_delete,  GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_question, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_A,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_B,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_C,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_D,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_E,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_cancelar,        GTK_STATE_ACTIVE, &color);

  gdk_color_parse (CORRECT_OPTION, &color);
  gtk_widget_modify_bg(EB_opcionA, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (CORRECT_OPTION_FR, &color);
  gtk_widget_modify_bg(FR_opcionA, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_texto_A, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(PB_A,       GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (DISTRACTOR, &color);
  gtk_widget_modify_bg(EB_opcionB, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_opcionC, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_opcionD, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_opcionE, GTK_STATE_NORMAL, &color);

  gdk_color_parse (DISTRACTOR_FR, &color);
  gtk_widget_modify_bg(FR_opcionB, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_opcionC, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_opcionD, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_opcionE, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_texto_B, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_texto_C, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_texto_D, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_texto_E, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(PB_A,       GTK_STATE_PRELIGHT,   &color);
  gtk_widget_modify_bg(PB_B,       GTK_STATE_PRELIGHT,   &color);
  gtk_widget_modify_bg(PB_C,       GTK_STATE_PRELIGHT,   &color);
  gtk_widget_modify_bg(PB_D,       GTK_STATE_PRELIGHT,   &color);
  gtk_widget_modify_bg(PB_E,       GTK_STATE_PRELIGHT,   &color);

  gdk_color_parse (BOLETOS_FR, &color);
  for (i=0; i < MAX_TOTAL_BOLETOS; i++)
      {
       gtk_widget_modify_bg(FR_boletos_dificultad    [i], GTK_STATE_NORMAL, &color);
       gtk_widget_modify_bg(FR_boletos_discriminacion[i], GTK_STATE_NORMAL, &color);
       gtk_widget_modify_bg(FR_boletos_dias          [i], GTK_STATE_NORMAL, &color);
       gtk_widget_modify_bg(FR_boletos_estudiantes   [i], GTK_STATE_NORMAL, &color);
       gtk_widget_modify_bg(FR_boletos_base          [i], GTK_STATE_NORMAL, &color);
      }

  gdk_color_parse (COLOR_BOLETOS_BASE, &color);
  for (i=0; i < MAX_TOTAL_BOLETOS; i++)
      gtk_widget_modify_bg(EB_boletos_base[i], GTK_STATE_NORMAL, &color);

  gdk_color_parse (COLOR_BOLETOS_DIF, &color);
  for (i=0; i < MAX_TOTAL_BOLETOS; i++)
      gtk_widget_modify_bg(EB_boletos_dificultad[i], GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_G_dificultad, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (COLOR_BOLETOS_DISC, &color);
  for (i=0; i < MAX_TOTAL_BOLETOS; i++)
      gtk_widget_modify_bg(EB_boletos_discriminacion[i], GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_G_discriminacion, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (COLOR_BOLETOS_DIAS, &color);
  for (i=0; i < MAX_TOTAL_BOLETOS; i++)
      gtk_widget_modify_bg(EB_boletos_dias[i], GTK_STATE_NORMAL, &color);

  gdk_color_parse (COLOR_BOLETOS_EST, &color);
  for (i=0; i < MAX_TOTAL_BOLETOS; i++)
      gtk_widget_modify_bg(EB_boletos_estudiantes[i], GTK_STATE_NORMAL, &color);

  gdk_color_parse (IMPORTANT_FR, &color);
  gtk_widget_modify_bg(FR_borrar,   GTK_STATE_NORMAL,   &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_delete,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_copy,                GTK_STATE_NORMAL, &color);

  gdk_color_parse (FINISHED_WORK_FR, &color);
  gtk_widget_modify_bg(FR_delete,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_update,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_copy,                GTK_STATE_NORMAL, &color);
}

void Init_Ejercicio()
{
   int i, Last;
   char Ejercicio           [EJERCICIO_SIZE];
   PGresult *res;

   gtk_combo_box_set_active (CB_pregunta, -1);
   for (i=0; i < N_preguntas; i++) gtk_combo_box_remove_text (CB_pregunta, 0);

   N_preguntas = 0;

   Clear_Pregunta ();

   /* Recupera el ejercicio con código más alto - supuestamente este fue el último en ser ingresado */
   res = PQEXEC(DATABASE, "SELECT codigo_ejercicio, texto_ejercicio from BD_ejercicios order by codigo_ejercicio DESC limit 1");
   if (PQntuples(res))
      {
       Last = atoi (PQgetvalue (res, 0, 0));
       strcpy  (Ejercicio,    PQgetvalue (res, 0, 0));
       strcpy  (Codigo_texto, PQgetvalue (res, 0, 1));

       if (Last > 1)
	  {
           gtk_range_set_range (GTK_RANGE(SC_ejercicio),  (gdouble) 1.0, (gdouble) Last);
           gtk_range_set_value (GTK_RANGE(SC_ejercicio),  (gdouble) 0);
           gtk_range_set_value (GTK_RANGE(SC_ejercicio),  (gdouble) Last);

           gtk_widget_set_can_focus    (SC_ejercicio, TRUE);
           gtk_widget_set_sensitive    (SC_ejercicio, 1);
	  }
       else
 	  { /* Solo hay un ejercicio */
           gtk_range_set_range (GTK_RANGE(SC_ejercicio),  (gdouble) 0.0, (gdouble) Last);
           gtk_range_set_value (GTK_RANGE(SC_ejercicio),  (gdouble) 0);
           gtk_range_set_value (GTK_RANGE(SC_ejercicio),  (gdouble) Last);

           gtk_widget_set_can_focus    (SC_ejercicio, FALSE);
           gtk_widget_set_sensitive    (SC_ejercicio, 0);
	  }

       gtk_widget_set_sensitive(BN_undo,   0);
       gtk_widget_set_sensitive(BN_save,   0);
       gtk_widget_set_sensitive(BN_delete, 0);
       gtk_widget_set_sensitive(BN_copy,   0);
       gtk_widget_set_sensitive(BN_print,  0);
       gtk_widget_set_sensitive(BN_up,     0);
       gtk_widget_grab_focus (GTK_WIDGET(SP_ejercicio));
      }
   else
     { /* Este caso no debiera de darse */
       Last = 0;
       sprintf (Ejercicio   ,"000001");
       sprintf (Codigo_texto,"000001");
      }

   PQclear(res);
}

void Clear_Pregunta()
{
   char text       [2];

   gtk_widget_set_sensitive(GTK_WIDGET(CK_alfiler_opcion_A), 0);
   gtk_image_set_from_file (IM_alfiler_opcion_A, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile_A, ".imagenes/smile.png");
   gtk_image_set_from_file (IM_littlesmile_A, ".imagenes/littlesmile.png");
   gtk_image_set_from_file (IM_littlealfiler_A, ".imagenes/empty.png");
   gtk_widget_set_sensitive(GTK_WIDGET(CK_alfiler_opcion_B), 0);
   gtk_image_set_from_file (IM_alfiler_opcion_B, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile_B, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_littlesmile_B, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_littlealfiler_B, ".imagenes/empty.png");
   gtk_widget_set_sensitive(GTK_WIDGET(CK_alfiler_opcion_C), 0);
   gtk_image_set_from_file (IM_alfiler_opcion_C, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile_C, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_littlesmile_C, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_littlealfiler_C, ".imagenes/empty.png");
   gtk_widget_set_sensitive(GTK_WIDGET(CK_alfiler_opcion_D), 0);
   gtk_image_set_from_file (IM_alfiler_opcion_D, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile_D, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_littlesmile_D, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_littlealfiler_D, ".imagenes/empty.png");
   gtk_widget_set_sensitive(GTK_WIDGET(CK_alfiler_opcion_E), 0);
   gtk_image_set_from_file (IM_alfiler_opcion_E, ".imagenes/pin2.png");
   gtk_image_set_from_file (IM_smile_E, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_littlesmile_E, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_littlealfiler_E, ".imagenes/littlepin2.png");

   gtk_entry_set_text(GTK_ENTRY(EN_stat_estudiantes)       , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_examenes)          , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_ultimo_uso)        , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_creacion)          , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_media_examenes)    , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_varianza_examenes) , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_media_correctos)   , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_media_incorrectos) , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_alfa_cronbach_con) , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_alfa_cronbach_sin) , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_rpb_A)             , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_rpb_B)             , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_rpb_C)             , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_rpb_D)             , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_rpb_E)             , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_A)      , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_B)      , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_C)      , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_D)      , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_E)      , "\0");
   gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_A), 0.0);
   gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_B), 0.0);
   gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_C), 0.0);
   gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_D), 0.0);
   gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_E), 0.0);

   gtk_entry_set_text(GTK_ENTRY(EN_dificultad)      , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_varianza)        , "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_stat_discriminacion)  , "\0");

   gtk_range_set_value (GTK_RANGE(SC_dificultad), (gdouble) 0.000);
   gtk_range_set_value (GTK_RANGE(SC_dificultad), (gdouble) 0.675);

   text[0] = '\0';
   gtk_text_buffer_set_text(buffer_TV_pregunta, text, -1);
   gtk_text_buffer_set_text(buffer_TV_opcion_A, text, -1);
   gtk_text_buffer_set_text(buffer_TV_opcion_B, text, -1);
   gtk_text_buffer_set_text(buffer_TV_opcion_C, text, -1);
   gtk_text_buffer_set_text(buffer_TV_opcion_D, text, -1);
   gtk_text_buffer_set_text(buffer_TV_opcion_E, text, -1);

   gtk_widget_set_sensitive(EN_rpb_A              , 0);
   gtk_widget_set_sensitive(EN_rpb_B              , 0);
   gtk_widget_set_sensitive(EN_rpb_C              , 0);
   gtk_widget_set_sensitive(EN_rpb_D              , 0);
   gtk_widget_set_sensitive(EN_rpb_E              , 0);
   gtk_widget_set_sensitive(EN_porcentaje_A       , 0);

   gtk_widget_set_sensitive(EN_porcentaje_B       , 0);
   gtk_widget_set_sensitive(EN_porcentaje_C       , 0);
   gtk_widget_set_sensitive(EN_porcentaje_D       , 0);
   gtk_widget_set_sensitive(EN_porcentaje_E       , 0);
   gtk_widget_set_sensitive(PB_A                  , 0);
   gtk_widget_set_sensitive(PB_B                  , 0);
   gtk_widget_set_sensitive(PB_C                  , 0);
   gtk_widget_set_sensitive(PB_D                  , 0);
   gtk_widget_set_sensitive(PB_E                  , 0);
   gtk_widget_set_sensitive(EN_stat_estudiantes        , 0);
   gtk_widget_set_sensitive(EN_examenes           , 0);
   gtk_widget_set_sensitive(EN_ultimo_uso         , 0);
   gtk_widget_set_sensitive(EN_creacion           , 0);
   gtk_widget_set_sensitive(EN_media_examenes     , 0);
   gtk_widget_set_sensitive(EN_varianza_examenes  , 0);
   gtk_widget_set_sensitive(EN_media_correctos    , 0);
   gtk_widget_set_sensitive(EN_media_incorrectos  , 0);
   gtk_widget_set_sensitive(EN_alfa_cronbach_con  , 0);
   gtk_widget_set_sensitive(EN_alfa_cronbach_sin  , 0);

   gtk_widget_set_sensitive(SC_dificultad    , 0);  
   gtk_widget_set_sensitive(EN_dificultad    , 0);
   gtk_widget_set_sensitive(EN_varianza      , 0);
   gtk_widget_set_sensitive(EN_stat_discriminacion, 0);

   gtk_widget_set_sensitive(GTK_WIDGET(TV_pregunta), 0);
   gtk_widget_set_sensitive(BN_extend_question,      0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_opcion_A), 0);
   gtk_widget_set_sensitive(BN_extend_A,             0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_opcion_B), 0);
   gtk_widget_set_sensitive(BN_extend_B,             0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_opcion_C), 0);
   gtk_widget_set_sensitive(BN_extend_C,             0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_opcion_D), 0);
   gtk_widget_set_sensitive(BN_extend_D,             0);
   gtk_widget_set_sensitive(GTK_WIDGET(TV_opcion_E), 0);
   gtk_widget_set_sensitive(BN_extend_E,             0);
   gtk_widget_set_sensitive(BN_estadisticas,         0);

   gtk_widget_set_sensitive(GTK_WIDGET(RB_opcion_A), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(RB_opcion_B), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(RB_opcion_C), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(RB_opcion_D), 0);
   gtk_widget_set_sensitive(GTK_WIDGET(RB_opcion_E), 0);

   gtk_toggle_button_set_active(CK_alfiler_opcion_A, FALSE);
   gtk_toggle_button_set_active(CK_alfiler_opcion_B, FALSE);
   gtk_toggle_button_set_active(CK_alfiler_opcion_C, FALSE);
   gtk_toggle_button_set_active(CK_alfiler_opcion_D, FALSE);
   gtk_toggle_button_set_active(CK_alfiler_opcion_E, TRUE);


   REGISTRO_YA_EXISTE = 0;
   PREGUNTA_USADA     = 0;
   Alfiler_Opcion_A = 0;
   Alfiler_Opcion_B = 0;
   Alfiler_Opcion_C = 0;
   Alfiler_Opcion_D = 0;
   Alfiler_Opcion_E = 1;

   gtk_toggle_button_set_active(RB_opcion_A, TRUE);

   gtk_widget_set_sensitive(SC_dificultad,    0);
}

void Cambio_Ejercicio ()
{
  long int k;
  int n;
  char codigo[10];
  char PG_command [1000];
  char hilera[10];
  char text   [2];
  PGresult *res, *res_aux;

  k = (long int) gtk_range_get_value (GTK_RANGE(SC_ejercicio));
  sprintf (codigo, "%06ld", k);
  gtk_spin_button_set_value (SP_ejercicio, k);

  sprintf (PG_command,"SELECT materia, tema, subtema, descripcion_ejercicio, texto_ejercicio from BD_ejercicios, BD_texto_ejercicios where codigo_ejercicio = '%s' and texto_ejercicio = consecutivo_texto", codigo);
  res = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res))
     {
      gtk_widget_set_sensitive(BN_ok, 1);

      sprintf (PG_command,"SELECT descripcion_materia from BD_materias where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", 
                          PQgetvalue (res, 0, 0));
      res_aux = PQEXEC(DATABASE, PG_command);
      if (PQntuples(res_aux))
         gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), PQgetvalue (res_aux, 0, 0));
      else
         gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), "*** Materia Inválida ***");
      PQclear(res_aux);

      sprintf (PG_command,"SELECT descripcion_materia from BD_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", 
                          PQgetvalue (res, 0, 0), PQgetvalue (res, 0, 1));
      res_aux = PQEXEC(DATABASE, PG_command);
      if (PQntuples(res_aux))
         gtk_entry_set_text(GTK_ENTRY(EN_tema_descripcion), PQgetvalue (res_aux, 0, 0));
      else
         gtk_entry_set_text(GTK_ENTRY(EN_tema_descripcion), "*** Tema Inválido ***");
      PQclear(res_aux);

      sprintf (PG_command,"SELECT descripcion_materia from BD_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", 
                          PQgetvalue (res, 0, 0), PQgetvalue (res, 0, 1), PQgetvalue (res, 0, 2));
      res_aux = PQEXEC(DATABASE, PG_command);
      if (PQntuples(res_aux))
         gtk_entry_set_text(GTK_ENTRY(EN_subtema_descripcion), PQgetvalue (res_aux, 0, 0));
      else
         gtk_entry_set_text(GTK_ENTRY(EN_subtema_descripcion), "*** Subtema Inválido ***");
      PQclear(res_aux);

      gtk_entry_set_text(GTK_ENTRY(EN_descripcion), PQgetvalue (res, 0, 3));
      strcpy (Codigo_texto,PQgetvalue (res, 0, 4));

      update_N_questions(Codigo_texto);
     }
  else
     {
      gtk_widget_set_sensitive(BN_ok, 0);
      gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), " ");
      gtk_entry_set_text(GTK_ENTRY(EN_tema_descripcion),    " ");
      gtk_entry_set_text(GTK_ENTRY(EN_subtema_descripcion), " ");
      gtk_entry_set_text(GTK_ENTRY(EN_descripcion), "Ejercicio no existe");
     }

  PQclear(res);
}

void update_N_questions (char *ejercicio)
{
  int n;
  char hilera[20];

  n= Count_questions(ejercicio);
  sprintf(hilera,"%3d", n);
  gtk_entry_set_text(GTK_ENTRY(EN_preguntas), hilera);
}

int Count_questions (char * ejercicio)
{
  char PG_command [1000];
  PGresult *res;
  int n;

  sprintf (PG_command,"SELECT secuencia_pregunta from BD_texto_preguntas where codigo_consecutivo_ejercicio = '%s'", ejercicio);
  res = PQEXEC(DATABASE, PG_command);
  n = PQntuples(res);
  PQclear(res);

  return n;
}

void Procesa_Ejercicio()
{
  gtk_widget_set_sensitive(GTK_WIDGET(SC_ejercicio), 0);
  gtk_widget_set_sensitive(BN_ok, 0);

  Init_Pregunta ();
}

void Init_Pregunta ()
{
  gchar * ejercicio;
  char Codigo    [CODIGO_SIZE];
  int  i;
  char PG_command [1000];
  char hilera     [1000];
  PGresult *res;

  ejercicio    = gtk_editable_get_chars(GTK_EDITABLE(SP_ejercicio), 0, -1);

  gtk_combo_box_set_active (CB_pregunta, -1);
  for (i=0; i < N_preguntas; i++) gtk_combo_box_remove_text (CB_pregunta, 0);

  sprintf (PG_command,"SELECT texto_pregunta from BD_Texto_preguntas where codigo_consecutivo_ejercicio = '%s' order by secuencia_pregunta", Codigo_texto);
  res = PQEXEC(DATABASE, PG_command);
  N_preguntas = PQntuples(res);
  for (i=0;i<N_preguntas;i++)
      {
       sprintf (hilera,"%d) %.80s%s", i+1, PQgetvalue (res, i, 0),strlen(PQgetvalue (res, i, 0))>80?"...":" ");
       gtk_combo_box_append_text(CB_pregunta, hilera);
      }
  sprintf (hilera,"%d) *** Pregunta Nueva ***", i+1);
  gtk_combo_box_append_text(CB_pregunta, hilera);

  if (!N_preguntas)
     gtk_widget_set_sensitive(BN_print,  0);
  else
     gtk_widget_set_sensitive(BN_print,  1);
  N_preguntas++;

  gtk_widget_set_sensitive(BN_save,   0);
  gtk_widget_set_sensitive(BN_delete, 0);
  gtk_widget_set_sensitive(BN_copy,   0);
  gtk_widget_set_sensitive(BN_undo,   0);
  gtk_widget_set_sensitive(BN_up  ,   1);

  gtk_widget_set_can_focus(GTK_WIDGET(CB_pregunta), TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(CB_pregunta), 1);
  gtk_widget_grab_focus   (GTK_WIDGET(CB_pregunta));
}

void Cambio_Pregunta_Combo(GtkWidget *widget, gpointer user_data)
{
  int k;
  char PG_command[2000];
  char boletos_label[100];
  PGresult *res, *res_aux;
  int boletos;

  k = gtk_combo_box_get_active(CB_pregunta);

  if (k == N_preguntas - 1)
     { /* Pregunta nueva */
      REGISTRO_YA_EXISTE = 0;
      PREGUNTA_USADA     = 0;
      gtk_widget_set_sensitive(SC_dificultad,    1);
      gtk_widget_set_sensitive(BN_delete, 0);
      gtk_widget_set_sensitive(BN_copy, 0);
      Codigo_Pregunta_Nuevo (Codigo_Pregunta);
     }
  else
     {
      REGISTRO_YA_EXISTE = 1;

      sprintf (PG_command,"SELECT * from BD_texto_preguntas where codigo_consecutivo_ejercicio = '%s' and secuencia_pregunta = %d", Codigo_texto, k+1);

      res = PQEXEC(DATABASE, PG_command);
      Carga_Pregunta (res);
      PQclear(res);

      /* Una pregunta dentro de un ejercicio sólo puede ser eliminada sino es usada en el resto de la base de datos */
      sprintf (PG_command,"SELECT pre_examen from EX_pre_examenes_Lineas where codigo_pregunta = '%s'", Codigo_Pregunta);
      res_aux = PQEXEC(DATABASE, PG_command);
      if (!PQntuples(res_aux))
	 {
          gtk_widget_set_sensitive(BN_delete, 1); 
	 }

      PQclear(res_aux);
     }

  gtk_widget_set_sensitive (GTK_WIDGET(CB_pregunta), 0);

  gtk_widget_set_sensitive(GTK_WIDGET(TV_pregunta), 1);
  gtk_widget_set_sensitive(BN_extend_question,      1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_opcion_A), 1);
  gtk_widget_set_sensitive(BN_extend_A,             1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_opcion_B), 1);
  gtk_widget_set_sensitive(BN_extend_B,             1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_opcion_C), 1);
  gtk_widget_set_sensitive(BN_extend_C,             1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_opcion_D), 1);
  gtk_widget_set_sensitive(BN_extend_D,             1);
  gtk_widget_set_sensitive(GTK_WIDGET(TV_opcion_E), 1);
  gtk_widget_set_sensitive(BN_extend_E,             1);
  gtk_widget_set_sensitive(BN_estadisticas,         1);
  gtk_widget_set_sensitive(BN_copy,                 1);

  gtk_widget_set_sensitive(GTK_WIDGET(RB_opcion_A), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(RB_opcion_B), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(RB_opcion_C), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(RB_opcion_D), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(RB_opcion_E), 1);

  gtk_widget_set_sensitive(GTK_WIDGET(CK_alfiler_opcion_A), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(CK_alfiler_opcion_B), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(CK_alfiler_opcion_C), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(CK_alfiler_opcion_D), 1);
  gtk_widget_set_sensitive(GTK_WIDGET(CK_alfiler_opcion_E), 1);

  gtk_widget_set_sensitive(EN_creacion           , 1);
  gtk_widget_set_sensitive(EN_ultimo_uso         , 1);
  gtk_widget_set_sensitive(EN_dificultad         , 1);
  gtk_widget_set_sensitive(EN_varianza           , 1);

  boletos = asigna_boletos();

  if (!REGISTRO_YA_EXISTE)
     gtk_entry_set_text(GTK_ENTRY(EN_creacion), "NUEVA");

  if (PREGUNTA_USADA)
     {
      gtk_widget_set_sensitive(EN_stat_estudiantes   , 1);
      gtk_widget_set_sensitive(EN_examenes           , 1);
      gtk_widget_set_sensitive(EN_media_examenes     , 1);
      gtk_widget_set_sensitive(EN_varianza_examenes  , 1);
      gtk_widget_set_sensitive(EN_media_correctos    , 1);
      gtk_widget_set_sensitive(EN_media_incorrectos  , 1);
      gtk_widget_set_sensitive(EN_alfa_cronbach_con  , 1);
      gtk_widget_set_sensitive(EN_alfa_cronbach_sin  , 1);

      gtk_widget_set_sensitive(EN_stat_discriminacion, 1);

      gtk_widget_set_sensitive(EN_rpb_A              , 1);
      gtk_widget_set_sensitive(EN_rpb_B              , 1);
      gtk_widget_set_sensitive(EN_rpb_C              , 1);
      gtk_widget_set_sensitive(EN_rpb_D              , 1);
      gtk_widget_set_sensitive(EN_rpb_E              , 1);

      gtk_widget_set_sensitive(EN_porcentaje_A       , 1);
      gtk_widget_set_sensitive(EN_porcentaje_B       , 1);
      gtk_widget_set_sensitive(EN_porcentaje_C       , 1);
      gtk_widget_set_sensitive(EN_porcentaje_D       , 1);
      gtk_widget_set_sensitive(EN_porcentaje_E       , 1);

      gtk_widget_set_sensitive(SC_dificultad,    0);
     }
  else
     {
      gtk_entry_set_text(GTK_ENTRY(EN_ultimo_uso), "NO USADA");
      gtk_widget_set_sensitive(SC_dificultad,    1);
     }

  gtk_widget_set_sensitive(BN_save,   1);
  gtk_widget_set_sensitive(BN_undo,   1);
  gtk_widget_set_sensitive(BN_up  ,   0);
  gtk_widget_grab_focus (GTK_WIDGET(TV_pregunta));
}

void Codigo_Pregunta_Nuevo (char * Codigo_Pregunta)
{
  PGresult *res;
  int Last;

  res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);
  res = PQEXEC(DATABASE, "DECLARE ultima_pregunta CURSOR FOR select codigo_unico_pregunta from BD_texto_preguntas order by codigo_unico_pregunta DESC"); PQclear(res);
  res = PQEXEC(DATABASE, "FETCH 1 in ultima_pregunta");

  if (PQntuples(res))
     {
      Last = atoi (PQgetvalue (res, 0, 0));
      sprintf (Codigo_Pregunta, "%06d", Last + 1);
     }
  else
     strcpy (Codigo_Pregunta, "000001");

  PQclear(res);
  res = PQEXEC(DATABASE, "CLOSE ultima_pregunta");  PQclear(res);
  res = PQEXEC(DATABASE, "END"); PQclear(res);
}

void Carga_Pregunta(PGresult *res)
{
  char Alfileres           [6];
  char Respuesta;
  long double dificultad_estimada, varianza;
  char hilera[20];

  Respuesta = *(PQgetvalue (res, 0, 14));
  if (Respuesta == 'A')  gtk_toggle_button_set_active(RB_opcion_A, TRUE);
  if (Respuesta == 'B')  gtk_toggle_button_set_active(RB_opcion_B, TRUE);
  if (Respuesta == 'C')  gtk_toggle_button_set_active(RB_opcion_C, TRUE);
  if (Respuesta == 'D')  gtk_toggle_button_set_active(RB_opcion_D, TRUE);
  if (Respuesta == 'E')  gtk_toggle_button_set_active(RB_opcion_E, TRUE);

  strcpy (Alfileres,  PQgetvalue (res, 0, 15));
  if (Alfileres[0] == '0')
     {
      gtk_toggle_button_set_active(CK_alfiler_opcion_A, FALSE);
      Alfiler_Opcion_A=0;
     }
  else
     {
      gtk_toggle_button_set_active(CK_alfiler_opcion_A, TRUE);
      Alfiler_Opcion_A=1;
     }

  if (Alfileres[1] == '0')
     {
      gtk_toggle_button_set_active(CK_alfiler_opcion_B, FALSE);
      Alfiler_Opcion_B=0;
     }
  else
     {
      gtk_toggle_button_set_active(CK_alfiler_opcion_B, TRUE);
      Alfiler_Opcion_B=1;
     }

  if (Alfileres[2] == '0')
     {
      gtk_toggle_button_set_active(CK_alfiler_opcion_C, FALSE);
      Alfiler_Opcion_C=0;
     }
  else
     {
      gtk_toggle_button_set_active(CK_alfiler_opcion_C, TRUE);
      Alfiler_Opcion_C=1;
     }

  if (Alfileres[3] == '0')
     {
      gtk_toggle_button_set_active(CK_alfiler_opcion_D, FALSE);
      Alfiler_Opcion_D=0;
     }
  else
     {
      gtk_toggle_button_set_active(CK_alfiler_opcion_D, TRUE);
      Alfiler_Opcion_D=1;
     }

  if (Alfileres[4] == '0')
     {
      gtk_toggle_button_set_active(CK_alfiler_opcion_E, FALSE);
      Alfiler_Opcion_E=0;
     }
  else
     {
      gtk_toggle_button_set_active(CK_alfiler_opcion_E, TRUE);
      Alfiler_Opcion_E=1;
     }

  gtk_text_buffer_set_text(buffer_TV_pregunta, PQgetvalue (res, 0,  8), -1);
  gtk_text_buffer_set_text(buffer_TV_opcion_A, PQgetvalue (res, 0,  9), -1);
  gtk_text_buffer_set_text(buffer_TV_opcion_B, PQgetvalue (res, 0, 10), -1);
  gtk_text_buffer_set_text(buffer_TV_opcion_C, PQgetvalue (res, 0, 11), -1);
  gtk_text_buffer_set_text(buffer_TV_opcion_D, PQgetvalue (res, 0, 12), -1);
  gtk_text_buffer_set_text(buffer_TV_opcion_E, PQgetvalue (res, 0, 13), -1);

  sprintf (hilera,"%02d/%02d/%4d",atoi(PQgetvalue (res, 0, 6)), atoi(PQgetvalue (res, 0, 5)), atoi(PQgetvalue (res, 0, 4)));
  gtk_entry_set_text(GTK_ENTRY(EN_creacion), hilera);

  dificultad_estimada = atof(PQgetvalue (res, 0, 7));
  varianza = dificultad_estimada * (1.0 - dificultad_estimada);

  sprintf (hilera, "%6.3Lf",varianza);
  gtk_entry_set_text(GTK_ENTRY(EN_varianza), hilera); 

  sprintf (hilera, "%6.3Lf",dificultad_estimada);
  gtk_entry_set_text(GTK_ENTRY(EN_dificultad), hilera); 
  gtk_range_set_value (GTK_RANGE(SC_dificultad),(gdouble) dificultad_estimada);

  strcpy (Codigo_Pregunta, PQgetvalue (res, 0, 2));
  Carga_Estadisticas (Codigo_Pregunta);
}

void Carga_Estadisticas (char * pregunta)
{
  PGresult *res;
  char PG_command[1000];
  long double dificultad, varianza, discriminacion, Rpb;
  long double frecuencia_total, frecuencia_opcion;
  long double media_examenes, varianza_examenes;
  long double media_correctos, media_incorrectos;
  long double alfa_cronbach_con, alfa_cronbach_sin;
  long double p;
  int N_estudiantes, N_examenes;
  char hilera[20];

  sprintf (PG_command,"SELECT * from BD_estadisticas_preguntas where pregunta = '%s'", pregunta);
  res = PQEXEC(DATABASE, PG_command);

  N_estudiantes = (long double) atoi(PQgetvalue (res, 0, 6));
  PREGUNTA_USADA = N_estudiantes > 0;

  if (PREGUNTA_USADA)
     {
      sprintf (hilera, "%11d",N_estudiantes);
      gtk_entry_set_text(GTK_ENTRY(EN_stat_estudiantes), hilera); 

      N_examenes    = (long double) atoi(PQgetvalue (res, 0, 5));
      sprintf (hilera, "%11d",N_examenes);
      gtk_entry_set_text(GTK_ENTRY(EN_examenes), hilera);

      sprintf (hilera,"%02d/%02d/%4d",atoi(PQgetvalue (res, 0, 3)), atoi(PQgetvalue (res, 0, 2)), atoi(PQgetvalue (res, 0, 1)));
      gtk_entry_set_text(GTK_ENTRY(EN_ultimo_uso), hilera);
 
      media_examenes = (long double) atof(PQgetvalue (res, 0, 7));
      sprintf (hilera, "%6.3Lf",media_examenes);
      gtk_entry_set_text(GTK_ENTRY(EN_media_examenes), hilera);
 
      varianza_examenes = (long double) atof(PQgetvalue (res, 0, 9));
      sprintf (hilera, "%6.3Lf",varianza_examenes);
      gtk_entry_set_text(GTK_ENTRY(EN_varianza_examenes), hilera); 

      media_correctos = (long double) atof(PQgetvalue (res, 0, 13));
      sprintf (hilera, "%6.3Lf",media_correctos);
      gtk_entry_set_text(GTK_ENTRY(EN_media_correctos), hilera); 

      media_incorrectos = (long double) atof(PQgetvalue (res, 0, 14));
      sprintf (hilera, "%6.3Lf",media_incorrectos);
      gtk_entry_set_text(GTK_ENTRY(EN_media_incorrectos), hilera); 

      alfa_cronbach_con = (long double) atof(PQgetvalue (res, 0, 17));
      sprintf (hilera, "%6.3Lf",alfa_cronbach_con);
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_cronbach_con), hilera); 

      alfa_cronbach_sin = (long double) atof(PQgetvalue (res, 0, 18));
      sprintf (hilera, "%6.3Lf",alfa_cronbach_sin);
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_cronbach_sin), hilera); 

      dificultad    = (long double) atof(PQgetvalue (res, 0, 10));
      sprintf (hilera, "%6.3Lf",dificultad);
      gtk_entry_set_text(GTK_ENTRY(EN_dificultad), hilera); 
      gtk_range_set_value (GTK_RANGE(SC_dificultad),(gdouble) dificultad);

      varianza = (long double) atof(PQgetvalue (res, 0, 11));
      sprintf (hilera, "%6.3Lf",varianza);
      gtk_entry_set_text(GTK_ENTRY(EN_varianza), hilera); 

      discriminacion = (long double) atof(PQgetvalue (res, 0, 12));
      sprintf (hilera, "%6.3Lf",discriminacion);
      gtk_entry_set_text(GTK_ENTRY(EN_stat_discriminacion), hilera);
 
      Rpb = (long double) atof(PQgetvalue (res, 0, 36));
      sprintf (hilera, "%6.3Lf",Rpb);
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_A), hilera);
 
      Rpb = (long double) atof(PQgetvalue (res, 0, 37));
      sprintf (hilera, "%6.3Lf",Rpb);
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_B), hilera); 

      Rpb = (long double) atof(PQgetvalue (res, 0, 38));
      sprintf (hilera, "%6.3Lf",Rpb);
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_C), hilera); 

      Rpb = (long double) atof(PQgetvalue (res, 0, 39));
      sprintf (hilera, "%6.3Lf",Rpb);
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_D), hilera); 

      Rpb = (long double) atof(PQgetvalue (res, 0, 40));
      sprintf (hilera, "%6.3Lf",Rpb);
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_E), hilera); 

      frecuencia_total  = (long double) atof(PQgetvalue (res, 0, 20));

      frecuencia_opcion = (long double) atof(PQgetvalue (res, 0, 21));
      sprintf (hilera, "%7.2Lf",frecuencia_opcion/frecuencia_total*100.0);
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_A), hilera);
      p=frecuencia_opcion/frecuencia_total;
      if (p==1.0)p=0.999;
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_A), (gdouble) p);

      frecuencia_opcion = (long double) atof(PQgetvalue (res, 0, 22));
      sprintf (hilera, "%7.2Lf",frecuencia_opcion/frecuencia_total*100.0);
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_B), hilera);
      p=frecuencia_opcion/frecuencia_total;
      if (p==1.0)p=0.999;
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_B), (gdouble) p);

      frecuencia_opcion = (long double) atof(PQgetvalue (res, 0, 23));
      sprintf (hilera, "%7.2Lf",frecuencia_opcion/frecuencia_total*100.0);
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_C), hilera);
      p=frecuencia_opcion/frecuencia_total;
      if (p==1.0)p=0.999;
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_C), (gdouble) p);

      frecuencia_opcion = (long double) atof(PQgetvalue (res, 0, 24));
      sprintf (hilera, "%7.2Lf",frecuencia_opcion/frecuencia_total*100.0);
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_D), hilera);
      p=frecuencia_opcion/frecuencia_total;
      if (p==1.0)p=0.999;
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_D), (gdouble) p);

      frecuencia_opcion = (long double) atof(PQgetvalue (res, 0, 25));
      sprintf (hilera, "%7.2Lf",frecuencia_opcion/frecuencia_total*100.0);
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_E), hilera);
      p=frecuencia_opcion/frecuencia_total;
      if (p==1.0)p=0.999;
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_E), (gdouble) p);
     }

  Despliega_Boletos();
  PQclear(res);
}

void Graba_Registro (GtkWidget *widget, gpointer user_data)
{
  PGresult *res;
  GtkTextIter start;
  GtkTextIter end;
  int secuencia;
  gchar  *codigo, *comentario;
  gchar *text_pregunta, *text_opcion_A, *text_opcion_B, *text_opcion_C, *text_opcion_D, *text_opcion_E;
  char respuesta;
  char PG_command [32000];
  char *pregunta_corregida;
  char *opcion_A_corregida;
  char *opcion_B_corregida;
  char *opcion_C_corregida;
  char *opcion_D_corregida;
  char *opcion_E_corregida;
  char alfileres_opciones [6] = "00000";
  int N_alfileres;
  long double dificultad, varianza, duracion;
  int N=100;
  time_t curtime;
  struct tm *loctime;
  int month, year, day;
  char hilera[200];

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

  secuencia    = gtk_combo_box_get_active(CB_pregunta)+1;

  gtk_text_buffer_get_start_iter (buffer_TV_pregunta, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_pregunta, &end);
  text_pregunta = gtk_text_buffer_get_text (buffer_TV_pregunta, &start, &end, FALSE);
  pregunta_corregida = (char *) malloc (sizeof(char) * strlen(text_pregunta)*2);
  hilera_POSTGRES (text_pregunta, pregunta_corregida);

  gtk_text_buffer_get_start_iter  (buffer_TV_opcion_A, &start);
  gtk_text_buffer_get_end_iter    (buffer_TV_opcion_A, &end);
  text_opcion_A = gtk_text_buffer_get_text (buffer_TV_opcion_A, &start, &end, FALSE);
  opcion_A_corregida = (char *) malloc (sizeof(char) * strlen(text_opcion_A)*2);
  hilera_POSTGRES (text_opcion_A, opcion_A_corregida);

  gtk_text_buffer_get_start_iter  (buffer_TV_opcion_B, &start);
  gtk_text_buffer_get_end_iter    (buffer_TV_opcion_B, &end);
  text_opcion_B = gtk_text_buffer_get_text (buffer_TV_opcion_B, &start, &end, FALSE);
  opcion_B_corregida = (char *) malloc (sizeof(char) * strlen(text_opcion_B)*2);
  hilera_POSTGRES (text_opcion_B, opcion_B_corregida);

  gtk_text_buffer_get_start_iter  (buffer_TV_opcion_C, &start);
  gtk_text_buffer_get_end_iter    (buffer_TV_opcion_C, &end);
  text_opcion_C = gtk_text_buffer_get_text (buffer_TV_opcion_C, &start, &end, FALSE);
  opcion_C_corregida = (char *) malloc (sizeof(char) * strlen(text_opcion_C)*2);
  hilera_POSTGRES (text_opcion_C, opcion_C_corregida);

  gtk_text_buffer_get_start_iter  (buffer_TV_opcion_D, &start);
  gtk_text_buffer_get_end_iter    (buffer_TV_opcion_D, &end);
  text_opcion_D = gtk_text_buffer_get_text (buffer_TV_opcion_D, &start, &end, FALSE);
  opcion_D_corregida = (char *) malloc (sizeof(char) * strlen(text_opcion_D)*2);
  hilera_POSTGRES (text_opcion_D, opcion_D_corregida);

  gtk_text_buffer_get_start_iter  (buffer_TV_opcion_E, &start);
  gtk_text_buffer_get_end_iter    (buffer_TV_opcion_E, &end);
  text_opcion_E = gtk_text_buffer_get_text (buffer_TV_opcion_E, &start, &end, FALSE);
  opcion_E_corregida = (char *) malloc (sizeof(char) * strlen(text_opcion_E)*2);
  hilera_POSTGRES (text_opcion_E, opcion_E_corregida);

  respuesta = Opcion_A_correcta? 'A':Opcion_B_correcta? 'B':Opcion_C_correcta? 'C':Opcion_D_correcta? 'D':'E';

  alfileres_opciones [0] = Alfiler_Opcion_A? '1':'0';
  alfileres_opciones [1] = Alfiler_Opcion_B? '1':'0';
  alfileres_opciones [2] = Alfiler_Opcion_C? '1':'0';
  alfileres_opciones [3] = Alfiler_Opcion_D? '1':'0';
  alfileres_opciones [4] = Alfiler_Opcion_E? '1':'0';

  N_alfileres = (Alfiler_Opcion_A? 1:0)+(Alfiler_Opcion_B? 1:0)+(Alfiler_Opcion_C? 1:0)+(Alfiler_Opcion_D? 1:0)+(Alfiler_Opcion_E? 1:0);
  if (N_alfileres == 4) strcpy (alfileres_opciones,"11111");

  dificultad = gtk_range_get_value (GTK_RANGE(SC_dificultad));
  varianza   = dificultad * (1 - dificultad);

  if (REGISTRO_YA_EXISTE)
     {
      res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

      sprintf (PG_command,"UPDATE bd_texto_preguntas SET tipo='1', texto_pregunta=E'%s', texto_opcion_A=E'%s',  texto_opcion_B=E'%s',  texto_opcion_C=E'%s',  texto_opcion_D=E'%s',  texto_opcion_E=E'%s', Respuesta='%c', Alfileres_opciones='%.5s' where codigo_consecutivo_ejercicio = '%.6s' and secuencia_pregunta = %d", 
               pregunta_corregida, opcion_A_corregida, opcion_B_corregida, opcion_C_corregida, opcion_D_corregida, opcion_E_corregida,
	       respuesta,alfileres_opciones, Codigo_texto, secuencia);
      res = PQEXEC(DATABASE, PG_command); PQclear(res);

      if (!PREGUNTA_USADA)
	 { /* Pregunta existe pero no ha sido usada. Sólo se actualiza dificultad y varianza */
          sprintf (PG_command,"UPDATE bd_texto_preguntas SET dificultad_estimada=%Lf where codigo_consecutivo_ejercicio = '%.6s' and secuencia_pregunta = %d", 
		   dificultad, Codigo_texto, secuencia);
          res = PQEXEC(DATABASE, PG_command); PQclear(res);
          sprintf (PG_command,"UPDATE bd_estadisticas_preguntas SET dificultad=%Lf, varianza_dificultad=%Lf where pregunta = '%.6s'", 
		   dificultad, varianza, Codigo_Pregunta);
          res = PQEXEC(DATABASE, PG_command); PQclear(res);
	 }
      else
	 { /* Pregunta ya ha sido usada y hay estadísticas asociadas. Se asignan estadísticas asociadas a opción correcta */
	  Genera_comando_actualizacion (PG_command, respuesta, Codigo_Pregunta);
          res = PQEXEC(DATABASE, PG_command); PQclear(res);

	  Cambio_Pregunta_Combo(NULL, NULL);
	 }

      res = PQEXEC(DATABASE, "END"); PQclear(res);
     }
  else
     {
      res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

      sprintf (PG_command,"INSERT into bd_texto_preguntas values ('%.6s',%d,'%.6s','1', %d,%d,%d, %Lf, E'%s',E'%s',E'%s',E'%s',E'%s',E'%s','%c','%.5s')",
	       Codigo_texto, secuencia, Codigo_Pregunta, year, month, day, dificultad,
               pregunta_corregida, opcion_A_corregida, opcion_B_corregida, opcion_C_corregida, opcion_D_corregida, opcion_E_corregida,
	       respuesta, alfileres_opciones);
      res = PQEXEC(DATABASE, PG_command); PQclear(res);

      /* Registro inicial de estadísticas */
      sprintf (PG_command,"INSERT into bd_estadisticas_preguntas values ('%.6s',0,0,0,'00000',0,0,0.0,0.0,0.0, %Lf, %Lf, 0.0, 0.0, 0.0, 0, 0,  0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)",
	       Codigo_Pregunta, dificultad, varianza);
      res = PQEXEC(DATABASE, PG_command); PQclear(res);

      res = PQEXEC(DATABASE, "END"); PQclear(res);

      sprintf (hilera,"%02d/%02d/%4d", day, month, year);
      gtk_entry_set_text(GTK_ENTRY(EN_creacion), hilera);
      gtk_widget_set_sensitive(BN_delete, 1);
      gtk_widget_set_sensitive(BN_copy,   1);
      gtk_widget_set_sensitive(BN_print,  1);

      REGISTRO_YA_EXISTE = 1;
     }
 
  g_free (text_pregunta);
  g_free (text_opcion_A);
  g_free (text_opcion_B);
  g_free (text_opcion_C);
  g_free (text_opcion_D);
  g_free (text_opcion_E);

  free (pregunta_corregida);
  free (opcion_A_corregida);
  free (opcion_B_corregida);
  free (opcion_C_corregida);
  free (opcion_D_corregida);
  free (opcion_E_corregida);

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

  update_N_questions(Codigo_texto);
}

void Genera_comando_actualizacion (char * PG_command, char respuesta, char * Codigo_pregunta)
{
  switch (respuesta)
         {
	  case 'A': sprintf(PG_command,"UPDATE BD_Estadisticas_Preguntas set dificultad= ((1.0 * frecuencia_1) / N_estudiantes),  varianza_dificultad = (((1.0 * frecuencia_1) / N_estudiantes) * (1 - ((1.0 * frecuencia_1) / N_estudiantes))), point_biserial = point_biserial_1, media_correctos = media_ex_1_1, media_incorrectos=media_ex_1_0, N_correctos = frecuencia_1, N_incorrectos = (N_estudiantes - frecuencia_1) where pregunta = '%s'", Codigo_pregunta);
	            break;

	  case 'B': sprintf(PG_command,"UPDATE BD_Estadisticas_Preguntas set dificultad= ((1.0 * frecuencia_2) / N_estudiantes),  varianza_dificultad = (((1.0 * frecuencia_2) / N_estudiantes) * (1 - ((1.0 * frecuencia_2) / N_estudiantes))), point_biserial = point_biserial_2, media_correctos = media_ex_2_1, media_incorrectos=media_ex_2_0, N_correctos = frecuencia_2, N_incorrectos = (N_estudiantes - frecuencia_2) where pregunta = '%s'", Codigo_pregunta);
	            break;

	  case 'C': sprintf(PG_command,"UPDATE BD_Estadisticas_Preguntas set dificultad= ((1.0 * frecuencia_3) / N_estudiantes),  varianza_dificultad = (((1.0 * frecuencia_3) / N_estudiantes) * (1 - ((1.0 * frecuencia_3) / N_estudiantes))), point_biserial = point_biserial_3, media_correctos = media_ex_3_1, media_incorrectos=media_ex_3_0, N_correctos = frecuencia_3, N_incorrectos = (N_estudiantes - frecuencia_3) where pregunta = '%s'", Codigo_pregunta);
	            break;

	  case 'D': sprintf(PG_command,"UPDATE BD_Estadisticas_Preguntas set dificultad= ((1.0 * frecuencia_4) / N_estudiantes),  varianza_dificultad = (((1.0 * frecuencia_4) / N_estudiantes) * (1 - ((1.0 * frecuencia_4) / N_estudiantes))), point_biserial = point_biserial_4, media_correctos = media_ex_4_1, media_incorrectos=media_ex_4_0, N_correctos = frecuencia_4, N_incorrectos = (N_estudiantes - frecuencia_4) where pregunta = '%s'", Codigo_pregunta);
	            break;

	  case 'E': sprintf(PG_command,"UPDATE BD_Estadisticas_Preguntas set dificultad= ((1.0 * frecuencia_5) / N_estudiantes),  varianza_dificultad = (((1.0 * frecuencia_5) / N_estudiantes) * (1 - ((1.0 * frecuencia_5) / N_estudiantes))), point_biserial = point_biserial_5, media_correctos = media_ex_5_1, media_incorrectos=media_ex_5_0, N_correctos = frecuencia_5, N_incorrectos = (N_estudiantes - frecuencia_5) where pregunta = '%s'", Codigo_pregunta);
	            break;

	  default: sprintf(PG_command, "**NO LISTO**");
	           break;
         }
}

void Reiniciar_Pregunta ()
{
  Clear_Pregunta ();
  Init_Pregunta  ();

  gtk_widget_hide (window5);
  stats_displayed = 0;
}


void Copia_Pregunta (GtkWidget *widget, gpointer user_data)
{
  PGresult *res;
  GtkTextIter start;
  GtkTextIter end;
  int secuencia;
  gchar  *codigo, *comentario;
  gchar *text_pregunta, *text_opcion_A, *text_opcion_B, *text_opcion_C, *text_opcion_D, *text_opcion_E;
  char respuesta;
  char PG_command [12000];
  char *pregunta_corregida;
  char *opcion_A_corregida;
  char *opcion_B_corregida;
  char *opcion_C_corregida;
  char *opcion_D_corregida;
  char *opcion_E_corregida;
  char alfileres_opciones [6] = "00000";
  int N_alfileres;
  long double dificultad, varianza, duracion;
  int N=100;
  time_t curtime;
  struct tm *loctime;
  int month, year, day;
  char hilera[200];
  char Codigo_Pregunta [CODIGO_SIZE];    /* Codigo único de pregunta */

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

  secuencia    = Count_questions(Codigo_texto) + 1;

  gtk_text_buffer_get_start_iter (buffer_TV_pregunta, &start);
  gtk_text_buffer_get_end_iter   (buffer_TV_pregunta, &end);
  text_pregunta = gtk_text_buffer_get_text (buffer_TV_pregunta, &start, &end, FALSE);
  pregunta_corregida = (char *) malloc (sizeof(char) * strlen(text_pregunta)*2);
  hilera_POSTGRES (text_pregunta, pregunta_corregida);

  gtk_text_buffer_get_start_iter  (buffer_TV_opcion_A, &start);
  gtk_text_buffer_get_end_iter    (buffer_TV_opcion_A, &end);
  text_opcion_A = gtk_text_buffer_get_text (buffer_TV_opcion_A, &start, &end, FALSE);
  opcion_A_corregida = (char *) malloc (sizeof(char) * strlen(text_opcion_A)*2);
  hilera_POSTGRES (text_opcion_A, opcion_A_corregida);

  gtk_text_buffer_get_start_iter  (buffer_TV_opcion_B, &start);
  gtk_text_buffer_get_end_iter    (buffer_TV_opcion_B, &end);
  text_opcion_B = gtk_text_buffer_get_text (buffer_TV_opcion_B, &start, &end, FALSE);
  opcion_B_corregida = (char *) malloc (sizeof(char) * strlen(text_opcion_B)*2);
  hilera_POSTGRES (text_opcion_B, opcion_B_corregida);

  gtk_text_buffer_get_start_iter  (buffer_TV_opcion_C, &start);
  gtk_text_buffer_get_end_iter    (buffer_TV_opcion_C, &end);
  text_opcion_C = gtk_text_buffer_get_text (buffer_TV_opcion_C, &start, &end, FALSE);
  opcion_C_corregida = (char *) malloc (sizeof(char) * strlen(text_opcion_C)*2);
  hilera_POSTGRES (text_opcion_C, opcion_C_corregida);

  gtk_text_buffer_get_start_iter  (buffer_TV_opcion_D, &start);
  gtk_text_buffer_get_end_iter    (buffer_TV_opcion_D, &end);
  text_opcion_D = gtk_text_buffer_get_text (buffer_TV_opcion_D, &start, &end, FALSE);
  opcion_D_corregida = (char *) malloc (sizeof(char) * strlen(text_opcion_D)*2);
  hilera_POSTGRES (text_opcion_D, opcion_D_corregida);

  gtk_text_buffer_get_start_iter  (buffer_TV_opcion_E, &start);
  gtk_text_buffer_get_end_iter    (buffer_TV_opcion_E, &end);
  text_opcion_E = gtk_text_buffer_get_text (buffer_TV_opcion_E, &start, &end, FALSE);
  opcion_E_corregida = (char *) malloc (sizeof(char) * strlen(text_opcion_E)*2);
  hilera_POSTGRES (text_opcion_E, opcion_E_corregida);

  respuesta = Opcion_A_correcta? 'A':Opcion_B_correcta? 'B':Opcion_C_correcta? 'C':Opcion_D_correcta? 'D':'E';

  alfileres_opciones [0] = Alfiler_Opcion_A? '1':'0';
  alfileres_opciones [1] = Alfiler_Opcion_B? '1':'0';
  alfileres_opciones [2] = Alfiler_Opcion_C? '1':'0';
  alfileres_opciones [3] = Alfiler_Opcion_D? '1':'0';
  alfileres_opciones [4] = Alfiler_Opcion_E? '1':'0';

  N_alfileres = (Alfiler_Opcion_A? 1:0)+(Alfiler_Opcion_B? 1:0)+(Alfiler_Opcion_C? 1:0)+(Alfiler_Opcion_D? 1:0)+(Alfiler_Opcion_E? 1:0);
  if (N_alfileres == 4) strcpy (alfileres_opciones,"11111");

  dificultad = gtk_range_get_value (GTK_RANGE(SC_dificultad));
  varianza   = dificultad * (1 - dificultad);

  Codigo_Pregunta_Nuevo (Codigo_Pregunta);

  res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

  sprintf (PG_command,"INSERT into bd_texto_preguntas values ('%.6s',%d,'%.6s','1', %d,%d,%d, %Lf, E'%s',E'%s',E'%s',E'%s',E'%s',E'%s','%c','%.5s')",
           Codigo_texto, secuencia, Codigo_Pregunta, year, month, day, dificultad,
           pregunta_corregida, opcion_A_corregida, opcion_B_corregida, opcion_C_corregida, opcion_D_corregida, opcion_E_corregida,
           respuesta, alfileres_opciones);
  res = PQEXEC(DATABASE, PG_command); PQclear(res);

  /* Registro inicial de estadísticas */
  sprintf (PG_command,"INSERT into bd_estadisticas_preguntas values ('%.6s',0,0,0,'00000',0,0,0.0,0.0,0.0, %Lf, %Lf, 0.0, 0.0, 0.0, 0, 0,  0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)",
           Codigo_Pregunta, dificultad, varianza);
  res = PQEXEC(DATABASE, PG_command); PQclear(res);

  res = PQEXEC(DATABASE, "END"); PQclear(res);

  g_free (text_pregunta);
  g_free (text_opcion_A);
  g_free (text_opcion_B);
  g_free (text_opcion_C);
  g_free (text_opcion_D);
  g_free (text_opcion_E);

  free (pregunta_corregida);
  free (opcion_A_corregida);
  free (opcion_B_corregida);
  free (opcion_C_corregida);
  free (opcion_D_corregida);
  free (opcion_E_corregida);

  carga_parametros_EXAMINER (&parametros, DATABASE);
  if (parametros.report_update)
     {
      gtk_widget_set_sensitive(window1, 0);
      gtk_widget_show (window6);

      if (parametros.timeout)
         {
          while (gtk_events_pending ()) gtk_main_iteration ();
          catNap(parametros.timeout);
          gtk_widget_hide (window6);
          gtk_widget_set_sensitive(window1, 1);
	 }
     }

  update_N_questions(Codigo_texto);
}

void Reiniciar_Ejercicio ()
{
  Init_Ejercicio ();
}

void Despliega_Boletos()
{
  char hilera [100];
  int i, N, total;
  gchar * dato_previo, *ejercicio;
  float F;
  char rotulo[100];

  N = dias_sin_usar();

  sprintf(hilera,"%d", N);
  gtk_entry_set_text(GTK_ENTRY(EN_stat_dias), hilera); 

  dato_previo = gtk_editable_get_chars(GTK_EDITABLE(EN_dificultad), 0, -1);
  gtk_entry_set_text(GTK_ENTRY(EN_stat_dificultad), dato_previo); 
  g_free(dato_previo);

  total = 0;
  N = boletos_dificultad ();
  sprintf(hilera,"%d", N);
  gtk_entry_set_text(GTK_ENTRY(EN_boletos_dificultad), hilera); 
  total += N;
  for (i=0; i <N                ; i++) gtk_widget_set_visible (EB_boletos_dificultad[i], TRUE);
  for (i=N; i <MAX_TOTAL_BOLETOS; i++) gtk_widget_set_visible (EB_boletos_dificultad[i], FALSE);

  N = boletos_discriminacion ();
  sprintf(hilera,"%d", N);
  gtk_entry_set_text(GTK_ENTRY(EN_boletos_discriminacion), hilera); 
  total += N;
  for (i=0; i <N                ; i++) gtk_widget_set_visible (EB_boletos_discriminacion[i], TRUE);
  for (i=N; i <MAX_TOTAL_BOLETOS; i++) gtk_widget_set_visible (EB_boletos_discriminacion[i], FALSE);

  N = boletos_novedad ();
  sprintf(hilera,"%d", N);
  gtk_entry_set_text(GTK_ENTRY(EN_boletos_dias), hilera); 
  total += N;
  for (i=0; i <N                ; i++) gtk_widget_set_visible (EB_boletos_dias[i], TRUE);
  for (i=N; i <MAX_TOTAL_BOLETOS; i++) gtk_widget_set_visible (EB_boletos_dias[i], FALSE);

  N = boletos_usos ();
  sprintf(hilera,"%d", N);
  gtk_entry_set_text(GTK_ENTRY(EN_boletos_estudiantes), hilera); 
  total += N;
  for (i=0; i <N                ; i++) gtk_widget_set_visible (EB_boletos_estudiantes[i], TRUE);
  for (i=N; i <MAX_TOTAL_BOLETOS; i++) gtk_widget_set_visible (EB_boletos_estudiantes[i], FALSE);

  N = boletos_base ();
  sprintf(hilera,"%d", N);
  gtk_entry_set_text(GTK_ENTRY(EN_boletos_base), hilera); 
  for (i=0; i <N                ; i++) gtk_widget_set_visible (EB_boletos_base[i], TRUE);
  for (i=N; i <MAX_TOTAL_BOLETOS; i++) gtk_widget_set_visible (EB_boletos_base[i], FALSE);
  total += N;

  sprintf(hilera,"%d", total);
  gtk_entry_set_text(GTK_ENTRY(EN_boletos_total), hilera); 
}

void Imprime_Ejercicio (GtkWidget *widget, gpointer user_data)
{
   FILE * Archivo_Latex;
   gchar *ejercicio;
   char PG_command [500];
   int i, N_preguntas;
   char hilera_antes [9000], hilera_despues [9000];
   char Correcta;
   char alfileres [6];
   PGresult *res, *res_aux;
   long double media, varianza, desviacion;

   ejercicio     = gtk_editable_get_chars(GTK_EDITABLE(SP_ejercicio), 0, -1);
 
   Archivo_Latex = fopen ("EX2040.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");
   fprintf (Archivo_Latex, "\\title{Ejercicio %s}\n", ejercicio);
   fprintf (Archivo_Latex, "\\maketitle\n\n");

   fprintf (Archivo_Latex, "\\rule{8.75cm}{5pt}\n\n");

   sprintf (PG_command,"SELECT * from bd_texto_preguntas, bd_estadisticas_preguntas where codigo_consecutivo_ejercicio = '%s' and codigo_unico_pregunta = pregunta order by secuencia_pregunta", Codigo_texto);
   res = PQEXEC(DATABASE, PG_command);
   N_preguntas = PQntuples(res);

   sprintf (PG_command,"SELECT usa_header, texto_header from bd_texto_ejercicios where consecutivo_texto = '%s'", Codigo_texto);

   res_aux = PQEXEC(DATABASE, PG_command);
   if (*PQgetvalue (res_aux, 0, 0) == 't' && N_preguntas > 1)
      {
       fprintf (Archivo_Latex, "\\textbf{Las preguntas de este ejercicio usan la siguiente informaci\\'{o}n}:\n    \n   \n");
       strcpy(hilera_antes,PQgetvalue (res_aux, 0, 1));
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n", hilera_despues);
       fprintf (Archivo_Latex, "\\rule{8.75cm}{1pt}\n");
      }

   fprintf (Archivo_Latex, "\\begin{questions}\n");

   fprintf (Archivo_Latex, "\n\n");
   for (i=0; i<N_preguntas; i++)
       {

	if (*PQgetvalue (res_aux, 0, 0) == 't' && N_preguntas == 1)
	   {
	    fprintf (Archivo_Latex,"\\question ");
	    sprintf (hilera_antes,"%s \n %s", PQgetvalue (res_aux, 0, 1), PQgetvalue (res, i, 8));
            hilera_LATEX (hilera_antes, hilera_despues);
            fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
	   }
	else
	   {
	    fprintf (Archivo_Latex,"\\question ");
	    sprintf (hilera_antes,"%s", PQgetvalue (res, i, 8));
            hilera_LATEX (hilera_antes, hilera_despues);
            fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
	   }

	strcpy (alfileres, PQgetvalue (res, i, 15));

        fprintf (Archivo_Latex, "\\begin{answers}\n");

	fprintf(Archivo_Latex, "\\item ");
	if (alfileres [0] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
	sprintf (hilera_antes," %s", PQgetvalue (res, i, 9));
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s", hilera_despues);
	if (*(PQgetvalue (res, i, 14)) == 'A')
	   {
            fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
            fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
            fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
	   }
        fprintf (Archivo_Latex, "\n\n");

	fprintf(Archivo_Latex, "\\item ");
	if (alfileres [1] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
	sprintf (hilera_antes," %s", PQgetvalue (res, i, 10));
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s", hilera_despues);
	if (*(PQgetvalue (res, i, 14)) == 'B')
	   {
            fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
            fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
            fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
	   }

        fprintf (Archivo_Latex, "\n\n");

	fprintf(Archivo_Latex, "\\item ");
	if (alfileres [2] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
	sprintf (hilera_antes," %s", PQgetvalue (res, i, 11));
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s", hilera_despues);
	if (*(PQgetvalue (res, i, 14)) == 'C')
	   {
            fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
            fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
            fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
	   }
        fprintf (Archivo_Latex, "\n\n");

	fprintf(Archivo_Latex, "\\item ");
	if (alfileres [3] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
	sprintf (hilera_antes," %s", PQgetvalue (res, i, 12));
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s", hilera_despues);
	if (*(PQgetvalue (res, i, 14)) == 'D')
	   {
            fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
            fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
            fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
	   }
        fprintf (Archivo_Latex, "\n\n");

	fprintf(Archivo_Latex, "\\item ");
	if (alfileres [4] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
	sprintf (hilera_antes," %s", PQgetvalue (res, i, 13));
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s", hilera_despues);
	if (*(PQgetvalue (res, i, 14)) == 'E')
	   {
            fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
            fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
            fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
	   }
        fprintf (Archivo_Latex, "\n\n");

        fprintf (Archivo_Latex, "\\end{answers}\n");

	media    = (long double) atof(PQgetvalue (res, i, 26));
	varianza = (long double) atof(PQgetvalue (res, i, 27));
	desviacion = sqrt(varianza);
        fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{Media: \\textbf{%6.4Lf} - Varianza: %6.4Lf - Desviaci\\'{o}n: %6.4Lf}\n\n", media, varianza, desviacion);

        fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
        fprintf (Archivo_Latex, "\n\n");
       }
   fprintf (Archivo_Latex, "\\end{questions}\n\n");

   fprintf (Archivo_Latex, "\\rule{8.75cm}{5pt}\n\n");

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX2040", 1, NULL, 0.0, 0.0, NULL, NULL);

   g_free (ejercicio);
}

void Borra_Registro (GtkWidget *widget, gpointer user_data)
{
   PGresult *res, *res_aux;
   char PG_command [1000];
   int secuencia;
   int i, k, N_mayores;
   char codigo_actual [CODIGO_SIZE];

   secuencia = gtk_combo_box_get_active(CB_pregunta)+1;

   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

   sprintf (PG_command,"DELETE from bd_estadisticas_preguntas where pregunta = '%.6s'", Codigo_Pregunta);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   sprintf (PG_command,"DELETE from bd_texto_preguntas where codigo_unico_pregunta = '%.6s'", Codigo_Pregunta);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   sprintf (PG_command,"SELECT * from bd_texto_preguntas where codigo_consecutivo_ejercicio = '%.6s' and secuencia_pregunta > %d", Codigo_texto, secuencia);
   res = PQEXEC(DATABASE, PG_command);

   N_mayores = PQntuples(res);

   for (i=0; i <N_mayores; i++)
       {
        strcpy (codigo_actual, PQgetvalue (res, i, 2));
	k              = atoi (PQgetvalue (res, i, 1)) - 1;
        sprintf (PG_command,"UPDATE bd_texto_preguntas SET secuencia_pregunta=%d where codigo_unico_pregunta = '%.6s'", 
		 k, codigo_actual);

        res_aux = PQEXEC(DATABASE, PG_command); PQclear(res_aux);
       }

   PQclear(res);

   res = PQEXEC(DATABASE, "END"); PQclear(res);

   Reiniciar_Pregunta ();

   carga_parametros_EXAMINER (&parametros, DATABASE);
   if (parametros.report_update)
      {
       gtk_widget_set_sensitive(window1, 0);
       gtk_widget_show (window4);

       if (parametros.timeout)
	  {
           while (gtk_events_pending ()) gtk_main_iteration ();
           catNap(parametros.timeout);
           gtk_widget_hide (window4);
           gtk_widget_set_sensitive(window1, 1);
	  }
      }

  update_N_questions(Codigo_texto);
}

void Cambia_dificultad ()
{
  int boletos;
  char boletos_label[100];

  boletos = asigna_boletos();

  Actualiza_Varianza ();
}

void Actualiza_Varianza ()
{
  long double dificultad, varianza;
  char hilera[20];

  dificultad = gtk_range_get_value (GTK_RANGE(SC_dificultad));
  sprintf (hilera, "%6.5Lf", dificultad);
  gtk_entry_set_text(GTK_ENTRY(EN_dificultad), hilera); 

  varianza = dificultad * (1 - dificultad);
  sprintf (hilera, "%6.5Lf", varianza);
  gtk_entry_set_text(GTK_ENTRY(EN_varianza), hilera); 
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

int asigna_boletos ()
{
  int boletos;

  boletos = boletos_base          () + 
            boletos_discriminacion() +
            boletos_dificultad    () +
            boletos_novedad       () +
            boletos_usos          ();

  return (boletos);
}

int boletos_base ()
{
  return (parametros.boletos_base);
}

int boletos_dificultad()
{
  gchar * dato;
  long double p;
  int N;

  dato = gtk_editable_get_chars(GTK_EDITABLE(EN_dificultad), 0, -1);
  p = (long double) atof(dato);
  g_free (dato);
 
  N = (parametros.b_dificultad + (int) round(parametros.m_dificultad * p));
  return N;
}

int boletos_discriminacion()
{
  gchar * dato;
  long double Rpb;
  int N;

  dato = gtk_editable_get_chars(GTK_EDITABLE(EN_stat_discriminacion), 0, -1);
  Rpb = (long double) atof(dato);
  g_free (dato);
  if (Rpb < 0.0) Rpb = 0.0;

  N = (parametros.b_discriminacion + (int) round(parametros.m_discriminacion * Rpb));
  return N;
}

int boletos_novedad()
{
  gchar * dato;
  int dias;
  int N;

  dato = gtk_editable_get_chars(GTK_EDITABLE(EN_stat_dias), 0, -1);
  dias = (int) atoi(dato);

  N = (parametros.b_novedad + (int) round(parametros.m_novedad * dias));
  return N;
}

int dias_sin_usar ()
{
  int dias;
#define DIAS_MAXIMO 730
int Dias_Acumulados [12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334}; 
  gchar * dato;
  time_t curtime;
  struct tm *loctime;
  int month, year, day;
  int month_P,year_P, day_P;
  char hilera[50];
  int i;

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

  dato = gtk_editable_get_chars(GTK_EDITABLE(EN_ultimo_uso), 0, -1);

  if (strcmp (dato,"NO USADA") == 0)
     dias = DIAS_MAXIMO;
  else
     {
      day_P   = atoi(dato);
      month_P = atoi(dato+3);
      year_P  = atoi(dato+6);

      dias = days_between (day_P, month_P, year_P, day, month, year);

      if (dias > DIAS_MAXIMO) dias = DIAS_MAXIMO;
     }

  g_free(dato);
  return (dias);
}

int boletos_usos()
{
#define ESTUDIANTES_MAXIMO 200
  gchar * dato;
  long double estudiantes;

  dato = gtk_editable_get_chars(GTK_EDITABLE(EN_stat_estudiantes), 0, -1);
  estudiantes  = atoi(dato);
  g_free (dato);
  if (estudiantes > ESTUDIANTES_MAXIMO) estudiantes = ESTUDIANTES_MAXIMO;

  return (parametros.b_usos + (int) round(parametros.m_usos * estudiantes));
}

void Toggle_alfiler_opcion_A(GtkWidget *widget, gpointer user_data)
{
  Alfiler_Opcion_A = 1 - Alfiler_Opcion_A;
  if (!Alfiler_Opcion_A)
     {
      gtk_image_set_from_file (IM_alfiler_opcion_A, ".imagenes/empty.png");
      gtk_image_set_from_file (IM_littlealfiler_A, ".imagenes/empty.png");
     }
  else
     {
      gtk_image_set_from_file (IM_alfiler_opcion_A, ".imagenes/pin2.png");
      gtk_image_set_from_file (IM_littlealfiler_A, ".imagenes/littlepin2.png");
     }
}

void Toggle_alfiler_opcion_B(GtkWidget *widget, gpointer user_data)
{
  Alfiler_Opcion_B = 1 - Alfiler_Opcion_B;
  if (!Alfiler_Opcion_B)
     {
      gtk_image_set_from_file (IM_alfiler_opcion_B, ".imagenes/empty.png");
      gtk_image_set_from_file (IM_littlealfiler_B, ".imagenes/empty.png");
     }
  else
     {
      gtk_image_set_from_file (IM_alfiler_opcion_B, ".imagenes/pin2.png");
      gtk_image_set_from_file (IM_littlealfiler_B, ".imagenes/littlepin2.png");
     }
}

void Toggle_alfiler_opcion_C(GtkWidget *widget, gpointer user_data)
{
  Alfiler_Opcion_C = 1 - Alfiler_Opcion_C;
  if (!Alfiler_Opcion_C)
     {
      gtk_image_set_from_file (IM_alfiler_opcion_C, ".imagenes/empty.png");
      gtk_image_set_from_file (IM_littlealfiler_C, ".imagenes/empty.png");
     }
  else
     {
      gtk_image_set_from_file (IM_alfiler_opcion_C, ".imagenes/pin2.png");
      gtk_image_set_from_file (IM_littlealfiler_C, ".imagenes/littlepin2.png");
     }
}

void Toggle_alfiler_opcion_D(GtkWidget *widget, gpointer user_data)
{
  Alfiler_Opcion_D = 1 - Alfiler_Opcion_D;
  if (!Alfiler_Opcion_D)
     {
      gtk_image_set_from_file (IM_alfiler_opcion_D, ".imagenes/empty.png");
      gtk_image_set_from_file (IM_littlealfiler_D, ".imagenes/empty.png");
     }
  else
     {
      gtk_image_set_from_file (IM_alfiler_opcion_D, ".imagenes/pin2.png");
      gtk_image_set_from_file (IM_littlealfiler_D, ".imagenes/littlepin2.png");
     }
}

void Toggle_alfiler_opcion_E(GtkWidget *widget, gpointer user_data)
{
  Alfiler_Opcion_E = 1 - Alfiler_Opcion_E;
  if (!Alfiler_Opcion_E)
     {
      gtk_image_set_from_file (IM_alfiler_opcion_E, ".imagenes/empty.png");
      gtk_image_set_from_file (IM_littlealfiler_E, ".imagenes/empty.png");
     }
  else
     {
      gtk_image_set_from_file (IM_alfiler_opcion_E, ".imagenes/pin2.png");
      gtk_image_set_from_file (IM_littlealfiler_E, ".imagenes/littlepin2.png");
     }
}

void Toggle_opcion_A(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  Opcion_A_correcta = 1 - Opcion_A_correcta;

  if (Opcion_A_correcta)
     {
      gdk_color_parse (CORRECT_OPTION, &color);
      gtk_widget_modify_bg(EB_opcionA,   GTK_STATE_NORMAL,   &color);
      gtk_image_set_from_file (IM_smile_A, ".imagenes/smile.png");
      gtk_image_set_from_file (IM_littlesmile_A, ".imagenes/littlesmile.png");

      gdk_color_parse (CORRECT_OPTION_FR, &color);
      gtk_widget_modify_bg(FR_opcionA,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(FR_texto_A,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_A,   GTK_STATE_PRELIGHT,   &color);
     }
  else
     {
      gdk_color_parse (DISTRACTOR, &color);
      gtk_widget_modify_bg(EB_opcionA,   GTK_STATE_NORMAL,   &color);
      gtk_image_set_from_file (IM_smile_A, ".imagenes/empty.png");
      gtk_image_set_from_file (IM_littlesmile_A, ".imagenes/empty.png");

      gdk_color_parse (DISTRACTOR_FR, &color);
      gtk_widget_modify_bg(FR_opcionA,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(FR_texto_A,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_A,   GTK_STATE_PRELIGHT,   &color);
     }
}

void Toggle_opcion_B(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  Opcion_B_correcta = 1 - Opcion_B_correcta;

  if (Opcion_B_correcta)
     {
      gdk_color_parse (CORRECT_OPTION, &color);
      gtk_widget_modify_bg(EB_opcionB,   GTK_STATE_NORMAL,   &color);
      gtk_image_set_from_file (IM_smile_B, ".imagenes/smile.png");
      gtk_image_set_from_file (IM_littlesmile_B, ".imagenes/littlesmile.png");

      gdk_color_parse (CORRECT_OPTION_FR, &color);
      gtk_widget_modify_bg(FR_opcionB,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(FR_texto_B,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_B,   GTK_STATE_PRELIGHT,   &color);
     }
  else
     {
      gdk_color_parse (DISTRACTOR, &color);
      gtk_widget_modify_bg(EB_opcionB,   GTK_STATE_NORMAL,   &color);
      gtk_image_set_from_file (IM_smile_B, ".imagenes/empty.png");
      gtk_image_set_from_file (IM_littlesmile_B, ".imagenes/empty.png");

      gdk_color_parse (DISTRACTOR_FR, &color);
      gtk_widget_modify_bg(FR_opcionB,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(FR_texto_B,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_B,   GTK_STATE_PRELIGHT,   &color);
     }
}

void Toggle_opcion_C(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  Opcion_C_correcta = 1 - Opcion_C_correcta;

  if (Opcion_C_correcta)
     {
      gdk_color_parse (CORRECT_OPTION, &color);
      gtk_widget_modify_bg(EB_opcionC,   GTK_STATE_NORMAL,   &color);
      gtk_image_set_from_file (IM_smile_C, ".imagenes/smile.png");
      gtk_image_set_from_file (IM_littlesmile_C, ".imagenes/littlesmile.png");

      gdk_color_parse (CORRECT_OPTION_FR, &color);
      gtk_widget_modify_bg(FR_opcionC,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(FR_texto_C,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_C,   GTK_STATE_PRELIGHT,   &color);
     }
  else
     {
      gdk_color_parse (DISTRACTOR, &color);
      gtk_widget_modify_bg(EB_opcionC,   GTK_STATE_NORMAL,   &color);
      gtk_image_set_from_file (IM_smile_C, ".imagenes/empty.png");
      gtk_image_set_from_file (IM_littlesmile_C, ".imagenes/empty.png");

      gdk_color_parse (DISTRACTOR_FR, &color);
      gtk_widget_modify_bg(FR_opcionC,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(FR_texto_C,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_C,   GTK_STATE_PRELIGHT,   &color);
     }
}

void Toggle_opcion_D(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  Opcion_D_correcta = 1 - Opcion_D_correcta;

  if (Opcion_D_correcta)
     {
      gdk_color_parse (CORRECT_OPTION, &color);
      gtk_widget_modify_bg(EB_opcionD,   GTK_STATE_NORMAL,   &color);
      gtk_image_set_from_file (IM_smile_D, ".imagenes/smile.png");
      gtk_image_set_from_file (IM_littlesmile_D, ".imagenes/littlesmile.png");

      gdk_color_parse (CORRECT_OPTION_FR, &color);
      gtk_widget_modify_bg(FR_opcionD,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(FR_texto_D,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_D,   GTK_STATE_PRELIGHT,   &color);
     }
  else
     {
      gdk_color_parse (DISTRACTOR, &color);
      gtk_widget_modify_bg(EB_opcionD,   GTK_STATE_NORMAL,   &color);
      gtk_image_set_from_file (IM_smile_D, ".imagenes/empty.png");
      gtk_image_set_from_file (IM_littlesmile_D, ".imagenes/empty.png");

      gdk_color_parse (DISTRACTOR_FR, &color);
      gtk_widget_modify_bg(FR_opcionD,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(FR_texto_D,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_D,   GTK_STATE_PRELIGHT,   &color);
     }
}

void Toggle_opcion_E(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  Opcion_E_correcta = 1 - Opcion_E_correcta;
  if (Opcion_E_correcta)
     {
      gdk_color_parse (CORRECT_OPTION, &color);
      gtk_widget_modify_bg(EB_opcionE,   GTK_STATE_NORMAL,   &color);
      gtk_image_set_from_file (IM_smile_E, ".imagenes/smile.png");
      gtk_image_set_from_file (IM_littlesmile_E, ".imagenes/littlesmile.png");

      gdk_color_parse (CORRECT_OPTION_FR, &color);
      gtk_widget_modify_bg(FR_opcionE,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(FR_texto_E,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_E,   GTK_STATE_PRELIGHT,   &color);
     }
  else
     {
      gdk_color_parse (DISTRACTOR, &color);
      gtk_widget_modify_bg(EB_opcionE,   GTK_STATE_NORMAL,   &color);
      gtk_image_set_from_file (IM_smile_E, ".imagenes/empty.png");
      gtk_image_set_from_file (IM_littlesmile_E, ".imagenes/empty.png");

      gdk_color_parse (DISTRACTOR_FR, &color);
      gtk_widget_modify_bg(FR_opcionE,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(FR_texto_E,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_E,   GTK_STATE_PRELIGHT,   &color);
     }
}


/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex2040_destroy (GtkWidget *widget, gpointer user_data) 
{
  Fin_Ventana (widget, user_data);
}

void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data)
{
  Fin_de_Programa (widget, user_data);
}

void on_BN_undo_clicked(GtkWidget *widget, gpointer user_data)
{
  Reiniciar_Pregunta ();
}

void on_BN_up_clicked(GtkWidget *widget, gpointer user_data)
{
  Reiniciar_Ejercicio ();
}

void on_BN_save_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Registro (widget, user_data);
}

void on_BN_copy_clicked(GtkWidget *widget, gpointer user_data)
{
  Copia_Pregunta (widget, user_data);
}

void on_BN_delete_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_set_sensitive (window1, 0);
  gtk_widget_show          (window2);
  gtk_widget_grab_focus    (BN_cancelar);
}

void on_BN_cancelar_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (BN_print);
}

void on_BN_confirm_delete_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
  Borra_Registro (widget, user_data);
}

void on_BN_print_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Ejercicio (widget, user_data);
}

void on_SC_dificultad_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_dificultad ();
}

void on_CK_alfiler_opcion_A_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_alfiler_opcion_A (widget, user_data);
}
void on_CK_alfiler_opcion_B_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_alfiler_opcion_B (widget, user_data);
}
void on_CK_alfiler_opcion_C_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_alfiler_opcion_C (widget, user_data);
}
void on_CK_alfiler_opcion_D_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_alfiler_opcion_D (widget, user_data);
}
void on_CK_alfiler_opcion_E_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_alfiler_opcion_E (widget, user_data);
}

void on_RB_opcion_A_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_opcion_A (widget, user_data);
}
void on_RB_opcion_B_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_opcion_B (widget, user_data);
}
void on_RB_opcion_C_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_opcion_C (widget, user_data);
}
void on_RB_opcion_D_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_opcion_D (widget, user_data);
}
void on_RB_opcion_E_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_opcion_E (widget, user_data);
}

void on_CB_pregunta_changed(GtkWidget *widget, gpointer user_data)
{
  gchar * pregunta;

  pregunta = gtk_combo_box_get_active_text(CB_pregunta);

  if (pregunta)
     {
      g_free (pregunta);
      Cambio_Pregunta_Combo (widget, user_data);
     }
  else
     g_free (pregunta);
}

void on_SC_ejercicio_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambio_Ejercicio ();
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  Procesa_Ejercicio ();
}

void on_SP_ejercicio_value_changed(GtkWidget *widget, gpointer user_data)
{
  long int k;

  k = (long int) gtk_spin_button_get_value_as_int (SP_ejercicio);
  gtk_range_set_value (GTK_RANGE(SC_ejercicio), (gdouble) k);
}

void on_SP_ejercicio_activate(GtkWidget *widget, gpointer user_data)
{
  long int k;
  gchar *ejercicio;

  ejercicio = gtk_editable_get_chars(GTK_EDITABLE(SP_ejercicio), 0, -1);
  k = atoi(ejercicio);
  g_free(ejercicio);

  gtk_range_set_value (GTK_RANGE(SC_ejercicio), (gdouble) k);
  Procesa_Ejercicio ();
}

void on_BN_ok_1_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window3);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_ok_2_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window4);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_ok_3_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window6);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_estadisticas_clicked(GtkWidget *widget, gpointer user_data)
{
  if (stats_displayed)
     {
      gtk_widget_hide (window5);
     }
  else
     {
      gtk_widget_show (window5);
     }

  stats_displayed = 1 - stats_displayed;
}

void on_BN_extend_question_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * ejercicio;
  int N;
  char window_title [100];
  char root[100];

  ejercicio = gtk_editable_get_chars(GTK_EDITABLE(SP_ejercicio), 0, -1);
  N         = gtk_combo_box_get_active(CB_pregunta)+1;
  sprintf (window_title, "Código LATEX de Pregunta %s.%d", ejercicio, N);
  sprintf (root, "%s%03d", ejercicio, N);

  Display_Latex_Window (window_title, root, buffer_TV_pregunta, window1);
}

void on_BN_extend_A_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * ejercicio;
  int N;
  char window_title [100];
  char root[100];

  ejercicio = gtk_editable_get_chars(GTK_EDITABLE(SP_ejercicio), 0, -1);
  N         = gtk_combo_box_get_active(CB_pregunta)+1;
  sprintf (window_title, "Código LATEX de Opción A de Pregunta %s.%d", ejercicio, N);
  sprintf (root, "%s-%d-A", ejercicio, N);

  Display_Latex_Window (window_title, root, buffer_TV_opcion_A, window1);
}

void on_BN_extend_B_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * ejercicio;
  int N;
  char window_title [100];
  char root[100];

  ejercicio = gtk_editable_get_chars(GTK_EDITABLE(SP_ejercicio), 0, -1);
  N         = gtk_combo_box_get_active(CB_pregunta)+1;
  sprintf (window_title, "Código LATEX de Opción B de Pregunta %s.%d", ejercicio, N);
  sprintf (root, "%s-%d-B", ejercicio, N);

  Display_Latex_Window (window_title, root, buffer_TV_opcion_B, window1);
}

void on_BN_extend_C_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * ejercicio;
  int N;
  char window_title [100];
  char root[100];

  ejercicio = gtk_editable_get_chars(GTK_EDITABLE(SP_ejercicio), 0, -1);
  N         = gtk_combo_box_get_active(CB_pregunta)+1;
  sprintf (window_title, "Código LATEX de Opción C de Pregunta %s.%d", ejercicio, N);
  sprintf (root, "%s-%d-C", ejercicio, N);

  Display_Latex_Window (window_title, root, buffer_TV_opcion_C, window1);
}

void on_BN_extend_D_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * ejercicio;
  int N;
  char window_title [100];
  char root[100];

  ejercicio = gtk_editable_get_chars(GTK_EDITABLE(SP_ejercicio), 0, -1);
  N         = gtk_combo_box_get_active(CB_pregunta)+1;
  sprintf (window_title, "Código LATEX de Opción D de Pregunta %s.%d", ejercicio, N);
  sprintf (root, "%s-%d-D", ejercicio, N);

  Display_Latex_Window (window_title, root, buffer_TV_opcion_D, window1);
}

void on_BN_extend_E_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * ejercicio;
  int N;
  char window_title [100];
  char root[100];

  ejercicio = gtk_editable_get_chars(GTK_EDITABLE(SP_ejercicio), 0, -1);
  N         = gtk_combo_box_get_active(CB_pregunta)+1;
  sprintf (window_title, "Código LATEX de Opción E de Pregunta %s.%d", ejercicio, N);
  sprintf (root, "%s-%d-E", ejercicio, N);

  Display_Latex_Window (window_title, root, buffer_TV_opcion_E, window1);
}

void on_WN_boletos_delete_event(GtkWidget *widget, gpointer user_data)
{
   gtk_widget_hide (window5);

   stats_displayed = 0;
}

void on_DA_dificultad_expose(GtkWidget *widget, gpointer user_data)
{
   gchar * dato;
   long double p, r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   dato = gtk_editable_get_chars(GTK_EDITABLE(EN_stat_dificultad), 0, -1);
   p = (long double) atof(dato);
   g_free (dato);

   r = (1.0 - p) * PI;
   cos_r = cos(r);
   sin_r = sin(r);

   image = cairo_image_surface_create_from_png(".imagenes/dialdif.png");
   cr = gdk_cairo_create(DA_dificultad->window );
  
   //   cairo_set_source_rgb(cr, 0.45, 0.45, 0.45);
   cairo_set_source_rgb(cr, 0.11, 0.11, 0.11);
   cairo_rectangle(cr, 0, 0, DA_WIDTH, DA_HEIGHT);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
   cairo_select_font_face(cr, "sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

   cairo_set_font_size(cr, 11);
   cairo_move_to(cr, 2, 15);
   cairo_show_text(cr, "Dificultad");  

   cairo_set_font_size(cr, 7);
   cairo_move_to(cr, 1, 107 + DA_BEGIN);
   cairo_show_text(cr, "0.0");  
   cairo_move_to(cr, 221, 107 + DA_BEGIN);
   cairo_show_text(cr, "1.0"); 

   cairo_move_to(cr, 112, 4 + DA_BEGIN);
   cairo_show_text(cr, "0.5"); 
   cairo_save(cr); 

   cairo_move_to(cr, 217, 75 + DA_BEGIN);
   cairo_rotate(cr, -0.314159); 
   cairo_show_text(cr, "0.90"); 
   cairo_restore(cr);
   cairo_save(cr); 
   cairo_move_to(cr, 174, 18 + DA_BEGIN);
   cairo_rotate(cr, -1.021017); 
   cairo_show_text(cr, "0.675"); 
   cairo_restore(cr);
   cairo_save(cr); 
   cairo_move_to(cr, 44, 7 + DA_BEGIN);
   cairo_rotate(cr, 0.942478); 
   cairo_show_text(cr, "0.30"); 
   cairo_restore(cr);
   cairo_stroke_preserve(cr);

   cairo_set_source_surface(cr, image, 12, DA_BEGIN);
   cairo_paint( cr );

   cairo_set_line_width (cr, 3);

   // shadow
   cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
   // cuerpo de la aguja
   cairo_set_line_width (cr, 3);
   x = round(cos_r * 68 - sin_r * -3);
   y = round(sin_r * 68 + cos_r * -3);
   x = x + 118;
   y = CERO - y;
   cairo_move_to(cr, 118, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   //   cairo_move_to(cr, 215, 139);
   x = round(cos_r * 88 - sin_r * -4);
   y = round(sin_r * 88 + cos_r * -4);
   x = x + 118;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   //cairo_line_to(cr, 193, 129);
   x = round(cos_r * 66 - sin_r * 6);
   y = round(sin_r * 66 + cos_r * 6);
   x = x + 118;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   //cairo_line_to(cr, 193, 149);
   x = round(cos_r * 66 - sin_r * -14);
   y = round(sin_r * 66 + cos_r * -14);
   x = x + 118;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   //cairo_stroke_preserve(cr);
   cairo_fill(cr);

// cuerpo de la aguja
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_move_to(cr, 118, CERO);
   cairo_arc( cr, 118, CERO, 2, 0, 2 * G_PI );
   x = round(cos_r * 70 - sin_r * 0);
   y = round(sin_r * 70 + cos_r * 0);
   x = x + 118;
   y = CERO - y;
   cairo_move_to(cr, 118, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   //   cairo_move_to(cr, 215, 139);
   x = round(cos_r * 90 - sin_r * 0);
   y = round(sin_r * 90 + cos_r * 0);
   x = x + 118;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   //cairo_line_to(cr, 193, 129);
   x = round(cos_r * 68 - sin_r * 10);
   y = round(sin_r * 68 + cos_r * 10);
   x = x + 118;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   //cairo_line_to(cr, 193, 149);
   x = round(cos_r * 68 - sin_r * -10);
   y = round(sin_r * 68 + cos_r * -10);
   x = x + 118;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   //cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_destroy( cr );
}

void on_DA_discriminacion_expose(GtkWidget *widget, gpointer user_data)
{
   gchar * dato;
   long double p, r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   dato = gtk_editable_get_chars(GTK_EDITABLE(EN_stat_discriminacion), 0, -1);
   p = (long double) atof(dato);
   g_free (dato);

   r = (1 - (p + 1)/2.0) * PI;
   cos_r = cos(r);
   sin_r = sin(r);

   image = cairo_image_surface_create_from_png(".imagenes/dialdisc.png");
   cr = gdk_cairo_create(DA_discriminacion->window );

   cairo_set_source_rgb(cr, 0.11, 0.11, 0.11);
   cairo_rectangle(cr, 0, 0, DA_WIDTH, DA_HEIGHT);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
   cairo_select_font_face(cr, "sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

   cairo_set_font_size(cr, 11);
   cairo_move_to(cr, 2, 15);
   cairo_show_text(cr, "Discriminación");  

   cairo_set_font_size(cr, 6);
   cairo_move_to(cr, 1, 107 + DA_BEGIN);
   cairo_show_text(cr, "-1.0");  
   cairo_move_to(cr, 220, 107 + DA_BEGIN);
   cairo_show_text(cr, "+1.0"); 

   cairo_set_font_size(cr, 7);
   cairo_move_to(cr, 112, 2 + DA_BEGIN);
   cairo_show_text(cr, "0.0"); 
   cairo_save(cr); 
   cairo_move_to(cr, 167, 11 + DA_BEGIN);
   cairo_rotate(cr, -1.09956); 
   cairo_show_text(cr, "0.30"); 
   cairo_restore(cr);
   cairo_stroke_preserve(cr);

   cairo_set_source_surface(cr, image, 12, DA_BEGIN);
   cairo_paint( cr );
   cairo_set_line_width (cr, 3);

   // shadow
   cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
   // cuerpo de la aguja
   cairo_set_line_width (cr, 3);
   x = round(cos_r * 68 - sin_r * -3);
   y = round(sin_r * 68 + cos_r * -3);
   x = x + 118;
   y = CERO - y;
   cairo_move_to(cr, 118, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   //   cairo_move_to(cr, 215, 139);
   x = round(cos_r * 88 - sin_r * -4);
   y = round(sin_r * 88 + cos_r * -4);
   x = x + 118;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   //cairo_line_to(cr, 193, 129);
   x = round(cos_r * 66 - sin_r * 6);
   y = round(sin_r * 66 + cos_r * 6);
   x = x + 118;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   //cairo_line_to(cr, 193, 149);
   x = round(cos_r * 66 - sin_r * -14);
   y = round(sin_r * 66 + cos_r * -14);
   x = x + 118;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   //cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_line_width (cr, 3);
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_move_to(cr, 118, CERO);
   cairo_arc( cr, 118, CERO, 2, 0, 2 * G_PI );

   // cuerpo de la aguja
   cairo_set_line_width (cr, 3);
   x = round(cos_r * 70 - sin_r * 0);
   y = round(sin_r * 70 + cos_r * 0);
   x = x + 118;
   y = CERO - y;
   cairo_move_to(cr, 118, CERO);
   cairo_line_to(cr, x, y);
   cairo_stroke(cr);

   // Punta de la flecha
   //   cairo_move_to(cr, 215, 139);
   x = round(cos_r * 90 - sin_r * 0);
   y = round(sin_r * 90 + cos_r * 0);
   x = x + 118;
   y = CERO - y;
   cairo_move_to(cr, x, y);

   //cairo_line_to(cr, 193, 129);
   x = round(cos_r * 68 - sin_r * 10);
   y = round(sin_r * 68 + cos_r * 10);
   x = x + 118;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   //cairo_line_to(cr, 193, 149);
   x = round(cos_r * 68 - sin_r * -10);
   y = round(sin_r * 68 + cos_r * -10);
   x = x + 118;
   y = CERO - y;
   cairo_line_to(cr, x, y);

   cairo_close_path(cr);
   //cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_destroy( cr );
}
