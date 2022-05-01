# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

| description | average time | average time(reference) | coefficient of variation | perfermance ratio | 
| :-: | :-: | :-: | :-: | :-: |
| from-string(mpi vs ossl) | 2482.98 | 29177.4 | 0.101201 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">11.7509</span> | 
| to-string(mpi vs ossl) | 1411.24 | 3082.19 | 0.0267725 | <span style="color:#008000;font-weight:bold;">2.18403</span> | 
| from-octets(mpi vs ossl) | 233.45 | 714.63 | 0.0654494 | <span style="color:#008000;font-weight:bold;">3.06117</span> | 
| to-octets(mpi vs ossl) | 133.166 | 1548.76 | 0.0866605 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">11.6304</span> | 
| add(mpi vs ossl) | 49.062 | 321.013 | 0.142776 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.54301</span> | 
| add-assign(mpi vs ossl) | 50.1422 | 321.173 | 0.141224 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.40525</span> | 
| sub(mpi vs ossl) | 66.7228 | 168.687 | 0.137598 | <span style="color:#008000;font-weight:bold;">2.52818</span> | 
| sub-assign(mpi vs ossl) | 65.2426 | 331.474 | 0.205869 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.08063</span> | 
| mul(mpi vs ossl) | 2018.64 | 12226.3 | 0.120029 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.05668</span> | 
| sqr(mpi vs ossl) | 1085.53 | 8245.82 | 0.0305599 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.59616</span> | 
| MUL2(a * 2 = a + a) | 24.621 | <span style="font-style:italic;">No Reference</span> | 0.20154 | N/A | 
| MUL2(a * 2 = a << 1) | 55.9622 | <span style="font-style:italic;">No Reference</span> | 0.133679 | N/A | 
