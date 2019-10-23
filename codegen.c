#include "9cc.h"
Node* code[100];
//ノード生成
Node* equality();
Node* assign();
Node* expr();
Node* stmt();
Node* relational();
Node* add();
Node* primary();
Node* mul();
Node* unary();
void program();
//変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok){
	for(LVar *var=locals;var;var=var->next){
		if(var->len==tok->len&&!memcmp(tok->str,var->name,var->len)){
			return var;
		}	
	}
	return NULL;

    
	}
void program(){
    int i=0;
    while(!at_eof()){
        code[i++]=stmt();

    }
    code[i]=NULL;
}
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
    return assign();
}
Node* assign(){
    Node *node=equality();
    if(consume("=")){
        node=new_node(ND_ASSIGN,node,assign());}
        return node;
    

}
Node* stmt(){
	Node *node;
	if(token->kind==TK_FOR){
		node=calloc(1,sizeof(Node));
		node->kind=ND_FOR;
		token=token->next;
		expect('(');
		if(token->len==1&&strncmp(token->str,";",1)==0){
			node->definition=NULL;
			token=token->next;

		}else{
			node->definition=expr();
			expect(';');

		}
		if(token->len==1&&strncmp(token->str,";",1)==0){
			node->cond=NULL;
			token=token->next;

		}else{
			node->cond=expr();
			expect(';');

		}
		if(token->len==1&&strncmp(token->str,")",1)==0){
			node->update=NULL;
			expect(')');

		}else{
			node->update=expr();
			expect(')');

		}
		node->then=stmt();
		return node;



	}
	else if(token->kind==TK_WHILE){
		node=calloc(1,sizeof(Node));
		node->kind=ND_WHILE;
		token=token->next;
		expect('(');
		node->cond=expr();
		expect(')');
		node->then=stmt();
		return node;



	}
	else if(token->kind==TK_IF){
		node=calloc(1,sizeof(Node));
		node->kind=ND_IF;
		//node->els=NULL;
		token=token->next;
		expect('(');
		node->cond=expr();
		expect(')');
		node->then=stmt();
		if(token->kind==TK_ELSE){

		token=token->next;
		node->els=stmt();
	}
		return node;

	}
	
	else if(token->kind==TK_RETURN){
		node=calloc(1,sizeof(Node));
		node->kind=ND_RETURN;
		token=token->next;
		node->lhs=expr();

	}else{
		node=expr();
	}
	if(!consume(";")){
		perror(";ではないトークンです");

	}
	return node;

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
    //ここどうするよ
    Token *tok=consume_ident();
    if(tok){
		Node *node=calloc(1,sizeof(Node));
		node->kind=ND_LVAR;
		LVar *lvar=find_lvar(tok);
		if(lvar){
				node->offset=lvar->offset;
		}else{
				lvar=calloc(1,sizeof(LVar));
				lvar->next=locals;
				lvar->name=tok->str;
				lvar->len=tok->len;
				lvar->offset=locals->offset+8;
				node->offset=lvar->offset;
				locals=lvar;

		}
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
void gen_lval(Node *node){
    if(node->kind!=ND_LVAR){
        error("代入の左辺値が変数じゃないけど???");

    }
    printf("    mov rax,rbp\n");
    printf("    sub rax,%d\n",node->offset);
    printf("    push rax\n");
}
void gen(Node *node){
	if(node->kind==ND_RETURN){
		gen(node->lhs);
		printf("    pop rax\n");
		//printf("    mov rsp,rbp\n");
		//printf("    pop rbp\n");
		printf("    jmp .LendXXX\n");
		return;
	}
    else if(node->kind==ND_NUM){
        printf("    push %d\n",node->val);
        return;
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

    }else if(node->kind==ND_ASSIGN){
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax],rdi\n");
        printf("    push rdi\n");
        return;
    }else if(node->kind==ND_LVAR){
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax,[rax]\n");
        printf("    push rax\n");
        return;
    }else if(node->kind==ND_IF){

		gen(node->cond);
		printf("    pop rax\n");
		printf("    cmp rax,0\n");
		if(!node->els){
		printf("    je .LendXXX\n");
		}else{
			printf("    je .LelseXXX\n");
		}
		gen(node->then);
		if(node->els){
		printf("    jmp .LendXXX\n");
		printf(".LelseXXX:\n");	
		gen(node->els);}
		

	}else if(node->kind==ND_WHILE){
		printf(".Lbeginwhile:\n");
		gen(node->cond);
		printf("	pop rax\n");
		printf("	cmp rax,0\n");
		printf("	je .Lendwhile\n");
		gen(node->then);
		printf("	jmp .Lbeginwhile\n");
		printf(".Lendwhile:\n");

	}else if(node->kind==ND_FOR){
		if(node->definition){
			gen(node->definition);
		}
		printf(".Lbeginfor:\n");
		if(node->cond){
			gen(node->cond);
		}
		printf("	pop rax\n");
		printf("	cmp rax,0\n");
		printf("	je .Lendfor\n");
		if(node->then){
			gen(node->then);
		}
		if(node->update){
			gen(node->update);
		}
		printf("	jmp .Lbeginfor\n");
		printf(".Lendfor:\n");

	}
	}





