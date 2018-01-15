/*

Para compilar:

cc -o EX2050 EX2050.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX2050:                                */
/*                                         */
/*    Recorre Preguntas por Materia, Tema  */
/*    y Subtema                            */
/*    The Examiner 0.3                     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    10 de Julio del 2013                 */
/*                                         */
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

GtkWidget *EB_browser             = NULL;
GtkWidget *FR_browser            = NULL;

GtkComboBox *CB_materia           = NULL;
GtkComboBox *CB_tema              = NULL;
GtkComboBox *CB_subtema           = NULL;
GtkImage  *IM_alfiler_materia     = NULL;
GtkImage  *IM_alfiler_tema        = NULL;
GtkImage  *IM_alfiler_subtema     = NULL;

GtkWidget *SC_pregunta            = NULL;
GtkWidget *EN_pregunta            = NULL;
GtkWidget *EN_N_preguntas         = NULL;
GtkWidget *EN_descripcion         = NULL;
GtkWidget *EN_materia_descripcion = NULL;
GtkWidget *EN_tema_descripcion    = NULL;
GtkWidget *EN_subtema_descripcion = NULL;

GtkTextView *TV_pregunta        = NULL;
GtkTextView *TV_opcion_A        = NULL;
GtkTextView *TV_opcion_B        = NULL;
GtkTextView *TV_opcion_C        = NULL;
GtkTextView *TV_opcion_D        = NULL;
GtkTextView *TV_opcion_E        = NULL;
GtkWidget   *BN_extend_question = NULL;
GtkWidget   *BN_extend_A        = NULL;
GtkWidget   *BN_extend_B        = NULL;
GtkWidget   *BN_extend_C        = NULL;
GtkWidget   *BN_extend_D        = NULL;
GtkWidget   *BN_extend_E        = NULL;

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
GtkWidget *EB_opcionA             = NULL;
GtkWidget *FR_opcionA             = NULL;
GtkWidget *FR_texto_A             = NULL;
GtkWidget *EB_opcionB             = NULL;
GtkWidget *FR_opcionB             = NULL;
GtkWidget *FR_texto_B             = NULL;
GtkWidget *EB_opcionC             = NULL;
GtkWidget *FR_opcionC             = NULL;
GtkWidget *FR_texto_C             = NULL;
GtkWidget *EB_opcionD             = NULL;
GtkWidget *FR_opcionD             = NULL;
GtkWidget *FR_texto_D             = NULL;
GtkWidget *EB_opcionE             = NULL;
GtkWidget *FR_opcionE             = NULL;
GtkWidget *FR_texto_E             = NULL;
GtkTextBuffer * buffer_TV_pregunta;
GtkTextBuffer * buffer_TV_opcion_A;
GtkTextBuffer * buffer_TV_opcion_B;
GtkTextBuffer * buffer_TV_opcion_C;
GtkTextBuffer * buffer_TV_opcion_D;
GtkTextBuffer * buffer_TV_opcion_E;

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
GtkImage  *IM_smile_A             = NULL;
GtkImage  *IM_smile_B             = NULL;
GtkImage  *IM_smile_C             = NULL;
GtkImage  *IM_smile_D             = NULL;
GtkImage  *IM_smile_E             = NULL;
GtkWidget *PB_A                   = NULL;
GtkWidget *PB_B                   = NULL;
GtkWidget *PB_C                   = NULL;
GtkWidget *PB_D                   = NULL;
GtkWidget *PB_E                   = NULL;

GtkWidget *EB_estadisticas        = NULL;
GtkWidget *FR_estadisticas        = NULL;

GtkWidget *EN_creacion            = NULL;
GtkWidget *EN_ultimo_uso          = NULL;
GtkWidget *EN_examenes            = NULL;
GtkWidget *EN_media_examenes      = NULL;
GtkWidget *EN_media_correctos     = NULL;
GtkWidget *EN_media_incorrectos   = NULL;
GtkWidget *EN_varianza_examenes   = NULL;
GtkWidget *EN_varianza            = NULL;
GtkWidget *EN_alfa_cronbach_con   = NULL;
GtkWidget *EN_alfa_cronbach_sin   = NULL;

GtkWidget *EN_dias                = NULL;
GtkWidget *EN_estudiantes         = NULL;
GtkWidget *EN_dificultad_2        = NULL;
GtkWidget *EN_discriminacion_2       = NULL;

GtkWidget *EN_boletos_dias           = NULL;
GtkWidget *EB_boletos_dias           [MAX_TOTAL_BOLETOS];
GtkWidget *FR_boletos_dias           [MAX_TOTAL_BOLETOS];
GtkWidget *EN_boletos_estudiantes    = NULL;
GtkWidget *EB_boletos_estudiantes    [MAX_TOTAL_BOLETOS];
GtkWidget *FR_boletos_estudiantes    [MAX_TOTAL_BOLETOS];
GtkWidget *EN_boletos_dificultad     = NULL;
GtkWidget *EB_boletos_dificultad     [MAX_TOTAL_BOLETOS];
GtkWidget *FR_boletos_dificultad     [MAX_TOTAL_BOLETOS];
GtkWidget *EN_boletos_discriminacion = NULL;
GtkWidget *EB_boletos_discriminacion [MAX_TOTAL_BOLETOS];
GtkWidget *FR_boletos_discriminacion [MAX_TOTAL_BOLETOS];
GtkWidget *EN_boletos_base           = NULL;
GtkWidget *EB_boletos_base           [MAX_TOTAL_BOLETOS];
GtkWidget *FR_boletos_base           [MAX_TOTAL_BOLETOS];
GtkWidget *EN_boletos_total          = NULL;

GtkWidget *FR_G_dificultad        = NULL;
GtkWidget *FR_G_discriminacion    = NULL;
GtkWidget *DA_dificultad          = NULL;
GtkWidget *DA_discriminacion      = NULL;

GtkImage  *IM_discriminacion      = NULL;
GtkWidget *EN_discriminacion      = NULL;
GtkImage  *IM_dificultad          = NULL;
GtkWidget *SC_dificultad          = NULL;
GtkWidget *EN_dificultad        = NULL;

GtkWidget *FR_botones             = NULL;
GtkWidget *BN_save                = NULL;
GtkWidget *BN_delete              = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_terminar            = NULL;

GtkWidget *FR_borrar              = NULL;






GtkWidget *FR_sin_datos           = NULL;
GtkLabel  *LB_materia             = NULL;
GtkLabel  *LB_tema                = NULL;
GtkLabel  *LB_subtema             = NULL;




GtkWidget *BN_ok                  = NULL;


GtkWidget *BN_confirm_delete      = NULL;
GtkWidget *BN_cancelar            = NULL;
GtkWidget *EB_delete              = NULL;
GtkWidget *EB_update              = NULL;
GtkWidget *FR_delete              = NULL;
GtkWidget *FR_update              = NULL;
GtkWidget *BN_ok_update           = NULL;
GtkWidget *BN_ok_delete           = NULL;


