#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <errno.h>

#include <netdb.h>
#include <sys/socket.h>

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

void uint32_to_ipv4(char *dest, size_t n, uint32_t ip)
{
	/*IPV4 integer to string*/
	snprintf(dest,
		 n,
		 "%u.%u.%u.%u",
		 (ip >> 24) & 0xFF,
		 (ip >> 16) & 0xFF,
		 (ip >>  8) & 0xFF,
		 (ip >>  0) & 0xFF
	);
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

		/*If a hostname is found for the given ip, "-> hostname" will be printed afterwards*/
		printf(ipaddr);

		hostlen = sizeof(host);
		if (ipv4_to_hostname(ipaddr, host, &hostlen)) {
			printf(" -> %s\n", host);
		} else
			putchar('\n');

		fflush(stdout);

		if (ipnum != end)
			ipnum++;
		else
			break;
	} while (true);
}

#include <unistd.h>

uint32_t strn_to_uint32(char *num, size_t *n)
{
	char c;
	size_t i;
	uint32_t ret;

	ret = 0;
	for (i = 0; i < (*n); i++) {
		c = num[i];

		if (c < '0' || c > '9')
			break;
		ret *= 10;
		ret += (c - '0');
	}

	*n = i;
	return ret;
}

bool ipv4_to_uint32(char* ip, size_t n, uint32_t* result)
{
	size_t i;
	size_t len;
	size_t octets;
	uint32_t ret, raw_value;

	/*shortest ipv4 address: 0.0.0.0 which is 7 bytes in length*/
	/*longest ipv4 address: 255.255.255.255 which is 15 bytes in length*/
	if (n < 7 || n > 15)
		return false;

	ret    = 0;
	octets = 0;
	for (i = 0; i < n; i++) {
		ret <<= 8;
		/*Since 256 is the longest possible number for an ipv4 address*/
		len = 3;
		raw_value = strn_to_uint32(ip+i, &len);
		/*no octet value may be greater than 255*/
		if (raw_value > 255)
			return false;
		/*enforces length requirements of ipv4 octets*/
		else if (len > 3 || !len)
			return false;

		i   += len;
		ret += raw_value;

		/*enforces the ipv4 dots*/
		if (((i+1) < len) && (ip[i+1] != '.'))
			return false;
		octets++;
	}

	/*an ipv4 address must be exactly four octets in length*/
	if (octets != 4)
		return false;

	*result = ret;
	return true;
}

void usage(void)
{
	fprintf(stderr, "-h prints this text\n");
	fprintf(stderr, "./rdns starting_ip [ending_ip]\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int opt;
	uint32_t start, end;

	end       = 0;
	start     = 0;
	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		case 'h':
		default:
			usage();
		}
	}

	if (optind < argc) {
		if (!ipv4_to_uint32(argv[optind], strlen(argv[optind]), &start)) {
			fprintf(stderr, "Invalid starting ip!\n");
			exit(1);
		}
	} else
		usage();
	optind++;

	/*in case there isn't another ip, set it to look up the only one specified*/
	end = start;
	if (optind < argc)
		if (!ipv4_to_uint32(argv[optind], strlen(argv[optind]), &end)) {
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







