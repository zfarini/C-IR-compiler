fn inc(int *a)
{
	*a = *a + 1;
}

fn swap(int *a, int *b)
{
	int t;
	t = *a;
	*a = *b;
	*b = t;
}

fn bad_swap(int a, int b)
{
	swap(&a, &b);
}

fn main()
{
	{
		int a;
		int b;

		a = 3;
		b = 5;

		swap(&a, &b);
		assert b == 3;
		assert a == 5;
		bad_swap(&a, &b);
		assert b == 3;
		assert a == 5;
	}
	{
		int a;
		int *b;
		int **c;
		int d;

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
	}
}
