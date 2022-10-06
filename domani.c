#include <stdio.h>

void main(){

    int g,m,a;
    printf("Inserisci il giorno, mese e anno\n");
    scanf("%d%d%d", &g, &m, &a);
    if((a%4==0 && a%100!=0) || a%400)

        if(g<29) printf("%d / %d / %d\n", g++, m, a);
        else if(g==29) printf("%d / %d / %d\n", g-29, ++m, a);

    else 
        if(g<28) printf("%d / %d / %d\n", g-29, m, a);
        else if(g==28) printf("%d / %d / %d\n", g-27, ++m, a);
        
    else if(m==4 || m== 6 || m==9 || m==11)

        if(g<30) printf("%d / %d / %d\n", g++, m, a);
        else printf("%d / %d / %d\n", g-29, ++m, a);

    else
        if (m==12)
            if(g<31) printf("%d / %d / %d\n", g++, m, a);
            if(g==31) printf("%d / %d / %d\n", g-30, m-11, a++);
        
    
    else
        if(g<31) printf("%d / %d / %d\n", g++, m, a);
        else if (g==31) printf("%d / %d / %d\n", g-30, ++m, a); 
}    
