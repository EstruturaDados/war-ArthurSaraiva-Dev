#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

// ============================================================================
// --- Constantes Globais ---
// ============================================================================
#define MAX_TERRITORIOS 20
#define MAX_MISSOES 4 // Número de missões implementadas
#define MAX_NOME_LEN 50
#define MAX_COR_LEN 10
#define ID_VITORIA_POR_TERRITORIOS 1 // ID para a missão de conquistar X territórios
#define ID_VITORIA_POR_DESTRUICAO 2 // ID para a missão de destruir um exército (cor)

// ============================================================================
// --- Estrutura de Dados ---
// ============================================================================
typedef struct {
    char nome[MAX_NOME_LEN];
    char cor_exercito[MAX_COR_LEN];
    int tropas;
} Territorio;

// ============================================================================
// --- Protótipos das Funções ---
// ============================================================================

// Funções de Setup e Gerenciamento de Memória
Territorio* alocarMapa();
void inicializarTerritorios(Territorio* mapa, const char* cor_jogador);
void liberarMemoria(Territorio** mapa_ptr); // Recebe ponteiro para ponteiro para poder setar *mapa_ptr como NULL

// Funções de Interface com o Usuário
void exibirMenuPrincipal(void);
void exibirMapa(const Territorio* mapa);
void exibirMissao(int id_missao, const char* cor_alvo);
void limparBufferEntrada(void);

// Funções de Lógica Principal do Jogo
void faseDeAtaque(Territorio* mapa, const char* cor_jogador);
void simularAtaque(Territorio* atacante, Territorio* defensor, const char* cor_jogador);
int sortearMissao(void);
int verificarVitoria(const Territorio* mapa, const char* cor_jogador, int id_missao, const char* cor_alvo);

// Função Utilitária
int rolarDado(void);

// ============================================================================
// --- Função Principal (main) ---
// ============================================================================
int main() {
    // 1. Configuração Inicial (Setup):
    setlocale(LC_ALL, "Portuguese");
    srand((unsigned int)time(NULL));

    Territorio* mapa = alocarMapa();
    if (mapa == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o mapa!\n");
        return 1;
    }

    const char cor_jogador[] = "AZUL"; // Cor fixa do jogador para o desafio
    inicializarTerritorios(mapa, cor_jogador);

    int id_missao_jogador = sortearMissao();
    const char* cor_alvo_missao = NULL;

    // Define o alvo da missão de destruição (se for o caso)
    if (id_missao_jogador == ID_VITORIA_POR_DESTRUICAO) {
        // Simulação de sorteio de cor alvo
        int r = rand() % 3; // 3 cores inimigas: VERDE, PRETO, AMARELO
        if (r == 0) cor_alvo_missao = "VERDE";
        else if (r == 1) cor_alvo_missao = "PRETO";
        else cor_alvo_missao = "AMARELO";
    }

    // 2. Laço Principal do Jogo (Game Loop):
    int escolha_jogador = -1;
    int jogo_vencido = 0;

    printf("\n\n*** BEM-VINDO AO WAR ESTRUTURADO! ***\n");
    printf("Você está no comando do exército **%s**.\n", cor_jogador);

    do {
        printf("\n--- RODADA ---\n");
        exibirMapa(mapa);
        exibirMissao(id_missao_jogador, cor_alvo_missao);
        exibirMenuPrincipal();

        printf("Escolha sua ação: ");
        if (scanf("%d", &escolha_jogador) != 1) {
            limparBufferEntrada();
            escolha_jogador = -1; // Valor inválido
            printf("Escolha inválida. Tente novamente.\n");
            continue;
        }
        limparBufferEntrada(); // Limpa o buffer após o scanf de inteiro

        switch (escolha_jogador) {
            case 1:
                faseDeAtaque(mapa, cor_jogador);
                break;
            case 2:
                jogo_vencido = verificarVitoria(mapa, cor_jogador, id_missao_jogador, cor_alvo_missao);
                if (jogo_vencido) {
                    printf("\n\n*** PARABÉNS! MISSÃO CUMPRIDA! VOCÊ VENCEU O JOGO! ***\n\n");
                } else {
                    printf("\nMissão ainda não cumprida. Continue atacando!\n");
                }
                break;
            case 0:
                printf("\nEncerrando o jogo. Até a próxima!\n");
                break;
            default:
                printf("\nOpção inválida. Tente novamente.\n");
                break;
        }

        if (jogo_vencido || escolha_jogador == 0) {
            break; // Sai do loop principal se vencer ou escolher sair
        }

        // Simulação de turno dos outros jogadores (apenas por uma pausa)
        printf("\n>>> Pressione ENTER para finalizar a rodada e continuar... <<<");
        getchar();

    } while (escolha_jogador != 0);

    // 3. Limpeza:
    liberarMemoria(&mapa);

    return 0;
}