/***********************/
/* Prototypes          */
/***********************/
void Actualiza_Dificultad           ();
void Actualiza_Pregunta             ();
void Borra_Registro                 (GtkWidget *widget, gpointer user_data);
void Cambio_Materia                 (GtkWidget *widget, gpointer user_data);
void Cambio_Pregunta                ();
void Cambio_Subtema                 (GtkWidget *widget, gpointer user_data);
void Cambio_Tema                    (GtkWidget *widget, gpointer user_data);
void Carga_SC_preguntas             ();
void Clear_Pregunta                 ();
void Connect_widgets                ();
void Despliega_detalle_boletos();
void Extrae_codigo                  (char * hilera, char * codigo);
void Fin_de_Programa                (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                    (GtkWidget *widget, gpointer user_data);
void Genera_comando_actualizacion   (char * PG_command, char respuesta, char * Codigo_pregunta);
void Imprime_Pregunta               ();
void Init_Fields                    ();
void Interface_Coloring             ();
int main                            (int argc, char *argv[]);
void on_BN_cancelar_clicked         (GtkWidget *widget, gpointer user_data);
void on_BN_delete_clicked           (GtkWidget *widget, gpointer user_data);
void on_BN_ok_clicked               (GtkWidget *widget, gpointer user_data);
void on_BN_print_clicked            (GtkWidget *widget, gpointer user_data);
void on_BN_save_clicked             (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked         (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked             (GtkWidget *widget, gpointer user_data);
void on_CB_materia_changed          (GtkWidget *widget, gpointer user_data);
void on_CB_subtema_changed          (GtkWidget *widget, gpointer user_data);
void on_CB_tema_changed             (GtkWidget *widget, gpointer user_data);
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
void on_SC_pregunta_value_changed   (GtkWidget *widget, gpointer user_data);
void on_WN_ex2050_destroy           (GtkWidget *widget, gpointer user_data);
void Read_Only_Fields               ();
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

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

PGresult *res_principal = NULL;

#define EJERCICIO_SIZE           7
#define CODIGO_SIZE              7
#define PI 3.141592654
#define DA_HEIGHT 143
#define DA_WIDTH  236
#define DA_BEGIN   25
#define CERO (DA_BEGIN + 105)
#define CODIGO_VACIO_MATERIA "*** Todas las Materias ***"
#define CODIGO_VACIO_TEMA    "*** Todos los Temas ***"
#define CODIGO_VACIO_SUBTEMA "*** Todos los Subtemas ***"

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

int  N_materias = 0;
int  N_temas    = 0;
int  N_subtemas = 0;
char *color_boletos[] = {"green", "red", "dark green","dark magenta","dark blue","dark red"};

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX2050.glade", &error))
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
	     /* Conecta funciones con eventos de la interfaz */
             gtk_builder_connect_signals (builder, NULL);

	     /* Crea conexión con los campos de la interfaz a ser controlados */
	     Connect_widgets ();

             /* Muchos campos son READ-ONLY */
	     Read_Only_Fields ();

	     /* Un poco de color */
	     Interface_Coloring ();

             /* Despliega ventana principal e inicializa todos los campos */
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
   int i;
   char hilera[100];

   window1                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex2050"));
   window2                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_borrar"));
   window3                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_sin_datos"));
   window4                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_update"));
   window5                = GTK_WIDGET (gtk_builder_get_object (builder, "WN_delete"));

   EB_browser           = GTK_WIDGET (gtk_builder_get_object (builder, "EB_browser"));
   FR_browser           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_browser"));

   CB_materia             = (GtkComboBox *)     GTK_WIDGET (gtk_builder_get_object (builder, "CB_materia"));
   CB_tema                = (GtkComboBox *)     GTK_WIDGET (gtk_builder_get_object (builder, "CB_tema"));
   CB_subtema             = (GtkComboBox *)     GTK_WIDGET (gtk_builder_get_object (builder, "CB_subtema"));
   IM_alfiler_materia     = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_alfiler_materia"));
   IM_alfiler_tema        = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_alfiler_tema"));
   IM_alfiler_subtema     = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_alfiler_subtema"));

   SC_pregunta            = GTK_WIDGET (gtk_builder_get_object (builder, "SC_pregunta"));
   EN_pregunta            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_pregunta"));
   EN_descripcion         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));
   EN_materia_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia_descripcion"));
   EN_tema_descripcion    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_tema_descripcion"));
   EN_subtema_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_subtema_descripcion"));
   EN_N_preguntas         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_N_preguntas"));

   EB_pregunta            = GTK_WIDGET (gtk_builder_get_object (builder, "EB_pregunta"));
   FR_pregunta            = GTK_WIDGET (gtk_builder_get_object (builder, "FR_pregunta"));
   FR_texto_pregunta      = GTK_WIDGET (gtk_builder_get_object (builder, "FR_texto_pregunta"));
   EB_opcionA             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_opcionA"));
   EB_opcionB             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_opcionB"));
   EB_opcionC             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_opcionC"));
   EB_opcionD             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_opcionD"));
   EB_opcionE             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_opcionE"));

   BN_extend_question     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_question"));
   BN_extend_A            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_A"));
   BN_extend_B            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_B"));
   BN_extend_C            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_C"));
   BN_extend_D            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_D"));
   BN_extend_E            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_extend_E"));

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
   IM_smile_A             = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile_A"));
   IM_smile_B             = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile_B"));
   IM_smile_C             = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile_C"));
   IM_smile_D             = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile_D"));
   IM_smile_E             = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile_E"));
   TV_pregunta            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_pregunta"));
   TV_opcion_A            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_opcion_A"));
   TV_opcion_B            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_opcion_B"));
   TV_opcion_C            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_opcion_C"));
   TV_opcion_D            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_opcion_D"));
   TV_opcion_E            = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_opcion_E"));
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
   EN_estudiantes         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_estudiantes"));
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
   EN_varianza            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_varianza"));
   IM_dificultad          = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_dificultad"));
   EN_dificultad          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_dificultad"));
   IM_discriminacion      = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_discriminacion"));
   EN_discriminacion      = GTK_WIDGET (gtk_builder_get_object (builder, "EN_discriminacion"));
   FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
   BN_save                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
   BN_delete              = GTK_WIDGET (gtk_builder_get_object (builder, "BN_delete"));
   BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
   BN_undo                = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
   BN_print               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));

   FR_borrar              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_borrar"));
   BN_cancelar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_cancelar"));
   BN_confirm_delete      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_confirm_delete"));

   FR_sin_datos           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_sin_datos"));
   LB_materia             = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_materia"));
   LB_tema                = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_tema"));
   LB_subtema             = (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_subtema"));
   BN_ok                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
   EB_delete              = GTK_WIDGET (gtk_builder_get_object (builder, "EB_delete"));
   EB_update              = GTK_WIDGET (gtk_builder_get_object (builder, "EB_update"));
   FR_delete              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_delete"));
   FR_update              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_update"));
   BN_ok_update           = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok_update"));
   BN_ok_delete           = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok_delete"));

   EN_dificultad_2        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_dificultad_2"));
   EN_discriminacion_2    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_discriminacion_2"));
   EN_dias                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_dias"));
   EN_boletos_dificultad     = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_dificultad"));
   EN_boletos_discriminacion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_discriminacion"));
   EN_boletos_dias           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_dias"));
   EN_boletos_estudiantes    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_estudiantes"));
   EN_boletos_base           = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_base"));
   EN_boletos_total          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_boletos_total"));
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

   EB_estadisticas     = GTK_WIDGET (gtk_builder_get_object (builder, "EB_estadisticas"));
   FR_estadisticas     = GTK_WIDGET (gtk_builder_get_object (builder, "FR_estadisticas"));
   FR_G_dificultad     = GTK_WIDGET (gtk_builder_get_object (builder, "FR_G_dificultad"));
   FR_G_discriminacion = GTK_WIDGET (gtk_builder_get_object (builder, "FR_G_discriminacion"));
   DA_dificultad       = GTK_WIDGET (gtk_builder_get_object (builder, "DA_dificultad"));
   DA_discriminacion   = GTK_WIDGET (gtk_builder_get_object (builder, "DA_discriminacion"));

   /* Establece buffers para pregunta y opciones */
   buffer_TV_pregunta = gtk_text_view_get_buffer(TV_pregunta);
   buffer_TV_opcion_A = gtk_text_view_get_buffer(TV_opcion_A);
   buffer_TV_opcion_B = gtk_text_view_get_buffer(TV_opcion_B);
   buffer_TV_opcion_C = gtk_text_view_get_buffer(TV_opcion_C);
   buffer_TV_opcion_D = gtk_text_view_get_buffer(TV_opcion_D);
   buffer_TV_opcion_E = gtk_text_view_get_buffer(TV_opcion_E);

  g_signal_connect( G_OBJECT( DA_dificultad    ), "expose-event", G_CALLBACK( on_DA_dificultad_expose ),     NULL);
  g_signal_connect( G_OBJECT( DA_discriminacion), "expose-event", G_CALLBACK( on_DA_discriminacion_expose ),     NULL);
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_pregunta              , FALSE);
  gtk_widget_set_can_focus(EN_N_preguntas           , FALSE);
  gtk_widget_set_can_focus(EN_descripcion           , FALSE);
  gtk_widget_set_can_focus(EN_materia_descripcion   , FALSE);
  gtk_widget_set_can_focus(EN_tema_descripcion      , FALSE);
  gtk_widget_set_can_focus(EN_subtema_descripcion   , FALSE);
  gtk_widget_set_can_focus(EN_varianza              , FALSE);
  gtk_widget_set_can_focus(EN_rpb_A                 , FALSE);
  gtk_widget_set_can_focus(EN_rpb_B                 , FALSE);
  gtk_widget_set_can_focus(EN_rpb_C                 , FALSE);
  gtk_widget_set_can_focus(EN_rpb_D                 , FALSE);
  gtk_widget_set_can_focus(EN_rpb_E                 , FALSE);
  gtk_widget_set_can_focus(EN_porcentaje_A          , FALSE);
  gtk_widget_set_can_focus(EN_porcentaje_B          , FALSE);
  gtk_widget_set_can_focus(EN_porcentaje_C          , FALSE);
  gtk_widget_set_can_focus(EN_porcentaje_D          , FALSE);
  gtk_widget_set_can_focus(EN_porcentaje_E          , FALSE);
  gtk_widget_set_can_focus(PB_A                     , FALSE);
  gtk_widget_set_can_focus(PB_B                     , FALSE);
  gtk_widget_set_can_focus(PB_C                     , FALSE);
  gtk_widget_set_can_focus(PB_D                     , FALSE);
  gtk_widget_set_can_focus(PB_E                     , FALSE);
  gtk_widget_set_can_focus(EN_estudiantes           , FALSE);
  gtk_widget_set_can_focus(EN_examenes              , FALSE);
  gtk_widget_set_can_focus(EN_ultimo_uso            , FALSE);
  gtk_widget_set_can_focus(EN_creacion              , FALSE);
  gtk_widget_set_can_focus(EN_media_examenes        , FALSE);
  gtk_widget_set_can_focus(EN_varianza_examenes     , FALSE);
  gtk_widget_set_can_focus(EN_media_correctos       , FALSE);
  gtk_widget_set_can_focus(EN_media_incorrectos     , FALSE);
  gtk_widget_set_can_focus(EN_alfa_cronbach_con     , FALSE);
  gtk_widget_set_can_focus(EN_alfa_cronbach_sin     , FALSE);
  gtk_widget_set_can_focus(EN_discriminacion        , FALSE);
  gtk_widget_set_can_focus(EN_dificultad            , FALSE);
  gtk_widget_set_can_focus(EN_dias                  , FALSE);
  gtk_widget_set_can_focus(EN_discriminacion_2      , FALSE);
  gtk_widget_set_can_focus(EN_dificultad_2          , FALSE);
  gtk_widget_set_can_focus(EN_boletos_dias          , FALSE);
  gtk_widget_set_can_focus(EN_boletos_estudiantes   , FALSE);
  gtk_widget_set_can_focus(EN_boletos_discriminacion, FALSE);
  gtk_widget_set_can_focus(EN_boletos_dificultad    , FALSE);
  gtk_widget_set_can_focus(EN_boletos_base          , FALSE);
  gtk_widget_set_can_focus(EN_boletos_total         , FALSE);
}

