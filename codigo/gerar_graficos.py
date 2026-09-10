#!/usr/bin/env python3
"""
Agrega os tempos medidos, gera as figuras do relatorio e escreve
../resultados/tabelas.tex, que e incluido pelo main.tex. Assim, ao
reexecutar o experimento em outra maquina, tabelas e graficos do
relatorio sao atualizados automaticamente.

Uso:  python3 gerar_graficos.py
Entradas:  ../resultados/resultados_brutos.csv
           ../resultados/estatisticas_estruturais.csv
Saidas:    ../figuras/*.pdf  e  ../resultados/tabelas.tex
"""
import csv
import math
import statistics as st
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

RES = "../resultados/"
FIGS = "../figuras/"
ALGS = ["Dijkstra Normal", "Dijkstra Heap Minimo", "Dijkstra Bucket"]
ROTULO = {
    "Dijkstra Normal": r"Dijkstra Normal  ($O(n^2)$)",
    "Dijkstra Heap Minimo": r"Dijkstra Heap Mínimo  ($O(n \log n)$)",
    "Dijkstra Bucket": r"Dijkstra Bucket  ($O(n + D_{max})$)",
}
COR = {"Dijkstra Normal": "#c0392b",
       "Dijkstra Heap Minimo": "#2471a3",
       "Dijkstra Bucket": "#1e8449"}
MARCA = {"Dijkstra Normal": "o", "Dijkstra Heap Minimo": "s", "Dijkstra Bucket": "^"}

# ----------------------------------------------------------------- leitura
linhas = list(csv.DictReader(open(RES + "resultados_brutos.csv")))
bruto = defaultdict(list)
m_de_n = {}
for r in linhas:
    n = int(r["n"])
    bruto[(n, r["algoritmo"])].append(float(r["tempo_ms"]))
    m_de_n[n] = int(r["m"])
ns = sorted({int(r["n"]) for r in linhas})
reps = len(bruto[(ns[0], ALGS[0])])

media = {k: st.mean(v) for k, v in bruto.items()}
desvio = {k: st.stdev(v) if len(v) > 1 else 0.0 for k, v in bruto.items()}

try:
    estrut = list(csv.DictReader(open(RES + "estatisticas_estruturais.csv")))
except FileNotFoundError:
    estrut = []


def razoes(alg):
    return [media[(ns[i + 1], alg)] / media[(ns[i], alg)] for i in range(len(ns) - 1)]


def expoente(alg):
    xs = [math.log(n) for n in ns]
    ys = [math.log(media[(n, alg)]) for n in ns]
    mx, my = st.mean(xs), st.mean(ys)
    return (sum((x - mx) * (y - my) for x, y in zip(xs, ys))
            / sum((x - mx) ** 2 for x in xs))


# ------------------------------------------------- Figura 1: tempo x n
fig, eixos = plt.subplots(1, 2, figsize=(11, 4.2))
for ax, log in zip(eixos, [False, True]):
    for a in ALGS:
        ax.plot(ns, [media[(n, a)] for n in ns], marker=MARCA[a], color=COR[a],
                lw=1.8, ms=5, label=ROTULO[a])
    ax.set_xlabel("$n$ = número de vértices")
    ax.set_ylabel("tempo médio de execução (ms)")
    ax.grid(alpha=.3, ls=":")
    ax.set_title("escala logarítmica nos dois eixos" if log else "escala linear",
                 fontsize=10)
    if log:
        ax.set_xscale("log"); ax.set_yscale("log")
eixos[0].legend(fontsize=8, loc="upper left")
fig.tight_layout()
fig.savefig(FIGS + "fig_tempo_vs_n.pdf")
plt.close(fig)

# ------------------------------- Figura 2: empirico x curva teorica
modelos = {
    "Dijkstra Normal": (lambda n: n ** 2, r"$c\,n^2$"),
    "Dijkstra Heap Minimo": (lambda n: n * math.log2(n), r"$c\,n\log_2 n$"),
    "Dijkstra Bucket": (lambda n: n, r"$c\,n$"),
}
fig, eixos = plt.subplots(1, 3, figsize=(13, 3.9))
for ax, a in zip(eixos, ALGS):
    f, rot = modelos[a]
    c = media[(ns[0], a)] / f(ns[0])          # constante calibrada em n0
    ax.plot(ns, [media[(n, a)] for n in ns], marker=MARCA[a], color=COR[a],
            lw=1.8, ms=5, label="medido")
    ax.plot(ns, [c * f(n) for n in ns], ls="--", color="black", lw=1.4,
            label="previsto " + rot)
    ax.set_xscale("log"); ax.set_yscale("log")
    ax.set_title(ROTULO[a], fontsize=9)
    ax.set_xlabel("$n$"); ax.set_ylabel("tempo (ms)")
    ax.grid(alpha=.3, ls=":")
    ax.legend(fontsize=8)
fig.tight_layout()
fig.savefig(FIGS + "fig_teoria_vs_empirico.pdf")
plt.close(fig)

# --------------------------- Figura 3: razoes de crescimento T(2n)/T(n)
fig, ax = plt.subplots(figsize=(6.8, 3.9))
x = ns[1:]
for a in ALGS:
    ax.plot(x, razoes(a), marker=MARCA[a], color=COR[a], lw=1.8, ms=5, label=ROTULO[a])
