#include <stdio.h>
#define N 3
char arr[3][30] = {"ceshi", "hello", "world"};

typedef char (*PET_ARR)[30];

int add(int a, int b) {
  return a + b;
}

int add1(int a) {
  int res = a + 1;
  return res;
}


int add2(int * a) {
  return  *a += 1;
}

typedef int (*PTR_ADD2)(int *);

typedef int (*PTR_ADD)(int, int);

int main() {
  char (*ptrArr)[30];
  ptrArr = arr;
  for (int i = 0; i < N; i++) {
    printf("第%d个是：%s\n", i, *(ptrArr + i));
  }
  printf("\n");
  PET_ARR ptr2;
  ptr2 = arr;

  for (int i = 0; i < N; i++) {
    printf("第%d个是：%s\n", i, *(ptr2 + i));
  }

  int a = 10, b = 20;
  printf("a + b = %d\n", add(a, b));

  PTR_ADD addFun;

  addFun = add;

  printf("a + b=%d\n", addFun(a, b));

  printf("a + 1 = %d\n", add1(a));
  printf("指针：a + 1 = %d\n", add2(&a));

  printf("a = %d\n", a);

  PTR_ADD2 addFun2;
  addFun2 = add2;
  printf("指针：a + 1 = %d\n", addFun2(&a));

  printf("a = %d\n", a);

  char s[] = "test";
  printf("地址：%p\n", s);
  printf("s[0]地址：%p\n", &s[0]);
  // s = "hello"; // 报错
  s[0] = 'h';
  printf("s = %s\n", s);
  printf("地址：%p\n", s);
  printf("s[0]地址：%p\n", &s[0]);

  return 0;
}