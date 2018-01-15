/*
Compilar con:

cc -o EX EX.c EX_utilities.c -I/usr/include/postgresql `pkg-config --cflags --libs gtk+-2.0` -L/usr/lib/postgresql/9.1/lib -lpq -export-dynamic
*/
/*******************************************/
/*  EX:                                    */
/*                                         */
/*    Programa Principal                   */
/*    The Examiner 0.4                     */
/*    26 de Enero 2014                     */
/*    Autor: Francisco J. Torres-Rojas     */
/*-----------------------------------------*/
#include <libpq-fe.h>
#include <gtk/gtk.h>
#include <locale.h>
#include "examiner.h"

/******************/
/* Postgres stuff */
/******************/
PGconn	 *DATABASE;

/***************************/
/* Globales y Definiciones */
/***************************/
struct PARAMETROS_EXAMINER parametros;

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
      setlocale (LC_NUMERIC, "en_US.UTF-8");
      carga_parametros_EXAMINER (&parametros, DATABASE);
      PQfinish(DATABASE);

      if      (parametros.programa == 0)
              system ("bin/EX-A &");  /* El programa es ejecutado como un proceso independiente */
      else if (parametros.programa == 1)
              system ("bin/EX-T &");  /* El programa es ejecutado como un proceso independiente */
      else if (parametros.programa == 2)
              system ("bin/EX-Z &");  /* El programa es ejecutado como un proceso independiente */
      else if (parametros.programa == 3)
              system ("bin/EX-O &");  /* El programa es ejecutado como un proceso independiente */
     }

   return 0;
}

