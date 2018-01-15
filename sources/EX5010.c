/*
Compilar con

cc -o EX5010 EX5010.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX5010:                                */
/*                                         */
/*    Lista Ejercicios Registrados         */
/*    The Examiner 0.0                     */
/*    11 de Julio 2011                     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    24 de Enero del 2012                 */
/*******************************************/
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
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

GtkWidget *EB_datos = NULL;
GtkWidget *FR_datos = NULL;

GtkComboBox *CB_materia           = NULL;
GtkComboBox *CB_tema              = NULL;
GtkComboBox *CB_subtema           = NULL;
GtkToggleButton *CK_detalles      = NULL;
GtkWidget *FR_botones             = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_print               = NULL;
GtkWidget *BN_terminar            = NULL;

GtkWidget *EB_reporte             = NULL;
GtkWidget *FR_reporte             = NULL;
GtkWidget *PB_reporte             = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Asigna_Banderas        (int i, PGresult * res, int * flags);
void Cajita_con_bandera     (FILE * Archivo_Latex, char * mensaje, int color);
void Cambio_Materia         (GtkWidget *widget, gpointer user_data);
void Cambio_Subtema         (GtkWidget *widget, gpointer user_data);
void Cambio_Tema            (GtkWidget *widget, gpointer user_data);
void Connect_Widgets();
void Extrae_codigo          (char * hilera, char * codigo);
void Fin_de_Programa        (GtkWidget *widget, gpointer user_data);
void Fin_Ventana            (GtkWidget *widget, gpointer user_data);
void Imprime_Banderas       (FILE * Archivo_Latex, int * flags, int i, PGresult * res);
void Imprime_listado        (GtkWidget *widget, gpointer user_data);
void Imprime_Opcion         (FILE * Archivo_Latex, PGresult *res, int Detalles, char * alfileres, long double Total, long double Porcentaje, int Frecuencia, long double Rpb, char correcta, int pregunta, int opcion);
void Imprime_pregunta       (int i, PGresult * res, FILE * Archivo_Latex,char * prefijo);
void Init_Fields            ();
void Interface_Coloring ();
void on_BN_print_clicked    (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked     (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked (GtkWidget *widget, gpointer user_data);
void on_CB_materia_changed  (GtkWidget *widget, gpointer user_data);
void on_CB_subtema_changed  (GtkWidget *widget, gpointer user_data);
void on_CB_tema_changed     (GtkWidget *widget, gpointer user_data);
void on_CK_detalles_toggled (GtkWidget *widget, gpointer user_data);
void on_WN_ex5010_destroy   (GtkWidget *widget, gpointer user_data) ;
void Resumen_de_Banderas    (FILE * Archivo_Latex);
void Toggle_detalles        (GtkWidget *widget, gpointer user_data);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define CODIGO_SIZE              7

#define N_FLAGS                  15

#define TODAS_MALAS                  0
#define TODAS_BUENAS                 1
#define PORCENTAJE_MENOR_30          2
#define RPB_BUENO                    3
#define RPB_NEGATIVO                 4
#define ALFA_SUBE_MUCHO              5
#define ALFA_SUBE                    6
#define RPB_DISTRACTOR_MUY_POSITIVO  7
#define RPB_DISTRACTOR_POSITIVO      8
#define RPB_DISTRACTOR_MUY_NEGATIVO  9
#define SOLO_2_DISTRACTORES          10
#define MENOS_DE_5_DISTRACTORES      11
#define USA_5_DISTRACTORES           12
#define RPB_Y_DIFICULTAD_OK          13
#define RPB_DIFI_DISTRA_OK           14

#define VERDE    0
#define AZUL     1
#define CYAN     2
#define AMARILLO 3
#define ROJO     4
int Banderas [5];
char * colores  [] = {"green", "blue", "cyan", "yellow", "red"};
char * banderas [] = {"FL-verde.jpg", "FL-azul.jpg", "FL-cyan.jpg", "FL-amarilla.jpg", "FL-roja.jpg"};

int Detalles   = 0;
int N_materias = 0;
int N_temas    = 0;
int N_subtemas = 0;

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX5010.glade", &error))
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

	  /* Despliega ventana */
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
  int i;

  window1              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex5010"));
  window2 = GTK_WIDGET (gtk_builder_get_object (builder, "WN_reporte"));

  EB_datos = GTK_WIDGET (gtk_builder_get_object (builder, "EB_datos"));
  FR_datos = GTK_WIDGET (gtk_builder_get_object (builder, "FR_datos"));

  CB_materia             = (GtkComboBox *)     GTK_WIDGET (gtk_builder_get_object (builder, "CB_materia"));
  CB_tema                = (GtkComboBox *)     GTK_WIDGET (gtk_builder_get_object (builder, "CB_tema"));
  CB_subtema             = (GtkComboBox *)     GTK_WIDGET (gtk_builder_get_object (builder, "CB_subtema"));
  CK_detalles            = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "CK_detalles"));

  FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  BN_print               = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));
  BN_terminar            = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));

  EB_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "EB_reporte"));
  FR_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reporte"));
  PB_reporte = GTK_WIDGET (gtk_builder_get_object (builder, "PB_reporte"));
  
}

