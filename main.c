#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <errno.h>

#include <netdb.h>
#include <sys/socket.h>

/*ip string stuff*/
#include "ip.c"

/*The resulting hostname will be returned in host and the length in hostlen*/
bool ipv4_to_hostname(const char* ip, char* host, size_t *hostlen)
{
	bool ret;
	int ai_ret;
	struct addrinfo hints, *results, *result;

	/*initialize variables*/
	result  = NULL;
	results = NULL;
	ret     = false;
	memset(&hints, 0, sizeof(hints));

	/*Set the hints properly*/
	hints.ai_family     = AF_INET;		/*IPV4 address*/
	hints.ai_protocol   = 0;		/*0 for autodetect*/
	hints.ai_flags      = 0;		/*Interesting flags in the man page*/
	hints.ai_canonname  = NULL;		/*More interesting stuff in the man page*/
	hints.ai_addr       = NULL;		/*current address*/
	hints.ai_next       = NULL;		/*next address*/

	/*prints error if there is one*/
	/*		     address,  port,  hints,  results*/
	ai_ret = getaddrinfo(ip,       NULL, &hints, &results);
	if (ai_ret) {
		fprintf(stderr, "\tgetaddrinfo: %s\n", gai_strerror(ai_ret));
		goto end;
	}

	/*Tries all results sequentially*/
	for (result = results; result != NULL; result = result->ai_next) {
		ai_ret = getnameinfo(result->ai_addr,		/*struct sockaddr*/
				     result->ai_addrlen,	/*socklen_t addrlen*/
				     host,			/*will hold the hostname*/
				     *hostlen,			/*length limit on hostname*/
				     NULL,			/*service, but it's not needed*/
				     0,				/*service length, but it's not needed*/
				     NI_NAMEREQD		/*error is set if hostname couldn't be found*/
			 );
		if (ai_ret == EAI_NONAME)
			continue;
		else if (ai_ret)
			fprintf(stderr, "\tgetnameinfo for %s: %s\n", ip, gai_strerror(ai_ret));
		else {
			*hostlen = strnlen(host, *hostlen);
			ret      = true;
			goto end;
		}
	}

	end:
		if (results)
			freeaddrinfo(results);
		fflush(stderr);
		return ret;
}

void rdns_range(uint32_t start, uint32_t end)
{
	uint32_t ipnum;
	size_t hostlen;
	char host[NI_MAXHOST];
	char ipaddr[NI_MAXHOST];

	if (start > end)
		return;

	ipnum = start;
	do {
		/*initialize buffers*/
		memset(&host,   0, sizeof(host));
		memset(&ipaddr, 0, sizeof(ipaddr));

		uint32_to_ipv4(ipaddr, sizeof(ipaddr), ipnum);

		hostlen = sizeof(host);
		if (ipv4_to_hostname(ipaddr, host, &hostlen))
			printf("%s -> %s\n", ipaddr, host);
		else
			printf("%s\n", ipaddr);
		fflush(stdout);

		if (ipnum != end)
			ipnum++;
		else
			break;
	} while (true);
}

void usage(void)
{
	fprintf(stderr, "-h prints this text\n");
	fprintf(stderr, "./rdns starting_ip [ending_ip]\n");
	exit(1);
}

#include <unistd.h>
int main(int argc, char *argv[])
{
	int opt;
	ip_t start, end;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		case 'h':
		default:
			usage();
		}
	}

	if (optind < argc) {
		if (!ip_init(&start, argv[optind], strlen(argv[optind]))) {
			fprintf(stderr, "Invalid starting ip!\n");
			exit(1);
		}
	} else
		usage();
	optind++;

	/*in case there isn't another ip, set it to look up the only one specified*/
	end = start;
	if (optind < argc)
		if (!ip_init(&end, argv[optind], strlen(argv[optind]))) {
			fprintf(stderr, "Invalid ending ip!\n");
			exit(1);
		}

	/*sanity check*/
	if (start > end) {
		fprintf(stderr, "Start must be less than or equal to end\n");
		usage();
		exit(1);
	}
	rdns_range(start, end);
	return 0;
}







