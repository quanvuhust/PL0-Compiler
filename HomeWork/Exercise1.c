#include <stdio.h>

#define N 2

const int DFA[5][N] = {{1, 0}, {1, 2}, {1, 3}, {1, 0}, {4, 4}};
int endOfLine = 0;

int toAddress(char c) {
    return c - 'a';
}

int isSpace(char c) {
    return c == ' ';
}

int check() {
    endOfLine = 0;
    int state = 0;
    char c;
    while(isSpace(c = getchar()));
    while(1) {
        if(c == '\n') {
            endOfLine = 1;
            break;
        }
        if(isSpace(c)) {
            break;
        }
        printf("%c", c);
        c = toAddress(c);
        if(c < 0 || c >= N) {
            state = 4;
        } else {
            state = DFA[state][c];
        }
        
        c = getchar();
    }
    return state;
}

int main() {
    int flag = 1;
    while(flag) {
        printf("Nhap lua chon: \n1.Bai tap 2\n2.Bai tap 3\n");
        int opt;
        fflush(stdin);
        scanf("%d", &opt);
        getchar();
        switch(opt) {
            case 1: {
                printf("Nhap xau: ");
                int state = check();
                if(state == 3) {
                    printf(": Xau duoc doan nhan.\n");
                } else if(state < 3){
                    printf(": Xau khong duoc doan nhan.\n");
                } else {
                    printf(": Khong phai xau tren tap (a, b).\n");
                }
                flag = 0;
                break;
            }
            case 2: {
                printf("Nhap xau: ");
                while(1) {
                    int state = check();
                    if(state == 3) {
                        printf(", yes\n");
                    } else if(state < 3){
                        printf(", no\n");
                    } else {
                        printf(": Khong phai xau tren tap (a, b).\n");
                    }
                    if(endOfLine == 1) {
                        break;
                    }
                }

                flag = 0;
                break;
            }
            default: {
                printf("No option\n");
            }
        }
    }

    return 0;
}
