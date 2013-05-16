
all: knit
	jekyll build

knit:
	$(MAKE) -C src

.PHONY: clean
clean:
	$(RM) -rf _site
	$(MAKE) clean -C src

preview: knit
	jekyll serve --watch