void Interface_Coloring ()
{
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_datos, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_datos, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones, GTK_STATE_NORMAL, &color);

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_ACTIVE, &color);

  gdk_color_parse (PROCESSING_WINDOW, &color);
  gtk_widget_modify_bg(EB_reporte,  GTK_STATE_NORMAL,   &color); 

  gdk_color_parse (PROCESSING_FR, &color);
  gtk_widget_modify_bg(FR_reporte,            GTK_STATE_NORMAL, &color);

  gdk_color_parse (PROCESSING_PB, &color);
  gtk_widget_modify_bg(GTK_WIDGET(PB_reporte),  GTK_STATE_PRELIGHT, &color);
}

void Init_Fields()
{
   int i;
   char hilera[300];

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
   /*
   gtk_combo_box_append_text(CB_materia, CODIGO_VACIO);
   */
   gtk_combo_box_append_text(CB_materia, "*** Todas las Materias ***");
   res = PQEXEC(DATABASE,"SELECT codigo_materia, descripcion_materia from bd_materias where codigo_tema = '          ' and codigo_subtema = '          ' order by codigo_materia"); 
   N_materias = PQntuples(res)+1;
   for (i=1; i < N_materias; i++)
       {
	sprintf (hilera,"%s %s",PQgetvalue (res, i-1, 0), PQgetvalue (res, i-1, 1));
        gtk_combo_box_append_text(CB_materia, hilera);
       }
   PQclear(res);
   gtk_combo_box_set_active (CB_materia, 0);

   gtk_toggle_button_set_active(CK_detalles, TRUE);
 
   gtk_widget_set_sensitive(BN_print, 1);

   gtk_widget_grab_focus (GTK_WIDGET(CB_materia));
}

void Cambio_Materia(GtkWidget *widget, gpointer user_data)
{
  int i, k;
  gchar *materia;
  char PG_command  [1000];
  PGresult *res;
  char hilera [300];
  char Codigo_materia [CODIGO_MATERIA_SIZE + 1];

  k = gtk_combo_box_get_active(CB_materia);
  materia = gtk_combo_box_get_active_text(CB_materia);
  Extrae_codigo (materia, Codigo_materia);

  gtk_combo_box_set_active (CB_tema, -1);
  for (i=0; i<N_temas; i++)
      {
       gtk_combo_box_remove_text (CB_tema, 0);
      }

  if (k != 0)
     {
      gtk_combo_box_append_text(CB_tema, "*** Todos los Temas ***");
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
      gtk_widget_grab_focus    (GTK_WIDGET(CB_tema));
     }
  else
    { /* No se escoge materia */
      N_temas = 1;
      gtk_combo_box_append_text(CB_tema, "*** Todos los Temas ***");
      gtk_combo_box_set_active (CB_tema   , 0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_tema),    0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_subtema), 0);
     }

  g_free (materia);
}

void Cambio_Tema(GtkWidget *widget, gpointer user_data)
{
  int i, k;
  gchar *materia, *tema = NULL;
  char PG_command  [1000];
  PGresult *res;
  char hilera [300];
  char Codigo_materia [CODIGO_MATERIA_SIZE + 1];
  char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];

  k = gtk_combo_box_get_active(CB_tema);
  materia = gtk_combo_box_get_active_text(CB_materia);
  tema    = gtk_combo_box_get_active_text(CB_tema);

  Extrae_codigo (materia, Codigo_materia);
  Extrae_codigo (tema,    Codigo_tema);

  gtk_combo_box_set_active(CB_subtema, -1);
  for (i=0; i<N_subtemas; i++)
      {
       gtk_combo_box_remove_text (CB_subtema, 0);
      }

  if (k != 0)
     {
      gtk_combo_box_append_text(CB_subtema, "*** Todos los Subtemas ***");
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
      gtk_widget_grab_focus    (GTK_WIDGET(CB_subtema));
     }
  else
     { /* No se escoge Tema */
      N_subtemas = 1;
      gtk_combo_box_append_text(CB_subtema, "*** Todos los Subtemas ***");
      gtk_combo_box_set_active (CB_subtema   , 0);
      gtk_widget_set_sensitive (GTK_WIDGET(CB_subtema), 0);
     }

  g_free (tema);
  g_free (materia);
}

void Cambio_Subtema(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (GTK_WIDGET(CB_materia));
}

void Extrae_codigo (char * hilera, char * codigo)
{
  int i;

  i = 0;

  while (hilera[i] != ' ') codigo [i] = hilera[i++];

  codigo[i] = '\0';
}


void Toggle_detalles(GtkWidget *widget, gpointer user_data)
{
  Detalles = 1 - Detalles;
}

