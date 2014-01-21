S=slides

all: $S.pdf

$S.pdf: *.tex 
	pdflatex $S.tex
	pdflatex $S.tex

_$S.pdf: *.tex 
	teximporter $S.tex > _$S.pdf
	pdflatex _$S.tex
	pdflatex _$S.tex

clean:
	rm -f *.aux *.toc *. *.bbl *.blg *.pdf *.log *.nav *.out *.snm *.vrb
