#include<stdio.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>
//とーくんの種類だよ〜
typedef enum {
  TK_RESERVED,  //記号
  TK_NUM,       //整数トークン
  TK_EOF,       //入力の終わりを表すトークン
} TokenKind;
typedef struct Token Token;
//うえの定義の中身
struct Token {
  TokenKind kind;//トークンの型
  Token *next;
  int val;//kindがTK_NUMの時、その数値
  char *str;//トークン文字列
};
//現在着目nowのトークン
Token *token;

//エラー報告関数
//printfと同じ引数
void error(char *fmt, ...) { 
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
//次のトークンが期待している記号のときには、トークンを1強み勧めて
//真を返す。それ以外は義を返す
bool consume(char op){
  if(token->kind!=TK_RESERVED||token->str[0]!=op){
    return false;
  }
  token = token->next;
  return true;
}
//次のトークンが期待している記号のときには、トークンを1強み勧めて
//真を返す。それ以外はエラーを報告
void expect(char op){
  if(token->kind!=TK_RESERVED||token->str[0]!=op){
    error("'%c'じゃないよー", op);
  }
  token = token->next;
  
}
//次のトークンが数値の時、トークンを一つ読み進めて、その数値を返す
//それ以外はエラーを返す
int expect_number(){
  if(token->kind!=TK_NUM){
    error("数じゃないけど???");
  }
  int val = token->val;
  token = token->next;
  return val;
}
bool at_eof() { return token->kind == TK_EOF; }
//新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  cur->next = tok;
  tok->kind = kind;
  tok -> str = str;
  return tok;
}
Token *tokenize(char *p) { 
  Token head;
  head.next = NULL;
  Token *cur = &head;
  while(*p){
    //空白の検出
    if(isspace(*p)){
      p++;
      continue;
    }
    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }
    error("トークナイズできんが...");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}

//エラー箇所特定用のコード
char *user_input;//入力プログラム
void error_at(char *loc, char *fmt, ...) { 
  va_list ap;
  va_start(ap, fmt);
  int pos = loc - user_input;//なん文字目のエラー家の特定
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");//pos個の空白
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
//抽象構文木作るよ〜
//抽象構文木のノード
typedef enum{
    ND_ADD,//+
    ND_SUB,//-
    ND_MUL,//X
    ND_DIV,// /
    ND_NUM,
}NodeKind;
//ノードの構造
typedef struct Node Node;
struct Node{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};
//ノード生成
Node* expr();
Node* primary();
Node* mul();
Node* new_node(NodeKind kind,Node* lhs,Node* rhs){
    Node *node=calloc(1,sizeof(Node));
    node->lhs=lhs;
    node->rhs=rhs;
    node->kind=kind;
    return node;
}
//数字の登録
Node* new_node_num(int val){
    Node *node=calloc(1,sizeof(Node));
    node->kind=ND_NUM;
    node->val=val;
    node->lhs=NULL;
    node->rhs=NULL;
    return node; 
    
}
Node* expr(){
    //expr=mul(+mul | -mul)*
    Node* node=mul();
    for(;;){
        if(consume('+')){
            node=new_node(ND_ADD,node,mul());
        }else if(consume('-')){
            node=new_node(ND_SUB,node,mul());
        }else{
            return node;
        }
    }
}
Node *mul(){
    //mul=primary('*'primary | '/'primary)*
    Node* node=primary();
    for(;;){
        if(consume('*')){
            node=new_node(ND_MUL,node,primary());
        }else if(consume('/')){
            node=primary(ND_DIV,node,primary());
        }else{
            return node;
        }
    }
}
Node* primary(){
    //primary=num | "(" expr ")"
    //次のトークンが（なら(expr)になる
    if(consume('(')){
        Node *node=expr();
        expect(')');
        return node;
    }
    return new_node_num(expect_number());
}
int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません。");
    return 1;
  }
  token = tokenize(argv[1]);
  user_input = argv[1];
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  //式の始まりは数値でないと困る
  printf("    mov rax,%d\n", expect_number());
  while(!at_eof()){
    //式の終わりでにない時ループ続行
    if(consume('+')){
      printf("  add rax,%d\n", expect_number());
      continue;
    }
    if(consume('-')) { 
      printf("  sub rax,%d\n", expect_number());
      continue;
    }
  }

  printf("    ret\n");
  return 0;
}
