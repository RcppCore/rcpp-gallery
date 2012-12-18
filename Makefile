
all: knit
	jekyll --no-server --no-auto

knit:
	$(MAKE) -C src

.PHONY: clean
clean:
	$(RM) -rf _site
	$(MAKE) clean -C src

preview: knit
	jekyll --server --auto --url http://localhost:4000

