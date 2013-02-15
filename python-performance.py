print "2"
for i in range(3, 10001):
	prime = True
	for j in range(2, i+1):
		k = int(i/j);
		l = k*j;
		if l == i:
			prime = False
	if prime:
		print i