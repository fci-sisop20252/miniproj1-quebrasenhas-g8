# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

**Aluno(s):** Geovana Bomfim RodriguesNome (10410514), Yating Zheng (10439511)
---

## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

Implementei um algoritmo que calcula o número total de senhas possíveis e divide igualmente entre os workers. Para casos onde a divisão não é exata, os primeiros workers recebem uma senha adicional.

**Código relevante:** Cole aqui a parte do coordinator.c onde você calcula a divisão:
```c
// Cole seu código de divisão aqui
long long passwords_per_worker = total_space/num_workers;
long long remaining = total_space % num_workers;
```

---

## 2. Implementação das System Calls

**Descreva como você usou fork(), execl() e wait() no coordinator:**

Utilizei fork() para criar processos filhos, execl() para executar o programa worker com os argumentos adequados, e wait() no coordinator para aguardar a conclusão de todos os workers. Cada processo worker recebe como argumentos o hash alvo, a senha inicial, a senha final, o charset, o tamanho da senha e seu ID único.

**Código do fork/exec:**
```c
// Cole aqui seu loop de criação de workers
for (int i = 0; i < num_workers; i++) {
    	long long start_index = i * passwords_per_worker;
    	if(i < remaining){
    		start_index += i;
		}
		else{
			start_index += remaining;
		}
		long long end_index = start_index + passwords_per_worker -1;
		if(i < remaining){
			end_index += 1;
		}
		if(i == num_workers -1){
			end_index = total_space -1;
		}
		char start_pass[password_len + 1];
		char end_pass[password_len +1];
		index_to_password(start_index, charset, charset_len, password_len, start_pass);
		index_to_password(end_index, charset, charset_len, password_len, end_pass);
		pid_t pid = fork();
		if (pid == 0){
			char worker_id_str[10];
			sprintf(worker_id_str, "%d", i);
			char pass_len_str[10];
			sprintf(pass_len_str, "%d", password_len);
			execl("./worker", "worker", worker_id_str, target_hash, pass_len_str, charset, start_pass, end_pass, NULL);
			perror("Erro executar worker");
			exit(1);
		}
		else{
			workers[i] = pid;
			printf("worker %d iniciado (PID: %d) - Intervalo: %s a %s \n", i, pid, start_pass, end_pass);
			
		}
```

---

## 3. Comunicação Entre Processos

**Como você garantiu que apenas um worker escrevesse o resultado?**

Para garantir que apenas um worker escrevesse o resultado, implementei uma escrita atômica usando a combinação de flags O_CREAT | O_EXCL na syscall open(). Esta combinação garante que a criação do arquivo seja uma operação atômica - se o arquivo já existir, a chamada falha, impedindo condições de corrida onde múltiplos processos poderiam escrever no arquivo simultaneamente.

**Como o coordinator consegue ler o resultado?**

O coordinator lê o resultado verificando a existência do arquivo após a conclusão de todos os workers. Se o arquivo existir, ele é lido e analisado para extrair a senha encontrada e o ID do worker que a encontrou.
---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | 0.003s | 0.003s | 0.003s | 1.0 |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | 0.002s | 0.002s | 0.003s | 0.667 |

**O speedup foi linear? Por quê?**
O speedup não foi perfeitamente linear devido ao overhead de criação e gerenciamento de processos.
o ganho ainda é significativo. O overhead inclui:

· Tempo para criar processos com fork()
· Tempo para carregar o executável do worker com execl()
· Comunicação entre processos através do sistema de arquivos
· Escalonamento de processos pelo sistema operacional

---

## 5. Desafios e Aprendizados
**Qual foi o maior desafio técnico que você enfrentou?**
A implementação do incremento de senhas usando um charset variável foi inicialmente complexa. Tratei este problema implementando um algoritmo que trata a senha como um número em base variável (onde a base é o tamanho do charset). Cada caractere da senha representa um dígito neste sistema numérico, e o incremento é feito da direita para a esquerda, similar a um odômetro.


---

## Comandos de Teste Utilizados

```bash
# Teste básico
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Teste de performance
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 1
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 4

# Teste com senha maior
time ./coordinator "5d41402abc4b2a76b9719d911017c592" 5 "abcdefghijklmnopqrstuvwxyz" 4
```
---

**Checklist de Entrega:**
- [ ] Código compila sem erros
- [ ] Todos os TODOs foram implementados
- [ ] Testes passam no `./tests/simple_test.sh`
- [ ] Relatório preenchido