CMD=latexmk
FLAGS=-pvc
N=main

$N.pdf: *.tex bib/*.bib
	pdflatex $N
	bibtex $N
	pdflatex $N

lmk:
	$(CMD) $(FLAGS)

check:
	hunspell -t tex/*.tex 
	# aspell -l en -c -C -t *.tex 
	# #	-C: Consider run-together words valid
	# # -c:	Spell-check a single file
	# # -l: Language  to  use
	# # -t: Mode tex

clean:
	latexmk -C
