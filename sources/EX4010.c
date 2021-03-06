/*
Compilar con

cc -o EX4010 EX4010.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic

*/
/*******************************************/
/*  EX4010:                                */
/*                                         */
/*    Analiza Exámenes                     */
/*    The Examiner 0.2                     */
/*    23 de Marzo del 2012                 */
/*    Autor: Francisco J. Torres-Rojas     */        
/*                                         */
/*******************************************/
#include <gtk/gtk.h>
#include <stdlib.h>
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
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define MAX_COLORES 18

#define EXAMEN_SIZE           6
#define DESCRIPCION_SIZE      201
#define CODIGO_EJERCICIO_SIZE 7
#define CODIGO_PREGUNTA_SIZE  7

#define CODIGO_VERSION_SIZE   5
#define TEXTO_PREGUNTA_SIZE   501
#define AUTOR_SIZE            101

int N_preguntas   = 0;
int N_versiones   = 0;
int N_estudiantes = 0;
int N_temas       = 0;
int N_subtemas    = 0;
int N_ajustes     = 0;

long double Nota_minima, Nota_maxima;
long double Nota_minima_ajustada, Nota_maxima_ajustada;

#define CATEGORIAS 10
int Frecuencias [CATEGORIAS];

#define N_FLAGS                  16

#define TODAS_MALAS                  0
#define TODAS_BUENAS                 1
#define PORCENTAJE_MENOR_30          2
#define RPB_BUENO                    3
#define RPB_MUY_NEGATIVO             4
#define RPB_NEGATIVO                 5
#define SUBE_MAS_DE_DESVIACION       6
#define BAJA_MAS_DE_DESVIACION       7
#define ALFA_SUBE_MUCHO              8
#define ALFA_SUBE                    9
#define RPB_DISTRACTOR_MUY_POSITIVO  10
#define RPB_DISTRACTOR_POSITIVO      11
#define RPB_DISTRACTOR_MUY_NEGATIVO  12
#define MAS_DE_3_DISTRACTORES        13
#define RPB_Y_DIFICULTAD_OK          14
#define RPB_DIFI_DISTRA_OK           15

struct VERSION
{
  char codigo [CODIGO_VERSION_SIZE];
  struct PREGUNTA_VERSION
  {
    char codigo [CODIGO_PREGUNTA_SIZE];
    char respuesta;
    int  orden_version [5];
  } * preguntas;
} * versiones = NULL;

struct PREGUNTA
{
  char tema      [CODIGO_TEMA_SIZE    + 1];
  char subtema   [CODIGO_SUBTEMA_SIZE + 1];
  char autor     [AUTOR_SIZE];

  int  orden_tema;
  int  orden_subtema;

  char ejercicio [CODIGO_EJERCICIO_SIZE];
  int  secuencia;
  char pregunta  [CODIGO_PREGUNTA_SIZE];
  int  buenos;
  int  malos;
  int  acumulado_opciones      [5];
  long double suma_seleccion   [5];
  long double media_ex_1       [5];
  long double media_ex_0       [5];
  long double Rpb_opcion       [5];

  char correcta;
  long double previo;
  long double desviacion;
  long double porcentaje;
  long double suma_buenos;
  long double suma_malos;
  long double Rpb;
  long double alfa_sin;
  int flags [N_FLAGS];

  char texto_pregunta [TEXTO_PREGUNTA_SIZE+1];
  int  ajuste;
  int  revision_especial;
  int  correctas_nuevas[5];
  int  actualizar;
  int  excluir;
  int  encoger;
  int  verbatim;
  int  header_encoger;
  int  header_verbatim;
  int  slide           [5];
  int  encoger_opcion  [5];
  int  verbatim_opcion [5];
  int  grupo_inicio;
  int  grupo_final;
};

struct PREGUNTA * preguntas            = NULL;
struct PREGUNTA * resumen_tema_subtema = NULL;
struct PREGUNTA * resumen_tema         = NULL;

#define VERDE    0
#define AZUL     1
#define CYAN     2
#define AMARILLO 3
#define ROJO     4
#define AJUSTE   5
int Banderas [6];
char * colores  [] = {"green", "blue", "cyan", "yellow", "red", "violet"};
char * banderas [] = {"FL-verde.jpg", "FL-azul.jpg", "FL-cyan.jpg", "FL-amarilla.jpg", "FL-roja.jpg", "fix.png"};

/* Parametros para barras en reporte */
#define MINIMA 1.15
#define MAXIMA 6.65

/* Tipos de Ajuste */
#define PREGUNTA_NORMAL  0
#define NO_CONSIDERAR    1
#define VARIAS_CORRECTAS 2
#define BUENA_TODOS      3
#define MALA_TODOS       4
#define SOLO_SI_BUENA    5
#define REVISA_Y_BONO    6
#define PREGUNTA_EXTRA   7

/* Variables para cruce discriminacion por dificultad */
int Niveles_Discriminacion;
int Niveles_Dificultad;
int Frecuencia_total [11][21];

/* Beamer Stuff */
char * Beamer_aspectratio[] = {"43", "169", "1610", "149", "141", "54", "32"};

/***************************/
/* Postgres stuff          */
/***************************/
PGconn	 *DATABASE;

/*************/
/* GTK stuff */
/*************/
GtkBuilder*builder; 
GError    * error;

GtkWidget *window1                   = NULL;
GtkWidget *window2                   = NULL;
GtkWidget *window3                   = NULL;
GtkWidget *window4                   = NULL;
GtkWidget *window5                   = NULL;
GtkWidget *window6                   = NULL;
GtkWidget *window7                   = NULL;

GtkWidget *EB_analisis               = NULL;
GtkWidget *FR_analisis               = NULL;

GtkSpinButton *SP_examen             = NULL;
GtkWidget *EN_descripcion            = NULL;
GtkWidget *EN_pre_examen             = NULL;
GtkWidget *EN_pre_examen_descripcion = NULL;
GtkWidget *EN_esquema                = NULL;
GtkWidget *EN_esquema_descripcion    = NULL;
GtkWidget *EN_materia                = NULL;
GtkWidget *EN_materia_descripcion    = NULL;
GtkWidget *EN_institucion            = NULL;
GtkWidget *EN_escuela                = NULL;
GtkWidget *EN_programa               = NULL;
GtkWidget *EN_profesor               = NULL;
GtkWidget *EN_fecha                  = NULL;
GtkWidget *EN_N_preguntas            = NULL;
GtkWidget *EN_N_versiones            = NULL;
GtkWidget *EN_N_estudiantes          = NULL;

GtkWidget *EB_prediccion             = NULL;
GtkWidget *FR_prediccion             = NULL;
GtkWidget *EB_real                   = NULL;
GtkWidget *FR_real                   = NULL;
GtkWidget *EB_grafico                = NULL;
GtkWidget *FR_grafico                = NULL;
GtkToggleButton *CK_smooth           = NULL;

GtkWidget *EB_preguntas              = NULL;
GtkWidget *FR_preguntas              = NULL;
GtkWidget *EB_ajustes                = NULL;
GtkWidget *FR_ajustes                = NULL;
GtkWidget *EB_formato                = NULL;
GtkWidget *FR_formato                = NULL;
GtkWidget *FR_procesado              = NULL;
GtkWidget *FR_botones                = NULL;
GtkWidget *FR_pregunta_actual        = NULL;
GtkWidget *SC_preguntas              = NULL;
GtkTextView *TV_pregunta             = NULL;
GtkTextBuffer * buffer_TV_pregunta;
GtkComboBox *CB_ajuste               = NULL;
GtkToggleButton *TG_A                = NULL;
GtkToggleButton *TG_B                = NULL;
GtkToggleButton *TG_C                = NULL;
GtkToggleButton *TG_D                = NULL;
GtkToggleButton *TG_E                = NULL;
GtkToggleButton *CK_no_actualiza     = NULL;
GtkToggleButton *CK_excluir          = NULL;
GtkToggleButton *CK_encoger          = NULL;
GtkToggleButton *CK_verbatim         = NULL;
GtkToggleButton *CK_header_encoger   = NULL;
GtkToggleButton *CK_header_verbatim  = NULL;
GtkToggleButton *CK_slide           [5];
GtkToggleButton *CK_encoger_opcion  [5];
GtkToggleButton *CK_verbatim_opcion [5];

GtkWidget *EB_beamer                 = NULL;
GtkWidget *FR_beamer                 = NULL;
GtkComboBox *CB_estilo               = NULL;
GtkComboBox *CB_color                = NULL;
GtkComboBox *CB_font                 = NULL;
GtkComboBox *CB_size                 = NULL;
GtkComboBox *CB_aspecto              = NULL;
GtkToggleButton *CK_general          = NULL;
GtkToggleButton *CK_sin_banderas     = NULL;

GtkWidget *EN_media_prediccion       = NULL;
GtkWidget *EN_desviacion_prediccion  = NULL;
GtkWidget *EN_alfa_prediccion        = NULL;
GtkWidget *EN_Rpb_prediccion         = NULL;

GtkWidget *EN_media_real             = NULL;
GtkWidget *EN_desviacion_real        = NULL;
GtkWidget *EN_alfa_real              = NULL;
GtkWidget *EN_Rpb_real               = NULL;

GtkWidget *BN_undo                = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_slides              = NULL;
GtkWidget *BN_save                = NULL;
GtkWidget *BN_terminar            = NULL;
GtkWidget *BN_ok                  = NULL;
GtkWidget *BN_error_encontrado_Beamer = NULL;

GtkWidget *EB_generando_beamer    = NULL;
GtkWidget *FR_generando_beamer    = NULL;
GtkWidget *EB_generando_analisis  = NULL;
GtkWidget *FR_generando_analisis  = NULL;
GtkWidget *FR_error_Beamer        = NULL;
GtkWidget *BN_confirma_revision   = NULL;
GtkWidget *BN_cancela_revision    = NULL;
GtkWidget *EB_revisando_beamer    = NULL;
GtkWidget *FR_revisando_beamer    = NULL;
GtkWidget *FR_error_encontrado_Beamer = NULL;
GtkLabel  *LB_error_encontrado_Beamer = NULL;

GtkWidget *PB_analisis            = NULL;
GtkWidget *PB_beamer              = NULL;
GtkWidget *PB_revisando_beamer    = NULL;

GtkSpinButton *SP_resolucion = NULL;
GtkSpinButton *SP_color      = NULL;
GtkSpinButton *SP_rotacion   = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Actualice_Estadisticas (int buena, char * ejercicio, int secuencia, long double Nota, char respuesta, int opcion_original[5]);
void Actualiza_Porcentajes ();
void Analisis_General (FILE * Archivo_Latex, long double alfa, long double Rpb, int Beamer_o_reporte);
void Analiza_Ajuste (FILE * Archivo_Latex, struct PREGUNTA item, int modo);
void Analiza_Banderas (FILE * Archivo_Latex, struct PREGUNTA item, int beamer, int N_flags, int i, char * Descripcion);
void Asigna_Banderas ();
void Beamer_Cierre          (FILE * Archivo_Latex);
void Beamer_Cover           (FILE * Archivo_Latex);
void Beamer_Dificultad_vs_Discriminacion(FILE * Archivo_Latex);
void Beamer_Failure();
void Beamer_Gracias(FILE * Archivo_Latex);
void Beamer_Grafico_Pastel  (FILE * Archivo_Latex);
void Beamer_Datos_Generales (FILE * Archivo_Latex, gchar * institucion, gchar * escuela, gchar * programa, gchar * materia_descripcion, gchar * profesor, gchar * descripcion, gchar * fecha, char * codigo, long double media_real, long double desviacion_real, long double alfa, long double Rpb);
void Beamer_Histograma_Notas(FILE * Archivo_Latex, long double media_real, long double desviacion_real, long double media_prediccion, long double desviacion_prediccion );
void Beamer_Histograma_Temas(FILE * Archivo_Latex);
void Beamer_Preamble        (FILE * Archivo_Latex, int aspecto, gchar * size, gchar * estilo, gchar * color, gchar * font, gchar * materia_descripcion, gchar * descripcion, gchar * profesor, gchar * programa, gchar * escuela, gchar * institucion, gchar * fecha);
void Beamer_Preamble_reducido (FILE * Archivo_Latex, int aspecto, gchar * size, gchar * estilo, gchar * color, gchar * font);
void Beamer_Preguntas(FILE * Archivo_Latex, GtkWidget *PB, long double base, long double limite);
void Beamer_TOC             (FILE * Archivo_Latex);
void Cajita_con_bandera (FILE * Archivo_Latex, char * mensaje, int color, int modo);
void Calcula_ajuste(int i);
void Calcula_estadisticas_examen ();
void Calcula_nota_ajustada (int k_version, char * respuestas, int * n, int * m);
void Calcula_Notas (char * version, char * respuestas, int * n_buenas, int * n_ajustado, int * m_ajustado);
void Calcular_Tabla();
void Cambia_Pregunta ();
void Cambio_A (GtkWidget *widget, gpointer user_data);
void Cambio_Ajuste (GtkWidget *widget, gpointer user_data);
void Cambio_B (GtkWidget *widget, gpointer user_data);
void Cambio_C (GtkWidget *widget, gpointer user_data);
void Cambio_D (GtkWidget *widget, gpointer user_data);
void Cambio_E (GtkWidget *widget, gpointer user_data);
void Cambio_encoger (GtkWidget *widget, gpointer user_data);
void Cambio_encoger_opcion (int i);
void Cambio_Examen();
void Cambio_excluir (GtkWidget *widget, gpointer user_data);
void Cambio_header_encoger (GtkWidget *widget, gpointer user_data);
void Cambio_header_verbatim (GtkWidget *widget, gpointer user_data);
void Cambio_no_actualizar (GtkWidget *widget, gpointer user_data);
void Cambio_slide (int i);
void Cambio_verbatim (GtkWidget *widget, gpointer user_data);
void Cambio_verbatim_opcion (int i);
void Carga_preguntas_examen ();
long double CDF (long double X, long double Media, long double Desv);
void Color_ajustes(int k);
void Color_Fila (FILE * Archivo_Latex, int flags[N_FLAGS]);
void colores_pastel (FILE * Archivo_Latex);
void Connect_Widgets();
void Construye_versiones (char * examen);
void Continuar_banderas (FILE * Archivo_Latex, int i, char * Descripcion);
void Crea_archivo_datos_pastel(int *Empates);
void Dificultad_vs_Discriminacion ();
void Establece_Directorio (char * Directorio, gchar * materia, char * year, char * month, char * day);
void Fin_de_Programa (GtkWidget *widget, gpointer user_data);
void Fin_Ventana (GtkWidget *widget, gpointer user_data);
void Genera_Beamer ();
int  Genera_Beamer_reducido ();
void Graba_Ajustes ();
void Imprime_Opcion (FILE * Archivo_Latex, PGresult *res, long double Porcentaje, int pregunta, int opcion);
void Imprime_Opcion_Beamer (FILE * Archivo_Latex, PGresult *res, long double Porcentaje, int pregunta, int opcion);
void Imprime_pregunta (int i, FILE * Archivo_Latex,char * prefijo);
void Imprime_pregunta_Beamer (int i, FILE * Archivo_Latex,char * prefijo, char * tema_descripcion);
void Imprime_Reporte ();
void Interface_Coloring ();
void Inicializa_Tabla_estadisticas ();
void Init_Fields();
void Lista_de_Notas (FILE * Archivo_Latex);
void Lista_de_Preguntas(FILE * Archivo_Latex, GtkWidget *PB, long double base, long double limite);
void Lista_de_Preguntas_Beamer(FILE * Archivo_Latex, GtkWidget *PB, long double base, long double limite);
int  main(int argc, char *argv[]);
void Marca_agua_ajuste(FILE * Archivo_Latex, int i);
void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_print_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_save_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_slides_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked(GtkWidget *widget, gpointer user_data);
void on_CB_ajuste_changed(GtkWidget *widget, gpointer user_data);
void on_CK_encoger_A_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_encoger_B_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_encoger_C_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_encoger_D_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_encoger_E_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_encoger_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_excluir_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_header_encoger_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_header_verbatim_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_no_actualiza_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_slide_A_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_slide_B_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_slide_C_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_slide_D_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_slide_E_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_verbatim_A_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_verbatim_B_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_verbatim_C_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_verbatim_D_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_verbatim_E_toggled(GtkWidget *widget, gpointer user_data);
void on_CK_verbatim_toggled(GtkWidget *widget, gpointer user_data);
void on_EN_examen_activate(GtkWidget *widget, gpointer user_data);
void on_SC_preguntas_value_changed(GtkWidget *widget, gpointer user_data);
void on_SP_examen_activate(GtkWidget *widget, gpointer user_data);
void on_TG_A_toggled(GtkWidget *widget, gpointer user_data);
void on_TG_B_toggled(GtkWidget *widget, gpointer user_data);
void on_TG_C_toggled(GtkWidget *widget, gpointer user_data);
void on_TG_D_toggled(GtkWidget *widget, gpointer user_data);
void on_TG_E_toggled(GtkWidget *widget, gpointer user_data);
void on_WN_ex4010_destroy (GtkWidget *widget, gpointer user_data);
void Prepara_Grafico_Normal(long double media, long double desviacion, long double media_pred, long double desviacion_pred, long double width);
void Prepara_Grafico_Pastel (FILE * Archivo_Latex);
void Prepara_Histograma_Notas();
void Prepara_Histograma_Subtemas();
void Prepara_Histograma_Temas();
void prepara_opciones (char * opciones_frame, int i, int k);
void Read_Only_Fields ();
void Quita_espacios (char * hilera);
void Resumen_de_Banderas (FILE * Archivo_Latex, int modo);
void Update_PB (GtkWidget *PB, long double R);
void Tabla_Datos_Generales (FILE * Archivo_Latex, char * institucion, char * escuela, char * programa, char * materia_descripcion, char * profesor, char * descripcion, char * fecha, char * codigo, long double media_real, long double desviacion_real, long double alfa, long double Rpb, int Beamer_o_reporte);

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX4010.glade", &error))
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
	  
          /* Most fields are READ ONLY */
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
  window1 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex4010"));
  window2 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_procesado"));
  window3 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_generando_beamer"));
  window4 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_generando_analisis"));
  window5 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_error_Beamer"));
  window6 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_revisando_beamer"));
  window7 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_error_encontrado_Beamer"));

  EB_analisis                  = GTK_WIDGET (gtk_builder_get_object (builder, "EB_analisis"));
  FR_analisis                  = GTK_WIDGET (gtk_builder_get_object (builder, "FR_analisis"));

  SP_examen          = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_examen"));
  EN_descripcion            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));
  EN_pre_examen             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_pre_examen"));
  EN_pre_examen_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_pre_examen_descripcion"));
  EN_esquema                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema"));
  EN_esquema_descripcion    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_esquema_descripcion"));
  EN_materia                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia"));
  EN_materia_descripcion    = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia_descripcion"));

  EN_institucion            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_institucion"));
  EN_escuela                = GTK_WIDGET (gtk_builder_get_object (builder, "EN_escuela"));
  EN_programa               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_programa"));
  EN_profesor               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_profesor"));

  EN_fecha                  = GTK_WIDGET (gtk_builder_get_object (builder, "EN_fecha"));
  EN_N_preguntas            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_N_preguntas"));
  EN_N_versiones            = GTK_WIDGET (gtk_builder_get_object (builder, "EN_N_versiones"));
  EN_N_estudiantes          = GTK_WIDGET (gtk_builder_get_object (builder, "EN_N_estudiantes"));

  EB_prediccion             = GTK_WIDGET (gtk_builder_get_object (builder, "EB_prediccion"));
  FR_prediccion             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_prediccion"));
  EN_media_prediccion       = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_prediccion"));
  EN_desviacion_prediccion  = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion_prediccion"));
  EN_alfa_prediccion        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_prediccion"));
  EN_Rpb_prediccion         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb_prediccion"));

  EB_real                   = GTK_WIDGET (gtk_builder_get_object (builder, "EB_real"));
  FR_real                   = GTK_WIDGET (gtk_builder_get_object (builder, "FR_real"));
  EN_media_real             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_media_real"));
  EN_desviacion_real        = GTK_WIDGET (gtk_builder_get_object (builder, "EN_desviacion_real"));
  EN_alfa_real              = GTK_WIDGET (gtk_builder_get_object (builder, "EN_alfa_real"));
  EN_Rpb_real               = GTK_WIDGET (gtk_builder_get_object (builder, "EN_Rpb_real"));

  FR_procesado              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_procesado"));
  EB_grafico                = GTK_WIDGET (gtk_builder_get_object (builder, "EB_grafico"));
  FR_grafico                = GTK_WIDGET (gtk_builder_get_object (builder, "FR_grafico"));

  SP_resolucion = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_resolucion"));
  SP_color      = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_color"));
  SP_rotacion   = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_rotacion"));
	  
  CK_smooth                 = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_smooth"));
  FR_botones                = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));

  EB_preguntas              = GTK_WIDGET (gtk_builder_get_object (builder, "EB_preguntas"));
  FR_preguntas              = GTK_WIDGET (gtk_builder_get_object (builder, "FR_preguntas"));

  EB_ajustes                = GTK_WIDGET (gtk_builder_get_object (builder, "EB_ajustes"));
  FR_ajustes                = GTK_WIDGET (gtk_builder_get_object (builder, "FR_ajustes"));
  EB_formato                = GTK_WIDGET (gtk_builder_get_object (builder, "EB_formato"));
  FR_formato                = GTK_WIDGET (gtk_builder_get_object (builder, "FR_formato"));

  FR_pregunta_actual        = GTK_WIDGET (gtk_builder_get_object (builder, "FR_pregunta_actual"));
  SC_preguntas              = GTK_WIDGET (gtk_builder_get_object (builder, "SC_preguntas"));
  TV_pregunta               = (GtkTextView *) GTK_WIDGET (gtk_builder_get_object (builder, "TV_pregunta"));
  buffer_TV_pregunta        = gtk_text_view_get_buffer(TV_pregunta);

  EB_beamer                 = GTK_WIDGET (gtk_builder_get_object (builder, "EB_beamer"));
  FR_beamer                 = GTK_WIDGET (gtk_builder_get_object (builder, "FR_beamer"));
  CB_estilo                 = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_estilo"));
  CB_color                  = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_color"));
  CB_font                   = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_font"));
  CB_size                   = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_size"));
  CB_aspecto                = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_aspecto"));
  CK_general                = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_general"));
  CK_sin_banderas           = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_sin_banderas"));

  TG_A                      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "TG_A"));
  TG_B                      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "TG_B"));
  TG_C                      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "TG_C"));
  TG_D                      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "TG_D"));
  TG_E                      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "TG_E"));
  CB_ajuste                 = (GtkComboBox *) GTK_WIDGET (gtk_builder_get_object (builder, "CB_ajuste"));
  CK_no_actualiza           = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_no_actualiza"));
  CK_excluir                = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_excluir"));
  CK_encoger                = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_encoger"));
  CK_verbatim               = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_verbatim"));
  CK_header_encoger         = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_header_encoger"));
  CK_header_verbatim        = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_header_verbatim"));
  CK_slide[0]               = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_slide_A"));
  CK_slide[1]               = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_slide_B"));
  CK_slide[2]               = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_slide_C"));
  CK_slide[3]               = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_slide_D"));
  CK_slide[4]               = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_slide_E"));
  CK_encoger_opcion[0]      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_encoger_A"));
  CK_encoger_opcion[1]      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_encoger_B"));
  CK_encoger_opcion[2]      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_encoger_C"));
  CK_encoger_opcion[3]      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_encoger_D"));
  CK_encoger_opcion[4]      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_encoger_E"));
  CK_verbatim_opcion[0]     = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_verbatim_A"));
  CK_verbatim_opcion[1]     = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_verbatim_B"));
  CK_verbatim_opcion[2]     = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_verbatim_C"));
  CK_verbatim_opcion[3]     = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_verbatim_D"));
  CK_verbatim_opcion[4]     = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_verbatim_E"));

  EB_generando_beamer       = GTK_WIDGET (gtk_builder_get_object (builder, "EB_generando_beamer"));
  FR_generando_beamer       = GTK_WIDGET (gtk_builder_get_object (builder, "FR_generando_beamer"));
  EB_generando_analisis     = GTK_WIDGET (gtk_builder_get_object (builder, "EB_generando_analisis"));
  FR_generando_analisis     = GTK_WIDGET (gtk_builder_get_object (builder, "FR_generando_analisis"));
  FR_error_Beamer           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_error_Beamer"));
  EB_revisando_beamer       = GTK_WIDGET (gtk_builder_get_object (builder, "EB_revisando_beamer"));
  FR_revisando_beamer       = GTK_WIDGET (gtk_builder_get_object (builder, "FR_revisando_beamer"));
  FR_error_encontrado_Beamer= GTK_WIDGET (gtk_builder_get_object (builder, "FR_error_encontrado_Beamer"));
  LB_error_encontrado_Beamer= (GtkLabel  *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_error_encontrado_Beamer"));

  BN_save                   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
  BN_slides                 = GTK_WIDGET (gtk_builder_get_object (builder, "BN_slides"));
  BN_print                  = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_undo                   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_terminar               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_ok                     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
  PB_analisis               = GTK_WIDGET (gtk_builder_get_object (builder, "PB_analisis"));
  PB_beamer                 = GTK_WIDGET (gtk_builder_get_object (builder, "PB_beamer"));
  PB_revisando_beamer       = GTK_WIDGET (gtk_builder_get_object (builder, "PB_revisando_beamer"));

  BN_confirma_revision      = GTK_WIDGET (gtk_builder_get_object (builder, "BN_confirma_revision"));
  BN_cancela_revision       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_cancela_revision"));
  BN_error_encontrado_Beamer= GTK_WIDGET (gtk_builder_get_object (builder, "BN_error_encontrado_Beamer"));
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_descripcion           , FALSE);
  gtk_widget_set_can_focus(EN_pre_examen            , FALSE);
  gtk_widget_set_can_focus(EN_pre_examen_descripcion, FALSE);
  gtk_widget_set_can_focus(EN_esquema               , FALSE);
  gtk_widget_set_can_focus(EN_esquema_descripcion   , FALSE);
  gtk_widget_set_can_focus(EN_materia               , FALSE);
  gtk_widget_set_can_focus(EN_materia_descripcion   , FALSE);
  gtk_widget_set_can_focus(EN_fecha                 , FALSE);
  gtk_widget_set_can_focus(EN_N_preguntas           , FALSE);
  gtk_widget_set_can_focus(EN_N_versiones           , FALSE);
  gtk_widget_set_can_focus(EN_N_estudiantes         , FALSE);
  gtk_widget_set_can_focus(EN_profesor              , FALSE);
  gtk_widget_set_can_focus(EN_institucion           , FALSE);
  gtk_widget_set_can_focus(EN_escuela               , FALSE);
  gtk_widget_set_can_focus(EN_programa              , FALSE);
  gtk_widget_set_can_focus(EN_media_prediccion      , FALSE);
  gtk_widget_set_can_focus(EN_desviacion_prediccion , FALSE);
  gtk_widget_set_can_focus(EN_alfa_prediccion       , FALSE);
  gtk_widget_set_can_focus(EN_Rpb_prediccion        , FALSE);
  gtk_widget_set_can_focus(EN_media_real            , FALSE);
  gtk_widget_set_can_focus(EN_desviacion_real       , FALSE);
  gtk_widget_set_can_focus(EN_alfa_real             , FALSE);
  gtk_widget_set_can_focus(EN_Rpb_real              , FALSE);
  gtk_widget_set_can_focus(GTK_WIDGET(TV_pregunta)  , FALSE);
}


