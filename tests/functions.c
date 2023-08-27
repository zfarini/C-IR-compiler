int add(int a, int b)
{
	return a + b;
}

int mul2(int a)
{
	return add(a, a);
}

int factorial(int n)
{
	if (n <= 1)
		return 1;
	return factorial(n - 1) * n;
}

int fib(int n)
{
	if (n <= 1)
		return n;
	return fib(n - 1) + fib(n - 2);
}

int fib_itr(int n)
{
	int a = 0;
	int b = 1;
	int c;
    
	while (n)
	{
		c = a + b;
		a = b;
		b = c;
		n = n - 1;
	}
	return a;
}

int test(int a, int b, int c)
{
	int d;
	int e;
	int f;
	int	r;
    
	r = 5;
	d = a;
	e = b;
	f = c;
	fib_itr(10);
	fib(10);
	factorial(10);
	assert r == 5;
	assert a == d;
	assert b == e;
	assert f == c;
}

void main()
{
	assert add(1, 2) == 3;
	assert add(add(1, 2), add(1, 2)) == 6;
	assert add(add(1, add(1, 1)), add(add(0, 1), 4)) == 8;
	assert mul2(5) == 10;
	assert mul2(mul2(add(mul2(5), 4))) == (2 * 5 + 4) * 2 * 2;
	assert factorial(5) == 120;
	assert factorial(0) == 1;
	{
		int i = 0;
		while (i <= 20)
		{
			assert fib_itr(i) == fib(i);
			i = i + 1;
		}
	}
	assert fib(fib(7)) == fib_itr(fib_itr(7));
    
	test(add(1, 2), 3, 4);
}
