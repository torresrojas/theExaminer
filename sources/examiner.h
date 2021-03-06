/*********************************************/
/*  THE EXAMINER:                            */
/*                                           */
/*    Data Structures and general parameters */
/*    Author: Francisco J. Torres-Rojas      */        
/*    September, 22nd 2012                   */
/*                                           */
/*********************************************/

/*****************************/
/* Tamaños de algunos campos */
/*****************************/
#define CODIGO_MATERIA_SIZE      10
#define CODIGO_TEMA_SIZE         10
#define CODIGO_SUBTEMA_SIZE      10
#define DESCRIPCION_MATERIA_SIZE 200
#define CODIGO_VACIO             "          "

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER
{
  /* Indica si alguna vez se ha grabado parametrización */
  int         configurado;

  /* Tipo de interacción y decoración */
  int         programa;
  int         decoracion;
  int         despliega_reportes;
  int         report_update;
  int         timeout;
    
  /* Parámetros para asignación de boletos por pregunta */
  int         boletos_base;
  int         boletos_ejercicio;
  int         min_boletos;
  int         max_boletos;
  long double min_p, max_p;
  int         boletos_p_1, boletos_p_2;
  long double min_Rpb, max_Rpb;
  int         boletos_Rpb_1, boletos_Rpb_2;
  int         min_dias, max_dias;
  int         boletos_dias_1, boletos_dias_2;
  int         min_usos, max_usos;
  int         boletos_usos_1, boletos_usos_2;
  int         b_dificultad, b_discriminacion, b_novedad, b_usos;
  long double m_dificultad, m_discriminacion, m_novedad, m_usos;

  /* Comando para convertir LATEX en PDF */
  char        latex_2_pdf   [600];

  /* Comando para desplegar un PDF */
  char        despliega_pdf [600];

  /* Comando para convertir EPS a PDF */
  char        epstopdf [600];

  /* Comando asociado a gnuplot */
  char        gnuplot [600];

  /* Comando para abrir consola */
  char        xterm [600];

  /* Ruta del directorio donde se almacenarán los exámenes generados */
  char        ruta_examenes [1000];

  /* Ruta del directorio donde se almacenarán las figuras usadas en las preguntas del examen */
  int         usa_figuras;
  char        ruta_figuras [1000];

  /* Ruta del directorio donde se almacenarán los reportes generados */
  char        ruta_reportes [1000];

  /* Ruta del directorio donde se almacenarán los fuentes de latex */
  char        ruta_latex [1000];

  /* Ruta del directorio donde se almacenarán los fuentes de gnuplot */
  char        ruta_gnuplot [1000];

  /* Ruta del directorio donde se almacenarán los respaldos*/
  char        ruta_backup [1000];

  /* Nombres por defecto de Institucion, Escuela y Programa */
  char        Institucion [200];
  char        Escuela     [200];
  char        Programa    [200];

  /* Instrucciones y Juramento */
  char        Instrucciones [4097];
  char        Juramento     [4097];

  /* Paquetes adicionales de Latex */
  char        Paquetes      [4097];

  /* Parametros de Presentacion Beamer */
  int         Beamer_Estilo;
  int         Beamer_Color;
  int         Beamer_Font;
  int         Beamer_Size;
  int         Beamer_Aspecto;

  /* Ultimo respaldo registrado */
  char        fecha_respaldo [11];
};

/**********/
/* COLORS */
/**********/
#define MAIN_WINDOW             "midnight blue"
#define MAIN_AREA               "light blue"
#define SECONDARY_AREA          "light cyan"
#define THIRD_AREA              "white"
#define DARK_AREA               "slate gray"
#define RARE_AREA               "orchid"
#define SPECIAL_AREA_1          "dark blue"
#define SPECIAL_AREA_2          "khaki"
#define SPECIAL_WINDOW          "dark green"
#define IMPORTANT_WINDOW        "yellow"
#define IMPORTANT_FR            "black"
#define FINISHED_WORK_WINDOW    "dark green"
#define FINISHED_WORK_FR        "green"
#define STANDARD_FRAME          "silver"
#define STANDARD_ENTRY          "orange"
#define BUTTON_PRELIGHT         "light goldenrod"
#define BUTTON_ACTIVE           "light slate gray"
#define SPECIAL_BUTTON_NORMAL   "light sky blue"
#define SPECIAL_BUTTON_PRELIGHT "orange"
#define SPECIAL_BUTTON_ACTIVE   "saddle brown"
#define QUESTION                "blue"
#define FR_QUESTION             "dark blue"
#define CORRECT_OPTION          "lime"
#define CORRECT_OPTION_FR       "orange"
#define DISTRACTOR              "dim gray"
#define DISTRACTOR_FR           "black"
#define BOLETOS_FR              "white"
#define COLOR_BOLETOS_BASE      "gray"
#define COLOR_BOLETOS_DIF       "red"
#define COLOR_BOLETOS_DISC      "dark green"
#define COLOR_BOLETOS_DIAS      "dark magenta"
#define COLOR_BOLETOS_EST       "dark blue"
#define COLOR_BOLETOS_EJER      "dark red"
#define PROCESSING_WINDOW       "dim gray"
#define PROCESSING_FR           "dark goldenrod"
#define PROCESSING_PB           "red"
#define FROZEN_COLOR            "blue"
#define ANSWER_BOX_COLOR        "black"

/*******************************************************/
/* Prototipos de funciones definidas en EX_utilities.c */
/* y en EX_latex.c                                     */
/*******************************************************/
PGresult *PQEXEC               (PGconn *conn, const char *command);
void carga_parametros_EXAMINER (struct PARAMETROS_EXAMINER * p, PGconn	 *conn);
int days_between(int day1, int month1, int year1, int day2, int month2, int year2);
void EX_latex_packages         (FILE * Archivo_Latex);
void hilera_LATEX              (unsigned char * antes, unsigned char * despues);
void hilera_GNUPLOT            (unsigned char * antes, unsigned char * despues);
void hilera_POSTGRES           (unsigned char * antes, unsigned char * despues);
int  existe_archivo            (const char *fname);
int  latex_2_pdf               (struct PARAMETROS_EXAMINER * p, char * directorio_PDF, char * directorio_LATEX, char * reporte, int desplegar, GtkWidget *PB, long double base, long double max, char * return_ruta_reporte, char * return_ruta_latex);
void Main_decoration           (GtkImage *IM_bg, char * color, int decoracion);
PGconn * EX_connect_data_base  (void);
void catNap                    (clock_t sec);
guint Connect_Latex_Window     (GtkBuilder * builder, GError ** error, struct PARAMETROS_EXAMINER parameters);
void Display_Latex_Window(char * window_title, char * Root, GtkTextBuffer * new_buffer, GtkWidget *prev_window);
