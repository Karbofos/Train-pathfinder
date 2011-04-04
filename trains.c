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
/*#include "SDL.h"*/

struct time
{
     int hour, minute, day;
     /* День недели: примем 1 для понедельника и 7 для воскресенья */
};

struct times
{
     struct time departure_time, arrival_time;
};

struct point
{
     struct times way[50];
     int ways_present;
};

struct closest_road
{
     bool good;
     int number;
};

struct point map[30][30];
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
     fclose(cityfile);
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
     cities_present = fill_city("city.txt");
     for (int i=0; i<=30; i++)
	  for (int j=0; j<=30; j++) map[i][j].ways_present=-1;
     for (int i = 0; i< cities_present; i++)
     {
	  FILE *cityfile = fopen(city[i].name,"r");
	  if (cityfile != NULL)
	  {
	       while (!feof(cityfile))
	       {
		    fscanf(cityfile,"%s",name);
		    number = city_number(name);
		    if (number != -1 ) 
		    {
			 map[i][number].ways_present++;
			 int l = map[i][number].ways_present;
			 fscanf(cityfile,"%i", &map[i][number].way[l].departure_time.day);
			 fscanf(cityfile,"%i", &map[i][number].way[l].departure_time.hour);
			 fscanf(cityfile,"%i", &map[i][number].way[l].departure_time.minute);
			 fscanf(cityfile,"%i", &map[i][number].way[l].arrival_time.day);
			 fscanf(cityfile,"%i", &map[i][number].way[l].arrival_time.hour);
			 fscanf(cityfile,"%i", &map[i][number].way[l].arrival_time.minute);
		    }
		    else printf ("Город %s присутствует в файле путей города %s, но не объявлен в списке, пропускаем.", name, city[i].name);
	       }
	       fclose(cityfile);
	  }
	  else printf("Город %s объявлен в списке, но не имеет файла маршрутов. Может быть, из него никто никуда не уезжает?\n",city[i].name);
     }
}

struct closest_road find_closest(int from_city, int to_city, struct time from_time)
{
     struct closest_road result;
     /*Возвращает -1 в result.number, если дорога не определена; номер дороги в ином случае."Хорошие" дороги возвращает с result.good = TRUE. "Плохие" с FALSE.*/
     int arrival_minutes, departure_minutes, difference_time, best_difference_time;
     best_difference_time = 10080; /*Количество минут в неделе*/
     arrival_minutes = from_time.day*24*60 + from_time.hour*60 + from_time.minute;
     result.number = -1;
     for (int i=0;i<=map[from_city][to_city].ways_present;i++)
     {
	  departure_minutes=map[from_city][to_city].way[i].departure_time.day*24*60 + map[from_city][to_city].way[i].departure_time.hour*60 + map[from_city][to_city].way[i].departure_time.minute;
	  if (departure_minutes <= arrival_minutes) difference_time = 10080 - arrival_minutes + departure_minutes;
	  else difference_time = departure_minutes - arrival_minutes;
	  if (difference_time < best_difference_time)
	  {
	       best_difference_time = difference_time;
	       result.number = i;
	  }
     }
     if (map[from_city][to_city].ways_present != 0)
     {
	  /*Если разница между временем прибытия и временм отправления меньше суток, помечаем найденный путь как "хороший"*/
	  if (best_difference_time <= 1440) result.good = true;
	  else result.good = false;
     }
     return result;
}

int main ()
{
     city_roads_load();
     char from[32], to[32];
     struct time asd;
     struct closest_road travel_result;
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
     printf("Введите название города, из которого едем\n");
     scanf("%s", from);
     printf("Введите название города, в который едем\n");
     scanf("%s", to);
     if (city_number(from) != -1 && city_number(to) != -1)
	  {
	       printf("Введите желаемое время отправления: день (1-7), час (0 - 23) и минута (0-59)\n");
	       scanf("%i %i %i",&asd.day, &asd.hour, &asd.minute);
	       int from_number, to_number;
	       from_number = city_number(from);
	       to_number =  city_number(to);
	       travel_result = find_closest(from_number, to_number, asd);
	       if (travel_result.number != -1) printf("Ближайший поезд отправляется в %i %i - %i\n", map[from_number][to_number].way[travel_result.number].departure_time.day,  map[from_number][to_number].way[travel_result.number].departure_time.hour,  map[from_number][to_number].way[travel_result.number].departure_time.minute);
	       else printf("Уехать вам не суждено\n");
	  }
     else printf ("Не найден город отправления/прибытия\n");
     return 0;
}
