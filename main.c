#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
int main()
{
    srand(time(0));
    FILE *fp;
    char str[50],arr[10][50];
    int c = 0,i,r;
    fp = fopen("hang.txt","r");         //reading data from file
    while(!feof(fp))
    {
        fscanf(fp,"%s",str);
        strcpy(arr[c++],str);
    }
    fclose(fp);
    r = rand()%(c-0);                   //using random function to choose a random word from the dataset
    char temp[50];
    strcpy(temp,arr[r]);
    int chances = 3;                    //No. of chances in guessing a letter
    for(i = 0;temp[i]!='\0';i++)        //creating the random word with missing letters
    {
        int fg = rand()%(1-0+1);
        if(fg == 1)
        {
            temp[i] = '_';
        }
    }
    do
    {

        printf("%s\n",temp);
        printf("Enter a character ");
        fflush(stdin);
        char ch;
        int flag = 0;
        scanf("%c",&ch);
        for(i = 0;temp[i]!='\0';i++)
        {

            if(temp[i] == '_')
            {
                if(ch == arr[r][i])
                {
                    flag = 1;
                    temp[i] = ch;
                }
            }
        }
        if(strcmp(temp,arr[r]) == 0)
        {
            printf("You won\n ");
            break;
        }
        if(flag == 0)
            chances -= 1;
        //printf("%s\n",temp);
        if(chances == 0)
            printf("You lost!\n");
        else
            printf("chances left : %d\n",chances);
    }while(chances > 0);
    return 0;
}
