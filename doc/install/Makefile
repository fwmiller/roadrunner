all: pdf ps

pdf: dvi
	dvipdfm install.dvi

ps: dvi
	dvips -f install.dvi > install.ps

dvi:
	latex install.tex
	latex install.tex

clean:
	$(RM) *~ *.toc *.aux *.dvi *.log *.pdf *.ps
