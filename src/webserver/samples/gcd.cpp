int notmain(int p, int q) {
	int temp;
	
	while (p != 0) {
		temp = p;
		p = q % p;
		q = temp;
	}
	
	return q;
}