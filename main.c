#include "9cc.h"
char *user_input;//入力プログラム
int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません。");
    return 1;
  }
  locals=malloc(sizeof(LVar));
  user_input = argv[1];
  token=tokenize();
  program();
  //アセンブリの前半部分の出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  
  //プロローグ
  //変数26こぶんの領域確保
  printf("	push rbp\n");
  printf("	mov rbp,rsp\n");
  printf("	sub rsp,208\n");
  //抽象構文木をいじいじ
  for(int i=0;code[i];i++){
	  gen(code[i]);
	  //式の評価結果としてスタックに一つ値が残っているので、スタックが溢れないようにpop
	  printf("	pop rax\n");
  }

  //スタックトップに式全体の値が有るよ〜
  printf("	mov rsp,rbp\n");
  printf("	pop rbp\n");
  printf("  ret\n");
  return 0;
}
