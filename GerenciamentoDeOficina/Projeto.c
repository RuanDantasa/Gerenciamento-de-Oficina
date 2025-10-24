#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- Estruturas de Dados ---

typedef enum {
    AGUARDANDO_AVALIACAO,
    EM_REPARO,
    FINALIZADO,
    ENTREGUE
} StatusOrdem;

typedef struct {
    char nome[100];
    char cpf[12];
    char telefone[15];
} Cliente;

typedef struct {
    char placa[8];
    char modelo[50];
    int ano;
    char cpf_cliente[12];
} Veiculo;

typedef struct {
    int id;
    char placa_veiculo[8];
    char data_entrada[11];
    char descricao_problema[200];
    StatusOrdem status;
} OrdemServico;


// --- Funcoes Utilitarias ---

void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void limparTela() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pausarSistema() {
    printf("\nPressione Enter para continuar...");
    getchar();
}

int lerString(char* buffer, int tamanho) {
    if (fgets(buffer, tamanho, stdin) == NULL) {
        buffer[0] = '\0';
        return 1; 
    }

    size_t len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        return 1; 
    } else if (len == (size_t)tamanho - 1) {
        limparBuffer(); 
        return 0; 
    } else {
        return 1; 
    }
}

int validarCPF(const char* cpf) {
    if (strlen(cpf) != 11) return 0;
    for (int i = 0; i < 11; i++) {
        if (!isdigit(cpf[i])) return 0;
    }
    return 1;
}

int validarPlaca(const char* placa) {
    if (strlen(placa) != 7) return 0;
    for (int i = 0; i < 3; i++) {
        if (!isalpha(placa[i])) return 0;
    }
    for (int i = 3; i < 7; i++) {
        if (!isdigit(placa[i])) return 0;
    }
    return 1;
}

int validarNome(const char* nome) {
    if (strlen(nome) == 0) return 0;
    for (int i = 0; nome[i] != '\0'; i++) {
        if (!isalpha(nome[i]) && !isspace(nome[i])) {
            return 0;
        }
    }
    return 1;
}


// --- Funcoes de Banco de Dados (Arquivos) ---

void carregarDados(const char* nomeArquivo, void** dados, int* total, size_t tamanhoElemento) {
    FILE* arquivo = fopen(nomeArquivo, "rb");
    if (arquivo == NULL) {
        *total = 0;
        *dados = NULL;
        return;
    }

    if (fread(total, sizeof(int), 1, arquivo) != 1) {
        *total = 0;
        *dados = NULL;
        fclose(arquivo);
        return;
    }
    
    if (*total < 0 || *total > 10000) {
        printf("Aviso: Arquivo '%s' corrompido. Iniciando com base limpa.\n", nomeArquivo);
        pausarSistema();
        *total = 0;
        *dados = NULL;
        fclose(arquivo);
        return;
    }
    
    if (*total == 0) {
        *dados = NULL;
        fclose(arquivo);
        return;
    }

    *dados = malloc(*total * tamanhoElemento);
    if (*dados == NULL) {
        printf("ERRO CRITICO: Falha ao alocar memoria para carregar '%s'!\n", nomeArquivo);
        exit(EXIT_FAILURE);
    }
    
    if (fread(*dados, tamanhoElemento, *total, arquivo) != (size_t)*total) {
        printf("ERRO CRITICO: Falha ao ler dados de '%s'.\n", nomeArquivo);
        free(*dados);
        *dados = NULL;
        *total = 0;
        pausarSistema();
    }
    
    fclose(arquivo);
}

void salvarClientes(Cliente* clientes, int total) {
    FILE* arquivo = fopen("clientes.dat", "wb");
    if (arquivo == NULL) {
        perror("Erro ao salvar arquivo de clientes");
        pausarSistema(); return;
    }
    fwrite(&total, sizeof(int), 1, arquivo);
    fwrite(clientes, sizeof(Cliente), total, arquivo);
    fclose(arquivo);
}

void salvarVeiculos(Veiculo* veiculos, int total) {
    FILE* arquivo = fopen("veiculos.dat", "wb");
    if (arquivo == NULL) {
        perror("Erro ao salvar arquivo de veiculos");
        pausarSistema(); return;
    }
    fwrite(&total, sizeof(int), 1, arquivo);
    fwrite(veiculos, sizeof(Veiculo), total, arquivo);
    fclose(arquivo);
}

void salvarOrdens(OrdemServico* ordens, int total) {
    FILE* arquivo = fopen("ordens.dat", "wb");
    if (arquivo == NULL) {
        perror("Erro ao salvar arquivo de ordens");
        pausarSistema(); return;
    }
    fwrite(&total, sizeof(int), 1, arquivo);
    fwrite(ordens, sizeof(OrdemServico), total, arquivo);
    fclose(arquivo);
}


