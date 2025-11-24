#include <stdio.h>

int main(){
    float N1, N2, Result;
    int O = 0;

    //=================================================================================//
    printf("=============== CALCULATOR ===============\n");
    while(1){
        do{
            printf("\nChoose an operator:\n");
            printf("\t[1] Addition\n");
            printf("\t[2] Subtraction\n");
            printf("\t[3] Multiplication\n");
            printf("\t[4] Division\n");
            printf("\t[5] Exit\n");
            scanf("%d", &O);
        } while (O < 1 || O > 5);

        if(O == 5){
            printf("Exiting calculator...\n");
            break;
        }

    //=================================================================================//
        printf("Enter first number: ");
        scanf("%f", &N1);
        printf("Enter second number: ");
        scanf("%f", &N2);
        printf("------------------------------------------\n");

    //=================================================================================//
        switch(O){
            case 1:
                Result = N1 + N2;
                printf("The result of the addition is: %.2f\n", Result);
                break;
        //---------------------------------------------------------------------------------//
            case 2:
                Result = N1 - N2;
                printf("The result of the subtraction is: %.2f\n", Result);
                break;
        //---------------------------------------------------------------------------------//
            case 3:
                Result = N1 * N2;
                printf("The result of the multiplication is: %.2f\n", Result);
                break;
        //---------------------------------------------------------------------------------//
            case 4:
                if(N2 != 0){
                    Result = N1 / N2;
                    printf("The result of the division is: %.2f\n", Result);
                } else{
                    printf("ERROR: Cannot divide by zero\n");
                }
                break;
        }
    }

    return 0;
}
