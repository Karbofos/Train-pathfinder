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

#define MAX_CITY_COUNT 30
#define STRING_LENGTH 32
#define CITY_MAX_WAYS 50
#define MAX_DIFFERENCE_TIME 1440
#define LIST_FILE "city.txt"

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
     struct times way[CITY_MAX_WAYS];
     int ways_present;
};

struct closest_road
{
     bool good;
     int number;
};

struct dfs_type
{
     bool washere;
     int camefrom;
};

struct dfs_type dfs_array[MAX_CITY_COUNT];

struct point map[MAX_CITY_COUNT][MAX_CITY_COUNT];
/* Отсюда и далее: в матрице map ось x - город, ИЗ которого едем */
/* Ось y - город, В который едем и ось z - время отправления/прибытия. */

struct cityname
{
     char name[STRING_LENGTH];
};

struct cityname city[MAX_CITY_COUNT];
/* Выглядит криво, но на самом деле гораздо читабельнее. */

int cities_present;

int fill_city(char cityfile_name[STRING_LENGTH])
{
     /* Возвращает число записей в массиве в случае удачного заполения оного  */
     /* (в т.ч. 0 если файл был пуст) или -1 при ошибке открытия */
     FILE *cityfile = fopen(cityfile_name,"r");
     if (cityfile == NULL) return -1; /* Словили ошибку */
     int i = 0;
     while (!feof(cityfile) && i <= MAX_CITY_COUNT)
     {
	  fscanf(cityfile,"%s\n",city[i].name);
	  i++;
     }
     fclose(cityfile);
     return i;
}

int city_number(char city_name[STRING_LENGTH])
{
     /* Возвращает номер города в списке, если город объявлен, и -1, если не найден. */
     /* Вообще, ситуация , когда город не найден, возникать не должна.  */
     for (int i=0; i<=cities_present; i++)
     {
	  if (strcmp(city_name,city[i].name) == 0) return i;
     }
     return -1;
}

int city_roads_load()
{
     /* Вот тут стоит сказать пару слов о том, как в файле представляются пути. */
     /* Название файла - имя города, _ИЗ_ которого идут дороги */
     /* В пределах файла записи отсортированы по городу, _В_ который дороги ведут */
     /* далее записаны в следующих типах */
     /* char time char time, оно же */
     /* char char char char char char (если смотреть на типы в записи) */
     /* день | час отправления | минута отправления | день прибытия | час прибытия | минута прибытия */
     /* Возвращает 0 в случае удачного открытия файла и наличия в нем записей; 1 если файл открыт, но записей нет и 2 при ошибке открытия */
     char name[STRING_LENGTH];
     char openfile[STRING_LENGTH + 10];
     int number, result;
     cities_present = fill_city(LIST_FILE);
     if (cities_present != -1)
     {
	  for (int i=0; i< MAX_CITY_COUNT; i++)
	       for (int j=0; j < MAX_CITY_COUNT; j++) map[i][j].ways_present=-1;
	  for (int i = 0; i< cities_present; i++)
	  {
	       sprintf(openfile,"%s/ways.txt",city[i].name);
	       FILE *cityfile = fopen(openfile,"r");
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
	  if (cities_present == 0) result = 1;
	  else result = 0;
     }
     else result = 2;
     return result;
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
	  if (best_difference_time <= MAX_DIFFERENCE_TIME) result.good = true;
	  else result.good = false;
     }
     return result;
}

void printstats_to_file (int final, bool good)
{
     /*Вывод в обратном порядке по указателям. Не очень ок.*/
     printf("Начинаем печать результата\n");
     FILE *print_file;
     char buf[100];
     int temp;
     if (good) print_file = fopen("good.txt","a");
     else print_file =fopen("bad.txt","a");
     temp = final;
     while (dfs_array[temp].camefrom != -1)
     {
	  sprintf(buf,"%s %i",buf, temp);
	  temp = dfs_array[temp].camefrom;
     }
     sprintf(buf,"%s %i",buf, temp);
     fprintf(print_file,"%s\n",buf);
     fclose(print_file);
}

int dfs (int now, int final, bool good, struct time arrival_time)
{
     printf("Входим в dfs\n");
     struct closest_road result;
     for (int i=0; i < cities_present; i++)
     {
	  result = find_closest(now, i, arrival_time);
	  if (!result.good) good = false;
	  if (map[now][i].ways_present != -1 && dfs_array[i].washere == false)
	  {
	       dfs_array[i].camefrom = now;
	       dfs_array[i].washere = true;
	       if (i == final) 
	       {
		    printstats_to_file(final, good);
	       }
	       else
	       {
		    dfs(i, final, good, map[now][i].way[result.number].arrival_time);
	       }
	       dfs_array[i].washere = false;
	       dfs_array[i].camefrom = -1;
	  }
     }     
     return 0;
}

int search (int from, int to, struct time departure_time)
{
     printf("Начинаем поиск\n");
     for (int i=0; i < cities_present; i++)
     {
	  dfs_array[i].washere = false;
	  dfs_array[i].camefrom = -1;
     }
     dfs_array[from].washere = true;
     dfs(from, to, true, departure_time);
     return 0;
}

int main ()
{
     char from[STRING_LENGTH], to[STRING_LENGTH];
     struct time asd;
     struct closest_road travel_result;
     if (city_roads_load() == 0)
     {
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
		    search(from_number, to_number, asd);
		    travel_result = find_closest(from_number, to_number, asd);
		    if (travel_result.number != -1) printf("Ближайший поезд отправляется в %i %i - %i\n", map[from_number][to_number].way[travel_result.number].departure_time.day,  map[from_number][to_number].way[travel_result.number].departure_time.hour,  map[from_number][to_number].way[travel_result.number].departure_time.minute);
		    else printf("Уехать вам не суждено\n");
	       }
	  else printf ("Не найден город отправления/прибытия\n");
     }
     else printf("Ошибка открытия файла-списка или загрузки путей.\n");
     return 0;
}
