
## Jekyll 1.0 broke with the pre-1.0 invocation,
## so let us test if we have major release 0 or 1
## awk get the version token, cut picks off the first part
jmaj	:= $(shell jekyll --version 2>&1 | awk '/^Jekyll/ {print $$2}' | cut -d. -f1 )

all: knit jek img

jek: knit
ifeq ($(jmaj),0)
	jekyll --no-server --no-auto
else
	jekyll build
endif

knit:
	$(MAKE) -C src

img:
	$(MAKE) -C src/figure

.PHONY: clean img
clean:
	$(RM) -rf _site
	$(MAKE) clean -C src

preview: knit
ifeq ($(jmaj),0)
	jekyll --server --no-auto --no-pygments
else
	jekyll serve --watch
endif
