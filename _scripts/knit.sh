#! /usr/bin/Rscript --vanilla

knit <- function (inputFile, outputFile) {
 
  # read input file
  input <- readInputFile(inputFile)
  
  # per-document cache and figure paths
  stem <- tools::file_path_sans_ext(inputFile)
  prefix <- paste(stem, "-", sep="")
  knitr::opts_chunk$set(cache.path=file.path('cache', prefix),
                        fig.path=file.path('figure', prefix))
  knitr::opts_chunk$set(tidy=FALSE, comment=NA)
  
  # configure output options
  knitr::pat_md()
  knitr::opts_knit$set(out.format = 'markdown')
  renderJekyll()
   
  # do the knit
  knitr::knit(text = input, output = outputFile)
}

readInputFile <- function(inputFile) {
  
  # read input file
  input <- readLines(inputFile)
  
  # convert cpp to Rmd if necessary
  if(tools::file_ext(inputFile) == "cpp") 
    input <- cppToRmd(input)
  
  # read and validate that we have front matter
  delimiters <- grep("^---\\s*$", input)
  if (length(delimiters) < 2)
     stop("No front-matter section found in post")
  if (delimiters[2] - delimiters[1] <= 1)	
     stop("Empty front-matter section in post")	
  frontMatter <- input[(delimiters[1]+1):(delimiters[2]-1)]
    
  # validate and ammend front matter as necessary
  hasField <- function(field) {
    any(grepl(paste("^", field, ":.*$" , sep=""), frontMatter))
  }
  verifyField <- function(field) {
    if (!hasField(field))
      stop("No ", field, " field specified in front-matter")
  }
  
  # verify required fields
  verifyField("title")
  verifyField("author")
  verifyField("summary")
  license <- stripWhitespace(readField(frontMatter, "license"))
  if (!identical(license, "MIT"))
    stop("You must include a license field specifying the MIT license")
  
  # read tags and create tag directories as necessary
  tagsField <- readField(frontMatter, "tags")
  if (!is.null(tagsField)) {
    if (grepl(",", tagsField, fixed=TRUE))
      stop("tags must be separated by spaces rather than commas")
    srcDir <- normalizePath(dirname(inputFile))
    siteDir <- dirname(srcDir)
    ensureTagDirs(siteDir, tagsField)
  } else {
    warning("No tags specified for article: ", inputFile)
  }
  
  # provide layout
  if (!hasField("layout"))
    frontMatter <- append(frontMatter, "layout: post")
  
  # add src field
  src <- basename(inputFile)
  frontMatter <- append(frontMatter, paste("src:", src))
  
  # recompose modified document
  input <- c(input[1:delimiters[1]],  
             frontMatter, 
             input[delimiters[2]:length(input)])
}

# read the value of a front-matter field
readField <- function(frontMatter, field) {
  res <- regexec(paste("^", field, ":(.*)$" , sep=""), frontMatter)
  matches <- regmatches(frontMatter, res)
  for (match in matches) {
    if (length(match) == 2)
      return(match[[2]])
  }
  return(NULL)
}   

# strip whitespace from a string
stripWhitespace <- function(x) {
  if (!is.null(x))
    gsub("^\\s+|\\s+$", "", x)
  else
    NULL
}

# ensure that all tags have a tags directory
ensureTagDirs <- function(siteDir, tagsField) {
  
  # parse tags field into a list of tags
  tags <- strsplit(tagsField, "\\s+")
  tags <- stripWhitespace(tags[[1]])
  tags <- tags[nzchar(tags)]
  
  # ensure main tags dir
  tagsDir <- file.path(siteDir, "tags")
  if (!file.exists(tagsDir))
    dir.create(tagsDir)
  
  # create tag directories and index pages if necessary
  for (tag in tags) {
    
    tagDir <- file.path(tagsDir, tag)
    if (!file.exists(tagDir))
      dir.create(tagDir)
    
    tagIndex <- file.path(tagDir, "index.html")
    if (!file.exists(tag)) {
      index <- paste("---\n",
                     "layout: tag\n",
                     "title: ", basename(tagDir), "\n",
                     "---\n\n",
                     "{% include tag_page.html %}\n",
                     sep = "")
      cat(index, file=tagIndex)
    }
  }   
}

cppToRmd <- function(input) {
  
  # break into chunks
  chunks <- cppChunks(input)
  
  # generate Rmd
  rmdLines <- character()
  
  frontMatter <- TRUE
  for (chunk in chunks) {     
    switch(attr(chunk, "type"),
           doxygen = {  
             rmdLines <- c(rmdLines, doxygenChunkToRmd(chunk, frontMatter))
             frontMatter <- FALSE
           },
           r = {
             rmdLines <- c(rmdLines, "```{r}", 
                                     stripWhitespaceLines(chunk), 
                                     "```")
           },
           cpp = {
             rmdLines <- c(rmdLines, "```{r engine='Rcpp'}", 
                                     stripWhitespaceLines(chunk), 
                                     "```")
           }
    )
  } 

  return(rmdLines)
}

