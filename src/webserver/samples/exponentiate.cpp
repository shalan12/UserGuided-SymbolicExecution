int notmain(int base, int power) {
	int leftovers = 1;

	while (power > 1) {
		if (power % 2 == 0) {
			base *= base;
			power /= 2;
		} else {
			leftovers *= base;
			power -= 1;
		}
	}

	return base * leftovers;
}