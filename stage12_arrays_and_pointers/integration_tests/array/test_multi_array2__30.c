//
// Created by scott on 6/29/25.
//
int main() {
    int a[5][4];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            a[i][j] = i;
        }
    }
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            sum += a[i][j];
        }
    }
    return sum;   // 20
}
