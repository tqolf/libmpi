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
|       from-string(mpi vs ossl)        |   2128.53    |         24333.7         |        0.0217132         |      <span style="color:#8000;">**11.4321**</span>      |
|        to-string(mpi vs ossl)         |   573.883    |         2245.09         |        0.0419402         |      <span style="color:#8000;">**3.91211**</span>      |
|       from-octets(mpi vs ossl)        |   237.621    |         550.363         |        0.0648722         |      <span style="color:#8000;">**2.31614**</span>      |
|        to-octets(mpi vs ossl)         |   115.921    |         1272.17         |        0.0967531         |      <span style="color:#8000;">**10.9745**</span>      |
|           add(mpi vs ossl)            |   69.9002    |         221.501         |         0.11967          |      <span style="color:#8000;">**3.16882**</span>      |
|        add-assign(mpi vs ossl)        |   68.5604    |         224.121         |         0.120773         |      <span style="color:#8000;">**3.26896**</span>      |
|           sub(mpi vs ossl)            |   55.1604    |         143.761         |         0.134647         |      <span style="color:#8000;">**2.60623**</span>      |
|        sub-assign(mpi vs ossl)        |   59.9604    |         230.301         |         0.129162         |      <span style="color:#8000;">**3.84089**</span>      |
|           mul(mpi vs ossl)            |   1605.87    |         9881.91         |        0.0250884         |      <span style="color:#8000;">**6.15363**</span>      |
|           sqr(mpi vs ossl)            |   977.965    |         6914.46         |        0.0332621         |      <span style="color:#8000;">**7.07025**</span>      |
|           div(mpi vs ossl)            |   3047.48    |         31359.8         |        0.0181186         |      <span style="color:#8000;">**10.2904**</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.87555e+06  |       1.19771e+07       |        0.00469403        |      <span style="color:#8000;">**3.09044**</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.49188e+06  |       1.06468e+08       |       0.000704219        |      <span style="color:#8000;">**12.5376**</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.18403e+07  |       1.13805e+08       |        0.00145203        |      <span style="color:#8000;">**9.61171**</span>      |
|          MUL2(a * 2 = a + a)          |   30.8402    |      <span style="font-style:italic;">No Reference</span>       |         0.180124         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   47.6804    |      <span style="font-style:italic;">No Reference</span>       |         0.144823         |        N/A        |