void Interface_Coloring ()
{
  GdkColor color;
  int i;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_browser, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_browser,      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_estadisticas, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones,      GTK_STATE_NORMAL, &color);

  gdk_color_parse (SECONDARY_AREA, &color);
  gtk_widget_modify_bg(EB_estadisticas, GTK_STATE_NORMAL, &color);

  gdk_color_parse (IMPORTANT_WINDOW, &color);
  gtk_widget_modify_bg(window2, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(window3, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(FR_G_dificultad,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_G_discriminacion, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_pregunta, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(EN_descripcion,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(BN_terminar,  GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,      GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_delete,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,      GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(EN_boletos_dificultad,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_boletos_discriminacion, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_boletos_dias,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_boletos_estudiantes,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_boletos_base,           GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EN_boletos_total,           GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_delete,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,          GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,      GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_cancelar,      GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,            GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_confirm_delete,GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok_update,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok_delete,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_question, GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_A,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_B,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_C,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_D,        GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_extend_E,        GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,          GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_delete,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,         GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,          GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar,      GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_cancelar,      GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,            GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_confirm_delete,GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok_update,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok_delete,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_question, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_A,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_B,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_C,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_D,        GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_extend_E,        GTK_STATE_ACTIVE, &color);

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

  gdk_color_parse (COLOR_BOLETOS_DISC, &color);
  for (i=0; i < MAX_TOTAL_BOLETOS; i++)
      gtk_widget_modify_bg(EB_boletos_discriminacion[i], GTK_STATE_NORMAL, &color);

  gdk_color_parse (COLOR_BOLETOS_DIAS, &color);
  for (i=0; i < MAX_TOTAL_BOLETOS; i++)
      gtk_widget_modify_bg(EB_boletos_dias[i], GTK_STATE_NORMAL, &color);

  gdk_color_parse (COLOR_BOLETOS_EST, &color);
  for (i=0; i < MAX_TOTAL_BOLETOS; i++)
      gtk_widget_modify_bg(EB_boletos_estudiantes[i], GTK_STATE_NORMAL, &color);

  gdk_color_parse (QUESTION, &color);
  gtk_widget_modify_bg(EB_pregunta,       GTK_STATE_NORMAL, &color);

  gdk_color_parse (FR_QUESTION, &color);
  gtk_widget_modify_bg(FR_pregunta,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_texto_pregunta, GTK_STATE_NORMAL, &color);

  gdk_color_parse (CORRECT_OPTION, &color);
  gtk_widget_modify_bg(EB_opcionA, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (CORRECT_OPTION_FR, &color);
  gtk_widget_modify_bg(FR_opcionA, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_texto_A, GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(PB_A,       GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (DISTRACTOR, &color);
  gtk_widget_modify_bg(EB_opcionB,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(EB_opcionC,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(EB_opcionD,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(EB_opcionE,   GTK_STATE_NORMAL,   &color);

  gdk_color_parse (DISTRACTOR_FR, &color);
  gtk_widget_modify_bg(FR_opcionB,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_opcionC,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_opcionD,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_opcionE,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(PB_B,   GTK_STATE_PRELIGHT,   &color);
  gtk_widget_modify_bg(PB_C,   GTK_STATE_PRELIGHT,   &color);
  gtk_widget_modify_bg(PB_D,   GTK_STATE_PRELIGHT,   &color);
  gtk_widget_modify_bg(PB_E,   GTK_STATE_PRELIGHT,   &color);
  gtk_widget_modify_bg(FR_texto_B,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_texto_C,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_texto_D,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_texto_E,   GTK_STATE_NORMAL,   &color);

  gdk_color_parse (IMPORTANT_FR, &color);
  gtk_widget_modify_bg(FR_borrar,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_sin_datos,   GTK_STATE_NORMAL,   &color);

  gdk_color_parse (FINISHED_WORK_FR, &color);
  gtk_widget_modify_bg(FR_delete,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_update,              GTK_STATE_NORMAL, &color);

  gdk_color_parse (FINISHED_WORK_WINDOW, &color);
  gtk_widget_modify_bg(EB_update,              GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(EB_delete,              GTK_STATE_NORMAL, &color);

  gdk_color_parse ("blue", &color);
  gtk_widget_modify_bg(EN_ultimo_uso,     GTK_STATE_NORMAL, &color);
}

void Init_Fields()
{
   int i, Last;
   char Ejercicio           [EJERCICIO_SIZE];
   PGresult *res;
   char hilera[500];

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

   gtk_combo_box_append_text(CB_materia, CODIGO_VACIO_MATERIA);

   res = PQEXEC(DATABASE,"SELECT codigo_materia, descripcion_materia from bd_materias where codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_materia"); 
   N_materias = PQntuples(res)+1;
   for (i=1; i < N_materias; i++)
       {
	sprintf (hilera,"%s %s",PQgetvalue (res, i-1, 0), PQgetvalue (res, i-1, 1));
        gtk_combo_box_append_text(CB_materia, hilera);
       }
   PQclear(res);

   Clear_Pregunta ();

   gtk_combo_box_set_active (CB_materia, 0);

   gtk_image_set_from_file (IM_alfiler_materia, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_alfiler_tema,    ".imagenes/empty.png");
   gtk_image_set_from_file (IM_alfiler_subtema, ".imagenes/empty.png");
}

void Cambio_Materia(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *materia;
  char PG_command  [1000];
  PGresult *res;
  char hilera [300];
  char Codigo_materia [CODIGO_MATERIA_SIZE + 1];
  GdkColor color;

  materia = gtk_combo_box_get_active_text(CB_materia);
  Extrae_codigo (materia, Codigo_materia);

  gtk_combo_box_set_active (CB_tema, -1);
  for (i=0; i<N_temas; i++)
      {
       gtk_combo_box_remove_text (CB_tema, 0);
      }

  if (strcmp(materia,CODIGO_VACIO_MATERIA) != 0)
     {
      gtk_combo_box_append_text(CB_tema, CODIGO_VACIO_TEMA);
      sprintf (PG_command,"SELECT codigo_tema, descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema != '          ' and codigo_subtema = '          ' order by orden", Codigo_materia); 
      res = PQEXEC(DATABASE, PG_command);
      N_temas = PQntuples(res)+1;
      for (i=1; i < N_temas; i++)
          {
	   sprintf (hilera,"%s %s", PQgetvalue (res, i-1, 0), PQgetvalue (res, i-1, 1));
           gtk_combo_box_append_text(CB_tema, hilera);
          }
      gtk_combo_box_set_active (CB_tema, 0);

      PQclear(res);

      gtk_widget_set_sensitive (GTK_WIDGET(CB_tema), 1);
      gtk_image_set_from_file (IM_alfiler_materia, ".imagenes/pin2.png");
      gdk_color_parse ("red", &color);
      gtk_widget_modify_bg(EN_materia_descripcion,  GTK_STATE_NORMAL,   &color);
     }
  else
     {
      N_temas = 1;
      gtk_combo_box_append_text(CB_tema, CODIGO_VACIO_TEMA);
      gtk_combo_box_set_active (CB_tema   , 0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_tema),    0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_subtema), 0);
      gtk_image_set_from_file (IM_alfiler_materia, ".imagenes/empty.png");
      gdk_color_parse ("gray", &color);
      gtk_widget_modify_bg(EN_materia_descripcion,  GTK_STATE_NORMAL,   &color);
     }

  g_free (materia);
}

void Cambio_Tema(GtkWidget *widget, gpointer user_data)
{
  int i;
  gchar *materia, *tema = NULL;
  char PG_command  [1000];
  PGresult *res;
  char hilera [300];
  char Codigo_materia [CODIGO_MATERIA_SIZE + 1];
  char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
  GdkColor color;

  materia = gtk_combo_box_get_active_text(CB_materia);
  tema    = gtk_combo_box_get_active_text(CB_tema);

  Extrae_codigo (materia, Codigo_materia);
  Extrae_codigo (tema,    Codigo_tema);

  gtk_combo_box_set_active(CB_subtema, -1);
  for (i=0; i<N_subtemas; i++)
      {
       gtk_combo_box_remove_text (CB_subtema, 0);
      }

  if (strcmp(tema, CODIGO_VACIO_TEMA) != 0)
     {
      gtk_combo_box_append_text(CB_subtema, CODIGO_VACIO_SUBTEMA);
      sprintf (PG_command,"SELECT codigo_subtema, descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema != '          ' order by orden",
               Codigo_materia, Codigo_tema); 
      res = PQEXEC(DATABASE, PG_command);
      N_subtemas = PQntuples(res)+1;
      for (i=1; i < N_subtemas; i++)
          {
	   sprintf(hilera,"%s %s" ,PQgetvalue (res, i-1, 0),PQgetvalue (res, i-1, 1));
           gtk_combo_box_append_text(CB_subtema, hilera);
          }
      PQclear(res);

      gtk_widget_set_sensitive (GTK_WIDGET(CB_subtema), 1);
      gtk_combo_box_set_active (CB_subtema, 0);
      gtk_image_set_from_file (IM_alfiler_tema, ".imagenes/pin2.png");
      gdk_color_parse ("red", &color);
      gtk_widget_modify_bg(EN_tema_descripcion,  GTK_STATE_NORMAL,   &color);
     }
  else
     {
      N_subtemas = 1;
      gtk_combo_box_append_text(CB_subtema, CODIGO_VACIO_SUBTEMA);
      gtk_combo_box_set_active (CB_subtema   , 0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_subtema), 0);
      gtk_image_set_from_file (IM_alfiler_tema, ".imagenes/empty.png");
      gdk_color_parse ("gray", &color);
      gtk_widget_modify_bg(EN_tema_descripcion,  GTK_STATE_NORMAL,   &color);
     }

  g_free (tema);
  g_free (materia);
}

void Cambio_Subtema(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  gtk_image_set_from_file (IM_alfiler_subtema, ".imagenes/empty.png");
  gdk_color_parse ("gray", &color);
  gtk_widget_modify_bg(EN_subtema_descripcion,  GTK_STATE_NORMAL,   &color);

  Carga_SC_preguntas ();
}

void Carga_SC_preguntas ()
{
   char * basic_query = "SELECT codigo_ejercicio, materia, tema, subtema, secuencia_pregunta, descripcion_ejercicio, texto_pregunta, texto_opcion_A, texto_opcion_B, texto_opcion_C, texto_opcion_D, texto_opcion_E, respuesta, alfileres_opciones, creacion_day, creacion_month, creacion_year, N_estudiantes, N_examenes, ultimo_uso_day, ultimo_uso_month, ultimo_uso_year, point_biserial, point_biserial_1, point_biserial_2, point_biserial_3, point_biserial_4, point_biserial_5, frecuencia, frecuencia_1, frecuencia_2, frecuencia_3, frecuencia_4, frecuencia_5, media_examenes, varianza_examenes, media_correctos, media_incorrectos, cronbach_alpha_media_con, cronbach_alpha_media_sin, dificultad, texto_ejercicio, codigo_unico_pregunta from bd_ejercicios, bd_texto_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas, bd_personas where texto_ejercicio = consecutivo_texto and consecutivo_texto = codigo_consecutivo_ejercicio and pregunta = codigo_unico_pregunta and autor = codigo_persona ";
   char * order_query = "order by codigo_ejercicio, secuencia_pregunta";

   gchar *materia, *tema, *subtema;
   char Codigo_materia [CODIGO_MATERIA_SIZE + 1];
   char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
   char Codigo_subtema [CODIGO_SUBTEMA_SIZE + 1];
   char PG_command [4000];
   char hilera[300];
   int N_preguntas;
   GdkColor color;

   if (res_principal) PQclear(res_principal);

   materia = gtk_combo_box_get_active_text(CB_materia);
   tema    = gtk_combo_box_get_active_text(CB_tema);
   subtema = gtk_combo_box_get_active_text(CB_subtema);

   Extrae_codigo (materia, Codigo_materia);
   Extrae_codigo (tema,    Codigo_tema);
   Extrae_codigo (subtema, Codigo_subtema);

   if (strcmp(materia,CODIGO_VACIO_MATERIA) == 0)
      {
       sprintf (PG_command,"%s %s", basic_query, order_query);
      }
   else
      {
       if (strcmp(tema, CODIGO_VACIO_TEMA) == 0)
	  {
	   sprintf (PG_command,"%s and materia = '%s' %s", basic_query, Codigo_materia, order_query);
	  }
       else
	  {
	   if (strcmp(subtema, CODIGO_VACIO_SUBTEMA) == 0)
	      {
	       sprintf (PG_command,"%s and materia = '%s' and tema = '%s' %s", basic_query, Codigo_materia, Codigo_tema, order_query);
	      }
	   else
	      {
	       sprintf (PG_command,"%s and materia = '%s' and tema = '%s' and subtema = '%s' %s", basic_query, Codigo_materia, Codigo_tema, Codigo_subtema, order_query);
               gtk_image_set_from_file (IM_alfiler_subtema, ".imagenes/pin2.png");
               gdk_color_parse ("red", &color);
               gtk_widget_modify_bg(EN_subtema_descripcion,  GTK_STATE_NORMAL,   &color);
	      }
	  }
      }

   res_principal         = PQEXEC(DATABASE, PG_command);
   N_preguntas = PQntuples(res_principal);

   if (N_preguntas == 0)
      {
       gtk_widget_set_sensitive (window1, 0);
       sprintf(hilera,"<b>Materia:</b> %s", materia);
       gtk_label_set_markup(LB_materia, hilera);
       sprintf(hilera,"<b>Tema:</b> %s", tema);
       gtk_label_set_markup(LB_tema, hilera);
       sprintf(hilera,"<b>Subtema:</b> %s", subtema);
       gtk_label_set_markup(LB_subtema, hilera);
       gtk_widget_show          (window3);
       gtk_widget_grab_focus    (BN_ok);
      }
   else
      {
       if (N_preguntas > 1)
          {
           gtk_widget_set_sensitive (SC_pregunta, 1);
           gtk_range_set_range (GTK_RANGE(SC_pregunta),  (gdouble) 1.0, (gdouble) N_preguntas);
           gtk_range_set_value (GTK_RANGE(SC_pregunta),  (gdouble) N_preguntas);  /* Para forzar que refresque */
	  }
       else
	  { /* Una sola pregunta - se desactiva selector */
           gtk_widget_set_sensitive (SC_pregunta, 0);
           gtk_range_set_range (GTK_RANGE(SC_pregunta),  (gdouble) 0.0, (gdouble) 1.0);
           gtk_range_set_value (GTK_RANGE(SC_pregunta),  (gdouble) 0.0);  /* Para forzar que refresque */
	  }

       gtk_range_set_value (GTK_RANGE(SC_pregunta),  (gdouble) 1);
       sprintf (hilera,"%d", N_preguntas);
       gtk_entry_set_text(GTK_ENTRY(EN_N_preguntas), hilera);
      }

  gtk_widget_grab_focus (GTK_WIDGET(SC_pregunta));
}

void Extrae_codigo (char * hilera, char * codigo)
{
  int i;

  i = 0;

  while (hilera[i] != ' ') codigo [i] = hilera[i++];

  codigo[i] = '\0';
}

void Clear_Pregunta()
{
   char text       [2];

   gtk_image_set_from_file (IM_alfiler_opcion_A, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile_B, ".imagenes/smile.png");
   gtk_image_set_from_file (IM_alfiler_opcion_B, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile_B, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_alfiler_opcion_C, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile_C, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_alfiler_opcion_D, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile_D, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_alfiler_opcion_E, ".imagenes/pin2.png");
   gtk_image_set_from_file (IM_smile_E, ".imagenes/empty.png");

   gtk_toggle_button_set_active(CK_alfiler_opcion_A, FALSE);
   gtk_toggle_button_set_active(CK_alfiler_opcion_B, FALSE);
   gtk_toggle_button_set_active(CK_alfiler_opcion_C, FALSE);
   gtk_toggle_button_set_active(CK_alfiler_opcion_D, FALSE);
   gtk_toggle_button_set_active(CK_alfiler_opcion_E, TRUE);

   Alfiler_Opcion_A = 0;
   Alfiler_Opcion_B = 0;
   Alfiler_Opcion_C = 0;
   Alfiler_Opcion_D = 0;
   Alfiler_Opcion_E = 1;

   gtk_toggle_button_set_active(RB_opcion_E, TRUE);
   gtk_toggle_button_set_active(RB_opcion_A, TRUE);
}

void Cambio_Pregunta ()
{
  long int k;
  int N_estudiantes;
  long double Rpb, Dificultad;
  char hilera[50];
  PGresult *res_aux;
  GdkColor color;
  long double frecuencia_total, frecuencia_opcion, p;
  int boletos;
  int n;
  char codigo[10];
  char PG_command [1000];
  char text   [2];
  PGresult *res;

  k = (long int) gtk_range_get_value (GTK_RANGE(SC_pregunta)) - 1;

  gtk_widget_set_sensitive(BN_delete, 0);
  /* Una pregunta sólo puede ser eliminada sino es usada en el resto de la base de datos */
  sprintf (PG_command,"SELECT pre_examen from EX_pre_examenes_Lineas where codigo_pregunta = '%s'", PQgetvalue (res_principal, k, 42));
  res_aux = PQEXEC(DATABASE, PG_command);
  if (!PQntuples(res_aux)) gtk_widget_set_sensitive(BN_delete, 1);
  PQclear(res_aux);

  sprintf (hilera, "%s.%d", PQgetvalue (res_principal, k, 0),atoi(PQgetvalue (res_principal, k, 4)));
  gtk_entry_set_text(GTK_ENTRY(EN_pregunta), hilera);

  sprintf (PG_command,"SELECT descripcion_materia from BD_materias where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", 
                      PQgetvalue (res_principal, k, 1));
  res_aux = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res_aux))
     gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), PQgetvalue (res_aux, 0, 0));
  else
     gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), "*** Materia Inválida ***");
  PQclear(res_aux);

  sprintf (PG_command,"SELECT descripcion_materia from BD_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", 
                      PQgetvalue (res_principal, k, 1), PQgetvalue (res_principal, k, 2));
  res_aux = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res_aux))
     gtk_entry_set_text(GTK_ENTRY(EN_tema_descripcion), PQgetvalue (res_aux, 0, 0));
  else
     gtk_entry_set_text(GTK_ENTRY(EN_tema_descripcion), "*** Tema Inválido ***");
  PQclear(res_aux);

  sprintf (PG_command,"SELECT descripcion_materia from BD_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", 
                      PQgetvalue (res_principal, k, 1), PQgetvalue (res_principal, k, 2), PQgetvalue (res_principal, k, 3));
  res_aux = PQEXEC(DATABASE, PG_command);
  if (PQntuples(res_aux))
     gtk_entry_set_text(GTK_ENTRY(EN_subtema_descripcion), PQgetvalue (res_aux, 0, 0));
  else
     gtk_entry_set_text(GTK_ENTRY(EN_subtema_descripcion), "*** Subtema Inválido ***");
  PQclear(res_aux);

  gtk_entry_set_text(GTK_ENTRY(EN_descripcion), PQgetvalue (res_principal, k, 5));

  gtk_text_buffer_set_text(buffer_TV_pregunta, PQgetvalue (res_principal, k, 6), -1);
  gtk_text_buffer_set_text(buffer_TV_opcion_A, PQgetvalue (res_principal, k, 7), -1);
  gtk_text_buffer_set_text(buffer_TV_opcion_B, PQgetvalue (res_principal, k, 8), -1);
  gtk_text_buffer_set_text(buffer_TV_opcion_C, PQgetvalue (res_principal, k, 9), -1);
  gtk_text_buffer_set_text(buffer_TV_opcion_D, PQgetvalue (res_principal, k,10), -1);
  gtk_text_buffer_set_text(buffer_TV_opcion_E, PQgetvalue (res_principal, k,11), -1);

  if      (*PQgetvalue (res_principal, k,12) == 'A') gtk_toggle_button_set_active(RB_opcion_A, TRUE);
  else if (*PQgetvalue (res_principal, k,12) == 'B') gtk_toggle_button_set_active(RB_opcion_B, TRUE);
  else if (*PQgetvalue (res_principal, k,12) == 'C') gtk_toggle_button_set_active(RB_opcion_C, TRUE);
  else if (*PQgetvalue (res_principal, k,12) == 'D') gtk_toggle_button_set_active(RB_opcion_D, TRUE);
  else if (*PQgetvalue (res_principal, k,12) == 'E') gtk_toggle_button_set_active(RB_opcion_E, TRUE);

  if (*(PQgetvalue (res_principal, k,13)+0) == '1') 
     gtk_toggle_button_set_active(CK_alfiler_opcion_A, TRUE);
  else
     gtk_toggle_button_set_active(CK_alfiler_opcion_A, FALSE);
  if (*(PQgetvalue (res_principal, k,13)+1) == '1') 
     gtk_toggle_button_set_active(CK_alfiler_opcion_B, TRUE);
  else
     gtk_toggle_button_set_active(CK_alfiler_opcion_B, FALSE);
  if (*(PQgetvalue (res_principal, k,13)+2) == '1') 
     gtk_toggle_button_set_active(CK_alfiler_opcion_C, TRUE);
  else
     gtk_toggle_button_set_active(CK_alfiler_opcion_C, FALSE);
  if (*(PQgetvalue (res_principal, k,13)+3) == '1') 
     gtk_toggle_button_set_active(CK_alfiler_opcion_D, TRUE);
  else
     gtk_toggle_button_set_active(CK_alfiler_opcion_D, FALSE);
  if (*(PQgetvalue (res_principal, k,13)+4) == '1') 
     gtk_toggle_button_set_active(CK_alfiler_opcion_E, TRUE);
  else
     gtk_toggle_button_set_active(CK_alfiler_opcion_E, FALSE);

  sprintf (hilera,"%02d/%02d/%4d",
                  atoi(PQgetvalue (res_principal, k, 14)), 
                  atoi(PQgetvalue (res_principal, k, 15)), 
                  atoi(PQgetvalue (res_principal, k, 16)));
  gtk_entry_set_text(GTK_ENTRY(EN_creacion), hilera);

  N_estudiantes = atoi(PQgetvalue (res_principal, k, 17));
  sprintf (hilera, "%d", N_estudiantes);
  gtk_entry_set_text(GTK_ENTRY(EN_estudiantes), hilera); 

  sprintf (hilera, "%d", atoi(PQgetvalue (res_principal, k, 18)));
  gtk_entry_set_text(GTK_ENTRY(EN_examenes), hilera);

  if (N_estudiantes)
     {
      sprintf (hilera,"%02d/%02d/%4d",
                      atoi(PQgetvalue (res_principal, k, 19)), 
                      atoi(PQgetvalue (res_principal, k, 20)), 
                      atoi(PQgetvalue (res_principal, k, 21)));
      gtk_entry_set_text(GTK_ENTRY(EN_ultimo_uso), hilera);

      Rpb = (long double) atof(PQgetvalue (res_principal, k, 22));
      sprintf (hilera, "%6.4Lf",Rpb);
      gtk_entry_set_text(GTK_ENTRY(EN_discriminacion),   hilera);
      gtk_entry_set_text(GTK_ENTRY(EN_discriminacion_2), hilera);
      if (Rpb <= 0.0)
	 {
          gdk_color_parse ("red", &color);
          gtk_image_set_from_file (IM_discriminacion, ".imagenes/ARROW-red.png");
	 }
      else
 	 if (Rpb < 0.3)
	    {
             gdk_color_parse ("black", &color);
             gtk_image_set_from_file (IM_discriminacion, ".imagenes/empty.png");
	    }
	 else
	    {
             gdk_color_parse ("lime", &color);
             gtk_image_set_from_file (IM_discriminacion, ".imagenes/ARROW-green.png");
	    }
      gtk_widget_modify_bg(EN_discriminacion, GTK_STATE_NORMAL, &color);

      sprintf (hilera, "%6.4Lf", (long double) atof(PQgetvalue (res_principal, k, 23)));
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_A), hilera);
      sprintf (hilera, "%6.4Lf", (long double) atof(PQgetvalue (res_principal, k, 24)));
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_B), hilera);
      sprintf (hilera, "%6.4Lf", (long double) atof(PQgetvalue (res_principal, k, 25)));
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_C), hilera);
      sprintf (hilera, "%6.4Lf", (long double) atof(PQgetvalue (res_principal, k, 26)));
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_D), hilera);
      sprintf (hilera, "%6.4Lf", (long double) atof(PQgetvalue (res_principal, k, 27)));
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_E), hilera);

      frecuencia_total  = (long double) atof(PQgetvalue (res_principal, k, 28));

      frecuencia_opcion = (long double) atof(PQgetvalue (res_principal, k, 29));
      sprintf (hilera, "%7.2Lf",frecuencia_opcion/frecuencia_total*100.0);
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_A), hilera);
      p=frecuencia_opcion/frecuencia_total;
      if (p==1.0)p=0.999;
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_A), (gdouble) p);

      frecuencia_opcion = (long double) atof(PQgetvalue (res_principal, k, 30));
      sprintf (hilera, "%7.2Lf",frecuencia_opcion/frecuencia_total*100.0);
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_B), hilera);
      p=frecuencia_opcion/frecuencia_total;
      if (p==1.0)p=0.999;
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_B), (gdouble) p);

      frecuencia_opcion = (long double) atof(PQgetvalue (res_principal, k, 31));
      sprintf (hilera, "%7.2Lf",frecuencia_opcion/frecuencia_total*100.0);
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_C), hilera);
      p=frecuencia_opcion/frecuencia_total;
      if (p==1.0)p=0.999;
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_C), (gdouble) p);

      frecuencia_opcion = (long double) atof(PQgetvalue (res_principal, k, 32));
      sprintf (hilera, "%7.2Lf",frecuencia_opcion/frecuencia_total*100.0);
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_D), hilera);
      p=frecuencia_opcion/frecuencia_total;
      if (p==1.0)p=0.999;
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_D), (gdouble) p);

      frecuencia_opcion = (long double) atof(PQgetvalue (res_principal, k, 33));
      sprintf (hilera, "%7.2Lf",frecuencia_opcion/frecuencia_total*100.0);
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_E), hilera);
      p=frecuencia_opcion/frecuencia_total;
      if (p==1.0)p=0.999;
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_E), (gdouble) p);

      sprintf (hilera, "%6.4Lf", (long double) atof(PQgetvalue (res_principal, k, 34)));
      gtk_entry_set_text(GTK_ENTRY(EN_media_examenes), hilera);
 
      sprintf (hilera, "%6.4Lf", (long double) atof(PQgetvalue (res_principal, k, 35)));
      gtk_entry_set_text(GTK_ENTRY(EN_varianza_examenes), hilera); 

      sprintf (hilera, "%6.4Lf", (long double) atof(PQgetvalue (res_principal, k, 36)));
      gtk_entry_set_text(GTK_ENTRY(EN_media_correctos), hilera); 

      sprintf (hilera, "%6.4Lf", (long double) atof(PQgetvalue (res_principal, k, 37)));
      gtk_entry_set_text(GTK_ENTRY(EN_media_incorrectos), hilera); 

      sprintf (hilera, "%6.4Lf", (long double) atof(PQgetvalue (res_principal, k, 38)));
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_cronbach_con), hilera); 

      sprintf (hilera, "%6.4Lf", (long double) atof(PQgetvalue (res_principal, k, 39)));
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_cronbach_sin), hilera); 

      Dificultad = atof(PQgetvalue (res_principal, k, 40));
      gtk_range_set_value (GTK_RANGE(SC_dificultad),(gdouble) Dificultad);
      gtk_widget_set_sensitive (GTK_WIDGET(SC_dificultad), 0);

      if ((Dificultad < 0.3) || (Dificultad >= 0.9))
	 {
          gdk_color_parse ("red", &color);
          gtk_image_set_from_file (IM_dificultad, ".imagenes/ARROW-red.png");
	 }
      else
	 if ((Dificultad >= 0.675) && (Dificultad < 0.9))
	    {
             gdk_color_parse ("lime", &color);
             gtk_image_set_from_file (IM_dificultad, ".imagenes/ARROW-green.png");
	    }
	 else
	    {
             gdk_color_parse ("black", &color);
             gtk_image_set_from_file (IM_dificultad, ".imagenes/empty.png");
	    }
      gtk_widget_modify_bg(EN_dificultad, GTK_STATE_NORMAL, &color);
     }
  else
     { /* Pregunta No ha sido usada - no hay mucho que desplegar */
      gtk_entry_set_text(GTK_ENTRY(EN_ultimo_uso), "NO USADA");
      gdk_color_parse ("gray", &color);
      gtk_widget_modify_bg(EN_discriminacion, GTK_STATE_NORMAL, &color);
      gtk_image_set_from_file (IM_discriminacion, ".imagenes/empty.png");
      gdk_color_parse ("gray", &color);
      gtk_widget_modify_bg(EN_dificultad, GTK_STATE_NORMAL, &color);
      gtk_image_set_from_file (IM_dificultad, ".imagenes/empty.png");

      gtk_entry_set_text(GTK_ENTRY(EN_discriminacion), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_A), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_B), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_C), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_D), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_rpb_E), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_A), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_B), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_C), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_D), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_porcentaje_E), "\0");
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_A), (gdouble) 0.0);
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_B), (gdouble) 0.0);
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_C), (gdouble) 0.0);
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_D), (gdouble) 0.0);
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(PB_E), (gdouble) 0.0);
      gtk_entry_set_text(GTK_ENTRY(EN_media_examenes), "\0");
      gtk_entry_set_text(GTK_ENTRY(EN_varianza_examenes), "\0"); 
      gtk_entry_set_text(GTK_ENTRY(EN_media_correctos), "\0"); 
      gtk_entry_set_text(GTK_ENTRY(EN_media_incorrectos), "\0"); 
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_cronbach_con), "\0"); 
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_cronbach_sin), "\0"); 
      gtk_range_set_value (GTK_RANGE(SC_dificultad),(gdouble) atof(PQgetvalue (res_principal, k, 40)));
      gtk_widget_set_sensitive (GTK_WIDGET(SC_dificultad), 1);
     }

  gtk_widget_queue_draw(DA_dificultad);
  gtk_widget_queue_draw(DA_discriminacion);
  Despliega_detalle_boletos ();
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

  dato = gtk_editable_get_chars(GTK_EDITABLE(EN_dificultad_2), 0, -1);
  p = (long double) atof(dato);
  g_free (dato);

  return (parametros.b_dificultad + (int) round(parametros.m_dificultad * p));
}

