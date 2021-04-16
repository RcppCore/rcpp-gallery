// 4/15/2020 Zach DeBruine (zach.debruine@vai.org)
// Please raise issues on github.com/zdebruine/RcppSparse/issues
//
// This header file extends the Rcpp namespace with a dgCMatrix sparse matrix class
// This class is documented at github.com/zdebruine/RcppSparse

#include <rcpp.h>

namespace Rcpp {
    class dgCMatrix {
    public:
        IntegerVector i, p, Dim;
        NumericVector x;
        List Dimnames;

        // constructors
        dgCMatrix(IntegerVector& A_i, IntegerVector& A_p, NumericVector& A_x, int nrow) {
            i = A_i;
            p = A_p;
            x = A_x;
            Dim = IntegerVector::create(nrow, A_p.size() - 1);
        };
        dgCMatrix(IntegerVector& A_i, IntegerVector& A_p, NumericVector& A_x, int nrow, List& A_Dimnames) {
            i = A_i;
            p = A_p;
            x = A_x;
            Dim = IntegerVector::create(nrow, A_p.size() - 1);
            Dimnames = A_Dimnames;
        };
        dgCMatrix(S4 mat) {
            i = mat.slot("i");
            p = mat.slot("p");
            x = mat.slot("x");
            Dim = mat.slot("Dim");
            Dimnames = mat.slot("Dimnames");
        };

        // basic properties
        int nrow() { return Dim[0]; };
        int ncol() { return Dim[1]; };
        int rows() { return Dim[0]; };
        int cols() { return Dim[1]; };
        int n_nonzero() { return x.size(); };
        NumericVector& nonzeros() { return x; };
        double sum() { return Rcpp::sum(x); };

        // forward constant iterator
        class const_iterator {
        public:
            int index;
            const_iterator(dgCMatrix& g, int ind) : parent(g) { index = ind; }
            bool operator!=(const_iterator x) const { return index != x.index; };
            bool operator==(const_iterator x) const { return index == x.index; };
            bool operator<(const_iterator x) const { return index < x.index; };
            bool operator>(const_iterator x) const { return index > x.index; };
            const_iterator& operator++(int) { ++index; return (*this); };
            const_iterator& operator--(int) { --index; return (*this); };
            int row() { return parent.i[index]; };
            int col() { int j = 0; for (; j < parent.p.size(); ++j) if (parent.p[j] >= index) break; return j; };
            double& operator*() { return parent.x[index]; };
        private:
            dgCMatrix& parent;
        };

        // iterator constructors
        const_iterator begin(int j) { return const_iterator(*this, (int)0); };
        const_iterator end(int j) { return const_iterator(*this, i.size() - 1); };
        const_iterator begin_col(int j) { return const_iterator(*this, p[j]); };
        const_iterator end_col(int j) { return const_iterator(*this, p[j + 1]); };

        // read-only element access
        double at(int row, int col) const {
            for (int j = p[col]; j < p[col + 1]; ++j) {
                if (i[j] == row) return x[j];
                else if (i[j] > row) break;
            }
            return 0.0;
        }
        double operator()(int row, int col) { return at(row, col); };
        NumericVector operator()(int row, IntegerVector& col) {
            NumericVector res(col.size());
            for (int j = 0; j < col.size(); ++j) res[j] = at(row, col[j]);
            return res;
        };
        NumericVector operator()(IntegerVector& row, int col) {
            NumericVector res(row.size());
            for (int j = 0; j < row.size(); ++j) res[j] = at(row[j], col);
            return res;
        };
        NumericMatrix operator()(IntegerVector& row, IntegerVector& col) {
            NumericMatrix res(row.size(), col.size());
            for (int j = 0; j < row.size(); ++j)
                for (int k = 0; k < col.size(); ++k)
                    res(j, k) = at(row[j], col[k]);
            return res;
        };

