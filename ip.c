/*The octet lengths of different supported IP versions*/
static const size_t IPV4LEN =  4;
static const size_t IPV6LEN = 16;

typedef struct {
	/*AF_INET or AF_INET6*/
	int family;

	union {
		uint8_t ipv4[IPV4LEN];	/*The size of an ipv4 address is 32bits*/
		uint8_t ipv6[IPV6LEN];	/*The size of an ipv6 address is 128bits*/
	} ip_data;

	/*INET6_ADDRSTRLEN = longest possible ip with a space for a null terminator*/
	char ipstr[INET6_ADDRSTRLEN];
}ip_t;

static void uint32_to_ipv4(char *dest, size_t n, uint32_t ip)
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
	size_t octets;
	uint32_t raw_value;

	len = strnlen(ip_ctx->ipstr, sizeof(ip_ctx->ipstr));
	/*shortest ipv4 address: 0.0.0.0 which is 7 bytes in length*/
	/*longest ipv4 address: 255.255.255.255 which is 15 bytes in length*/
	if (len <= 7 || len >= 15)
		return false;

	/*octets is set to IPV4LEN, the length of ipv4 address and must be enforced*/
	for (i = 0, octets = IPV4LEN; octets && (i < sizeof(ip_ctx->ipstr)); i++, octets--) {
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
		if (((i+1) < len) && (ip[i+1] != '.'))
			return false;

		if (ip_ctx->family == AF_INET)
			ip_ctx->ip_data.ipv4[octets] = raw_value;
		/**************************************************************
		 * This is to support the alternative form of ipv6 addresses. *
		 *	specified in: rfc4291, section 2.2                    *
		 * 	example: ::AAAA:127.0.0.1                             *
		 **************************************************************/
		else if (ip_ctx->family == AF_INET6)
			ip_ctx->ip_data.ipv6[octets + 12] = raw_value;
	}

	/*octet length check*/
	if (octets != -1)
		return false;

	return true;
}

static bool ipv6_to_buf(ip_t* ip_ctx)
{
	
}




static int fast_detect_family(ip_t* ip_ctx)
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

	ip_ctx->family = fast_detect_family(ip_ctx);
	if (ip_ctx->family == -1)
		return false;

	if (ip_ctx->family == AF_INET) {
		if (!ipv4_to_buf(ip_ctx)) {
			fprintf(stderr, "\tInvalid ipv4 address\n");
			return false;
		}
	} else if (ip_ctx->family == AF_INET6) {
		if (!ipv6_to_buf(ip_ctx)) {
			fprintf(stderr, "\tInvalid ipv6 address\n");
			return false;
		}
	} else
		return false;

	return true;
}