/*******************************************************/
/* Query monstruoso usado para imprimir listado:       */
/*

BD_EJERCICIOS:
0   codigo_ejercicio char(6) PRIMARY KEY,
1   materia          char(5),
2   tema             char(5),
3   subtema          char(5),
4   liga_year            int,
5   liga_month           int,
6   liga_day             int,
7   automatico       boolean,
8   comentario       char varying(100),
9   texto_ejercicio  char(6) REFERENCES BD_Texto_Ejercicios ON DELETE RESTRICT,
10  objetivo1        char(1),
11  objetivo2        char(1),
12  objetivo3        char(1),
13  objetivo4        char(1),
14  objetivo5        char(1),
15  objetivo6        char(1),
16  objetivo7        char(1),
17  objetivo8        char(1),
18  bloom                int,

BD_TEXTO_EJERCICIOS:
19  consecutivo_texto char(6) PRIMARY KEY,
20  descripcion_ejercicio char varying(200),
21  contador_referencias int,
22  autor             char(10) REFERENCES bd_autores ON DELETE RESTRICT,
23  usa_header        boolean,
24  texto_header      char varying(4096)

BD_TEXTO_PREGUNTAS:
25  codigo_consecutivo_ejercicio char(6) REFERENCES BD_Texto_Ejercicios ON DELETE RESTRICT,
26  secuencia_pregunta           int,
27  codigo_unico_pregunta        char(6) UNIQUE NOT NULL,
28  tipo             char(1),
29  creacion_year    int,
30  creacion_month   int,
31  creacion_day     int,
32  dificultad_estimada      float,
33  texto_pregunta   char varying (4096),
34  texto_opcion_A   char varying (4096),
35  texto_opcion_B   char varying (4096),
36  texto_opcion_C   char varying (4096),
37  texto_opcion_D   char varying (4096),
38  texto_opcion_E   char varying (4096),
39  respuesta        char,
40  alfileres_opciones char(5),

BD_ESTADISTICAS_PREGUNTAS:
41  pregunta        char(6) UNIQUE NOT NULL,
42  ultimo_uso_year          int,
43  ultimo_uso_month         int,
44  ultimo_uso_day           int,
45  ultimo_examen            char(5),
46  N_examenes               int,
47  N_estudiantes            int,
48  media_examenes           float,
49  suma_cuadrados_examenes  float,
50  varianza_examenes        float,
51  dificultad               float,
52  varianza_dificultad      float,
53  point_biserial           float,
54  media_correctos          float,
55  media_incorrectos        float,
56  N_correctos              int,
57  N_incorrectos            int,
58  cronbach_alpha_media_con float,
59  cronbach_alpha_media_sin float,
60  ponderacion_cronbach   int,
61  frecuencia       int,
62  frecuencia_1     int,
63  frecuencia_2     int,
64  frecuencia_3     int,
65  frecuencia_4     int,
66  frecuencia_5     int,
67  media_ex_1_1        float,
68  media_ex_1_0        float,
69  media_ex_2_1        float,
70  media_ex_2_0        float,
71  media_ex_3_1        float,
72  media_ex_3_0        float,
73  media_ex_4_1        float,
74  media_ex_4_0        float,
75  media_ex_5_1        float,
76  media_ex_5_0        float,
77  point_biserial_1          float,
78  point_biserial_2          float,
79  point_biserial_3          float,
80  point_biserial_4          float,
81  point_biserial_5          float,

BD_PERSONAS:
82  codigo_persona    char(10),
83  nombre            char varying(100),
84  afiliacion        char varying(100)
*/
/*******************************************************/

