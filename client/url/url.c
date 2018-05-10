#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <ctype.h>

#include "url.h"

#define HTTP "http://"
#define DEFAULT_PORT 80

// prints the proper format for a URL
#define print_format() printf("Invalid URL format, should be http://<hostanme>[:portnumber]<resource_id>\n")

/**
 * Parses the hostname from the url input, the host is between http:// and the port number
 * or the resource id, so start after http:// and end at a semi colon or a /. start is
 * equal to the index of the next ':' or '/' at the end
 * @method parse_hostname
 * @param  input          the user input
 * @param  start          the destination to start from (after http://)
 * @return                the hostname of the url
 */
char *parse_hostname(char *input, int *start)
{
  char *host_name = calloc(strlen(input), sizeof(char));
  int i, j;
  char c;
  for (i = (*start), j = 0; (c = input[i]) != '\0' && c != '/' && c != ':'; i++, j++)
    host_name[j] = c;
  host_name[j++] = '\0';

  (*start) = i;
  return host_name;
}

/**
 * Parses the user input to find the port number, starts at the first ':' after http:// and
 * ends at the next '/'. start equals the index of the next '/' after the function has completed
 * @method parse_portnumber
 * @param  input            the user input with the url
 * @param  start            the index of the first ':' after http://
 * @return                  the port number
 */
int parse_portnumber(char *input, int *start)
{
  int portnumber = 0;
  int i;
  char c;
  for (i = (*start); (c = input[i]) != '\0' && c != '/'; i++)
  {
    if (!isdigit(c))
    {
      printf("Port number should be a digit\n");
      return -1;
    }
    portnumber *= 10;
    portnumber += c - '0';
  }

  (*start) = i;
  return portnumber;
}

/**
 * Parses the user input to find the resource id, after the hostname and port number
 * starting with '/'.
 * @method parse_resource
 * @param  input          the input from the user containing the url
 * @param  start          the first '/' after http://
 * @return                returns the resource identification
 */
char *parse_resource(char *input, int *start)
{
  char *resource = calloc(strlen(input), sizeof(char));
  int i, j;
  char c;
  for (i = (*start), j = 0; (c = input[i]) != '\0'; i++, j++)
    resource[j] = c;
  resource[j++] = '\0';

  (*start) = i;
  return resource;
}

/**
 * Creates the url object created from the input of the format
 * http://<hostname>[:<portnumber>]<resource identifier>
 *
 * If no portnumber, then the portnumber is defaulted to PORT
 * @method parse_url
 * @param  url       The url to enter the hostname, portnumber, and resource identifier
 * @param  input     The url string the user inputted
 * @return           0 if error, 1 otherwise
 */
int parse_url(url_t *url, char *input)
{
  if (strncmp(input, HTTP, strlen(HTTP)) != 0)
  {
    print_format();
    return 0;
  }

  int i = strlen(HTTP);
  if (!(url->hostname = parse_hostname(input, &i))) return 0;

  if (input[i] == ':')
  {
    i++;
    if ((url->portnumber = parse_portnumber(input, &i)) == -1)
    {
      free(url->hostname);
      return 0;
    }
  }

  if (input[i] != '/')
  {
    print_format();
    return 0;
  }

  if (!(url->resource_id = parse_resource(input, &i)))
  {
    print_format();
    free(url->hostname);
    return 0;
  }

  return 1;
}

/**
 * Initializes the url and parses the user input to create the url
 * @method init_url
 * @param  url      the struct to initialize
 * @param  input    url the user inputted
 * @return          0 if error, 1 otherwise
 */
int init_url(url_t *url, char *input)
{
  url->portnumber = DEFAULT_PORT;
  if (!parse_url(url, input))
    return 0;
  return 1;
}

/**
 * Destroys and cleans up the memory used by the url
 * @method destroy_url
 * @param  url         url to free
 */
void destroy_url(url_t *url)
{
  if (!url) return;

  free(url->hostname);
  free(url->resource_id);
  free(url);
  url = NULL;
}

/**
 * prints the url in the url format,
 * http://<hostname><portnumber><resource identifier>
 * @method print_url
 * @param  url       url to print
 */
void print_url(const url_t url)
{
  printf("%s%s:%d%s\n", HTTP, url.hostname, url.portnumber, url.resource_id);
}
