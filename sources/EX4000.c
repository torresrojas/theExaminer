/*
Compilar con

cc -o EX4000 EX4000.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX4000:                                */
/*                                         */
/*    Revisa Exámenes                      */
/*    The Examiner 0.2                     */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    5 de Febrero 2011                    */
/*                                         */
/*-----------------------------------------*/
/*    Se reemplaza libglade por Gtkbuilder */
/*    Se incluye scroll window             */
/*    Autor: Francisco J. Torres-Rojas     */        
/*    17 de Marzo del 2012                 */
/*******************************************/
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include <locale.h>
#include <math.h>
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

GtkWidget *EB_examen   = NULL;
GtkWidget *FR_examen   = NULL;

GtkWidget *EB_respuestas = NULL;

GtkSpinButton *SP_examen          = NULL;
GtkWidget *EN_descripcion         = NULL;
GtkWidget *EN_materia             = NULL;
GtkWidget *EN_materia_descripcion = NULL;
GtkSpinButton *SP_secuencia       = NULL;
GtkWidget *EN_nombre              = NULL;
GtkWidget *EN_version             = NULL;
GtkWidget *EN_buenas              = NULL;

GtkWidget *FR_reporte             = NULL;
GtkWidget *FR_respuestas          = NULL;
GtkWidget *SW_respuestas          = NULL;
GtkWidget *FR_botones             = NULL;
GtkWidget *FR_procesado           = NULL;

GtkToggleButton *RB_secuencia     = NULL;
GtkToggleButton *RB_nota          = NULL;
GtkToggleButton *RB_nombre        = NULL;

GtkWidget *BN_print               = NULL;
GtkWidget *BN_save                = NULL;
GtkWidget *BN_delete              = NULL;
GtkWidget *BN_undo                = NULL;
GtkWidget *BN_up                  = NULL;
GtkWidget *BN_terminar            = NULL;
GtkWidget *BN_ok                  = NULL;

#define MAX_PREGUNTAS 240
#define MAX_FRAMES    (MAX_PREGUNTAS/3)

GtkWidget *EN_respuesta [MAX_PREGUNTAS];
GtkWidget *FR_trio      [MAX_FRAMES];

GtkWidget *SC_versiones           = NULL;
GtkLabel *LB_nota                = NULL;
GtkImage  *IM_smile1 = NULL;
GtkImage  *IM_smile2 = NULL;
GtkImage  *IM_smile3 = NULL;
GtkImage  *IM_smile4 = NULL;
GtkImage  *IM_smile5 = NULL;

GtkWidget *FR_ex = NULL;
GtkWidget *DA_ex = NULL;

GtkLabel *LB_preguntas  = NULL;

/***********************/
/* Prototypes          */
/***********************/
void Actualiza_Porcentajes         ();
void Borra_Respuesta               (GtkWidget *widget, gpointer user_data);
void Cambia_secuencia              ();
void Cambio_Examen                 ();
void Cambia_Respuesta              (int k);
void Cambia_Version                ();
void Connect_Widgets               ();
void Fin_de_Programa               (GtkWidget *widget, gpointer user_data);
void Fin_Ventana                   (GtkWidget *widget, gpointer user_data);
void Graba_Respuestas              (GtkWidget *widget, gpointer user_data);
void Imprime_Reporte               ();
void Init_Examen                   ();
void Init_Respuestas               ();
void Init_secuencia                ();
void Interface_Coloring            ();
void on_BN_delete_clicked          (GtkWidget *widget, gpointer user_data);
void on_BN_print_clicked           (GtkWidget *widget, gpointer user_data);
void on_BN_save_clicked            (GtkWidget *widget, gpointer user_data);
void on_BN_terminar_clicked        (GtkWidget *widget, gpointer user_data);
void on_BN_undo_clicked            (GtkWidget *widget, gpointer user_data);
void on_BN_up_clicked              (GtkWidget *widget, gpointer user_data);
void on_EN_nombre_activate         (GtkWidget *widget, gpointer user_data);
void on_EN_insert_text             (GtkWidget *widget, gpointer user_data);
void on_SC_versiones_value_changed (GtkWidget *widget, gpointer user_data);
void on_SP_secuencia_activate      (GtkWidget *widget, gpointer user_data);
void on_WN_ex4000_destroy          (GtkWidget *widget, gpointer user_data);
void Read_Only_Fields              ();
void Update_background_color(long double nota);
void on_DA_ex_expose(GtkWidget *widget, gpointer user_data);

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

#define EXAMEN_SIZE              6
#define DESCRIPCION_SIZE         201
#define CODIGO_SIZE              5

int N_versiones        = 0;
int N_contestadas      = 0;
int Numero_Preguntas   = 0;
int Numero_Estudiantes = 0;
int N_frames           = 0;
long double Nota       = 0.0;

struct VERSION
{
  char codigo [CODIGO_SIZE];
  char * respuestas;
} * versiones = NULL;

int Correctas [MAX_PREGUNTAS];
int Total_Correctas;

int RESPUESTA_YA_EXISTE = 0;
int DESPLIEGUE_INICIAL  = 0;
int MODO_REVISION       = 0;

#define PI 3.141592654
#define DA_HEIGHT 95
#define DA_WIDTH  181
#define DA_MIDDLE ((DA_WIDTH / 2) - 1)
#define DA_BEGIN   5
#define CERO (DA_BEGIN + 84)
#define ARROW_LENGTH 68
#define ARROW_SIZE   16
#define ARROW_WIDTH  10
#define ARROW_BODY (ARROW_LENGTH - ARROW_SIZE + 2)
#define SHADOW_OFFSET 4

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
      if (!gtk_builder_add_from_file (builder, ".interfaces/EX4000.glade", &error))
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

          /* Some fields are READ ONLY */
	  Read_Only_Fields ();

	  /* The magic of color...  */
	  Interface_Coloring ();

          gtk_widget_show (window1);
          Init_Examen ();

          /* start the event loop */
          gtk_main();
	 }
     }

  return 0;
}

