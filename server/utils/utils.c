#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <time.h>
#include <ctype.h>

#include "utils.h"

#define HEAD "HEAD "
#define GET "GET "

#define NUM_MONTHS 12
#define NUM_WKDAYS 7

const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char *wkdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

/**
 * Parses the request, should be of form
 * <command> <resource id> HTTP/1.1\r\nHost: <hostname>[\r\nIf-Modified-Since: <time-interval>]\r\n\r\n
 * Which if printed:
 *
 * begin
 * <command> <resource id> HTTP/1.1
 * Host: <hostname>
 * [If-Modified-Since: <time-interval>]
 *
 * end
 * @method parse_request
 * @param  request       request sent by the client
 * @return
 */
char *parse_request(char *request, char **time_interval, int *header_only)
{
  int i, j;
  if (strncmp(request, HEAD, strlen(HEAD)) == 0)
  {
    (*header_only) = 1;
    i = strlen(HEAD);
  }
  else if (strncmp(request, GET, strlen(GET)) == 0)
  {
    (*header_only) = 0;
    i = strlen(GET);
  }
  else return NULL;

  char *resource_id = calloc(strlen(request), sizeof(char));
  char c;
  for (j = 0; (c = request[i]) != ' '; i++, j++) resource_id[j] = c;

  resource_id[j] = '\0';

  char *line = strtok(request, "\r\n");
  line = strtok(NULL, "\r\n");

  // if there was a last modified
  if ((line = strtok(NULL, "\r\n\r\n"))) (*time_interval) = strdup(line);
  else (*time_interval) = NULL;

  return resource_id;
}

/**
 * Converts a month (represented as first 3 letters of the month with first character
 * capitalized) to its integer version [0, 11].
 * @method month_to_num
 * @param  month        String month to convert
 * @return              -1 if not recognized month, otherwise between 0 and 11
 */
int month_to_num(char *month)
{
  for (int i = 0; i < NUM_MONTHS; i++)
  {
    if (strcmp(months[i], month) == 0)
      return i;
  }

  return -1;
}

/**
 * Converts the day of the week (represented as the first 3 letters of the month with
 * first character capitalized) to its integer version [0, 6]
 * @method wkday_to_num
 * @param  wkday        String weekday to convert
 * @return              -1 if not recognized day of the week, otherwise between 0 and 6,
 *                      0 being sunday
 */
int wkday_to_num(char *wkday)
{
  for (int i = 0; i < NUM_WKDAYS; i++)
  {
    if (strcmp(wkdays[i], wkday) == 0)
      return i;
  }

  return -1;
}

int string_to_int(char *string)
{
  char c;
  int num = 0;
  for (int i = 0; (c = string[i]) != '\0'; i++)
  {
    if (!isdigit(c)) return -1;
    num *= 10;
    num += c - '0';
  }

  return num;
}

/**
 * Parses the date requested of the form
 *
 * If-Modified-Since: <Www> <Mmm> <dd> <hh>:<mm>:<ss> <yyyy>
 *
 * Into a time_t object
 *
 * @method parse_date
 * @param  date       the date to parse of the above form
 * @return            a time_t object based off the input date
 */
time_t *parse_date(char *date)
{
  struct tm *d = malloc(sizeof(struct tm));

  strtok(date, " "); // shave off 'If-Modified-Since: '
  char *wkday = strtok(NULL, " ");
  char *month = strtok(NULL, " ");
  char *day   = strtok(NULL, " ");
  char *hour  = strtok(NULL, ":");
  char *min   = strtok(NULL, ":");
  char *sec   = strtok(NULL, " ");
  char *year  = strtok(NULL, "\0");

  if ((d->tm_wday = wkday_to_num(wkday)) == -1) return NULL;
  if ((d->tm_mon  = month_to_num(month)) == -1) return NULL;
  d->tm_mday = atoi(day);
  d->tm_hour = atoi(hour);
  d->tm_min = atoi(min);
  d->tm_sec = atoi(sec);
  d->tm_year = atoi(year) - 1900;

  time_t *t = malloc(sizeof(time_t));
  *t = mktime(d);
  return t;
}
