#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "utils.h"

#define HEADER_OPTION "-h"
#define TIME_OPTION "-d"

/**
 * Prints the proper format a user should use when starting this program
 * @method print_input_format
 */
#define print_input_format() printf("Invalid input: should be ./client [–h] [–d <time-interval>] <URL>\n")

/**
 * parses the time interval after -d to find the if modified since time the user
 * is requesting
 * @method parse_interval
 * @param  input          the string containing the days, hours, and minutes ago for if modified since
 *                        in the format <days>:<hours>:<minutes>
 * @return                the time interval, or NULL if an error occurred
 */
char *parse_interval(char *input)
{
  int date[] = { 0, 0, 0 }; // { days, hours, minutes }
  int j = 0;
  char c;
  for (int i = 0; (c = input[i]) != '\0'; i++)
  {
    if (c == ':')
    {
      if (++j == 3)
      {
        printf("Invalid input, date should be of the form <day>:<hour>:<minute>\n");
        return NULL;
      }
      continue;
    }
    else if (!isdigit(c))
    {
      printf("Invalid input, date should only contain 3 numbers and 2 semicolons\n");
      return NULL;
    }
    date[j] *= 10;
    date[j] += c - '0';
  }

  char *s_time = calloc(30, sizeof(char));
  time_t n_time = time(0);
  n_time -= (date[0] * 24 * 3600 + date[1] * 3600 + date[2] * 60);
  strcpy(s_time, ctime(&n_time));

  return ctime(&n_time);
}

/**
 * Parses the user input from argv. User input should be of the format
 *   ./client [–h] [–d <time-interval>] <URL>
 * where -h denotes to request header only, -d denotes to use an if-modified-since the
 * time interval entered, and the url is the url to request from
 * @method parse_input
 * @param  argc          number of args in argv
 * @param  argv          array of strings containing the arguments entered
 * @param  url           the url struct to enter url information into
 * @param  header_only   1 if -h is used, 0 otherwise
 * @param  time_interval the string to enter the parsed time interval into if the user has entered
 *                       a time interval after -d
 * @return               0 if an error occurred, 1 otherwise
 */
int parse_input(int argc, char **argv, url_t *url, int *header_only, char **time_interval)
{
  int url_l, time_l; // url location and time location in the argv array
  url_l = time_l = 0;
  (*header_only) = 0; // header only option

  if (argc == 1)
  {
    printf("Missing URL\n");
    return 0;
  }
  else if (argc == 2) url_l = 1;
  else if (argc == 3)
  {
    if (strcmp(argv[1], HEADER_OPTION) != 0)
    {
      print_input_format();
      return 0;
    }
    (*header_only) = 1;
    url_l = 2;
  }
  else if (argc == 4)
  {
    if (strcmp(argv[1], TIME_OPTION) != 0)
    {
      print_input_format();
      return 0;
    }
    time_l = 2;
    url_l = 3;
  }
  else if (argc == 5)
  {
    if (strcmp(argv[1], TIME_OPTION) == 0 && strcmp(argv[3], HEADER_OPTION) == 0) time_l = 2;
    else if (strcmp(argv[1], HEADER_OPTION) == 0 && strcmp(argv[2], TIME_OPTION) == 0) time_l = 3;
    else
    {
      print_input_format();
      return 0;
    }
    url_l = 4;
    (*header_only) = 1;
  }

  if (!init_url(url, argv[url_l])) return 0;
  if (time_l && !((*time_interval) = parse_interval(argv[time_l]))) return 0;

  return 1;
}
