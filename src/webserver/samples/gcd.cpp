int notmain(int p, int q) {
	int temp;
	
	while (p != 0) {
		temp = p;
		p = q - (q / p) * p;
		q = temp;
	}
	
	return q;
}