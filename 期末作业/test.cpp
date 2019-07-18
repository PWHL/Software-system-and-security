#include <stdio.h>
#include <string.h>
#define PASSWORD "123456"
int Verify(char *password)
{
	int result;
	char buffer[8];
	result = strcmp(password, PASSWORD);
	strcpy(buffer, password);
	return result;
}
int main()
{
	int flag = 1;
	char pass[100];
	printf("please enter the password\n");
	while (true)
	{
		scanf("%s", pass);
		flag = Verify(pass);
		if (flag != 1)
		{
			printf("the password is correct!");
		}
		else
		{
			printf("the password is incorrect!");

		}
	}
}

