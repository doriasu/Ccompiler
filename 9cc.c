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
  int len;//トークン文字列の長さ
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
bool consume(char *op){
  if(token->kind!=TK_RESERVED||strlen(op)!=token->len||memcmp(token->str,op,token->len)){
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
Token *new_token(TokenKind kind, Token *cur, char *str,int len) {
  Token *tok = calloc(1, sizeof(Token));
  cur->next = tok;
  tok->kind = kind;
  tok -> str = str;
  tok->len=len;
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
    if (*p == '+' || *p == '-'||*p=='*'||*p=='/'||*p=='('||*p==')') {
      cur = new_token(TK_RESERVED, cur, p++,1);
      continue;
    }
    if(*p=='>'){
        if(*(p+1)=='='){
            cur=new_token(TK_RESERVED,cur,p,2);
            p+=2;
        }else{
            cur=new_token(TK_RESERVED,cur,p++,1);
        }
        continue;
    }
    if(*p=='<'){
        if(*(p+1)=='='){
            cur=new_token(TK_RESERVED,cur,p,2);
            p+=2;
        }else{
            cur=new_token(TK_RESERVED,cur,p++,1);
        }
        continue;
    }
    if(*p=='='&&*(p+1)=='='){
        cur=new_token(TK_RESERVED,cur,p,2);
        p+=2;
        continue;
    }
     if(*p=='!'&&*(p+1)=='='){
        cur=new_token(TK_RESERVED,cur,p,2);
        p+=2;
        continue;
    }
    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p,1);
      char *q=p;
      cur->val = strtol(p, &p, 10);
      cur->len=p-q;
      
      continue;
    }
    error("トークナイズできんが...");
  }
  new_token(TK_EOF, cur, p++,1);
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
    ND_DIV,// 
    ND_ONAJI,//==
    ND_CHIGAU,//!=
    ND_DAINARI,//>
    ND_DAINARINARI,//>=
    ND_SHONARI,//<
    ND_SHONARINARI,//<=
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
Node* equality();
Node* expr();
Node* relational();
Node* add();
Node* primary();
Node* mul();
Node* unary();
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
Node *expr(){
    return equality();
}
//nodeの生成
Node *equality(){
    //expr=relational("=="relational|"!="relational)
    Node* node=relational();
    for(;;){
        if(consume("==")){
            node=new_node(ND_ONAJI,node,relational());
        }else if(consume("!=")){
            node=new_node(ND_CHIGAU,node,relational());
        }else{
            return node;
        }
    }

}
Node *relational(){
    //relational=add("<"add|"<="add|">"add|">="add)
    Node* node=add();
    for(;;){
        if(consume("<")){
            node=new_node(ND_SHONARI,node,add());
        }else if(consume("<=")){
            node=new_node(ND_SHONARINARI,node,add());
        }else if(consume(">")){
            node=new_node(ND_DAINARI,node,add());
        }else if(consume(">=")){
            node=new_node(ND_DAINARINARI,node,add());
        }else{
            return node;
        }
    }

}
Node* add(){
    //expr=mul(+mul | -mul)*
    Node* node=mul();
    for(;;){
        if(consume("+")){
            node=new_node(ND_ADD,node,mul());
        }else if(consume("-")){
            node=new_node(ND_SUB,node,mul());
        }else{
            return node;
        }
    }
}
Node *mul(){
    //mul=primary('*'primary | '/'primary)*
    Node* node=unary();
    for(;;){
        if(consume("*")){
            node=new_node(ND_MUL,node,unary());
        }else if(consume("/")){
            node=new_node(ND_DIV,node,unary());
        }else{
            return node;
        }
    }
}
Node* primary(){
    //primary=num | "(" expr ")"
    //次のトークンが（なら(expr)になる
    if(consume("(")){
        Node *node=expr();
        expect(')');
        return node;
    }
    return new_node_num(expect_number());
}
Node* unary(){
    if(consume("+")){
        return unary();
    }
    if(consume("-")){
        return new_node(ND_SUB,new_node_num(0),unary());
    }
    return primary();
}
//構文解析木を実際にスタックマシンで計算してみる
void gen(Node *node){
    if(node->kind==ND_NUM){
        printf("    push %d\n",node->val);
    }else if(node->kind==ND_ADD){
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rax\n");
        printf("    pop rdi\n");
        printf("    add rax,rdi\n");
        printf("    push rax\n");
    }else if(node->kind==ND_SUB){
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    sub rax,rdi\n");
        printf("    push rax\n");
    }else if(node->kind==ND_MUL){
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    imul rax,rdi\n");
        printf("    push rax\n");
    }else if(node->kind==ND_DIV){
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    cqo\n");
        printf("    idiv rdi\n");
        printf("    push rax\n");

    }else if(node->kind==ND_ONAJI){
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    cmp rax,rdi\n");
        printf("    sete al\n");
        printf("    movzb rax,al\n");
        printf("    push rax\n");

    }else if(node->kind==ND_CHIGAU){
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    cmp rax,rdi\n");
        printf("    setne al\n");
        printf("    movzb rax,al\n");
        printf("    push rax\n");

    }else if(node->kind==ND_SHONARI){
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    cmp rax,rdi\n");
        printf("    setl al\n");
        printf("    movzb rax,al\n");
        printf("    push rax\n");

    }else if(node->kind==ND_SHONARINARI){
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    cmp rax,rdi\n");
        printf("    setle al\n");
        printf("    movzb rax,al\n");
        printf("    push rax\n");

    }else if(node->kind==ND_DAINARI){
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    cmp rdi,rax\n");
        printf("    setl al\n");
        printf("    movzb rax,al\n");
        printf("    push rax\n");

    }else if(node->kind==ND_DAINARINARI){
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    cmp rdi,rax\n");
        printf("    setle al\n");
        printf("    movzb rax,al\n");
        printf("    push rax\n");

    }





}
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
