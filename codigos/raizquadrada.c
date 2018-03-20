int input(void)
{
}

void output(int out)
{
}

int raiz(int n)
{
    int start;
    int end;
    int mid;
    int ans;
    int x;

    if (n == 0)
        return n;
    if (n == 1)
      return n;

    start = 1;
    end = n / 2;
    while (start <= end)
    {
        mid = start + ((end - start) / 2);
        x = mid * mid;
        if (x == n)
            return mid;
        if (x < n)
        {
            start = mid + 1;
            ans = mid;
        }
        else
            end = mid - 1;
    }
    return ans;
}

void main(void)
{
    int number;
    number = input();
	output(number);
	/*number = 16;*/
    number = raiz(number);
    output(number);
}
