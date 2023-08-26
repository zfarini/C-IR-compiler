void inc(int *a)
{
	*a = *a + 1;
}

void swap(int *a, int *b)
{
	int t;
	t = *a;
	*a = *b;
	*b = t;
}

void bad_swap(int a, int b)
{
	swap(&a, &b);
}

int main()
{
	{
		int a = 3, b = 5;

		swap(&a, &b);
		assert b == 3;
		assert a == 5;
		bad_swap(&a, &b);
		assert b == 3;
		assert a == 5;
	}
	{
		int a, *b, **c, d;

		a = 5;
		d = 9;
		assert &b == &a + 4;
		b = &a;
		assert *b == a;
		*b = 7;
		assert *b == a;
		assert a == 7;
		assert b == &a;
		c = &b;
		assert *c == &a;
		*c = &d;
		assert b == &d;
		assert **c == d;
		**c = 10;
		assert *b == d;
		assert **c == d;
		assert d == 10;

		//c = 0;
		//assert &(*c) == 0;
		//assert *(&a) == a;
		//assert *(&a + 4) == b;
		//b = &a;
		//assert **(&a + 4) == a;
	}
}

