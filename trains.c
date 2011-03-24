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
     char hour, minute, day;
     /* День недели: примем 1 для понедельника и 7 для воскресенья */
};

struct times
{
     struct time departure_time, arrival_time;
};

struct city
{
     struct times way[50];
     int ways_present;
};

struct city map[30][30];
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
	  if (strcmp(city_name,city[i].name) == 0) return i;
     }
     return -1;
}

void city_roads_load()
{
     /* Вот тут стоит сказать пару слов о том, как в файле представляются пути. */
     /* Название файла - имя города, _ИЗ_ которого идут дороги */
     /* В пределах файла записи отсортированы по городу, _В_ который дороги ведут */
     /* далее записаны в следующих типах */
     /* char time char time, оно же */
     /* char char char char char char (если смотреть на типы в записи) */
     /* день | час отправления | минута отправления | день прибытия | час прибытия | минута прибытия */
     char name[32];
     int number;
     for (int i=0; i<=30; i++)
	  for (int j=0; j<=30; j++) map[i][j].ways_present=-1;
     for (int i = 0; i< cities_present; i++)
     {
	  FILE *cityfile = fopen(city[i].name,"r");
	  if (cityfile != NULL)
	  {
	       while (!feof(cityfile))
	       {
		    fscanf(cityfile,"%s\n",name);
		    number = city_number(name);
		    if (number != -1 ) 
		    {
			 map[i][number].ways_present++;
			 int l = map[i][number].ways_present;
			 fscanf(cityfile,"%i %i %i %i %i %i\n",map[i][number].way[l].departure_time.day, map[i][number].way[l].departure_time.hour, map[i][number].way[l].departure_time.minute, map[i][number].way[l].arrival_time.day, map[i][number].way[l].arrival_time.hour, map[i][number].way[l].arrival_time.minute);
		    }
		    else printf ("Город %s присутствует в файле путей города %s, но не объявлен в списке, пропускаем.", name, city[i].name);
	       }
	       fclose(cityfile);
	  }
	  else printf("Город %s объявлен в списке, но не имеет файла маршрутов. Может быть, из него никто никуда не уезжает?\n",city[i].name);
     }
}

int find_closest(int from_city, int to_city, struct time from_time)
{
     /* Вот тут проблема - функция должна не только возратить номер пути для массива map[from_city][to_city].way[], но и указать, попадает ли он в рамки 24х часов */
     /* 	  т.е. пометить его как "плохой" или "хороший". Наверное, сделаю через тип функции. */
     return 0;
}

int main ()
{
     cities_present = fill_city("city.txt");
     city_roads_load();
     char asd[32];
     if (cities_present == -1)
     {
	  printf("Ошибка открытия файла.\n");
	  return 1;
     }
     if (cities_present != 0) printf("Всего городов - %i \n", cities_present);
     else 
     {
	  printf("Ни один город не объявлен в файле. Некуда идти ._.\n");
	  return 0;
     }
     printf("Введите название искомого города\n");
     scanf("%s", asd);
     if (city_number(asd) != -1) printf("Город %s значится в массиве под номером %i\n",asd, city_number(asd));
     else printf("Город %s не найден в списке\n", asd);
}
