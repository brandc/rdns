/*The octet lengths of different supported IP versions*/
#define IPV4LEN  4
#define IPV6LEN 16

/*minimum and maximum lengths for ipv4 and ipv6 ips in string form*/
#define IPV4MINSTRLEN  7 /*strlen("0.0.0.0")                                       =  7*/
#define IPV4MAXSTRLEN 15 /*strlen("255.255.255.255")                               = 15*/

#define IPV6MINSTRLEN  2 /*strlen("::")                                            =  2*/
#define IPV6MAXSTRLEN 45 /*strlen("FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:255.255.255.255") = 45*/

typedef struct {
	/*AF_INET or AF_INET6*/
	int family;

	union {
		uint8_t ipv4[IPV4LEN];	/*The size of an ipv4 address is 32bits*/
		uint8_t ipv6[IPV6LEN];	/*The size of an ipv6 address is 128bits*/
	} ip_data;

	/*INET6_ADDRSTRLEN = longest possible ip with room for one null terminator*/
	char ipstr[INET6_ADDRSTRLEN];
}ip_t;

static size_t ip_to_ipv4(ip_t* ip)
{
	/*IPV4 integer to string*/
	return snprintf(ip->ipstr,
			/*longest possible ipv4 string with room for one null terminator*/
			INET_ADDRSTRLEN,
			"%u.%u.%u.%u",
			ip->ip_data.ipv4[0],
			ip->ip_data.ipv4[1],
			ip->ip_data.ipv4[2],
			ip->ip_data.ipv4[3]
	);
}

static uint32_t strn_to_uint32(char *num, size_t *n)
{
	char c;
	size_t i;
	uint32_t ret;

	ret = 0;
	for (i = 0; (i < (*n)) && (c = num[i]) ; i++) {

		if (c < '0' || c > '9')
			break;
		ret *= 10;
		ret += (c - '0');
	}

	*n = i;
	return ret;
}

static bool ipv4_to_buf(ip_t* ip_ctx)
{
	size_t i;
	size_t len;
	size_t octet;
	uint32_t raw_value;

	/*Sanity check of string size*/
	len = strnlen(ip_ctx->ipstr, sizeof(ip_ctx->ipstr));
	if (len < IPV4MINSTRLEN || len > IPV4MAXSTRLEN)
		return false;

	/*octets is set to IPV4LEN, the length of ipv4 address and must be enforced*/
	for (i = 0, octet = (IPV4LEN-1); (octet >= 0) && (i < sizeof(ip_ctx->ipstr)); i++, octet--) {
		/*Since 256 is the longest possible number for an ipv4 address*/
		len = 3;
		raw_value = strn_to_uint32(ip_ctx->ipstr+i, &len);
		/*no octet value may be greater than 255*/
		if (raw_value > 255)
			return false;
		/*enforce length requirements of ipv4 numbers between the dots*/
		else if (len > 3 || !len)
			return false;

		/*Add the new offset to the loop counter*/
		i   += len;

		/*enforce the ipv4 dots*/
		if (((i+1) < len) && (ip_ctx->ipstr[i+1] != '.'))
			return false;

		if (ip_ctx->family == AF_INET)
			ip_ctx->ip_data.ipv4[octet] = raw_value & 0xFF;
		/**************************************************************
		 * This is to support the alternative form of ipv6 addresses. *
		 *	specified in: rfc4291, section 2.2                    *
		 * 	example: ::AAAA:127.0.0.1                             *
		 **************************************************************/
		else if (ip_ctx->family == AF_INET6)
			ip_ctx->ip_data.ipv6[octet + 12] = raw_value & 0xFF;
	}

	return true;
}

static bool ipv6_to_buf(ip_t* ip_ctx)
{
	
}

static int guess_family(ip_t* ip_ctx)
{
	char c;
	size_t i;

	for (i = 0; (i < sizeof(ip_ctx->ipstr)) && (c = ip_ctx->ipstr[i]); i++) {
		if (c == ':')
			return AF_INET6;
		else if (c == '.')
			return AF_INET;
	}

	return -1;
}

bool ip_init(ip_t *ip_ctx, const char *ip, size_t iplen)
{
	/*INET6_ADDRSTRLEN = longest possible ip with a space for a null terminator*/
	if (iplen >= INET6_ADDRSTRLEN)
		return false;

	/*initialize memory*/
	memset(ip_ctx, 0, sizeof(ip_t));
	memcpy(&ip_ctx->ipstr, ip, iplen);

	ip_ctx->family = guess_family(ip_ctx);
	if (ip_ctx->family == -1)
		return false;

	if (ip_ctx->family == AF_INET) {
		if (!ipv4_to_buf(ip_ctx))
			return false;
	} else if (ip_ctx->family == AF_INET6) {
		if (!ipv6_to_buf(ip_ctx))
			return false;
	} else
		return false;

	return true;
}












