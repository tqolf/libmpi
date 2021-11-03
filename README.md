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
|       from-string(mpi vs ossl)        |   1873.22    |         25056.6         |        0.0239124         |      <span style="color:#8000;">**13.3763**</span>      |
|        to-string(mpi vs ossl)         |    510.11    |         2005.24         |         0.045557         |      <span style="color:#8000;">**3.93099**</span>      |
|       from-octets(mpi vs ossl)        |   246.165    |         491.089         |        0.0861525         |      <span style="color:#8000;">1.99496</span>      |
|        to-octets(mpi vs ossl)         |   114.182    |         1784.25         |        0.0935861         |      <span style="color:#8000;">**15.6264**</span>      |
|           add(mpi vs ossl)            |   66.9014    |         221.824         |         0.122267         |      <span style="color:#8000;">**3.31569**</span>      |
|        add-assign(mpi vs ossl)        |   68.3612    |         225.284         |         0.120981         |      <span style="color:#8000;">**3.2955**</span>       |
|           sub(mpi vs ossl)            |   55.5812    |         146.283         |         0.13415          |      <span style="color:#8000;">**2.63188**</span>      |
|        sub-assign(mpi vs ossl)        |   60.3612    |         224.324         |         0.128715         |      <span style="color:#8000;">**3.71637**</span>      |
|           mul(mpi vs ossl)            |   1605.85    |         9963.33         |        0.0250714         |      <span style="color:#8000;">**6.20439**</span>      |
|           sqr(mpi vs ossl)            |   991.419    |         6907.27         |         0.031949         |      <span style="color:#8000;">**6.96706**</span>      |
|           div(mpi vs ossl)            |   3358.81    |         31953.8         |         0.017279         |      <span style="color:#8000;">**9.51345**</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.86789e+06  |       1.20016e+07       |       0.000676397        |      <span style="color:#8000;">**3.10288**</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.57652e+06  |       1.0776e+08        |       0.000738746        |      <span style="color:#8000;">**12.5645**</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.18151e+07  |       1.14888e+08       |         0.000312         |      <span style="color:#8000;">**9.72385**</span>      |
|          MUL2(a * 2 = a + a)          |   55.9412    |      <span style="font-style:italic;">No Reference</span>       |         0.140636         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   67.2814    |      <span style="font-style:italic;">No Reference</span>       |         0.133672         |        N/A        |