cppChunks <- function(input) {
  
  # chunk management
  chunkLines <- character()
  chunks <- list()
  addChunk <- function(chunks, type, lines) {
      if (!all(grepl("^\\s*$", lines)))
        chunks[[length(chunks)+1]] <- structure(lines, type = type)
      return(chunks)
  }
  
  # parcel the lines into chunks
  inDoxygen <- FALSE
  inR <- FALSE
  for (line in input) {
    
    # start R
    if (grepl("^\\s*\\/\\*{3,}\\s*[Rr]\\s*$", line)) {
      chunks <- addChunk(chunks, "cpp", chunkLines)
      chunkLines <- character()
      inR <- TRUE
    }
    
    # start doxygen
    else if (grepl("^\\/\\*[\\*\\!].*$", line)) {
      chunks <- addChunk(chunks, "cpp", chunkLines)
      chunkLines <- character()
      inDoxygen <- TRUE
    }
    
    # end comment
    else if (grepl("^.*\\*\\/.*$", line)) {
      if (inDoxygen) {
        chunks <- addChunk(chunks, "doxygen", chunkLines)
        chunkLines <- character()
        inDoxygen <- FALSE
      }
      else if (inR) {
        chunks <- addChunk(chunks, "r", chunkLines)
        chunkLines <- character()
        inR <- FALSE
      }
      else {
        chunkLines <- append(chunkLines, line)
      }
    }
    
    # append to current chunk lines
    else {
      chunkLines <- append(chunkLines, line)
    }
  }
  
  # whatever is left over is cpp
  chunks <- addChunk(chunks, "cpp", chunkLines)
    
  return(chunks)
}

doxygenChunkToRmd <- function(chunk, frontMatter) {
  
  # start rmdLines 
  rmdLines <- character()
  if (frontMatter)
    rmdLines <- c(rmdLines, "---")
  else
    rmdLines <- c(rmdLines, "")
  
  # examine chunk lines 
  kwRes <- regexec("^\\s\\*\\s+@(\\w+)\\s+(.+)$", chunk)
  kwMatches <-regmatches(chunk, kwRes)
  contentRes <- regexec("^\\s\\*(\\s+)(.+)$", chunk)
  contentMatches <- regmatches(chunk, contentRes)
  
  for (i in 1:length(chunk)) {
    
    # get matches
    kwMatch <- kwMatches[i][[1]]
    contentMatch <- contentMatches[i][[1]]
     
    # check for a doxygen field
    if (length(kwMatch) == 3) {
      rmdLines <- c(rmdLines, paste(kwMatch[2], ": ", kwMatch[3], sep=""))
    }
    
    # check for a line of content
    else if (length(contentMatch) == 3) {
      
      # front matter is handled specially
      if (frontMatter) {
        if (nchar(contentMatch[2]) > 1) {
          rmdLines <- c(rmdLines, paste("  ", contentMatch[3], sep=""))
        } else {
          rmdLines <- c(rmdLines, "---")
          rmdLines <- c(rmdLines, contentMatch[3])
          frontMatter <- FALSE
        }
      } else {
        rmdLines <- c(rmdLines, contentMatch[3])  
      }
    }
    
    # empty line could mean the end of front matter
    else if (frontMatter) {
      rmdLines <- c(rmdLines, "---")
      frontMatter <- FALSE
    }
    
    # just an empty line
    else {
      rmdLines <- c(rmdLines, "")
    }
  }
  
  # if we're still in front matter then exit it
  if (frontMatter) 
    rmdLines <- c(rmdLines, "---")
    
  return(rmdLines)
}


# adaption of knitr::render_jekyll
renderJekyll <- function(extra = '') {
  knitr::render_markdown(TRUE)
  hook.r = function(x, options) {
      stringr::str_c('\n\n{% highlight ', tolower(options$engine), 
                     if (extra != '') ' ', extra, ' %}\n',
                    x, 
                    '{% endhighlight %}\n\n')
  }
  hook.t = function(x, options) {
    stringr::str_c('\n\n<pre class="output">\n', 
                   x, 
                   '</pre>\n\n')
  }
  hook.o = function(x, options) {
    if (knitr:::output_asis(x, options)) 
      x 
    else 
      hook.t(x, options)
  } 
  knitr::knit_hooks$set(source = hook.r, output = hook.o, warning = hook.t,
                        error = hook.t, message = hook.t)
}


stripWhitespaceLines <- function(chunk) {
  wsLines <- grepl("^\\s*$", chunk)
  firstLine <- match(FALSE, wsLines, nomatch = 1)
  lastLine <- length(wsLines) - match(FALSE, rev(wsLines), nomatch = 1) + 1
  return(chunk[firstLine:lastLine])
}


# get arguments and call knit
set.seed(123)
args <- commandArgs(TRUE)
inputFile <- args[1]
outputFile <- args[2]
knit(inputFile, outputFile)

