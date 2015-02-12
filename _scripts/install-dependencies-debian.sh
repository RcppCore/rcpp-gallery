# jekyll and related
sudo apt-get install ruby-dev
sudo gem install therubyracer
sudo gem install jekyll

# system dependencies of gallery articles
sudo apt-get install libgsl0-dev libboost-all-dev python2.7 python2.7-dev libgmp3-dev
                     
# R packages
R --slave -e 'install.packages(c("Rcpp", "RcppEigen", "RcppArmadillo", "RcppProgress", "RcppGSL", "RcppParallel", "TTR", "xts", "BH", "bigmemory", "fields", "bayesm", "mvtnorm", "numbers", "Zelig", "knitr", "benchmark", "rbenchmark", "microbenchmark", "sensitivity"), repos = "http://cran.rstudio.com")'