void Imprime_listado (GtkWidget *widget, gpointer user_data)
{
   int k_materia, k_tema, k_subtema;
   FILE * Archivo_Latex;
   gchar *materia, *tema, *subtema, *materia_descripcion=" ", *tema_descripcion=" ", *subtema_descripcion=" ";

   char ejercicio_actual [CODIGO_SIZE] = "000000";
   char PG_command [3000];
   int i, actual, N_preguntas, N_preguntas_ejercicio;
   char hilera_antes [4000], hilera_despues [4000];
   char alfileres [6];
   char Correcta;
   PGresult *res, *res_materia, *res_tema, *res_subtema, *res_preguntas;
   char Codigo_materia [CODIGO_MATERIA_SIZE + 1];
   char Codigo_tema    [CODIGO_TEMA_SIZE    + 1];
   char Codigo_subtema [CODIGO_SUBTEMA_SIZE + 1];


   gtk_widget_set_sensitive(window1, 0);
   gtk_widget_show         (window2);
   
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.0);
   while (gtk_events_pending ()) gtk_main_iteration ();
   
   Banderas [0] =Banderas [1] =Banderas [2] =Banderas [3] = Banderas [4] = 0;

   k_materia = gtk_combo_box_get_active(CB_materia);
   k_tema    = gtk_combo_box_get_active(CB_tema);
   k_subtema = gtk_combo_box_get_active(CB_subtema);

   materia = gtk_combo_box_get_active_text(CB_materia);
   tema    = gtk_combo_box_get_active_text(CB_tema);
   subtema = gtk_combo_box_get_active_text(CB_subtema);

   Extrae_codigo (materia, Codigo_materia);
   Extrae_codigo (tema,    Codigo_tema);
   Extrae_codigo (subtema, Codigo_subtema);

   Archivo_Latex = fopen ("EX5010.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\graphicspath{{%s/}}\n\n", parametros.ruta_figuras);
   fprintf (Archivo_Latex, "\\SetWatermarkText{Confidencial}\n");
   fprintf (Archivo_Latex, "\\SetWatermarkLightness{0.9}\n");

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.03);
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\begin{document}\n");

   if (!k_materia)
      {
       sprintf (PG_command,"SELECT * from bd_ejercicios, bd_texto_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas, bd_personas where texto_ejercicio = consecutivo_texto and consecutivo_texto = codigo_consecutivo_ejercicio and pregunta = codigo_unico_pregunta and autor = codigo_persona order by materia, tema, subtema, codigo_ejercicio, secuencia_pregunta");
       fprintf (Archivo_Latex, "\\title{Base de Datos de Preguntas}\n");
      }
   else
      {
       sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '          ' and codigo_subtema = '          '", Codigo_materia);
       res_materia = PQEXEC(DATABASE, PG_command);

       if (!k_tema)
	  {
	   sprintf (PG_command,"SELECT * from bd_ejercicios, bd_texto_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas, bd_personas where texto_ejercicio = consecutivo_texto and consecutivo_texto = codigo_consecutivo_ejercicio and materia = '%s' and pregunta = codigo_unico_pregunta and autor = codigo_persona order by materia, tema, subtema, codigo_ejercicio, secuencia_pregunta",
                    Codigo_materia);
           sprintf (hilera_antes, "\\title{Base de Datos de Preguntas\\\\(%s)}\n", PQgetvalue (res_materia, 0, 0));
           hilera_LATEX (hilera_antes, hilera_despues);
           fprintf (Archivo_Latex, "%s", hilera_despues);
	  }
       else
	  {
	   sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '          '", Codigo_materia, Codigo_tema);
           res_tema = PQEXEC(DATABASE, PG_command);

	   if (!k_subtema)
	      {
	       sprintf (PG_command,"SELECT * from bd_ejercicios, bd_texto_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas, bd_personas where texto_ejercicio = consecutivo_texto and consecutivo_texto = codigo_consecutivo_ejercicio and materia = '%s' and tema = '%s' and pregunta = codigo_unico_pregunta and autor = codigo_persona order by materia, tema, subtema, codigo_ejercicio, secuencia_pregunta", Codigo_materia, Codigo_tema);
               sprintf (hilera_antes, "\\title{Base de Datos de Preguntas\\\\(%s - %s)}\n", PQgetvalue (res_materia, 0, 0), PQgetvalue (res_tema, 0, 0));
               hilera_LATEX (hilera_antes, hilera_despues);
               fprintf (Archivo_Latex, "%s", hilera_despues);
	      }
	   else
	      {
	       sprintf (PG_command,"SELECT descripcion_materia from bd_materias where codigo_materia = '%s' and codigo_tema = '%s' and codigo_subtema = '%s'", 
                       Codigo_materia, Codigo_tema, Codigo_subtema);
               res_subtema = PQEXEC(DATABASE, PG_command);

	       sprintf (PG_command,"SELECT * from bd_ejercicios, bd_texto_ejercicios, bd_texto_preguntas, bd_estadisticas_preguntas, bd_personas where texto_ejercicio = consecutivo_texto and consecutivo_texto = codigo_consecutivo_ejercicio and materia = '%s' and tema = '%s' and subtema = '%s' and pregunta = codigo_unico_pregunta and autor = codigo_persona order by materia, tema, subtema, codigo_ejercicio, secuencia_pregunta", Codigo_materia, Codigo_tema, Codigo_subtema);
               sprintf (hilera_antes, "\\title{Base de Datos de Preguntas\\\\(%s - %s - %s)}\n", 
               PQgetvalue (res_materia, 0, 0),  PQgetvalue (res_tema, 0, 0), PQgetvalue (res_subtema, 0, 0));
               hilera_LATEX (hilera_antes, hilera_despues);
               fprintf (Archivo_Latex, "%s", hilera_despues);
               PQclear(res_subtema);
	      }
           PQclear(res_tema);
	  }
       PQclear(res_materia);
      }
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.08);
   while (gtk_events_pending ()) gtk_main_iteration ();

   fprintf (Archivo_Latex, "\\maketitle\n\n");
   fprintf (Archivo_Latex, "\n\n");


   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.09);
   while (gtk_events_pending ()) gtk_main_iteration ();

   actual      = 0;
   res         = PQEXEC(DATABASE, PG_command);
   N_preguntas = PQntuples(res);

   if (N_preguntas)
      {
       strcpy (ejercicio_actual, PQgetvalue (res, actual, 0));
       N_preguntas_ejercicio = 0;
       i = actual;
       for (i = actual; (i < N_preguntas) && (strcmp(ejercicio_actual, PQgetvalue (res, i, 0)) == 0); i++) N_preguntas_ejercicio++;

       if (N_preguntas_ejercicio > 1)
          fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

       if (*PQgetvalue (res, actual, 23) == 't') /* 23 => usa_header */
	  {
           if (N_preguntas_ejercicio > 1)
	      {
	       if (N_preguntas_ejercicio == 2)
	          fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
	       else
	          fprintf (Archivo_Latex, "\\textbf{Las preguntas %d a %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	       strcpy (hilera_antes, PQgetvalue (res, actual, 24)); /* 24 => texto_header */
               hilera_LATEX (hilera_antes, hilera_despues);
               fprintf (Archivo_Latex, "%s\n", hilera_despues);
               fprintf (Archivo_Latex, "\\rule{8.9cm}{1pt}\n");
               fprintf (Archivo_Latex, "\\begin{questions}\n");
	      }
	   else
	      {
               fprintf (Archivo_Latex, "\\begin{questions}\n");
	       Imprime_pregunta (actual, res, Archivo_Latex, PQgetvalue (res, actual, 24));  /* 24 => texto_header */
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
	    Imprime_pregunta (actual+i, res, Archivo_Latex," ");
	   }

       if (N_preguntas_ejercicio > 1)
          fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

       actual += N_preguntas_ejercicio;
       
       gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.09 + (0.46 * ((long double) actual / (long double) N_preguntas)));
       while (gtk_events_pending ()) gtk_main_iteration ();

       while (actual < N_preguntas)
             {
              strcpy (ejercicio_actual, PQgetvalue (res, actual, 0));
              N_preguntas_ejercicio = 0;
              i = actual;
              for (i = actual; (i < N_preguntas) && (strcmp(ejercicio_actual, PQgetvalue (res, i, 0)) == 0); i++) N_preguntas_ejercicio++;

              if (N_preguntas_ejercicio > 1)
                 fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

              if (*PQgetvalue (res, actual, 23) == 't') /* 23 => usa_header */
		 {
                  if (N_preguntas_ejercicio > 1)
		     {
  	              if (N_preguntas_ejercicio == 2)
	                 fprintf (Archivo_Latex, "\\textbf{Las preguntas %d y %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual+2);
	              else
	                 fprintf (Archivo_Latex, "\\textbf{Las preguntas %d a %d requieren la siguiente informaci\\'{o}n:}\n\n \n\n", actual+1, actual + N_preguntas_ejercicio);

	              strcpy (hilera_antes, PQgetvalue (res, actual, 24)); /* 24 => texto_header */
                      hilera_LATEX (hilera_antes, hilera_despues);
                      fprintf (Archivo_Latex, "%s\n", hilera_despues);

                      fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
		     }
		  else
		     {
		      Imprime_pregunta (actual, res, Archivo_Latex, PQgetvalue(res,actual, 24));  /* 24 => texto_header */
		      N_preguntas_ejercicio = 0;
		      actual++;
		     }
		 }

	      for (i=0; i < N_preguntas_ejercicio; i++)
		  {
		   Imprime_pregunta (actual+i, res, Archivo_Latex, " ");
		  }

              if (N_preguntas_ejercicio > 1)
                 fprintf (Archivo_Latex, "\n\n\\rule{8.1cm}{5pt}\n\n");

              actual += N_preguntas_ejercicio;

              gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.09 + (0.46 * ((long double) actual / (long double) N_preguntas)));
              while (gtk_events_pending ()) gtk_main_iteration ();
             }

       fprintf (Archivo_Latex, "\\end{questions}\n");
      }
   else
      fprintf (Archivo_Latex, "No hay ejercicios registrados.\n\n");

   fprintf (Archivo_Latex, "\\onecolumn\n");
   fprintf (Archivo_Latex, "\\clearpage\n");
   fprintf  (Archivo_Latex, "\\SetWatermarkText{}\n");

   Resumen_de_Banderas (Archivo_Latex);

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 0.55);
   while (gtk_events_pending ()) gtk_main_iteration ();

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX5010", 1, PB_reporte, 0.55, 0.9, NULL, NULL);

   g_free (materia);
   g_free (tema);
   g_free (subtema);
   
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB_reporte), 1.0);
   while (gtk_events_pending ()) gtk_main_iteration ();
   gtk_widget_hide (window2);
   gtk_widget_set_sensitive(window1, 1);

   gtk_widget_grab_focus (GTK_WIDGET(CB_materia));
}