void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_analisis,   GTK_STATE_NORMAL,   &color);

  gdk_color_parse (SECONDARY_AREA, &color);
  gtk_widget_modify_bg(EB_ajustes,     GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(EB_formato,     GTK_STATE_NORMAL,   &color);

  gdk_color_parse (THIRD_AREA, &color);
  gtk_widget_modify_bg(EB_preguntas,   GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(EB_beamer,      GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(EB_grafico,     GTK_STATE_NORMAL,   &color);

  gdk_color_parse (SPECIAL_AREA_1, &color);
  gtk_widget_modify_bg(EB_prediccion,  GTK_STATE_NORMAL,   &color);  

  gdk_color_parse (SPECIAL_AREA_2, &color);
  gtk_widget_modify_bg(EB_real,             GTK_STATE_NORMAL,   &color);

  gdk_color_parse (IMPORTANT_WINDOW, &color);
  gtk_widget_modify_bg(window2,  GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(window5,  GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(window7,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (IMPORTANT_FR, &color);
  gtk_widget_modify_bg(FR_procesado,               GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_error_Beamer,            GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_error_encontrado_Beamer, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_analisis,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_preguntas,    GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_beamer,       GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones,      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_grafico,      GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_generando_analisis,  GTK_STATE_NORMAL,   &color);  
  gtk_widget_modify_bg(FR_generando_beamer,    GTK_STATE_NORMAL,   &color);  
  gtk_widget_modify_bg(FR_revisando_beamer,    GTK_STATE_NORMAL,   &color);  
  gtk_widget_modify_bg(FR_ajustes,      GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_formato,      GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_prediccion,  GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_real,        GTK_STATE_NORMAL,   &color);
  gtk_widget_modify_bg(FR_pregunta_actual, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_examen), GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,                   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_slides,                 GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,                   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,               GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,                  GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_cancela_revision,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_error_encontrado_Beamer,GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_confirma_revision,      GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_cancela_revision,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,                     GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,                   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_slides,                 GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,                   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar,               GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,                  GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_cancela_revision,       GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_error_encontrado_Beamer,GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_confirma_revision,      GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_cancela_revision,       GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,                     GTK_STATE_ACTIVE, &color);

  gdk_color_parse (PROCESSING_PB, &color);
  gtk_widget_modify_bg(GTK_WIDGET(PB_analisis),         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(GTK_WIDGET(PB_beamer),           GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(GTK_WIDGET(PB_revisando_beamer), GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (PROCESSING_WINDOW, &color);
  gtk_widget_modify_bg(EB_generando_analisis,  GTK_STATE_NORMAL,   &color);  
  gtk_widget_modify_bg(EB_generando_beamer,    GTK_STATE_NORMAL,   &color);  
  gtk_widget_modify_bg(EB_revisando_beamer,    GTK_STATE_NORMAL,   &color);  
}

void Init_Fields()
{
   char Examen [EXAMEN_SIZE];
   char hilera [100];
   int month, year, day;
   PGresult *res;
   int i, Last;
   GdkColor color;

   /* Recupera el examen con código más alto */
   res = PQEXEC(DATABASE, "SELECT codigo_examen from EX_examenes order by codigo_examen DESC limit 1");
   Last = 0;
   if (PQntuples(res))
      {
       Last = atoi (PQgetvalue (res, 0, 0));
      }
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

   if (preguntas)            free (preguntas);
   preguntas            = NULL;
   if (resumen_tema)         free (resumen_tema);
   resumen_tema         = NULL;
   if (resumen_tema_subtema) free (resumen_tema_subtema);
   resumen_tema_subtema = NULL;
   if (versiones)
      {
       for (i=0;i<N_versiones;i++) free (versiones[i].preguntas);

       free (versiones);
       versiones            = NULL;
      }

   gtk_entry_set_text(GTK_ENTRY(EN_fecha), "\0");
   N_preguntas = N_versiones = N_estudiantes = 0;
   sprintf (hilera,"%7d", N_preguntas);
   gtk_entry_set_text(GTK_ENTRY(EN_N_preguntas),   hilera);
   gtk_entry_set_text(GTK_ENTRY(EN_N_versiones),   hilera);
   gtk_entry_set_text(GTK_ENTRY(EN_N_estudiantes), hilera);

   gtk_entry_set_text(GTK_ENTRY(EN_descripcion),            "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_pre_examen),             "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_pre_examen_descripcion), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema),                "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion),    "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia),                "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion),    "\0");
 
   gtk_entry_set_text(GTK_ENTRY(EN_institucion), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_escuela),     "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_programa),    "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_profesor),    "\0");

   gtk_widget_set_sensitive(GTK_WIDGET(SP_examen), 1);
   gtk_widget_set_sensitive(EN_pre_examen            , 0);
   gtk_widget_set_sensitive(EN_pre_examen_descripcion, 0);
   gtk_widget_set_sensitive(EN_esquema               , 0);
   gtk_widget_set_sensitive(EN_esquema_descripcion   , 0);
   gtk_widget_set_sensitive(EN_materia               , 0);
   gtk_widget_set_sensitive(EN_materia_descripcion   , 0);
   gtk_widget_set_sensitive(EN_fecha                 , 0);
   gtk_widget_set_sensitive(EN_N_preguntas           , 0);
   gtk_widget_set_sensitive(EN_N_versiones           , 0);
   gtk_widget_set_sensitive(EN_N_estudiantes         , 0);

   gtk_widget_set_sensitive(EN_institucion           , 0);
   gtk_widget_set_sensitive(EN_escuela               , 0);
   gtk_widget_set_sensitive(EN_programa              , 0);
   gtk_widget_set_sensitive(EN_profesor              , 0);
   gtk_widget_set_sensitive(EN_descripcion           , 0);

   gtk_widget_set_sensitive(FR_prediccion            , 0);
   gtk_widget_set_sensitive(FR_real                  , 0);
   gtk_widget_set_sensitive(FR_grafico               , 0);
   gtk_widget_set_sensitive(FR_preguntas             , 0);
   gtk_widget_set_sensitive(FR_beamer                , 0);

   gtk_spin_button_set_value (SP_resolucion, 10);
   gtk_spin_button_set_value (SP_color,      7);
   gtk_spin_button_set_value (SP_rotacion,   30);

   gtk_toggle_button_set_active(CK_smooth, TRUE);
   gtk_range_set_range (GTK_RANGE(SC_preguntas),  (gdouble) 0.0, (gdouble) 1.0);
   gtk_range_set_value (GTK_RANGE(SC_preguntas),  (gdouble) 0.0);
   gtk_text_buffer_set_text(buffer_TV_pregunta, "\0", -1);
   gtk_toggle_button_set_active(TG_A, FALSE);     
   gtk_toggle_button_set_active(TG_B, FALSE);     
   gtk_toggle_button_set_active(TG_C, FALSE);     
   gtk_toggle_button_set_active(TG_D, FALSE);     
   gtk_toggle_button_set_active(TG_E, FALSE);  
   gtk_combo_box_set_active    (CB_ajuste, -1);
   gtk_toggle_button_set_active(CK_no_actualiza, FALSE);
   gtk_toggle_button_set_active(CK_excluir,      FALSE);
   gtk_toggle_button_set_active(CK_encoger,      FALSE);
   gtk_toggle_button_set_active(CK_verbatim,     FALSE);
   gtk_toggle_button_set_active(CK_header_encoger,  FALSE);
   gtk_toggle_button_set_active(CK_header_verbatim, FALSE);

   for (i=0;i<5;i++)
       {
        gtk_toggle_button_set_active(CK_slide           [i], FALSE);
        gtk_toggle_button_set_active(CK_encoger_opcion  [i], FALSE);
        gtk_toggle_button_set_active(CK_verbatim_opcion [i], FALSE);
        gtk_widget_set_sensitive    (GTK_WIDGET(CK_encoger_opcion  [i]), 0);
        gtk_widget_set_sensitive    (GTK_WIDGET(CK_verbatim_opcion [i]), 0);
       }

   gtk_widget_set_sensitive(BN_save,        0);
   gtk_widget_set_sensitive(BN_slides,      0);
   gtk_widget_set_sensitive(BN_print,       0);
   gtk_widget_set_sensitive(BN_undo,        1);

   gdk_color_parse (SECONDARY_AREA, &color);
   gtk_widget_modify_bg(EB_ajustes,     GTK_STATE_NORMAL,   &color);

   gtk_combo_box_set_active    (CB_estilo,  parametros.Beamer_Estilo);
   gtk_combo_box_set_active    (CB_color,   parametros.Beamer_Color);
   gtk_combo_box_set_active    (CB_font,    parametros.Beamer_Font);
   gtk_combo_box_set_active    (CB_size,    parametros.Beamer_Size);
   gtk_combo_box_set_active    (CB_aspecto, parametros.Beamer_Aspecto);

   gtk_toggle_button_set_active(CK_general, FALSE);
   gtk_toggle_button_set_active(CK_sin_banderas, FALSE);

   if (Last > 1)
      gtk_widget_grab_focus (GTK_WIDGET(SP_examen));
   else
      Cambio_Examen ();
}

void Cambio_Examen()
{
  char examen[10];
  char hilera      [200];
  char Descripcion [2*DESCRIPCION_SIZE]         = "*** Examen no está registrado ***";
  char PG_command  [4000];
  long double media_prediccion, desviacion_prediccion, alfa_prediccion, Rpb_prediccion;
  PGresult *res, *res_aux, *res_aux_2;
  int i, j, k;

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  sprintf (PG_command,"SELECT codigo_examen, descripcion, pre_examen, profesor, EX_examenes.materia, institucion, escuela, programa, esquema, descripcion_pre_examen, descripcion_esquema, descripcion_materia, nombre, prediccion_media, prediccion_desviacion, prediccion_alfa, prediccion_Rpb, Year, month, day, ejecutado from EX_examenes, EX_pre_examenes, EX_esquemas, BD_materias, BD_personas where codigo_examen = '%s' and pre_examen = codigo_pre_examen and esquema = codigo_esquema and EX_examenes.materia = codigo_materia and codigo_tema = '          ' and codigo_subtema = '          ' and profesor = codigo_persona", examen);

  res = PQEXEC(DATABASE, PG_command);

  if (PQntuples(res))
     {
      gtk_widget_set_sensitive(GTK_WIDGET(SP_examen), 0);

      gtk_widget_set_sensitive(EN_pre_examen            , 1);
      gtk_widget_set_sensitive(EN_pre_examen_descripcion, 1);
      gtk_widget_set_sensitive(EN_esquema               , 1);
      gtk_widget_set_sensitive(EN_esquema_descripcion   , 1);
      gtk_widget_set_sensitive(EN_materia               , 1);
      gtk_widget_set_sensitive(EN_materia_descripcion   , 1);
      gtk_widget_set_sensitive(EN_fecha                 , 1);
      gtk_widget_set_sensitive(EN_N_preguntas           , 1);
      gtk_widget_set_sensitive(EN_N_versiones           , 1);
      gtk_widget_set_sensitive(EN_N_estudiantes         , 1);
      gtk_widget_set_sensitive(EN_institucion           , 1);
      gtk_widget_set_sensitive(EN_escuela               , 1);
      gtk_widget_set_sensitive(EN_programa              , 1);
      gtk_widget_set_sensitive(EN_profesor              , 1);
      gtk_widget_set_sensitive(EN_descripcion           , 1);

      gtk_widget_set_sensitive(FR_prediccion            , 1);
      gtk_widget_set_sensitive(FR_real                  , 1);
      gtk_widget_set_sensitive(FR_ajustes               , 1);
      gtk_widget_set_sensitive(FR_grafico               , 1);
      gtk_widget_set_sensitive(FR_preguntas             , 1);
      gtk_widget_set_sensitive(FR_beamer                , 1);

      gtk_widget_set_sensitive(BN_save,        1);
      gtk_widget_set_sensitive(BN_slides,      1);
      gtk_widget_set_sensitive(BN_print,       1);

      sprintf (PG_command,"SELECT DISTINCT version from EX_examenes_preguntas where examen = '%s' order by version", examen);
      res_aux = PQEXEC(DATABASE, PG_command);
      N_versiones = PQntuples(res_aux);
      PQclear(res_aux);

      sprintf (PG_command,"SELECT * from EX_examenes_preguntas where examen = '%s'", examen);
      res_aux = PQEXEC(DATABASE, PG_command);
      N_preguntas = PQntuples(res_aux);
      N_preguntas = N_preguntas/N_versiones;
      PQclear(res_aux);

      strcpy (Descripcion,                                     PQgetvalue (res, 0, 1));
      gtk_entry_set_text(GTK_ENTRY(EN_pre_examen),             PQgetvalue (res, 0, 2));
      gtk_entry_set_text(GTK_ENTRY(EN_pre_examen_descripcion), PQgetvalue (res, 0, 9));
      gtk_entry_set_text(GTK_ENTRY(EN_esquema),                PQgetvalue (res, 0, 8));
      gtk_entry_set_text(GTK_ENTRY(EN_esquema_descripcion),    PQgetvalue (res, 0, 10));
      gtk_entry_set_text(GTK_ENTRY(EN_profesor),               PQgetvalue (res, 0, 12));
      gtk_entry_set_text(GTK_ENTRY(EN_materia),                PQgetvalue (res, 0, 4));
      gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion),    PQgetvalue (res, 0, 11));
      gtk_entry_set_text(GTK_ENTRY(EN_institucion),            PQgetvalue (res, 0, 5));
      gtk_entry_set_text(GTK_ENTRY(EN_escuela),                PQgetvalue (res, 0, 6));
      gtk_entry_set_text(GTK_ENTRY(EN_programa),               PQgetvalue (res, 0, 7));

      sprintf (hilera,"%7d", N_preguntas);
      gtk_entry_set_text(GTK_ENTRY(EN_N_preguntas), hilera);
      sprintf (hilera,"%7d", N_versiones);
      gtk_entry_set_text(GTK_ENTRY(EN_N_versiones), hilera);

      media_prediccion = atof (PQgetvalue (res, 0, 13));
      sprintf (hilera,"%8.3Lf", media_prediccion);
      gtk_entry_set_text(GTK_ENTRY(EN_media_prediccion), hilera);

      desviacion_prediccion = atof (PQgetvalue (res, 0, 14));
      sprintf (hilera,"%8.3Lf", desviacion_prediccion);
      gtk_entry_set_text(GTK_ENTRY(EN_desviacion_prediccion), hilera);

      alfa_prediccion = atof (PQgetvalue (res, 0, 15));
      sprintf (hilera,"%7.4Lf", alfa_prediccion);
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_prediccion), hilera);

      Rpb_prediccion = atof (PQgetvalue (res, 0, 16));
      sprintf (hilera,"%7.4Lf", Rpb_prediccion);
      gtk_entry_set_text(GTK_ENTRY(EN_Rpb_prediccion), hilera);

      sprintf (hilera,"%02d/%02d/%02d", atoi(PQgetvalue (res, 0, 19)),atoi(PQgetvalue (res, 0, 18)),atoi(PQgetvalue (res, 0, 17)));
      gtk_entry_set_text(GTK_ENTRY(EN_fecha), hilera);

      preguntas            = (struct PREGUNTA *) malloc ((sizeof (struct PREGUNTA) * N_preguntas));
      resumen_tema         = (struct PREGUNTA *) malloc ((sizeof (struct PREGUNTA) * N_preguntas));
      resumen_tema_subtema = (struct PREGUNTA *) malloc ((sizeof (struct PREGUNTA) * N_preguntas));

      Construye_versiones           (examen);

      Carga_preguntas_examen        ();

      Actualiza_Porcentajes         ();

      Inicializa_Tabla_estadisticas ();
      Calcula_estadisticas_examen   ();

      if (*PQgetvalue (res, 0, 20) == 't')
	 {
          gtk_widget_set_sensitive(FR_ajustes, 0);
          gtk_widget_set_sensitive(window1, 0);
          gtk_widget_show         (window2);
	 }
      else
	 {
          gtk_widget_set_sensitive(FR_ajustes, 1);
	 }

      if (N_estudiantes)
         gtk_widget_grab_focus   (BN_print);
      else
         gtk_widget_grab_focus   (BN_undo);
     }
  else
     gtk_widget_grab_focus   (GTK_WIDGET(SP_examen));

  PQclear(res);

  gtk_entry_set_text(GTK_ENTRY(EN_descripcion), Descripcion);
}

void Construye_versiones (char * examen)
{
  int i, j, k;
  char PG_command [3000];
  PGresult *res, *res_aux;
  int respuesta_original;
  int opcion_1,opcion_2,opcion_3,opcion_4,opcion_5;

  sprintf (PG_command,"SELECT DISTINCT version from EX_examenes_preguntas where examen = '%s' order by version", examen);
  res = PQEXEC(DATABASE, PG_command);
  N_versiones = PQntuples(res);
  versiones = (struct VERSION *) malloc (sizeof(struct VERSION) * N_versiones);

  for (i=0;i<N_versiones;i++)
      {
       strcpy (versiones[i].codigo, PQgetvalue (res, i, 0));

       sprintf (PG_command,"SELECT respuesta, opcion_1, opcion_2, opcion_3, opcion_4, opcion_5, codigo_pregunta from ex_examenes_preguntas, bd_texto_preguntas where examen = '%.5s' and version = '%.4s' and codigo_pregunta = codigo_unico_pregunta order by posicion", examen, PQgetvalue (res, i, 0));
       res_aux = PQEXEC(DATABASE, PG_command);

       N_preguntas = PQntuples(res_aux);

       versiones[i].preguntas = (struct PREGUNTA_VERSION *)  malloc(sizeof(struct PREGUNTA_VERSION)   * (N_preguntas));

       for (j=0; j<N_preguntas; j++)
	   {
  	    respuesta_original =     *PQgetvalue (res_aux, j, 0) - 'A';
	    opcion_1           = atoi(PQgetvalue (res_aux, j, 1));
	    opcion_2           = atoi(PQgetvalue (res_aux, j, 2));
	    opcion_3           = atoi(PQgetvalue (res_aux, j, 3));
	    opcion_4           = atoi(PQgetvalue (res_aux, j, 4));
	    opcion_5           = atoi(PQgetvalue (res_aux, j, 5));

	    if (respuesta_original == opcion_1) versiones[i].preguntas [j].respuesta = 'A';
	    if (respuesta_original == opcion_2) versiones[i].preguntas [j].respuesta = 'B';
	    if (respuesta_original == opcion_3) versiones[i].preguntas [j].respuesta = 'C';
	    if (respuesta_original == opcion_4) versiones[i].preguntas [j].respuesta = 'D';
	    if (respuesta_original == opcion_5) versiones[i].preguntas [j].respuesta = 'E';

	    versiones[i].preguntas [j].orden_version [0] = opcion_1;
	    versiones[i].preguntas [j].orden_version [1] = opcion_2;
	    versiones[i].preguntas [j].orden_version [2] = opcion_3;
	    versiones[i].preguntas [j].orden_version [3] = opcion_4;
	    versiones[i].preguntas [j].orden_version [4] = opcion_5;

	    strcpy (versiones[i].preguntas [j].codigo, PQgetvalue (res_aux, j, 6));
	   }

       PQclear(res_aux);
      }

  PQclear(res);
}

void Actualiza_Porcentajes ()
{ /* En caso que se haya actualizado la respuesta correcta de alguna pregunta */
  char PG_command          [2000];
  PGresult *res, *res_aux;
  int i, j, k;
  int N_estudiantes;
  long double Porcentaje, Porcentaje_ajustado;
  char examen[10];
  int N_correctas, N_ajustado, M_ajustado;

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  sprintf (PG_command,"SELECT estudiante, version, respuestas from EX_examenes_respuestas where examen = '%s' order by estudiante", examen);
  res = PQEXEC(DATABASE, PG_command);
  N_estudiantes = PQntuples(res);

  for (i=0; i<N_estudiantes; i++)
      {
       Calcula_Notas (PQgetvalue(res, i, 1),
		      PQgetvalue(res, i, 2),
		      &N_correctas, &N_ajustado, &M_ajustado);

       Porcentaje          = (long double) N_correctas/N_preguntas * 100.0;
       if (M_ajustado)
          Porcentaje_ajustado = (long double) N_ajustado /M_ajustado  * 100.0;
       else
	  Porcentaje_ajustado = 0.0;

       sprintf (PG_command,"UPDATE ex_examenes_respuestas set correctas = %d, porcentaje = %Lf, nota_ajustada = %Lf, nota_final = %Lf where examen = '%s' and estudiante = %d", 
		N_correctas, Porcentaje, Porcentaje_ajustado, Porcentaje_ajustado, 
	        examen, i+1);
       res_aux = PQEXEC(DATABASE, PG_command);
      }
}