void Connect_Widgets()
{
  int i;
  char hilera [30];

  window1              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_ex4000"));
  window2              = GTK_WIDGET (gtk_builder_get_object (builder, "WN_procesado"));

  EB_examen = GTK_WIDGET (gtk_builder_get_object (builder, "EB_examen"));
  FR_examen = GTK_WIDGET (gtk_builder_get_object (builder, "FR_examen"));

  EB_respuestas = GTK_WIDGET (gtk_builder_get_object (builder, "EB_respuestas"));

  SP_examen          = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_examen"));
  EN_descripcion         = GTK_WIDGET (gtk_builder_get_object (builder, "EN_descripcion"));
  EN_materia             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia"));
  EN_materia_descripcion = GTK_WIDGET (gtk_builder_get_object (builder, "EN_materia_descripcion"));
  EN_version             = GTK_WIDGET (gtk_builder_get_object (builder, "EN_version"));
  EN_buenas              = GTK_WIDGET (gtk_builder_get_object (builder, "EN_buenas"));
  SP_secuencia           = (GtkSpinButton *) GTK_WIDGET (gtk_builder_get_object (builder, "SP_secuencia"));
  EN_nombre              = GTK_WIDGET (gtk_builder_get_object (builder, "EN_nombre"));

  SC_versiones           = GTK_WIDGET (gtk_builder_get_object (builder, "SC_versiones"));

  FR_reporte             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_reporte"));
  FR_respuestas          = GTK_WIDGET (gtk_builder_get_object (builder, "FR_respuestas"));
  FR_botones             = GTK_WIDGET (gtk_builder_get_object (builder, "FR_botones"));
  FR_procesado           = GTK_WIDGET (gtk_builder_get_object (builder, "FR_procesado"));

  SW_respuestas          = GTK_WIDGET (gtk_builder_get_object (builder, "SW_respuestas"));

  for (i=0; i<MAX_PREGUNTAS;i++)
      {
       sprintf (hilera,"EN_%03d",i+1);
       EN_respuesta [i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));
      }

  for (i=0; i<MAX_FRAMES;i++)
      {
       sprintf (hilera,"FR_%03d",i+1);
       FR_trio [i] = GTK_WIDGET (gtk_builder_get_object (builder, hilera));
      }

  RB_secuencia = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_secuencia"));
  RB_nota      = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_nota"));
  RB_nombre    = (GtkToggleButton *) GTK_WIDGET (gtk_builder_get_object (builder, "RB_nombre"));

  BN_terminar = GTK_WIDGET (gtk_builder_get_object (builder, "BN_terminar"));
  BN_undo     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_undo"));
  BN_up       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_up"));
  BN_delete   = GTK_WIDGET (gtk_builder_get_object (builder, "BN_delete"));
  BN_save     = GTK_WIDGET (gtk_builder_get_object (builder, "BN_save"));
  BN_ok       = GTK_WIDGET (gtk_builder_get_object (builder, "BN_ok"));
  BN_print    = GTK_WIDGET (gtk_builder_get_object (builder, "BN_print"));

  LB_preguntas = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_preguntas"));
  
  LB_nota = (GtkLabel *) GTK_WIDGET (gtk_builder_get_object (builder, "LB_nota"));
  IM_smile1 = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile1"));
  IM_smile2 = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile2"));
  IM_smile3 = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile3"));
  IM_smile4 = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile4"));
  IM_smile5 = (GtkImage *) GTK_WIDGET (gtk_builder_get_object (builder, "IM_smile5"));

  FR_ex = GTK_WIDGET (gtk_builder_get_object (builder, "FR_ex"));
  DA_ex = GTK_WIDGET (gtk_builder_get_object (builder, "DA_ex"));

  g_signal_connect( G_OBJECT( DA_ex), "expose-event", G_CALLBACK( on_DA_ex_expose ),     NULL);
}

void Read_Only_Fields ()
{
  gtk_widget_set_can_focus(EN_materia            , FALSE);
  gtk_widget_set_can_focus(EN_version            , FALSE);
  gtk_widget_set_can_focus(EN_buenas             , FALSE);
  gtk_widget_set_can_focus(EN_descripcion        , FALSE);
  gtk_widget_set_can_focus(EN_materia_descripcion, FALSE);
}

