#include "9cc.h"
char *user_input;//入力プログラム
int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません。");
    return 1;
  }
  token = tokenize(argv[1]);
  user_input = argv[1];
  Node *node=expr();
  //アセンブリの前半部分の出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  
  //抽象構文木をいじいじ
  gen(node);

  //スタックトップに式全体の値が有るよ〜
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