void Carga_preguntas_examen ()
{
  int i, j, k;
  char PG_command  [3000];
  PGresult *res, *res_aux;
  gchar *materia;
  char examen [10];
  char ejercicio_actual[10];
  struct PREGUNTA temporal;
  unsigned int ajustes_Beamer;

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);

  sprintf (PG_command,"SELECT tema, subtema, ejercicio, secuencia, codigo_pregunta, dificultad, BD_texto_preguntas.respuesta, texto_pregunta, nombre from EX_examenes_preguntas, BD_ejercicios, BD_texto_preguntas, BD_estadisticas_preguntas, BD_texto_ejercicios, BD_personas where examen = '%s' and codigo_ejercicio = ejercicio and materia = '%s' and codigo_pregunta = pregunta and codigo_unico_pregunta = pregunta and consecutivo_texto = texto_ejercicio and codigo_persona = autor order by version, tema, subtema, ejercicio, secuencia", examen, materia);
  res = PQEXEC(DATABASE, PG_command);

  N_ajustes = 0;
  for (i=0;i<N_preguntas;i++)
      {
       strcpy (preguntas[i].tema,      PQgetvalue (res, i, 0));
       strcpy (preguntas[i].subtema,   PQgetvalue (res, i, 1));
       strcpy (preguntas[i].ejercicio, PQgetvalue (res, i, 2));

       preguntas[i].secuencia =   atoi(PQgetvalue (res, i, 3));
       strcpy (preguntas[i].pregunta,  PQgetvalue (res, i, 4));

       preguntas[i].previo      = atof(PQgetvalue (res, i, 5));
       preguntas[i].correcta    =     *PQgetvalue (res, i, 6);
       strncpy (preguntas[i].texto_pregunta, PQgetvalue (res, i, 7), TEXTO_PREGUNTA_SIZE-1);
       preguntas[i].texto_pregunta[TEXTO_PREGUNTA_SIZE-1] = '\0';
       strcpy (preguntas[i].autor,     PQgetvalue (res, i, 8));

       preguntas[i].desviacion  = sqrt(preguntas[i].previo * (1.0 - preguntas[i].previo));

       preguntas[i].ajuste               = 0;
       preguntas[i].revision_especial    = 0;
       preguntas[i].actualizar           = 1;
       preguntas[i].excluir              = 0;
       preguntas[i].encoger              = 0;
       preguntas[i].verbatim             = 0;
       preguntas[i].header_encoger       = 0;
       preguntas[i].header_verbatim      = 0;

       for (j=0;j<5;j++)
	   {
            preguntas[i].correctas_nuevas [j] = 0;
            preguntas[i].slide            [j] = 0;
            preguntas[i].encoger_opcion   [j] = 0;
            preguntas[i].verbatim_opcion  [j] = 0;
	   }

       sprintf (PG_command,"SELECT * from EX_examenes_ajustes where examen = '%s' and codigo_pregunta = '%s'", examen, PQgetvalue (res, i, 4));
       res_aux = PQEXEC(DATABASE, PG_command);
       if (PQntuples(res_aux))
	  {
           preguntas[i].ajuste               = atoi(PQgetvalue (res_aux, 0, 3));
           preguntas[i].revision_especial    = preguntas[i].ajuste & 0xFF;
           ajustes_Beamer                    = preguntas[i].ajuste >> 8;
           preguntas[i].excluir              = ajustes_Beamer & 0x01; ajustes_Beamer >>= 1;
           preguntas[i].encoger              = ajustes_Beamer & 0x01; ajustes_Beamer >>= 1;
           preguntas[i].verbatim             = ajustes_Beamer & 0x01; ajustes_Beamer >>= 1;
           preguntas[i].header_encoger       = ajustes_Beamer & 0x01; ajustes_Beamer >>= 1;
           preguntas[i].header_verbatim      = ajustes_Beamer & 0x01; ajustes_Beamer >>= 1;
	   for (j=0; j<5;j++)
	       {
                preguntas[i].slide [j]           = ajustes_Beamer & 0x01; ajustes_Beamer >>= 1;
                preguntas[i].encoger_opcion [j]  = ajustes_Beamer & 0x01; ajustes_Beamer >>= 1;
                preguntas[i].verbatim_opcion [j] = ajustes_Beamer & 0x01; ajustes_Beamer >>= 1;
	       }

           preguntas[i].correctas_nuevas [0] = atoi(PQgetvalue (res_aux, 0, 4));
           preguntas[i].correctas_nuevas [1] = atoi(PQgetvalue (res_aux, 0, 5));
           preguntas[i].correctas_nuevas [2] = atoi(PQgetvalue (res_aux, 0, 6));
           preguntas[i].correctas_nuevas [3] = atoi(PQgetvalue (res_aux, 0, 7));
           preguntas[i].correctas_nuevas [4] = atoi(PQgetvalue (res_aux, 0, 8));
           preguntas[i].actualizar           = atoi(PQgetvalue (res_aux, 0, 9));
	   if (preguntas[i].revision_especial) N_ajustes++;
	  }
       PQclear(res_aux);

       sprintf (PG_command,"SELECT orden from BD_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", materia, PQgetvalue (res, i, 0));
       res_aux = PQEXEC(DATABASE, PG_command);
       preguntas[i].orden_tema = atoi(PQgetvalue (res_aux, 0, 0));
       PQclear(res_aux);

       sprintf (PG_command,"SELECT orden from BD_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", materia, PQgetvalue (res, i, 0), PQgetvalue (res, i, 1));
       res_aux = PQEXEC(DATABASE, PG_command);
       preguntas[i].orden_subtema = atoi(PQgetvalue (res_aux, 0, 0));
       PQclear(res_aux);
      }
  PQclear(res);

  for (i=N_preguntas; i >= 0; i--)
      for (j=0; j < (i-1); j++)
	  {
	   if ( (preguntas[j].orden_tema > preguntas[j+1].orden_tema)         ||
	       ((preguntas[j].orden_tema == preguntas[j+1].orden_tema) && 
                (preguntas[j].orden_subtema  > preguntas[j+1].orden_subtema)) ||
	       ((preguntas[j].orden_tema == preguntas[j+1].orden_tema) &&
                (preguntas[j].orden_subtema == preguntas[j+1].orden_subtema) &&
		strcmp(preguntas[j].ejercicio, preguntas[j+1].ejercicio) > 0))
	      {
	       temporal       = preguntas[j+1];
               preguntas[j+1] = preguntas[j];
               preguntas[j]   = temporal;
	      }
	  }

  g_free (materia);
}

void Inicializa_Tabla_estadisticas ()
{
  int i, j;
  char ejercicio_actual[10];
  int inicio_actual;

  inicio_actual = 0;
  strcpy (ejercicio_actual, preguntas[0].ejercicio);

  for (i=0;i<N_preguntas;i++)
      {
       if (strcmp(ejercicio_actual, preguntas[i].ejercicio) != 0)
	  { /* Cambio de ejercicio */
	   strcpy(ejercicio_actual, preguntas[i].ejercicio);
	   for (j=inicio_actual; j < i; j++)
               preguntas[j].grupo_final = i - 1;
	   inicio_actual = i;
	  }
       preguntas[i].grupo_inicio = inicio_actual;

       preguntas[i].buenos      = 0;
       preguntas[i].malos       = 0;
       for (j=0;j<5;j++)
	   {
            preguntas[i].acumulado_opciones [j] = 0;
            preguntas[i].suma_seleccion     [j] = 0.0;
            preguntas[i].Rpb_opcion         [j] = 0.0;
	   }

       preguntas[i].suma_buenos = 0.0;
       preguntas[i].suma_malos  = 0.0;
      }

  for (i=inicio_actual; i < N_preguntas; i++)
      preguntas[i].grupo_final = N_preguntas - 1;
}

void Calcula_estadisticas_examen ()
{
  int i,j, k, N, version_actual;
  int buenas, malas;
  char PG_command  [3000];
  PGresult *res, *res_aux, *res_aux_2;
  char examen[10];
  char hilera[100];
  long double media, diferencia, varianza, desviacion, varianza_pregunta, desviacion_pregunta, suma_varianzas, alfa, Rpb;
  long double media_buenos, media_malos;
  long double media_sin, varianza_sin, alfa_sin;
  long double nota;
  char tema_actual         [CODIGO_TEMA_SIZE    + 1];
  char tema_subtema_actual [CODIGO_SUBTEMA_SIZE + 1];
  int buenas_tema, malas_tema, buenas_subtema, malas_subtema;
  struct PREGUNTA temporal;
  int opcion_original [5];

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  for (i=0; i<CATEGORIAS; i++) Frecuencias[i] = 0;

  sprintf (PG_command,"SELECT porcentaje, version, respuestas, estudiante, nota_ajustada from EX_examenes_respuestas where examen = '%s' order by estudiante", examen);
  res = PQEXEC(DATABASE, PG_command);
  N_estudiantes = PQntuples(res);

  sprintf (hilera,"%7d", N_estudiantes);
  gtk_entry_set_text(GTK_ENTRY(EN_N_estudiantes), hilera);

  if (N_estudiantes <= 1)
     {
      gtk_widget_set_sensitive(FR_real,    0);
      gtk_widget_set_sensitive(FR_ajustes, 0);
      gtk_widget_set_sensitive(BN_save,    0);
      gtk_widget_set_sensitive(BN_slides,  0);
      gtk_widget_set_sensitive(BN_print,   0);
     }
  else
     {
      gtk_range_set_range (GTK_RANGE(SC_preguntas),  (gdouble) 1.0, (gdouble) N_preguntas);
      gtk_range_set_value (GTK_RANGE(SC_preguntas),  (gdouble) 1.0);

      media = 0.0;
      Nota_minima          = 100.0;
      Nota_maxima          = 0.0;
      Nota_minima_ajustada = 100.0;
      Nota_maxima_ajustada = 0.0;
      for (i=0;i<N_estudiantes;i++)
	  {
	   nota = atof(PQgetvalue (res, i, 0));
	   media += nota;

	   if (nota < Nota_minima) Nota_minima = nota;
	   if (nota > Nota_maxima) Nota_maxima = nota;

   	   j=nota/(100.0/(long double) CATEGORIAS);
	   Frecuencias[j]++;

	   nota = atof(PQgetvalue (res, i, 4));
	   if (nota < Nota_minima_ajustada) Nota_minima_ajustada = nota;
	   if (nota > Nota_maxima_ajustada) Nota_maxima_ajustada = nota;
	  }

      media /= N_estudiantes;
      sprintf (hilera,"%8.3Lf", media);
      gtk_entry_set_text(GTK_ENTRY(EN_media_real), hilera);

      varianza = 0.0;
      for (i=0;i<N_estudiantes;i++)
	  {
	   diferencia = media - atof(PQgetvalue (res, i, 0));
	   varianza += (diferencia * diferencia);
	  }

      varianza /= N_estudiantes;

      desviacion = sqrt(varianza);
      sprintf (hilera,"%8.3Lf", desviacion);
      gtk_entry_set_text(GTK_ENTRY(EN_desviacion_real), hilera);

      for (i=0; i<N_estudiantes; i++)
	  {
           sprintf (PG_command,"SELECT ejercicio, secuencia, opcion_1, opcion_2, opcion_3, opcion_4, opcion_5 from EX_examenes_preguntas where examen = '%s' and version = '%s' order by posicion", examen, PQgetvalue (res, i, 1));
           res_aux = PQEXEC(DATABASE, PG_command);
           N = PQntuples(res_aux);

           for (version_actual=0; (version_actual < N_versiones) && strcmp (versiones[version_actual].codigo, PQgetvalue(res, i, 1)) != 0; version_actual++);

	   buenas = malas = 0;
	   for (j=0; j < N_preguntas; j++)
	       {
  	        opcion_original [0] =atoi(PQgetvalue (res_aux, j, 2));
	        opcion_original [1] =atoi(PQgetvalue (res_aux, j, 3));
	        opcion_original [2] =atoi(PQgetvalue (res_aux, j, 4));
	        opcion_original [3] =atoi(PQgetvalue (res_aux, j, 5));
	        opcion_original [4] =atoi(PQgetvalue (res_aux, j, 6));

		if (versiones[version_actual].preguntas[j].respuesta == PQgetvalue (res, i, 2)[j])
		   {
		    buenas++;
		   }
		else
		   {
		    malas++;
		   }

		Actualice_Estadisticas (versiones[version_actual].preguntas[j].respuesta == PQgetvalue (res, i, 2)[j],
                                        PQgetvalue (res_aux, j, 0), atoi(PQgetvalue (res_aux, j, 1)), (long double)atof(PQgetvalue (res, i, 0)),
					PQgetvalue (res, i, 2)[j], opcion_original);
	       }

           PQclear(res_aux);
	  }

      Rpb = 0.0;
      suma_varianzas = 0.0;
      for (i=0; i <N_preguntas; i++)
	  {
  	   preguntas[i].porcentaje = (long double)preguntas[i].buenos/N_estudiantes;

           varianza_pregunta = preguntas[i].porcentaje * (1.0 - preguntas[i].porcentaje);
	   desviacion_pregunta = sqrt(varianza_pregunta);
           suma_varianzas   += varianza_pregunta;

	   media_buenos = 0.0;
	   if (preguntas[i].buenos) media_buenos = preguntas[i].suma_buenos/preguntas[i].buenos;

	   media_malos = 0.0;
	   if (preguntas[i].malos)  media_malos  = preguntas[i].suma_malos/preguntas[i].malos;

	   preguntas[i].Rpb  = (media_buenos - media_malos) * (desviacion_pregunta/desviacion);
	   Rpb += preguntas[i].Rpb;

 	   for (j=0;j<5;j++)
	       {
		if ((preguntas[i].acumulado_opciones[j] == 0) || (preguntas[i].acumulado_opciones[j] == N_estudiantes))
		   {
		    preguntas[i].Rpb_opcion [j] = 0.0;
		   }
	        else
	           {
		    preguntas[i].Rpb_opcion [j] = (((preguntas[i].suma_seleccion[j]/preguntas[i].acumulado_opciones[j]) -
						     ((preguntas[i].suma_buenos + preguntas[i].suma_malos - preguntas[i].suma_seleccion[j])/
                                                      (N_estudiantes-preguntas[i].acumulado_opciones[j])))/ 
                                                    desviacion) *
		                                  sqrt((     (long double) preguntas[i].acumulado_opciones[j]/N_estudiantes) * 
                                                        (1- ((long double) preguntas[i].acumulado_opciones[j]/N_estudiantes)));
	           }
	       }

	  }

      alfa = ((long double) N_preguntas/ (long double) (N_preguntas - 1)) * (1.0 - (suma_varianzas/varianza));

      sprintf (hilera,"%8.3Lf", alfa);
      gtk_entry_set_text(GTK_ENTRY(EN_alfa_real), hilera);

      Rpb /= N_preguntas;
      sprintf (hilera,"%8.3Lf", Rpb);
      gtk_entry_set_text(GTK_ENTRY(EN_Rpb_real), hilera);

      for (i=0; i <N_preguntas; i++)
	  {
	   media_sin = 0.0;

	   for (j=0; j<N_estudiantes;j++)
	       {/* calcular media ignorando pregunta i */
                sprintf (PG_command,"SELECT version from EX_versiones where examen = '%s' and version <= '%s'", examen, PQgetvalue(res, j, 1));
                res_aux_2 = PQEXEC(DATABASE, PG_command);
                version_actual = PQntuples(res_aux_2)-1;
                PQclear(res_aux_2);

		N = 0;
		for (k=0; k<N_preguntas;k++) N += (versiones[version_actual].preguntas[k].respuesta == PQgetvalue (res, j, 2)[k]);
		N -= (versiones[version_actual].preguntas[i].respuesta == PQgetvalue (res, j, 2)[i]);

		media_sin += ((long double) N * 100.0/ (N_preguntas-1));
	       }

	   media_sin /= N_estudiantes;

	   varianza_sin = 0.0;

	   for (j=0; j<N_estudiantes;j++)
	       {/* calcular varianza ignorando pregunta i */
                sprintf (PG_command,"SELECT version from EX_versiones where examen = '%s' and version <= '%s'", examen, PQgetvalue(res, j, 1));
                res_aux_2 = PQEXEC(DATABASE, PG_command);
                version_actual = PQntuples(res_aux_2)-1;
                PQclear(res_aux_2);

		N = 0;
		for (k=0; k<N_preguntas;k++) N += (versiones[version_actual].preguntas[k].respuesta == PQgetvalue (res, j, 2)[k]);
		N -= (versiones[version_actual].preguntas[i].respuesta == PQgetvalue (res, j, 2)[i]);

		varianza_sin += ((media_sin - ((long double) N * 100.0/ (N_preguntas-1))) * (media_sin - ((long double) N * 100.0/ (N_preguntas-1))));
	       }

	   varianza_sin /= N_estudiantes;
           varianza_pregunta = (((long double)preguntas[i].buenos/N_estudiantes)*((long double)preguntas[i].malos/N_estudiantes));

           preguntas[i].alfa_sin = ((long double) (N_preguntas-1)/ (long double) (N_preguntas - 2)) * (1.0 - ((suma_varianzas-varianza_pregunta)/varianza_sin));
	  }

      i=0;
      N_temas   =0;
      N_subtemas=0;
      while (i < N_preguntas)
	    {
	     strcpy (resumen_tema [N_temas].tema, preguntas[i].tema);
	     strcpy (resumen_tema [N_temas].subtema, CODIGO_VACIO);

	     resumen_tema [N_temas].buenos = 0;
	     resumen_tema [N_temas].malos  = 0;

	     while ((i < N_preguntas) && (strcmp(resumen_tema [N_temas].tema, preguntas[i].tema) == 0))
	           {
 	            strcpy (resumen_tema [N_temas].subtema, preguntas[i].subtema);

 	            strcpy (resumen_tema_subtema [N_subtemas].tema,    preguntas[i].tema);
 	            strcpy (resumen_tema_subtema [N_subtemas].subtema, preguntas[i].subtema);

		    resumen_tema_subtema [N_subtemas].buenos = 0;
		    resumen_tema_subtema [N_subtemas].malos  = 0;

  	            while ((i < N_preguntas) && 
                           (strcmp(resumen_tema         [N_temas].tema,    preguntas[i].tema)    == 0) && 
                           (strcmp(resumen_tema_subtema [N_subtemas].subtema, preguntas[i].subtema) == 0))
		           {
			    resumen_tema_subtema [N_subtemas].buenos += preguntas [i].buenos;
			    resumen_tema_subtema [N_subtemas].malos  += preguntas [i].malos;

			    i++;
		           }

		    resumen_tema [N_temas].buenos += resumen_tema_subtema [N_subtemas].buenos;
		    resumen_tema [N_temas].malos  += resumen_tema_subtema [N_subtemas].malos;

		    N_subtemas++;
	           }

	     N_temas++;
	    }

      for (i=0; i < N_temas;    i++) resumen_tema [i].porcentaje = (long double) resumen_tema [i].buenos/(resumen_tema [i].buenos + resumen_tema [i].malos);

      for (i=N_temas; i >= 0; i--)
          for (j=0; j < (i-1); j++)
	      {
	       if (resumen_tema[j].porcentaje > resumen_tema[j+1].porcentaje)
	          {
	           temporal          = resumen_tema[j+1];
                   resumen_tema[j+1] = resumen_tema[j];
                   resumen_tema[j]   = temporal;
	          }
	      }

      for (i=0; i < N_subtemas; i++) resumen_tema_subtema[i].porcentaje = (long double) resumen_tema_subtema[i].buenos/(resumen_tema_subtema[i].buenos + resumen_tema_subtema[i].malos);
      for (i=N_subtemas; i >= 0; i--)
          for (j=0; j < (i-1); j++)
	      {
	       if (resumen_tema_subtema[j].porcentaje > resumen_tema_subtema[j+1].porcentaje)
	          {
	           temporal                  = resumen_tema_subtema[j+1];
                   resumen_tema_subtema[j+1] = resumen_tema_subtema[j];
                   resumen_tema_subtema[j]   = temporal;
	          }
	      }
     }

  PQclear(res);
}

void Actualice_Estadisticas (int buena, char * ejercicio, int secuencia, long double Nota, char respuesta, int opcion_original[5])
{
  int i, j;
  int found;

  for (i=0; (i < N_preguntas) && !((strcmp(ejercicio,preguntas[i].ejercicio)==0) && (preguntas[i].secuencia == secuencia)) ; i++);

  if (buena)
     {
      preguntas[i].buenos++;
      preguntas[i].suma_buenos += Nota;
     }
  else
     {
      preguntas[i].malos++;
      preguntas[i].suma_malos += Nota;
     }

  if ((respuesta >= 'A' )  && (respuesta <= 'E'))
     {
      preguntas[i].acumulado_opciones [opcion_original[respuesta-'A']]++;
      preguntas[i].suma_seleccion     [opcion_original[respuesta-'A']] += Nota;
     } 
}

void Imprime_Reporte ()
{
   int k;
   FILE * Archivo_Latex;
   long double media_real, desviacion_real, media_prediccion, desviacion_prediccion, alfa, Rpb, width;
   char hilera_antes [9000], hilera_despues [9000];
   gchar *materia, *materia_descripcion, *descripcion, *version, *institucion, *escuela, *programa, *fecha, *profesor;
   char Directorio[2000];
   char comando[200];
   char codigo[10];
   DIR * pdir;

   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show         (window4);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Banderas [0] =Banderas [1] =Banderas [2] =Banderas [3] = Banderas [4] = Banderas [5] = 0;

   k = (int) gtk_spin_button_get_value_as_int (SP_examen);
   sprintf (codigo, "%05d", k);

   materia               = gtk_editable_get_chars(GTK_EDITABLE(EN_materia)            , 0, -1);
   materia_descripcion   = gtk_editable_get_chars(GTK_EDITABLE(EN_materia_descripcion), 0, -1);
   descripcion           = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion)        , 0, -1);
   institucion           = gtk_editable_get_chars(GTK_EDITABLE(EN_institucion)        , 0, -1);
   escuela               = gtk_editable_get_chars(GTK_EDITABLE(EN_escuela)            , 0, -1);
   programa              = gtk_editable_get_chars(GTK_EDITABLE(EN_programa)           , 0, -1);
   profesor              = gtk_editable_get_chars(GTK_EDITABLE(EN_profesor)           , 0, -1);
   fecha                 = gtk_editable_get_chars(GTK_EDITABLE(EN_fecha)              , 0, -1);
   media_real            = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_media_real)           , 0, -1));
   desviacion_real       = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_desviacion_real)      , 0, -1));
   media_prediccion      = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_media_prediccion)     , 0, -1));
   desviacion_prediccion = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_desviacion_prediccion), 0, -1));
   alfa                  = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_alfa_real)            , 0, -1));
   Rpb                   = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_Rpb_real)             , 0, -1));

   Establece_Directorio (Directorio, materia,fecha+6,fecha+3,fecha);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.05);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Archivo_Latex = fopen ("analisis.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, twoside, onecolumn]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex); /* En EX_utilities.c */
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);
   fprintf  (Archivo_Latex, "\\SetWatermarkText{}\n");
   fprintf  (Archivo_Latex, "\\pagestyle{fancy}\n");

   fprintf (Archivo_Latex, "\\fancyhead{}\n");
   fprintf (Archivo_Latex, "\\fancyhead[LO, RE]{%s}\n", fecha);
   fprintf (Archivo_Latex, "\\fancyhead[LE,RO]{\\thepage}\n");
   sprintf (hilera_antes,"\\fancyhead[CE,CO]{\\textbf{%s - %s}}", descripcion, materia_descripcion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   fprintf  (Archivo_Latex, "\\fancyfoot{}\n");
   sprintf  (hilera_antes,"\\fancyfoot[CE,CO]{%s}",           institucion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   sprintf  (hilera_antes,"\\fancyfoot[LE,LO]{\\textbf{%s}}", escuela);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   sprintf  (hilera_antes,"\\fancyfoot[RE,RO]{%s}",           programa);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   fprintf (Archivo_Latex, "\\renewcommand{\\headrulewidth}{0.4pt}\n");
   fprintf (Archivo_Latex, "\\renewcommand{\\footrulewidth}{0.4pt}\n");

   fprintf (Archivo_Latex, "\\begin{document}\n");
   fprintf (Archivo_Latex, "\n\n");

   sprintf (hilera_antes, "\\title{An\\'{a}lisis de %s\\\\%s (%s)}", descripcion, materia_descripcion, fecha);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex,"%s\n", hilera_despues);

   fprintf (Archivo_Latex, "\\maketitle\n");
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.125);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Tabla_Datos_Generales (Archivo_Latex, institucion, escuela, programa, materia_descripcion, profesor, descripcion, fecha, codigo, media_real, desviacion_real, alfa, Rpb, 0);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.15);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Analisis_General (Archivo_Latex, alfa, Rpb, 0);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.175);
   while (gtk_events_pending ()) gtk_main_iteration ();


   width = media_real - Nota_minima;
   if ((Nota_maxima - media_real) > width) width = Nota_maxima - media_real;

   Prepara_Histograma_Notas     ();
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.22);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Prepara_Grafico_Normal (media_real, desviacion_real, media_prediccion, desviacion_prediccion, width);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.25);
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\centering\n");

   fprintf (Archivo_Latex, "\\begin{minipage}[c]{0.49\\textwidth}\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=0.8]{EX4010-h.pdf}\n");
   fprintf (Archivo_Latex, "\\caption*{\\small {\\textbf{Histograma de Notas}}}\n");
   fprintf (Archivo_Latex, "\\end{minipage}\n");

   fprintf (Archivo_Latex, "\\begin{minipage}[c]{0.49\\textwidth}\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=0.8]{EX4010-n.pdf}\n");
   fprintf (Archivo_Latex, "\\caption*{\\small {\\textbf{Distribuci\\'{o}n Normal (real y predicci\\'{o}n)}}}\n");
   fprintf (Archivo_Latex, "\\end{minipage}\n");

   fprintf (Archivo_Latex, "\\end{figure}\n");

   fprintf (Archivo_Latex, "\\clearpage\n");

   Dificultad_vs_Discriminacion ();
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.35);
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=1.0]{EX4010p.pdf}\n");
   hilera_LATEX ("\\caption*{\\textbf{Cruce entre Coeficiente de Discriminación ($r_{pb}$) y Dificultad ($p$)}}", hilera_despues);
   fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
   fprintf (Archivo_Latex, "\\end{figure}\n\n");

   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=1.0]{EX4010c.pdf}\n");
   hilera_LATEX ("\\caption*{\\textbf{Líneas de Contorno del Cruce entre Coeficiente de Discriminación ($r_{pb}$) y Dificultad ($p$)}}", hilera_despues);
   fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
   fprintf (Archivo_Latex, "\\end{figure}\n\n");

   fprintf (Archivo_Latex, "\\clearpage\n");

   if (N_temas > 1) Prepara_Grafico_Pastel (Archivo_Latex);

   Prepara_Histograma_Temas ();
   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=1.0, angle=-90]{EX4010-t.pdf}\n");
   fprintf (Archivo_Latex, "\\caption*{\\small {\\textbf{An\\'{a}lisis por Temas (ordenado por rendimiento)}}}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n");
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.4);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Prepara_Histograma_Subtemas ();
   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=1.0, angle=-90]{EX4010-s.pdf}\n");
   fprintf (Archivo_Latex, "\\caption*{\\small {\\textbf{An\\'{a}lisis por Subtemas (ordenado por rendimiento)}}}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n");
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.45);
   while (gtk_events_pending ()) gtk_main_iteration ();

   Asigna_Banderas ();
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.5);
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\twocolumn\n");
   fprintf (Archivo_Latex, "\\clearpage\n");
   fprintf  (Archivo_Latex, "\\SetWatermarkText{Confidencial}\n");
   Lista_de_Preguntas (Archivo_Latex, PB_analisis, 0.5, 0.29);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.8);
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\onecolumn\n");
   fprintf (Archivo_Latex, "\\clearpage\n");
   fprintf  (Archivo_Latex, "\\SetWatermarkText{}\n");

   Resumen_de_Banderas (Archivo_Latex, 0);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.82);
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\clearpage\n");
   fprintf  (Archivo_Latex, "\\SetWatermarkText{Confidencial}\n");
   Lista_de_Notas (Archivo_Latex);

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.85);
   while (gtk_events_pending ()) gtk_main_iteration ();

   latex_2_pdf (&parametros, Directorio, parametros.ruta_latex, "analisis", 1,PB_analisis, 0.85, 0.1, NULL, NULL);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.95);
   while (gtk_events_pending ()) gtk_main_iteration ();

   system ("rm EX4010-h.pdf");
   system ("rm EX4010-n.pdf");
   system ("rm EX4010-t.pdf");
   system ("rm EX4010-s.pdf");
   system ("rm EX4010p.pdf");
   system ("rm EX4010c.pdf");
   system ("rm EX4010.dat");

   g_free(materia);
   g_free(materia_descripcion);
   g_free(descripcion);
   g_free(institucion);
   g_free(escuela);
   g_free(programa);
   g_free(fecha);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 1.0);
   while (gtk_events_pending ()) gtk_main_iteration ();

   gtk_widget_hide (window4);
   gtk_widget_set_sensitive(window1, 1);
}

