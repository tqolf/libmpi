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
|       from-string(mpi vs ossl)        |   1735.18    |         23491.6         |        0.0584451         |      <span style="color:#8000;">**13.5384**</span>      |
|        to-string(mpi vs ossl)         |    565.28    |         2294.88         |         0.09732          |      <span style="color:#8000;">**4.05972**</span>      |
|       from-octets(mpi vs ossl)        |    182.48    |          623.3          |        0.0999351         |      <span style="color:#8000;">**3.41572**</span>      |
|        to-octets(mpi vs ossl)         |    131.78    |         1234.94         |         0.173033         |      <span style="color:#8000;">**9.37123**</span>      |
|           add(mpi vs ossl)            |    45.36     |         260.48          |         0.212884         |      <span style="color:#8000;">**5.7425**</span>       |
|        add-assign(mpi vs ossl)        |    44.12     |         281.06          |         0.216103         |      <span style="color:#8000;">**6.37035**</span>      |
|           sub(mpi vs ossl)            |    56.04     |          131.8          |         0.18143          |      <span style="color:#8000;">**2.35189**</span>      |
|        sub-assign(mpi vs ossl)        |    59.78     |         296.64          |         0.175714         |      <span style="color:#8000;">**4.96219**</span>      |
|           mul(mpi vs ossl)            |   1583.64    |         9054.44         |        0.0787427         |      <span style="color:#8000;">**5.71749**</span>      |
|           sqr(mpi vs ossl)            |    817.42    |         6439.54         |        0.0426073         |      <span style="color:#8000;">**7.87788**</span>      |
|           div(mpi vs ossl)            |   2860.76    |         29879.1         |         0.055969         |      <span style="color:#8000;">**10.4445**</span>      |
|      gcd_consttime(mpi vs ossl)       | 4.03556e+06  |       1.26262e+07       |        0.0367229         |      <span style="color:#8000;">**3.12873**</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.29787e+06  |       1.11105e+08       |        0.0432311         |      <span style="color:#8000;">**13.3896**</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.15184e+07  |       1.07129e+08       |        0.0299762         |      <span style="color:#8000;">**9.30065**</span>      |
|          MUL2(a * 2 = a + a)          |     21.8     |      <span style="font-style:italic;">No Reference</span>       |         0.214176         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   51.1004    |      <span style="font-style:italic;">No Reference</span>       |         0.203722         |        N/A        |