void Imprime_pregunta (int i, PGresult * res, FILE * Archivo_Latex,char * prefijo)
{
   char alfileres[6];
   char hilera_antes [30000], hilera_despues [50000];
   long double media, varianza, desviacion;
   long double Por_A,Por_B,Por_C,Por_D,Por_E, Total;
   long double cota_inferior;
   PGresult * res_aux;
   char PG_command[2000];
   int j, N_links;
#define MINIMA 1.15
#define MAXIMA 6.65
   int flags [N_FLAGS];

   cota_inferior = MINIMA/MAXIMA*100.0;

   strcpy (alfileres, PQgetvalue (res, i, 40)); /* 40 => alfileres_opciones */

   sprintf (hilera_antes,"\\question %s\n%s", prefijo, PQgetvalue (res, i, 33)); /* 33 => texto_pregunta */
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s\n", hilera_despues);

   for (j=0; j < N_FLAGS;j++) flags[j] = 0;

   Total = (long double) atof(PQgetvalue (res, i, 61)); /* 61 => frecuencia */
   if (Total > 0.0)
      {
       Por_A = (long double) atof(PQgetvalue (res, i, 62))/Total*100.0;  /* 62 => frecuencia_1 */
       Por_B = (long double) atof(PQgetvalue (res, i, 63))/Total*100.0;  /* 63 => frecuencia_2 */
       Por_C = (long double) atof(PQgetvalue (res, i, 64))/Total*100.0;  /* 64 => frecuencia_3 */
       Por_D = (long double) atof(PQgetvalue (res, i, 65))/Total*100.0;  /* 65 => frecuencia_4 */
       Por_E = (long double) atof(PQgetvalue (res, i, 66))/Total*100.0;  /* 66 => frecuencia_5 */

       Asigna_Banderas (i, res, flags);
      }

   fprintf (Archivo_Latex, "\\begin{answers}\n");

   Imprime_Opcion (Archivo_Latex, res, Detalles, alfileres, Total, Por_A, atoi(PQgetvalue (res, i, 62)), atof(PQgetvalue (res, i, 77)), *(PQgetvalue (res, i, 39)), i, 0);
   Imprime_Opcion (Archivo_Latex, res, Detalles, alfileres, Total, Por_B, atoi(PQgetvalue (res, i, 63)), atof(PQgetvalue (res, i, 78)), *(PQgetvalue (res, i, 39)), i, 1);
   Imprime_Opcion (Archivo_Latex, res, Detalles, alfileres, Total, Por_C, atoi(PQgetvalue (res, i, 64)), atof(PQgetvalue (res, i, 79)), *(PQgetvalue (res, i, 39)), i, 2);
   Imprime_Opcion (Archivo_Latex, res, Detalles, alfileres, Total, Por_D, atoi(PQgetvalue (res, i, 65)), atof(PQgetvalue (res, i, 80)), *(PQgetvalue (res, i, 39)), i, 3);
   Imprime_Opcion (Archivo_Latex, res, Detalles, alfileres, Total, Por_E, atoi(PQgetvalue (res, i, 66)), atof(PQgetvalue (res, i, 81)), *(PQgetvalue (res, i, 39)), i, 4);

   fprintf (Archivo_Latex, "\\end{answers}\n");

   if (Detalles)
      {
       Imprime_Banderas (Archivo_Latex, flags, i, res);

       fprintf (Archivo_Latex,"\\framebox[8cm][l]{\\textbf{%.6s.%s} -  %s %s %s}\n\n", PQgetvalue (res, i, 0), PQgetvalue (res, i, 26),  
                                                                                       PQgetvalue (res, i, 1), PQgetvalue (res, i,  2), PQgetvalue (res, i,  3));

       sprintf (PG_command,"SELECT * from bd_ejercicios where texto_ejercicio = '%.6s' and materia != '%s'", PQgetvalue (res, i, 9),PQgetvalue (res, i, 1));
       res_aux = PQEXEC(DATABASE, PG_command);
       N_links = PQntuples(res_aux);
       for (j=0;j<N_links;j++)
	   {
            fprintf (Archivo_Latex,"\\framebox[8cm][l]{Ligada con %.6s - %s %s %s}\n\n", PQgetvalue (res_aux, j, 0),
                                                                                         PQgetvalue (res_aux, j, 1), PQgetvalue (res_aux, j,  2), PQgetvalue (res_aux, j,  3));
	   }
       PQclear(res_aux);

       sprintf (hilera_antes, "\\framebox[8cm][l]{Autor: %s}", PQgetvalue (res, i, 83));
       hilera_LATEX (hilera_antes, hilera_despues);
       fprintf (Archivo_Latex, "%s\n\n", hilera_despues);

       if (atoi(PQgetvalue (res, i, 61)))
	  {
           fprintf (Archivo_Latex,"\\framebox[8cm][l]{Creada: %s/%s/%s - \\'{U}ltimo Uso: %s/%s/%s - %s %s}\n\n",    PQgetvalue (res, i, 31), PQgetvalue (res, i, 30), PQgetvalue (res, i, 29),
	  	                  PQgetvalue (res, i, 44), PQgetvalue (res, i, 43), PQgetvalue (res, i, 42), PQgetvalue (res, i, 61), strcmp("1",PQgetvalue (res, i, 61))==0?"vez":"veces");
 	   fprintf (Archivo_Latex,"\\framebox[8cm][l]{Ha aparecido en %s %s}\n\n", PQgetvalue (res, i, 46), strcmp("1",PQgetvalue (res, i, 46))==0?"examen":"ex\\'{a}menes diferentes");
	  }
       else
          {
	   fprintf (Archivo_Latex,"\\framebox[8cm][l]{Creada: %s/%s/%s - \\textbf{No ha sido usada}}\n\n",    PQgetvalue (res, i, 31), PQgetvalue (res, i, 30), PQgetvalue (res, i, 29));
          }


       media      = (long double) atof(PQgetvalue (res, i, 51));
       varianza   = (long double) atof(PQgetvalue (res, i, 52));
       desviacion = sqrt(varianza);
       fprintf (Archivo_Latex,"\\framebox[8cm][l]{Media: %6.4Lf - Varianza: %6.4Lf - Desviaci\\'{o}n: %6.4Lf}\n\n", media, varianza, desviacion);
       if (atoi(PQgetvalue (res, i, 61)))
	  {
 	   fprintf (Archivo_Latex,"\\framebox[8cm][l]{Media de Ex\\'{a}menes: %6.2Lf - %6.2Lf (\\textbf{+}) - %6.2Lf (\\textbf{-})}\n\n", (long double)atof(PQgetvalue (res, i, 48)),(long double)atof(PQgetvalue (res, i, 54)),(long double)atof(PQgetvalue (res, i, 55)));
 	   fprintf (Archivo_Latex,"\\framebox[8cm][l]{\\'{I}ndice de Discriminaci\\'{o}n ($r_{pb}$): %6.4Lf}\n\n", (long double)atof(PQgetvalue (res, i, 53)));
 	   fprintf (Archivo_Latex,"\\framebox[8cm][l]{$\\alpha$ de Cronbach promedio: %6.4Lf (con) %6.4Lf (sin)}\n\n", 
                   (long double)atof(PQgetvalue (res, i, 58)), (long double)atof(PQgetvalue (res, i, 59)));
	  }
      }

   fprintf (Archivo_Latex, "\\rule{8cm}{1pt}\n");
   fprintf (Archivo_Latex, "\n\n");
}

