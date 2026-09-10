// =====================================================================
//  Analise Empirica de Algoritmos - Estrutura de Dados Basicas II
//  Algoritmo de Dijkstra com tres estruturas de dados distintas:
//    (1) Dijkstra Normal   - busca linear do minimo        -> O(V^2 + A)
//    (2) Dijkstra Heap Min - fila de prioridade (heap bin.) -> O((V + A) log V)
//    (3) Dijkstra Bucket   - buckets circulares (Dial 1969) -> O(A + V + C)
//
//  Os pseudocodigos das tres versoes foram reproduzidos de:
//    RODRIGUES, G. C. S. Analise de desempenho do algoritmo de Dijkstra
//    com diferentes estruturas de dados. TCC, UFES, Alegre-ES, 2023.
//    Codigo original do TCC: https://github.com/guiconrado/Djkstra
//
//  Adaptacoes feitas por este grupo (ver Secao 3 do relatorio):
//    - substituicao das instancias fixas do DIMACS por um gerador
//      parametrico de grafos conexos, permitindo variar n de forma
//      controlada;
//    - insercao do harness de medicao de tempo (chrono) com repeticoes;
//    - insercao da rotina de validacao cruzada entre as tres versoes;
//    - saida em CSV para posterior geracao de graficos.
//
//  Compilacao:  g++ -O2 -std=c++17 -o dijkstra dijkstra_experimento.cpp
//  Execucao:    ./dijkstra > resultados_brutos.csv
// =====================================================================

#include <bits/stdc++.h>
using namespace std;
using Clock = chrono::steady_clock;

const long long INF = numeric_limits<long long>::max() / 4;

// ---------------------------------------------------------------------
// Representacao do grafo em formato CSR (Compressed Sparse Row).
// Escolhida por ser compacta e por dar localidade de memoria igual
// para as tres versoes do algoritmo - assim a estrutura de dados da
// fila de prioridade e a UNICA variavel do experimento.
// ---------------------------------------------------------------------
struct Grafo {
    int n;                 // numero de vertices  (este e o nosso "n")
    long long m;           // numero de arestas (nao direcionadas)
    vector<int> ini;       // ini[v] .. ini[v+1]-1 : faixa de v em dest/peso
    vector<int> dest;      // vertice de destino da aresta
    vector<int> peso;      // peso/custo da aresta
    int C;                 // maior peso de aresta (necessario ao Bucket)
};

// ---------------------------------------------------------------------
// Gerador de grafo conexo, nao direcionado e com pesos inteiros em [1,C].
//   1) sorteia uma arvore geradora aleatoria (garante conexidade);
//   2) completa com arestas aleatorias ate atingir m arestas.
// Manter m = grau_medio/2 * n mantem a DENSIDADE fixa, de modo que o
// unico parametro que cresce e n.
// ---------------------------------------------------------------------
Grafo gerarGrafo(int n, int grauMedio, int C, uint64_t semente) {
    mt19937_64 rng(semente);
    uniform_int_distribution<int> distPeso(1, C);

    long long m = (long long)n * grauMedio / 2;
    if (m < n - 1) m = n - 1;

    vector<int> u, v, w;
    u.reserve(m); v.reserve(m); w.reserve(m);

    // (1) arvore geradora aleatoria
    for (int i = 1; i < n; ++i) {
        int pai = (int)(rng() % (uint64_t)i);
        u.push_back(pai); v.push_back(i); w.push_back(distPeso(rng));
    }
    // (2) arestas extras
    while ((long long)u.size() < m) {
        int a = (int)(rng() % (uint64_t)n);
        int b = (int)(rng() % (uint64_t)n);
        if (a == b) continue;
        u.push_back(a); v.push_back(b); w.push_back(distPeso(rng));
    }

    Grafo g;
    g.n = n;
    g.m = (long long)u.size();
    g.C = C;
    g.ini.assign(n + 1, 0);
    for (size_t i = 0; i < u.size(); ++i) { g.ini[u[i] + 1]++; g.ini[v[i] + 1]++; }
    for (int i = 0; i < n; ++i) g.ini[i + 1] += g.ini[i];
    g.dest.resize(2 * u.size());
    g.peso.resize(2 * u.size());
    vector<int> pos(g.ini.begin(), g.ini.end() - 1);
    for (size_t i = 0; i < u.size(); ++i) {
        g.dest[pos[u[i]]] = v[i]; g.peso[pos[u[i]]++] = w[i];
        g.dest[pos[v[i]]] = u[i]; g.peso[pos[v[i]]++] = w[i];
    }
    return g;
}