void Prepara_Grafico_Pastel (FILE * Archivo_Latex)
{
  int Empates;

  Crea_archivo_datos_pastel (&Empates);

  fprintf(Archivo_Latex, "\\DTLloaddb{datosesquema}{EX4010.dat}\n");
  fprintf(Archivo_Latex, "\\begin{figure}[htpb]\n");
  fprintf(Archivo_Latex, "\\centering\n");
  fprintf(Archivo_Latex, "\\setlength{\\DTLpieoutlinewidth}{1pt}\n");

  colores_pastel (Archivo_Latex);

  fprintf(Archivo_Latex, "\\renewcommand*{\\DTLdisplayinnerlabel}[1]{\\textsf{#1}}\n");
  fprintf(Archivo_Latex, "\\renewcommand*{\\DTLdisplayouterlabel}[1]{\\textsf{#1}}\n");
  fprintf(Archivo_Latex, "\\DTLpiechart{variable=\\quantity,%%\n");

  fprintf(Archivo_Latex, "radius=4.1cm,%%\n");
  fprintf(Archivo_Latex, "outerratio=1.05,%%\n");

  if (Empates < MAX_COLORES)
     if (Empates == 1)
        fprintf(Archivo_Latex, "cutaway={1},%%\n");
     else
        fprintf(Archivo_Latex, "cutaway={1-%d},%%\n",Empates);

  fprintf(Archivo_Latex, "innerlabel={},%%\n");
  fprintf(Archivo_Latex, "outerlabel={\\DTLpievariable}}{datosesquema}{%%\n");
  fprintf(Archivo_Latex, "\\name=Name,\\quantity=Quantity}\n");
  fprintf(Archivo_Latex, "\\begin{tabular}[b]{ll}\n");
  fprintf(Archivo_Latex, "\\DTLforeach{datosesquema}{\\name=Name,\\quantity=Quantity}{\\DTLiffirstrow{}{\\\\}%%\n");
  fprintf(Archivo_Latex, "\\DTLdocurrentpiesegmentcolor\\rule{10pt}{10pt}&\n");
  fprintf(Archivo_Latex, "\\textsf{\\name}\n");
  fprintf(Archivo_Latex, "}\n");
  fprintf(Archivo_Latex, "\\end{tabular}\n");
  fprintf (Archivo_Latex, "\\caption*{\\small {\\textbf{Material Evaluado en el Examen}}}\n");
  fprintf(Archivo_Latex, "\\end{figure}\n");
}

void Prepara_Grafico_Pastel_Beamer (FILE * Archivo_Latex)
{
  int Empates;
  int aspecto;

  aspecto = gtk_combo_box_get_active (CB_aspecto);

  Crea_archivo_datos_pastel (&Empates);

  fprintf(Archivo_Latex, "\\DTLloaddb{datosesquema}{EX4010.dat}\n");
  fprintf(Archivo_Latex, "\\begin{columns}\n");

  fprintf(Archivo_Latex, "\\begin{column}{0.5\\textwidth}\n");

  fprintf(Archivo_Latex, "\\begin{figure}[H]\n");

  fprintf(Archivo_Latex, "\\centering\n");

  fprintf(Archivo_Latex, "\\setlength{\\DTLpieoutlinewidth}{1pt}\n");

  colores_pastel (Archivo_Latex);

  fprintf(Archivo_Latex, "\\renewcommand*{\\DTLdisplayinnerlabel}[1]{\\textsf{#1}}\n");
  fprintf(Archivo_Latex, "\\renewcommand*{\\DTLdisplayouterlabel}[1]{\\textsf{#1}}\n");
  fprintf(Archivo_Latex, "\\DTLpiechart{variable=\\quantity,%%\n");
  if ((aspecto == 1) || (aspecto == 2))
     fprintf(Archivo_Latex, "radius=3.0cm,%%\n");
  else
     fprintf(Archivo_Latex, "radius=2.25cm,%%\n");

  fprintf(Archivo_Latex, "outerratio=1.05,%%\n");

  if (Empates < MAX_COLORES)
     if (Empates == 1)
        fprintf(Archivo_Latex, "cutaway={1},%%\n");
     else
        fprintf(Archivo_Latex, "cutaway={1-%d},%%\n",Empates);

  fprintf(Archivo_Latex, "innerlabel={},%%\n");
  fprintf(Archivo_Latex, "outerlabel={\\DTLpievariable}}{datosesquema}{%%\n");
  fprintf(Archivo_Latex, "\\name=Name,\\quantity=Quantity}\n");

  fprintf(Archivo_Latex, "\\end{figure}\n");

  fprintf(Archivo_Latex, "\\end{column}\n");

  fprintf(Archivo_Latex, "\\begin{column}{0.5\\textwidth}\n");

  fprintf(Archivo_Latex, "\\begin{figure}[htpb]\n");

  fprintf(Archivo_Latex, "\\centering\n");

  colores_pastel (Archivo_Latex);

  fprintf(Archivo_Latex, "\\begin{tabular}[b]{ll}\n");
  fprintf(Archivo_Latex, "\\DTLforeach{datosesquema}{\\name=Name,\\quantity=Quantity}{\\DTLiffirstrow{}{\\\\}%%\n");
  fprintf(Archivo_Latex, "\\DTLdocurrentpiesegmentcolor\\rule{10pt}{10pt}&\n");
  fprintf(Archivo_Latex, "\\textsf{\\name}\n");
  fprintf(Archivo_Latex, "}\n");
  fprintf(Archivo_Latex, "\\end{tabular}\n");

  fprintf(Archivo_Latex, "\\end{figure}\n");
  fprintf(Archivo_Latex, "\\end{column}\n");
  fprintf(Archivo_Latex, "\\end{columns}\n");
}

void Crea_archivo_datos_pastel(int *Empates)
{
  gchar *materia;
  FILE *Archivo_Datos;
  int i,j, N, N_otros;
  char hilera_antes   [9000];
  char hilera_despues [9000];
  struct {
          char tema[CODIGO_TEMA_SIZE + 1];
          int  cantidad;
         } temporal, tabla [N_temas];
  char PG_command [2000];
  PGresult *res_tema;
  char Descripcion[300];
  int MAX_DESCRIPCION;
  int aspecto;

  aspecto = gtk_combo_box_get_active (CB_aspecto);
  if ((aspecto == 1) || (aspecto == 2))
     MAX_DESCRIPCION = 43;
  else
     MAX_DESCRIPCION = 35;

  materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);

  Archivo_Datos = fopen ("EX4010.dat","w");
  fprintf(Archivo_Datos,"Name, Quantity\n");

  for (i=0; i<N_temas; i++)
      {
       strcpy(tabla[i].tema,resumen_tema[i].tema);
       tabla[i].cantidad = (resumen_tema[i].buenos+resumen_tema[i].malos)/N_estudiantes;
      }

  for (i=N_temas; i >= 0; i--)
      for (j=0; j < (i-1); j++)
          {
           if (tabla[j].cantidad < tabla[j+1].cantidad)
	          {
	           temporal    = tabla[j+1];
                   tabla [j+1] = tabla[j];
                   tabla [j]   = temporal;
	          }
	      }
  N = N_temas;
  if (N >  MAX_COLORES) N = MAX_COLORES - 1;

  for (i=0; i < N; i++)
      {
       sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '",
                  materia,tabla[i].tema);
       res_tema = PQEXEC(DATABASE, PG_command);
       strcpy(Descripcion, PQgetvalue (res_tema, 0, 0));

       if (strlen(Descripcion) > MAX_DESCRIPCION)
	  {
	   Descripcion[MAX_DESCRIPCION - 3] ='.';
	   Descripcion[MAX_DESCRIPCION - 2] ='.';
	   Descripcion[MAX_DESCRIPCION - 1] ='.';
	   Descripcion[MAX_DESCRIPCION] ='\0';
	  }

       sprintf (hilera_antes,"\"\\textbf{%s}\", %d",Descripcion,tabla[i].cantidad);
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf(Archivo_Datos,"%s\n", hilera_despues);
      }

  N_otros = 0;
  for (i=N; i < N_temas; i++) N_otros += tabla[i].cantidad;

  if (N_otros)
     fprintf(Archivo_Datos,"\"\\textbf{Otros} (%d temas)\", %d\n", N_temas - N, N_otros);

  fclose (Archivo_Datos);

  i=1;
  while((i < N_temas) && tabla[0].cantidad == tabla[i++].cantidad);
  *Empates = i - 1;

  g_free (materia);
}

void colores_pastel (FILE * Archivo_Latex)
{
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{1}{green}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{2}{blue}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{3}{red}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{4}{yellow}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{5}{magenta}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{6}{cyan}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{7}{orange}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{8}{violet}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{9}{gray}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{10}{lime}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{11}{teal}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{12}{pink}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{13}{brown}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{14}{purple}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{15}{darkgray}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{16}{olive}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{17}{black}\n");
  fprintf(Archivo_Latex, "\\DTLsetpiesegmentcolor{18}{lightgray}\n");
}

void Establece_Directorio (char * Directorio, gchar * materia, char * year, char * month, char * day)
{
   char ruta_final [200];
   DIR * pdir;
   char Directorio_materia       [600];
   char Directorio_materia_fecha [600];
   int i, n;

   sprintf (Directorio_materia,"%s/%s",parametros.ruta_examenes, materia);
   n = strlen(Directorio_materia);
   for (i=n-1; Directorio_materia[i]==' ';i--);
   Directorio_materia[i+1]='\0';
   sprintf (Directorio, "%s/%s-%.2s-%.2s", Directorio_materia,year,month,day);

   pdir = opendir(Directorio_materia);
   if (!pdir)
      {
       mkdir(Directorio_materia,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      }
   else
      closedir(pdir);

   pdir = opendir(Directorio);
   if (!pdir)
      {
       mkdir(Directorio,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      }
   else
      closedir(pdir);
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

void Genera_Beamer ()
{
   int k;
   char codigo[10];
   gchar *size, *font, *color, *estilo, *materia, *materia_descripcion, *descripcion, *version, *institucion, *escuela, *programa, *fecha, *profesor;
   long double media_real, desviacion_real, media_prediccion, desviacion_prediccion, alfa, Rpb, width;
   FILE * Archivo_Latex;
   char Directorio[3000];
   char hilera_antes [9000], hilera_despues [9000];
   int aspecto;
   int resultado_OK;

   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show         (window3);

   Update_PB(PB_beamer, 0.0);

   Banderas [0] =Banderas [1] =Banderas [2] =Banderas [3] = Banderas [4] = Banderas [5] = 0;
   k = (int) gtk_spin_button_get_value_as_int (SP_examen);
   sprintf (codigo, "%05d", k);

   materia               = gtk_editable_get_chars(GTK_EDITABLE(EN_materia)                   , 0, -1);
   materia_descripcion   = gtk_editable_get_chars(GTK_EDITABLE(EN_materia_descripcion)       , 0, -1);
   descripcion           = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion)               , 0, -1);
   institucion           = gtk_editable_get_chars(GTK_EDITABLE(EN_institucion)               , 0, -1);
   escuela               = gtk_editable_get_chars(GTK_EDITABLE(EN_escuela)                   , 0, -1);
   programa              = gtk_editable_get_chars(GTK_EDITABLE(EN_programa)                  , 0, -1);
   profesor              = gtk_editable_get_chars(GTK_EDITABLE(EN_profesor)                  , 0, -1);
   fecha                 = gtk_editable_get_chars(GTK_EDITABLE(EN_fecha)                     , 0, -1);
   media_real            = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_media_real)           , 0, -1));
   desviacion_real       = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_desviacion_real)      , 0, -1));
   media_prediccion      = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_media_prediccion)     , 0, -1));
   desviacion_prediccion = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_desviacion_prediccion), 0, -1));
   alfa                  = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_alfa_real)            , 0, -1));
   Rpb                   = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_Rpb_real)             , 0, -1));
   estilo                = gtk_combo_box_get_active_text(CB_estilo);
   color                 = gtk_combo_box_get_active_text(CB_color);
   font                  = gtk_combo_box_get_active_text(CB_font);
   size                  = gtk_combo_box_get_active_text(CB_size);
   aspecto               = gtk_combo_box_get_active     (CB_aspecto);

   Establece_Directorio (Directorio, materia,fecha+6,fecha+3,fecha);

   Update_PB(PB_beamer, 0.05);

   Archivo_Latex = fopen ("analisis-beamer.tex","w");
   Beamer_Preamble                     (Archivo_Latex, aspecto, size, estilo,color, font,materia_descripcion, descripcion,profesor, programa,escuela, institucion, fecha);
   Update_PB(PB_beamer, 0.08);
   Beamer_Cover                        (Archivo_Latex);
   Update_PB(PB_beamer, 0.09);
   Beamer_TOC                          (Archivo_Latex);
   Update_PB(PB_beamer, 0.10);
   Beamer_Datos_Generales              (Archivo_Latex, institucion, escuela, programa,materia_descripcion, profesor, descripcion, fecha, codigo,media_real, desviacion_real, alfa, Rpb);
   Update_PB(PB_beamer, 0.12);
   if (N_temas > 1) Beamer_Grafico_Pastel               (Archivo_Latex);
   Update_PB(PB_beamer, 0.15);
   Beamer_Histograma_Notas             (Archivo_Latex, media_real, desviacion_real, media_prediccion, desviacion_prediccion);
   Update_PB(PB_beamer, 0.17);
   Beamer_Dificultad_vs_Discriminacion (Archivo_Latex);
   Update_PB(PB_beamer, 0.19);
   Beamer_Histograma_Temas             (Archivo_Latex);
   Update_PB(PB_beamer, 0.21);
   Beamer_Preguntas                    (Archivo_Latex, PB_beamer, 0.21, 0.33);
   Beamer_Gracias                      (Archivo_Latex);
   Beamer_Cierre                       (Archivo_Latex);
   fclose                              (Archivo_Latex);

   Update_PB(PB_beamer, 0.55);

   resultado_OK = latex_2_pdf (&parametros, Directorio, parametros.ruta_latex, "analisis-beamer", 1, PB_beamer, 0.55, 0.43, NULL, NULL);

   system ("rm EX4010-h.pdf");
   system ("rm EX4010-n.pdf");
   system ("rm EX4010-t.pdf");
   system ("rm EX4010p.pdf");
   system ("rm EX4010c.pdf");
   system ("rm EX4010.dat");
   system ("rm analisis-beamer.*");
   Update_PB(PB_beamer, 0.99);

   g_free(estilo);
   g_free(color);
   g_free(font);
   g_free(size);
   g_free(materia);
   g_free(materia_descripcion);
   g_free(descripcion);
   g_free(institucion);
   g_free(escuela);
   g_free(programa);
   g_free(fecha);

   Update_PB(PB_beamer, 1.0);

   gtk_widget_hide (window3);
   gtk_widget_set_sensitive(window1, 1);

   if (!resultado_OK)
      {
       Beamer_Failure();
      }
}

void Beamer_Preamble (FILE * Archivo_Latex, int aspecto, gchar * size, gchar * estilo, gchar * color, gchar * font, gchar * materia_descripcion, gchar * descripcion, gchar * profesor, gchar * programa, gchar * escuela, gchar * institucion, gchar * fecha)
{
   char hilera_antes [9000], hilera_despues [9000];

   fprintf (Archivo_Latex, "\\documentclass[aspectratio=%s,%s]{beamer}\n", Beamer_aspectratio[aspecto], size);
   fprintf (Archivo_Latex, "\\def \\BEAMER {}\n");

   EX_latex_packages (Archivo_Latex); /* En EX_utilities.c */
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);
   fprintf (Archivo_Latex, "\\newcommand*\\circled[1]{\\tikz[baseline=(char.base)]{\\node[shape=circle,draw,inner sep=2pt] (char) {#1};}}\n");
   fprintf (Archivo_Latex, "\\newcommand*\\correcta[1]{\\tikz[baseline=(char.base)]{\\node[shape=circle,fill=gray,draw,inner sep=2pt] (char) {#1};}}\n");

   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\usetheme{%s}\n", estilo);
   fprintf (Archivo_Latex, "\\usecolortheme{%s}\n", color);
   fprintf (Archivo_Latex, "\\usefonttheme{%s}\n", font);

   fprintf (Archivo_Latex, "\\useoutertheme{shadow}\n");
   fprintf (Archivo_Latex, "\\setbeamertemplate{navigation symbols}{}\n");
   fprintf (Archivo_Latex, "\\setbeamertemplate{caption}[numbered]\n");
   fprintf (Archivo_Latex, "\\captionsetup{labelsep = colon,figureposition = bottom}\n");

   fprintf (Archivo_Latex, "\\setbeamercolor{caption name}{fg=black}\n");
   fprintf (Archivo_Latex, "\\setbeamertemplate{headline}{}\n");

   fprintf (Archivo_Latex, "\\definecolor{DoradoPalido}{RGB}{238,232,170}\n");
   fprintf (Archivo_Latex, "\\setbeamercolor{pregunta} {bg=DoradoPalido, fg=black}\n");
   fprintf (Archivo_Latex, "\\setbeamertemplate{blocks}[rounded][shadow=true]\n");
   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");

   sprintf (hilera_antes, "\\title{%s}", materia_descripcion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes, "\\subtitle{Análisis de %s}", descripcion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes, "\\author[%s]{Prof. %s}", profesor, profesor);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   fprintf (Archivo_Latex, "\\institute{\n");

   sprintf (hilera_antes, "%s\\\\", programa);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   sprintf (hilera_antes, "%s\\\\", escuela);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   sprintf (hilera_antes, "%s\\\\", institucion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   fprintf (Archivo_Latex, "}\n");
   fprintf (Archivo_Latex, "\\date{%s}\n", fecha);
   fprintf (Archivo_Latex, "\\begin{document}\n\n");
}

void Beamer_Cover(FILE * Archivo_Latex)
{
   fprintf (Archivo_Latex, "\\begin{frame}[plain]\n");

   fprintf (Archivo_Latex, "\\titlepage\n");

   fprintf (Archivo_Latex, "\\begin{figure}\n");
   fprintf (Archivo_Latex, "\\includegraphics[height=1.3cm]{.imagenes/EX.png}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n");

   fprintf (Archivo_Latex, "\\end{frame}\n");
}

void Beamer_TOC(FILE * Archivo_Latex)
{
   fprintf (Archivo_Latex, "\\begin{frame}[plain]{Contenido}\n");
   fprintf (Archivo_Latex, "\\tableofcontents\n");
   fprintf (Archivo_Latex, "\\end{frame}\n");
}

void Beamer_Datos_Generales(FILE * Archivo_Latex, gchar * institucion, gchar * escuela, gchar * programa, gchar * materia_descripcion, gchar * profesor, gchar * descripcion, gchar * fecha, char * codigo, long double media_real, long double desviacion_real, long double alfa, long double Rpb)
{
   fprintf (Archivo_Latex, "\\section{Estad\\'isticas B\\'asicas}\n");
   fprintf (Archivo_Latex, "\\begin{frame}{Datos Generales}\n");
   Tabla_Datos_Generales (Archivo_Latex, institucion, escuela, programa, materia_descripcion, profesor, descripcion, fecha, codigo, media_real, desviacion_real, alfa, Rpb, 1);
   fprintf (Archivo_Latex, "\\end{frame}\n");

   fprintf (Archivo_Latex, "\\begin{frame}{An\\'alisis General}\n");
   Analisis_General (Archivo_Latex, alfa, Rpb, 1);
   fprintf (Archivo_Latex, "\\end{frame}\n");
}

void Beamer_Grafico_Pastel(FILE * Archivo_Latex)
{
   fprintf (Archivo_Latex, "\\begin{frame}[shrink,fragile]{Material Evaluado en el Examen}\n");
   Prepara_Grafico_Pastel_Beamer (Archivo_Latex);
   fprintf (Archivo_Latex, "\\end{frame}\n");
}

void Beamer_Histograma_Notas(FILE * Archivo_Latex, long double media_real, long double desviacion_real, long double media_prediccion, long double desviacion_prediccion )
{
   long double width;

   width = media_real - Nota_minima;
   if ((Nota_maxima - media_real) > width) width = Nota_maxima - media_real;

   Prepara_Histograma_Notas     ();

   Prepara_Grafico_Normal (media_real, desviacion_real, media_prediccion, desviacion_prediccion, width);

   fprintf (Archivo_Latex, "\\begin{frame}{Histograma de Notas}\n");
   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=0.8]{EX4010-h.pdf}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n");
   fprintf (Archivo_Latex, "\\end{frame}\n");

   fprintf (Archivo_Latex, "\\begin{frame}{Distribuci\\'{o}n Normal (Predicci\\'on y Real)}\n");
   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=0.8]{EX4010-n.pdf}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n");
   fprintf (Archivo_Latex, "\\end{frame}\n");
}

void Beamer_Dificultad_vs_Discriminacion(FILE * Archivo_Latex)
{
   Dificultad_vs_Discriminacion ();
   fprintf (Archivo_Latex, "\\begin{frame}{Cruce de Discriminaci\\'on ($r_{pb}$) y Dificultad ($p$)}");
   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=0.65]{EX4010p.pdf}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n\n");
   fprintf (Archivo_Latex, "\\end{frame}\n");

   fprintf (Archivo_Latex, "\\begin{frame}{L\\'ineas de Contorno del Cruce entre \\\\Coeficiente de Discriminaci\\'on ($r_{pb}$) y Dificultad ($p$)}");
   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=0.65]{EX4010c.pdf}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n\n");
   fprintf (Archivo_Latex, "\\end{frame}\n");
}

void Beamer_Histograma_Temas(FILE * Archivo_Latex)
{
   Prepara_Histograma_Temas ();
   fprintf (Archivo_Latex, "\\begin{frame}{An\\'{a}lisis por Temas (ordenado por rendimiento)}\n");
   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=0.67, angle=-90]{EX4010-t.pdf}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n");
   fprintf (Archivo_Latex, "\\end{frame}\n");
}

void Beamer_Preguntas(FILE * Archivo_Latex, GtkWidget *PB, long double base, long double limite)
{
   if (!gtk_toggle_button_get_active(CK_general))
      {
       Asigna_Banderas ();
       Lista_de_Preguntas_Beamer (Archivo_Latex, PB, base, limite);
      }
}

