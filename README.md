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
|       from-string(mpi vs ossl)        |   1946.23    |         23951.1         |        0.0226987         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.3064</span>      |
|        to-string(mpi vs ossl)         |   931.352    |         2300.31         |        0.0328518         |      <span style="color:#008000;font-weight:bold;">2.46986</span>      |
|       from-octets(mpi vs ossl)        |   199.423    |         548.567         |        0.0708133         |      <span style="color:#008000;font-weight:bold;">2.75078</span>      |
|        to-octets(mpi vs ossl)         |   114.342    |         1270.18         |        0.0935261         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">11.1086</span>      |
|           add(mpi vs ossl)            |   41.8606    |         222.763         |         0.154565         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.32154</span>      |
|        add-assign(mpi vs ossl)        |   41.9406    |         223.463         |         0.154424         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.32808</span>      |
|           sub(mpi vs ossl)            |   50.5406    |         144.602         |         0.140666         |      <span style="color:#008000;font-weight:bold;">2.8611</span>       |
|        sub-assign(mpi vs ossl)        |   49.7806    |         226.363         |         0.141775         |      <span style="color:#008000;font-weight:bold;">4.54721</span>      |
|           mul(mpi vs ossl)            |   1626.48    |         9934.15         |        0.0249115         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.10776</span>      |
|           sqr(mpi vs ossl)            |   1012.55    |         6920.19         |        0.0323246         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.8344</span>       |
|           div(mpi vs ossl)            |   3011.92    |         31948.2         |        0.0185004         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">10.6073</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.12333e+06  |       1.19363e+07       |       0.000772273        |      <span style="color:#008000;font-weight:bold;">3.82167</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.58493e+06  |       1.04249e+08       |       0.000376109        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.1433</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.18506e+07  |       1.10744e+08       |       0.000529352        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.34497</span>      |
|          MUL2(a * 2 = a + a)          |   22.4804    |      <span style="font-style:italic;">No Reference</span>       |         0.210937         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   51.3806    |      <span style="font-style:italic;">No Reference</span>       |         0.139516         |        N/A        |
