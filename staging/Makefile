KNIT = ../_scripts/knit.sh
POSTS_DIR = ../_posts
CPP_FILES := $(wildcard *.cpp)
RMD_FILES := $(wildcard *.Rmd)
MD_FILES := $(patsubst %.cpp, $(POSTS_DIR)/%.md, ${CPP_FILES}) \
            $(patsubst %.Rmd, $(POSTS_DIR)/%.md, ${RMD_FILES})

all: $(MD_FILES)
	
$(POSTS_DIR)/%.md: %.cpp
	cp "$<" "../src/$<"
	$(KNIT) $< $@

$(POSTS_DIR)/%.md: %.Rmd
	cp "$<" "../src/$<"
	$(KNIT) $< $@

.PHONY: clean
clean:
	$(RM) ${MD_FILES}