void Beamer_Gracias(FILE * Archivo_Latex)
{
   fprintf (Archivo_Latex, "\\begin{frame}[plain]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "{\\Huge Muchas gracias}\n");
   fprintf (Archivo_Latex, "\\end{frame}\n");
}

void Beamer_Cierre(FILE * Archivo_Latex)
{
   fprintf (Archivo_Latex, "\\end{document}\n");
}

void Beamer_Failure()
{
  gtk_widget_set_sensitive(window1, 0);
  gtk_widget_show         (window5);
}

void Graba_Ajustes ()
{
   int i, k;
   char PG_command [3000];
   PGresult *res;
   char examen [10];

   k = (int) gtk_spin_button_get_value_as_int (SP_examen);
   sprintf (examen, "%05d", k);

   /* start a transaction block */
   res = PQEXEC(DATABASE, "BEGIN"); PQclear(res);

   sprintf (PG_command,"DELETE from EX_examenes_ajustes where examen = '%s'", examen);
   res = PQEXEC(DATABASE, PG_command); PQclear(res);

   for (i=0; i < N_preguntas; i++)
       {
	Calcula_ajuste(i);

	if (preguntas[i].ajuste != PREGUNTA_NORMAL)
	   {
 	    sprintf (PG_command,"INSERT into EX_examenes_ajustes values (\'%s\', %d, \'%s\', %d, %d, %d, %d, %d, %d, %d)", 
            examen, i+1, preguntas[i].pregunta, preguntas[i].ajuste,
	    preguntas[i].correctas_nuevas[0], preguntas[i].correctas_nuevas[1], preguntas[i].correctas_nuevas[2], 
	    preguntas[i].correctas_nuevas[3], preguntas[i].correctas_nuevas[4],
            preguntas[i].actualizar);
            res = PQEXEC(DATABASE, PG_command); PQclear(res);
	   }
       }

   /* end the transaction */
   res = PQEXEC(DATABASE, "END"); PQclear(res);
}

void Calcula_ajuste(int i)
{
  preguntas[i].ajuste = preguntas[i].revision_especial +
	               (preguntas[i].excluir            <<  8) +
	               (preguntas[i].encoger            <<  9) +
	               (preguntas[i].verbatim           << 10) +
	               (preguntas[i].header_encoger     << 11) +
	               (preguntas[i].header_verbatim    << 12) +

	               (preguntas[i].slide[0]           << 13) +
	               (preguntas[i].encoger_opcion[0]  << 14) +
	               (preguntas[i].verbatim_opcion[0] << 15) +

	               (preguntas[i].slide[1]           << 16) +
	               (preguntas[i].encoger_opcion[1]  << 17) +
	               (preguntas[i].verbatim_opcion[1] << 18) +

	               (preguntas[i].slide[2]           << 19) +
	               (preguntas[i].encoger_opcion[2]  << 20) +
	               (preguntas[i].verbatim_opcion[2] << 21) +

	               (preguntas[i].slide[3]           << 22) +
	               (preguntas[i].encoger_opcion[3]  << 23) +
	               (preguntas[i].verbatim_opcion[3] << 24) +

	               (preguntas[i].slide[4]           << 25) +
	               (preguntas[i].encoger_opcion[4]  << 26) +
       	               (preguntas[i].verbatim_opcion[4] << 27);
}

void Tabla_Datos_Generales (FILE * Archivo_Latex, char * institucion, char * escuela, char * programa, char * materia_descripcion, char * profesor, char * descripcion, char * fecha, char * codigo, long double media_real, long double desviacion_real, long double alfa, long double Rpb, int Beamer_o_reporte)
{
   char hilera_antes [9000], hilera_despues[9000], mensaje[9000];

   fprintf (Archivo_Latex, "\\begin{longtable}{|l||l|}\n");
   fprintf (Archivo_Latex, "\\hline\n");

   sprintf (hilera_antes,"\\multicolumn{2}{|c|}{\\textbf{%s}}\\\\ \\hline", institucion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\multicolumn{2}{|c|}{\\textbf{%s}}\\\\ \\hline", escuela);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\multicolumn{2}{|c|}{\\textbf{%s}}\\\\ \\hline", programa);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Materia} & %s \\\\ \\hline", materia_descripcion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Profesor} & %s\\\\ \\hline", profesor);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Descripción} & %s\\\\ \\hline", descripcion);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Fecha} & %s\\\\ \\hline", fecha);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Código de Examen} & %s\\\\ \\hline", codigo);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Versiones} & %d \\\\ \\hline", N_versiones);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Preguntas} & %d \\\\ \\hline", N_preguntas);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Estudiantes} & %d \\\\ \\hline", N_estudiantes);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Media} & %6.2Lf \\\\ \\hline", media_real);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Nota m\\'{i}nima} & %6.2Lf \\\\ \\hline", Nota_minima);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Nota m\\'{i}nima ajustada} & %6.2Lf \\\\ \\hline", Nota_minima_ajustada);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Nota m\\'{a}xima} & %6.2Lf \\\\ \\hline", Nota_maxima);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Nota m\\'{a}xima ajustada} & %6.2Lf \\\\ \\hline", Nota_maxima_ajustada);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{Desviación Estándar} & %6.4Lf \\\\ \\hline", desviacion_real);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{$\\alpha$ de Cronbach} & %6.4Lf \\\\ \\hline", alfa);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   sprintf (hilera_antes,"\\textbf{$r_{pb}$ promedio} & %6.4Lf \\\\ \\hline", Rpb);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   fprintf (Archivo_Latex, "\\hline\\hline\n");

   fprintf (Archivo_Latex, "\\end{longtable}\n");
}

void Analisis_General (FILE * Archivo_Latex, long double alfa, long double Rpb, int Beamer_o_reporte)
{
   char mensaje[2000];

   if (alfa > 0.9)
      {
       sprintf (mensaje,"El examen muestra una \\textbf{excelente consistencia interna} ($\\alpha$ de Cronbach = \\textbf{%Lf}). Diversos \\'itemes que miden la misma caracter\\'istica muestran un comportamiento bastante similar.", alfa);

       Cajita_con_bandera (Archivo_Latex, mensaje, VERDE, Beamer_o_reporte);
      }
   else
      if (alfa > 0.8)
         {
          sprintf (mensaje,"El examen muestra una \\textbf{buena consistencia interna} ($\\alpha$ de Cronbach = \\textbf{%Lf}). Diversos \\'itemes que miden la misma caracter\\'istica muestran comportamientos similares.", alfa);

          Cajita_con_bandera (Archivo_Latex, mensaje, AZUL, Beamer_o_reporte);
         }
      else
         if (alfa > 0.7)
            {
             sprintf (mensaje,"La \\textbf{consistencia interna} del examen es aceptable, sin ser muy buena ($\\alpha$ de Cronbach = \\textbf{%Lf}). Se nota que el comportamiento de diversos \\'itemes del examen es divergente.", alfa);
             Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO, Beamer_o_reporte);
            }
	 else
            {
             sprintf (mensaje,"La \\textbf{consistencia interna} de este examen es muy pobre ($\\alpha$ de Cronbach = \\textbf{%Lf}). Los \\'itemes del examen muestran comportamientos divergente.", alfa);
             Cajita_con_bandera (Archivo_Latex, mensaje, ROJO, Beamer_o_reporte);
            }


   if (Rpb > 0.3)
      {
       sprintf (mensaje,"La discriminaci\\'on promedio de los \\'itemes es bastante alta (\\textbf{%Lf}). El examen es muy preciso en distinguir entre estudiantes de buen rendimiento y bajo rendimiento.", Rpb);

       Cajita_con_bandera (Archivo_Latex, mensaje, VERDE, Beamer_o_reporte);
      }
   else
      if (Rpb > 0.15)
         {
          sprintf (mensaje,"El examen muestra una buena discriminaci\\'on promedio (\\textbf{%Lf}). Distingue aceptablemente entre estudiantes de alto rendimiento y bajo rendimiento.", Rpb);

          Cajita_con_bandera (Archivo_Latex, mensaje, AZUL, Beamer_o_reporte);
         }
      else
         if (Rpb > 0.0)
            {
             sprintf (mensaje,"La discriminaci\\'on promedio del examen es muy baja (\\textbf{%Lf}) aunque positiva. Se recomienda una revisi\\'on general de los enunciados y las opciones.", Rpb);
             Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO, Beamer_o_reporte);
            }
	 else
            {
             sprintf (mensaje,"El promedio de discriminaci\\'on de las preguntas de este examen es negativo (\\textbf{%Lf}). Es indispensable hacer una revisión detallada de todos los enunciados y sus opciones.", Rpb);
             Cajita_con_bandera (Archivo_Latex, mensaje, ROJO, Beamer_o_reporte);
            }
}

void Asigna_Banderas ()
{
   int i, j, N_distractores_usados;
   long double alfa;

   alfa                  = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_alfa_real), 0, -1));

   for (i=0; i < N_preguntas; i++)
       {
	for (j=0; j < N_FLAGS;j++) preguntas[i].flags[j] = 0;

	if (!preguntas [i].buenos)                                                     preguntas[i].flags[TODAS_MALAS]            = 1;
  	if (!preguntas [i].malos)                                                      preguntas[i].flags[TODAS_BUENAS]           = 1;
        if ((preguntas[i].porcentaje < 0.31) && preguntas [i].buenos)                  preguntas[i].flags[PORCENTAJE_MENOR_30]    = 1;
	/*
	if (preguntas[i].Rpb < -0.2)                                                   preguntas[i].flags[RPB_MUY_NEGATIVO]       = 1;
        else 
	*/
        if (preguntas[i].Rpb < 0.0)                                                    preguntas[i].flags[RPB_NEGATIVO]           = 1;
        if (preguntas[i].Rpb >= 0.3)                                                   preguntas[i].flags[RPB_BUENO]              = 1;
	if ((preguntas[i].porcentaje - preguntas[i].previo) > preguntas[i].desviacion) preguntas[i].flags[SUBE_MAS_DE_DESVIACION] = 1;
        if ((preguntas[i].previo - preguntas[i].porcentaje) > preguntas[i].desviacion) preguntas[i].flags[BAJA_MAS_DE_DESVIACION] = 1;
	if ((preguntas[i].alfa_sin - alfa) > 0.1)                                      preguntas[i].flags[ALFA_SUBE_MUCHO]        = 1;
	else if ((preguntas[i].alfa_sin - alfa) > 0.04)                                preguntas[i].flags[ALFA_SUBE]              = 1;

	N_distractores_usados = 0;
	for (j=0;j<5;j++)
	    {
	     if ((preguntas[i].Rpb_opcion[j] > 0.3) && (preguntas[i].correcta != ('A' + j)))
                preguntas[i].flags[RPB_DISTRACTOR_MUY_POSITIVO] = 1;
	     else
  	        if ((preguntas[i].Rpb_opcion[j] > 0.0) && (preguntas[i].correcta != ('A' + j)))
                   preguntas[i].flags[RPB_DISTRACTOR_POSITIVO] = 1;
		else
  	           if ((preguntas[i].Rpb_opcion[j] < -0.29) && (preguntas[i].correcta != ('A' + j)))
                      preguntas[i].flags[RPB_DISTRACTOR_MUY_NEGATIVO] = 1;

             if (preguntas[i].acumulado_opciones[j] > 0) N_distractores_usados++;
	    }
	if (N_distractores_usados > 3)                                                 preguntas[i].flags[MAS_DE_3_DISTRACTORES] = 1;

	if ((preguntas [i].Rpb >= 0.3) && (preguntas [i].porcentaje <= 0.75) && N_distractores_usados <= 3) preguntas[i].flags[RPB_Y_DIFICULTAD_OK] = 1;
	if ((preguntas [i].Rpb >= 0.3) && (preguntas [i].porcentaje <= 0.75) && N_distractores_usados >  3) preguntas[i].flags[RPB_DIFI_DISTRA_OK]  = 1;
       }
}

void Color_Fila (FILE * Archivo_Latex, int flags[N_FLAGS])
{
   if (flags[TODAS_MALAS] || flags[RPB_NEGATIVO] || flags[ALFA_SUBE_MUCHO] || flags[BAJA_MAS_DE_DESVIACION])
      fprintf (Archivo_Latex,"\\rowcolor{red}\n");
   else
      if (flags[PORCENTAJE_MENOR_30] || flags[ALFA_SUBE])
         fprintf (Archivo_Latex,"\\rowcolor{yellow}\n");
      else
	if (flags[TODAS_BUENAS] || flags[SUBE_MAS_DE_DESVIACION] || flags[RPB_BUENO])
           fprintf (Archivo_Latex,"\\rowcolor{blue}\n");
}

void Lista_de_Preguntas(FILE * Archivo_Latex, GtkWidget *PB, long double base, long double limite)
{
   int i, actual;
   char ejercicio_actual [CODIGO_EJERCICIO_SIZE] = "00000";
   char PG_command [9000];
   PGresult *res;
   char hilera_antes [9000], hilera_despues [9000];

   int N_preguntas_ejercicio;


   actual      = 0;
   strcpy (ejercicio_actual, preguntas[actual].ejercicio);

   N_preguntas_ejercicio = 0;
   for (i = actual; (i < N_preguntas) && (strcmp(ejercicio_actual, preguntas[i].ejercicio) == 0); i++) N_preguntas_ejercicio++;

   sprintf (PG_command,"SELECT usa_header, texto_header from bd_texto_ejercicios, bd_ejercicios where codigo_ejercicio = '%s' and texto_ejercicio = consecutivo_texto",
                       ejercicio_actual);
   res = PQEXEC(DATABASE, PG_command);

   if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

   if (*PQgetvalue (res, 0, 0) == 't') /* usa_header */
      {
       if (N_preguntas_ejercicio > 1)
	  {
	   if (N_preguntas_ejercicio == 2)
	      fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
	   else
	      fprintf (Archivo_Latex, "\\textbf{Las preguntas %d a %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	   strcpy (hilera_antes, PQgetvalue (res, 0, 1)); /* texto_header */
           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf (Archivo_Latex, "%s\n", hilera_despues);
           fprintf (Archivo_Latex, "\\rule{8.9cm}{1pt}\n");
           fprintf (Archivo_Latex, "\\begin{questions}\n");
	  }
       else
	  {
           fprintf (Archivo_Latex, "\\begin{questions}\n");
	   Imprime_pregunta (actual, Archivo_Latex, PQgetvalue (res, 0, 1));
	   actual++;
	   N_preguntas_ejercicio = 0;
	  }
      }
   else
      {
       fprintf (Archivo_Latex, "\\begin{questions}\n");
      }
   PQclear(res);

   for (i=0; i < N_preguntas_ejercicio; i++)
       {
        Imprime_pregunta (actual+i, Archivo_Latex," ");
       }

   if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

   actual += N_preguntas_ejercicio;

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.5 + ((long double) actual/N_preguntas * 0.3));
   while (gtk_events_pending ()) gtk_main_iteration ();

   while (actual < N_preguntas)
         {
          strcpy (ejercicio_actual, preguntas[actual].ejercicio);

          N_preguntas_ejercicio = 0;
          for (i = actual; (i < N_preguntas) && (strcmp(ejercicio_actual, preguntas[i].ejercicio) == 0); i++) N_preguntas_ejercicio++;

          if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

          sprintf (PG_command,"SELECT usa_header, texto_header from bd_texto_ejercicios, bd_ejercicios where codigo_ejercicio = '%s' and texto_ejercicio = consecutivo_texto",
                       ejercicio_actual);
          res = PQEXEC(DATABASE, PG_command);

          if (*PQgetvalue (res, 0, 0) == 't') /* usa_header */
	     {
              if (N_preguntas_ejercicio > 1)
		 {
  	          if (N_preguntas_ejercicio == 2)
	             fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
	          else
	             fprintf (Archivo_Latex, "\\textbf{Las preguntas %d a %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	          strcpy (hilera_antes, PQgetvalue (res, 0, 1)); /* texto_header */
                  hilera_LATEX (hilera_antes, hilera_despues);
                  fprintf (Archivo_Latex, "%s\n", hilera_despues);

                  fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
	         }
	      else
	         {
		  Imprime_pregunta (actual, Archivo_Latex, PQgetvalue(res, 0, 1));  /* texto_header */
		  N_preguntas_ejercicio = 0;
		  actual++;
		 }
	     }

	  for (i=0; i < N_preguntas_ejercicio; i++)
	      {
	       Imprime_pregunta (actual+i,Archivo_Latex, " ");
	      }

          if (N_preguntas_ejercicio > 1) fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

          actual += N_preguntas_ejercicio;
          gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_analisis), 0.5 + ((long double) actual/N_preguntas * 0.3));
          while (gtk_events_pending ()) gtk_main_iteration ();
         }

   fprintf (Archivo_Latex, "\\end{questions}\n\n");
}

void Imprime_pregunta (int i, FILE * Archivo_Latex,char * prefijo)
{
   long double cota_inferior;
   char hilera_antes [9000], hilera_despues [9000];
   char PG_command[9000];
   PGresult * res;
   long double Por_A,Por_B,Por_C,Por_D,Por_E, Total;
   gchar * materia;

   materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);

   cota_inferior = MINIMA/MAXIMA*100.0;

   sprintf (PG_command,"SELECT texto_pregunta, texto_opcion_A, texto_opcion_B, texto_opcion_C, texto_opcion_D, texto_opcion_E from bd_texto_preguntas where codigo_unico_pregunta = '%s'", preguntas[i].pregunta);
   res = PQEXEC(DATABASE, PG_command);

   sprintf (hilera_antes,"\\question %s\n%s", prefijo, PQgetvalue (res, 0, 0)); /* texto_pregunta */
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   Por_A = (long double) preguntas[i].acumulado_opciones[0]/N_estudiantes*100.0;
   Por_B = (long double) preguntas[i].acumulado_opciones[1]/N_estudiantes*100.0;
   Por_C = (long double) preguntas[i].acumulado_opciones[2]/N_estudiantes*100.0;
   Por_D = (long double) preguntas[i].acumulado_opciones[3]/N_estudiantes*100.0;
   Por_E = (long double) preguntas[i].acumulado_opciones[4]/N_estudiantes*100.0;

   fprintf (Archivo_Latex, "\\begin{answers}\n");

   Imprime_Opcion (Archivo_Latex, res, Por_A, i, 0);
   Imprime_Opcion (Archivo_Latex, res, Por_B, i, 1);
   Imprime_Opcion (Archivo_Latex, res, Por_C, i, 2);
   Imprime_Opcion (Archivo_Latex, res, Por_D, i, 3);
   Imprime_Opcion (Archivo_Latex, res, Por_E, i, 4);

   fprintf (Archivo_Latex, "\\end{answers}\n");

   Analiza_Banderas (Archivo_Latex, preguntas[i], 0, 0, 0, NULL);
   Analiza_Ajuste   (Archivo_Latex, preguntas[i], 0);

   fprintf (Archivo_Latex,"\\framebox[7.5cm][l]{%.6s.%d - %.6s - %s %s %s}\n\n",
	    preguntas[i].ejercicio, preguntas[i].secuencia, preguntas[i].pregunta, materia, preguntas[i].tema, preguntas[i].subtema);

   sprintf (hilera_antes, "\\framebox[7.5cm][l]{Autor: %s}", preguntas[i].autor);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n\n", hilera_despues);
   fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
   fprintf (Archivo_Latex, "\n\n");

   g_free (materia);
}

void Imprime_Opcion (FILE * Archivo_Latex, PGresult *res, long double Porcentaje, int pregunta, int opcion)
{
   char hilera_antes [9000], hilera_despues [9000];

   fprintf (Archivo_Latex, "\\item ");
   strcpy (hilera_antes, PQgetvalue (res, 0, 1 + opcion)); /* texto opcion*/
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex,"\n\n{\\color{green} \\rule{%Lfcm}{5pt}\n\n{\\footnotesize \\textbf{\\texttt{%6.2Lf \\%%}}}} (\\textbf{%d})\n\n",
                          0.05+MAXIMA*Porcentaje/100.0, Porcentaje, 
                          preguntas[pregunta].acumulado_opciones[opcion]);
   fprintf (Archivo_Latex," $r_{pb}$ = \\textbf{%Lf}\n\n", preguntas[pregunta].Rpb_opcion[opcion]);

   if (preguntas[pregunta].correcta == ('A' + opcion))
      {
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   fprintf (Archivo_Latex, "\n\n");
}

