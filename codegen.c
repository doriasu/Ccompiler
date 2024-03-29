#include "9cc.h"
Node* code[100];
int block;
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
	if(token->kind==TK_KAKKOLEFT){
		node=calloc(1,sizeof(Node));
		node->kind=ND_KAKKOLEFT;
		token=token->next;
		Node* move=node;
		while(token->kind!=TK_KAKKORIGHT){
			move->kakko=stmt();
			move=move->kakko;
		}
		token=token->next;
		move->kakko=NULL;
		return node;

	}
	else if(token->kind==TK_FOR){
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
			if(strncmp(token->str,"(",1)==0){

				node->kind=ND_FUNCTION;
				node->funcname=malloc(100);
				strncpy(node->funcname,tok->str,tok->len);
				token=token->next;
				while(1){
					if(strncmp(token->str,")",1)==0){
						break;
					}
					Node* node1=expr();
					node->hikisuu1=node1;
					if(strncmp(token->str,",",1)!=0){
						continue;
					}
					token=token->next;

					Node* node2=expr();
					node->hikisuu2=node2;
					if(strncmp(token->str,",",1)!=0){
						continue;
					}
					token=token->next;

					Node* node3=expr();
					node->hikisuu3=node3;
					if(strncmp(token->str,",",1)!=0){
						continue;
					}
					token=token->next;

					Node* node4=expr();
					node->hikisuu4=node4;
					if(strncmp(token->str,",",1)!=0){
						continue;
					}
					token=token->next;

					Node* node5=expr();
					node->hikisuu5=node5;
					if(strncmp(token->str,",",1)!=0){
						continue;
					}
					token=token->next;

					Node* node6=expr();
					node->hikisuu6=node6;
					if(strncmp(token->str,",",1)!=0){
						continue;
					}
					token=token->next;





				}
				expect(')');
				if(strncmp(token->str,"{",1)==0){
					node->func_definition=1;
					node->func=stmt();
				}



			}else{
				lvar=calloc(1,sizeof(LVar));
				lvar->next=locals;
				lvar->name=tok->str;
				lvar->len=tok->len;
				lvar->offset=locals->offset+8;
				node->offset=lvar->offset;
				locals=lvar;}

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
		printf("    je .Lendif%d\n",block);
		}else{
			printf("    je .LelseXXX%d\n",block);
		}
		gen(node->then);
		if(node->els){
		printf("    jmp .Lendif%d\n",block);
		printf(".LelseXXX%d:\n",block);	
		gen(node->els);}
		printf(".Lendif%d:\n",block);
		block++;
		

	}else if(node->kind==ND_WHILE){
		printf(".Lbeginwhile%d:\n",block);
		gen(node->cond);
		printf("	pop rax\n");
		printf("	cmp rax,0\n");
		printf("	je .Lendwhile%d\n",block);
		gen(node->then);
		printf("	jmp .Lbeginwhile%d\n",block);
		printf(".Lendwhile%d:\n",block);
		block++;

	}else if(node->kind==ND_FOR){
		if(node->definition){
			gen(node->definition);
		}
		printf(".Lbeginfor%d:\n",block);
		if(node->cond){
			gen(node->cond);
		}
		printf("	pop rax\n");
		printf("	cmp rax,0\n");
		printf("	je .Lendfor%d\n",block);
		if(node->then){
			gen(node->then);
		}
		if(node->update){
			gen(node->update);
		}
		printf("	jmp .Lbeginfor%d\n",block);
		printf(".Lendfor%d:\n",block);
		block++;

	}else if(node->kind==ND_KAKKOLEFT){
		//{}用のエンドジャンプラベルがひつようそう???多分gen(node->kakko)のwhileの後に作る？？？if文のLendxxxのバグが頭おかしいほかは大丈夫そうか←そもそも数字で管理する必要は有ると思われ
		while(node->kakko){
			gen(node->kakko);
			node=node->kakko;
			printf("	pop rax\n");
			
		}
	}else if(node->kind==ND_FUNCTION){
		printf("	mov rax,rsp\n");
		printf("	and rax,15\n");
		printf("	xor rax,15\n");
		//printf("	add rsp,rax\n");
		printf("	push rax\n");

		if(node->hikisuu1){
			gen(node->hikisuu1);
			printf("	pop rax\n");
			printf("	mov rdi,rax\n");

		}
		if(node->hikisuu2){
			gen(node->hikisuu2);
			printf("	pop rax\n");
			printf("	mov rsi,rax\n");

		}
		if(node->hikisuu3){
			gen(node->hikisuu3);
			printf("	pop rax\n");
			printf("	mov rdx,rax\n");

		}
		if(node->hikisuu4){
			gen(node->hikisuu4);
			printf("	pop rax\n");
			printf("	mov rcx,rax\n");

		}
		if(node->hikisuu5){
			gen(node->hikisuu5);
			printf("	pop rax\n");
			printf("	mov r8,rax\n");

		}
		if(node->hikisuu6){
			gen(node->hikisuu6);
			printf("	pop rax\n");
			printf("	mov r9,rax\n");

		}
		printf("	pop rax\n");
		printf("	cmp rax,0\n");
		printf("	jz .Idousuru%s\n",node->funcname);
		printf("	sub rsp,8\n");
		printf("	mov rax,0\n");
		printf("	call %s\n",node->funcname);
		printf("	add rsp,8\n");
		printf("	jmp .Lastsuru%s\n",node->funcname);
		printf(".Idousuru%s:\n",node->funcname);
		printf("	mov rax,0\n");
		printf("    call %s\n",node->funcname);
		printf(".Lastsuru%s:\n",node->funcname);
		printf("    push rax\n");
	}
	}





