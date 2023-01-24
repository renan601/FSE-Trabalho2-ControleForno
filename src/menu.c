#include <stdio.h>
#include "astruct.h"
#include "pid.h"
 
void define_constants(Oven *ovenItem) {
    printf("Controle Forno\n\n");
    printf("Defina os valores das constantes de PID\n");

    printf("Insira o valor da constante Kp\n");
    scanf("%f", &ovenItem->Kp);
    printf("Insira o valor da constante Ki\n");
    scanf("%f", &ovenItem->Ki);
    printf("Insira o valor da constante Kd\n");
    scanf("%f", &ovenItem->Kd);

    pid_configura_constantes(ovenItem->Kp, ovenItem->Ki, ovenItem->Kd);
}

int define_working_mode() {
    int option;
    while (true) {
        printf("Defina o modo de funcionamento\n");
        printf("0 - Temperatura de referência definida no dashboard\n");
        printf("1 - Temperatura de referência definida na planilha (TIVE PROBLEMA PRA CONTABILIZAR O TEMPO DE EXECUÇÂO E NÂO CONSEGUI IMPLEMENTAR)\n");
        printf("2 - Temperatura de referência definida pelo terminal\n");
        scanf("%d",&option);

        switch (option) {
            case 0:
                return option;
                break;
            case 2:
                return option;
                break;
            default:
                printf("Opção inexistente. Insira novamente\n");
                break;
        }
    
    }

    return 0;
}

//void* menu_control_oven(void *vargp) {}