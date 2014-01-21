N=article

$N.pdf: *.tex bib/*.bib
	pdflatex $N
	bibtex $N
	pdflatex $N
	pdflatex $N

check:
	hunspell -t *.tex
	# hunspell -l en -c -t *.tex

clean:
	rm -f *.aux *.toc *. *.bbl *.blg *.log *.nav *.out *.snm *.vrb *.vtc
