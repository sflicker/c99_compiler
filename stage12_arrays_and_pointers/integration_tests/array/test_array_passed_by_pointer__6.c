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
//    _print(nums[0]);
//    _print(nums[1]);
//    _print(nums[2]);
//    int sum = nums[0] + nums[1] + nums[2];
//    _print(sum);
    return sum3(nums);       // 6
}