void Analiza_Banderas (FILE * Archivo_Latex, struct PREGUNTA item, int beamer, int N_flags, int i, char * Descripcion)
{
   int N;
   int Ajuste;
   long double alfa;
   char mensaje [1000];

   Ajuste = (item.revision_especial != PREGUNTA_NORMAL);
   N = Ajuste;

   alfa = atof(gtk_editable_get_chars(GTK_EDITABLE(EN_alfa_real), 0, -1));

   if (item.flags[RPB_BUENO])
      {
       sprintf (mensaje,"Esta pregunta muestra un buen \\'{i}ndice de discrimaci\\'{o}n ($r_{pb}$ = \\textbf{%6.4Lf}). Separa muy bien a los estudiantes de buen rendimiento de los de bajo rendimiento.",
		               item.Rpb);
       Cajita_con_bandera (Archivo_Latex, mensaje, AZUL, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[RPB_DISTRACTOR_MUY_NEGATIVO])
      {
       sprintf (mensaje,"Hay al menos un \\textbf{distractor} con un $r_{pb}$ muy negativo, que atrajo a los estudiantes de bajo rendimiento y no fue considerado por los estudiantes de buen rendimiento. \\textbf{Buen distractor}.");
       Cajita_con_bandera (Archivo_Latex, mensaje, CYAN, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[MAS_DE_3_DISTRACTORES])
      {
       sprintf (mensaje,"Por lo menos 4 opciones diferentes fueron escogidas por los estudiantes.");
       Cajita_con_bandera (Archivo_Latex, mensaje, CYAN, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[SUBE_MAS_DE_DESVIACION])
      {
       sprintf (mensaje,"La media de respuestas correctas a esta pregunta (\\textbf{%6.4Lf}) \\textbf{supera} en m\\'{a}s de una desviaci\\'{o}n est\\'{a}ndar (\\textbf{%6.4Lf}) a la media hist\\'{o}rica o estimada (\\textbf{%6.4Lf}).",
		item.porcentaje, item.desviacion, item.previo);
       Cajita_con_bandera (Archivo_Latex, mensaje, CYAN, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[TODAS_BUENAS])
      {
       sprintf (mensaje,"\\textbf{Todos} los estudiantes contestaron correctamente, por lo que la pregunta no discrimina de manera efectiva.");
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[PORCENTAJE_MENOR_30])
      {
       sprintf (mensaje,"S\\'{o}lo el \\textbf{%5.2Lf}\\%% de los estudiantes contestaron correctamente esta pregunta.",
		item.porcentaje*100);
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[BAJA_MAS_DE_DESVIACION])
      {
       sprintf (mensaje,"La media de respuestas correctas a esta pregunta (\\textbf{%6.4Lf}) es \\textbf{menor} en m\\'{a}s de una desviaci\\'{o}n est\\'{a}ndar (\\textbf{%6.4Lf}) a la media hist\\'{o}rica o estimada (\\textbf{%6.4Lf}).",
		item.porcentaje, item.desviacion, item.previo);
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[ALFA_SUBE])
      {
       sprintf (mensaje,"Si esta pregunta se elimina del examen, el $\\alpha$ de Cronbach subir\\'{i}a de \\textbf{%6.4Lf} a \\textbf{%6.4Lf}.",
		alfa, item.alfa_sin);
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[RPB_DISTRACTOR_POSITIVO])
      {
       sprintf (mensaje,"Hay al menos un \\textbf{distractor} con un $r_{pb}$ ligeramente positivo. Revisar enunciado y opciones.");
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[RPB_NEGATIVO])
      {
       sprintf (mensaje, "Esta pregunta muestra un \\'{i}ndice de discrimaci\\'{o}n ($r_{pb}$ = \\textbf{%6.4Lf}) negativo. Los estudiantes de buen rendimiento en este examen tendieron a equivocarse, mientras que los de bajo rendimiento tendieron a contestarla bien. \\textbf{Revisar muy bien el enunciado y las opciones de la pregunta}",
		               item.Rpb);
       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[TODAS_MALAS])
      {
       sprintf (mensaje,"\\textbf{Todos} los estudiantes contestaron equivocadamente esta pregunta. La pregunta no discrimina de manera efectiva y posiblemente est\\'a mal redactada.");

       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[RPB_MUY_NEGATIVO])
      {
       sprintf (mensaje, "Esta pregunta muestra un \\'{i}ndice de discrimaci\\'{o}n ($r_{pb}$ = \\textbf{%6.4Lf}) muy negativo. Los estudiantes de buen rendimiento en este examen tendieron a equivocarse, mientras que los de bajo rendimiento tendieron a contestarla bien. \\textbf{Revisar muy bien el enunciado y las opciones de la pregunta}",
		               item.Rpb);
       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[ALFA_SUBE_MUCHO])
      {
       sprintf (mensaje, "Si esta pregunta se elimina del examen, el $\\alpha$ de Cronbach subir\\'{i}a \\textbf{considerablemente} (de \\textbf{%6.4Lf} a \\textbf{%6.4Lf}).",
		alfa, item.alfa_sin);

       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[RPB_DISTRACTOR_MUY_POSITIVO])
      {
       sprintf (mensaje,"Hay al menos un \\textbf{distractor} con un $r_{pb}$ muy positivo, esto significa que, pese a ser incorrecto, atrajo a los estudiantes de mejor rendimiento. \\textbf{Revisar cuidadosamente enunciado y opciones}.");
       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[RPB_Y_DIFICULTAD_OK])
      {
       sprintf (mensaje,"Esta pregunta no es f\\'acil (\\textit{p} = \\textbf{%6.4Lf}) y tiene un buen \\'indice de discriminaci\\'on ($r_{pb}$ = \\textbf{%6.4Lf}). \\textbf{Buena pregunta}\n", 
		item.porcentaje, item.Rpb);

       Cajita_con_bandera (Archivo_Latex, mensaje, AZUL, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }

   if (item.flags[RPB_DIFI_DISTRA_OK])
      {
       sprintf (mensaje,"Esta pregunta no es f\\'acil (\\textit{p} = \\textbf{%6.4Lf}), muestra un buen \\'indice de discriminaci\\'on ($r_{pb}$ = \\textbf{%6.4Lf}), y los estudiantes usaron al menos 4 de las opciones. \\textbf{Excelente pregunta}\n", 
		item.porcentaje, item.Rpb);

       Cajita_con_bandera (Archivo_Latex, mensaje, VERDE, beamer);
       N++;
       if (beamer && ((N % 3) == 0) && N < (N_flags + Ajuste))
          Continuar_banderas(Archivo_Latex, i, Descripcion);
      }
}

void Continuar_banderas (FILE * Archivo_Latex, int i, char * Descripcion)
{
  char hilera_antes   [9000];
  char hilera_despues [9000];

  fprintf (Archivo_Latex, "\\end{frame}\n");
  fprintf (Archivo_Latex, "}\n");

  fprintf (Archivo_Latex, "{\n");
  sprintf (hilera_antes, "\\begin{frame}{Análisis de Pregunta %d - cont. \\hfill {\\small %s}}", i, Descripcion);
  hilera_LATEX (hilera_antes, hilera_despues);
  fprintf (Archivo_Latex, "%s\n", hilera_despues);
}

void Analiza_Ajuste (FILE * Archivo_Latex, struct PREGUNTA item, int modo)
{
   if (item.revision_especial == NO_CONSIDERAR)    
     Cajita_con_bandera (Archivo_Latex, "\\textbf{Ajuste:} Pregunta no ser\\'a tomada en cuenta para la evaluaci\\'on.", AJUSTE, modo);

   if (item.revision_especial == VARIAS_CORRECTAS) 
     Cajita_con_bandera (Archivo_Latex, "\\textbf{Ajuste:} Se considerar\\'an varias opciones como correctas.", AJUSTE, modo);

   if (item.revision_especial == BUENA_TODOS)      
     Cajita_con_bandera (Archivo_Latex, "\\textbf{Ajuste:} Debido a problemas en su formulaci\\'on, esta pregunta se da como \\textbf{correcta} a todos los estudiantes.", AJUSTE, modo);

   if (item.revision_especial == MALA_TODOS)
     Cajita_con_bandera (Archivo_Latex, "\\textbf{Ajuste:} Debido a situaciones especiales, esta pregunta se da como \\textbf{incorrecta} a todos los estudiantes.", AJUSTE, modo);
      
   if (item.revision_especial == SOLO_SI_BUENA)
     Cajita_con_bandera (Archivo_Latex, "\\textbf{Ajuste:} Por caracter\\'isticas especiales (dificultad, ambig\\\"uedad, falta de informaci\\'on, falta de tiempo, etc.), esta pregunta ser\\'a tomada en cuenta \\'unicamente para los estudiantes que la contestaron correctamente.", AJUSTE, modo);

   if (item.revision_especial == REVISA_Y_BONO)
     Cajita_con_bandera (Archivo_Latex, "\\textbf{Ajuste:} Esta pregunta otorga un bono a los estudiantes que la contestaron correctamente.", AJUSTE, modo);

   if (item.revision_especial == PREGUNTA_EXTRA)
     Cajita_con_bandera (Archivo_Latex, "\\textbf{Ajuste:} Esta pregunta es \\textbf{extra} al examen, dando puntos adicionales sobre 100 a los que la contesten correctamente.", AJUSTE, modo);
      
}

void Cajita_con_bandera (FILE * Archivo_Latex, char * mensaje, int color, int modo)
{
   int i;

   Banderas [color]++;

   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{4\\fboxrule}\n");
   fprintf (Archivo_Latex, "\\fcolorbox{%s}{white}{\n", colores[color]);

   fprintf (Archivo_Latex, "\\begin{minipage}{1.4 cm}\n");
   if (color == AJUSTE)
      fprintf (Archivo_Latex, "\\includegraphics[scale=0.27]{.imagenes/%s}\n", banderas[color]);
   else
      fprintf (Archivo_Latex, "\\includegraphics[scale=0.07, angle=45]{.imagenes/%s}\n", banderas[color]);

   fprintf (Archivo_Latex, "\\end{minipage}\n");

   if (modo == 1)
      fprintf (Archivo_Latex, "\\begin{minipage}{8.6 cm}\n");
   else
      fprintf (Archivo_Latex, "\\begin{minipage}{6.1 cm}\n");

   fprintf (Archivo_Latex, "\\small{%s}\n", mensaje); 
   fprintf (Archivo_Latex, "\\end{minipage}\n");
   fprintf (Archivo_Latex, "}\n");
   fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.25\\fboxrule}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n");
}

void Resumen_de_Banderas (FILE * Archivo_Latex, int modo)
{
   int color;

   if (!modo)
      fprintf (Archivo_Latex, "\\center{\\textbf{Resumen de Observaciones}}\n\n");

   for (color=0; color<5; color++)
       {
        fprintf (Archivo_Latex, "\\begin{minipage}{3.50 cm}\n");
        fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
        fprintf (Archivo_Latex, "\\centering\n");
        fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{4\\fboxrule}\n");
        fprintf (Archivo_Latex, "\\fcolorbox{%s}{white}{\n", colores[color]);
        fprintf (Archivo_Latex, "\\begin{minipage}{1.5 cm}\n");
        fprintf (Archivo_Latex, "\\includegraphics[scale=0.07, angle=45]{.imagenes/%s}\n", banderas[color]);
        fprintf (Archivo_Latex, "\\end{minipage}\n");
        fprintf (Archivo_Latex, "\\begin{minipage}{0.5 cm}\n");
        fprintf (Archivo_Latex, "\\large{\\textbf{%d}}\n", Banderas[color]); 
        fprintf (Archivo_Latex, "\\end{minipage}\n");
        fprintf (Archivo_Latex, "}\n");
        fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.25\\fboxrule}\n");
        fprintf (Archivo_Latex, "\\end{figure}\n");
        fprintf (Archivo_Latex, "\\end{minipage}\n");
       }

   fprintf (Archivo_Latex, "\\begin{minipage}{3.50 cm}\n");
   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{4\\fboxrule}\n");
   fprintf (Archivo_Latex, "\\fcolorbox{%s}{white}{\n", colores[AJUSTE]);
   fprintf (Archivo_Latex, "\\begin{minipage}{1.5 cm}\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=0.27]{.imagenes/%s}\n", banderas[AJUSTE]);
   fprintf (Archivo_Latex, "\\end{minipage}\n");
   fprintf (Archivo_Latex, "\\begin{minipage}{0.5 cm}\n");
   fprintf (Archivo_Latex, "\\large{\\textbf{%d}}\n", Banderas[AJUSTE]); 
   fprintf (Archivo_Latex, "\\end{minipage}\n");
   fprintf (Archivo_Latex, "}\n");
   fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.25\\fboxrule}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n");
   fprintf (Archivo_Latex, "\\end{minipage}\n");
}

void Lista_de_Notas (FILE * Archivo_Latex)
{
   int i, k, N_estudiantes;
   char examen[10];
   char PG_command [9000];
   PGresult *res;
   long double Porcentaje, Porcentaje_ajustado;
   int N_correctas, N_ajustado, M_ajustado;
   char hilera_antes [9000], hilera_despues [9000];

   k = (int) gtk_spin_button_get_value_as_int (SP_examen);
   sprintf (examen, "%05d", k);

   sprintf (PG_command,"SELECT nombre, version, respuestas, correctas, porcentaje from EX_examenes_respuestas where examen = '%s' order by nombre", examen);
   res = PQEXEC(DATABASE, PG_command);
   N_estudiantes = PQntuples(res);

   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\begin{center}\n");
   fprintf (Archivo_Latex, "\\begin{longtable}{|l|c|c|c|c|c|}\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Nombre} & \\textbf{Versi\\'{o}n} & \\textbf{Correctas} & \\textbf{Porcentaje} & \\textbf{Ajuste} & \\textbf{Nota Ajustada}\\\\ \\hline \\hline\n");
   fprintf (Archivo_Latex, "\\endfirsthead\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Nombre} & \\textbf{Versi\\'{o}n} & \\textbf{Correctas} & \\textbf{Porcentaje} & \\textbf{Ajuste} & \\textbf{Nota Ajustada}\\\\ \\hline \\hline\n");
   fprintf (Archivo_Latex, "\\endhead\n");

   for (i=0; i< N_estudiantes; i++)
       {
        Calcula_Notas (PQgetvalue(res, i, 1),
 		       PQgetvalue(res, i, 2),
		       &N_correctas, &N_ajustado, &M_ajustado);
        Porcentaje = (long double) N_correctas/N_preguntas * 100.0;
        if (M_ajustado)
           Porcentaje_ajustado = (long double) N_ajustado /M_ajustado  * 100.0;
        else
	   Porcentaje_ajustado = 0.0;

	sprintf (hilera_antes,"%s & %s & %d/%d & %7.2Lf & %d/%d & \\textbf{%7.2Lf} \\\\ \\hline", 
		              PQgetvalue (res, i, 0), PQgetvalue (res, i, 1),
		              N_correctas, N_preguntas, Porcentaje,
   		              N_ajustado, M_ajustado, Porcentaje_ajustado);

        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s\n", hilera_despues);
       }

   fprintf (Archivo_Latex, "\\end{longtable}\n");
   fprintf (Archivo_Latex, "\\end{center}\n");
}

void Calcula_Notas (char * version, char * respuestas, int * n_buenas, int * n_ajustado, int * m_ajustado)
{
  int i, k_version;
  int N, K, M;

  for (k_version=0; (k_version < N_versiones) && strcmp (versiones[k_version].codigo, version) != 0; k_version++);

  /* Revisión Normal */
  N = 0;
  for (i=0; i<N_preguntas; i++)
      if (versiones[k_version].preguntas [i].respuesta == respuestas[i]) N++;

  /* Revisión Ajustada */
  if (N_ajustes == 0)
     {
      K = N;
      M = N_preguntas;
     }
  else
     Calcula_nota_ajustada(k_version, respuestas, &K, &M);

  *n_buenas   = N;
  *n_ajustado = K;
  *m_ajustado = M;
}

void Calcula_nota_ajustada (int k_version, char * respuestas, int * n, int * m)
{
  int i, k;
  int N_correctas, N_evaluadas;

  N_correctas = N_evaluadas = 0;
  for (i=0; i<N_preguntas; i++)
      {
	/* Se encuentra el ajuste de revision de la pregunta ==> k*/
       for (k=0; (k < N_preguntas) && strcmp (versiones[k_version].preguntas[i].codigo, preguntas[k].pregunta) != 0; k++);

       switch (preguntas[k].revision_especial)
	      {
	       case PREGUNTA_NORMAL: /* Revisar normal */
                    if (versiones[k_version].preguntas [i].respuesta == respuestas [i]) N_correctas++;
                    N_evaluadas++;
		    break;

	       case NO_CONSIDERAR: /* No revisar ni incrementar contadores */
		    break;

	       case BUENA_TODOS: /* No revisar, incrementar correctas y evaluadas */
		    N_correctas++;
		    N_evaluadas++;
		    break;

	       case VARIAS_CORRECTAS: /* PENDIENTE */
		    break;

	       case MALA_TODOS: /* No revisar, incrementar evaluadas */
		    N_evaluadas++;
		    break;

	       case SOLO_SI_BUENA: /* Tomar en cuenta sólo si la tienen buena */
                    if (versiones[k_version].preguntas [i].respuesta == respuestas [i]) 
		       {
                        N_correctas++;
                        N_evaluadas++;
		       }
		    break;

	       case REVISA_Y_BONO: /* Revisar normal pero dar bono si está correcta*/
                    if (versiones[k_version].preguntas [i].respuesta == respuestas [i]) N_correctas+=2;
                    N_evaluadas++;
		    break;

	       case PREGUNTA_EXTRA: /* Tomar en cuenta sólo si la tienen buena */
                    if (versiones[k_version].preguntas [i].respuesta == respuestas [i]) N_correctas++;
		    break;
	      }
      }

  *n = N_correctas;
  *m = N_evaluadas;
}

void Cambia_Pregunta ()
{
  int i, k;
  GdkColor color;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k && preguntas)
     {
      Color_ajustes(k-1);
      gtk_combo_box_set_active (CB_ajuste, preguntas[k-1].revision_especial);
      if (preguntas [k-1].revision_especial != PREGUNTA_NORMAL)
	 {
	  if (preguntas[k-1].actualizar)
             gtk_toggle_button_set_active(CK_no_actualiza, FALSE);
	  else
             gtk_toggle_button_set_active(CK_no_actualiza, TRUE);
	  if (preguntas[k-1].revision_especial == VARIAS_CORRECTAS)
	     {
	      if (preguntas[k-1].correctas_nuevas[0]) gtk_toggle_button_set_active(TG_A, TRUE);
	      if (preguntas[k-1].correctas_nuevas[1]) gtk_toggle_button_set_active(TG_B, TRUE);
	      if (preguntas[k-1].correctas_nuevas[2]) gtk_toggle_button_set_active(TG_C, TRUE);
	      if (preguntas[k-1].correctas_nuevas[3]) gtk_toggle_button_set_active(TG_D, TRUE);
	      if (preguntas[k-1].correctas_nuevas[4]) gtk_toggle_button_set_active(TG_E, TRUE);
	     }
	 }

      gtk_text_buffer_set_text (buffer_TV_pregunta, preguntas[k-1].texto_pregunta, -1);

      if (preguntas[k-1].grupo_inicio == preguntas[k-1].grupo_final)
	 {
          gtk_toggle_button_set_active(CK_header_encoger,  FALSE);
          gtk_toggle_button_set_active(CK_header_verbatim, FALSE);
          gtk_widget_set_sensitive(GTK_WIDGET(CK_header_encoger),  0);
          gtk_widget_set_sensitive(GTK_WIDGET(CK_header_verbatim), 0);
	 }
      else
	 {
	  gtk_widget_set_sensitive(GTK_WIDGET(CK_header_encoger),  1);
          gtk_widget_set_sensitive(GTK_WIDGET(CK_header_verbatim), 1);

          if (preguntas[k-1].header_encoger)
             gtk_toggle_button_set_active(CK_header_encoger, TRUE);
          else
             gtk_toggle_button_set_active(CK_header_encoger, FALSE);

          if (preguntas[k-1].header_verbatim)
             gtk_toggle_button_set_active(CK_header_verbatim, TRUE);
          else
             gtk_toggle_button_set_active(CK_header_verbatim, FALSE);
	 }


      if (preguntas[k-1].excluir)
         gtk_toggle_button_set_active(CK_excluir, TRUE);
      else
         gtk_toggle_button_set_active(CK_excluir, FALSE);

      if (preguntas[k-1].encoger)
         gtk_toggle_button_set_active(CK_encoger, TRUE);
      else
         gtk_toggle_button_set_active(CK_encoger, FALSE);

      if (preguntas[k-1].verbatim)
         gtk_toggle_button_set_active(CK_verbatim, TRUE);
      else
         gtk_toggle_button_set_active(CK_verbatim, FALSE);

      /************************************/
      /* Displays "new slide" check marks */
      /************************************/
      for (i=0; i<5; i++) 
	  {
           if (preguntas[k-1].slide[i])
              gtk_toggle_button_set_active(CK_slide[i], TRUE);
           else
              gtk_toggle_button_set_active(CK_slide[i], FALSE);
	  }

      /************************************/
      /* Displays "shrink" check marks    */
      /************************************/
      for (i=0; i<5; i++)
	  {
           if (preguntas[k-1].encoger_opcion[i])
              gtk_toggle_button_set_active(CK_encoger_opcion[i], TRUE);
           else
              gtk_toggle_button_set_active(CK_encoger_opcion[i], FALSE);
	  }

      /************************************/
      /* Displays "verbatim" check marks    */
      /************************************/
      for (i=0; i<5; i++)
	  {
           if (preguntas[k-1].verbatim_opcion[i])
              gtk_toggle_button_set_active(CK_verbatim_opcion[i], TRUE);
           else
              gtk_toggle_button_set_active(CK_verbatim_opcion[i], FALSE);
	  }
     }
}

void Cambio_Ajuste (GtkWidget *widget, gpointer user_data)
{
  int j, k, previo;
  GdkColor color;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));
  j = gtk_combo_box_get_active(CB_ajuste);

  if (k && preguntas)
     {
      previo                = preguntas[k-1].revision_especial;
      preguntas[k-1].revision_especial = j;

      if (j != VARIAS_CORRECTAS)
	 {
          gtk_toggle_button_set_active(TG_A, FALSE);     
          gtk_toggle_button_set_active(TG_B, FALSE);     
          gtk_toggle_button_set_active(TG_C, FALSE);     
          gtk_toggle_button_set_active(TG_D, FALSE);     
          gtk_toggle_button_set_active(TG_E, FALSE);     

          gtk_widget_set_sensitive(GTK_WIDGET(TG_A), 0);
          gtk_widget_set_sensitive(GTK_WIDGET(TG_B), 0);
          gtk_widget_set_sensitive(GTK_WIDGET(TG_C), 0);
          gtk_widget_set_sensitive(GTK_WIDGET(TG_D), 0);
          gtk_widget_set_sensitive(GTK_WIDGET(TG_E), 0);
	 }
      else
	 {
	  gtk_widget_set_sensitive(GTK_WIDGET(TG_A), 1);
	  gtk_widget_set_sensitive(GTK_WIDGET(TG_B), 1);
	  gtk_widget_set_sensitive(GTK_WIDGET(TG_C), 1);
	  gtk_widget_set_sensitive(GTK_WIDGET(TG_D), 1);
	  gtk_widget_set_sensitive(GTK_WIDGET(TG_E), 1);
	 }

      if (j == PREGUNTA_NORMAL)
	 {
          gtk_toggle_button_set_active(CK_no_actualiza, FALSE);
          gtk_widget_set_sensitive    (GTK_WIDGET(CK_no_actualiza), 0);
          preguntas[k-1].actualizar = 1;
	 }
      else
	 {
          gtk_widget_set_sensitive(GTK_WIDGET(CK_no_actualiza), 1);
	  if (previo == PREGUNTA_NORMAL)
             gtk_toggle_button_set_active(CK_no_actualiza, TRUE);
	  else
	     {
	      if (preguntas[k-1].actualizar)
                 gtk_toggle_button_set_active(CK_no_actualiza, FALSE);
	      else
                 gtk_toggle_button_set_active(CK_no_actualiza, TRUE);
	     }
	 }

      Color_ajustes(k-1);
     }

  N_ajustes = 0;
  for (j=0;j<N_preguntas;j++) if (preguntas[j].revision_especial != PREGUNTA_NORMAL) N_ajustes++;
}

void Cambio_no_actualizar (GtkWidget *widget, gpointer user_data)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k)
     {
      if (gtk_toggle_button_get_active(CK_no_actualiza))
         preguntas[k-1].actualizar = 0;
      else
         preguntas[k-1].actualizar = 1;
     }
}

void Cambio_excluir (GtkWidget *widget, gpointer user_data)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k)
     {
      if (gtk_toggle_button_get_active(CK_excluir))
         preguntas[k-1].excluir = 1;
      else
         preguntas[k-1].excluir = 0;

      Color_ajustes(k-1);
     }
}

void Cambio_encoger (GtkWidget *widget, gpointer user_data)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k)
     {
      if (gtk_toggle_button_get_active(CK_encoger))
         preguntas[k-1].encoger = 1;
      else
         preguntas[k-1].encoger = 0;

      Color_ajustes(k-1);
     }
}

void Cambio_verbatim (GtkWidget *widget, gpointer user_data)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k)
     {
      if (gtk_toggle_button_get_active(CK_verbatim))
         preguntas[k-1].verbatim = 1;
      else
         preguntas[k-1].verbatim = 0;

      Color_ajustes(k-1);
     }
}

void Cambio_header_encoger (GtkWidget *widget, gpointer user_data)
{
  int k, j;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k)
     {
      if (gtk_toggle_button_get_active(CK_header_encoger))
	 for (j=preguntas[k-1].grupo_inicio; j <= preguntas[k-1].grupo_final; j++)
             preguntas[j].header_encoger = 1;
      else
	 for (j=preguntas[k-1].grupo_inicio; j <= preguntas[k-1].grupo_final; j++)
             preguntas[j].header_encoger = 0;

      Color_ajustes(k-1);
     }
}

void Cambio_header_verbatim (GtkWidget *widget, gpointer user_data)
{
  int k, j;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k)
     {
      if (gtk_toggle_button_get_active(CK_header_verbatim))
	 for (j=preguntas[k-1].grupo_inicio; j <= preguntas[k-1].grupo_final; j++)
             preguntas[j].header_verbatim = 1;
      else
	 for (j=preguntas[k-1].grupo_inicio; j <= preguntas[k-1].grupo_final; j++)
             preguntas[j].header_verbatim = 0;

      Color_ajustes(k-1);
     }
}

void Cambio_slide (int i)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k)
     {
      if (gtk_toggle_button_get_active(CK_slide[i]))
	 {
          preguntas[k-1].slide[i] = 1;

          gtk_widget_set_sensitive(GTK_WIDGET(CK_encoger_opcion  [i]),  1);
          gtk_widget_set_sensitive(GTK_WIDGET(CK_verbatim_opcion [i]), 1);
	 }
      else
	 {
          preguntas[k-1].slide           [i] = 0;
          preguntas[k-1].encoger_opcion  [i] = 0;
          preguntas[k-1].verbatim_opcion [i] = 0;
          gtk_toggle_button_set_active(CK_encoger_opcion  [i], FALSE);
          gtk_toggle_button_set_active(CK_verbatim_opcion [i], FALSE);
          gtk_widget_set_sensitive(GTK_WIDGET(CK_encoger_opcion [i]), 0);
          gtk_widget_set_sensitive(GTK_WIDGET(CK_verbatim_opcion[i]), 0);
	 }

      Color_ajustes(k-1);
     }
}

void Cambio_encoger_opcion (int i)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k)
     {
      if (gtk_toggle_button_get_active(CK_encoger_opcion[i]))
         preguntas[k-1].encoger_opcion[i] = 1;
      else
         preguntas[k-1].encoger_opcion[i] = 0;

      Color_ajustes(k-1);
     }
}

void Cambio_verbatim_opcion (int i)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  if (k)
     {
      if (gtk_toggle_button_get_active(CK_verbatim_opcion[i]))
         preguntas[k-1].verbatim_opcion[i] = 1;
      else
         preguntas[k-1].verbatim_opcion[i] = 0;

      Color_ajustes(k-1);
     }
}

void Cambio_A (GtkWidget *widget, gpointer user_data)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  preguntas [k-1].correctas_nuevas [0] = gtk_toggle_button_get_active(TG_A)?1:0;

  Color_ajustes(k-1);
}

void Cambio_B (GtkWidget *widget, gpointer user_data)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  preguntas [k-1].correctas_nuevas [1] = gtk_toggle_button_get_active(TG_B)?1:0;

  Color_ajustes(k-1);
}

void Cambio_C (GtkWidget *widget, gpointer user_data)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  preguntas [k-1].correctas_nuevas [2] = gtk_toggle_button_get_active(TG_C)?1:0;

  Color_ajustes(k-1);
}

void Cambio_D (GtkWidget *widget, gpointer user_data)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  preguntas [k-1].correctas_nuevas [3] = gtk_toggle_button_get_active(TG_D)?1:0;

  Color_ajustes(k-1);
}

void Cambio_E (GtkWidget *widget, gpointer user_data)
{
  int k;

  k = (int) gtk_range_get_value (GTK_RANGE(SC_preguntas));

  preguntas [k-1].correctas_nuevas [4] = gtk_toggle_button_get_active(TG_E)?1:0;

  Color_ajustes(k-1);
}