ax.axhline(4, ls="--", c="#c0392b", lw=1, alpha=.7)
ax.axhline(2, ls="--", c="#2471a3", lw=1, alpha=.7)
ax.text(x[0], 4.1, r"referência 4 (esperado para $n^2$)", fontsize=7, color="#c0392b")
ax.text(x[0], 2.06, r"referência 2 (esperado para $n$)", fontsize=7, color="#2471a3")
ax.set_xscale("log")
ax.set_xlabel(r"$n$ (cada ponto compara $T(n)$ com $T(n/2)$)")
ax.set_ylabel(r"$T(2n)\,/\,T(n)$")
ax.grid(alpha=.3, ls=":")
ax.legend(fontsize=8)
fig.tight_layout()
fig.savefig(FIGS + "fig_razoes.pdf")
plt.close(fig)

# --------------------------------------------------- tabelas em LaTeX
def mil(v):
    return f"{v:,}".replace(",", ".")


def linhas_medias():
    out = []
    for n in ns:
        cel = " & ".join(f"{media[(n,a)]:.3f} & {desvio[(n,a)]:.3f}" for a in ALGS)
        out.append(f"{mil(n)} & {mil(m_de_n[n])} & {cel} \\\\")
    return "\n".join(out)


def linhas_razoes():
    rz = {a: razoes(a) for a in ALGS}
    out = []
    for i, n in enumerate(ns[1:]):
        cel = " & ".join(f"{rz[a][i]:.2f}" for a in ALGS)
        out.append(f"{mil(ns[i])} $\\rightarrow$ {mil(n)} & {cel} \\\\")
    return "\n".join(out)


def linhas_constantes():
    out = []
    for n in ns:
        out.append(f"{mil(n)} & {media[(n,'Dijkstra Normal')]/n**2*1e6:.3f} & "
                   f"{media[(n,'Dijkstra Heap Minimo')]/(n*math.log2(n))*1e6:.2f} & "
                   f"{media[(n,'Dijkstra Bucket')]/n*1e6:.1f} \\\\")
    return "\n".join(out)


def linhas_estruturais():
    out = []
    for r in estrut:
        n = int(r["n"])
        mem = 9 * n / 1024.0     # dist[] (8 B) + fechado[] (1 B) por vertice
        out.append(f"{mil(n)} & {mil(int(r['dmax']))} & "
                   f"{mil(int(r['varreduras_bucket']))} & {mem:.0f} \\\\")
    return "\n".join(out)


with open(RES + "tabelas.tex", "w") as f:
    f.write("% Arquivo gerado automaticamente por codigo/gerar_graficos.py\n")
    f.write("% Nao editar a mao: reexecute o script apos novas medicoes.\n")
    f.write("\\newcommand{\\numRepeticoes}{%d}\n" % reps)
    relmax = max(desvio[k] / media[k] for k in media)
    f.write("\\newcommand{\\desvioRelMax}{%.1f}\n" % (100 * relmax))
    relmaior = max(desvio[(n, a)] / media[(n, a)] for n in ns[1:] for a in ALGS)
    f.write("\\newcommand{\\desvioRelMaior}{%.1f}\n" % (100 * relmaior))
    f.write("\\newcommand{\\linhasMedias}{%%\n%s}\n" % linhas_medias())
    f.write("\\newcommand{\\linhasRazoes}{%%\n%s}\n" % linhas_razoes())
    f.write("\\newcommand{\\linhasConstantes}{%%\n%s}\n" % linhas_constantes())
    f.write("\\newcommand{\\linhasEstruturais}{%%\n%s}\n" % linhas_estruturais())
    for a, nome in zip(ALGS, ["Normal", "Heap", "Bucket"]):
        f.write("\\newcommand{\\expoente%s}{%.2f}\n" % (nome, expoente(a)))
    f.write("\\newcommand{\\ganhoHeap}{%.0f}\n"
            % (media[(ns[-1], "Dijkstra Normal")] / media[(ns[-1], "Dijkstra Heap Minimo")]))
    f.write("\\newcommand{\\ganhoBucket}{%.0f}\n"
            % (media[(ns[-1], "Dijkstra Normal")] / media[(ns[-1], "Dijkstra Bucket")]))
    f.write("\\newcommand{\\heapSobreBucket}{%.1f}\n"
            % (media[(ns[-1], "Dijkstra Heap Minimo")] / media[(ns[-1], "Dijkstra Bucket")]))
    f.write("\\newcommand{\\nMin}{%s}\n\\newcommand{\\nMax}{%s}\n" % (mil(ns[0]), mil(ns[-1])))
    f.write("\\newcommand{\\tempoMaxNormal}{%.0f}\n" % media[(ns[-1], "Dijkstra Normal")])
    f.write("\\newcommand{\\tempoMaxHeap}{%.1f}\n" % media[(ns[-1], "Dijkstra Heap Minimo")])
    f.write("\\newcommand{\\tempoMaxBucket}{%.1f}\n" % media[(ns[-1], "Dijkstra Bucket")])

print("tabelas.tex e figuras gerados com sucesso.")
print("expoentes empiricos:", {a: round(expoente(a), 3) for a in ALGS})