void Imprime_Opcion (FILE * Archivo_Latex, PGresult *res, int Detalles, char * alfileres, long double Total, long double Porcentaje, int Frecuencia, long double Rpb, char correcta, int pregunta, int opcion)
{
   char hilera_antes [30000], hilera_despues [30000];

   fprintf (Archivo_Latex, "\\item ");
   if (Detalles && alfileres [opcion] == '1') fprintf(Archivo_Latex,"{\\huge $\\bigtriangledown$} ");
   strcpy (hilera_antes, PQgetvalue (res, pregunta, 34 + opcion)); /* 34 ==> Inicio de textos de opciones */
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);

   if (Detalles)
      {
       if (Total > 0.0)
	  {
           fprintf (Archivo_Latex,"\n\n{\\color{green} \\rule{%Lfcm}{5pt}\n\n{\\footnotesize \\textbf{\\texttt{%6.2Lf \\%%}}}} (\\textbf{%d})\n\n",
   	                          0.05+MAXIMA*Porcentaje/100.0, Porcentaje, Frecuencia);
           fprintf (Archivo_Latex," $r_{pb}$ = \\textbf{%Lf}\n\n", Rpb);
	  }

       if (correcta == ('A' + opcion))
          {
           fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{2\\fboxrule}\n");
           fprintf (Archivo_Latex, "\n\n\\fcolorbox{black}{blue}{\\color{white} $\\star\\star\\star$ \\textbf{CORRECTA} $\\star\\star\\star$}");
           fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.5\\fboxrule}\n");
          }
      }

   fprintf (Archivo_Latex, "\n\n");
}

