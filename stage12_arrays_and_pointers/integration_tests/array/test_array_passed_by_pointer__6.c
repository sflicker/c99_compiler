//
// Created by scott on 6/29/25.
//

int sum3(int *a) {
//    _print(a[0]);
//    _print(a[1]);
//    _print(a[2]);

    return a[0] + a[1] + a[2];
}

int main() {
    int nums[3] = {1, 2, 3};
    int sum = sum3(nums);
//    _print(sum);
    return sum;       // 6
}