// ============================================================================
// --- Implementação das Funções ---
// ============================================================================

/**
 * @brief Aloca dinamicamente a memória para o vetor de territórios.
 * @return Ponteiro para a memória alocada ou NULL em caso de falha.
 */
Territorio* alocarMapa() {
    // Alocação com calloc para inicializar a memória com zeros
    Territorio* mapa = (Territorio*)calloc(MAX_TERRITORIOS, sizeof(Territorio));
    return mapa;
}

/**
 * @brief Inicializa os dados dos territórios (nome, cor, tropas iniciais).
 * @param mapa Ponteiro para o vetor de Territorios a ser modificado.
 * @param cor_jogador A cor do exército do jogador.
 */
void inicializarTerritorios(Territorio* mapa, const char* cor_jogador) {
    // Apenas os 5 primeiros territórios serão inicializados com dados
    // e divididos entre o jogador e três oponentes (VERDE, PRETO, AMARELO).
    const char* cores[] = {cor_jogador, "VERDE", "PRETO", "AMARELO"};

    for (int i = 0; i < 5; i++) {
        sprintf(mapa[i].nome, "T-%02d", i + 1); // Ex: T-01, T-02...
        // Distribui as cores ciclicamente
        strcpy(mapa[i].cor_exercito, cores[i % 4]);
        // Tropas iniciais entre 2 e 4
        mapa[i].tropas = 2 + (rand() % 3);
    }
    // Os demais (MAX_TERRITORIOS - 5) ficam com dados vazios/zero de calloc
    printf("\nInicialização de 5 territórios concluída.\n");
}

/**
 * @brief Libera a memória alocada e define o ponteiro como NULL.
 * @param mapa_ptr Ponteiro para o ponteiro do mapa (passagem por referência para modificar o ponteiro em main).
 */
void liberarMemoria(Territorio** mapa_ptr) {
    if (*mapa_ptr != NULL) {
        free(*mapa_ptr);
        *mapa_ptr = NULL; // Evita ponteiro pendurado (dangling pointer)
        printf("\nMemória do mapa liberada com sucesso.\n");
    }
}

/**
 * @brief Imprime o menu de ações do jogo.
 */
void exibirMenuPrincipal(void) {
    printf("\n--- Ações ---\n");
    printf(" 1 - Atacar um Território\n");
    printf(" 2 - Verificar Condição de Vitória (Missão)\n");
    printf(" 0 - Sair do Jogo\n");
    printf("---------------\n");
}

/**
 * @brief Mostra o estado atual dos territórios em formato de tabela.
 * @param mapa Ponteiro constante para o vetor de Territorios (apenas leitura).
 */
void exibirMapa(const Territorio* mapa) {
    printf("\n======================================================\n");
    printf("| ID | Nome        | Dono        | Tropas (T)      |\n");
    printf("======================================================\n");
    int territorios_exibidos = 0;
    for (int i = 0; i < MAX_TERRITORIOS; i++) {
        // Exibe apenas os territórios que foram inicializados/conquistados (tropas > 0)
        if (mapa[i].tropas > 0) {
            printf("| %-2d | %-11s | %-11s | %-14d|\n",
                   i + 1, mapa[i].nome, mapa[i].cor_exercito, mapa[i].tropas);
            territorios_exibidos++;
        }
    }
    printf("======================================================\n");
    printf("Total de Territórios no Jogo: %d\n", territorios_exibidos);
}

