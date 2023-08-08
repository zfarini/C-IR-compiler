int fib(int n, int m)
{
	if (n <= 1)
		return n;
	return fib(n - 1, m) + fib(n - 2, m) + m;
}

#include <stdio.h>

int main()
{
	printf("%d\n", fib(fib(5, 3), fib(3, 1) + fib(2, 2)) + fib(fib(fib(2, 3), 4), fib(fib(1, 1), 1)));
}
