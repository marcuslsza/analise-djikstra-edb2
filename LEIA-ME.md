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

## Importante antes de entregar

1. **Rodem `make experimento` na máquina do grupo.** Os números que estão nos
   CSV vieram da máquina descrita na Tabela 1 do relatório (um container Linux
   com Xeon 2.10 GHz, 1 núcleo). Tabelas e figuras do relatório são geradas a
   partir dos CSV, então basta reexecutar para tudo se atualizar — mas a
   Tabela 1 do `main.tex` precisa ser editada à mão com a configuração real.
2. **Procurem por `\pendente` no `main.tex`.** Cada ocorrência aparece em
   vermelho no PDF e marca algo que só vocês podem preencher: nomes e
   matrículas, palavras-chave e base usadas na busca, datas de acesso, link do
   repositório do grupo e a declaração de uso de IA.
3. A declaração de uso de IA está escrita conforme o uso que houve na produção
   deste material; ajustem para refletir o processo real do grupo.

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
