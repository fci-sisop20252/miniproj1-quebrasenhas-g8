#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

/**
 * PROCESSO COORDENADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 * 
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 * 
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 * 
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 * 
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 * 
 * @param charset_len Tamanho do conjunto de caracteres
 * @param password_len Comprimento da senha
 * @return Número total de combinações possíveis
 */
long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) {
        total *= charset_len;
    }
    return total;
}

/**
 * Converte um índice numérico para uma senha
 * Usado para definir os limites de cada worker
 * 
 * @param index Índice numérico da senha
 * @param charset Conjunto de caracteres
 * @param charset_len Tamanho do conjunto
 * @param password_len Comprimento da senha
 * @param output Buffer para armazenar a senha gerada
 */
void index_to_password(long long index, const char *charset, int charset_len, 
                       int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

/**
 * Função principal do coordenador
 */
int main(int argc, char *argv[]) {
    // TODO 1: Validar argumentos de entrada
    // Verificar se argc == 5 (programa + 4 argumentos)
    // Se não, imprimir mensagem de uso e sair com código 1
    if(argc != 5){
    	fprintf(stderr, "Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>\n", argv[0]);
    	return 1;
	}
    // IMPLEMENTE AQUI: verificação de argc e mensagem de erro
    
    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);
    
    // TODO: Adicionar validações dos parâmetros
    // - password_len deve estar entre 1 e 10
    // - num_workers deve estar entre 1 e MAX_WORKERS
    // - charset não pode ser vazio
    if(password_len < 1 || password_len > 10){
    	fprintf(stderr, "Erro: valor deve ser entre 1 e 10\n");
    	return 1;
	}
	if(num_workers < 1 || num_workers > MAX_WORKERS){
		fprintf(stderr, "Erro: valor deve ser entre 1 e %d\n", MAX_WORKERS);
		return 1;
	}
	if(charset_len == 0){
		fprintf(stderr, "Erro: charset não pode ser vazio\n");
		return 1;
	}
    printf("=== Mini-Projeto 1: Quebra de Senhas Paralelo ===\n");
    printf("Hash MD5 alvo: %s\n", target_hash);
    printf("Tamanho da senha: %d\n", password_len);
    printf("Charset: %s (tamanho: %d)\n", charset, charset_len);
    printf("Número de workers: %d\n", num_workers);
    
    // Calcular espaço de busca total
    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n\n", total_space);
    
    // Remover arquivo de resultado anterior se existir
    unlink(RESULT_FILE);
    
    // Registrar tempo de início
    time_t start_time = time(NULL);
    
    // TODO 2: Dividir o espaço de busca entre os workers
    // Calcular quantas senhas cada worker deve verificar
    // DICA: Use divisão inteira e distribua o resto entre os primeiros workers
    long long passwords_per_worker = total_space/num_workers;
    long long remaining = total_space % num_workers;
    // IMPLEMENTE AQUI:
    // long long passwords_per_worker = ?
    // long long remaining = ?
    
    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];
    
    // TODO 3: Criar os processos workers usando fork()
    printf("Iniciando workers...\n");
    
    // IMPLEMENTE AQUI: Loop para criar workers
    for (int i = 0; i < num_workers; i++) {
    	long long start_index = i * passwords_per_worker;
    	if(i < remaining){
    		start_index += i;
		}
		else{
			start_index += remaining;
		}
		long long end_index = start_index + passwords_per_worker -1；
		if(i < remaining){
			end_index += 1;
		}
		if(i == num_workers -1){
			end_index = total_space -1;
		}
		char start_pass[password_len + 1];
		char end_pass[password_len +1];
		index_password(start_index, charset, charset_len, password_len, start_pass);
		index_password(end_index, charset, charset_len, password_len, end_pass);
		pid_t pid = fork();
		if (pid == 0){
			char worker_id_str[10];
			sprintf(worker_id_str, "%d", i);
			char pass_len_str[10];
			sprintf(pass_len_str, "%d", password_len);
			excel("./worker", "worker", worker_id_str, target_hash, pass_len_str, charset, start_pass, end_pass, NULL);
			perror("Erro executar worker");
			exit(1);
		}
		else{
			workers[i] = pid;
			printf("worker %d iniciado (PID: %d) - Intervalo: %s a %s \n", i, pid, start_pass, end_pass);
			
		}
		     // TODO: Calcular intervalo de senhas para este worker
        // TODO: Converter indices para senhas de inicio e fim
        // TODO 4: Usar fork() para criar processo filho
        // TODO 5: No processo pai: armazenar PID
        // TODO 6: No processo filho: usar execl() para executar worker
        // TODO 7: Tratar erros de fork() e execl()
    }
    
    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");
    
    // TODO 8: Aguardar todos os workers terminarem usando wait()
    // IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis
    
    // IMPLEMENTE AQUI:
    // - Loop para aguardar cada worker terminar
    // - Usar wait() para capturar status de saída
    // - Identificar qual worker terminou
    // - Verificar se terminou normalmente ou com erro
    // - Contar quantos workers terminaram
    int workers_completed = 0;
    int workers_found = 0;
    
    while (workers_completed < num_workers) {
        int status;
        pid_t finished_pid = wait(&status);
        
        if (finished_pid == -1) {
            perror("Erro ao aguardar processo");
            break;
        }
        
        // Encontrar qual worker terminou
        for (int i = 0; i < num_workers; i++) {
            if (workers[i] == finished_pid) {
                if (WIFEXITED(status)) {
                    int exit_status = WEXITSTATUS(status);
                    if (exit_status == 0) {
                        printf("Worker %d terminou com sucesso\n", i);
                    } else if (exit_status == 2) {
                        printf("Worker %d encontrou a senha!\n", i);
                        workers_found++;
                    } else {
                        printf("Worker %d terminou com código de erro: %d\n", i, exit_status);
                    }
                } else if (WIFSIGNALED(status)) {
                    printf("Worker %d foi terminado por sinal: %d\n", i, WTERMSIG(status));
                }
                
                workers_completed++;
                break;
            }
        }
    }
    // Registrar tempo de fim
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);
    
    printf("\n=== Resultado ===\n");
    
    // TODO 9: Verificar se algum worker encontrou a senha
    // Ler o arquivo password_found.txt se existir
    
    // IMPLEMENTE AQUI:
    // - Abrir arquivo RESULT_FILE para leitura
    // - Ler conteúdo do arquivo
    // - Fazer parse do formato "worker_id:password"
    // - Verificar o hash usando md5_string()
    // - Exibir resultado encontrado
    
    // Estatísticas finais (opcional)
    // TODO: Calcular e exibir estatísticas de performance
    FILE *result_file = fopen(RESULT_FILE, "r");
    if (result_file != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), result_file)) {
            // Formato esperado: "worker_id:password"
            char *colon = strchr(line, ':');
            if (colon != NULL) {
                *colon = '\0';
                int worker_id = atoi(line);
                char *password = colon + 1;
                
                // Remover nova linha se existir
                char *newline = strchr(password, '\n');
                if (newline != NULL) *newline = '\0';
                
                // Verificar o hash
                char *hash = md5_string(password);
                if (hash != NULL && strcmp(hash, target_hash) == 0) {
                    printf("Senha encontrada pelo worker %d: %s\n", worker_id, password);
                    printf("Hash verificado: %s\n", hash);
                } else {
                    printf("Aviso: Senha reportada não corresponde ao hash alvo\n");
                }
                
                free(hash);
            }
        }
        fclose(result_file);
    } else if (workers_found == 0) {
        printf("Nenhuma senha encontrada no espaço de busca.\n");
    }
    
    // Estatísticas finais
    printf("\n=== Estatísticas ===\n");
    printf("Tempo total de execução: %.2f segundos\n", elapsed_time);
    printf("Taxa média: %.2f combinações/segundo\n", 
           total_space / (elapsed_time > 0 ? elapsed_time : 1));
    printf("Workers que reportaram sucesso: %d/%d\n", workers_found, num_workers);
    return 0;
}