// =====================================================================
// (1) DIJKSTRA NORMAL - selecao do minimo por varredura linear do vetor
//     Cada uma das n iteracoes varre os n vertices em busca do menor
//     Dist[] ainda aberto  ->  n^2 operacoes de comparacao.
// =====================================================================
void dijkstraNormal(const Grafo& g, int s, vector<long long>& dist) {
    int n = g.n;
    dist.assign(n, INF);
    vector<char> fechado(n, 0);
    dist[s] = 0;

    for (int it = 0; it < n; ++it) {
        // --- extrai o minimo: O(n) ---
        int v = -1;
        long long melhor = INF;
        for (int k = 0; k < n; ++k)
            if (!fechado[k] && dist[k] < melhor) { melhor = dist[k]; v = k; }
        if (v < 0) break;                 // restaram apenas inalcancaveis
        fechado[v] = 1;
        // --- relaxamento das arestas de v ---
        for (int e = g.ini[v]; e < g.ini[v + 1]; ++e) {
            int u = g.dest[e];
            if (dist[v] + g.peso[e] < dist[u]) dist[u] = dist[v] + g.peso[e];
        }
    }
}

// =====================================================================
// (2) DIJKSTRA COM HEAP MINIMO - fila de prioridade com remocao lazy
//     Extrair-Minimo e Atualizar-Heap custam O(log n).
// =====================================================================
void dijkstraHeap(const Grafo& g, int s, vector<long long>& dist) {
    int n = g.n;
    dist.assign(n, INF);
    priority_queue<pair<long long,int>,
                   vector<pair<long long,int>>,
                   greater<pair<long long,int>>> heap;
    dist[s] = 0;
    heap.push({0, s});

    while (!heap.empty()) {
        auto [d, v] = heap.top(); heap.pop();
        if (d != dist[v]) continue;       // entrada obsoleta (lazy delete)
        for (int e = g.ini[v]; e < g.ini[v + 1]; ++e) {
            int u = g.dest[e];
            long long nd = d + g.peso[e];
            if (nd < dist[u]) { dist[u] = nd; heap.push({nd, u}); }
        }
    }
}

// =====================================================================
// (3) DIJKSTRA COM BUCKETS (Dial) - fila de prioridade monotona
//     Como todo peso esta em [1,C], as chaves presentes na fila nunca
//     diferem mais de C entre si; logo C+1 buckets circulares bastam.
//     Extracao e insercao sao O(1); o custo extra vem da varredura
//     sequencial dos indices de bucket, proporcional a maior distancia.
// =====================================================================
void dijkstraBucket(const Grafo& g, int s, vector<long long>& dist) {
    int n = g.n;
    int B = g.C + 1;                      // numero de buckets circulares
    dist.assign(n, INF);
    vector<char> fechado(n, 0);
    vector<vector<int>> bucket(B);

    dist[s] = 0;
    bucket[0].push_back(s);
    long long naFila = 1;
    long long d = 0;

    while (naFila > 0) {
        int idx = (int)(d % B);
        if (bucket[idx].empty()) { ++d; continue; }   // pula bucket vazio
        int v = bucket[idx].back(); bucket[idx].pop_back(); --naFila;
        if (fechado[v] || dist[v] != d) continue;     // entrada obsoleta
        fechado[v] = 1;
        for (int e = g.ini[v]; e < g.ini[v + 1]; ++e) {
            int u = g.dest[e];
            long long nd = d + g.peso[e];
            if (nd < dist[u]) {
                dist[u] = nd;
                bucket[(int)(nd % B)].push_back(u);
                ++naFila;
            }
        }
    }
}

// ---------------------------------------------------------------------
// Validacao: as tres versoes devem devolver EXATAMENTE o mesmo vetor
// de distancias minimas. Sem isso, comparar tempos nao faria sentido.
// ---------------------------------------------------------------------
bool validar(int n, int grauMedio, int C, uint64_t semente) {
    Grafo g = gerarGrafo(n, grauMedio, C, semente);
    vector<long long> a, b, c;
    dijkstraNormal(g, 0, a);
    dijkstraHeap  (g, 0, b);
    dijkstraBucket(g, 0, c);
    return a == b && b == c;
}

