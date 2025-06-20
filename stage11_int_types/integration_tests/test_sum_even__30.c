int main() {
    int sum = 0;
    for (int i=0;i<=10;i+=1) {
        _print(i);
        if (i % 2 == 0) {
            sum += i;
        }
        _print(sum);
    }
    return sum;	 // expected 30 = (0+2+4+6+8+10)
} 