        // column access (copy)
        NumericVector col(int col) {
            NumericVector c(Dim[0], 0.0);
            for (int j = p[col]; j < p[col + 1]; ++j)
                c[i[j]] = x[j];
            return c;
        }
        NumericVector column(int c) { return col(c); }
        NumericMatrix cols(IntegerVector& c) {
            NumericMatrix res(Dim[0], c.size());
            for (int j = 0; j < c.size(); ++j) {
                res.column(j) = col(c[j]);
            }
            return res;
        }
        NumericMatrix columns(IntegerVector& c) { return cols(c); }

        // row access (copy)
        NumericVector row(int row) {
            NumericVector r(Dim[1], 0.0);
            for (int col = 0; col < Dim[1]; ++col) {
                for (int j = p[col]; j < p[col + 1]; ++j) {
                    if (i[j] == row) r[col] = x[j];
                    else if (i[j] > row) break;
                }
            }
            return r;
        }
        NumericMatrix rows(IntegerVector& r) {
            NumericMatrix res(r.size(), Dim[1]);
            for (int j = 0; j < r.size(); ++j) {
                res.row(j) = row(r[j]);
            }
            return res;
        }

        // colSums and rowSums family
        NumericVector colSums() {
            NumericVector sums(Dim[1]);
            for (int col = 0; col < Dim[1]; ++col)
                for (int j = p[col]; j < p[col + 1]; ++j)
                    sums(col) += x[j];
            return sums;
        }
        NumericVector rowSums() {
            NumericVector sums(Dim[0]);
            for (int col = 0; col < Dim[1]; ++col)
                for (int j = p[col]; j < p[col + 1]; ++j)
                    sums(i[j]) += x[j];
            return sums;
        }
        NumericVector colMeans() {
            NumericVector sums = colSums();
            for (int i = 0; i < sums.size(); ++i) sums[i] = sums[i] / Dim[0];
            return sums;
        };
        NumericVector rowMeans() {
            NumericVector sums = rowSums();
            for (int i = 0; i < sums.size(); ++i) sums[i] = sums[i] / Dim[1];
            return sums;
        };

        // crossprod
        NumericMatrix crossprod() {
            NumericMatrix res(Dim[1], Dim[1]);
            #if defined(_OPENMP)
            #pragma omp parallel for
            #endif
            for (int col1 = 0; col1 < Dim[1]; ++col1) {
                for (int col2 = col1; col2 < Dim[1]; ++col2) {
                    if (col1 == col2) {
                        for (int j = p[col1]; j < p[col1 + 1]; ++j)
                            res(col1, col1) += x[j] * x[j];
                    } else {
                        int col1_ind = p[col1], col1_max = p[col1 + 1];
                        int col2_ind = p[col2], col2_max = p[col2 + 1];
                        while (col1_ind < col1_max && col2_ind < col2_max) {
                            int row1 = i[col1_ind];
                            int row2 = i[col2_ind];
                            if (row1 == row2) {
                                res(col1, col2) += x[col1_ind] * x[col2_ind];
                                ++col1_ind; ++col2_ind;
                            } else if (row1 < row2) {
                                do { ++col1_ind; } while (i[col1_ind] < row2 && col1_ind < col1_max);
                            } else if (row2 < row1) {
                                do { ++col2_ind; } while (i[col2_ind] < row1 && col2_ind < col2_max);
                            }
                        }
                        res(col2, col1) = res(col1, col2);
                    }
                }
            }
            return res;
        }
    };

    // Rcpp::as
    template <> dgCMatrix as(SEXP mat) { return dgCMatrix(mat); }

    // Rcpp::wrap
    template <> SEXP wrap(const dgCMatrix& sm) {
        S4 s(std::string("dgCMatrix"));
        s.slot("i") = sm.i;
        s.slot("p") = sm.p;
        s.slot("x") = sm.x;
        s.slot("Dim") = sm.Dim;
        s.slot("Dimnames") = sm.Dimnames;
        return s;
    }
}