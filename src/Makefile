KNIT = ../_scripts/knit.sh
POSTS_DIR = ../_posts
MD_FILES := $(patsubst %.cpp, $(POSTS_DIR)/%.md ,$(wildcard *.cpp)) \
            $(patsubst %.Rmd, $(POSTS_DIR)/%.md, $(wildcard *.Rmd))

all: $(MD_FILES)
	
$(POSTS_DIR)/%.md: %.cpp
	$(KNIT) $< $@

$(POSTS_DIR)/%.md: %.Rmd
	$(KNIT) $< $@

.PHONY: clean
clean:
	$(RM) *.md; $(RM) *.html
	$(RM) ../_posts/*.md
	$(RM) -rf ../tags

