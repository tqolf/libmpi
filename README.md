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
|       from-string(mpi vs ossl)        |   2081.36    |         25101.4         |        0.0641143         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.0601</span>      |
|        to-string(mpi vs ossl)         |   1021.97    |         2238.46         |        0.0356639         |      <span style="color:#008000;font-weight:bold;">2.19034</span>      |
|       from-octets(mpi vs ossl)        |   265.583    |         611.947         |         0.164523         |      <span style="color:#008000;font-weight:bold;">2.30416</span>      |
|        to-octets(mpi vs ossl)         |   131.341    |         1367.13         |        0.0968235         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">10.409</span>       |
|           add(mpi vs ossl)            |   85.1408    |         267.103         |         0.130136         |      <span style="color:#008000;font-weight:bold;">3.13719</span>      |
|        add-assign(mpi vs ossl)        |   68.3008    |         250.943         |         0.178361         |      <span style="color:#008000;font-weight:bold;">3.67408</span>      |
|           sub(mpi vs ossl)            |   62.2406    |         161.582         |         0.126757         |      <span style="color:#008000;font-weight:bold;">2.59608</span>      |
|        sub-assign(mpi vs ossl)        |   56.1406    |         255.423         |         0.133528         |      <span style="color:#008000;font-weight:bold;">4.5497</span>       |
|           mul(mpi vs ossl)            |    1774.8    |         9947.93         |        0.0644442         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.6051</span>       |
|           sqr(mpi vs ossl)            |   881.269    |         6939.15         |        0.0703219         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.87404</span>      |
|           div(mpi vs ossl)            |   3001.13    |         29362.9         |        0.0858123         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.78395</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.70471e+06  |       1.16537e+07       |        0.0245831         |      <span style="color:#008000;font-weight:bold;">3.14564</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.51874e+06  |       1.02819e+08       |        0.0605523         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.0697</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.31388e+07  |       1.18071e+08       |         0.130605         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">8.9864</span>       |
|          MUL2(a * 2 = a + a)          |   57.9008    |      <span style="font-style:italic;">No Reference</span>       |         0.131464         |        N/A        |
|         MUL2(a * 2 = a << 1)          |    80.681    |      <span style="font-style:italic;">No Reference</span>       |         0.125999         |        N/A        |
