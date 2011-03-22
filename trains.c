/* This program is free software; you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation; either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program; if not, write to the Free Software */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, */
/* MA 02110-1301, USA. */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "SDL.h"	/*В будущем для графики*/

struct time
{
     char hour, minute;
};

struct way
{
     struct time departure_time, arrival_time;
     char departure_day, arrival_day; /* День недели: примем 1 для понедельника и 7 для воскресенья */
};

struct way map[30][30][24];
/* Отсюда и далее: в матрице map ось x - город, ИЗ которого едем */
/* Ось y - город, В который едем и ось z - время отправления/прибытия. */

struct cityname
{
     char name[32];
};

struct cityname city[30];
/* Выглядит криво, но на самом деле гораздо читабельнее. */

const int max_cities = 32;
int cities_present;

int fill_city(char cityfile_name[32])
{
     /* Возвращает число записей в массиве в случае удачного заполения оного  */
     /* (в т.ч. 0 если файл был пуст) или -1 при ошибке открытия */
     FILE *cityfile = fopen(cityfile_name,"r");
     if (cityfile == NULL) return -1; /* Словили ошибку */
     int i = 0;
     while (!feof(cityfile) && i <= max_cities)
     {
	  fscanf(cityfile,"%s\n",city[i].name);
	  i++;
     }
     return i;
}

int city_number(char city_name[32])
{
     /* Возвращает номер города в списке, если город объявлен, и -1, если не найден. */
     /* Вообще, ситуация , когда город не найден, возникать не должна.  */
     for (int i=0; i<=cities_present; i++)
     {
	  if (strcmp(sity_name,city[i].cityname) == 0) return i;
     }
     return -1;
}

int city_roads_load(char city_name[32])
{
     /* Вот тут стоит сказать пару слов о том, как в файле представляются пути. */
     /* Название файла - имя города, _ИЗ_ которого идут дороги */
     /* В пределах файла записи отсортированы по городу, _В_ который дороги ведут */
     /* далее записаны в следующих типах */
     /* char time char time, оно же */
     /* char char char char char char (если смотреть на типы в записи) */
     /* день | час отправления | минута отправления | день прибытия | час прибытия | минута прибытия */
}

int main ()
{
     cities_present = fill_city("city.txt");
     if (cities_present == -1)
     {
	  printf("Ошибка открытия файла.\n");
	  return 1;
     }
     if (cities_present != 0) 
     {
	  for (int i = 0; i<=cities_present ; i++) printf("%s\n",city[i].name);
	  printf("Всего городов - %i \n", cities_present);
     }
     else 
     {
	  printf("Ни один город не объявлен в файле. Некуда идти ._.\n");
	  return 0;
     }
}
