//
// Created by scott on 6/29/25.
//

// other variables are global to isolate the array in local
int sum;
int i;
int j;

int main() {
    int a[3][4];
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
//            _print(i);
//            _print(j);
            a[i][j] = 1;
        }
    }
    sum = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
//            _print(i);
//            _print(j);
            sum += a[i][j];
//            _print(sum);
        }
    }
//    _print(-256);
//    _print(sum);
    return sum;   // 20
}
