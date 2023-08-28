int bad()
{
	assert 0;	
}

int main()
{

	assert (1 && 1);
	assert (1 && 0) == 0;
	assert  (0 && bad()) == 0;
	assert (0 && 0) == 0;

	assert (1 || 1);
	assert (0 || 0) == 0;
	assert (1 || bad()) == 1;
	assert (0 || 1) == 1;
}