void Color_ajustes(int k)
{
  GdkColor color;

  Calcula_ajuste(k);


  if (preguntas [k].ajuste - preguntas [k].revision_especial > 0)
     { /* Hay ajustes para Beamer */
      gdk_color_parse (RARE_AREA, &color);
      gtk_widget_modify_bg(EB_formato,     GTK_STATE_NORMAL,   &color);
     }
  else
     {
      gdk_color_parse (SECONDARY_AREA, &color);
      gtk_widget_modify_bg(EB_formato,     GTK_STATE_NORMAL,   &color);
     }

  if (preguntas [k].revision_especial != PREGUNTA_NORMAL)
     {
      gdk_color_parse (RARE_AREA, &color);
      gtk_widget_modify_bg(EB_ajustes,     GTK_STATE_NORMAL,   &color);
     }
  else
     {
      gdk_color_parse (SECONDARY_AREA, &color);
      gtk_widget_modify_bg(EB_ajustes,     GTK_STATE_NORMAL,   &color);
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

void Prepara_Grafico_Normal(long double media, long double desviacion, long double media_pred, long double desviacion_pred, long double width)
{
    int i;
    FILE * Archivo_gnuplot;
    char Hilera_Antes [9000], Hilera_Despues [9000];
    char comando[9000];

    Archivo_gnuplot = fopen ("EX4010-n.gp","w");

    fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 12\n");
    fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
    fprintf (Archivo_gnuplot, "set size 0.9, 0.9\n");
    fprintf (Archivo_gnuplot, "set grid xtics\n");

    fprintf (Archivo_gnuplot, "set output \"EX4010-n.eps\"\n");
    hilera_GNUPLOT ("set ylabel \"Proporción\"\n", Hilera_Despues);
    fprintf (Archivo_gnuplot, "%s", Hilera_Despues);
    hilera_GNUPLOT ("set xlabel \"Nota\"\n", Hilera_Despues);
    fprintf (Archivo_gnuplot, "%s", Hilera_Despues);

    fprintf (Archivo_gnuplot, "set xrange [%Lf:%Lf]\n", (media - width) - 3.0, (media + width) + 3.0);
    fprintf (Archivo_gnuplot, "load \".scripts/stat.inc\"\n");
    fprintf (Archivo_gnuplot, "set style fill solid 0.3 border 1\n");

    fprintf (Archivo_gnuplot, "set boxwidth 1.0\n");

    fprintf (Archivo_gnuplot, "set arrow from %Lf,0.0 to %Lf,graph(1, 1) linetype 2 lw 4 lc 1 nohead front\n", media, media); 
    fprintf (Archivo_gnuplot, "set arrow from %Lf,0.0 to %Lf,graph(1, 1) linetype 2 lw 4 lc 1 nohead front\n", media_pred, media_pred); 

    sprintf (Hilera_Antes, "plot normal (x, %Lf, %Lf) with lines linetype 1 lw 7 lc 3 title \"Predicción\", normal (x, %Lf, %Lf) with lines linetype 1 lw 7 lc 2 title \"Real\"",
             media_pred, desviacion_pred, media, desviacion);
    hilera_GNUPLOT (Hilera_Antes, Hilera_Despues);
    fprintf (Archivo_gnuplot, "%s\n", Hilera_Despues);


    fclose (Archivo_gnuplot);

    sprintf(comando, "%s EX4010-n.gp", parametros.gnuplot);
    system (comando);

    sprintf(comando, "mv EX4010-n.gp %s", parametros.ruta_gnuplot);
    system (comando);

    sprintf(comando, "%s EX4010-n.eps", parametros.epstopdf);
    system (comando);

    system ("rm EX4010-n.eps");
}

void Prepara_Histograma_Notas()
{
    int i, maximo;
    FILE * Archivo_gnuplot, *Archivo_Datos;
    char Hilera_Antes [9000], Hilera_Despues [9000];
    char comando[9000];

    maximo = 0;
    Archivo_Datos   = fopen ("EX4010y.dat","w");
    for (i=0; i < CATEGORIAS; i++)
        {
 	 if (Frecuencias[i]) fprintf(Archivo_Datos,"%Lf %Lf\n", (long double)i*(100.0/(long double) CATEGORIAS), (long double) Frecuencias[i]);

	 if (Frecuencias[i] > maximo) maximo = Frecuencias[i];
        }
    fclose (Archivo_Datos);

    Archivo_gnuplot = fopen ("EX4010-h.gp","w");

    fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 12\n");
    fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
    fprintf (Archivo_gnuplot, "set size 0.9, 0.9\n");
    fprintf (Archivo_gnuplot, "set grid xtics\n");

    fprintf (Archivo_gnuplot, "set output \"EX4010-h.eps\"\n");
    hilera_GNUPLOT ("set ylabel \"Cantidad\"\n", Hilera_Despues);
    fprintf (Archivo_gnuplot, "%s", Hilera_Despues);
    hilera_GNUPLOT ("set xlabel \"Nota\"\n", Hilera_Despues);
    fprintf (Archivo_gnuplot, "%s", Hilera_Despues);

    fprintf (Archivo_gnuplot, "set xrange [0.0:100.0]\n");
    fprintf (Archivo_gnuplot, "set xtics 10\n");
    fprintf (Archivo_gnuplot, "set yrange [0.0:%d]\n",maximo+2);

    fprintf (Archivo_gnuplot, "set style fill solid 1.0 border -1\n");

    fprintf (Archivo_gnuplot, "set boxwidth 8\n");

    fprintf (Archivo_gnuplot, "plot \"EX4010y.dat\" with boxes fill lw 3 lc 2 notitle");

    fclose (Archivo_gnuplot);

    sprintf(comando, "%s EX4010-h.gp", parametros.gnuplot);
    system (comando);

    sprintf(comando, "mv EX4010-h.gp %s", parametros.ruta_gnuplot);
    system (comando);

    sprintf(comando, "%s EX4010-h.eps", parametros.epstopdf);
    system (comando);

    system ("rm EX4010-h.eps");
    system ("rm EX4010y.dat");
}

void Prepara_Histograma_Temas()
{
    int i, maximo;
    FILE * Archivo_gnuplot, *Archivo_Datos;
    char Hilera_Antes [9000], Hilera_Despues [9000];
    char PG_command [9000];
    gchar * materia;
    PGresult *res;
    char comando[9000];

    materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);

    Archivo_Datos = fopen ("EX4010z.dat","w");
    for (i=0; i < N_temas; i++)
        {
	 fprintf (Archivo_Datos, "%d %Lf %Lf\n", i, (long double)((resumen_tema[i].buenos+resumen_tema[i].malos)/N_estudiantes)/N_preguntas * 100.0,
		                                    (long double)  resumen_tema[i].buenos / (resumen_tema[i].buenos+resumen_tema[i].malos)  * 100.0);
        }
    fclose (Archivo_Datos);

    Archivo_gnuplot = fopen ("EX4010-t.gp","w");

    fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 12\n");
    fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");

    fprintf (Archivo_gnuplot, "set grid y2tics ytics\n");
    fprintf (Archivo_gnuplot, "set output \"EX4010-t.eps\"\n");
    fprintf (Archivo_gnuplot, "set key at graph 0.24, 0.85 horizontal samplen 0.1\n");
    fprintf (Archivo_gnuplot, "set style data histogram\n");
    fprintf (Archivo_gnuplot, "set style histogram cluster gap 1\n");
    fprintf (Archivo_gnuplot, "set style fill solid border -1\n");
    fprintf (Archivo_gnuplot, "set boxwidth 0.8\n");

    fprintf (Archivo_gnuplot, "set xtics (");
    for (i=0; i < N_temas; i++)
        {
	 sprintf (PG_command,"SELECT descripcion_materia from BD_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", materia, resumen_tema[i].tema);
         res = PQEXEC(DATABASE, PG_command);

         sprintf (Hilera_Antes,"\"%.44s (%3d)\" %d", PQgetvalue (res, 0, 0),(resumen_tema[i].buenos+resumen_tema[i].malos)/N_estudiantes, i);
         hilera_GNUPLOT (Hilera_Antes, Hilera_Despues);
         fprintf (Archivo_gnuplot, "%s", Hilera_Despues);
         if (i == (N_temas - 1))
            fprintf (Archivo_gnuplot,")\n");
         else
            fprintf (Archivo_gnuplot,", ");

         PQclear(res);
        }

    fprintf (Archivo_gnuplot, "set xtics rotate by -270 scale 0\n");
    fprintf (Archivo_gnuplot, "set ytics rotate by 90\n");
    fprintf (Archivo_gnuplot, "set ytics 5,10\n");
    fprintf (Archivo_gnuplot, "set y2tics rotate by 90\n");
    fprintf (Archivo_gnuplot, "set y2tics 0,10\n");
    fprintf (Archivo_gnuplot, "set yrange [0:100.0]\n");
    fprintf (Archivo_gnuplot, "set xlabel \" \"\n");
    fprintf (Archivo_gnuplot, "set size 0.63, 2.0\n");
    fprintf (Archivo_gnuplot, "set label 2 \"%% preguntas\" at graph 0.13, 0.85 left rotate by 90\n");
    fprintf (Archivo_gnuplot, "set label 3 \"%% correctas\" at graph 0.21, 0.85 left rotate by 90\n");

    fprintf (Archivo_gnuplot, "plot \"EX4010z.dat\" using 2 title \" \" lc 3 lt 1, \"\" using 3 title \" \" lc 2 lt 1\n");

    fclose (Archivo_gnuplot);

    sprintf(comando, "%s EX4010-t.gp", parametros.gnuplot);
    system (comando);

    sprintf(comando, "mv EX4010-t.gp %s", parametros.ruta_gnuplot);
    system (comando);

    sprintf(comando, "%s EX4010-t.eps", parametros.epstopdf);
    system (comando);

    system ("rm EX4010-t.eps");
    system ("rm EX4010z.dat");

    g_free(materia);
}

void Prepara_Histograma_Subtemas()
{
    int i, maximo;
    FILE * Archivo_gnuplot, *Archivo_Datos;
    char Hilera_Antes [9000], Hilera_Despues [9000];
    char PG_command [9000];
    gchar * materia;
    PGresult *res;
    char comando[2000];

    materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);

    Archivo_Datos = fopen ("EX4010y.dat","w");
    for (i=0; i < N_subtemas; i++)
        {
	 fprintf (Archivo_Datos, "%d %Lf %Lf\n", i, (long double)((resumen_tema_subtema[i].buenos+resumen_tema_subtema[i].malos)/N_estudiantes)/N_preguntas * 100.0,
		                                    (long double)  resumen_tema_subtema[i].buenos / (resumen_tema_subtema[i].buenos+resumen_tema_subtema[i].malos)  * 100.0);
        }
    fclose (Archivo_Datos);

    Archivo_gnuplot = fopen ("EX4010-s.gp","w");

    fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 12\n");
    fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");

    fprintf (Archivo_gnuplot, "set grid y2tics ytics\n");
    fprintf (Archivo_gnuplot, "set output \"EX4010-s.eps\"\n");
    fprintf (Archivo_gnuplot, "set key at graph 0.24, 0.85 horizontal samplen 0.1\n");
    fprintf (Archivo_gnuplot, "set style data histogram\n");
    fprintf (Archivo_gnuplot, "set style histogram cluster gap 1\n");
    fprintf (Archivo_gnuplot, "set style fill solid border -1\n");
    fprintf (Archivo_gnuplot, "set boxwidth 0.8\n");

    fprintf (Archivo_gnuplot, "set xtics (");
    for (i=0; i < N_subtemas; i++)
        {
	 sprintf (PG_command,"SELECT descripcion_materia from BD_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", 
                  materia, resumen_tema_subtema[i].tema, resumen_tema_subtema[i].subtema);
         res = PQEXEC(DATABASE, PG_command);

         sprintf (Hilera_Antes,"\"%.44s (%3d)\" %d", PQgetvalue (res, 0, 0),(resumen_tema_subtema[i].buenos+resumen_tema_subtema[i].malos)/N_estudiantes, i);
         hilera_GNUPLOT (Hilera_Antes, Hilera_Despues);
         fprintf (Archivo_gnuplot, "%s", Hilera_Despues);
         if (i == (N_subtemas - 1))
            fprintf (Archivo_gnuplot,")\n");
         else
            fprintf (Archivo_gnuplot,", ");

         PQclear(res);
        }

    fprintf (Archivo_gnuplot, "set xtics rotate by -270 scale 0\n");
    fprintf (Archivo_gnuplot, "set ytics rotate by 90\n");
    fprintf (Archivo_gnuplot, "set ytics 5,10\n");
    fprintf (Archivo_gnuplot, "set y2tics rotate by 90\n");
    fprintf (Archivo_gnuplot, "set y2tics 0,10\n");
    fprintf (Archivo_gnuplot, "set yrange [0:100.0]\n");
    fprintf (Archivo_gnuplot, "set xlabel \" \"\n");
    fprintf (Archivo_gnuplot, "set size 1.15, 2.0\n");
    fprintf (Archivo_gnuplot, "set label 2 \"%% preguntas\" at graph 0.17, 0.86 left rotate by 90\n");
    fprintf (Archivo_gnuplot, "set label 3 \"%% correctas\" at graph 0.22, 0.86 left rotate by 90\n");

    fprintf (Archivo_gnuplot, "plot \"EX4010y.dat\" using 2 title \"  \" lc 3 lt 1, \"\" using 3 title \" \" lc 2 lt 1\n");

    fclose (Archivo_gnuplot);

    sprintf(comando, "%s EX4010-s.gp", parametros.gnuplot);
    system (comando);

    sprintf(comando, "mv EX4010-s.gp %s", parametros.ruta_gnuplot);
    system (comando);

    sprintf(comando, "%s EX4010-s.eps", parametros.epstopdf);
    system (comando);

    system ("rm EX4010-s.eps");
    system ("rm EX4010y.dat");

    g_free(materia);
}

long double CDF (long double X, long double Media, long double Desv)
{
  return (0.5*(1 + (long double)erf((X-Media)/(Desv*M_SQRT2))));
}

void Dificultad_vs_Discriminacion ()
{
  int i;
  FILE *Archivo_gnuplot;
  int Color, Smooth, Rotacion;

  int Total_preguntas;
  long double dificultad, discriminacion;
  int frecuencia;
  char comando[1000];
  char Hilera_Antes [9000], Hilera_Despues [9000];
  char * Colores [] = {"unset pm3d",
                       "set palette gray", 
                       "set palette gray negative", 
                       "set palette rgb 21,22,23",
                       "set palette rgb 34,35,36",
                       "set palette rgb 7,5,15",
                       "set palette rgb 3,11,6",
                       "set palette rgb 23,28,3",
                       "set palette rgb 33,13,10",
                       "set palette rgb 30,31,32"};

  Color    = (int) gtk_spin_button_get_value_as_int (SP_color);
  Rotacion = (int) gtk_spin_button_get_value_as_int (SP_rotacion);
  
  Smooth    = gtk_toggle_button_get_active(CK_smooth);


  Calcular_Tabla ();

  Archivo_gnuplot = fopen ("EX4010-p.gp","w");
      fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 12\n");
      fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
      fprintf (Archivo_gnuplot, "set pm3d\n");
      fprintf (Archivo_gnuplot, "set style line 100 lt 5 lw 0.5 lc 2\n");
      fprintf (Archivo_gnuplot, "set pm3d hidden3d 100\n");
      fprintf (Archivo_gnuplot, "%s\n", Colores[Color]);
      fprintf (Archivo_gnuplot, "set size 1.55, 1.0\n");

      if (Niveles_Discriminacion < 12)
         fprintf (Archivo_gnuplot, "set xtics -1.0, %4.3Lf, 1.0 offset 1\n",(long double)2.0/(Niveles_Discriminacion));
      else
         fprintf (Archivo_gnuplot, "set xtics -1.0, %4.3Lf, 1.0 offset 1\n",2*(long double)2.0/(Niveles_Discriminacion));

      fprintf (Archivo_gnuplot, "set ytics  0.0, %4.3Lf, 1.0 offset 2\n",(long double)1.0/(Niveles_Dificultad));

      fprintf (Archivo_gnuplot, "set grid xtics\n");
      fprintf (Archivo_gnuplot, "set grid ytics\n");
      fprintf (Archivo_gnuplot, "set contour base\n");
      fprintf (Archivo_gnuplot, "set output \"EX4010p.eps\"\n");

      hilera_GNUPLOT ("set xlabel \"Discriminación\"\n", Hilera_Despues);
      fprintf (Archivo_gnuplot, "%s", Hilera_Despues);
      fprintf (Archivo_gnuplot, "set ylabel \"Dificultad\"\n");
      fprintf (Archivo_gnuplot, "set xyplane at -0.5\n");
      fprintf (Archivo_gnuplot, "set hidden3d\n");

      if (Smooth) fprintf (Archivo_gnuplot, "set dgrid3 %d,%d,gauss 0.15 0.15\n",Niveles_Dificultad+1,Niveles_Discriminacion+1);

      fprintf (Archivo_gnuplot, "set view 60,%d,1,1\n", Rotacion);
      fprintf (Archivo_gnuplot, "splot \"EX4010p.dat\" title \" \" with lines lt 1 lw 1 lc 2\n");


  fclose (Archivo_gnuplot);

  sprintf(comando, "%s EX4010-p.gp", parametros.gnuplot);
  system (comando);

  sprintf(comando, "mv EX4010-p.gp %s", parametros.ruta_gnuplot);
  system (comando);

  sprintf(comando, "%s EX4010p.eps", parametros.epstopdf);
  system (comando);

  system ("rm EX4010p.eps");

  Archivo_gnuplot = fopen ("EX4010c.gp","w");
      fprintf (Archivo_gnuplot, "set term postscript eps enhanced color \"Times\" 10\n");
      fprintf (Archivo_gnuplot, "set encoding iso_8859_1\n");
      fprintf (Archivo_gnuplot, "set pm3d\n");
      fprintf (Archivo_gnuplot, "%s\n", Colores[Color]);
      fprintf (Archivo_gnuplot, "set size 1.55, 1.0\n");
      fprintf (Archivo_gnuplot, "set output \"EX4010c.eps\"\n");

      hilera_GNUPLOT ("set xlabel \"Discriminación\"\n", Hilera_Despues);
      fprintf (Archivo_gnuplot, "%s", Hilera_Despues);
      fprintf (Archivo_gnuplot, "set ylabel \"Dificultad\"\n");
      fprintf (Archivo_gnuplot, "set view map\n");
      fprintf (Archivo_gnuplot, "set contour\n");

      if (Niveles_Discriminacion == 18)
         fprintf (Archivo_gnuplot, "set xtics -1.0, %4.3Lf, 1.0\n",2*(long double)2.0/(Niveles_Discriminacion));
      else
         fprintf (Archivo_gnuplot, "set xtics -1.0, %4.3Lf, 1.0\n",(long double)2.0/(Niveles_Discriminacion));

      fprintf (Archivo_gnuplot, "set ytics  0.0, %4.3Lf, 1.0\n",(long double)1.0/(Niveles_Dificultad));
      fprintf (Archivo_gnuplot, "set key at -1.1, -0.07 left box lt 1\n");
      if (Smooth) fprintf (Archivo_gnuplot, "set dgrid3 %d,%d,gauss 0.15 0.15\n",Niveles_Dificultad+1,Niveles_Discriminacion+1);

      fprintf (Archivo_gnuplot, "splot \"EX4010p.dat\" notitle with lines lt 1 lw 1 lc 2\n");
  fclose (Archivo_gnuplot);

  sprintf(comando, "%s EX4010c.gp", parametros.gnuplot);
  system (comando);

  sprintf(comando, "mv EX4010c.gp %s", parametros.ruta_gnuplot);
  system (comando);
  sprintf(comando, "%s EX4010c.eps", parametros.epstopdf);
  system (comando);

  system ("rm EX4010c.eps");

  system ("rm EX4010p.dat");
}

void Calcular_Tabla()
{
  int i, j, k;
  long double dificultad, discriminacion;
  FILE * Archivo_Datos;

  Niveles_Dificultad = (int) gtk_spin_button_get_value_as_int (SP_resolucion);
  Niveles_Discriminacion = Niveles_Dificultad * 2;
  for (i=0; i <= Niveles_Dificultad; i++)
      for (j=0; j <= Niveles_Discriminacion; j++)
	  Frecuencia_total [i][j] = 0;

  for (k=0; k <N_preguntas; k++)
      {
       dificultad     = preguntas[k].porcentaje;
       discriminacion = preguntas[k].Rpb;

       i = (int) round (dificultad * Niveles_Dificultad);
       j = (int) round ((discriminacion+1.0)/2 * Niveles_Discriminacion);

       Frecuencia_total [i][j]++;
      }

  Archivo_Datos = fopen ("EX4010p.dat","w");
  for (j=0; j <= Niveles_Discriminacion; j++)
      {
       for (i=0; i <= Niveles_Dificultad; i++)
 	   {
	    fprintf (Archivo_Datos, "%Lf %Lf %d\n",(long double) j/Niveles_Discriminacion*2.0 - 1.0, (long double) i/Niveles_Dificultad, Frecuencia_total[i][j]);
	   }
       fprintf (Archivo_Datos, "\n");
      }
  fclose (Archivo_Datos);
}

void Lista_de_Preguntas_Beamer(FILE * Archivo_Latex, GtkWidget *PB, long double base, long double limite)
{
   int i, actual;
   char ejercicio_actual [CODIGO_EJERCICIO_SIZE] = "00000";
   char PG_command [9000];
   PGresult *res, *res_tema;
   char hilera_antes [9000], hilera_despues [9000];
   char tema_actual[CODIGO_TEMA_SIZE + 1]="          ";
   gchar * materia;
   char tema_descripcion[201];
   char opciones_frame[200];

   int N_preguntas_ejercicio, N_validas;

   materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);

   actual      = 0;

   while (actual < N_preguntas)
         {
	  if (strcmp(tema_actual,preguntas[actual].tema) != 0)
	     {
	      strcpy (tema_actual,preguntas[actual].tema);
              sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'",
		       materia, tema_actual, CODIGO_VACIO);
              res_tema = PQEXEC(DATABASE, PG_command);
	      strcpy (tema_descripcion, PQgetvalue (res_tema, 0, 0));
	      sprintf (hilera_antes, "\\section{%s}", tema_descripcion);
              hilera_LATEX (hilera_antes, hilera_despues);
              fprintf (Archivo_Latex, "%s\n", hilera_despues);
              PQclear(res_tema);
              fprintf (Archivo_Latex, "\\frame{\\tableofcontents[currentsection]}\n");
	     }

          strcpy (ejercicio_actual, preguntas[actual].ejercicio);

          N_preguntas_ejercicio = N_validas = 0;
          for (i = actual; (i < N_preguntas) && (strcmp(ejercicio_actual, preguntas[i].ejercicio) == 0); i++)
	      {
               if (!preguntas[actual].excluir) N_validas++;
               N_preguntas_ejercicio++;
	      }

          sprintf (PG_command,"SELECT usa_header, texto_header from bd_texto_ejercicios, bd_ejercicios where codigo_ejercicio = '%s' and texto_ejercicio = consecutivo_texto",
                       ejercicio_actual);
          res = PQEXEC(DATABASE, PG_command);

          if (*PQgetvalue (res, 0, 0) == 't') /* usa_header */
	     {
              if (N_validas > 1)
		 {
		  prepara_opciones (opciones_frame, actual, -1);
  	          if (N_validas == 2)
		     {
		      fprintf (Archivo_Latex, "\\begin{frame}%s{Preguntas %d y %d}\n", opciones_frame, actual+1, actual+2);
 	              fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
		     }
	          else
		     {
		      fprintf (Archivo_Latex, "\\begin{frame}%s{Preguntas %d a %d}\n", opciones_frame, actual+1, actual+ N_preguntas_ejercicio);
 	              fprintf (Archivo_Latex, "\\textbf{Las preguntas %d a %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);
		     }

                  fprintf (Archivo_Latex, "\\begin{beamercolorbox}[shadow=true, rounded=true]{pregunta}\n");
	          strcpy (hilera_antes, PQgetvalue (res, 0, 1)); /* texto_header */
                  hilera_LATEX (hilera_antes, hilera_despues);
                  fprintf (Archivo_Latex, "%s\n", hilera_despues);
                  fprintf (Archivo_Latex, "\\end{beamercolorbox}\n");

                  fprintf (Archivo_Latex, "\\end{frame}\n");
	         }
	      else
	         {
		   if ((N_validas > 0) && !preguntas[actual].excluir)
		      {
		       prepara_opciones (opciones_frame, actual, -2);
                       fprintf (Archivo_Latex, "{\n");
                       Marca_agua_ajuste(Archivo_Latex, actual);
		       sprintf (hilera_antes, "\\begin{frame}%s{Pregunta %d \\hfill {\\small %s}}", opciones_frame, actual+1, tema_descripcion);
                       hilera_LATEX (hilera_antes, hilera_despues);
                       fprintf (Archivo_Latex, "%s\n", hilera_despues);
		       Imprime_pregunta_Beamer (actual, Archivo_Latex, PQgetvalue(res, 0, 1), tema_descripcion);  /* Se despliega texto_header junto a pregunta */
                       fprintf (Archivo_Latex, "\\end{frame}\n");
                       fprintf (Archivo_Latex, "}\n");
		      }

		  N_preguntas_ejercicio = 0;
		  actual++;
		 }
 	     }
          PQclear(res);
          if (PB) Update_PB(PB, base + ((long double) actual/N_preguntas * limite));

	  for (i=0; i < N_preguntas_ejercicio; i++)
	      {
	       if (!preguntas[actual+i].excluir)
		  {
                   fprintf (Archivo_Latex, "{\n");
                   Marca_agua_ajuste(Archivo_Latex, actual + i);
	           if (N_preguntas_ejercicio > 1)
		      {
		       prepara_opciones (opciones_frame, actual+i, -2);
		       sprintf (hilera_antes, "\\begin{frame}%s{Pregunta %d (%d - %d) \\hfill {\\small %s}}", 
                                opciones_frame, actual+i+1, actual+1, actual+N_preguntas_ejercicio, tema_descripcion);
                       hilera_LATEX (hilera_antes, hilera_despues);
                       fprintf (Archivo_Latex, "%s\n", hilera_despues);
		      }
	           else
		      {
		       prepara_opciones (opciones_frame, actual+i, -2);
		       sprintf (hilera_antes, "\\begin{frame}%s{Pregunta %d \\hfill {\\small %s}}",
                                opciones_frame, actual+i+1, tema_descripcion);
                       hilera_LATEX (hilera_antes, hilera_despues);
                       fprintf (Archivo_Latex, "%s\n", hilera_despues);
		      }

	           Imprime_pregunta_Beamer (actual+i,Archivo_Latex, " ", tema_descripcion);
                   fprintf (Archivo_Latex, "\\end{frame} \%\% CIERRA PREGUNTA\n");
                   fprintf (Archivo_Latex, "}\n");
		  }
	      }

          actual += N_preguntas_ejercicio;
          if (PB) Update_PB(PB, base + ((long double) actual/N_preguntas * limite));
         }

   g_free (materia);
}