// ---------------------------------------------------------------------
// Modo "stats": coleta grandezas estruturais que ajudam a explicar os
// tempos observados - a maior distancia final (Dmax, que limita a
// varredura de buckets) e o numero de indices de bucket vazios
// visitados. NAO faz parte da medicao de tempo.
// ---------------------------------------------------------------------
void estatisticasBucket(const Grafo& g, int s, long long& dmax, long long& varreduras) {
    int n = g.n, B = g.C + 1;
    vector<long long> dist(n, INF);
    vector<char> fechado(n, 0);
    vector<vector<int>> bucket(B);
    dist[s] = 0; bucket[0].push_back(s);
    long long naFila = 1, d = 0;
    varreduras = 0;
    while (naFila > 0) {
        int idx = (int)(d % B);
        if (bucket[idx].empty()) { ++d; ++varreduras; continue; }
        int v = bucket[idx].back(); bucket[idx].pop_back(); --naFila;
        if (fechado[v] || dist[v] != d) continue;
        fechado[v] = 1;
        for (int e = g.ini[v]; e < g.ini[v + 1]; ++e) {
            int u = g.dest[e];
            long long nd = d + g.peso[e];
            if (nd < dist[u]) { dist[u] = nd; bucket[(int)(nd % B)].push_back(u); ++naFila; }
        }
    }
    dmax = 0;
    for (int v = 0; v < n; ++v) if (dist[v] < INF) dmax = max(dmax, dist[v]);
}

int main(int argc, char** argv) {
    const int   GRAU_MEDIO = 8;      // densidade fixa: m = 4n arestas
    const int   C          = 1000;   // pesos inteiros em [1, 1000]
    const int   REPETICOES = 5;
    vector<int> tamanhos = {1000, 2000, 4000, 8000, 16000, 32000, 64000};

    // ---------- modo estatisticas estruturais ----------
    if (argc > 1 && string(argv[1]) == "stats") {
        printf("n,m,dmax,varreduras_bucket\n");
        for (int n : tamanhos) {
            Grafo g = gerarGrafo(n, GRAU_MEDIO, C, 12345ULL + 1000ULL * n + 1);
            long long dmax, varr;
            estatisticasBucket(g, 0, dmax, varr);
            printf("%d,%lld,%lld,%lld\n", n, g.m, dmax, varr);
        }
        return 0;
    }

    // ---------- etapa de validacao ----------
    for (int n : {500, 5000}) {
        bool ok = validar(n, GRAU_MEDIO, C, 987654321ULL + n);
        fprintf(stderr, "[validacao] n=%d -> %s\n", n, ok ? "OK (3 versoes concordam)" : "FALHOU");
        if (!ok) return 1;
    }

    // ---------- etapa de medicao ----------
    printf("n,m,grau_medio,C,algoritmo,repeticao,tempo_ms\n");
    for (int n : tamanhos) {
        for (int r = 1; r <= REPETICOES; ++r) {
            // o grafo e gerado FORA da medicao (nao faz parte do algoritmo)
            Grafo g = gerarGrafo(n, GRAU_MEDIO, C, 12345ULL + 1000ULL * n + r);
            vector<long long> dist;

            struct Alvo { const char* nome; void (*fn)(const Grafo&, int, vector<long long>&); };
            Alvo alvos[3] = {
                {"Dijkstra Normal", dijkstraNormal},
                {"Dijkstra Heap Minimo", dijkstraHeap},
                {"Dijkstra Bucket", dijkstraBucket}
            };
            for (auto& alvo : alvos) {
                auto t0 = Clock::now();
                alvo.fn(g, 0, dist);
                auto t1 = Clock::now();
                double ms = chrono::duration<double, milli>(t1 - t0).count();
                printf("%d,%lld,%d,%d,%s,%d,%.6f\n",
                       n, g.m, GRAU_MEDIO, C, alvo.nome, r, ms);
            }
            fprintf(stderr, "n=%d rep=%d concluido\n", n, r);
        }
    }
    return 0;
}