void Interface_Coloring ()
{
  int i;
  GdkColor color;

  gdk_color_parse (MAIN_WINDOW, &color);
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &color);

  gdk_color_parse (IMPORTANT_WINDOW, &color);
  gtk_widget_modify_bg(window2, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (IMPORTANT_FR, &color);
  gtk_widget_modify_bg(FR_procesado,  GTK_STATE_NORMAL, &color);

  gdk_color_parse (MAIN_AREA, &color);
  gtk_widget_modify_bg(EB_examen, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (THIRD_AREA, &color);
  gtk_widget_modify_bg(EB_respuestas, GTK_STATE_NORMAL,   &color);

  gdk_color_parse (STANDARD_FRAME, &color);
  gtk_widget_modify_bg(FR_respuestas, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_botones,  GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(FR_reporte,  GTK_STATE_NORMAL, &color);
  for (i=0; i<MAX_FRAMES;i++)
      {
       gtk_widget_modify_bg(FR_trio[i], GTK_STATE_NORMAL, &color);
      }

  gdk_color_parse (STANDARD_ENTRY, &color);
  gtk_widget_modify_bg(EN_nombre,     GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_examen), GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(GTK_WIDGET(SP_secuencia), GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(GTK_WIDGET(RB_secuencia), GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(GTK_WIDGET(RB_nota),      GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(GTK_WIDGET(RB_nombre),    GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (ANSWER_BOX_COLOR, &color);
  for (i=0; i<MAX_PREGUNTAS;i++)
      {
       gtk_widget_modify_bg(EN_respuesta[i], GTK_STATE_NORMAL, &color);
      }

  gdk_color_parse (BUTTON_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_undo,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_up,         GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_save,       GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_delete,     GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_terminar,   GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_print,      GTK_STATE_PRELIGHT, &color);
  gtk_widget_modify_bg(BN_ok,         GTK_STATE_PRELIGHT, &color);

  gdk_color_parse (BUTTON_ACTIVE, &color);
  gtk_widget_modify_bg(BN_save,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_undo,     GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_up,       GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_delete,   GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_terminar, GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_print,    GTK_STATE_ACTIVE, &color);
  gtk_widget_modify_bg(BN_ok,       GTK_STATE_ACTIVE, &color);

}

void Init_Examen()
{
   int  i, Last;
   char Examen              [EXAMEN_SIZE];

   gtk_entry_set_text(GTK_ENTRY(EN_descripcion),         "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_version),             "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_materia),             "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_nombre),              "\0");
   gtk_entry_set_text(GTK_ENTRY(EN_buenas),              "0");

   gtk_widget_set_sensitive(EN_descripcion          , 0);
   gtk_widget_set_sensitive(EN_materia              , 0);
   gtk_widget_set_sensitive(EN_materia_descripcion  , 0);
   gtk_widget_set_sensitive(EN_version              , 0);
   gtk_widget_set_sensitive(EN_buenas               , 0);
   gtk_widget_set_sensitive(GTK_WIDGET(SP_secuencia), 0);
   gtk_widget_set_sensitive(EN_nombre               , 0);

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

   Numero_Preguntas = N_versiones = N_frames = 0;

   if (versiones) free(versiones);
   versiones = NULL;

   Init_Respuestas ();

   gtk_widget_set_sensitive(FR_reporte, 0);
   gtk_toggle_button_set_active(RB_secuencia, TRUE);

   gtk_widget_set_sensitive(BN_undo,   0);
   gtk_widget_set_sensitive(BN_up,     0);
   gtk_widget_set_sensitive(BN_save,   0);
   gtk_widget_set_sensitive(BN_delete, 0);

   gtk_range_set_range (GTK_RANGE(SC_versiones),  (gdouble) 0.0, (gdouble) 1.0);
   gtk_range_set_value (GTK_RANGE(SC_versiones),  (gdouble) 0.0);

   gtk_widget_set_sensitive(SC_versiones, 0);

   DESPLIEGUE_INICIAL = 1;
   gtk_spin_button_set_range (SP_secuencia, 1.0, 1.0);
   gtk_spin_button_set_value (SP_secuencia, 1);

   gtk_widget_set_sensitive(GTK_WIDGET(SP_examen), 1);

   if (Last > 1)
      gtk_widget_grab_focus (GTK_WIDGET(SP_examen));
   else
      Cambio_Examen ();
   
   Update_background_color(0.0);
}

void Init_Respuestas ()
{
  int i;
  char label[50];
  
  if (Numero_Preguntas == 0)
     gtk_widget_set_sensitive(FR_respuestas, 0);
  else
     gtk_widget_set_sensitive(FR_respuestas, 1);

  for (i=0; i<MAX_PREGUNTAS;i++)
      {
       gtk_entry_set_text      (GTK_ENTRY(EN_respuesta [i]), "\0");
       gtk_widget_set_sensitive          (EN_respuesta [i], 0);
      }

  for (i=0; i<MAX_FRAMES;i++)
      {
       gtk_widget_set_visible  (FR_trio [i], FALSE);
      }

  N_contestadas = 0;
  gtk_widget_set_sensitive(BN_save,   0);
  N_frames = Numero_Preguntas/3;

  if (Numero_Preguntas % 3) N_frames++;

  for (i=0;i<Numero_Preguntas;i++)
      {
       gtk_widget_set_sensitive          (EN_respuesta [i], 1);
      }

  for (i=0;i<N_frames;i++)
      {
       gtk_widget_set_visible  (FR_trio [i], TRUE);
      }

  Nota = 0.0;

  sprintf (label,"de %d", Numero_Preguntas);
  gtk_label_set_markup(LB_preguntas, label);

  MODO_REVISION = 0;
}

void Cambio_Examen()
{
  int   i,j,k;
  char examen[10];
  char  hilera [500];
  int respuesta_original;
  int opcion_1,opcion_2,opcion_3,opcion_4,opcion_5;

  char Descripcion         [2*DESCRIPCION_SIZE]         = "Examen no está registrado o ya fue procesado";
  char PG_command          [2000];
  PGresult *res, *res_aux;

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  sprintf (PG_command,"SELECT codigo_examen, descripcion, descripcion_materia, version, ejecutado, materia from EX_examenes, EX_versiones, BD_materias where codigo_examen = '%s' and codigo_examen = examen and materia = codigo_materia and codigo_tema = '          ' and codigo_subtema = '          ' order by version", examen);
  res = PQEXEC(DATABASE, PG_command);

  N_versiones = PQntuples(res);

  if (N_versiones)
     {
      if (*PQgetvalue (res, 0, 4) == 't')
	 {
          gtk_widget_set_sensitive(window1, 0);
          gtk_widget_show (window2);
	  MODO_REVISION = 1;
	 }

      gtk_widget_set_sensitive(GTK_WIDGET(SP_examen), 0);
      gtk_widget_set_sensitive(EN_materia, 1);
      gtk_widget_set_sensitive(EN_materia_descripcion, 1);
      gtk_widget_set_sensitive(EN_descripcion        , 1);
      gtk_widget_set_sensitive(GTK_WIDGET(SP_secuencia), 1);
      gtk_widget_set_sensitive(BN_undo,   1);

      versiones = (struct VERSION *) malloc (sizeof(struct VERSION) * N_versiones);
      for (i=0;i<N_versiones;i++)
          {
	   strcpy (versiones[i].codigo,     PQgetvalue (res, i, 3));

           sprintf (PG_command,"SELECT respuesta, opcion_1, opcion_2, opcion_3, opcion_4, opcion_5 from ex_examenes_preguntas, bd_texto_preguntas where examen = '%.5s' and version = '%.4s' and codigo_pregunta = codigo_unico_pregunta order by posicion", examen, PQgetvalue (res, i, 3));
	   res_aux = PQEXEC(DATABASE, PG_command);

	   Numero_Preguntas = PQntuples(res_aux);
	   versiones[i].respuestas = (char *) malloc(sizeof(char) * Numero_Preguntas + 1);

	   for (j=0; j<Numero_Preguntas; j++)
	       {
  	        respuesta_original =     *PQgetvalue (res_aux, j, 0) - 'A';
	        opcion_1           = atoi(PQgetvalue (res_aux, j, 1));
	        opcion_2           = atoi(PQgetvalue (res_aux, j, 2));
	        opcion_3           = atoi(PQgetvalue (res_aux, j, 3));
	        opcion_4           = atoi(PQgetvalue (res_aux, j, 4));
	        opcion_5           = atoi(PQgetvalue (res_aux, j, 5));

	        if (respuesta_original == opcion_1) versiones[i].respuestas[j] = 'A';
	        if (respuesta_original == opcion_2) versiones[i].respuestas[j] = 'B';
	        if (respuesta_original == opcion_3) versiones[i].respuestas[j] = 'C';
	        if (respuesta_original == opcion_4) versiones[i].respuestas[j] = 'D';
	        if (respuesta_original == opcion_5) versiones[i].respuestas[j] = 'E';
	       }
           PQclear(res_aux);
	  }

      strcpy (Descripcion, PQgetvalue (res, 0, 1));
      gtk_entry_set_text(GTK_ENTRY(EN_materia),             PQgetvalue (res, 0, 5));
      gtk_entry_set_text(GTK_ENTRY(EN_materia_descripcion), PQgetvalue (res, 0, 2));

      for (i=0;i<Numero_Preguntas;i++) Correctas[i] = 0;
      Total_Correctas = 0;

      if (N_versiones > 1)
         {
          gtk_widget_set_sensitive(SC_versiones          , 1);
          gtk_range_set_range (GTK_RANGE(SC_versiones),  (gdouble) 1.0, (gdouble) N_versiones);
          gtk_range_set_value (GTK_RANGE(SC_versiones),  (gdouble) 1.0);
         }

      Init_secuencia ();
      Init_Respuestas ();
      gtk_widget_set_sensitive(FR_respuestas, 0);
      gtk_widget_grab_focus   (GTK_WIDGET(SP_secuencia));

      Actualiza_Porcentajes();
     }
  else
     gtk_widget_grab_focus   (GTK_WIDGET(SP_examen));

  PQclear(res);

  gtk_entry_set_text(GTK_ENTRY(EN_descripcion), Descripcion);
}

void Actualiza_Porcentajes ()
{ /* En caso que se haya actualizado la respuesta correcta de alguna pregunta */
  char PG_command          [2000];
  PGresult *res, *res_aux;
  int i, j, k;
  int N_estudiantes, N_correctas;
  long double Porcentaje;
  char examen[10];

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  sprintf (PG_command,"SELECT estudiante, version, respuestas from EX_examenes_respuestas where examen = '%s' order by estudiante", examen);
  res = PQEXEC(DATABASE, PG_command);
  N_estudiantes = PQntuples(res);

  for (i=0; i<N_estudiantes; i++)
      {
       sprintf (PG_command,"SELECT version from EX_versiones where examen = '%s' and version <= '%s'", examen, PQgetvalue(res, i, 1));
       res_aux = PQEXEC(DATABASE, PG_command);
       j = PQntuples(res_aux)-1;
       PQclear(res_aux);
       N_correctas = 0;
       for (k=0; k<Numero_Preguntas; k++)
	   {
	    if (versiones[j].respuestas [k] == PQgetvalue (res, i, 2)[k]) N_correctas++;
	   }
       Porcentaje = (long double) N_correctas/Numero_Preguntas * 100.0;

       sprintf (PG_command,"UPDATE ex_examenes_respuestas set correctas = %d, porcentaje = %Lf where examen = '%s' and estudiante = %d", 
		N_correctas, Porcentaje,
	        examen, i+1);
       res_aux = PQEXEC(DATABASE, PG_command);
      }
}

void Cambia_secuencia ()
{
  int i, j, k;
  char PG_command [1000];
  char examen[10];
  PGresult *res, *res_aux;
  char respuestas[1000];

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  k      = gtk_spin_button_get_value_as_int (SP_secuencia);

  sprintf (PG_command,"SELECT * from EX_examenes_respuestas where examen = '%s' and estudiante = %d", examen, k);
  res = PQEXEC(DATABASE, PG_command);

  if (PQntuples(res))
     {
      gtk_widget_set_sensitive(BN_delete, 1);
      RESPUESTA_YA_EXISTE = 1;

      gtk_entry_set_text(GTK_ENTRY(EN_nombre) , PQgetvalue(res, 0, 2));
      gtk_entry_set_text(GTK_ENTRY(EN_version), PQgetvalue(res, 0, 3));

      sprintf (PG_command,"SELECT version from EX_versiones where examen = '%s' and version <= '%s'", examen, PQgetvalue(res, 0, 3));
      res_aux = PQEXEC(DATABASE, PG_command);
      j = PQntuples(res_aux);
      PQclear(res_aux);

      gtk_range_set_value (GTK_RANGE(SC_versiones),  (gdouble) j);
      for (i=0;i<Numero_Preguntas;i++)
          {
           gtk_entry_set_text(GTK_ENTRY(EN_respuesta [i]),PQgetvalue(res, 0, 8)+i);
          }
     }
  else
     {
      RESPUESTA_YA_EXISTE = 0;
     }

  gtk_widget_set_sensitive(GTK_WIDGET(SP_secuencia), 0);

  gtk_widget_set_sensitive(SC_versiones , 1);
  gtk_widget_set_sensitive(EN_version   , 1);
  gtk_widget_set_sensitive(EN_buenas    , 1);
  gtk_widget_set_sensitive(EN_nombre    , 1);
  gtk_widget_set_sensitive(FR_respuestas, 1);
  gtk_widget_set_sensitive(BN_up        , 1);

  gtk_widget_grab_focus   (EN_nombre);

  PQclear(res);
}

void Cambia_Version ()
{
  int k;
  int i;
  gchar * respuesta;
  char hilera[30];

  k = (int) gtk_range_get_value (GTK_RANGE(SC_versiones));

  if (k > 0)
     {
      gtk_entry_set_text(GTK_ENTRY(EN_version), versiones[k-1].codigo);

      Total_Correctas = 0;
      for (i=0;i<Numero_Preguntas;i++)
	  {
           respuesta = gtk_editable_get_chars(GTK_EDITABLE(EN_respuesta [i]), 0, -1);

           Correctas [i] = (versiones [k-1].respuestas[i] == respuesta[0])?1:0;
           Total_Correctas += Correctas[i];

	   g_free (respuesta);
	  }

      Nota = 0.0;
      if (Numero_Preguntas) Nota = (long double) Total_Correctas / Numero_Preguntas;
      if (Nota > 1.0) Nota = 1.0;

      sprintf (hilera, "%11d",Total_Correctas);
      gtk_entry_set_text(GTK_ENTRY(EN_buenas), hilera);
      Update_background_color(Nota);
     }
}

void Cambia_Respuesta (int k)
{
  gchar *respuesta;
  int i,j;
  char hilera[20];


  j = (int) gtk_range_get_value (GTK_RANGE(SC_versiones))-1;

  respuesta = gtk_editable_get_chars(GTK_EDITABLE(EN_respuesta [k]), 0, -1);

  if ((respuesta [0] >= 'a') && (respuesta [0] <= 'e'))
     {
      respuesta [0] = respuesta [0]-'a'+'A';
     }

  if ((respuesta [0] < 'A') || (respuesta [0] > 'E'))
     {
      if ((respuesta [0] == ' ') || (respuesta[0] == '*'))
	 {
          respuesta [0] = '*';
          Total_Correctas -= Correctas[k];
          Correctas [k] = 0;

          gtk_widget_grab_focus (EN_respuesta [(k+1)%Numero_Preguntas]);
	 }
      else
	 {
          respuesta [0] = '\0';
          Total_Correctas -= Correctas[k];
          Correctas [k] = 0;

          gtk_widget_grab_focus (EN_respuesta [k]);
	 }
     }
  else
     {
      Total_Correctas -= Correctas[k];
      Correctas [k] = (versiones [j].respuestas[k] == respuesta[0])?1:0;
      Total_Correctas += Correctas[k];

      gtk_widget_grab_focus (EN_respuesta [(k+1)%Numero_Preguntas]);
     }

  Nota = 0.0;
  if (Numero_Preguntas) Nota = (long double) Total_Correctas / Numero_Preguntas;
  if (Nota > 1.0) Nota = 1.0;

  sprintf (hilera, "%11d",Total_Correctas);
  gtk_entry_set_text(GTK_ENTRY(EN_buenas), hilera);
  Update_background_color(Nota);

  gtk_entry_set_text(GTK_ENTRY(EN_respuesta[k]), respuesta);

  g_free (respuesta);

  N_contestadas=0;
  for (i=0;i<Numero_Preguntas;i++)
      {
       respuesta = gtk_editable_get_chars(GTK_EDITABLE(EN_respuesta [i]), 0, -1);
       if (respuesta[0]) N_contestadas++;
       g_free (respuesta);
      }
 
  if ((N_contestadas == Numero_Preguntas) && !MODO_REVISION)
     gtk_widget_set_sensitive(BN_save, 1);
  else
     gtk_widget_set_sensitive(BN_save, 0);
}

void Graba_Respuestas(GtkWidget *widget, gpointer user_data)
{
  int i, k, N_estudiante;
  char examen[10];
  gchar * version, * nombre, * respuesta, * buenas_texto;
  int correctas;
  char respuestas [MAX_PREGUNTAS];
  char PG_command          [2000];
  PGresult *res;

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  version      = gtk_editable_get_chars(GTK_EDITABLE(EN_version),    0, -1);
  nombre       = gtk_editable_get_chars(GTK_EDITABLE(EN_nombre), 0, -1);
  
  buenas_texto = gtk_editable_get_chars(GTK_EDITABLE(EN_buenas),     0, -1);

  N_estudiante = gtk_spin_button_get_value_as_int (SP_secuencia);
  correctas    = atoi(buenas_texto);

  for (i=0;i<Numero_Preguntas;i++)
      {
       respuesta = gtk_editable_get_chars(GTK_EDITABLE(EN_respuesta [i]), 0, -1);
       respuestas [i] = respuesta [0];
       g_free (respuesta);
      }

  respuestas[i] = '\0';

  if (RESPUESTA_YA_EXISTE)
     {
      sprintf (PG_command,"UPDATE ex_examenes_respuestas set nombre ='%s', version = '%s', correctas = %d, porcentaje = %Lf, respuestas = '%s' where examen = '%s' and estudiante = %d", 
	       nombre, version, correctas, Nota*100.0, respuestas,
	       examen, N_estudiante);
      res = PQEXEC(DATABASE, PG_command);
     }
  else
     {
      sprintf (PG_command,"INSERT into ex_examenes_respuestas values ('%.5s',%d, '%s', '%.4s', %d, %Lf, %Lf, %Lf, '%s')", 
  	                   examen, N_estudiante, nombre, version, correctas, Nota*100, Nota*100, Nota*100, respuestas);
      res = PQEXEC(DATABASE, PG_command);
     }

  PQclear(res);

  g_free (version);
  g_free (nombre);
  g_free (buenas_texto);

  Init_Respuestas ();
  Init_secuencia  ();
  gtk_widget_grab_focus (GTK_WIDGET(SP_secuencia));
}

void Borra_Respuesta(GtkWidget *widget, gpointer user_data)
{
  int i, k, N_estudiante;
  char examen[10];
  gchar * version, * comentario, * respuesta, * nota_texto, * buenas_texto;
  long double nota;
  int correctas;
  char respuestas [MAX_PREGUNTAS];
  char PG_command          [2000];
  PGresult *res;

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  N_estudiante = gtk_spin_button_get_value_as_int (SP_secuencia);

  sprintf (PG_command,"DELETE from ex_examenes_respuestas where examen = '%s' and estudiante = %d", examen, N_estudiante);
  res = PQEXEC(DATABASE, PG_command);
  PQclear(res);

  Init_secuencia  ();
  Init_Respuestas ();
  gtk_widget_grab_focus (GTK_WIDGET(SP_secuencia));
}

void Init_secuencia ()
{
  int k;
  char PG_command[1000];
  PGresult *res;
  char examen[10];

  k = (int) gtk_spin_button_get_value_as_int (SP_examen);
  sprintf (examen, "%05d", k);

  sprintf (PG_command,"SELECT estudiante from EX_examenes_respuestas where examen = '%s' order by estudiante DESC", examen);
  res = PQEXEC(DATABASE, PG_command);

  Numero_Estudiantes = 1;
  if (PQntuples(res))
     {
      Numero_Estudiantes = atoi (PQgetvalue(res, 0, 0)) + 1;
      gtk_widget_set_sensitive(FR_reporte, 1);
     }

  PQclear(res);

  DESPLIEGUE_INICIAL = 1;
  gtk_spin_button_set_range (SP_secuencia, 1.0, (long double) Numero_Estudiantes);
  gtk_spin_button_set_value (SP_secuencia, Numero_Estudiantes);
  DESPLIEGUE_INICIAL = 0;

  gtk_entry_set_text(GTK_ENTRY(EN_nombre), "\0");

  gtk_range_set_value (GTK_RANGE(SC_versiones),  (gdouble) 1.0);

  gtk_widget_set_sensitive(FR_respuestas, 0);
  gtk_widget_set_sensitive(EN_version            , 0);
  gtk_widget_set_sensitive(EN_buenas             , 0);

  gtk_widget_set_sensitive(BN_up,     0);
  gtk_widget_set_sensitive(BN_delete, 0);

  gtk_widget_set_sensitive(SC_versiones, 0);

  gtk_widget_set_sensitive(EN_nombre   , 0);
  gtk_widget_set_sensitive(GTK_WIDGET (SP_secuencia), 1);
}

void Imprime_Reporte()
{
   int i, k, N_examenes;
   char PG_command [2000];
   PGresult *res;
   char examen[10];
   gchar * descripcion, *materia;
   char hilera_antes [2000], hilera_despues [2000];
   FILE * Archivo_Latex;

   k = (int) gtk_spin_button_get_value_as_int (SP_examen);
   sprintf (examen, "%05d", k);

   materia     = gtk_editable_get_chars(GTK_EDITABLE(EN_materia_descripcion), 0, -1);
   descripcion = gtk_editable_get_chars(GTK_EDITABLE(EN_descripcion),         0, -1);

   Archivo_Latex = fopen ("EX4000.tex","w");

   fprintf (Archivo_Latex, "\\documentclass[9pt,journal, onecolumn, oneside]{EXAMINER}\n");

   EX_latex_packages (Archivo_Latex);
   fprintf (Archivo_Latex, "\n%s\n\n", parametros.Paquetes);

   fprintf (Archivo_Latex, "\\SetWatermarkText{Confidencial}\n");

   fprintf (Archivo_Latex, "\\begin{document}\n");
   sprintf (hilera_antes, "\\title{Examen %s - Respuestas Registradas\\\\%s - %s}\n", examen, descripcion, materia);
   hilera_LATEX (hilera_antes, hilera_despues);
   fprintf (Archivo_Latex, "%s", hilera_despues);
   fprintf (Archivo_Latex, "\\maketitle\n\n");

   if (gtk_toggle_button_get_active(RB_secuencia))
      sprintf (PG_command,"SELECT * from EX_examenes_respuestas where examen = '%s' order by estudiante", examen);
   else
      if (gtk_toggle_button_get_active(RB_nota))
         sprintf (PG_command,"SELECT * from EX_examenes_respuestas where examen = '%s' order by porcentaje DESC", examen);
      else
         sprintf (PG_command,"SELECT * from EX_examenes_respuestas where examen = '%s' order by nombre", examen);

   res = PQEXEC(DATABASE, PG_command);
   N_examenes = PQntuples(res);

   fprintf (Archivo_Latex, "\n\n");

   fprintf (Archivo_Latex, "\\begin{center}\n");
   fprintf (Archivo_Latex, "\\begin{longtable}{|r|l|c|c|r|}\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Secuencia} & \\textbf{Nombre} & \\textbf{Versi\\'{o}n} & \\textbf{Correctas} & \\textbf{Porcentaje} \\\\ \\hline \\hline\n");
   fprintf (Archivo_Latex, "\\endfirsthead\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Secuencia} & \\textbf{Nombre} & \\textbf{Versi\\'{o}n} & \\textbf{Correctas} & \\textbf{Porcentaje} \\\\ \\hline \\hline\n");
   fprintf (Archivo_Latex, "\\endhead\n");

   for (i=0; i<N_examenes; i++)
       {
	sprintf (hilera_antes,"%d & %s & %s & %d/%d & %7.2Lf\\\\ \\hline", 
		              atoi(PQgetvalue (res, i, 1)), PQgetvalue (res, i, 2), PQgetvalue (res, i, 3),
		              atoi(PQgetvalue (res, i, 4)), Numero_Preguntas,
		              (long double) atof(PQgetvalue (res, i, 4))/Numero_Preguntas*100.0);
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s\n", hilera_despues);
       }

   fprintf (Archivo_Latex, "\\end{longtable}\n");
   fprintf (Archivo_Latex, "\\end{center}\n");

   fprintf (Archivo_Latex, "\\clearpage\n");

   fprintf (Archivo_Latex, "\\begin{center}\n");
   fprintf (Archivo_Latex, "\\begin{longtable}{|l|l|l|}\n");
   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Nombre} & \\textbf{Versi\\'{o}n} & \\textbf{Respuestas} \\\\ \\hline\n");
   fprintf (Archivo_Latex, "\\endfirsthead\n");

   fprintf (Archivo_Latex, "\\hline\n");
   fprintf (Archivo_Latex, "\\textbf{Nombre} & \\textbf{Versi\\'{o}n} & \\textbf{Respuestas} \\\\ \\hline\n");
   fprintf (Archivo_Latex, "\\endhead\n");

   for (i=0; i<N_examenes; i++)
       {
	sprintf (hilera_antes,"%s & %s & \\small{\\texttt{%s}} \\\\ \\hline", 
		              PQgetvalue (res, i, 2), PQgetvalue (res, i, 3),PQgetvalue (res, i, 8));
        hilera_LATEX (hilera_antes, hilera_despues);
        fprintf (Archivo_Latex, "%s\n", hilera_despues);
       }

   fprintf (Archivo_Latex, "\\end{longtable}\n");
   fprintf (Archivo_Latex, "\\end{center}\n");

   fprintf (Archivo_Latex, "\\end{document}\n");
   fclose (Archivo_Latex);

   latex_2_pdf (&parametros, parametros.ruta_reportes, parametros.ruta_latex, "EX4000", 1, NULL, 0.0, 0.0, NULL, NULL);

   g_free(materia);
   g_free(descripcion);
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

void Update_background_color(long double nota)
{
#define N_COLORS 8
  
  GdkColor COLOR, color[N_COLORS];
  char label[200];
  long double alfa, beta;
  int BC[] = {1, 5, 10, 10, 5, 1};
  int i;

  alfa = 1 - nota;
  beta = nota;
  
  gdk_color_parse ("black",      &color [0]);
  gdk_color_parse ("dark red",   &color [1]);
  gdk_color_parse ("red",        &color [2]);
  gdk_color_parse ("yellow",     &color [3]);
  gdk_color_parse ("light cyan",      &color [4]);
  gdk_color_parse ("dodger blue",       &color [5]);
  gdk_color_parse ("green",      &color [6]);
  gdk_color_parse ("dark green",      &color [7]);
#if 0
  gdk_color_parse ("black",    &COLOR);
  for (i=0;i<N_COLORS;i++)
      {
       COLOR.red   += ( BC[i] * pow(alfa, N_COLORS - 1 - i) * pow(beta, i) * color[i].red);
       COLOR.green += ( BC[i] * pow(alfa, N_COLORS - 1 - i) * pow(beta, i) * color[i].green);
       COLOR.blue  += ( BC[i] * pow(alfa, N_COLORS - 1 - i) * pow(beta, i) * color[i].blue);
      }
#endif
   gtk_image_set_from_file (IM_smile1, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile2, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile3, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile4, ".imagenes/empty.png");
   gtk_image_set_from_file (IM_smile5, ".imagenes/empty.png");

  if (nota < 0.2)
     {
      beta = (nota - 0.0)/0.2;
      alfa  = 1 - beta;
      COLOR.red    = (alfa * color[0].red)   + (beta * color[1].red); 
      COLOR.green  = (alfa * color[0].green) + (beta * color[1].green); 
      COLOR.blue   = (alfa * color[0].blue)  + (beta * color[1].blue);
     }
  else if (nota < 0.4)
     {
      beta = (nota - 0.2)/0.2;
      alfa  = 1 - beta;
      COLOR.red    = (alfa * color[1].red)   + (beta * color[2].red); 
      COLOR.green  = (alfa * color[1].green) + (beta * color[2].green); 
      COLOR.blue   = (alfa * color[1].blue)  + (beta * color[2].blue);
     }
  else if (nota < 0.55)
     {
      beta = (nota - 0.4)/0.15;
      alfa  = 1 - beta;
      COLOR.red    = (alfa * color[2].red)   + (beta * color[3].red); 
      COLOR.green  = (alfa * color[2].green) + (beta * color[3].green); 
      COLOR.blue   = (alfa * color[2].blue)  + (beta * color[3].blue);
     }
  else if (nota < 0.675)
     {
      beta = (nota - 0.55)/0.125;
      alfa  = 1 - beta;
      COLOR.red    = (alfa * color[3].red)   + (beta * color[4].red); 
      COLOR.green  = (alfa * color[3].green) + (beta * color[4].green); 
      COLOR.blue   = (alfa * color[3].blue)  + (beta * color[4].blue);
     }
  else if (nota < 0.775)
     {
      
      beta = (nota - 0.675)/0.1;
      alfa  = 1 - beta;
      COLOR.red    = (alfa * color[4].red)   + (beta * color[5].red); 
      COLOR.green  = (alfa * color[4].green) + (beta * color[5].green); 
      COLOR.blue   = (alfa * color[4].blue)  + (beta * color[5].blue);
     }
  else if (nota < 0.9)
     {
      beta = (nota - 0.775)/0.125;
      alfa  = 1 - beta;
      COLOR.red    = (alfa * color[5].red)   + (beta * color[6].red); 
      COLOR.green  = (alfa * color[5].green) + (beta * color[6].green); 
      COLOR.blue   = (alfa * color[5].blue)  + (beta * color[6].blue);
     }
  else if (nota <= 1.0)
     {
      beta = (nota - 0.9)/0.1;
      alfa  = 1 - beta;
      COLOR.red    = (alfa * color[6].red)   + (beta * color[7].red); 
      COLOR.green  = (alfa * color[6].green) + (beta * color[7].green); 
      COLOR.blue   = (alfa * color[6].blue)  + (beta * color[7].blue);
     }

  if (nota >= 0.675) gtk_image_set_from_file (IM_smile1, ".imagenes/smile.png");
  if (nota >= 0.725) gtk_image_set_from_file (IM_smile2, ".imagenes/smile.png");
  if (nota >= 0.775) gtk_image_set_from_file (IM_smile3, ".imagenes/smile.png");
  if (nota >= 0.825) gtk_image_set_from_file (IM_smile4, ".imagenes/smile.png");
  if (nota >= 0.900) gtk_image_set_from_file (IM_smile5, ".imagenes/smile.png");
    
  gtk_widget_modify_bg(window1,  GTK_STATE_NORMAL,   &COLOR);
  
  if (nota < 1.0)
     sprintf (label,"<span font_desc=\"Arial 58\">%4.2Lf</span>", nota * 100.0);
  else
     sprintf (label,"<span font_desc=\"Arial 60\">100</span>");
    
  gtk_label_set_markup(LB_nota, label);

  gtk_widget_queue_draw(DA_ex);
}

void Update_dial()
{
   long double p, r, cos_r, sin_r;
   int x, y;
   cairo_t  *cr;
   cairo_surface_t *image; 
   
   p = Nota;
   r = (1.0 - p) * PI;
   cos_r = cos(r);
   sin_r = sin(r);

   image = cairo_image_surface_create_from_png(".imagenes/dialex.png");
     
   cr = gdk_cairo_create(DA_ex->window );
  
   //   cairo_set_source_rgb(cr, 0.45, 0.45, 0.45);
   cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
     
   cairo_rectangle(cr, 0, 0, DA_WIDTH, DA_HEIGHT);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);
   
   cairo_set_source_surface(cr, image, 7, DA_BEGIN);
   cairo_paint( cr );

   cairo_set_line_width (cr, 3);

// shadow
   cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);

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
   cairo_set_source_rgb(cr, 1.0, 0.5, 0.0);
      
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
void on_WN_ex4000_destroy (GtkWidget *widget, gpointer user_data) 
{
  Fin_Ventana (widget, user_data);
}

void on_BN_terminar_clicked(GtkWidget *widget, gpointer user_data)
{
  Fin_de_Programa (widget, user_data);
}

void on_EN_nombre_activate(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_grab_focus (EN_respuesta[0]);
}

void on_SC_versiones_value_changed(GtkWidget *widget, gpointer user_data)
{
  Cambia_Version ();
}

void on_EN_insert_text(GtkWidget *widget, gpointer user_data)
{
  int k;

  k = atoi (gtk_buildable_get_name (GTK_BUILDABLE (widget))+ 3) - 1;
  Cambia_Respuesta (k); 
}

void on_BN_undo_clicked(GtkWidget *widget, gpointer user_data)
{
   gtk_range_set_value (GTK_RANGE(SC_versiones),  (gdouble) 1.0);
   Init_Examen ();
}

void on_BN_up_clicked(GtkWidget *widget, gpointer user_data)
{
   Init_Respuestas ();
   Init_secuencia  ();
   gtk_widget_grab_focus (GTK_WIDGET(SP_secuencia));
}

void on_BN_save_clicked(GtkWidget *widget, gpointer user_data)
{
  Graba_Respuestas (widget, user_data);
}

void on_BN_print_clicked(GtkWidget *widget, gpointer user_data)
{
  Imprime_Reporte (widget, user_data);
}

void on_BN_delete_clicked(GtkWidget *widget, gpointer user_data)
{
  Borra_Respuesta (widget, user_data);
}

void on_SP_secuencia_activate(GtkWidget *widget, gpointer user_data)
{
  long int k;
  gchar * estudiante;

  if (!DESPLIEGUE_INICIAL)
     {
      estudiante = gtk_editable_get_chars(GTK_EDITABLE(SP_secuencia), 0, -1);
      k = atoi(estudiante);
      g_free(estudiante);
      gtk_spin_button_set_value (SP_secuencia, k);
      Cambia_secuencia ();
     }
}

void on_BN_ok_clicked(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (window2);
  gtk_widget_set_sensitive(window1, 1);
  gtk_widget_grab_focus (GTK_WIDGET(SP_examen));
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

void on_DA_ex_expose(GtkWidget *widget, gpointer user_data)
{
   Update_dial ();
}