/**
 * @brief Exibe a descrição da missão atual do jogador.
 * @param id_missao O ID da missão sorteada.
 * @param cor_alvo A cor alvo para a missão de destruição (se aplicável).
 */
void exibirMissao(int id_missao, const char* cor_alvo) {
    printf("\n--- Missão Secreta ---\n");
    switch (id_missao) {
        case ID_VITORIA_POR_TERRITORIOS:
            printf("ID %d: Conquiste **10** territórios para vencer o jogo.\n", id_missao);
            break;
        case ID_VITORIA_POR_DESTRUICAO:
            printf("ID %d: Destrua completamente o exército **%s** e conquiste **3** territórios.\n", id_missao, cor_alvo);
            break;
        case 3:
            printf("ID %d: Conquiste 2 continentes (não implementado: apenas exemplo).\n", id_missao);
            break;
        case 4:
            printf("ID %d: Conquiste 18 territórios (não implementado: apenas exemplo).\n", id_missao);
            break;
        default:
            printf("Missão de ID %d desconhecida.\n", id_missao);
            break;
    }
    printf("------------------------\n");
}

/**
 * @brief Gerencia a interface para a ação de ataque.
 * @param mapa Ponteiro para o vetor de Territorios (para modificação).
 * @param cor_jogador A cor do exército do jogador.
 */
void faseDeAtaque(Territorio* mapa, const char* cor_jogador) {
    int id_origem, id_destino;

    printf("\n--- Fase de Ataque ---\n");
    printf("ID do território atacante: ");
    if (scanf("%d", &id_origem) != 1) {
        printf("Entrada inválida.\n");
        limparBufferEntrada();
        return;
    }

    printf("ID do território defensor: ");
    if (scanf("%d", &id_destino) != 1) {
        printf("Entrada inválida.\n");
        limparBufferEntrada();
        return;
    }
    limparBufferEntrada();

    // Validação de IDs e limites
    if (id_origem < 1 || id_origem > MAX_TERRITORIOS || id_destino < 1 || id_destino > MAX_TERRITORIOS) {
        printf("IDs de território fora do limite permitido (1 a %d).\n", MAX_TERRITORIOS);
        return;
    }

    Territorio* atacante = &mapa[id_origem - 1];
    Territorio* defensor = &mapa[id_destino - 1];

    // Validação de tropas e dono
    if (atacante->tropas <= 1) {
        printf("Território atacante (%s) precisa de pelo menos 2 tropas.\n", atacante->nome);
        return;
    }
    if (strcmp(atacante->cor_exercito, cor_jogador) != 0) {
        printf("Você não é o dono do território atacante (%s).\n", atacante->nome);
        return;
    }
    if (strcmp(atacante->cor_exercito, defensor->cor_exercito) == 0) {
        printf("Você não pode atacar seu próprio território (%s).\n", defensor->nome);
        return;
    }

    // Validação de adjacência (Simulação simples, assumindo adjacência para o desafio)
    // if (!verificarAdjacencia(atacante, defensor)) { ... }

    simularAtaque(atacante, defensor, cor_jogador);
}

/**
 * @brief Executa a lógica da batalha, rolando dados e atualizando tropas.
 * @param atacante Ponteiro para o Territorio atacante (para modificação).
 * @param defensor Ponteiro para o Territorio defensor (para modificação).
 * @param cor_jogador A cor do exército do jogador.
 */
