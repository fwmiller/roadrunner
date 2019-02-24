all: pdf ps

pdf: dvi
	dvipdfm kernel.dvi

ps: dvi
	dvips -f kernel.dvi > kernel.ps

dvi:
	latex kernel.tex
	latex kernel.tex

clean:
	$(RM) *~ *.toc *.aux *.dvi *.log *.pdf *.ps