void Asigna_Banderas (int i, PGresult * res, int * flags)
{
  int j, N_distractores_usados;

  for (j=0;j<N_FLAGS;j++) flags[j] = 0;

  if (atof(PQgetvalue (res, i, 51)) == 1.0) 
     flags[TODAS_BUENAS] = 1; /* 51 dificultad */

  if (atof(PQgetvalue (res, i, 51)) == 0.0) 
     flags[TODAS_MALAS]  = 1; /* 51 dificultad */

  if ((atof(PQgetvalue (res, i, 51)) >  0.0) && (atof(PQgetvalue (res, i, 51)) <=  0.3))
     flags[PORCENTAJE_MENOR_30]    = 1; /* 51 dificultad */

  if (atof(PQgetvalue (res, i, 53)) >= 0.3) 
     flags[RPB_BUENO]    = 1; /* 53 point_biserial */

  if (atof(PQgetvalue (res, i, 53)) < 0.0) 
     flags[RPB_NEGATIVO] = 1; /* 53 point_biserial */

  if ((atof(PQgetvalue (res, i, 59)) - atof(PQgetvalue (res, i, 58))) > 0.1)
     flags[ALFA_SUBE_MUCHO] = 1; /* 58 Cronbach con - 59 Cronbach sin */
  else
     if ((atof(PQgetvalue (res, i, 59)) - atof(PQgetvalue (res, i, 58))) > 0.04)
        flags[ALFA_SUBE] = 1; /* 58 Cronbach con - 59 Cronbach sin */

  N_distractores_usados = 0;
  for (j=0;j<5;j++)
      {
       if ((atof(PQgetvalue (res, i, 77+j)) > 0.3) && (*PQgetvalue (res, i, 39) != ('A' + j)))
          flags[RPB_DISTRACTOR_MUY_POSITIVO] = 1;
       else
          if ((atof(PQgetvalue (res, i, 77+j)) > 0.0) && (*PQgetvalue (res, i, 39) != ('A' + j)))
             flags[RPB_DISTRACTOR_POSITIVO] = 1;
	  else
             if ((atof(PQgetvalue (res, i, 77+j)) < -0.29) && (*PQgetvalue (res, i, 39) != ('A' + j)))
                flags[RPB_DISTRACTOR_MUY_NEGATIVO] = 1;

       if (atoi(PQgetvalue (res, i, 62 + j)) > 0) N_distractores_usados++;  /* 62 frecuencia_1 */
      }

  if (N_distractores_usados == 2)
     flags[SOLO_2_DISTRACTORES] = 1;
  else
     if ((N_distractores_usados == 3) || (N_distractores_usados == 4))
        flags[MENOS_DE_5_DISTRACTORES] = 1;
     else
        if (N_distractores_usados == 5)
           flags[USA_5_DISTRACTORES] = 1;

  if ((atof(PQgetvalue (res, i, 53)) >= 0.3) && (atof(PQgetvalue (res, i, 51)) <= 0.75))
     {
      if (N_distractores_usados <= 3) 
         flags[RPB_Y_DIFICULTAD_OK] = 1;
      else
	if ((N_distractores_usados == 5) && !flags[RPB_DISTRACTOR_MUY_POSITIVO])
           flags[RPB_DIFI_DISTRA_OK]  = 1;
     }
}

