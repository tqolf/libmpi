# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

|              description              | average time | average time(reference) | coefficient of variation | perfermance ratio |
| :-: | :-: | :-: | :-: | :-: |
|       from-string(mpi vs ossl)        |   2109.99    |          29136          |        0.0218565         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">13.8086</span>      |
|        to-string(mpi vs ossl)         |   1090.12    |         2900.52         |        0.0310607         |      <span style="color:#008000;font-weight:bold;">2.66074</span>      |
|       from-octets(mpi vs ossl)        |   234.615    |         775.323         |        0.0660814         |      <span style="color:#008000;font-weight:bold;">3.30466</span>      |
|        to-octets(mpi vs ossl)         |   105.258    |         1534.69         |        0.0974715         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">14.5803</span>      |
|           add(mpi vs ossl)            |    41.739    |         291.574         |         0.154812         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.98564</span>      |
|        add-assign(mpi vs ossl)        |   41.1192    |         294.894         |         0.155973         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.17168</span>      |
|           sub(mpi vs ossl)            |    47.659    |         169.216         |         0.144881         |      <span style="color:#008000;font-weight:bold;">3.55057</span>      |
|        sub-assign(mpi vs ossl)        |    45.519    |         294.054         |         0.148319         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.46002</span>      |
|           mul(mpi vs ossl)            |   1898.62    |         12062.3         |        0.0230759         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.35319</span>      |
|           sqr(mpi vs ossl)            |   1063.06    |         8376.64         |        0.0306988         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.87976</span>      |
|           div(mpi vs ossl)            |   2756.28    |          38027          |         0.019224         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">13.7965</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.50551e+06  |       1.40876e+07       |        0.00124217        |      <span style="color:#008000;font-weight:bold;">4.01871</span>      |
|      montgomery-exp(mpi vs ossl)      | 9.52743e+06  |       1.30129e+08       |       0.000447768        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">13.6584</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.34459e+07  |       1.38193e+08       |       0.000440759        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">10.2777</span>      |
|          MUL2(a * 2 = a + a)          |   27.7196    |      <span style="font-style:italic;">No Reference</span>       |         0.190105         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   61.8986    |      <span style="font-style:italic;">No Reference</span>       |         0.127133         |        N/A        |