// --- Funcoes de logica e busca ---

int buscarClientePorCPF(Cliente* clientes, int total, const char* cpf) {
    for (int i = 0; i < total; i++) {
        if (strcmp(clientes[i].cpf, cpf) == 0) return i;
    }
    return -1;
}

int buscarVeiculoPorPlaca(Veiculo* veiculos, int total, const char* placa) {
    for (int i = 0; i < total; i++) {
        if (strcmp(veiculos[i].placa, placa) == 0) return i;
    }
    return -1;
}

// --- Funcoes de gerenciamento do Clientes ---

void cadastrarCliente(Cliente** clientes, int* totalClientes) {
    limparTela();
    printf("--- Cadastro de Cliente ---\n");
    Cliente novoCliente;
    int overflow; 
    
    do {
        printf("Nome: ");
        if (!lerString(novoCliente.nome, 101)) {
            printf("ERRO: Nome muito longo. Maximo de 99 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
        if (!overflow && strlen(novoCliente.nome) == 0) {
            printf("ERRO: Nome nao pode ser vazio.\n");
        } else if (!overflow && !validarNome(novoCliente.nome)) {
            printf("ERRO: Nome deve conter apenas letras e espacos.\n");
        }
    } while (overflow || strlen(novoCliente.nome) == 0 || !validarNome(novoCliente.nome));

    do {
        printf("CPF (11 digitos, sem pontos): ");
        if (!lerString(novoCliente.cpf, 13)) { 
            printf("ERRO: CPF muito longo. Maximo de 11 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
        
        if (!overflow && !validarCPF(novoCliente.cpf)) {
            printf("ERRO: Formato de CPF invalido. Deve ter 11 digitos.\n");
        } else if (!overflow && buscarClientePorCPF(*clientes, *totalClientes, novoCliente.cpf) != -1) {
            printf("ERRO: CPF ja cadastrado.\n");
            novoCliente.cpf[0] = '\0';
        }
    } while (overflow || !validarCPF(novoCliente.cpf));
    
    do {
        printf("Telefone: ");
        if (!lerString(novoCliente.telefone, 16)) {
            printf("ERRO: Telefone muito longo. Maximo de 14 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);

    Cliente* temp = malloc((*totalClientes + 1) * sizeof(Cliente));
    if (temp == NULL) {
        printf("ERRO CRITICO: Falha ao alocar memoria!\n");
        pausarSistema(); return;
    }
    
    for (int i = 0; i < *totalClientes; i++) temp[i] = (*clientes)[i];
    temp[*totalClientes] = novoCliente;
    if (*clientes != NULL) free(*clientes);
    *clientes = temp;
    (*totalClientes)++;

    printf("\nCliente cadastrado com sucesso!\n");
    pausarSistema();
}

void atualizarCliente(Cliente* clientes, int totalClientes) {
    limparTela();
    printf("--- Atualizacao de Cliente ---\n");
    if (totalClientes == 0) {
        printf("Nenhum cliente cadastrado.\n");
        pausarSistema(); return;
    }
    char cpf[12];
    int overflow;
    do {
        printf("Digite o CPF do cliente a ser atualizado: ");
        if (!lerString(cpf, 13)) {
             printf("ERRO: Formato de CPF invalido. Maximo de 11 caracteres.\n");
             overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);

    int index = buscarClientePorCPF(clientes, totalClientes, cpf);
    if (index == -1) {
        printf("Cliente nao encontrado.\n");
        pausarSistema(); return;
    }

    printf("Digite os novos dados (deixe em branco para manter o atual):\n");
    char buffer[100];

    do {
        printf("Nome atual: %s\nNovo nome: ", clientes[index].nome);
        if (!lerString(buffer, 101)) {
            printf("ERRO: Nome muito longo. Maximo de 99 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
            if (strlen(buffer) > 0) { 
                if (!validarNome(buffer)) {
                    printf("ERRO: Nome deve conter apenas letras e espacos.\n");
                    overflow = 1; 
                } else {
                    strcpy(clientes[index].nome, buffer);
                }
            }
        }
    } while (overflow);

    do {
        printf("Telefone atual: %s\nNovo telefone: ", clientes[index].telefone);
        if (!lerString(buffer, 16)) {
            printf("ERRO: Telefone muito longo. Maximo de 14 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
            if (strlen(buffer) > 0) strcpy(clientes[index].telefone, buffer);
        }
    } while (overflow);
    
    printf("\nCliente atualizado com sucesso!\n");
    pausarSistema();
}

void removerCliente(Cliente** clientes, int* totalClientes, Veiculo* veiculos, int totalVeiculos) {
    limparTela();
    printf("--- Remocao de Cliente ---\n");
    if (*totalClientes == 0) {
        printf("Nenhum cliente para remover.\n");
        pausarSistema(); return;
    }
    char cpf[12];
    int overflow;
    do {
        printf("Digite o CPF do cliente a ser removido: ");
        if (!lerString(cpf, 13)) {
             printf("ERRO: Formato de CPF invalido. Maximo de 11 caracteres.\n");
             overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);

    for (int i = 0; i < totalVeiculos; i++) {
        if (strcmp(veiculos[i].cpf_cliente, cpf) == 0) {
            printf("ERRO: Nao e possivel remover cliente com veiculo cadastrado.\n");
            pausarSistema(); return;
        }
    }

    int index = buscarClientePorCPF(*clientes, *totalClientes, cpf);
    if (index == -1) {
        printf("Cliente nao encontrado.\n");
        pausarSistema(); return;
    }
    
    for (int i = index; i < (*totalClientes - 1); i++) (*clientes)[i] = (*clientes)[i + 1];
    
    (*totalClientes)--;
    if (*totalClientes > 0) {
        Cliente* temp = malloc(*totalClientes * sizeof(Cliente));
        if (temp == NULL) {
            printf("AVISO: Falha ao diminuir memoria, pode haver espaco desperdicado.\n");
            return;
        }
        for (int i = 0; i < *totalClientes; i++) temp[i] = (*clientes)[i];
        free(*clientes);
        *clientes = temp;
    } else {
        free(*clientes);
        *clientes = NULL;
    }
    
    printf("\nCliente removido com sucesso!\n");
    pausarSistema();
}

void gerenciarClientes(Cliente** clientes, int* totalClientes, Veiculo* veiculos, int totalVeiculos) {
    int opcao = -1;
    char buffer[10];
    int overflow;
    do {
        limparTela();
        printf("--- Gerenciar Clientes ---\n");
        printf("1. Cadastrar Cliente\n");
        printf("2. Atualizar Cliente\n");
        printf("3. Remover Cliente\n");
        printf("0. Voltar\n");
        printf("Escolha uma opcao: ");
        
        if (!lerString(buffer, 4)) { 
            printf("ERRO: Opcao muito longa.\n");
            overflow = 1;
            opcao = -1;
        } else {
            overflow = 0;
            opcao = atoi(buffer);
        }

        if(overflow) {
            pausarSistema();
            continue;
        }

        switch (opcao) {
            case 1: cadastrarCliente(clientes, totalClientes); break;
            case 2: atualizarCliente(*clientes, *totalClientes); break;
            case 3: removerCliente(clientes, totalClientes, veiculos, totalVeiculos); break;
            case 0: break;
            default: printf("Opcao invalida!\n"); pausarSistema();
        }
    } while (opcao != 0);
}

// --- Funcoes de gerenciamenti dos Veiculos ---

void cadastrarVeiculo(Veiculo** veiculos, int* totalVeiculos, Cliente* clientes, int totalClientes) {
    limparTela();
    printf("--- Cadastro de Veiculo ---\n");
    if (totalClientes == 0) {
        printf("Nenhum cliente cadastrado. Cadastre um cliente primeiro.\n");
        pausarSistema(); return;
    }

    Veiculo novoVeiculo;
    char cpf[12];
    int overflow;

    do {
        printf("CPF do proprietario: ");
        if (!lerString(cpf, 13)) {
            printf("ERRO: Formato de CPF invalido. Maximo de 11 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);
    
    if (buscarClientePorCPF(clientes, totalClientes, cpf) == -1) {
        printf("ERRO: Cliente nao encontrado.\n");
        pausarSistema(); return;
    }
    strcpy(novoVeiculo.cpf_cliente, cpf);

    do {
        printf("Placa (formato AAA1234): ");
        if (!lerString(novoVeiculo.placa, 9)) { 
            printf("ERRO: Placa muito longa. Maximo de 7 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
        if (!overflow && !validarPlaca(novoVeiculo.placa)) {
            printf("ERRO: Formato de placa invalido.\n");
        } else if (!overflow && buscarVeiculoPorPlaca(*veiculos, *totalVeiculos, novoVeiculo.placa) != -1) {
            printf("ERRO: Placa ja cadastrada.\n");
            novoVeiculo.placa[0] = '\0';
        }
    } while (overflow || !validarPlaca(novoVeiculo.placa));

    do {
        printf("Modelo: ");
        if (!lerString(novoVeiculo.modelo, 51)) { 
            printf("ERRO: Modelo muito longo. Maximo de 49 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
        if (!overflow && strlen(novoVeiculo.modelo) == 0) printf("ERRO: Modelo nao pode ser vazio.\n");
    } while (overflow || strlen(novoVeiculo.modelo) == 0);
    
    char anoBuffer[10];
    do {
        printf("Ano: ");
        if (!lerString(anoBuffer, 6)) { 
            printf("ERRO: Ano muito longo. Maximo de 4 digitos.\n");
            overflow = 1;
            novoVeiculo.ano = 0; 
        } else {
            overflow = 0;
            novoVeiculo.ano = atoi(anoBuffer);
        }
        
        if (!overflow && (novoVeiculo.ano < 1900 || novoVeiculo.ano > 2026)) {
             printf("ERRO: Ano invalido (use 1900-2026).\n");
        }
    } while (overflow || novoVeiculo.ano < 1900 || novoVeiculo.ano > 2026);
    

    Veiculo* temp = malloc((*totalVeiculos + 1) * sizeof(Veiculo));
    if (temp == NULL) {
        printf("ERRO CRITICO: Falha ao alocar memoria para novo veiculo!\n");
        pausarSistema(); return;
    }

    for (int i = 0; i < *totalVeiculos; i++) temp[i] = (*veiculos)[i];
    temp[*totalVeiculos] = novoVeiculo;
    if (*veiculos != NULL) free(*veiculos);
    *veiculos = temp;
    (*totalVeiculos)++;

    printf("\nVeiculo cadastrado com sucesso!\n");
    pausarSistema();
}

void atualizarVeiculo(Veiculo* veiculos, int totalVeiculos) {
    limparTela();
    printf("--- Atualizacao de Veiculo ---\n");
    if (totalVeiculos == 0) {
        printf("Nenhum veiculo cadastrado.\n");
        pausarSistema(); return;
    }
    char placa[8];
    int overflow;
    do {
        printf("Digite a placa do veiculo a ser atualizado: ");
        if (!lerString(placa, 9)) {
            printf("ERRO: Placa muito longa. Maximo de 7 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);

    int index = buscarVeiculoPorPlaca(veiculos, totalVeiculos, placa);
    if (index == -1) {
        printf("Veiculo nao encontrado.\n");
        pausarSistema(); return;
    }

    printf("Digite os novos dados (deixe em branco para manter o atual):\n");
    char buffer[51];

    do {
        printf("Modelo atual: %s\nNovo modelo: ", veiculos[index].modelo);
        if (!lerString(buffer, 51)) {
            printf("ERRO: Modelo muito longo. Maximo de 49 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
            if (strlen(buffer) > 0) strcpy(veiculos[index].modelo, buffer);
        }
    } while (overflow);

    do {
        printf("Ano atual: %d\nNovo ano: ", veiculos[index].ano);
        if (!lerString(buffer, 6)) {
            printf("ERRO: Ano muito longo. Maximo de 4 digitos.\n");
            overflow = 1;
        } else {
            overflow = 0;
            if (strlen(buffer) > 0) {
                int ano = atoi(buffer);
                if (ano >= 1900 && ano <= 2026) {
                    veiculos[index].ano = ano;
                } else {
                    printf("AVISO: Ano invalido, valor nao alterado.\n");
                }
            }
        }
    } while (overflow);

    printf("\nVeiculo atualizado com sucesso!\n");
    pausarSistema();
}

void removerVeiculo(Veiculo** veiculos, int* totalVeiculos, OrdemServico* ordens, int totalOrdens) {
    limparTela();
    printf("--- Remocao de Veiculo ---\n");
    if (*totalVeiculos == 0) {
        printf("Nenhum veiculo para remover.\n");
        pausarSistema(); return;
    }
    char placa[8];
    int overflow;
    do {
        printf("Digite a placa do veiculo a ser removido: ");
        if (!lerString(placa, 9)) {
            printf("ERRO: Placa muito longa. Maximo de 7 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);
    
    for(int i = 0; i < totalOrdens; i++){
        if(strcmp(ordens[i].placa_veiculo, placa) == 0){
            printf("ERRO: Nao e possivel remover veiculo com ordem de servico associada.\n");
            pausarSistema(); return;
        }
    }

    int index = buscarVeiculoPorPlaca(*veiculos, *totalVeiculos, placa);
    if (index == -1) {
        printf("Veiculo nao encontrado.\n");
        pausarSistema(); return;
    }

    for (int i = index; i < *totalVeiculos - 1; i++) (*veiculos)[i] = (*veiculos)[i + 1];
    
    (*totalVeiculos)--;
    if (*totalVeiculos > 0) {
        Veiculo* temp = malloc(*totalVeiculos * sizeof(Veiculo));
        if (temp == NULL) {
            printf("AVISO: Falha ao diminuir memoria, pode haver espaco desperdicado.\n");
            return;
        }
        for (int i = 0; i < *totalVeiculos; i++) temp[i] = (*veiculos)[i];
        free(*veiculos);
        *veiculos = temp;
    } else {
        free(*veiculos);
        *veiculos = NULL;
    }
    
    printf("\nVeiculo removido com sucesso!\n");
    pausarSistema();
}

void gerenciarVeiculos(Veiculo** veiculos, int* totalVeiculos, Cliente* clientes, int totalClientes, OrdemServico* ordens, int totalOrdens) {
    int opcao = -1;
    char buffer[10];
    int overflow;
    do {
        limparTela();
        printf("--- Gerenciar Veiculos ---\n");
        printf("1. Cadastrar Veiculo\n");
        printf("2. Atualizar Veiculo\n");
        printf("3. Remover Veiculo\n");
        printf("0. Voltar\n");
        printf("Escolha uma opcao: ");
        
        if (!lerString(buffer, 4)) {
            printf("ERRO: Opcao muito longa.\n");
            overflow = 1;
            opcao = -1;
        } else {
            overflow = 0;
            opcao = atoi(buffer);
        }

        if(overflow) {
            pausarSistema();
            continue;
        }

        switch (opcao) {
            case 1: cadastrarVeiculo(veiculos, totalVeiculos, clientes, totalClientes); break;
            case 2: atualizarVeiculo(*veiculos, *totalVeiculos); break;
            case 3: removerVeiculo(veiculos, totalVeiculos, ordens, totalOrdens); break;
            case 0: break;
            default: printf("Opcao invalida!\n"); pausarSistema();
        }
    } while (opcao != 0);
}

// --- Funcoes de gerenciamento de Ordens de Servico ---

const char* getStatusString(StatusOrdem status) {
    switch (status) {
        case AGUARDANDO_AVALIACAO: return "Aguardando Avaliacao";
        case EM_REPARO: return "Em Reparo";
        case FINALIZADO: return "Finalizado";
        case ENTREGUE: return "Entregue";
        default: return "Desconhecido";
    }
}

void abrirOrdemServico(OrdemServico** ordens, int* totalOrdens, Veiculo* veiculos, int totalVeiculos) {
    limparTela();
    printf("--- Abertura de Ordem de Servico ---\n");
    if (totalVeiculos == 0) {
        printf("Nenhum veiculo cadastrado. Cadastre um veiculo primeiro.\n");
        pausarSistema(); return;
    }

    OrdemServico novaOrdem;
    char placa[8];
    int overflow;

    do {
        printf("Placa do veiculo: ");
        if (!lerString(placa, 9)) {
            printf("ERRO: Placa muito longa. Maximo de 7 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);
    
    if (buscarVeiculoPorPlaca(veiculos, totalVeiculos, placa) == -1) {
        printf("ERRO: Veiculo nao encontrado.\n");
        pausarSistema(); return;
    }
    strcpy(novaOrdem.placa_veiculo, placa);

    novaOrdem.id = *totalOrdens + 1;

    do {
        printf("Data de Entrada (DD/MM/AAAA): ");
        if (!lerString(novaOrdem.data_entrada, 12)) { 
            printf("ERRO: Data muito longa. Maximo de 10 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);

    do {
        printf("Descricao do Problema: ");
         if (!lerString(novaOrdem.descricao_problema, 201)) { 
            printf("ERRO: Descricao muito longa. Maximo de 199 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);

    novaOrdem.status = AGUARDANDO_AVALIACAO;

    OrdemServico* temp = malloc((*totalOrdens + 1) * sizeof(OrdemServico));
    if (temp == NULL) {
        printf("ERRO CRITICO: Falha ao alocar memoria para nova ordem!\n");
        pausarSistema(); return;
    }
    for (int i = 0; i < *totalOrdens; i++) temp[i] = (*ordens)[i];
    temp[*totalOrdens] = novaOrdem;
    if (*ordens != NULL) free(*ordens);
    *ordens = temp;
    (*totalOrdens)++;
    
    printf("\nOrdem de servico aberta com sucesso! ID: %d\n", novaOrdem.id);
    pausarSistema();
}

void atualizarOrdemServico(OrdemServico* ordens, int totalOrdens) {
    limparTela();
    printf("--- Atualizar Status da Ordem de Servico ---\n");
    if (totalOrdens == 0) {
        printf("Nenhuma ordem de servico cadastrada.\n");
        pausarSistema(); return;
    }
    char idBuffer[11]; 
    int overflow;
    do {
        printf("Digite o ID da Ordem de Servico: ");
        if (!lerString(idBuffer, 11)) { 
            printf("ERRO: ID muito longo.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
    } while(overflow);
    
    int id = atoi(idBuffer);

    int index = -1;
    for (int i = 0; i < totalOrdens; i++) {
        if (ordens[i].id == id) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        printf("Ordem de Servico nao encontrada.\n");
        pausarSistema(); return;
    }

    printf("Status atual: %s\n", getStatusString(ordens[index].status));
    printf("Selecione o novo status:\n");
    printf("0. AGUARDANDO_AVALIACAO\n1. EM_REPARO\n2. FINALIZADO\n3. ENTREGUE\n");
    
    char statusBuffer[10];
    do {
        printf("Opcao: ");
        if (!lerString(statusBuffer, 4)) {
            printf("ERRO: Opcao muito longa.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);
    
    int novoStatus = atoi(statusBuffer);

    if (novoStatus >= 0 && novoStatus <= 3) {
        ordens[index].status = (StatusOrdem)novoStatus;
        printf("Status atualizado com sucesso!\n");
    } else {
        printf("Opcao de status invalida.\n");
    }
    pausarSistema();
}

void listarOrdens(OrdemServico* ordens, int totalOrdens) {
    limparTela();
    printf("--- Lista de Todas as Ordens de Servico ---\n");
    if(totalOrdens == 0){
        printf("Nenhuma ordem de servico cadastrada.\n");
        pausarSistema(); return;
    }

    for (int i = 0; i < totalOrdens; i++) {
        printf("----------------------------------------\n");
        printf("ID: %d\n", ordens[i].id);
        printf("Placa do Veiculo: %s\n", ordens[i].placa_veiculo);
        printf("Data de Entrada: %s\n", ordens[i].data_entrada);
        printf("Problema: %s\n", ordens[i].descricao_problema);
        printf("Status: %s\n", getStatusString(ordens[i].status));
    }
    printf("----------------------------------------\n");
    pausarSistema();
}

void gerenciarOrdens(OrdemServico** ordens, int* totalOrdens, Veiculo* veiculos, int totalVeiculos) {
    int opcao = -1;
    char buffer[10];
    int overflow;
    do {
        limparTela();
        printf("--- Gerenciar Ordens de Servico ---\n");
        printf("1. Abrir Ordem de Servico\n");
        printf("2. Atualizar Status da Ordem\n");
        printf("3. Listar Todas as Ordens\n");
        printf("0. Voltar\n");
        printf("Escolha uma opcao: ");
        
        if (!lerString(buffer, 4)) {
            printf("ERRO: Opcao muito longa.\n");
            overflow = 1;
            opcao = -1;
        } else {
            overflow = 0;
            opcao = atoi(buffer);
        }

        if(overflow) {
            pausarSistema();
            continue;
        }

        switch (opcao) {
            case 1: abrirOrdemServico(ordens, totalOrdens, veiculos, totalVeiculos); break;
            case 2: atualizarOrdemServico(*ordens, *totalOrdens); break;
            case 3: listarOrdens(*ordens, *totalOrdens); break;
            case 0: break;
            default: printf("Opcao invalida!\n"); pausarSistema();
        }
    } while (opcao != 0);
}

// --- Funcoes de Relatorio ---

void relatorioHistoricoVeiculo(Veiculo* veiculos, int totalVeiculos, OrdemServico* ordens, int totalOrdens) {
    limparTela();
    printf("--- Relatorio: Historico de Servicos por Veiculo ---\n");
    if (totalVeiculos == 0) {
        printf("Nenhum veiculo cadastrado.\n");
        pausarSistema(); return;
    }
    char placa[8];
    int overflow;
    do {
        printf("Digite a placa do veiculo: ");
        if (!lerString(placa, 9)) {
            printf("ERRO: Placa muito longa. Maximo de 7 caracteres.\n");
            overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);
    
    if (buscarVeiculoPorPlaca(veiculos, totalVeiculos, placa) == -1) {
        printf("Veiculo nao encontrado.\n");
        pausarSistema(); return;
    }

    FILE* relatorio = fopen("relatorio_historico_veiculo.txt", "w");
    if (relatorio == NULL) {
        perror("Erro ao criar arquivo de relatorio");
        pausarSistema(); return;
    }

    fprintf(relatorio, "Historico de Servicos do Veiculo - Placa: %s\n", placa);
    fprintf(relatorio, "==============================================\n");
    int encontrou = 0;
    for (int i = 0; i < totalOrdens; i++) {
        if (strcmp(ordens[i].placa_veiculo, placa) == 0) {
            fprintf(relatorio, "ID Ordem: %d\n", ordens[i].id);
            fprintf(relatorio, "Data Entrada: %s\n", ordens[i].data_entrada);
            fprintf(relatorio, "Problema: %s\n", ordens[i].descricao_problema);
            fprintf(relatorio, "Status: %s\n", getStatusString(ordens[i].status));
            fprintf(relatorio, "----------------------------------------------\n");
            encontrou = 1;
        }
    }
    if (!encontrou) {
        fprintf(relatorio, "Nenhuma ordem de servico encontrada para este veiculo.\n");
    }
    fclose(relatorio);
    printf("Relatorio 'relatorio_historico_veiculo.txt' gerado com sucesso!\n");
    pausarSistema();
}

void relatorioVeiculosCliente(Cliente* clientes, int totalClientes, Veiculo* veiculos, int totalVeiculos) {
    limparTela();
    printf("--- Relatorio: Veiculos por Cliente ---\n");
    if (totalClientes == 0) {
        printf("Nenhum cliente cadastrado.\n");
        pausarSistema(); return;
    }
    char cpf[12];
    int overflow;
    do {
        printf("Digite o CPF do cliente: ");
        if (!lerString(cpf, 13)) {
             printf("ERRO: Formato de CPF invalido. Maximo de 11 caracteres.\n");
             overflow = 1;
        } else {
            overflow = 0;
        }
    } while (overflow);

    int indexCliente = buscarClientePorCPF(clientes, totalClientes, cpf);
    if (indexCliente == -1) {
        printf("Cliente nao encontrado.\n");
        pausarSistema(); return;
    }

    FILE* relatorio = fopen("relatorio_veiculos_cliente.txt", "w");
     if (relatorio == NULL) {
        perror("Erro ao criar arquivo de relatorio");
        pausarSistema(); return;
    }

    fprintf(relatorio, "Veiculos do Cliente: %s (CPF: %s)\n", clientes[indexCliente].nome, cpf);
    fprintf(relatorio, "==============================================\n");
    int encontrou = 0;
    for (int i = 0; i < totalVeiculos; i++) {
        if (strcmp(veiculos[i].cpf_cliente, cpf) == 0) {
            fprintf(relatorio, "Placa: %s\n", veiculos[i].placa);
            fprintf(relatorio, "Modelo: %s\n", veiculos[i].modelo);
            fprintf(relatorio, "Ano: %d\n", veiculos[i].ano);
            fprintf(relatorio, "----------------------------------------------\n");
            encontrou = 1;
        }
    }
     if (!encontrou) {
        fprintf(relatorio, "Nenhum veiculo encontrado para este cliente.\n");
    }
    fclose(relatorio);
    printf("Relatorio 'relatorio_veiculos_cliente.txt' gerado com sucesso!\n");
    pausarSistema();
}

void gerarRelatorios(Cliente* clientes, int totalClientes, Veiculo* veiculos, int totalVeiculos, OrdemServico* ordens, int totalOrdens) {
     int opcao = -1;
     char buffer[10];
     int overflow;
    do {
        limparTela();
        printf("--- Gerar Relatorios ---\n");
        printf("1. Historico de servicos de um veiculo\n");
        printf("2. Listar veiculos de um cliente\n");
        printf("0. Voltar\n");
        printf("Escolha uma opcao: ");
        
        if (!lerString(buffer, 4)) {
            printf("ERRO: Opcao muito longa.\n");
            overflow = 1;
            opcao = -1;
        } else {
            overflow = 0;
            opcao = atoi(buffer);
        }

        if(overflow) {
            pausarSistema();
            continue;
        }

        switch (opcao) {
            case 1: relatorioHistoricoVeiculo(veiculos, totalVeiculos, ordens, totalOrdens); break;
            case 2: relatorioVeiculosCliente(clientes, totalClientes, veiculos, totalVeiculos); break;
            case 0: break;
            default: printf("Opcao invalida!\n"); pausarSistema();
        }
    } while (opcao != 0);
}

// --- Manual ---

void exibirManual() {
    limparTela();
    printf("--- Manual do Usuario: Sistema de Gerenciamento de Oficina ---\n\n");
    printf("1. INTRODUCAO\n");
    printf("   Este sistema permite o cadastro e gerenciamento de clientes, veiculos e\n");
    printf("   ordens de servico. Ele opera por menus de texto e salva todos os dados\n");
    printf("   automaticamente ao sair.\n\n");

    printf("2. FUNCIONAMENTO GERAL\n");
    printf("   - Para escolher uma opcao, digite o numero correspondente e pressione Enter.\n");
    printf("   - Os dados sao carregados ao iniciar e salvos ao escolher a opcao 'Sair'.\n");
    printf("   - Fechar a janela do terminal diretamente fara com que as alteracoes\n");
    printf("     nao sejam salvas.\n\n");

    printf("3. GERENCIAR CLIENTES (Menu 1)\n");
    printf("   - Cadastrar: Adiciona um novo cliente. CPF deve ser unico e com 11 digitos.\n");
    printf("     Nome deve conter apenas letras e espacos.\n");
    printf("   - Atualizar: Modifica o nome e/ou telefone de um cliente existente via CPF.\n");
    printf("   - Remover: Apaga um cliente via CPF. So e permitido se o cliente nao\n");
    printf("     possuir veiculos cadastrados.\n\n");

    printf("4. GERENCIAR VEICULOS (Menu 2)\n");
    printf("   - Cadastrar: Adiciona um novo veiculo. E necessario informar o CPF de um\n");
    printf("     cliente ja cadastrado. A placa (formato AAA1234) deve ser unica.\n");
    printf("   - Atualizar: Modifica o modelo e/ou ano de um veiculo existente via Placa.\n");
    printf("   - Remover: Apaga um veiculo via Placa. So e permitido se o veiculo nao\n");
    printf("     possuir ordens de servico associadas.\n\n");

    printf("5. GERENCIAR ORDENS DE SERVICO (Menu 3)\n");
    printf("   - Abrir: Cria uma nova ordem de servico para um veiculo cadastrado.\n");
    printf("     A ordem recebe um ID unico e o status 'AGUARDANDO AVALIACAO'.\n");
    printf("   - Atualizar Status: Altera o status de uma O.S. existente (Em Reparo,\n");
    printf("     Finalizado, Entregue).\n");
    printf("   - Listar Todas: Exibe todas as ordens de servico ja criadas.\n\n");

    printf("6. GERAR RELATORIOS (Menu 4)\n");
    printf("   - Gera arquivos de texto (.txt) na mesma pasta do programa.\n");
    printf("   - Relatorio 1: Pede uma placa e lista todo o historico de servicos do veiculo.\n");
    printf("   - Relatorio 2: Pede um CPF e lista todos os veiculos daquele cliente.\n\n");
    
    pausarSistema();
}

// --- Funcao Principal ---

void menuPrincipal() {
    Cliente* clientes = NULL;
    int totalClientes = 0;
    Veiculo* veiculos = NULL;
    int totalVeiculos = 0;
    OrdemServico* ordens = NULL;
    int totalOrdens = 0;

    carregarDados("clientes.dat", (void**)&clientes, &totalClientes, sizeof(Cliente));
    carregarDados("veiculos.dat", (void**)&veiculos, &totalVeiculos, sizeof(Veiculo));
    carregarDados("ordens.dat", (void**)&ordens, &totalOrdens, sizeof(OrdemServico));

    int opcao = -1;
    char buffer[10];
    int overflow;
    do {
        limparTela();
        printf("--- Sistema de Gerenciamento de Oficina ---\n");
        printf("1. Gerenciar Clientes\n");
        printf("2. Gerenciar Veiculos\n");
        printf("3. Gerenciar Ordens de Servico\n");
        printf("4. Gerar Relatorios\n");
        printf("5. Manual do Usuario\n");
        printf("0. Sair\n");
        printf("Escolha uma opcao: ");
        
        if (!lerString(buffer, 4)) {
            printf("ERRO: Opcao muito longa.\n");
            overflow = 1;
            opcao = -1;
        } else {
            overflow = 0;
            opcao = atoi(buffer);
        }

        if(overflow) {
            pausarSistema();
            continue;
        }

        switch (opcao) {
            case 1: gerenciarClientes(&clientes, &totalClientes, veiculos, totalVeiculos); break;
            case 2: gerenciarVeiculos(&veiculos, &totalVeiculos, clientes, totalClientes, ordens, totalOrdens); break;
            case 3: gerenciarOrdens(&ordens, &totalOrdens, veiculos, totalVeiculos); break;
            case 4: gerarRelatorios(clientes, totalClientes, veiculos, totalVeiculos, ordens, totalOrdens); break;
            case 5: exibirManual(); break;
            case 0:
                salvarClientes(clientes, totalClientes);
                salvarVeiculos(veiculos, totalVeiculos);
                salvarOrdens(ordens, totalOrdens);
                printf("Dados salvos. Saindo do sistema...\n");
                break;
            default:
                printf("Opcao invalida!\n");
                pausarSistema();
        }
    } while (opcao != 0);

    free(clientes);
    free(veiculos);
    free(ordens);
}

int main() {
    menuPrincipal();
    return 0;

}
