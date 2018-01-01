PIP_DEPS=nltk
PIP_BIN=pip3
PYTHON_BIN=python3
EXEC=antispam

dep:
	$(PIP_BIN) install --user $(PIP_DEPS)
	$(PYTHON_BIN) -c "import nltk; nltk.download('stopwords'); nltk.download('punkt')"

zip:
	rm -fv xsumsa01.zip
	cp doc/antispam.pdf .
	zip -r xsumsa01.zip \
		Makefile \
		.classifier \
		antispam \
		spamfilters \
		antispam.pdf \
		-x \*.pyc -x \*.swp
	rm -fv antispam.pdf
	ls -lh xsumsa01.zip

all: dep
	chmod +x $(EXEC)