int boletos_discriminacion()
{
  gchar * dato;
  long double Rpb;

  dato = gtk_editable_get_chars(GTK_EDITABLE(EN_discriminacion), 0, -1);
  Rpb = (long double) atof(dato);
  g_free (dato);
  if (Rpb < 0.0) Rpb = 0.0;

  return (parametros.b_discriminacion + (int) round(parametros.m_discriminacion * Rpb));
}

int boletos_novedad()
{
  int dias;

  dias = dias_sin_usar();
  return (parametros.b_novedad + (int) round(parametros.m_novedad * dias));
}

int dias_sin_usar ()
{
  int dias;
#define DIAS_MAXIMO 730
  gchar * dato;
  time_t curtime;
  struct tm *loctime;
  int month, year, day;
  int month_P,year_P, day_P;
  char hilera[50];

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
      day_P=atoi(dato);
      month_P=atoi(dato+3);
      year_P=atoi(dato+6);

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

  dato = gtk_editable_get_chars(GTK_EDITABLE(EN_estudiantes), 0, -1);
  estudiantes  = atoi(dato);
  g_free (dato);
  if (estudiantes > ESTUDIANTES_MAXIMO) estudiantes = ESTUDIANTES_MAXIMO;

  return (parametros.b_usos + (int) round(parametros.m_usos * estudiantes));
}