void Marca_agua_ajuste(FILE * Archivo_Latex, int i)
{
  if (preguntas[i].revision_especial)
     fprintf (Archivo_Latex, 
              "\\usebackgroundtemplate{\\vbox to \\paperheight{\\vfil\\hbox to \\paperwidth{\\hfil\\includegraphics[width=0.5in]{.imagenes/fix.png}\\hfil}\\vfil}}\n");
}

void prepara_opciones (char * opciones_frame, int i, int k)
{
  opciones_frame[0]='\0';

  if (k == -2)
     {
       /* Opciones de la pregunta */
      if (preguntas[i].encoger && !preguntas[i].verbatim)
         strcpy (opciones_frame,"[shrink]");
      else
         if (preguntas[i].encoger && preguntas[i].verbatim)
            strcpy (opciones_frame,"[shrink, fragile]");
         else
            if (!preguntas[i].encoger && preguntas[i].verbatim)
               strcpy (opciones_frame,"[fragile]");
     }
  else
    if (k == -1)
       {
	 /* Opciones del encabezado */
        if (preguntas[i].header_encoger && !preguntas[i].header_verbatim)
           strcpy (opciones_frame,"[shrink]");
        else
           if (preguntas[i].header_encoger && preguntas[i].header_verbatim)
              strcpy (opciones_frame,"[shrink, fragile]");
           else
              if (!preguntas[i].header_encoger && preguntas[i].header_verbatim)
                 strcpy (opciones_frame,"[fragile]");
       }
    else
       {
	 /* Opciones de las opciones... */

        if (preguntas[i].encoger_opcion[k] && !preguntas[i].verbatim_opcion[k])
           strcpy (opciones_frame,"[shrink]");
        else
           if (preguntas[i].encoger_opcion[k] && preguntas[i].verbatim_opcion[k])
              strcpy (opciones_frame,"[shrink, fragile]");
           else
              if (!preguntas[i].encoger_opcion[k] && preguntas[i].verbatim_opcion[k])
                 strcpy (opciones_frame,"[fragile]");
       }
}

void Imprime_pregunta_Beamer (int i, FILE * Archivo_Latex,char * prefijo, char * tema_descripcion)
{
   long double cota_inferior;
   char hilera_antes [10000], hilera_despues [10000];
   char PG_command[9000];
   PGresult * res, *res_ejercicio;
   long double Por_A,Por_B,Por_C,Por_D,Por_E, Total;
   gchar * materia;
   char opciones_frame[200];
   int j, N_flags;

   materia = gtk_editable_get_chars(GTK_EDITABLE(EN_materia), 0, -1);

   cota_inferior = MINIMA/MAXIMA*100.0;

   /*
   sprintf (PG_command,"SELECT usa_header, texto_header from bd_texto_ejercicios, bd_ejercicios where codigo_ejercicio = '%s' and texto_ejercicio = consecutivo_texto",
                       preguntas[i].ejercicio);
   res_ejercicio = PQEXEC(DATABASE, PG_command);
   */

   sprintf (PG_command,"SELECT texto_pregunta, texto_opcion_A, texto_opcion_B, texto_opcion_C, texto_opcion_D, texto_opcion_E from bd_texto_preguntas where codigo_unico_pregunta = '%s'", preguntas[i].pregunta);
   res = PQEXEC(DATABASE, PG_command);

   fprintf (Archivo_Latex, "\\begin{beamercolorbox}[shadow=true, rounded=true]{pregunta}\n");
   sprintf (hilera_antes,"%s %s", prefijo, PQgetvalue (res, 0, 0));
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);
   fprintf (Archivo_Latex, "\\end{beamercolorbox}\n");


   Por_A = (long double) preguntas[i].acumulado_opciones[0]/N_estudiantes*100.0;
   Por_B = (long double) preguntas[i].acumulado_opciones[1]/N_estudiantes*100.0;
   Por_C = (long double) preguntas[i].acumulado_opciones[2]/N_estudiantes*100.0;
   Por_D = (long double) preguntas[i].acumulado_opciones[3]/N_estudiantes*100.0;
   Por_E = (long double) preguntas[i].acumulado_opciones[4]/N_estudiantes*100.0;

   if (preguntas[i].slide[0])
      {
       fprintf (Archivo_Latex, "\\end{frame}\n");
       fprintf (Archivo_Latex, "}\n");
       prepara_opciones (opciones_frame, i, 0);
       fprintf (Archivo_Latex, "{\n");
       Marca_agua_ajuste(Archivo_Latex, i);
       sprintf (hilera_antes, "\\begin{frame}%s{Pregunta %d - cont. \\hfill {\\small %s}}", opciones_frame, i+1, tema_descripcion);
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n", hilera_despues);
      }
   fprintf (Archivo_Latex, "\\begin{itemize}\n");
   Imprime_Opcion_Beamer (Archivo_Latex, res, Por_A, i, 0);

   if (preguntas[i].slide[1])
      {
       fprintf (Archivo_Latex, "\\end{itemize}\n");
       fprintf (Archivo_Latex, "\\end{frame}\n");
       fprintf (Archivo_Latex, "}\n");
       prepara_opciones (opciones_frame, i, 1);
       fprintf (Archivo_Latex, "{\n");
       Marca_agua_ajuste(Archivo_Latex, i);
       sprintf (hilera_antes, "\\begin{frame}%s{Pregunta %d - cont. \\hfill {\\small %s}}", opciones_frame, i+1, tema_descripcion);
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n", hilera_despues);
       fprintf (Archivo_Latex, "\\begin{itemize}\n");
      }
   Imprime_Opcion_Beamer (Archivo_Latex, res, Por_B, i, 1);

   if (preguntas[i].slide[2])
      {
       fprintf (Archivo_Latex, "\\end{itemize}\n");
       fprintf (Archivo_Latex, "\\end{frame}\n");
       fprintf (Archivo_Latex, "}\n");
       prepara_opciones (opciones_frame, i, 2);
       fprintf (Archivo_Latex, "{\n");
       Marca_agua_ajuste(Archivo_Latex, i);
       sprintf (hilera_antes, "\\begin{frame}%s{Pregunta %d - cont. \\hfill {\\small %s}}", opciones_frame, i+1, tema_descripcion);
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n", hilera_despues);
       fprintf (Archivo_Latex, "\\begin{itemize}\n");
      }
   Imprime_Opcion_Beamer (Archivo_Latex, res, Por_C, i, 2);

   if (preguntas[i].slide[3])
      {
       fprintf (Archivo_Latex, "\\end{itemize}\n");
       fprintf (Archivo_Latex, "\\end{frame}\n");
       fprintf (Archivo_Latex, "}\n");
       prepara_opciones (opciones_frame, i, 3);
       fprintf (Archivo_Latex, "{\n");
       Marca_agua_ajuste(Archivo_Latex, i);
       sprintf (hilera_antes, "\\begin{frame}%s{Pregunta %d - cont. \\hfill {\\small %s}}", opciones_frame, i+1, tema_descripcion);
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n", hilera_despues);
       fprintf (Archivo_Latex, "\\begin{itemize}\n");
      }
   Imprime_Opcion_Beamer (Archivo_Latex, res, Por_D, i, 3);

   if (preguntas[i].slide[4])
      {
       fprintf (Archivo_Latex, "\\end{itemize}\n");
       fprintf (Archivo_Latex, "\\end{frame}\n");
       fprintf (Archivo_Latex, "}\n");
       prepara_opciones (opciones_frame, i, 4);
       fprintf (Archivo_Latex, "{\n");
       Marca_agua_ajuste(Archivo_Latex, i);
       sprintf (hilera_antes, "\\begin{frame}%s{Pregunta %d - cont. \\hfill {\\small %s}}", opciones_frame, i+1, tema_descripcion);
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n", hilera_despues);
       fprintf (Archivo_Latex, "\\begin{itemize}\n");
      }
   Imprime_Opcion_Beamer (Archivo_Latex, res, Por_E, i, 4);

   fprintf (Archivo_Latex, "\\end{itemize}\n");

   PQclear(res);

   N_flags = 0;
   for (j=0; j < N_FLAGS;j++) N_flags += preguntas[i].flags[j];

   if (N_flags && !gtk_toggle_button_get_active(CK_sin_banderas))
      {
       fprintf (Archivo_Latex, "\\end{frame}\n");
       fprintf (Archivo_Latex, "}\n");

       fprintf (Archivo_Latex, "{\n");
       sprintf (hilera_antes, "\\begin{frame}{Análisis de Pregunta %d \\hfill {\\small %s}}", i+1, tema_descripcion);
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n", hilera_despues);

       Analiza_Ajuste   (Archivo_Latex, preguntas[i], 1);

       Analiza_Banderas (Archivo_Latex, preguntas[i], 1, N_flags, i+1, tema_descripcion);
      }

   g_free (materia);
}

void Imprime_Opcion_Beamer (FILE * Archivo_Latex, PGresult *res, long double Porcentaje, int pregunta, int opcion)
{
   char hilera_antes [9000], hilera_despues [9000];

   if (preguntas[pregunta].correcta == ('A' + opcion))
      fprintf (Archivo_Latex, "\\item [\\correcta{%c}]", 'A'+opcion);
   else
      fprintf (Archivo_Latex, "\\item [\\circled{%c}]", 'A'+opcion);

   strcpy (hilera_antes, PQgetvalue (res, 0, 1 + opcion)); /* texto opcion*/
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex,"\n\n{\\color{green} \\rule{%Lfcm}{5pt} {\\footnotesize \\textbf{\\texttt{%6.2Lf \\%%}}}} (\\textbf{%d}) \\hfill ",
                          0.05+MAXIMA*Porcentaje/100.0, Porcentaje, 
                          preguntas[pregunta].acumulado_opciones[opcion]);

   if (preguntas[pregunta].correcta == ('A' + opcion))
      {
       if (preguntas[pregunta].Rpb_opcion[opcion] < 0.0)
          fprintf (Archivo_Latex, "\\fcolorbox{black}{red}{\\color{white} $r_{pb}$ = \\textbf{%7.4Lf}}\n\n", preguntas[pregunta].Rpb_opcion[opcion]);
       else
          if (preguntas[pregunta].Rpb_opcion[opcion] >= 0.3)
             fprintf (Archivo_Latex, "\\fcolorbox{black}{blue}{\\color{white} $r_{pb}$ = \\textbf{%7.4Lf}}\n\n", preguntas[pregunta].Rpb_opcion[opcion]);
          else
             if (preguntas[pregunta].Rpb_opcion[opcion] >= 0.2)
                fprintf (Archivo_Latex, "\\fcolorbox{black}{cyan}{\\color{white} $r_{pb}$ = \\textbf{%7.4Lf}}\n\n", preguntas[pregunta].Rpb_opcion[opcion]);
             else
                if (preguntas[pregunta].acumulado_opciones[opcion] == 0)
                   fprintf (Archivo_Latex, "\\fcolorbox{black}{red}{\\color{white} $r_{pb}$ = \\textbf{%7.4Lf}}\n\n", preguntas[pregunta].Rpb_opcion[opcion]);
                else
                   if (preguntas[pregunta].Rpb_opcion[opcion] == 0.0)
                      fprintf (Archivo_Latex, "\\fcolorbox{black}{orange}{\\color{white} $r_{pb}$ = \\textbf{%7.4Lf}}\n\n", preguntas[pregunta].Rpb_opcion[opcion]);
                   else
                      fprintf (Archivo_Latex, "\\fcolorbox{black}{white}{\\color{black} $r_{pb}$ = \\textbf{%7.4Lf}}\n\n", preguntas[pregunta].Rpb_opcion[opcion]);


       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
       fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{red}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
       fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
      }
   else
      {
       if (preguntas[pregunta].Rpb_opcion[opcion] > 0.3)
          fprintf (Archivo_Latex, "\\fcolorbox{black}{red}{\\color{white} $r_{pb}$ = \\textbf{%7.4Lf}}\n\n", preguntas[pregunta].Rpb_opcion[opcion]);
       else
          if (preguntas[pregunta].Rpb_opcion[opcion] > 0.0)
             fprintf (Archivo_Latex, "\\fcolorbox{black}{orange}{\\color{white} $r_{pb}$ = \\textbf{%7.4Lf}}\n\n", preguntas[pregunta].Rpb_opcion[opcion]);
          else
             if (preguntas[pregunta].Rpb_opcion[opcion] <= -0.3)
                fprintf (Archivo_Latex, "\\fcolorbox{black}{blue}{\\color{white} $r_{pb}$ = \\textbf{%7.4Lf}}\n\n", preguntas[pregunta].Rpb_opcion[opcion]);
             else
                fprintf (Archivo_Latex, "\\fcolorbox{black}{white}{\\color{black} $r_{pb}$ = \\textbf{%7.4Lf}}\n\n", preguntas[pregunta].Rpb_opcion[opcion]);
      }

   fprintf (Archivo_Latex, "\n\n");
}

void Busca_pregunta_mala_Beamer (int N)
{
  int i;
  int Exclusiones_previas [N];
  /* N es N_preguntas */
  int low, high, mid;
  int resultado;
  char mensaje_error[2000];

  gtk_widget_show         (window6);
  Update_PB(PB_revisando_beamer, 0.0);

  for (i=0;i<N;i++) Exclusiones_previas[i] = preguntas[i].excluir;

  low = 0;
  high = N-1;
  while (high != low)
        {
         mid = (high + low)/2;
         for (i=  0; i <    N; i++) preguntas[i].excluir = 1; /* Excluya todas */
	 for (i=low; i <= mid; i++) preguntas[i].excluir = 0; /* Active el rango deseado */
	 for (i=low; i <= mid; i++) preguntas[i].excluir = Exclusiones_previas [i]; /* Excluya las que ya estaban excluidas */
	 resultado = Genera_Beamer_reducido ();
         Update_PB(PB_revisando_beamer, (long double)(N_preguntas - (high - mid))/N_preguntas);

	 if (resultado)
	    { /* Beamer no falló - Error está en otra mitad */
	     low = mid+1;
	    }
	 else
	    { /* Beamer falló - Error está en esta mitad */
	     high = mid;
	    }
        }

  sprintf (mensaje_error, "La pregunta número <b>%2d</b> de este examen tiene\nalgún conflicto con <b><i>Beamer</i></b>. Esta corresponde\na la pregunta <b>%s.%d</b> de la Base de Datos.\n\nRevise detalladamente la sintaxis de\nesta pregunta. Considere marcar ya\nsea la casilla \"<b>verbatim</b>\" o la casilla\n\"<b>Excluir</b>\" en la pregunta <b>%d</b>.",
           low + 1, preguntas[low].ejercicio, preguntas[low].secuencia, low + 1);
  gtk_label_set_markup(LB_error_encontrado_Beamer, mensaje_error);

  for (i=0;i<N;i++) preguntas[i].excluir = Exclusiones_previas[i];

  gtk_widget_hide (window6);
  gtk_widget_show (window7);
}

int Genera_Beamer_reducido ()
{
   int k;
   char codigo[10];
   int resultado_OK=0;
   FILE * Archivo_Latex;
   gchar *size, *font, *color, *estilo, *materia, *fecha;
   int aspecto;
   char Directorio[3000];

   Banderas [0] =Banderas [1] =Banderas [2] =Banderas [3] = Banderas [4] = Banderas [5] = 0;
   k = (int) gtk_spin_button_get_value_as_int (SP_examen);
   sprintf (codigo, "%05d", k);

   materia               = gtk_editable_get_chars(GTK_EDITABLE(EN_materia)                   , 0, -1);
   fecha                 = gtk_editable_get_chars(GTK_EDITABLE(EN_fecha)                     , 0, -1);
   estilo                = gtk_combo_box_get_active_text(CB_estilo);
   color                 = gtk_combo_box_get_active_text(CB_color);
   font                  = gtk_combo_box_get_active_text(CB_font);
   size                  = gtk_combo_box_get_active_text(CB_size);
   aspecto               = gtk_combo_box_get_active     (CB_aspecto);

   Establece_Directorio (Directorio, materia,fecha+6,fecha+3,fecha);

   Archivo_Latex = fopen ("analisis-beamer.tex","w");
   Beamer_Preamble_reducido (Archivo_Latex, aspecto, size, estilo,color, font);
   Beamer_Preguntas         (Archivo_Latex, NULL, 0.0, 0.0);
   Beamer_Cierre            (Archivo_Latex);
   fclose                   (Archivo_Latex);

   resultado_OK = latex_2_pdf (&parametros, Directorio, parametros.ruta_latex, "analisis-beamer", 0, NULL, 0.0, 0.0, NULL, NULL);
#if 0

   system ("rm EX4010-h.pdf");
   system ("rm EX4010-n.pdf");
   system ("rm EX4010-t.pdf");
   system ("rm EX4010p.pdf");
   system ("rm EX4010c.pdf");
   system ("rm EX4010.dat");
   system ("rm analisis-beamer.*");
   Update_PB(PB_beamer, 0.99);

   g_free(estilo);
   g_free(color);
   g_free(font);
   g_free(size);
   g_free(materia);
   g_free(fecha);

   gtk_widget_hide (window3);
   gtk_widget_set_sensitive(window1, 1);
#endif

   return (resultado_OK);
}

void Beamer_Preamble_reducido (FILE * Archivo_Latex, int aspecto, gchar * size, gchar * estilo, gchar * color, gchar * font)
{
   char hilera_antes [9000], hilera_despues [9000];

   fprintf (Archivo_Latex, "\\documentclass[aspectratio=%s,%s]{beamer}\n", Beamer_aspectratio[aspecto], size);
   fprintf (Archivo_Latex, "\\def \\BEAMER {}\n");

   EX_latex_packages (Archivo_Latex); /* En EX_utilities.c */
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);
   fprintf (Archivo_Latex, "\\newcommand*\\circled[1]{\\tikz[baseline=(char.base)]{\\node[shape=circle,draw,inner sep=2pt] (char) {#1};}}\n");
   fprintf (Archivo_Latex, "\\newcommand*\\correcta[1]{\\tikz[baseline=(char.base)]{\\node[shape=circle,fill=gray,draw,inner sep=2pt] (char) {#1};}}\n");

   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);

   fprintf (Archivo_Latex, "\\usetheme{%s}\n", estilo);
   fprintf (Archivo_Latex, "\\usecolortheme{%s}\n", color);
   fprintf (Archivo_Latex, "\\usefonttheme{%s}\n", font);

   fprintf (Archivo_Latex, "\\useoutertheme{shadow}\n");
   fprintf (Archivo_Latex, "\\setbeamertemplate{navigation symbols}{}\n");
   fprintf (Archivo_Latex, "\\setbeamertemplate{caption}[numbered]\n");
   fprintf (Archivo_Latex, "\\captionsetup{labelsep = colon,figureposition = bottom}\n");

   fprintf (Archivo_Latex, "\\setbeamercolor{caption name}{fg=black}\n");
   fprintf (Archivo_Latex, "\\setbeamertemplate{headline}{}\n");

   fprintf (Archivo_Latex, "\\definecolor{DoradoPalido}{RGB}{238,232,170}\n");
   fprintf (Archivo_Latex, "\\setbeamercolor{pregunta} {bg=DoradoPalido, fg=black}\n");
   fprintf (Archivo_Latex, "\\setbeamertemplate{blocks}[rounded][shadow=true]\n");
   fprintf (Archivo_Latex, "\\SetWatermarkText{}\n");

   fprintf (Archivo_Latex, "\\title{Test}\n");

   fprintf (Archivo_Latex, "\\begin{document}\n\n");
}

void Update_PB (GtkWidget *PB, long double R)
{
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB), R);
  while (gtk_events_pending ()) gtk_main_iteration ();
}

/***********************/
/*  Interface Hookups  */
/***********************/
void on_WN_ex4010_destroy (GtkWidget *widget, gpointer user_data) 
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
  Imprime_Reporte ();
}

void on_BN_save_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Ajustes ();
}

void on_BN_slides_clicked(GtkWidget *widget, gpointer user_data)
{
  Genera_Beamer ();
}

void on_EN_examen_activate(GtkWidget *widget, gpointer user_data)
{
  Cambio_Examen ();
}

void on_SC_preguntas_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_Pregunta ();
}

void on_CB_ajuste_changed(GtkWidget *widget, gpointer user_data)
{
  Cambio_Ajuste (widget, user_data);
}

void on_CK_no_actualiza_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_no_actualizar(widget, user_data);
}

void on_CK_excluir_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_excluir(widget, user_data);
}

void on_CK_encoger_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_encoger(widget, user_data);
}

void on_CK_verbatim_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_verbatim(widget, user_data);
}

void on_CK_header_encoger_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_header_encoger(widget, user_data);
}

void on_CK_header_verbatim_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_header_verbatim(widget, user_data);
}

void on_CK_slide_A_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_slide (0);
}

void on_CK_slide_B_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_slide (1);
}

void on_CK_slide_C_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_slide (2);
}

void on_CK_slide_D_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_slide (3);
}

void on_CK_slide_E_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_slide (4);
}

void on_CK_encoger_A_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_encoger_opcion (0);
}
void on_CK_encoger_B_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_encoger_opcion (1);
}
void on_CK_encoger_C_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_encoger_opcion (2);
}
void on_CK_encoger_D_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_encoger_opcion (3);
}
void on_CK_encoger_E_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_encoger_opcion (4);
}

void on_CK_verbatim_A_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_verbatim_opcion (0);
}
void on_CK_verbatim_B_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_verbatim_opcion (1);
}
void on_CK_verbatim_C_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_verbatim_opcion (2);
}
void on_CK_verbatim_D_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_verbatim_opcion (3);
}
void on_CK_verbatim_E_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_verbatim_opcion (4);
}

void on_TG_A_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_A(widget, user_data);
}

void on_TG_B_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_B(widget, user_data);
}

void on_TG_C_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_C(widget, user_data);
}

void on_TG_D_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_D(widget, user_data);
}

void on_TG_E_toggled(GtkWidget *widget, gpointer user_data)
{
  Cambio_E(widget, user_data);
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (GTK_WIDGET(SP_examen));

  if (N_estudiantes)
     gtk_widget_grab_focus   (BN_print);
  else
     gtk_widget_grab_focus   (BN_undo);
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

void on_BN_cancela_revision_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window5);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (GTK_WIDGET(SP_examen));

  if (N_estudiantes)
     gtk_widget_grab_focus   (BN_print);
  else
     gtk_widget_grab_focus   (BN_undo);
}

void on_BN_confirma_revision_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide            (window5);
  Busca_pregunta_mala_Beamer (N_preguntas);
}

void on_BN_error_encontrado_Beamer_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window7);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (GTK_WIDGET(SP_examen));

  if (N_estudiantes)
     gtk_widget_grab_focus   (BN_print);
  else
     gtk_widget_grab_focus   (BN_undo);
}
