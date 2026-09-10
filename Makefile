# Reproduz o experimento inteiro e recompila o relatorio.
#   make            -> experimento + figuras + tabelas + PDF
#   make experimento -> apenas roda as medicoes (regrava os CSV)
#   make relatorio   -> apenas recompila o PDF
#   make limpar      -> remove arquivos auxiliares do LaTeX

CXX      := g++
CXXFLAGS := -O2 -std=c++17

all: relatorio

codigo/dijkstra: codigo/dijkstra_experimento.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

experimento: codigo/dijkstra
	cd codigo && ./dijkstra > ../resultados/resultados_brutos.csv \
	                       2> ../resultados/log_execucao.txt
	cd codigo && ./dijkstra stats > ../resultados/estatisticas_estruturais.csv

resultados/tabelas.tex: resultados/resultados_brutos.csv codigo/gerar_graficos.py
	cd codigo && python3 gerar_graficos.py

relatorio: resultados/tabelas.tex
	pdflatex -interaction=nonstopmode main.tex
	pdflatex -interaction=nonstopmode main.tex

limpar:
	rm -f main.aux main.log main.out main.toc

.PHONY: all experimento relatorio limpar
