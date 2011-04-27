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
#include "SDL.h"
#include "SDL_image.h"

#define MAX_CITY_COUNT 30
#define STRING_LENGTH 32
#define CITY_MAX_WAYS 50
#define MAX_DIFFERENCE_TIME 24
#define LIST_FILE "data/city.txt"
#define MAP_IMAGE_PATH "data/map.jpg"

struct time
{
     int year, month, day, hour;
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
     int way_number;
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

void ShowImage(char image_path[100], SDL_Surface *screen_local, int x, int y)
{
     SDL_Surface *image;
     SDL_Rect dest;
     
     image = IMG_Load(image_path);
     if ( image == NULL ) 
     {
	  fprintf(stderr, "Невозможно загрузить %s: %s\n", image_path, SDL_GetError());
	  return;
     }
     else
     {
	  dest.x = x;
	  dest.y = y;
	  dest.w = image->w;
	  dest.h = image->h;
	  SDL_BlitSurface(image, NULL, screen_local, &dest);
	  SDL_UpdateRects(screen_local, 1, &dest);
     }
}

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
     int number, result, l;
     cities_present = fill_city(LIST_FILE);
     if (cities_present != -1)
     {
	  for (int i=0; i< MAX_CITY_COUNT; i++)
	       for (int j=0; j < MAX_CITY_COUNT; j++) map[i][j].ways_present=-1;
	  for (int i = 0; i< cities_present; i++)
	  {
	       sprintf(openfile,"data/%s/ways.txt",city[i].name);
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
			      l = map[i][number].ways_present;
			      fscanf(cityfile,"%i", &map[i][number].way[l].departure_time.year);
			      fscanf(cityfile,"%i", &map[i][number].way[l].departure_time.month);
			      fscanf(cityfile,"%i", &map[i][number].way[l].departure_time.day);
			      fscanf(cityfile,"%i", &map[i][number].way[l].departure_time.hour);
			      fscanf(cityfile,"%i", &map[i][number].way[l].arrival_time.year);
			      fscanf(cityfile,"%i", &map[i][number].way[l].arrival_time.month);
			      fscanf(cityfile,"%i", &map[i][number].way[l].arrival_time.day);
			      fscanf(cityfile,"%i", &map[i][number].way[l].arrival_time.hour);
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
     int difference_time, best_difference_time;
     if (map[from_city][to_city].ways_present != -1)
     {
	  best_difference_time = (map[from_city][to_city].way[0].departure_time.year - from_time.year)*365*24 + (map[from_city][to_city].way[0].departure_time.month - from_time.month)*30*24 + (map[from_city][to_city].way[0].departure_time.day - from_time.day)*24 +  (map[from_city][to_city].way[0].departure_time.hour - from_time.hour);
	  result.number = 0;
	  for (int i=1;i<=map[from_city][to_city].ways_present;i++)
	  {
	       difference_time = (map[from_city][to_city].way[i].departure_time.year - from_time.year)*365*24 + (map[from_city][to_city].way[i].departure_time.month - from_time.month)*30*24 + (map[from_city][to_city].way[i].departure_time.day - from_time.day)*24 +  (map[from_city][to_city].way[i].departure_time.hour - from_time.hour);
	       if (difference_time < best_difference_time && difference_time > 0)
	       {
		    best_difference_time = difference_time;
		    result.number = i;
	       }
	       else
	       {
		    if (difference_time > 0 && best_difference_time < 0) best_difference_time = difference_time;
	       }
	  }
	  /*Если разница между временем прибытия и временем отправления меньше указаной константы, помечаем найденный путь как "хороший"*/
	  if (best_difference_time < 0) result.number = -1;
	  if (best_difference_time <= MAX_DIFFERENCE_TIME) result.good = true;
	  else result.good = false;
     }
     else 
     {
	  result.number = -1;
	  result.good = false;
     }
     return result;
}

void printstats_to_file (int final, bool good)
{
     FILE *print_file;
     int temp, count;
     if (good) print_file = fopen("good.txt","a");
     else print_file =fopen("bad.txt","a");
     count = 0;
     temp = final;
     while (dfs_array[temp].camefrom != -1) 
     {
	  count++;
	  temp = dfs_array[temp].camefrom;
     }
     do
     {
	  temp = final;
     	  for (int i = 0; i < count; i++) temp = dfs_array[temp].camefrom;
     	  fprintf(print_file,"%i ", temp);
	  count--;
     }while (count != 0);
     fprintf(print_file,"%i -1\n", final);
     fclose(print_file);
}

int dfs (int now, int final, bool good, struct time arrival_time)
{
     struct closest_road result;
     for (int i=0; i < cities_present; i++)
     {
	  if (map[now][i].ways_present != -1 && dfs_array[i].washere == false && i != now)
	  {
	       dfs_array[i].camefrom = now;
	       dfs_array[i].washere = true;
	       dfs_array[i].way_number = result.number;
	       result = find_closest(now, i, arrival_time);
	       if (result.number != -1)
	       {
		    if (i == final)
		    {
			 if (!result.good) printstats_to_file(final, false); 
			 else printstats_to_file(final, good);
		    }
		    else
		    {
			 if (!result.good) dfs(i, final, false, map[now][i].way[result.number].arrival_time);
			 else dfs(i, final, good, map[now][i].way[result.number].arrival_time);
		    }
	       }
	       dfs_array[i].washere = false;
	       dfs_array[i].camefrom = -1;
	       dfs_array[i].way_number = -1;
	  }
     }     
     return 0;
}

int draw_way(char *source_file_name, int max_ways)
{
     int asd;
     FILE *source_file;
     do
     {
	  printf("Введите номер пути, который вы хотите просмотреть или 0 для выхода:\n");
	  scanf("%i", &asd);
	  if (asd > 0 && asd <= max_ways)
	  {
	       if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
	       {
		    fprintf(stderr, "Не могу инициализировать SDL: %s\n", SDL_GetError());
		    exit(1);
	       }
	       SDL_Surface *screen;
	       screen = SDL_SetVideoMode(800, 600, 24, SDL_SWSURFACE);
	       if ( screen == NULL )
	       {
		    fprintf(stderr, "Невозможно установить разрешение 800x600: %s\n", SDL_GetError());
		    exit(1);
	       }
	       ShowImage(MAP_IMAGE_PATH, screen, 0, 0);
	       atexit(SDL_Quit);

	       source_file = fopen(source_file_name, "r");
	       /*Не проверяем наличие файла, он был. Точно был. Проверяли в процедуре, из которой вызывали.*/
	       int s, prev;
	       char name[100];
	       
	       for (int i=1; i<asd; i++)
	       {
		    do
		    {
			 fscanf(source_file,"%i", &s);
		    }while(s != -1);
		    /*Вот тут обработка изображения*/
	       }

	       fscanf(source_file, "%i", &s);
	       sprintf(name,"data/%s/%s.png",city[s].name, city[s].name);
	       ShowImage(name, screen, 0, 0);
	       prev = s;
	       fscanf(source_file, "%i", &s);
	       while(s != -1)
	       {
		    sprintf(name,"data/%s/%s.png", city[prev].name, city[s].name);
		    ShowImage(name, screen, 0, 0);
		    sprintf(name,"data/%s/%s.png", city[s].name, city[s].name);
		    ShowImage(name, screen, 0, 0);
		    prev = s;
		    fscanf(source_file, "%i", &s);
	       }
	       scanf("%i", &s);
	       SDL_Quit();
	  }
	  else if (asd != 0) printf("Нет пути с таким номером!\n");
     }while(asd != 0);
     return 0;
}

int search (int from, int to, struct time departure_time)
{
     FILE *good_file, *bad_file;
     int ways_count, point;
     for (int i=0; i < cities_present; i++)
     {
	  dfs_array[i].washere = false;
	  dfs_array[i].camefrom = -1;
	  dfs_array[i].way_number = -1;
     }
     dfs_array[from].washere = true;
     dfs(from, to, true, departure_time);
     
     /*Начало вывода списка*/
     good_file = fopen("good.txt", "r");
     bad_file = fopen("bad.txt", "r");
     ways_count = 0;
     if (good_file != NULL)
     {
	  printf("Найдены маршруты, ожидание на станциях между которыми меньше 24 часов:\n");
	  fscanf(good_file,"%i", &point);
	  while (!feof(good_file))
	  {
	       ways_count++;
	       if (!feof(good_file))  printf("%i)%s ", ways_count, city[point].name);
	       fscanf(good_file,"%i", &point);
	       while (point != -1)
	       {
		    printf("=> %s ", city[point].name);
		    fscanf(good_file,"%i", &point);
	       }
	       fscanf(good_file,"%i", &point);
	       printf("\n");
	  }
	  fclose(good_file);
	  draw_way("good.txt",ways_count);
     }
     else 
     {
	  if (bad_file != NULL)
	  {
	       printf("Оптимальных маршрутов не найдено, но есть несколько других:\n");
	       printf("(Оптимальным маршрутом считается маршрут, ожидание на станциях между которым менее 24х часов)\n");
	       fscanf(bad_file,"%i", &point);
	       while (!feof(bad_file))
	       {
		    ways_count++;
		    printf("%i)%s ",ways_count, city[point].name);
		    fscanf(bad_file,"%i", &point);
		    while (point != -1)
		    {
			 printf("=> %s ", city[point].name);
			 fscanf(bad_file,"%i", &point);
		    }
		    fscanf(bad_file,"%i", &point);
		    printf("\n");
	       }
	       fclose(bad_file);
	       draw_way("bad.txt", ways_count);
	  }
	  else printf("Не найдено ни одного маршрута.\nВозможно, все поезда уже уехали?\n");
     }
     remove("bad.txt");
     remove("good.txt");
     return 0;
}

int main ()
{
     char from[STRING_LENGTH], to[STRING_LENGTH];
     struct time asd;
     int select;
     if (city_roads_load() == 0)
     {
	  do
	  {
	       printf("Выберите опцию:\n[1]Ввести искомые города вручную\n[2]Выбрать города из списка\n[0]Выход\n");
	       scanf("%i", &select);
	       switch (select)
	       {
	       case 1:
	       {
		    printf("Введите название города, из которого едем\n");
		    scanf("%s", from);
		    printf("Введите название города, в который едем\n");
		    scanf("%s", to);
		    if (city_number(from) != -1 && city_number(to) != -1)
		    {
			 printf("Введите желаемое время отправления(год месяц день час):\n");
			 scanf("%i %i %i %i", &asd.year, &asd.month, &asd.day, &asd.hour);
			 int from_number, to_number;
			 from_number = city_number(from);
			 to_number =  city_number(to);
			 search(from_number, to_number, asd);
		    }
		    else printf ("Не найден город отправления/прибытия\n");
		    break;
	       }
	       case 2:
	       {
		    int from_number, to_number;
		    printf("Выберите исходный город:\n");
		    for (int i=1; i<= cities_present; i++)
		    {
			 printf("[%i]%s\n",i,city[i-1].name);
		    }
		    printf("[0]Выход\n");
		    scanf("%i", &from_number);
		    if (from_number > 0 && from_number <= cities_present)
		    {
			 printf("Выберите конечный город:\n");
			 for (int i=1; i<= cities_present; i++)
			 {
			      printf("[%i]%s\n",i,city[i-1].name);
			 }
			 printf("[0]Выход\n");
			 scanf("%i", &to_number);
			 if (to_number > 0 && to_number <= cities_present)
			 {
			      if (from_number != to_number)
			      {
				   printf("Введите желаемое время отправления(год месяц день час):\n");
				   scanf("%i %i %i %i", &asd.year, &asd.month, &asd.day, &asd.hour);
				   search(from_number -1, to_number -1, asd);
			      }
			      else printf("Нельзя указывать один и тот же город отправления/прибытия!\n");
			 }
			 else if (to_number < 0 || to_number > cities_present) printf("Введены некорректные данные!\n");
		    }
		    else if (from_number < 0 || from_number > cities_present) printf("Введены некорректные данные!\n");
		    break;
	       }
	       case 0:
	       {
		    printf("До свидания!\n");
		    break;
	       }
	       default:
	       {
		    printf("Неверная опция\n");
		    break;			 
	       }
	       }
	  }while (select != 0);
     }
     else printf("Ошибка открытия файла-списка или загрузки путей.\n");
     return 0;
}