void Imprime_Banderas (FILE * Archivo_Latex, int * flags, int i, PGresult * res)
{
   char mensaje [1000];

   if (flags[RPB_BUENO])
      {
       sprintf (mensaje,"Esta pregunta muestra un buen \\'{i}ndice de discrimaci\\'{o}n ($r_{pb}$ = \\textbf{%6.4Lf}). Separa muy bien a los estudiantes de buen rendimiento de los de bajo rendimiento.",
		         (long double) atof(PQgetvalue (res, i, 53)));
       Cajita_con_bandera (Archivo_Latex, mensaje, AZUL);
      }

   if (flags[RPB_Y_DIFICULTAD_OK])
      {
       sprintf (mensaje,"Esta pregunta no es f\\'acil (\\textit{p} = \\textbf{%6.4Lf}) y tiene un buen \\'indice de discriminaci\\'on ($r_{pb}$ = \\textbf{%6.4Lf}). \\textbf{Buena pregunta}\n", 
		(long double) atof(PQgetvalue (res, i, 51)), (long double) atof(PQgetvalue (res, i, 53)));

       Cajita_con_bandera (Archivo_Latex, mensaje, AZUL);
      }

   if (flags[USA_5_DISTRACTORES])
      {
       sprintf (mensaje,"Todos los distractores han sido utilizados. \\textbf{Pregunta bien dise\\~nada}.");
       Cajita_con_bandera (Archivo_Latex, mensaje, CYAN);
      }

   if (flags[RPB_DISTRACTOR_MUY_NEGATIVO])
      {
       sprintf (mensaje,"Hay al menos un \\textbf{distractor} con un $r_{pb}$ muy negativo, que atrae a los estudiantes de bajo rendimiento y no es considerado por los estudiantes de buen rendimiento. \\textbf{Buen distractor}.");
       Cajita_con_bandera (Archivo_Latex, mensaje, CYAN);
      }

   if (flags[PORCENTAJE_MENOR_30])
      {
       sprintf (mensaje,"S\\'{o}lo el \\textbf{%5.2Lf}\\%% de los estudiantes evaluados han contestado correctamente esta pregunta.",
                       (long double) atof(PQgetvalue (res, i, 51)) * 100.0);
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO);
      }

   if (flags[ALFA_SUBE])
      {
       sprintf (mensaje,"En promedio, al eliminar esta pregunta de los ex\\'{a}menes donde aparece, el $\\alpha$ de Cronbach sube de \\textbf{%6.4Lf} a \\textbf{%6.4Lf}.",
		(long double) atof(PQgetvalue (res, i, 58)), (long double) atof(PQgetvalue (res, i, 59)));
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO);
      }

   if (flags[RPB_DISTRACTOR_POSITIVO])
      {
       sprintf (mensaje,"Hay al menos un \\textbf{distractor} con un $r_{pb}$ ligeramente positivo. Es necesario revisar el enunciado y las opciones.");
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO);
      }

   if (flags[MENOS_DE_5_DISTRACTORES])
      {
       sprintf (mensaje,"Hay por lo menos un distractor que \\textbf{nunca} ha atra\\'ido a los estudiantes. Mejorar redacci\\'on.");
       Cajita_con_bandera (Archivo_Latex, mensaje, AMARILLO);
      }

   if (flags[TODAS_BUENAS])
      {
       sprintf (mensaje,"\\textbf{Todos} los estudiantes han contestado correctamente, por lo que la pregunta no discrimina de manera efectiva.");
       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO);
      }

   if (flags[SOLO_2_DISTRACTORES])
      {
       sprintf (mensaje,"Los estudiantes s\\'olo se interesan en 2 distractores, los otros 3 han sido in\\'{u}tiles.");
       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO);
      }

   if (flags[TODAS_MALAS])
      {
       sprintf (mensaje,"Ni un solo estudiante ha contestado correctamente esta pregunta. La pregunta no discrimina de manera efectiva y posiblemente est\\'a mal redactada.");

       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO);
      }

   if (flags[RPB_NEGATIVO])
      {
       sprintf (mensaje, "Esta pregunta muestra un \\'{i}ndice de discrimaci\\'{o}n ($r_{pb}$ = \\textbf{%6.4Lf}) negativo. Los estudiantes de buen rendimiento en este examen tendieron a equivocarse, mientras que los de bajo rendimiento tendieron a contestarla bien. \\textbf{Revisar muy bien el enunciado y las opciones de la pregunta}",
                         (long double) atof(PQgetvalue (res, i, 53)));

       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO);
      }

   if (flags[ALFA_SUBE_MUCHO])
      {
       sprintf (mensaje, "Si esta pregunta se elimina del examen, el $\\alpha$ de Cronbach subir\\'{i}a \\textbf{considerablemente} (de \\textbf{%6.4Lf} a \\textbf{%6.4Lf}).",
                         (long double) atof(PQgetvalue (res, i, 58)), (long double) atof(PQgetvalue (res, i, 59)));

       Cajita_con_bandera (Archivo_Latex, mensaje, ROJO);
      }

   if (flags[RPB_DIFI_DISTRA_OK])
      {
       sprintf (mensaje,"Esta pregunta no es f\\'acil (\\textit{p} = \\textbf{%6.4Lf}), muestra un buen \\'indice de discriminaci\\'on ($r_{pb}$ = \\textbf{%6.4Lf}), y los estudiantes usaron todos los distractores. \\textbf{Excelente pregunta}\n", 
		(long double) atof(PQgetvalue (res, i, 51)), (long double) atof(PQgetvalue (res, i, 53)));

       Cajita_con_bandera (Archivo_Latex, mensaje, VERDE);
      }
}

void Cajita_con_bandera (FILE * Archivo_Latex, char * mensaje, int color)
{
   int i;

   Banderas [color]++;

   fprintf (Archivo_Latex, "\\begin{figure}[H]\n");
   fprintf (Archivo_Latex, "\\centering\n");
   fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{4\\fboxrule}\n");
   fprintf (Archivo_Latex, "\\fcolorbox{%s}{white}{\n", colores[color]);
   fprintf (Archivo_Latex, "\\begin{minipage}{1.4 cm}\n");
   fprintf (Archivo_Latex, "\\includegraphics[scale=0.07, angle=45]{.imagenes/%s}\n", banderas[color]);
   fprintf (Archivo_Latex, "\\end{minipage}\n");
   fprintf (Archivo_Latex, "\\begin{minipage}{6.1 cm}\n");
   fprintf (Archivo_Latex, "\\small{%s}\n", mensaje); 
   fprintf (Archivo_Latex, "\\end{minipage}\n");
   fprintf (Archivo_Latex, "}\n");
   fprintf (Archivo_Latex, "\\setlength{\\fboxrule}{0.25\\fboxrule}\n");
   fprintf (Archivo_Latex, "\\end{figure}\n");
}

void Resumen_de_Banderas (FILE * Archivo_Latex)
{
   int color;

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
void on_WN_ex5010_destroy (GtkWidget *widget, gpointer user_data) 
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

void on_CK_detalles_toggled (GtkWidget *widget, gpointer user_data)
{
  Toggle_detalles (widget, user_data);
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
