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
}