void simularAtaque(Territorio* atacante, Territorio* defensor, const char* cor_jogador) {
    int tropas_atacante = atacante->tropas - 1; // Máximo de tropas no ataque (mínimo 1 fica)
    int tropas_defensor = defensor->tropas;

    // Número de dados: Máximo 3 para ataque, Máximo 3 para defesa
    int dados_ataque = (tropas_atacante > 3) ? 3 : tropas_atacante;
    int dados_defesa = (tropas_defensor > 3) ? 3 : tropas_defensor;

    // Simulação da rolagem de dados (simplificada, apenas para efeito de modificação)
    int dado_a = rolarDado();
    int dado_d = rolarDado();

    printf("\n--- Combate: %s (%s) vs %s (%s) ---\n",
           atacante->nome, atacante->cor_exercito, defensor->nome, defensor->cor_exercito);
    printf("Dados Rolados: Ataque %d vs Defesa %d\n", dado_a, dado_d);

    // Comparação do dado mais alto (simplificada para o desafio)
    if (dado_a > dado_d) {
        defensor->tropas--;
        printf("Vitória do Atacante! O defensor perdeu 1 tropa. Tropas restantes: Defensor %d.\n", defensor->tropas);
    } else { // Empate e vitória do defensor
        atacante->tropas--;
        printf("Vitória do Defensor! O atacante perdeu 1 tropa. Tropas restantes: Atacante %d.\n", atacante->tropas);
    }

    // Verificar se o território defensor foi conquistado
    if (defensor->tropas <= 0) {
        printf("\n*** CONQUISTA! Território %s foi conquistado! ***\n", defensor->nome);
        // O defensor passa a ser do atacante
        strcpy(defensor->cor_exercito, cor_jogador);
        // Move uma tropa
        defensor->tropas = 1;
        atacante->tropas--;
    }

    printf("Fim do ataque. Tropas atuais: %s: %d, %s: %d\n",
           atacante->nome, atacante->tropas, defensor->nome, defensor->tropas);
}

/**
 * @brief Sorteia e retorna um ID de missão aleatório para o jogador.
 * @return O ID da missão sorteada.
 */
int sortearMissao(void) {
    // Sorteia um ID de missão entre 1 e MAX_MISSOES
    return 1 + (rand() % MAX_MISSOES);
}

/**
 * @brief Verifica se o jogador cumpriu os requisitos de sua missão atual.
 * @param mapa Ponteiro constante para o vetor de Territorios (apenas leitura).
 * @param cor_jogador A cor do exército do jogador.
 * @param id_missao O ID da missão a ser verificada.
 * @param cor_alvo A cor alvo para a missão de destruição (se aplicável).
 * @return 1 se a missão foi cumprida, 0 caso contrário.
 */
int verificarVitoria(const Territorio* mapa, const char* cor_jogador, int id_missao, const char* cor_alvo) {
    int territorios_conquistados = 0;

    // 1. Contagem base de territórios do jogador
    for (int i = 0; i < MAX_TERRITORIOS; i++) {
        if (mapa[i].tropas > 0 && strcmp(mapa[i].cor_exercito, cor_jogador) == 0) {
            territorios_conquistados++;
        }
    }

    // 2. Lógica de verificação por missão
    switch (id_missao) {
        case ID_VITORIA_POR_TERRITORIOS:
            printf("Requisito: Conquistar 10 territórios.\n");
            printf("Progresso: %d territórios conquistados.\n", territorios_conquistados);
            return (territorios_conquistados >= 10);

        case ID_VITORIA_POR_DESTRUICAO:
            printf("Requisito: Destruir o exército %s e conquistar 3 territórios.\n", cor_alvo);

            // Verifica se o exército alvo foi destruído (não possui mais territórios)
            int alvo_destruido = 1; // Assume que sim
            for (int i = 0; i < MAX_TERRITORIOS; i++) {
                if (mapa[i].tropas > 0 && strcmp(mapa[i].cor_exercito, cor_alvo) == 0) {
                    alvo_destruido = 0; // Se encontrar um, não foi destruído
                    break;
                }
            }

            printf("Progresso: Exército %s destruído: %s. Territórios conquistados: %d.\n",
                   cor_alvo, alvo_destruido ? "SIM" : "NÃO", territorios_conquistados);

            return (alvo_destruido && territorios_conquistados >= 3);

        default:
            // Para as missões não implementadas (3, 4)
            printf("Verificação para a Missão ID %d não implementada, considere-a falha por enquanto.\n", id_missao);
            return 0;
    }
}

/**
 * @brief Gera um número aleatório entre 1 e 6.
 * @return O resultado da rolagem de um dado (1-6).
 */
int rolarDado(void) {
    return 1 + (rand() % 6);
}

/**
 * @brief Função utilitária para limpar o buffer de entrada do teclado (stdin).
 */
void limparBufferEntrada(void) {
    int c;
    // Lê e descarta caracteres até encontrar uma nova linha ('\n') ou o fim do arquivo (EOF).
    while ((c = getchar()) != '\n' && c != EOF);
}
