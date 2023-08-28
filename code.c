void f() {}



// TODO: function that takes an array???

int main()
{
	int a[4];

	int i = 0;
	while (i < 4)
	{
		a[i] = i * 2 + 1;
		i = i + 1;
	}
	i = 0;
	while (i < 4)
	{
		print *(a + i);
		i = i + 1;
	}
}
