#include <stdlib.h>
#include <stdio.h>
struct k;
struct k {
  struct k *next;
  double val;
  int com;
};
int main() {
  struct k *a = calloc(1,sizeof(struct k));
  a->val = 1.0;
  a->com = 12;
  printf("%d\n", a->com);
  printf("%f\n", a->val);
}
