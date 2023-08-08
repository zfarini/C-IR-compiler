int fib(int n)
{
	if (n <= 1)
		return n;
	return fib(n - 1) + fib(n - 2);
}

#include <stdio.h>

int main()
{
	printf("%d\n", fib(fib(fib(2) + fib(3 + fib(2))) + fib(fib(4))));
}
