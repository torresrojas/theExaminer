/*******************************************/
/*  EXAMINER:                              */
/*                                         */
/*    Rutinas de uso general en            */
/*    The Examiner                         */
/*    Autor: Francisco J. Torres-Rojas     */        
/*******************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "examiner.h"
#include <time.h>

/* Prototipos locales */
void recupera_actualizacion        (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_beamer               (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_boletos              (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_configurado          (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_latex_2_pdf          (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_epstopdf             (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_despliega_pdf        (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_despliega_reportes   (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_escuela              (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_fecha_backup         (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_gnuplot              (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_xterm                (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_institucion          (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_instrucciones        (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_interaccion          (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_juramento            (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_programa             (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_ruta_backup          (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_ruta_examenes        (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_ruta_figuras         (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_ruta_reportes        (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_ruta_latex           (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_ruta_gnuplot         (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_usa_figuras          (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_paquetes             (struct PARAMETROS_EXAMINER * p, PGconn *conn);
void recupera_interaccion          (struct PARAMETROS_EXAMINER * p, PGconn *conn);

extern PGconn * EX_connect_data_base ()
{
  FILE * Config_file;
  int local;
  PGconn *DATABASE = NULL;
  char dbName [300]="EXAMENES";
  char comment[1024];
  int N_DB;
  char conninfo[1024] ="\0";


  if (existe_archivo (".conf/examiner.rc"))
     {
      Config_file = fopen (".conf/examiner.rc","r");

      fscanf(Config_file,"NUMBER OF DATABASES = %d\n", &N_DB);
      fscanf(Config_file,"DESCRIPTION = %[^\n]\n", comment);
      fscanf(Config_file,"LOCAL = %d\n", &local);

      if (local)
	 {
          fscanf(Config_file,"DATABASE = %[^\n]\n", dbName);
	  sprintf(conninfo,"dbname = '%s'", dbName);
	 }
      else
	 {
          fscanf(Config_file,"%[^\n]\n", conninfo);
	 }

      DATABASE = PQconnectdb(conninfo);

      fclose(Config_file);
     }
  else
     {
      DATABASE = PQsetdb(NULL, NULL, NULL, NULL, dbName);
     }

  return (DATABASE);
}

/*******************************************/
/* Intenta cargar parámetros desde base de */
/* datos, si algo no está lo inicializa en */
/* valores de default.                     */
/*******************************************/
extern void carga_parametros_EXAMINER (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char hilera [4096];

  /* Recupera indicador de configuración */
  recupera_configurado (p, DATABASE);

  /* Recupera forma de interaccion y decoración */
  recupera_interaccion (p, DATABASE);

  /* Recupera indicador de desplegar rutas de reportes */
  recupera_despliega_reportes (p, DATABASE);

  /* Recupera indicador de reportar actualizaciones */
  recupera_actualizacion (p, DATABASE);

  /* Recupera parámetros de asignación de boletos */
  recupera_boletos (p, DATABASE);

  /* Recupera comando para convertir LATEX en PDF */
  recupera_latex_2_pdf (p, DATABASE);

  /* Recupera comando para desplegar PDF */
  recupera_despliega_pdf (p, DATABASE);

  /* Recupera comando para convertir EPS a PDF */
  recupera_epstopdf (p, DATABASE);

  /* Recupera comando asociado a gnuplot */
  recupera_gnuplot (p, DATABASE);

  /* Recupera comando asociado a xterm */
  recupera_xterm (p, DATABASE);

  /* Recupera ruta de exámenes */
  recupera_ruta_examenes (p, DATABASE);

  /* Recupera indicador de uso de figuras */
  recupera_usa_figuras (p, DATABASE);

  /* Recupera ruta de figuras */
  recupera_ruta_figuras (p, DATABASE);

  /* Recupera ruta de reportes */
  recupera_ruta_reportes (p, DATABASE);

  /* Recupera ruta de fuentes de latex */
  recupera_ruta_latex (p, DATABASE);

  /* Recupera ruta de fuentes de gnuplot */
  recupera_ruta_gnuplot (p, DATABASE);

  /* Recupera ruta de respaldos */
  recupera_ruta_backup (p, DATABASE);

  /* Recupera nombre de institucion */
  recupera_institucion (p, DATABASE);

  /* Recupera nombre de escuela */
  recupera_escuela (p, DATABASE);

  /* Recupera nombre de programa */
  recupera_programa (p, DATABASE);

  /* Recupera instrucciones */
  recupera_instrucciones (p, DATABASE);

  /* Recupera juramento */
  recupera_juramento (p, DATABASE);

  /* Recupera paquetes de Latex */
  recupera_paquetes (p, DATABASE);

  /* Recupera parámetros de Beamer */
  recupera_beamer (p, DATABASE);

  /* Recupera fecha último respaldo */
  recupera_fecha_backup (p, DATABASE);
}

void recupera_boletos (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char hilera [4096];

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'BOLETOS'");
  if (PQntuples(res))
     {
      strcpy (hilera, PQgetvalue (res, 0, 1));
      sscanf (hilera, "BOLETOS_BASE=%d BOLETOS_EJERCICIO=%d MIN_BOLETOS=%d MAX_BOLETOS=%d MIN_P=%Lf MAX_P=%Lf BOLETOS_P_1=%d BOLETOS_P_2=%d MIN_RPB=%Lf MAX_RPB=%Lf BOLETOS_RPB_1=%d BOLETOS_RPB_2=%d MIN_DIAS=%d MAX_DIAS=%d BOLETOS_DIAS_1=%d BOLETOS_DIAS_2=%d MIN_USOS=%d MAX_USOS=%d BOLETOS_USOS_1=%d BOLETOS_USOS_2=%d B_DIFICULTAD=%d B_DISCRIMINACION=%d B_NOVEDAD=%d B_USOS=%d M_DIFICULTAD=%Lf M_DISCRIMINACION=%Lf M_NOVEDAD=%Lf M_USOS=%Lf",
	      &p->boletos_base,
	      &p->boletos_ejercicio,
	      &p->min_boletos, &p->max_boletos,
	      &p->min_p,    &p->max_p,    &p->boletos_p_1,    &p->boletos_p_2,
	      &p->min_Rpb,  &p->max_Rpb,  &p->boletos_Rpb_1,  &p->boletos_Rpb_2,
	      &p->min_dias, &p->max_dias, &p->boletos_dias_1, &p->boletos_dias_2,
	      &p->min_usos, &p->max_usos, &p->boletos_usos_1, &p->boletos_usos_2,
	      &p->b_dificultad, &p->b_discriminacion, &p->b_novedad, &p->b_usos,
	      &p->m_dificultad, &p->m_discriminacion, &p->m_novedad, &p->m_usos);
     }
  else
     {
      p->boletos_base     = 1;
      p->boletos_ejercicio= 0;
      p->min_boletos      = 0;
      p->max_boletos      = 20;
      p->min_p            = 0.0;
      p->max_p            = 1.0;
      p->boletos_p_1      = 0;
      p->boletos_p_2      = 0;
      p->min_Rpb          = 0.0;
      p->max_Rpb          = 1.0;
      p->boletos_Rpb_1    = 0;
      p->boletos_Rpb_2    = 0;
      p->min_dias         = 0;
      p->max_dias         = 730; /* 2 años */
      p->boletos_dias_1   = 0;
      p->boletos_dias_2   = 0;
      p->min_usos         = 0;
      p->max_usos         = 200; /* ¿10 grupos de 20 estudiantes? */
      p->boletos_usos_1   = 0;
      p->boletos_usos_2   = 0;
      p->b_dificultad     = 0;
      p->b_discriminacion = 0;
      p->b_novedad        = 0;
      p->b_usos           = 0;
      p->m_dificultad     = 0.0;
      p->m_discriminacion = 0.0;
      p->m_novedad        = 0.0;
      p->m_usos           = 0.0;
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('BOLETOS',E'BOLETOS_BASE=1 BOLETOS_EJERCICIO=0 MIN_BOLETOS=0 MAX_BOLETOS=50 MIN_P=0.0 MAX_P=1.0 BOLETOS_P_1=0 BOLETOS_P_2=0 MIN_RPB=0.0 MAX_RPB=1.0 BOLETOS_RPB_1=0 BOLETOS_RPB_2=0 MIN_DIAS=0 MAX_DIAS=730 BOLETOS_DIAS_1=0 BOLETOS_DIAS_2=0 MIN_USOS=0 MAX_USOS=200 BOLETOS_USOS_1=0 BOLETOS_USOS_2=0 B_DIFICULTAD=0 B_DISCRIMINACION=0 B_NOVEDAD=0 B_USOS=0 M_DIFICULTAD=0.0 M_DISCRIMINACION=0.0 M_NOVEDAD=0.0 M_USOS=0.0')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_interaccion (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char hilera [4096];

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'INTER'");
  if (PQntuples(res))
     {
      strcpy (hilera, PQgetvalue (res, 0, 1));
      sscanf (hilera, "PROGRAMA=%d DECORACION=%d",
	      &p->programa,
	      &p->decoracion);
     }
  else
     {
      p->programa   = 0;
      p->decoracion = 0;
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('INTER',E'PROGRAMA=0 DECORACION=0')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_despliega_reportes (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char hilera [4096];

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'DISPREP'");
  if (PQntuples(res))
     {
      p->despliega_reportes = atoi(PQgetvalue (res, 0, 1));
     }
  else
     {
      p->despliega_reportes = 1;
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('DISPREP','1')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_actualizacion (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char hilera [4096];

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'UPDATE'");
  if (PQntuples(res))
     {
      strcpy (hilera, PQgetvalue (res, 0, 1));
      sscanf (hilera, "REPORT_UPDATE=%d TIMEOUT=%d",
	      &p->report_update,
	      &p->timeout);
     }
  else
     {
      p->report_update = 1;
      p->timeout = 3;
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('UPDATE',E'REPORT_UPDATE=1 TIMEOUT=3')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_configurado (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'CONFIG'");
  if (PQntuples(res))
     {
      p->configurado = atoi(PQgetvalue (res, 0, 1));
     }
  else
     {
      p->configurado = 0;
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('CONFIG','0')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_latex_2_pdf (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'LATEXPDF'");
  if (PQntuples(res))
     {
      strcpy (p->latex_2_pdf, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->latex_2_pdf, "pdflatex");
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('LATEXPDF',E'pdflatex')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_despliega_pdf (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'DISPPDF'");
  if (PQntuples(res))
     {
      strcpy (p->despliega_pdf, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->despliega_pdf, "evince");
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('DISPPDF',E'evince')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_epstopdf (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'EPSTOPDF'");
  if (PQntuples(res))
     {
      strcpy (p->epstopdf, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->epstopdf, "epstopdf");
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('EPSTOPDF',E'epstopdf')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_gnuplot (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'GNUPLOT'");
  if (PQntuples(res))
     {
      strcpy (p->gnuplot, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->gnuplot, "gnuplot");
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('GNUPLOT',E'gnuplot')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_xterm (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'XTERM'");
  if (PQntuples(res))
     {
      strcpy (p->xterm, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->xterm, "xterm");
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('XTERM',E'xterm')");
      PQclear(res_aux);
     }

  PQclear(res);
}


void recupera_ruta_examenes (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char * ruta;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'EXAMEN'");
  if (PQntuples(res))
     {
      strcpy (p->ruta_examenes, PQgetvalue (res, 0, 1));
     }
  else
     {
      ruta = getcwd (NULL, 0);
      strcpy (p->ruta_examenes, ruta);
      free(ruta);

      sprintf (PG_command,"INSERT into PA_parametros values ('EXAMEN',E'%s')",p->ruta_examenes);
      res_aux = PQEXEC(DATABASE, PG_command);
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_usa_figuras (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'USAFIG'");
  if (PQntuples(res))
     {
      p->usa_figuras = atoi(PQgetvalue (res, 0, 1));
     }
  else
     {
      p->usa_figuras = 0;
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('USAFIG','0')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_ruta_figuras (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char * ruta;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'FIGURES'");
  if (PQntuples(res))
     {
      strcpy (p->ruta_figuras, PQgetvalue (res, 0, 1));
     }
  else
     {
      ruta = getcwd (NULL, 0);
      strcpy (p->ruta_figuras, ruta);
      free(ruta);

      sprintf (PG_command,"INSERT into PA_parametros values ('FIGURES',E'%s')",p->ruta_figuras);
      res = PQEXEC(DATABASE, PG_command);
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_ruta_reportes (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char * ruta;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'REPORTES'");
  if (PQntuples(res))
     {
      strcpy (p->ruta_reportes, PQgetvalue (res, 0, 1));
     }
  else
     {
      ruta = getcwd (NULL, 0);
      strcpy (p->ruta_reportes, ruta);
      free(ruta);

      sprintf (PG_command,"INSERT into PA_parametros values ('REPORTES',E'%s')",p->ruta_reportes);
      res_aux = PQEXEC(DATABASE, PG_command);
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_ruta_latex (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char * ruta;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'LATEX'");
  if (PQntuples(res))
     {
      strcpy (p->ruta_latex, PQgetvalue (res, 0, 1));
     }
  else
     {
      ruta = getcwd (NULL, 0);
      strcpy (p->ruta_latex, ruta);
      free(ruta);

      sprintf (PG_command,"INSERT into PA_parametros values ('LATEX',E'%s')",p->ruta_latex);
      res_aux = PQEXEC(DATABASE, PG_command);
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_ruta_gnuplot (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char * ruta;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'PATHGNUP'");
  if (PQntuples(res))
     {
      strcpy (p->ruta_gnuplot, PQgetvalue (res, 0, 1));
     }
  else
     {
      ruta = getcwd (NULL, 0);
      strcpy (p->ruta_gnuplot, ruta);
      free(ruta);

      sprintf (PG_command,"INSERT into PA_parametros values ('PATHGNUP',E'%s')",p->ruta_gnuplot);
      res_aux = PQEXEC(DATABASE, PG_command);
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_ruta_backup (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char * ruta;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'BACKUP'");
  if (PQntuples(res))
     {
      strcpy (p->ruta_backup, PQgetvalue (res, 0, 1));
     }
  else
     {
      ruta = getcwd (NULL, 0);
      strcpy (p->ruta_backup, ruta);
      free(ruta);

      sprintf (PG_command,"INSERT into PA_parametros values ('BACKUP',E'%s')",p->ruta_backup);
      res_aux = PQEXEC(DATABASE, PG_command);
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_institucion (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'INSTITU'");
  if (PQntuples(res))
     {
      strcpy (p->Institucion, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->Institucion, "Institución");
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('INSTITU',E'Institución')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_escuela (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'ESCUELA'");
  if (PQntuples(res))
     {
      strcpy (p->Escuela, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->Escuela, "Escuela");
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('ESCUELA',E'Escuela')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_programa (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'PROGRAM'");
  if (PQntuples(res))
     {
      strcpy (p->Programa, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->Programa, "Programa");
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('PROGRAM',E'Programa')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_instrucciones (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;
  char PG_command[5000];
  char * Instrucciones_default = "Sólo se tomará en cuenta lo que aparezca en la caja de respuestas arriba, independientemente de cualquier anotación hecha en el enunciado interno del examen. Use letras mayúsculas (\\textbf{A}, \\textbf{B}, \\textbf{C}, \\textbf{D}, \\textbf{E}) para contestar. Se considera como incorrecta cualquier casilla vacía, ilegible, ambigua o con una letra diferente a las permitidas. No hay penalidad adicional por dar una respuesta equivocada. Si desea cambiar respuestas ya escritas, táchelas y escriba las respuestas que desee a un lado de la caja, junto a una nota explicativa y su firma.";
  char * Instrucciones_corregidas;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'INSTRUC'");
  if (PQntuples(res))
     {
      strcpy (p->Instrucciones, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->Instrucciones, Instrucciones_default);

      Instrucciones_corregidas = (char *) malloc (sizeof(char) * strlen(Instrucciones_default)*2);
      hilera_POSTGRES (Instrucciones_default, Instrucciones_corregidas);
      sprintf (PG_command,"INSERT into PA_parametros values ('INSTRUC',E'%s')", Instrucciones_corregidas);
      res_aux = PQEXEC(DATABASE, PG_command);
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_juramento (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;
  char PG_command[5000];
  char * Juramento_default = "Las respuestas de este examen serán resultado de mis decisiones individuales. No usaré, recibiré, ni ofreceré ayuda no autorizada. No copiaré de otros exámenes, ni permitiré que nadie copie parte alguna de este examen. No realizaré ninguna trampa ni procedimiento deshonesto. \\textbf{Juro por mi honor que todo lo anterior es cierto}. ";

  char * Juramento_corregido;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'JURAM'");
  if (PQntuples(res))
     {
      strcpy (p->Juramento, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->Juramento, Juramento_default);

      Juramento_corregido = (char *) malloc (sizeof(char) * strlen(Juramento_default)*2);
      hilera_POSTGRES (Juramento_default, Juramento_corregido);
      sprintf (PG_command,"INSERT into PA_parametros values ('JURAM',E'%s')", Juramento_corregido);
      res_aux = PQEXEC(DATABASE, PG_command);
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_paquetes (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;
  char PG_command[5000];

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'PACKA'");
  if (PQntuples(res))
     {
      strcpy (p->Paquetes, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->Paquetes, " ");
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('PACKA',E' ')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_beamer (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  char PG_command [4000];
  PGresult *res, *res_aux;
  char hilera [4096];

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'BEAMER'");
  if (PQntuples(res))
     {
      strcpy (hilera, PQgetvalue (res, 0, 1));
      sscanf (hilera, "ESTILO=%d COLOR=%d FONT=%d SIZE=%d ASPECTO=%d",
	      &p->Beamer_Estilo,
	      &p->Beamer_Color,
	      &p->Beamer_Font,
	      &p->Beamer_Size,
	      &p->Beamer_Aspecto);
     }
  else
     {
      p->Beamer_Estilo  = 18;
      p->Beamer_Color   = 4;
      p->Beamer_Font    = 1;
      p->Beamer_Size    = 1;
      p->Beamer_Aspecto = 0;

      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('BEAMER',E'ESTILO=18 COLOR=4 FONT=1 SIZE=1 ASPECTO=0')");
      PQclear(res_aux);
     }

  PQclear(res);
}

void recupera_fecha_backup (struct PARAMETROS_EXAMINER * p, PGconn *DATABASE)
{
  PGresult *res, *res_aux;

  res = PQEXEC(DATABASE, "SELECT * from PA_parametros where codigo_parametro = 'DATEBUP'");
  if (PQntuples(res))
     {
      strcpy (p->fecha_respaldo, PQgetvalue (res, 0, 1));
     }
  else
     {
      strcpy (p->fecha_respaldo, "* NUNCA *");
      res_aux = PQEXEC(DATABASE, "INSERT into PA_parametros values ('DATEBUP',E'* NUNCA *')");
      PQclear(res_aux);
     }

  PQclear(res);
}

extern void EX_latex_packages (FILE * Archivo_Latex)
{
   fprintf (Archivo_Latex, "\\usepackage{framed}\n");
   fprintf (Archivo_Latex, "\\usepackage{longtable}\n");
   fprintf (Archivo_Latex, "\\usepackage{graphics}\n");
   fprintf (Archivo_Latex, "\\usepackage{graphicx}\n");
   fprintf (Archivo_Latex, "\\usepackage{epsfig}\n");
   fprintf (Archivo_Latex, "\\usepackage{wrapfig}\n");
   fprintf (Archivo_Latex, "\\usepackage{caption}\n");
   fprintf (Archivo_Latex, "\\usepackage{amsfonts}\n");
   fprintf (Archivo_Latex, "\\usepackage{enumerate}\n");
   fprintf (Archivo_Latex, "\\usepackage{multicol}\n");
   fprintf (Archivo_Latex, "\\usepackage{hhline}\n");
   fprintf (Archivo_Latex, "\\usepackage{float}\n");
   fprintf (Archivo_Latex, "\\usepackage[spanish,es-tabla]{babel}\n");
   fprintf (Archivo_Latex, "\\usepackage{amsmath, amsthm, amssymb}\n");
   fprintf (Archivo_Latex, "\\usepackage{multirow}\n");
   fprintf (Archivo_Latex, "\\usepackage{tabularx}\n");
   fprintf (Archivo_Latex, "\\ifx \\BEAMER \\undefined\n");
   fprintf (Archivo_Latex, "\\usepackage[table]{xcolor}\n");
   fprintf (Archivo_Latex, "\\fi\n");
   fprintf (Archivo_Latex, "\\usepackage{fancyhdr}\n");
   fprintf (Archivo_Latex, "\\usepackage{sidecap}\n");
   fprintf (Archivo_Latex, "\\usepackage{venndiagram}\n");
   fprintf (Archivo_Latex, "\\usepackage{tikz}\n");
   fprintf (Archivo_Latex, "\\usepackage{circuitikz}\n");
   fprintf (Archivo_Latex, "\\usepackage{listings}\n");
   fprintf (Archivo_Latex, "\\usepackage{mdframed}\n");
   fprintf (Archivo_Latex, "\\usepackage{datapie}\n");
   fprintf (Archivo_Latex, "\\usepackage{draftwatermark}\n");
   fprintf (Archivo_Latex, "\\renewcommand{\\thetable}{\\arabic{table}}\n");
}

extern void hilera_LATEX (unsigned char * antes, unsigned char * despues)
{
  while (*antes)
        {
	 if (*antes == 194)
	    {
	     antes++;
	     if (*antes == 191)
	       {
	        *despues++ = '?';
	        *despues++ = '`';
                antes++;
	       }
	    }
         else
            if (*antes == 195)
	       {
                antes++;
                switch (*antes)
	            {
		     case 129: /* Á */
                     *despues++ = '\\';
                     *despues++ = '\'';
                     *despues++ = 'A';
                     break;

		     case 137: /* É */
                     *despues++ = '\\';
                     *despues++ = '\'';
                     *despues++ = 'E';
                     break;

		     case 141: /* Í */
                     *despues++ = '\\';
                     *despues++ = '\'';
                     *despues++ = 'I';
                     break;

		     case 145: /* Ñ */
                     *despues++ = '\\';
                     *despues++ = '~';
                     *despues++ = 'N';
                     break;

		     case 147: /* Ó */
                     *despues++ = '\\';
                     *despues++ = '\'';
                     *despues++ = 'O';
                     break;

		     case 154: /* Ú */
                     *despues++ = '\\';
                     *despues++ = '\'';
                     *despues++ = 'U';
                     break;

		     case 161: /* á */
                     *despues++ = '\\';
                     *despues++ = '\'';
                     *despues++ = 'a';
                     break;

		     case 164: /* ä */
                     *despues++ = '\\';
                     *despues++ = '\"';
                     *despues++ = 'a';
                     break;

		     case 169: /* é */
                     *despues++ = '\\';
                     *despues++ = '\'';
                     *despues++ = 'e';
                     break;

		     case 173: /* í */
                     *despues++ = '\\';
                     *despues++ = '\'';
                     *despues++ = 'i';
                     break;

		     case 177: /* ñ */
                     *despues++ = '\\';
                     *despues++ = '~';
                     *despues++ = 'n';
                     break;

		     case 179: /* ó */
                     *despues++ = '\\';
                     *despues++ = '\'';
                     *despues++ = 'o';
                     break;

		     case 186: /* ú */
                     *despues++ = '\\';
                     *despues++ = '\'';
                     *despues++ = 'u';
                     break;

		     case 188: /* ü */
                     *despues++ = '\\';
                     *despues++ = '\"';
                     *despues++ = 'u';
                     break;

	            }
                antes++;
	       }
            else
               *despues++ = *(antes++); 
        }
  *despues = '\0';
}

extern void hilera_GNUPLOT (unsigned char * antes, unsigned char * despues)
{

  while (*antes)
        {
         if (*antes == ' ')
	    {
             if ((*(antes+1)==' ') || (*(antes+1)==',') || (*(antes+1)=='.') || (*(antes+1)=='}'))
                antes++;
             else
	        {
                 *despues++ = *(antes++);
	        }
	    }
         else
            if (*antes == 195)
	       {
                antes++;
                switch (*antes)
	            {
		     case 129: /* Á */
                     *despues++ = 0xC1;
                     break;

		     case 137: /* É */
                     *despues++ = 0xC9;
                     break;

		     case 141: /* Í */
                     *despues++ = 0xCD;
                     break;

		     case 145: /* Ñ */
                     *despues++ = 0xD1;
                     break;

		     case 147: /* Ó */
                     *despues++ = 0xD3;
                     break;

		     case 154: /* Ú */
                     *despues++ = 0xDA;
                     break;

		     case 161: /* á */
                     *despues++ = 0xE1;
                     break;

		     case 164: /* ä */
                     *despues++ = 0xE4;
                     break;

		     case 169: /* é */
                     *despues++ = 0xE9;
                     break;

		     case 173: /* í */
                     *despues++ = 0xED;
                     break;

		     case 177: /* ñ */
                     *despues++ = 0xF1;
                     break;

		     case 179: /* ó */
                     *despues++ = 0xF3;
                     break;

		     case 186: /* ú */
                     *despues++ = 0xFa;
                     break;

		     case 188: /* ü */
                     *despues++ = 0xFc;
                     break;

	            }
                antes++;
	       }
            else
               if (*antes == '&')
	          {
                   *despues++ = '\\';
                   *despues++ = *(antes++);
	          }
               else
                  *despues++ = *(antes++);
        }
  *despues = '\0';
}

extern void hilera_POSTGRES (unsigned char * antes, unsigned char * despues)
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

extern int existe_archivo(const char *fname)
{
  int result;
  FILE *file;

  result = 0;
  if (file = fopen(fname, "r"))
     {
      fclose(file);
      result = 1;
     }

  return (result);
}

extern int latex_2_pdf (struct PARAMETROS_EXAMINER * p, char * directorio_PDF, char * directorio_LATEX, char * reporte, int desplegar, GtkWidget *PB, long double base, long double max, char * return_ruta_reporte, char * return_ruta_latex)
{
  char ruta_reporte[3000];
  char ruta_latex  [3000];
  char comando[3000];
  int resultado;

  resultado = 1;

  sprintf (ruta_reporte,"\"%s/%s.pdf\"", directorio_PDF, reporte);
  if (return_ruta_reporte) strcpy(return_ruta_reporte, ruta_reporte);
  sprintf (ruta_latex,  "\"%s/%s.tex\"", directorio_LATEX, reporte);
  if (return_ruta_latex) strcpy(return_ruta_latex, ruta_latex);

  sprintf (comando,"rm %s", ruta_reporte);

  if (existe_archivo(ruta_reporte)) system (comando);
  if (PB)
     {
      gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB), base + 0.05*max);
      while (gtk_events_pending ()) gtk_main_iteration ();
     }

  sprintf (comando, "%s %s.tex > /dev/null", p->latex_2_pdf, reporte);
  system  (comando);
  if (PB)
     {
      gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB), base + 0.45*max);
      while (gtk_events_pending ()) gtk_main_iteration ();
     }

  sprintf (ruta_reporte, "%s.pdf", reporte);

  if (desplegar)
     {
      if (existe_archivo(ruta_reporte))
         {
	  system  (comando); /* Se ejecuta pdflatex 2 veces para que las tablas salgan perfectas */
          if (PB)
             {
              gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB), base + 0.85*max);
              while (gtk_events_pending ()) gtk_main_iteration ();
             }

          sprintf (comando, "rm %s.aux", reporte);
	  system  (comando);
          if (PB)
             {
              gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB), base + 0.88*max);
              while (gtk_events_pending ()) gtk_main_iteration ();
             }

          sprintf (comando, "rm %s.log", reporte);
	  system  (comando);
          if (PB)
             {
              gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB), base + 0.91*max);
              while (gtk_events_pending ()) gtk_main_iteration ();
             }

          sprintf (comando, "mv %s.tex \"%s\"", reporte, directorio_LATEX);
	  system  (comando);
          if (PB)
             {
              gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB), base + 0.94*max);
              while (gtk_events_pending ()) gtk_main_iteration ();
             }

          sprintf (comando, "mv %s.pdf \"%s\"", reporte, directorio_PDF);
	  system  (comando);
          if (PB)
             {
              gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB), base + 0.97*max);
              while (gtk_events_pending ()) gtk_main_iteration ();
             }

          sprintf (comando, "%s \"%s/%s.pdf\" &", p->despliega_pdf, directorio_PDF, reporte);
	  system  (comando);
          if (PB)
             {
              gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB), base + max);
              while (gtk_events_pending ()) gtk_main_iteration ();
             }
         }
      else
	 { /* No se generó PDF - error de sintaxis en LATEX o problemas con el comando usado para compilar LATEX */
	  resultado = 0;

          if (PB)
             {
              gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB), base + max);
              while (gtk_events_pending ()) gtk_main_iteration ();
             }
	  sprintf (comando, "%s -geometry -0+0 -T \"Error de Ejecución o Error de Sintaxis en LATEX\" -e \"%s %s.tex\"", 
		             p->xterm, p->latex_2_pdf, reporte);
	  system  (comando);

          sprintf (comando, "mv %s.tex \"%s\"", reporte, directorio_LATEX);
	  system  (comando);

          sprintf (comando, "rm %s.aux", reporte);
	  system  (comando);

          sprintf (comando, "rm %s.log", reporte);
	  system  (comando);

          sprintf (comando, "rm %s.pdf", reporte);
	  system  (comando);
         }
     }
  else
     { /* Solo revisar sintaxis */
      if (PB)
         {
          gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PB), base + max);
          while (gtk_events_pending ()) gtk_main_iteration ();
         }

      if (existe_archivo(ruta_reporte))
	 {
          sprintf (comando, "rm %s.tex", reporte);
	  system  (comando);

          sprintf (comando, "rm %s.aux", reporte);
	  system  (comando);

          sprintf (comando, "rm %s.log", reporte);
	  system  (comando);

          sprintf (comando, "rm %s.pdf", reporte);
	  system  (comando);
 	 }
      else
	 { /* No se generó PDF - error de sintaxis en LATEX o problemas con el comando usado para compilar LATEX */
	  resultado = 0;

          sprintf (comando, "mv %s.tex \"%s\"", reporte, directorio_LATEX);
	  system  (comando);

          sprintf (comando, "rm %s.aux", reporte);
	  system  (comando);

          sprintf (comando, "rm %s.log", reporte);
	  system  (comando);
	 }
     }

  return (resultado);
}

extern PGresult *PQEXEC(PGconn *DATABASE, const char *command)
{
  PGresult       * resultado;
  ExecStatusType   status;
  char           * status_string;
  char           * error_message;

  resultado = PQexec(DATABASE, command);
  status    = PQresultStatus(resultado);
  if ((status != PGRES_COMMAND_OK) && (status != PGRES_TUPLES_OK))
     {
      status_string = PQresStatus(status);
      printf("\n*=*=*=*=*=*=*=*=* %s *=*=*=*=*=*=*=*=*\n", status_string);
      printf("%s\n", command);
      error_message = PQresultErrorMessage(resultado);
      printf ("%s\n", error_message);
     }

  return(resultado);
}

extern void Main_decoration(GtkImage *IM_bg, char * color, int decoracion)
{
#define N_BACKGROUNDS 20
  struct {char * file; char * color;} Background [N_BACKGROUNDS]=
{{".imagenes/EX-BG-01.jpg","dark goldenrod"},
 {".imagenes/EX-BG-02.jpg","goldenrod"},
 {".imagenes/EX-BG-03.jpg","red"},
 {".imagenes/EX-BG-04.jpg","red"},
 {".imagenes/EX-BG-05.jpg","yellow"},
 {".imagenes/EX-BG-06.jpg","blue"},
 {".imagenes/EX-BG-07.jpg","yellow"},
 {".imagenes/EX-BG-08.jpg","blue"},
 {".imagenes/EX-BG-09.jpg","yellow"},
 {".imagenes/EX-BG-10.jpg","red"},
 {".imagenes/EX-BG-11.jpg","red"},
 {".imagenes/EX-BG-12.jpg","green"},
 {".imagenes/EX-BG-13.jpg","green"},
 {".imagenes/EX-BG-14.jpg","dark goldenrod"},
 {".imagenes/EX-BG-15.jpg","black"},
 {".imagenes/EX-BG-16.jpg","red"},
 {".imagenes/EX-BG-17.jpg","red"},
 {".imagenes/EX-BG-18.jpg","blue"},
 {".imagenes/EX-BG-19.jpg","white"},
 {".imagenes/EX-BG-20.jpg","light goldenrod"}};
  int k;

  if (decoracion == 0)
     k = (int) (N_BACKGROUNDS * (rand() / (RAND_MAX + 1.0))) % N_BACKGROUNDS;
  else
     k = decoracion - 1;

  gtk_image_set_from_file (IM_bg, Background[k].file);
  strcpy (color, Background[k].color);
}

extern void catNap(clock_t sec) // clock_t is a like typedef unsigned int clock_t. Use clock_t instead of integer in this context
{
  int i;

  clock_t start_time = clock();
  clock_t end_time = sec * 1000 * 1000 + start_time;
  while ((clock() < end_time))
        while (gtk_events_pending ()) gtk_main_iteration ();
} 

extern int days_between(int day1, int month1, int year1, int day2, int month2, int year2)
{
  int days;
  int Previous_days [12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334}; 
  int temp;

  if ((year1 > year2) || ((year1 == year2) && (month1 > month2)) || ((year1 == year2) && (month1 == month2) && (day1 > day2)))
     {
      temp   = year2;
      year2  = year1;
      year1  = temp;
      temp   = month2;
      month2 = month1;
      month1 = temp;
      temp   = day2;
      day2   = day1;
      day1   = temp;
     }

  days = ((year2 - year1) * 365 +
          (Previous_days[month2-1] - Previous_days[month1-1]) +
	  (day2 - day1));

  return(days);
}