void Actualiza_Pregunta ()
{
  int k;
  PGresult *res;
  char PG_command [32000];
  GtkTextIter start;
  GtkTextIter end;
  char respuesta;
  gchar *text_pregunta, *text_opcion_A, *text_opcion_B, *text_opcion_C, *text_opcion_D, *text_opcion_E;
  char *pregunta_corregida;
  char *opcion_A_corregida;
  char *opcion_B_corregida;
  char *opcion_C_corregida;
  char *opcion_D_corregida;
  char *opcion_E_corregida;
  int N_alfileres;
  char alfileres_opciones [6] = "00000";
  long double dificultad, varianza;
  int N_estudiantes;
  char Codigo_texto [EJERCICIO_SIZE];
  char Codigo_Pregunta [CODIGO_SIZE];
  int secuencia;

  /* Los campos que pueden ser alterados se toman de la interfaz, los otros se toman del query principal */

  k = (long int) gtk_range_get_value (GTK_RANGE(SC_pregunta)) - 1;

  N_estudiantes = atoi(    PQgetvalue (res_principal, k, 17));
  strcpy (Codigo_texto,    PQgetvalue (res_principal, k, 41));
  strcpy (Codigo_Pregunta, PQgetvalue (res_principal, k, 42));
  secuencia =     atoi(    PQgetvalue (res_principal, k, 4));

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

  res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

  sprintf (PG_command,"UPDATE bd_texto_preguntas SET tipo='1', texto_pregunta=E'%s', texto_opcion_A=E'%s',  texto_opcion_B=E'%s',  texto_opcion_C=E'%s',  texto_opcion_D=E'%s',  texto_opcion_E=E'%s', Respuesta='%c', Alfileres_opciones='%.5s'where codigo_consecutivo_ejercicio = '%.6s' and secuencia_pregunta = %d", 
           pregunta_corregida, opcion_A_corregida, opcion_B_corregida, opcion_C_corregida, opcion_D_corregida, opcion_E_corregida,
           respuesta,alfileres_opciones, Codigo_texto, secuencia);
  res = PQEXEC(DATABASE, PG_command); PQclear(res);

  if (!N_estudiantes)
     { /* Pregunta no ha sido usada. Sólo se actualiza dificultad y varianza */
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
     }

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

  Carga_SC_preguntas ();
  gtk_range_set_value (GTK_RANGE(SC_pregunta), (gdouble) (k + 1));

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

void Imprime_Pregunta ()
{
   FILE * Archivo_Latex;
   gchar * pregunta;
   int k;
   PGresult *res,*res_aux;
   char PG_command [1000];
   char hilera_antes [9000], hilera_despues [9000];
   char alfileres [6];
   long double media, discriminacion;
   int boletos;

   k = (long int) gtk_range_get_value (GTK_RANGE(SC_pregunta)) - 1;

   pregunta     = gtk_editable_get_chars(GTK_EDITABLE(EN_pregunta), 0, -1);
   Archivo_Latex = fopen ("EX2050.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\begin{document}\n");
   fprintf (Archivo_Latex, "\\title{Pregunta %s}\n", pregunta);
   fprintf (Archivo_Latex, "\\maketitle\n\n");

   fprintf (Archivo_Latex, "\\rule{8.75cm}{5pt}\n\n");

   sprintf (PG_command,"SELECT * from bd_texto_preguntas, bd_estadisticas_preguntas where codigo_consecutivo_ejercicio = '%s' and secuencia_pregunta = %d and codigo_unico_pregunta = pregunta", 
            PQgetvalue (res_principal, k, 41),
            atoi(PQgetvalue (res_principal, k, 4)));
   res = PQEXEC(DATABASE, PG_command);

   sprintf (PG_command,"SELECT usa_header, texto_header from bd_texto_ejercicios where consecutivo_texto = '%s'",PQgetvalue (res_principal, k, 41)); 
   res_aux = PQEXEC(DATABASE, PG_command);

   fprintf (Archivo_Latex, "\\begin{questions}\n");

   fprintf (Archivo_Latex, "\n\n");

   if (*PQgetvalue (res_aux, 0, 0) == 't')
      {
       fprintf (Archivo_Latex,"\\question ");
       sprintf (hilera_antes,"%s \n %s", PQgetvalue (res_aux, 0, 1), PQgetvalue (res, 0, 8));
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
      }
   else
      {
       fprintf (Archivo_Latex,"\\question ");
       sprintf (hilera_antes,"%s", PQgetvalue (res, 0, 8));
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
      }

   strcpy (alfileres, PQgetvalue (res, 0, 15));

   fprintf (Archivo_Latex, "\\begin{answers}\n");

   fprintf(Archivo_Latex, "\\item ");
   if (alfileres [0] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   sprintf (hilera_antes," %s", PQgetvalue (res, 0, 9));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   if (*(PQgetvalue (res, 0, 14)) == 'A')
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   fprintf (Archivo_Latex, "\n\n");

   fprintf(Archivo_Latex, "\\item ");
   if (alfileres [1] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   sprintf (hilera_antes," %s", PQgetvalue (res, 0, 10));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   if (*(PQgetvalue (res, 0, 14)) == 'B')
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   fprintf (Archivo_Latex, "\n\n");

   fprintf(Archivo_Latex, "\\item ");
   if (alfileres [2] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   sprintf (hilera_antes," %s", PQgetvalue (res, 0, 11));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   if (*(PQgetvalue (res, 0, 14)) == 'C')
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   fprintf (Archivo_Latex, "\n\n");

   fprintf(Archivo_Latex, "\\item ");
   if (alfileres [3] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   sprintf (hilera_antes," %s", PQgetvalue (res, 0, 12));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   if (*(PQgetvalue (res, 0, 14)) == 'D')
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   fprintf (Archivo_Latex, "\n\n");

   fprintf(Archivo_Latex, "\\item ");
   if (alfileres [4] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   sprintf (hilera_antes," %s", PQgetvalue (res, 0, 13));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   if (*(PQgetvalue (res, 0, 14)) == 'E')
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\end{answers}\n");

   fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\begin{center}");
   media          = (long double) atof(PQgetvalue (res, 0, 26));
   discriminacion = (long double) atof(PQgetvalue (res, 0, 28));
   boletos = asigna_boletos();

   fprintf (Archivo_Latex,"\\framebox[7.75cm][l]{Dificultad: \\textbf{%6.4Lf} - Discriminaci\\'on: \\textbf{%6.4Lf} - Boletos: \\textbf{%d}}\n\n", 
            media, discriminacion, boletos);
   fprintf (Archivo_Latex, "\\end{center}");

   fprintf (Archivo_Latex, "\\end{questions}\n\n");

   fprintf (Archivo_Latex, "\\rule{8.75cm}{5pt}\n\n");

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX2050", 1, NULL, 0.0, 0.0, NULL, NULL);
   g_free (pregunta);
}

void Borra_Registro (GtkWidget *widget, gpointer user_data)
{
   PGresult *res, *res_aux;
   char PG_command [1000];
   int secuencia;
   int i, j, k, N_mayores;
   char codigo_actual [CODIGO_SIZE];

   k = (long int) gtk_range_get_value (GTK_RANGE(SC_pregunta)) - 1;

   secuencia = atoi(PQgetvalue (res_principal, k, 4));

   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

   sprintf (PG_command,"DELETE from bd_estadisticas_preguntas where pregunta = '%.6s'", PQgetvalue (res_principal, k, 42));
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   sprintf (PG_command,"DELETE from bd_texto_preguntas where codigo_unico_pregunta = '%.6s'", PQgetvalue (res_principal, k, 42));
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   sprintf (PG_command,"SELECT * from bd_texto_preguntas where codigo_consecutivo_ejercicio = '%.6s' and secuencia_pregunta > %d order by secuencia_pregunta",
	    PQgetvalue (res_principal, k, 41),
            secuencia);
   res = PQEXEC(DATABASE, PG_command);

   N_mayores = PQntuples(res);

   for (i=0; i <N_mayores; i++)
       {
        strcpy (codigo_actual, PQgetvalue (res, i, 2));
	j              = atoi (PQgetvalue (res, i, 1)) - 1;
        sprintf (PG_command,"UPDATE bd_texto_preguntas SET secuencia_pregunta=%d where codigo_unico_pregunta = '%.6s'", 
		 j, codigo_actual);

        res_aux = PQEXEC(DATABASE, PG_command); PQclear(res_aux);
       }

   PQclear(res);

   res = PQEXEC(DATABASE, "END"); PQclear(res);

   gtk_combo_box_set_active (CB_materia, 1);
   gtk_combo_box_set_active (CB_materia, 0);

  carga_parametros_EXAMINER (&parametros, DATABASE);
  if (parametros.report_update)
     {
      gtk_widget_set_sensitive(window1, 0);
      gtk_widget_show (window5);

      if (parametros.timeout)
         {
          while (gtk_events_pending ()) gtk_main_iteration ();
          catNap(parametros.timeout);
          gtk_widget_hide (window5);
          gtk_widget_set_sensitive(window1, 1);
	 }
     }
}

void Actualiza_Dificultad ()
{
  long double dificultad, varianza;
  char hilera[20];
  int boletos;

  dificultad = gtk_range_get_value (GTK_RANGE(SC_dificultad));
  sprintf (hilera, "%6.5Lf", dificultad);
  gtk_entry_set_text(GTK_ENTRY(EN_dificultad_2), hilera); 
  gtk_entry_set_text(GTK_ENTRY(EN_dificultad), hilera); 

  varianza = dificultad * (1 - dificultad);
  sprintf (hilera, "%6.5Lf", varianza);
  gtk_entry_set_text(GTK_ENTRY(EN_varianza), hilera); 

  boletos = asigna_boletos();
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

void Toggle_alfiler_opcion_A(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  Alfiler_Opcion_A = 1 - Alfiler_Opcion_A;
  if (!Alfiler_Opcion_A)
     gtk_image_set_from_file (IM_alfiler_opcion_A, ".imagenes/empty.png");
  else
     gtk_image_set_from_file (IM_alfiler_opcion_A, ".imagenes/pin2.png");
}

void Toggle_alfiler_opcion_B(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  Alfiler_Opcion_B = 1 - Alfiler_Opcion_B;
  if (!Alfiler_Opcion_B)
     gtk_image_set_from_file (IM_alfiler_opcion_B, ".imagenes/empty.png");
  else
     gtk_image_set_from_file (IM_alfiler_opcion_B, ".imagenes/pin2.png");

#if 0
  /* Redibujar marco de opción */
  if (Opcion_B_correcta)
     {
      gdk_color_parse ("gold", &color);
      gtk_widget_modify_bg(FR_opcionB,   GTK_STATE_NORMAL,   &color);
     }
  else
     {
      gdk_color_parse ("slate gray", &color);
      gtk_widget_modify_bg(FR_opcionB,   GTK_STATE_NORMAL,   &color);
     }
#endif
}

void Toggle_alfiler_opcion_C(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  Alfiler_Opcion_C = 1 - Alfiler_Opcion_C;
  if (!Alfiler_Opcion_C)
     gtk_image_set_from_file (IM_alfiler_opcion_C, ".imagenes/empty.png");
  else
     gtk_image_set_from_file (IM_alfiler_opcion_C, ".imagenes/pin2.png");

#if 0
  /* Redibujar marco de opción */
  if (Opcion_C_correcta)
     {
      gdk_color_parse ("gold", &color);
      gtk_widget_modify_bg(FR_opcionC,   GTK_STATE_NORMAL,   &color);
     }
  else
     {
      gdk_color_parse ("slate gray", &color);
      gtk_widget_modify_bg(FR_opcionC,   GTK_STATE_NORMAL,   &color);
     }
#endif
}

void Toggle_alfiler_opcion_D(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  Alfiler_Opcion_D = 1 - Alfiler_Opcion_D;
  if (!Alfiler_Opcion_D)
     gtk_image_set_from_file (IM_alfiler_opcion_D, ".imagenes/empty.png");
  else
     gtk_image_set_from_file (IM_alfiler_opcion_D, ".imagenes/pin2.png");

#if 0
  /* Redibujar marco de opción */
  if (Opcion_C_correcta)
     {
      gdk_color_parse ("gold", &color);
      gtk_widget_modify_bg(FR_opcionD,   GTK_STATE_NORMAL,   &color);
     }
  else
     {
      gdk_color_parse ("slate gray", &color);
      gtk_widget_modify_bg(FR_opcionD,   GTK_STATE_NORMAL,   &color);
     }
#endif
}

void Toggle_alfiler_opcion_E(GtkWidget *widget, gpointer user_data)
{
  GdkColor color;

  Alfiler_Opcion_E = 1 - Alfiler_Opcion_E;
  if (!Alfiler_Opcion_E)
     gtk_image_set_from_file (IM_alfiler_opcion_E, ".imagenes/empty.png");
  else
     gtk_image_set_from_file (IM_alfiler_opcion_E, ".imagenes/pin2.png");

#if 0
  /* Redibujar marco de opción */
  if (Opcion_E_correcta)
     {
      gdk_color_parse ("gold", &color);
      gtk_widget_modify_bg(FR_opcionE,   GTK_STATE_NORMAL,   &color);
     }
  else
     {
      gdk_color_parse ("slate gray", &color);
      gtk_widget_modify_bg(FR_opcionE,   GTK_STATE_NORMAL,   &color);
     }
#endif
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

      gdk_color_parse (DISTRACTOR_FR, &color);
      gtk_widget_modify_bg(FR_opcionA,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_A,   GTK_STATE_PRELIGHT,   &color);
      gtk_widget_modify_bg(FR_texto_A,   GTK_STATE_NORMAL,   &color);
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

      gdk_color_parse (DISTRACTOR_FR, &color);
      gtk_widget_modify_bg(FR_opcionB,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_B,   GTK_STATE_PRELIGHT,   &color);
      gtk_widget_modify_bg(FR_texto_B,   GTK_STATE_NORMAL,   &color);
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

      gdk_color_parse (DISTRACTOR_FR, &color);
      gtk_widget_modify_bg(FR_opcionC,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_C,   GTK_STATE_PRELIGHT,   &color);
      gtk_widget_modify_bg(FR_texto_C,   GTK_STATE_NORMAL,   &color);
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

      gdk_color_parse (DISTRACTOR_FR, &color);
      gtk_widget_modify_bg(FR_opcionD,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_D,   GTK_STATE_PRELIGHT,   &color);
      gtk_widget_modify_bg(FR_texto_D,   GTK_STATE_NORMAL,   &color);
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

      gdk_color_parse (DISTRACTOR_FR, &color);
      gtk_widget_modify_bg(FR_opcionE,   GTK_STATE_NORMAL,   &color);
      gtk_widget_modify_bg(PB_E,   GTK_STATE_PRELIGHT,   &color);
      gtk_widget_modify_bg(FR_texto_E,   GTK_STATE_NORMAL,   &color);
     }
}

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex2050_destroy (GtkWidget *widget, gpointer user_data) 
{
  Fin_Ventana (widget, user_data);
}

void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data)
{
  Fin_de_Programa (widget, user_data);
}

void on_BN_undo_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_combo_box_set_active (CB_materia, 1);
  gtk_combo_box_set_active (CB_materia, 0);
}

void on_BN_save_clicked(GtkWidget *widget, gpointer user_data)
{
  Actualiza_Pregunta ();
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

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window3);
  gtk_widget_set_sensitive(window1, 1);

  Init_Fields();

  gtk_widget_grab_focus (SC_pregunta);
}

void Despliega_detalle_boletos()
{
  char hilera [100];
  int i, N, total;
  gchar * dato_previo, *pregunta;
  float F;
  char rotulo[100];

  N = dias_sin_usar();
  sprintf(hilera,"%d", N);
  gtk_entry_set_text(GTK_ENTRY(EN_dias), hilera); 

  dato_previo = gtk_editable_get_chars(GTK_EDITABLE(EN_dificultad), 0, -1);
  gtk_entry_set_text(GTK_ENTRY(EN_dificultad_2), dato_previo); 
  g_free(dato_previo);

  dato_previo = gtk_editable_get_chars(GTK_EDITABLE(EN_discriminacion), 0, -1);
  F = atof(dato_previo);
  sprintf(hilera,"%6.4f", F);
  gtk_entry_set_text(GTK_ENTRY(EN_discriminacion_2), hilera);
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

void on_BN_print_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Pregunta ();
}

void on_SC_dificultad_value_changed(GtkWidget *widget, gpointer user_data)
{
  Actualiza_Dificultad ();
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

void on_SC_pregunta_value_changed(GtkWidget *widget, gpointer user_data)
{
  int k;

  k = (long int) gtk_range_get_value (GTK_RANGE(SC_pregunta));

  if (k) Cambio_Pregunta ();
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

void on_BN_ok_update_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window4);
  gtk_widget_set_sensitive(window1, 1);
}

void on_BN_ok_delete_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window5);
  gtk_widget_set_sensitive(window1, 1);
}

void on_DA_dificultad_expose(GtkWidget *widget, gpointer user_data)
{
   gchar * dato;
   long double p, r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   dato = gtk_editable_get_chars(GTK_EDITABLE(EN_dificultad_2), 0, -1);
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
   
   dato = gtk_editable_get_chars(GTK_EDITABLE(EN_discriminacion), 0, -1);
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

void on_BN_extend_question_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * pregunta;
  int N;
  char window_title [100];
  char root[100];

  pregunta = gtk_editable_get_chars(GTK_EDITABLE(EN_pregunta), 0, -1);
  sprintf (window_title, "Código LATEX de Pregunta %s", pregunta);
  sprintf (root, "%s", pregunta);

  Display_Latex_Window (window_title, root, buffer_TV_pregunta, window1);
}

void on_BN_extend_A_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * pregunta;
  char window_title [100];
  char root[100];

  pregunta = gtk_editable_get_chars(GTK_EDITABLE(EN_pregunta), 0, -1);
  sprintf (window_title, "Código LATEX de Opción A de Pregunta %s", pregunta);
  sprintf (root, "%s-A", pregunta);

  Display_Latex_Window (window_title, root, buffer_TV_opcion_A, window1);
}

