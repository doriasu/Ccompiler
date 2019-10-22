#include<stdio.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>
typedef enum {
  TK_RESERVED,  //記号
  TK_IDENT,//識別子
  TK_NUM,       //整数トークン
  TK_EOF,       //入力の終わりを表すトークン
  TK_RETURN,//returnを表すトークン
  TK_IF,
  TK_WHILE,
  TK_FOR,
  TK_ELSE
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
    ND_LVAR,//aとか
    ND_NUM,
    ND_ASSIGN,//=
	ND_RETURN,
	ND_IF,
	ND_WHILE,
	ND_FOR,
	ND_ELSE
}NodeKind;
//ノードの構
typedef struct LVar LVar;
struct LVar{
	struct LVar *next;//次の変数かNULL
	char* name;//変数名
	int len;//長さ
	int offset;//RBPからのオフセット
};
//ローカル変数
LVar* locals;
typedef struct Node Node;
struct Node{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
	//if用
	Node *cond;
	Node *then;
	Node *els;
    int val;
    int offset;
};
//現在着目nowのトークン
extern Node* code[100];
extern Token *token;
extern char *user_input;//入力プログラム
void error(char *fmt, ...);
Token *tokenize();
Node* expr();
void gen(Node *node);
bool consume(char *op);
int expect_number();
void expect(char op);
Token* consume_ident();
void program();
bool at_eof();
