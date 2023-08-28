#include <stdio.h>
#include <stdlib.h>

int *func(short arr[5])
{
	printf("%u\n", sizeof(arr));
	return arr;
}

int main()
{
	int a[7];
	func(a);
}
