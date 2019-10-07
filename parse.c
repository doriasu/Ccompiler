#include "9cc.h"
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