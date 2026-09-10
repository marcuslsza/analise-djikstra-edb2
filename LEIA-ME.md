# Trabalho da Unidade 1 — IMD0039 (Análise Empírica de Algoritmos)

Comparação empírica do algoritmo de Dijkstra com três estruturas de fila de
prioridade (varredura linear, heap mínimo e buckets de Dial), a partir do TCC
de RODRIGUES (2023).

## Reproduzir tudo

```bash
make experimento    # roda as medições (~1min30 na máquina de referência)
make                # gera figuras, tabelas e compila o PDF
```

Requisitos: `g++` (C++17), `python3` com `matplotlib`, LaTeX com `babel-portuges`,
`booktabs`, `listings`, `algpseudocode`.

## Arquivos

| Arquivo | Conteúdo |
|---|---|
| `main.tex` | relatório (7 seções + declaração de IA + apêndices) |
| `codigo/dijkstra_experimento.cpp` | as três variantes, gerador de grafos, validação e medição |
| `codigo/gerar_graficos.py` | agrega os CSV, gera as figuras e `resultados/tabelas.tex` |
| `resultados/resultados_brutos.csv` | todas as medições individuais |
| `resultados/estatisticas_estruturais.csv` | `Dmax` e varreduras de bucket |
| `resultados/log_execucao.txt` | log, incluindo o resultado da validação cruzada |
| `figuras/*.pdf` | os gráficos usados no relatório |