void on_BN_extend_B_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * pregunta;
  char window_title [100];
  char root[100];

  pregunta = gtk_editable_get_chars(GTK_EDITABLE(EN_pregunta), 0, -1);
  sprintf (window_title, "Código LATEX de Opción B de Pregunta %s", pregunta);
  sprintf (root, "%s-B", pregunta);

  Display_Latex_Window (window_title, root, buffer_TV_opcion_B, window1);
}

void on_BN_extend_C_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * pregunta;
  char window_title [100];
  char root[100];

  pregunta = gtk_editable_get_chars(GTK_EDITABLE(EN_pregunta), 0, -1);
  sprintf (window_title, "Código LATEX de Opción C de Pregunta %s", pregunta);
  sprintf (root, "%s-C", pregunta);

  Display_Latex_Window (window_title, root, buffer_TV_opcion_C, window1);
}

void on_BN_extend_D_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * pregunta;
  char window_title [100];
  char root[100];

  pregunta = gtk_editable_get_chars(GTK_EDITABLE(EN_pregunta), 0, -1);
  sprintf (window_title, "Código LATEX de Opción D de Pregunta %s", pregunta);
  sprintf (root, "%s-D", pregunta);

  Display_Latex_Window (window_title, root, buffer_TV_opcion_D, window1);
}

void on_BN_extend_E_clicked(GtkWidget *widget, gpointer user_data)
{
  gchar * pregunta;
  char window_title [100];
  char root[100];

  pregunta = gtk_editable_get_chars(GTK_EDITABLE(EN_pregunta), 0, -1);
  sprintf (window_title, "Código LATEX de Opción E de Pregunta %s", pregunta);
  sprintf (root, "%s-E", pregunta);

  Display_Latex_Window (window_title, root, buffer_TV_opcion_E, window1);
}
