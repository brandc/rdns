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

static uint32_t strn_to_uint32(char *num, size_t *n)
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
